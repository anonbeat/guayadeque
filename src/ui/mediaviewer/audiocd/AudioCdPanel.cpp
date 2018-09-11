// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 3, or (at your option)
//    any later version.
//
//    This Program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; see the file LICENSE.  If not, write to
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "AudioCdPanel.h"

#include "Accelerators.h"
#include "Config.h"
#include "EditWithOptions.h"
#include "EventCommandIds.h"
#include "Images.h"
#include "MainFrame.h"
#include "MusicBrainz.h"

#include <gst/gst.h>
#include <gst/tag/tag.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guAudioCdReadTocThread : public wxThread
{
  protected :
    guAudioCdPanel *    m_AudioCdPanel;
    GstElement *        m_Pipeline;
    bool                m_Running;
    guTrackArray        m_CdTracks;
    wxString            m_MBDiscId;

    bool                BuildPipeline( void );

  public :
    guAudioCdReadTocThread( guAudioCdPanel * audiocdpanel );
    ~guAudioCdReadTocThread();

    virtual ExitCode    Entry();

    void                Stop( void );

    guTrackArray &      GetCdTracks( void ) { return m_CdTracks; }
    void                SetMbDiscId( const wxString &mbdiscid );
};

// -------------------------------------------------------------------------------- //
extern "C" {

// -------------------------------------------------------------------------------- //
static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guAudioCdReadTocThread * pobj )
{
    //guLogMessage( wxT( "Got gstreamer message %u" ), GST_MESSAGE_TYPE( message ) );
    switch( GST_MESSAGE_TYPE( message ) )
    {
        case GST_MESSAGE_ERROR :
        {
            GError * err;
            gchar * debug;
            gst_message_parse_error( message, &err, &debug );
            guLogError( wxT( "Gstreamer error '%s'" ), wxString( err->message, wxConvUTF8 ).c_str() );
            g_error_free( err );
            g_free( debug );

            pobj->Stop();
            break;
        }

        case GST_MESSAGE_EOS :
        {
          //guLogMessage( wxT( "EOS Detected..." ) );
          pobj->Stop();
          break;
        }

        case GST_MESSAGE_TOC :
        {
            GstToc * Toc;
            gst_message_parse_toc( message, &Toc, NULL );
            if( Toc )
            {
              guTrackArray & CdTracks = pobj->GetCdTracks();

              GList * TocEntries = gst_toc_get_entries( Toc );
              if( TocEntries && CdTracks.Count() <= g_list_length( TocEntries ) )
              {
                int Index = 0;
                for( GList * Node = TocEntries; Node != NULL; Node = Node->next )
                {
                  GstTocEntry * TocEntry = static_cast<GstTocEntry*>( Node->data );
                  gint64 Length = 0;
                  gint64 Start;
                  gint64 Stop;
                  if( gst_toc_entry_get_start_stop_times( TocEntry, &Start, &Stop ) )
                    Length = Stop - Start;

                  CdTracks[ Index ].m_Length = ( Length / GST_MSECOND );
                  Index++;
                }
              }
            }
            break;
        }

        case GST_MESSAGE_TAG :
        {
          GstTagList * tags = NULL;
          gst_message_parse_tag( message, &tags );
          char * MBDiscId = NULL;
          if( gst_tag_list_get_string( tags, GST_TAG_CDDA_MUSICBRAINZ_DISCID, &MBDiscId ) )
          {
            pobj->SetMbDiscId( wxString( MBDiscId, wxConvUTF8 ) );
            g_free( MBDiscId );
            gst_tag_list_free( tags );
          }
          break;
        }

        default :
            break;
    }

    return TRUE;
}

}


// -------------------------------------------------------------------------------- //
guAudioCdReadTocThread::guAudioCdReadTocThread( guAudioCdPanel * audiocdpanel )
{
    m_AudioCdPanel = audiocdpanel;
    m_Running = false;
    m_Pipeline = NULL;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guAudioCdReadTocThread::~guAudioCdReadTocThread()
{
    if( GST_IS_ELEMENT( m_Pipeline ) )
    {
        gst_element_set_state( m_Pipeline, GST_STATE_NULL );
        gst_object_unref( GST_OBJECT( m_Pipeline ) );
        m_Pipeline = NULL;
    }

    if( !TestDestroy() )
    {
        m_AudioCdPanel->SetAudioCdTracks( m_CdTracks );
    }
}

// -------------------------------------------------------------------------------- //
bool guAudioCdReadTocThread::BuildPipeline( void )
{
  m_Pipeline = gst_pipeline_new( "guPipeline" );
  if( GST_IS_ELEMENT( m_Pipeline ) )
  {
    GstElement * src = gst_element_make_from_uri( GST_URI_SRC, "cdda://", "guAudioCdSrc", NULL );
    if( GST_IS_ELEMENT( src ) )
    {
      if( gst_element_set_state( src, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE ||
          gst_element_set_state( src, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
      {
        guLogMessage( wxT( "Could not start the cdda pipeline." ) );
        gst_element_set_state( src, GST_STATE_NULL );
      }
      else
      {
        GstFormat Format = gst_format_get_by_nick( "track" );
        GstFormat OutFormat = Format;
        gint64 TrackCount = 0;
        if( !gst_element_query_duration( src, OutFormat, &TrackCount ) || OutFormat != Format )
        {
          guLogMessage( wxT( "Error reading tracks from cdda element" ) );
        }
        else
        {
          // Create the tracks...
          for( int Index = 0; Index < TrackCount; Index++ )
          {
            guTrack * CdTrack = new guTrack();
            if( CdTrack )
            {
              CdTrack->m_SongId = Index + 1;
              CdTrack->m_Number = Index + 1;
              CdTrack->m_Type = guTRACK_TYPE_AUDIOCD;
              CdTrack->m_FileName = wxString::Format( wxT( "cdda://%i" ), CdTrack->m_Number );
              CdTrack->m_SongName = wxString::Format( wxT( "Track%03i" ), CdTrack->m_Number );
              m_CdTracks.Add( CdTrack );
            }
            else
            {
              guLogMessage( wxT( "Could not create cd track object." ) );
            }
          }

          // Need musicbrainz diskid tag
          gst_tag_register_musicbrainz_tags();

          GstElement * fake = gst_element_factory_make( "fakesink", "guFakeSink" );
          if( GST_IS_ELEMENT( fake ) )
          {
            g_object_set( G_OBJECT( fake ), "sync", 0, NULL );

            gst_bin_add_many( GST_BIN( m_Pipeline ), src, fake, NULL );

            g_object_set( m_Pipeline, "async-handling", true, NULL );

            GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Pipeline ) );
            gst_bus_add_watch( bus, ( GstBusFunc ) gst_bus_async_callback, this );
            gst_object_unref( G_OBJECT( bus ) );

            if( gst_element_link( src, fake ) )
            {
                gst_element_set_state( m_Pipeline, GST_STATE_READY );
                gst_element_set_state( m_Pipeline, GST_STATE_PAUSED );

                return true;
            }
            else
            {
                guLogMessage( wxT( "Error linking the cdda elements" ) );
            }

            gst_object_unref( fake );
          }
          else
          {
            guLogError( wxT( "Error creating the AcousticId sink" ) );
          }

        }
      }

      gst_object_unref( src );
    }
    else
    {
      guLogMessage( wxT( "Could not create the gstreamer element for cdda." ) );
    }

  }

  return false;
}

// -------------------------------------------------------------------------------- //
guAudioCdReadTocThread::ExitCode guAudioCdReadTocThread::Entry()
{
    if( BuildPipeline() )
    {
        gst_element_set_state( m_Pipeline, GST_STATE_PLAYING );

        m_Running = true;
        while( !TestDestroy() && m_Running )
        {
            Sleep( 1000 );
            //gint64 Pos = -1;
            //gst_element_query_position( m_Pipeline, GST_FORMAT_TIME, &Pos );
            //guLogMessage( wxT( "Running...%llu" ), Pos );
        }
        //guLogMessage( wxT( "Finished guAcousticIdThread..." ) );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guAudioCdReadTocThread::SetMbDiscId( const wxString &mbdiscid )
{
    m_MBDiscId = wxString( mbdiscid );
    guLogMessage( wxT( "The MB DiskId is %s" ), m_MBDiscId.c_str() );
    guMusicBrainz MusicBrainz;
    guMBReleaseArray MBReleases;

    int TrackCount = m_CdTracks.Count();
    //guLogMessage( wxT( "There was %i tracks..." ), TrackCount );
    if( TrackCount && ( MusicBrainz.GetDiscIdReleases( m_MBDiscId, &MBReleases ) > 0 ) )
    {
        // MusicBrainz is very strict with request rate... wait 1 second
        wxSleep( 1 );

        // Get the release track list
        int ReleaseCount = MBReleases.GetCount();
        //guLogMessage( wxT( "Got %i releases"), ReleaseCount );
        for( int ReleaseIndex = 0; ReleaseIndex < ReleaseCount; ReleaseIndex++ )
        {
            MusicBrainz.GetRecordings( MBReleases[ ReleaseIndex ] );
            wxSleep( 1 );
            //guLogMessage( wxT( "Got %li tracks in this release..." ), MBReleases[ ReleaseIndex ].m_Recordings.Count() );
        }
        for( int ReleaseIndex = 0; ReleaseIndex < ReleaseCount; ReleaseIndex++ )
        {
            guMBRelease & MBRelease = MBReleases[ ReleaseIndex ];
            if( TrackCount == ( int ) MBRelease.m_Recordings.Count() )
            {
                for( int TrackIndex = 0; TrackIndex < TrackCount; TrackIndex++ )
                {
                    guTrack & Track = m_CdTracks[ TrackIndex ];
                    Track.m_SongName = MBRelease.m_Recordings[ TrackIndex ].m_Title;
                    Track.m_AlbumArtist = MBRelease.m_ArtistName;
                    Track.m_ArtistName = MBRelease.m_Recordings[ TrackIndex ].m_ArtistName;
                    if( Track.m_ArtistName.IsEmpty() )
                        Track.m_ArtistName = Track.m_AlbumArtist;
                    Track.m_AlbumName = MBRelease.m_Title;
                    Track.m_Year = MBRelease.m_Year;
                    Track.m_Format = wxT( "cdda" );
                }
            }
        }
    }

    Stop();
}

// -------------------------------------------------------------------------------- //
void guAudioCdReadTocThread::Stop( void )
{
    m_Running = false;
}



// -------------------------------------------------------------------------------- //
guCdTracksListBox::guCdTracksListBox( wxWindow * parent ) :
    guListView( parent, wxLB_MULTIPLE|guLISTVIEW_COLUMN_SELECT|
                guLISTVIEW_COLUMN_SORTING|guLISTVIEW_COLUMN_CLICK_EVENTS|
                guLISTVIEW_ALLOWDRAG )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_LastColumnRightClicked = wxNOT_FOUND;
    m_LastRowRightClicked = wxNOT_FOUND;

    m_ColumnNames.Add( wxT( "#" ) );
    m_ColumnNames.Add( _( "Title" ) );
    m_ColumnNames.Add( _( "Artist" ) );
    m_ColumnNames.Add( _( "Album Artist" ) );
    m_ColumnNames.Add( _( "Album" ) );
    m_ColumnNames.Add( _( "Genre" ) );
    m_ColumnNames.Add( _( "Composer" ) );
    m_ColumnNames.Add( _( "Disk" ) );
    m_ColumnNames.Add( _( "Length" ) );
    m_ColumnNames.Add( _( "Year" ) );

    m_Order = Config->ReadNum( CONFIG_KEY_AUDIOCD_ORDER, 0, CONFIG_PATH_AUDIOCD );
    m_OrderDesc = Config->ReadNum( CONFIG_KEY_AUDIOCD_ORDERDESC, false, CONFIG_PATH_AUDIOCD );

    int ColId;
    wxString ColName;
    int Count = m_ColumnNames.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "id%u" ), Index ), Index, wxT( "audiocd/columns/ids" ) );

        ColName = m_ColumnNames[ ColId ];

        ColName += ( ( ColId == m_Order ) ? ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "width%u" ), Index ), 80, wxT( "audiocd/columns/widths" ) ),
            Config->ReadBool( wxString::Format( wxT( "show%u" ), Index ), true, wxT( "audiocd/columns/shows" ) )
            );
        InsertColumn( Column );
    }

    Bind( guConfigUpdatedEvent, &guCdTracksListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );
    Bind( guEVT_LISTBOX_ITEM_COL_RCLICKED, &guCdTracksListBox::OnItemColumnRClicked, this );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guCdTracksListBox::~guCdTracksListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    int Count = m_ColumnNames.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "id%u" ), Index ),
                          ( * m_Columns )[ Index ].m_Id, wxT( "audiocd/columns/ids" ) );
        Config->WriteNum( wxString::Format( wxT( "width%u" ), Index ),
                          ( * m_Columns )[ Index ].m_Width, wxT( "audiocd/columns/widths" ) );
        Config->WriteBool( wxString::Format( wxT( "show%u" ), Index ),
                           ( * m_Columns )[ Index ].m_Enabled, wxT( "audiocd/columns/shows" ) );
    }

    Config->WriteNum( CONFIG_KEY_AUDIOCD_ORDER, m_Order, CONFIG_PATH_AUDIOCD );
    Config->WriteBool( CONFIG_KEY_AUDIOCD_ORDERDESC, m_OrderDesc, CONFIG_PATH_AUDIOCD );


    Unbind( guConfigUpdatedEvent, &guCdTracksListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );
    Unbind( guEVT_LISTBOX_ITEM_COL_RCLICKED, &guCdTracksListBox::OnItemColumnRClicked, this );
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );

    RealAccelCmds.Add( ID_AUDIOCD_ITEM_PLAY );
    RealAccelCmds.Add( ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_AUDIOCD_ITEM_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ARTIST );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
wxString guCdTracksListBox::OnGetItemText( const int row, const int col ) const
{
    guTrack * Track;
    //guLogMessage( wxT( "GetItemText( %i, %i )    %i-%i  %i" ), row, col, m_ItemsFirst, m_ItemsLast, m_Items.Count() );

    Track = &m_AudioCdTracks[ row ];

    switch( ( * m_Columns )[ col ].m_Id )
    {
        case guAUDIOCD_COLUMN_NUMBER :
          if( Track->m_Number )
            return wxString::Format( wxT( "%u" ), Track->m_Number );
          else
            return wxEmptyString;

        case guAUDIOCD_COLUMN_TITLE  :
          return Track->m_SongName;

        case guAUDIOCD_COLUMN_ARTIST :
          return Track->m_ArtistName;

        case guAUDIOCD_COLUMN_ALBUMARTIST :
            return Track->m_AlbumArtist;

        case guAUDIOCD_COLUMN_ALBUM :
          return Track->m_AlbumName;

        case guAUDIOCD_COLUMN_GENRE :
          return Track->m_GenreName;

        case guAUDIOCD_COLUMN_COMPOSER :
          return Track->m_Composer;

        case guAUDIOCD_COLUMN_DISK :
          return Track->m_Disk;

        case guAUDIOCD_COLUMN_LENGTH :
          return LenToString( Track->m_Length );

        case guAUDIOCD_COLUMN_YEAR :
            if( Track->m_Year )
                return wxString::Format( wxT( "%u" ), Track->m_Year );
            else
                return wxEmptyString;
    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
void guCdTracksListBox::GetItemsList( void )
{
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::SetTrackList( guTrackArray &audiocdtracks )
{
    m_AudioCdTracks.Clear();
    int Count = audiocdtracks.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        m_AudioCdTracks.Add( new guTrack( audiocdtracks[ Index ] ) );
    }
    ReloadItems( true );
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::ClearTracks( void )
{
    m_AudioCdTracks.Clear();
    ReloadItems( true );
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::UpdatedTracks( guTrackArray * tracks )
{
    int ItemCount = m_AudioCdTracks.Count();

    if( !ItemCount )
        return;

    int TrackCount = tracks->Count();
    for( int TrackIndex = 0; TrackIndex < TrackCount; TrackIndex++ )
    {
        guTrack &CurTrack = tracks->Item( TrackIndex );

        for( int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
        {
            if( ( CurTrack.m_SongId == m_AudioCdTracks[ ItemIndex ].m_SongId ) )
            {
                m_AudioCdTracks[ ItemIndex ] = CurTrack;
                RefreshRow( ItemIndex );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::ReloadItems( bool reset )
{
    //
    wxArrayInt Selection;
    int FirstVisible = 0;

    if( reset )
    {
        SetSelection( -1 );
    }
    else
    {
        Selection = GetSelectedItems( false );
        FirstVisible = GetVisibleRowsBegin();
    }

    //m_AudioCdTracks.Empty();

    GetItemsList();

    SetItemCount( m_AudioCdTracks.Count() );

    if( !reset )
    {
        SetSelectedItems( Selection );
        ScrollToRow( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::OnItemColumnRClicked( wxListEvent &event )
{
    m_LastColumnRightClicked = event.m_col;
    m_LastRowRightClicked = event.GetInt();
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::AppendFastEditMenu( wxMenu * menu, const int selcount ) const
{
    wxMenuItem * MenuItem;
    //guLogMessage( wxT( "guCdTracksListBox::AppendFastEditMenu %i - %i" ), m_LastColumnRightClicked, m_LastRowRightClicked );
    if( m_LastColumnRightClicked == wxNOT_FOUND || ( m_LastColumnRightClicked >= ( int ) m_Columns->Count() ) )
    {
        return;
    }

    int ColumnId = GetColumnId( m_LastColumnRightClicked );

    // If its a column not editable
    if( ColumnId == guAUDIOCD_COLUMN_NUMBER ||
        ColumnId == guAUDIOCD_COLUMN_LENGTH )
        return;

    menu->AppendSeparator();

    wxString MenuText = _( "Edit" );
    MenuText += wxT( " " ) + m_ColumnNames[ ColumnId ];

    MenuItem = new wxMenuItem( menu, ID_AUDIOCD_EDIT_COLUMN,  MenuText, _( "Edit the clicked column for the selected tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
    menu->Append( MenuItem );

    if( selcount > 1 )
    {
        MenuText = _( "Set" );
        MenuText += wxT( " " ) + m_ColumnNames[ ColumnId ] + wxT( " " );
        MenuText += _( "to" );
        wxString ItemText = OnGetItemText( m_LastRowRightClicked, m_LastColumnRightClicked );
        ItemText.Replace( wxT( "&" ), wxT( "&&" ) );
        MenuText += wxT( " '" ) + ItemText + wxT( "'" );

        MenuItem = new wxMenuItem( menu, ID_AUDIOCD_SET_COLUMN,  MenuText, _( "Set the clicked column for the selected tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        menu->Append( MenuItem );
    }
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::CreateContextMenu( wxMenu * Menu ) const
{
    int SelCount = GetSelectedCount();
    if( SelCount )
    {
        wxMenuItem * MenuItem;
        MenuItem = new wxMenuItem( Menu, ID_AUDIOCD_ITEM_PLAY,
                                wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                                _( "Play current selected tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ALL,
                                wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                                _( "Add current selected songs to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_AUDIOCD_REFRESH_METADATA,
                                        _( "Search again" ),
                                        _( "Search again for cd metadata" ) );
        Menu->Append( MenuItem );


        AppendFastEditMenu( Menu, SelCount );

        Menu->AppendSeparator();

        guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
        MainFrame->CreateCopyToMenu( Menu );
    }
    else
    {
        wxMenuItem * MenuItem;
        MenuItem = new wxMenuItem( Menu, wxID_ANY, _( "No selected items..." ), _( "Copy the current selected tracks to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_status_error ) );
        Menu->Append( MenuItem );
    }
}

// -------------------------------------------------------------------------------- //
int inline guCdTracksListBox::GetItemId( const int row ) const
{
    return m_AudioCdTracks[ row ].m_SongId;
}

// -------------------------------------------------------------------------------- //
wxString inline guCdTracksListBox::GetItemName( const int row ) const
{
    return m_AudioCdTracks[ row ].m_SongName;
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::SetOrder( int columnid )
{
    if( m_Order != columnid )
    {
        m_Order = columnid;
        m_OrderDesc = ( columnid != 0 );
    }
    else
        m_OrderDesc = !m_OrderDesc;

    int CurColId;
    int count = m_ColumnNames.Count();
    for( int Index = 0; Index < count; Index++ )
    {
        CurColId = GetColumnId( Index );
        SetColumnLabel( Index,
            m_ColumnNames[ CurColId ]  + ( ( CurColId == m_Order ) ?
                ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::GetAllSongs( guTrackArray * tracks )
{
    int Count = GetItemCount();
    for( int Index = 0; Index < Count; Index++ )
    {
        tracks->Add( new guTrack( m_AudioCdTracks[ Index ] ) );
    }
}

// -------------------------------------------------------------------------------- //
int guCdTracksListBox::GetSelectedSongs( guTrackArray * tracks, const bool isdrag ) const
{
    unsigned long cookie;
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        tracks->Add( new guTrack( m_AudioCdTracks[ item ] ) );
        item = GetNextSelected( cookie );
    }
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
wxVariant guCdTracksListBox::GetLastDataClicked( void )
{
    guTrack * Track = &m_AudioCdTracks[ m_LastRowRightClicked ];

    int ColId = GetColumnId( m_LastColumnRightClicked );
    switch( ColId )
    {
        case guSONGS_COLUMN_TITLE :
            return wxVariant( Track->m_SongName );

        case guSONGS_COLUMN_ARTIST :
            return wxVariant( Track->m_ArtistName );

        case guSONGS_COLUMN_ALBUMARTIST :
            return wxVariant( Track->m_AlbumArtist );

        case guSONGS_COLUMN_ALBUM :
            return wxVariant( Track->m_AlbumName );

        case guSONGS_COLUMN_GENRE :
            return wxVariant( Track->m_GenreName );

        case guSONGS_COLUMN_COMPOSER :
            return wxVariant( Track->m_Composer );

        case guSONGS_COLUMN_DISK :
            return wxVariant( Track->m_Disk );

        case guSONGS_COLUMN_YEAR :
            return wxVariant( ( long ) Track->m_Year );
    }

    return wxVariant();
}

// -------------------------------------------------------------------------------- //
void guCdTracksListBox::GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
    int Count = m_AudioCdTracks.Count();
    * count = Count;
    * len = 0;
    * size = 0;
    for( int Index = 0; Index < Count; Index++ )
    {
        * len += m_AudioCdTracks[ Index ].m_Length;
    }
}




// -------------------------------------------------------------------------------- //
guAudioCdPanel::guAudioCdPanel( wxWindow * parent, guPlayerPanel * playerpanel ) :
    wxPanel( parent )
{
    m_PlayerPanel = playerpanel;

    wxBoxSizer * MainSizer;
    MainSizer = new wxBoxSizer( wxVERTICAL );

    m_CdTracksListBox = new guCdTracksListBox( this );
    MainSizer->Add( m_CdTracksListBox, 1, wxEXPAND, 5 );

    SetSizer( MainSizer );
    Layout();

    Bind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdTrackPlay, this, ID_AUDIOCD_ITEM_PLAY );
    Bind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdTrackEnqueue, this, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ALL, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ARTIST );
    m_CdTracksListBox->Bind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdTrackCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    m_CdTracksListBox->Bind( wxEVT_LISTBOX_DCLICK, &guAudioCdPanel::OnAudioCdTrackActivated, this );

    //Bind( guConfigUpdatedEvent, &guAudioCdPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );
    Bind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdRefresh, this, ID_AUDIOCD_REFRESH_METADATA );
    Bind( wxEVT_MENU, &guAudioCdPanel::OnSongSetField, this, ID_AUDIOCD_SET_COLUMN );
    Bind( wxEVT_MENU, &guAudioCdPanel::OnSongEditField, this, ID_AUDIOCD_EDIT_COLUMN );

    guAudioCdReadTocThread * AudioCdReadTocThread = new guAudioCdReadTocThread( this );
    if( !AudioCdReadTocThread )
    {
        guLogMessage( wxT( "Could not create the audio cd read toc thread" ) );
    }
}

// -------------------------------------------------------------------------------- //
guAudioCdPanel::~guAudioCdPanel()
{
    Unbind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdTrackPlay, this, ID_AUDIOCD_ITEM_PLAY );
    Unbind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdTrackEnqueue, this, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ALL, ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ARTIST );
    m_CdTracksListBox->Unbind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdTrackCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    m_CdTracksListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guAudioCdPanel::OnAudioCdTrackActivated, this );

    //Unbind( guConfigUpdatedEvent, &guAudioCdPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );
    Unbind( wxEVT_MENU, &guAudioCdPanel::OnAudioCdRefresh, this, ID_AUDIOCD_REFRESH_METADATA );
    Unbind( wxEVT_MENU, &guAudioCdPanel::OnSongSetField, this, ID_AUDIOCD_SET_COLUMN );
    Unbind( wxEVT_MENU, &guAudioCdPanel::OnSongEditField, this, ID_AUDIOCD_EDIT_COLUMN );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
    m_CdTracksListBox->GetCounters( count, len, size );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::SetAudioCdTracks( guTrackArray &audiocdtracks )
{
    m_CdTracksListBox->SetTrackList( audiocdtracks );

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::UpdateVolume( const bool added )
{
    guLogMessage( wxT( "The audio cd volume changed " ) );
    if( added )
    {
        guAudioCdReadTocThread * AudioCdReadTocThread = new guAudioCdReadTocThread( this );
        if( !AudioCdReadTocThread )
        {
            guLogMessage( wxT( "Could not create the audio cd read toc thread" ) );
        }
    }
    else
    {
        m_CdTracksListBox->ClearTracks();

        wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
        AddPendingEvent( event );
    }
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnSelectAudioCdTracks( const bool enqueue, const int aftercurrent )
{
    guTrackArray Tracks;

    m_CdTracksListBox->GetSelectedSongs( &Tracks );

    if( !Tracks.Count() )
    {
        m_CdTracksListBox->GetAllSongs( &Tracks );
    }

    if( Tracks.Count() )
    {
        if( enqueue )
        {
            m_PlayerPanel->AddToPlayList( Tracks, true, aftercurrent );
        }
        else
        {
            m_PlayerPanel->SetPlayList( Tracks );
        }
    }

}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnAudioCdTrackPlay( wxCommandEvent &event )
{
    OnSelectAudioCdTracks( false );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnAudioCdTrackEnqueue( wxCommandEvent &event )
{
    OnSelectAudioCdTracks( true, event.GetId() - ID_AUDIOCD_ITEM_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnAudioCdTrackCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_CdTracksListBox->GetSelectedSongs( Tracks );


    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
    {
        event.SetId( ID_MAINFRAME_COPYTO );
    }
    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( guMainFrame::GetMainFrame(), event );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnAudioCdTrackActivated( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    OnSelectAudioCdTracks( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false, CONFIG_PATH_GENERAL ) );
}


// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnAudioCdRefresh( wxCommandEvent &event )
{
    UpdateVolume( true );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnSongSetField( wxCommandEvent &event )
{
    int ColumnId = m_CdTracksListBox->GetColumnId( m_CdTracksListBox->GetLastColumnClicked() );
    //guLogMessage( wxT( "guLibPanel::OnSongSetField %i" ), ColumnId );

    guTrackArray Tracks;
    m_CdTracksListBox->GetSelectedSongs( &Tracks );

    wxVariant NewData = m_CdTracksListBox->GetLastDataClicked();

    //guLogMessage( wxT( "Setting Data to : %s" ), NewData.GetString().c_str() );

    // This should be done in a thread for huge selections of tracks...
    int Count = Tracks.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &Tracks[ Index ];
        switch( ColumnId )
        {
            case guSONGS_COLUMN_NUMBER :
                Track->m_Number = NewData.GetLong();
                break;

            case guSONGS_COLUMN_TITLE :
                Track->m_SongName = NewData.GetString();
                break;

            case guSONGS_COLUMN_ARTIST :
                Track->m_ArtistName = NewData.GetString();
                break;

            case guSONGS_COLUMN_ALBUMARTIST :
                Track->m_AlbumArtist = NewData.GetString();
                break;

            case guSONGS_COLUMN_ALBUM :
                Track->m_AlbumName = NewData.GetString();
                break;

            case guSONGS_COLUMN_GENRE :
                Track->m_GenreName = NewData.GetString();
                break;

            case guSONGS_COLUMN_COMPOSER :
                Track->m_Composer = NewData.GetString();
                break;

            case guSONGS_COLUMN_DISK :
                Track->m_Disk = NewData.GetString();
                break;

            case guSONGS_COLUMN_YEAR :
                Track->m_Year = NewData.GetLong();
                break;

        }
    }

    m_CdTracksListBox->UpdatedTracks( &Tracks );
}

// -------------------------------------------------------------------------------- //
void guAudioCdPanel::OnSongEditField( wxCommandEvent &event )
{
    int ColumnId = m_CdTracksListBox->GetColumnId( m_CdTracksListBox->GetLastColumnClicked() );
    //guLogMessage( wxT( "guLibPanel::OnSongSetField %i" ), ColumnId );

    guTrackArray Tracks;
    m_CdTracksListBox->GetSelectedSongs( &Tracks );

    wxString Label = m_CdTracksListBox->GetColumnNames()[ ColumnId ];
    wxVariant DefValue = m_CdTracksListBox->GetLastDataClicked();

    wxArrayString Items;

    int Count = Tracks.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        wxVariant Value;
        guTrack * Track = &Tracks[ Index ];

        switch( ColumnId )
        {
            case guSONGS_COLUMN_TITLE :
                Value = Track->m_SongName;
                break;

            case guSONGS_COLUMN_ARTIST :
                Value = Track->m_ArtistName;
                break;

            case guSONGS_COLUMN_ALBUMARTIST :
                Value = Track->m_AlbumArtist;
                break;

            case guSONGS_COLUMN_ALBUM :
                Value = Track->m_AlbumName;
                break;

            case guSONGS_COLUMN_GENRE :
                Value = Track->m_GenreName;
                break;

            case guSONGS_COLUMN_COMPOSER :
                Value = Track->m_Composer;
                break;

            case guSONGS_COLUMN_DISK :
                Value = Track->m_Disk;
                break;

            case guSONGS_COLUMN_YEAR :
                Value = ( long ) Track->m_Year;
                break;
        }
        if( Items.Index( Value.GetString() ) == wxNOT_FOUND )
            Items.Add( Value.GetString() );
    }

    guEditWithOptions * FieldEditor = new guEditWithOptions( this, _( "Field Editor" ), Label, DefValue.GetString(), Items );

    if( FieldEditor )
    {
        if( FieldEditor->ShowModal() == wxID_OK )
        {
            DefValue = FieldEditor->GetData();

            //guLogMessage( wxT( "Setting Data to : %s" ), DefValue.GetString().c_str() );

            // This should be done in a thread for huge selections of tracks...
            int Count = Tracks.Count();
            for( int Index = 0; Index < Count; Index++ )
            {
                guTrack * Track = &Tracks[ Index ];
                switch( ColumnId )
                {
                    case guSONGS_COLUMN_TITLE :
                        Track->m_SongName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_ARTIST :
                        Track->m_ArtistName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_ALBUMARTIST :
                        Track->m_AlbumArtist = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_ALBUM :
                        Track->m_AlbumName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_GENRE :
                        Track->m_GenreName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_COMPOSER :
                        Track->m_Composer = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_DISK :
                        Track->m_Disk = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_YEAR :
                        Track->m_Year = DefValue.GetLong();
                        break;
                }
            }

            m_CdTracksListBox->UpdatedTracks( &Tracks );
        }
        FieldEditor->Destroy();
    }
}


}

// -------------------------------------------------------------------------------- //
