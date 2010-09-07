// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
//	anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2, or (at your option)
//    any later version.
//
//    This Program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; see the file LICENSE.  If not, write to
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "Magnatune.h"

#include "TagInfo.h"
#include "MainFrame.h"
#include "SelCoverFile.h"
#include "StatusBar.h"

#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/zstream.h>
#include <wx/xml/xml.h>

#include <id3v1genres.h>


// -------------------------------------------------------------------------------- //
// guMagnatuneLibrary
// -------------------------------------------------------------------------------- //
guMagnatuneLibrary::guMagnatuneLibrary( const wxString &dbname ) :
    guDbLibrary( dbname )
{
}

// -------------------------------------------------------------------------------- //
guMagnatuneLibrary::~guMagnatuneLibrary()
{
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::UpdateArtistsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ArIndex;
  int           ArCount = labelsets.Count();
  for( ArIndex = 0; ArIndex < ArCount; ArIndex++ )
  {
    wxArrayInt ArLabels = labelsets[ ArIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt Artists;
    Artists.Add( labelsets[ ArIndex ].GetId() );
    SetArtistsLabels( Artists, ArLabels );

  }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::UpdateSongsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ItemIndex;
  int           ItemCount = labelsets.Count();
  for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetSongsLabels( ItemIds, ItemLabels );

  }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::UpdateAlbumsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ItemIndex;
  int           ItemCount = labelsets.Count();
  for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetAlbumsLabels( ItemIds, ItemLabels );
  }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::CreateNewSong( guTrack * track )
{
    wxString query;
    wxSQLite3ResultSet dbRes;

    track->m_SongId = FindTrack( track->m_ArtistName, track->m_AlbumName );
    if( track->m_SongId != wxNOT_FOUND )
    {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', "
                                 "song_genreid = %u, song_genre = '%s', "
                                 "song_artistid = %u, song_artist = '%s', "
                                 "song_albumid = %u, song_album = '%s', "
                                 "song_pathid = %u, song_path = '%s', "
                                 "song_filename = '%s', "
                                 "song_number = %u, song_year = %u, "
                                 "song_length = %u "
                                 "WHERE song_id = %u;" ),
                    escape_query_str( track->m_SongName ).c_str(),
                    track->m_GenreId,
                    escape_query_str( track->m_GenreName ).c_str(),
                    track->m_ArtistId,
                    escape_query_str( track->m_ArtistName ).c_str(),
                    track->m_AlbumId,
                    escape_query_str( track->m_AlbumName ).c_str(),
                    track->m_PathId,
                    escape_query_str( track->m_Path ).c_str(),
                    escape_query_str( track->m_FileName ).c_str(),
                    track->m_Number,
                    track->m_Year,
                    track->m_Length,
                    track->m_SongId );

        ExecuteUpdate( query );
    }
    else
    {
        wxString query = wxString::Format( wxT( "INSERT INTO songs( "
                    "song_id, song_playcount, song_addedtime, "
                    "song_name, song_genreid, song_genre, song_artistid, song_artist, "
                    "song_albumid, song_album, song_pathid, song_path, song_filename, song_format, song_number, song_year, "
                    "song_coverid, song_disk, song_length, song_offset, song_bitrate, song_rating, "
                    "song_filesize ) VALUES( NULL, 0, %u, '%s', %u, '%s', %u, '%s', %u, '%s', "
                    "%u, '%s', '%s', 'mp3,ogg', %u, %u, %u, '%s', %u, 0, 0, -1, 0 )" ),
                    wxDateTime::GetTimeNow(),
                    escape_query_str( track->m_SongName ).c_str(),
                    track->m_GenreId,
                    escape_query_str( track->m_GenreName ).c_str(),
                    track->m_ArtistId,
                    escape_query_str( track->m_ArtistName ).c_str(),
                    track->m_AlbumId,
                    escape_query_str( track->m_AlbumName ).c_str(),
                    track->m_PathId,
                    escape_query_str( track->m_Path ).c_str(),
                    escape_query_str( track->m_FileName ).c_str(),
                    track->m_Number,
                    track->m_Year,
                    track->m_CoverId,
                    escape_query_str( track->m_Disk ).c_str(),
                    track->m_Length );

        //guLogMessage( wxT( "%s" ), query.c_str() );
        ExecuteUpdate( query );

    }

//    guLogMessage( wxT( "%s/%s/%s/%i - %s" ),
//                track->m_GenreName.c_str(),
//                track->m_ArtistName.c_str(),
//                track->m_AlbumName.c_str(),
//                track->m_Number,
//                track->m_SongName.c_str() );

}




// -------------------------------------------------------------------------------- //
// guMagnatunePanel
// -------------------------------------------------------------------------------- //
guMagnatunePanel::guMagnatunePanel( wxWindow * parent, guMagnatuneLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix ) :
    guLibPanel( parent, db, playerpanel, prefix )
{
    SetBaseCommand( ID_MENU_VIEW_MAGNATUNE );

    m_ContextMenuFlags = ( guLIBRARY_CONTEXTMENU_DOWNLOAD_COVERS | guLIBRARY_CONTEXTMENU_LINKS );

    m_DownloadThread = NULL;
    m_UpdateThread = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this ); // Get notified when configuration changes

    Connect( ID_MAGNATUNE_EDIT_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnEditSetup ), NULL, this );
    Connect( ID_MAGNATUNE_SETUP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnEditSetup ), NULL, this );
    Connect( ID_MAGNATUNE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnUpdate ), NULL, this );
    Connect( ID_MAGNATUNE_UPGRADE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnUpgrade ), NULL, this );
    Connect( ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnDownloadAlbum ), NULL, this );
    Connect( ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnDownloadTrackAlbum ), NULL, this );

    Connect( ID_MAGNATUNE_COVER_DOWNLAODED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnCoverDownloaded ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guMagnatunePanel::OnConfigUpdated ), NULL, this );

    wxArrayInt AllowedGenres = Config->ReadANum( wxT( "Genre" ), 0, wxT( "MagnatuneGenres" ) );
    if( AllowedGenres.IsEmpty() )
    {
        wxCommandEvent event;
        OnEditSetup( event );
    }
}

// -------------------------------------------------------------------------------- //
guMagnatunePanel::~guMagnatunePanel()
{
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count;
    if( tracks && ( Count = tracks->Count() ) )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        int AudioFormat = Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Magnatune" ) );
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = &( * tracks )[ Index ];
            guLogMessage( wxT( "'%s'" ), Track->m_FileName.c_str() );
            Track->m_FileName = Track->m_FileName.Mid( Track->m_FileName.Find( wxT( "http://he3" ) ) );
            Track->m_FileName += AudioFormat ? guMAGNATUNE_STREAM_FORMAT_OGG : guMAGNATUNE_STREAM_FORMAT_MP3;
            Track->m_Type = guTRACK_TYPE_MAGNATUNE;
            if( isdrag )
                Track->m_FileName.Replace( wxT( "http://" ), wxT( "/" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu *     SubMenu;
    SubMenu = new wxMenu();

    if( ( windowid == guLIBRARY_ELEMENT_ALBUMS ) )
    {
        wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
        SubMenu->Append( MenuItem );

        SubMenu->AppendSeparator();
    }
    else if( ( windowid == guLIBRARY_ELEMENT_TRACKS ) )
    {
        wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
        SubMenu->Append( MenuItem );

        SubMenu->AppendSeparator();
    }

    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_UPDATE, _( "Update Database" ), _( "Download the latest Magnatune database" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_EDIT_GENRES, _( "Select Genres" ), _( "Selects the enabled Magnatune genres" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_SETUP, _( "Preferences" ), _( "Configure the Magnatune options" ) );
    SubMenu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( SubMenu, _( "Magnatune" ), _( "Global Magnatune options" ) );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnEditSetup( wxCommandEvent &event )
{
    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
    //CmdEvent.SetInt( guPREFERENCE_PAGE_MAGNATUNE );
    wxPostEvent( wxTheApp->GetTopWindow(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::StartUpdateTracks( const int action )
{
    wxMutexLocker Lock( m_UpdateThreadMutex );

    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    guStatusBar * StatusBar = ( guStatusBar * ) MainFrame->GetStatusBar();
    int GaugeId = StatusBar->AddGauge( _( "Magnatune" ) );
    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guMagnatuneUpdateThread( ( guMagnatuneLibrary * ) m_Db,
                                action, GaugeId );
    if( !m_UpdateThread )
    {
        guLogError( wxT( "Could not create the Magnatune update thread" ) );
        StatusBar->RemoveGauge( GaugeId );
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::EndUpdateThread( void )
{
    wxMutexLocker Lock( m_UpdateThreadMutex );
    m_UpdateThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnUpdate( wxCommandEvent &event )
{
    StartUpdateTracks( guMAGNATUNE_ACTION_UPDATE );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnUpgrade( wxCommandEvent &event )
{
    StartUpdateTracks( guMAGNATUNE_ACTION_UPGRADE );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnConfigUpdated( wxCommandEvent &event )
{
//    if( event.GetInt() & guPREFERENCE_PAGE_FLAG_MAGNATUNE )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        bool DoUpgrade = Config->ReadBool( wxT( "NeedUpgrade" ), false, wxT( "Magnatune" ) );

        if( DoUpgrade )
        {
            OnUpgrade( event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::AddDownload( const int albumid, const bool iscover )
{
    wxMutexLocker Lock( m_DownloadThreadMutex );

    if( !m_DownloadThread )
    {
        m_DownloadThread = new guMagnatuneDownloadThread( this );

        if( !m_DownloadThread )
        {
            guLogMessage( wxT( "Could not create the magnatune download thread" ) );
            return;
        }
    }

    m_DownloadThread->AddAlbum( albumid, iscover );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::AddDownloads( wxArrayInt &albumids, const bool iscover )
{
    wxMutexLocker Lock( m_DownloadThreadMutex );

    if( !m_DownloadThread )
    {
        m_DownloadThread = new guMagnatuneDownloadThread( this );

        if( !m_DownloadThread )
        {
            guLogMessage( wxT( "Could not create the magnatune download thread" ) );
            return;
        }
    }

    m_DownloadThread->AddAlbums( albumids, iscover );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::EndDownloadThread( void )
{
    wxMutexLocker Lock( m_DownloadThreadMutex );
    m_DownloadThread = NULL;
}

// -------------------------------------------------------------------------------- //
wxImage * guMagnatunePanel::GetAlbumCover( const int albumid, wxString &coverpath )
{
    wxString CoverFile = wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/Covers/" );
    CoverFile += wxString::Format( wxT( "%u.jpg" ), albumid );
    if( wxFileExists( CoverFile ) )
    {
        wxImage * CoverImage = new wxImage( CoverFile, wxBITMAP_TYPE_JPEG );
        if( CoverImage )
        {
            if( CoverImage->IsOk() )
            {
                coverpath = CoverFile;
                return CoverImage;
            }
            delete CoverImage;
        }
    }
    AddDownload( albumid );
    return NULL;
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnCoverDownloaded( wxCommandEvent &event )
{
    int AlbumId =  event.GetInt();
    if( AlbumId )
    {
        ReloadAlbums( false );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnAlbumDownloadCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        AddDownloads( Albums );
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnAlbumSelectCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        int AlbumId = Albums[ 0 ];
        guSelCoverFile * SelCoverFile = new guSelCoverFile( this, m_Db, AlbumId );
        if( SelCoverFile )
        {
            if( SelCoverFile->ShowModal() == wxID_OK )
            {
//                wxString CoverFile = SelCoverFile->GetSelFile();
//                if( !CoverFile.IsEmpty() )
//                {
//                    guConfig * Config = ( guConfig * ) guConfig::Get();
//                    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
//                    wxString CoverName = wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/Covers/" );
//                    CoverName += wxString::Format( wxT( "%u.jpg" ), AlbumId );
//
//                    wxURI Uri( CoverFile );
//                    if( Uri.IsReference() )
//                    {
//                        wxImage CoverImage( CoverFile );
//                        if( CoverImage.IsOk() )
//                        {
//                            if( ( CoverFile == CoverName ) || CoverImage.SaveFile( CoverName, wxBITMAP_TYPE_JPEG ) )
//                            {
//                                m_Db->SetAlbumCover( AlbumId, CoverName );
//                                ReloadAlbums( false );
//                            }
//                        }
//                        else
//                        {
//                            guLogError( wxT( "Could not load the imate '%s'" ), CoverFile.c_str() );
//                        }
//                    }
//                    else
//                    {
//                        if( DownloadImage( CoverFile, CoverName ) )
//                        {
//                            m_Db->SetAlbumCover( AlbumId, CoverName );
//                            ReloadAlbums( false );
//                        }
//                        else
//                        {
//                            guLogError( wxT( "Failed to download file '%s'" ), CoverFile.c_str() );
//                        }
//                    }
//                }
            }
            delete SelCoverFile;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnDownloadAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadAlbum" ) );

//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    wxString TorrentCmd = Config->ReadStr( wxT( "TorrentCommand" ), wxEmptyString, wxT( "Magnatune" ) );
//    if( TorrentCmd.IsEmpty() )
//    {
//        OnEditSetup( event );
//        return;
//    }
//
//    int Index;
//    int Count;
//    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
//    if( ( Count = Albums.Count() ) )
//    {
//        if( event.GetId() == ID_MAGNATUNE_DOWNLOAD_TORRENT_ALBUM )
//        {
//            AddDownloads( Albums, false );
//        }
//        else
//        {
//            for( Index = 0; Index < Count; Index++ )
//            {
//                guWebExecute( wxString::Format( guMAGNATUNE_DOWNLOAD_DIRECT, Albums[ Index ] ) );
//            }
//        }
//    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadTrackAlbum" ) );

//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    wxString TorrentCmd = Config->ReadStr( wxT( "TorrentCommand" ), wxEmptyString, wxT( "Magnatune" ) );
//    if( TorrentCmd.IsEmpty() )
//    {
//        OnEditSetup( event );
//        return;
//    }
//
//    guTrackArray Tracks;
//    m_SongListCtrl->GetSelectedSongs( &Tracks );
//
//    wxArrayInt Albums;
//    int Index;
//    int Count = Tracks.Count();
//    for( Index = 0; Index < Count; Index++ )
//    {
//        if( Albums.Index( Tracks[ Index ].m_AlbumId ) == wxNOT_FOUND )
//            Albums.Add( Tracks[ Index ].m_AlbumId );
//    }
//    if( ( Count = Albums.Count() ) )
//    {
//        if( event.GetId() == ID_MAGNATUNE_DOWNLOAD_TORRENT_TRACK_ALBUM )
//        {
//            AddDownloads( Albums, false );
//        }
//        else
//        {
//            for( Index = 0; Index < Count; Index++ )
//            {
//                guWebExecute( wxString::Format( guMAGNATUNE_DOWNLOAD_DIRECT, Albums[ Index ] ) );
//            }
//        }
//    }
}

// -------------------------------------------------------------------------------- //
// guMagnatuneDownloadThread
// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::guMagnatuneDownloadThread( guMagnatunePanel * magnatunepanel )
{
    m_MagnatunePanel = magnatunepanel;
    m_Db = magnatunepanel->GetMagnatuneDb();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::~guMagnatuneDownloadThread()
{
    if( !TestDestroy() )
    {
        m_MagnatunePanel->EndDownloadThread();
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneDownloadThread::AddAlbum( const int albumid, const bool iscover )
{
    if( iscover )
    {
        m_CoversMutex.Lock();
        m_Covers.Add( albumid );
        m_CoversMutex.Unlock();
    }
    else
    {
        m_AlbumsMutex.Lock();
        m_Albums.Add( albumid );
        m_AlbumsMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneDownloadThread::AddAlbums( wxArrayInt &albumids, const bool iscover )
{
    int Index;
    int Count = albumids.Count();

    if( iscover )
    {
        m_CoversMutex.Lock();
        for( Index = 0; Index < Count; Index++ )
        {
            m_Covers.Add( albumids[ Index ] );
        }
        m_CoversMutex.Unlock();
    }
    else
    {
        m_AlbumsMutex.Lock();
        for( Index = 0; Index < Count; Index++ )
        {
            m_Albums.Add( albumids[ Index ] );
        }
        m_AlbumsMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::ExitCode guMagnatuneDownloadThread::Entry()
{
//    int Count;
//    int LoopCount = 0;
//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    int AudioFormat = Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Magnatune" ) );
//    wxString TorrentCmd = Config->ReadStr( wxT( "TorrentCommand" ), wxEmptyString, wxT( "Magnatune" ) );
//    while( !TestDestroy() )
//    {
//        m_CoversMutex.Lock();
//        Count = m_Covers.Count();
//        m_CoversMutex.Unlock();
//
//        size_t CurTime = wxGetLocalTimeMillis().GetLo();
//        if( Count )
//        {
//            LoopCount = 0;
//            wxString CoverFile = wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/Covers/" );
//            CoverFile += wxString::Format( wxT( "%u.jpg" ), m_Covers[ 0 ] );
//
//            if( !wxFileExists( CoverFile ) )
//            {
//                if( !wxDirExists( wxPathOnly( CoverFile ) + wxT( "/" ) ) )
//                {
//                    wxMkdir( wxPathOnly( CoverFile ) + wxT( "/" ), 0770 );
//                }
//                wxString CoverUrl = wxString::Format( guMAGNATUNE_COVER_DOWNLOAD_URL, m_Covers[ 0 ], 300 );
//                DownloadImage( CoverUrl, CoverFile, 300 );
//            }
//
//            if( wxFileExists( CoverFile ) )
//            {
//                int CoverId = m_Db->AddCoverFile( CoverFile );
//
//                wxString query = wxString::Format( wxT( "UPDATE songs SET song_coverid = %u WHERE song_albumid = %u" ),
//                                    CoverId, m_Covers[ 0 ] );
//
//                m_Db->ExecuteUpdate( query );
//
//                // Notify the panel that the cover is downloaded
//                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAGNATUNE_COVER_DOWNLAODED );
//                event.SetInt( m_Covers[ 0 ] );
//                wxPostEvent( m_MagnatunePanel, event );
//            }
//            else
//            {
//                guLogMessage( wxT( "Could not get the magnatune cover art %s" ), CoverFile.c_str() );
//            }
//
//            m_CoversMutex.Lock();
//            m_Covers.RemoveAt( 0 );
//            m_CoversMutex.Unlock();
//        }
//        else
//        {
//            LoopCount++;
//            if( LoopCount > 8 )
//            {
//                break;
//            }
//        }
//
//        if( TestDestroy() )
//            break;
//
//        size_t Elapsed = wxGetLocalTimeMillis().GetLo() - CurTime;
//        if( !( Elapsed > 1000 ) )
//        {
//            Sleep( 1000 - Elapsed );
//        }
//
//        //
//        // Album Torrents
//        //
//        m_AlbumsMutex.Lock();
//        Count = m_Albums.Count();
//        m_AlbumsMutex.Unlock();
//
//        CurTime = wxGetLocalTimeMillis().GetLo();
//        if( Count )
//        {
//            LoopCount = 0;
//
//            wxString Url = wxString::Format( guMAGNATUNE_TORRENT_DOWNLOAD_URL, m_Albums[ 0 ] );
//            Url += AudioFormat ? guMAGNATUNE_DOWNLOAD_FORMAT_OGG : guMAGNATUNE_DOWNLOAD_FORMAT_MP3;
//
//            //guLogMessage( wxT( "Getting %s" ), Url.c_str() );
//
//            wxString TorrentUrl = GetUrlContent( Url );
//
//            //guLogMessage( wxT( "Downloading '%s'" ), TorrentUrl.c_str() );
//            if( !TorrentUrl.IsEmpty() )
//            {
//                wxString TmpFileName = wxFileName::CreateTempFileName( wxString::Format( wxT( "%u" ), m_Albums[ 0 ] ) );
//                TmpFileName += wxT( ".torrent" );
//                if( DownloadFile( TorrentUrl, TmpFileName ) )
//                {
//                    guExecute( TorrentCmd + wxT( " " ) + TmpFileName );
//                }
//            }
//
//            m_AlbumsMutex.Lock();
//            m_Albums.RemoveAt( 0 );
//            m_AlbumsMutex.Unlock();
//        }
//        else
//        {
//            LoopCount++;
//            if( LoopCount > 8 )
//            {
//                break;
//            }
//        }
//
//        if( TestDestroy() )
//            break;
//
//        Elapsed = wxGetLocalTimeMillis().GetLo() - CurTime;
//        if( !( Elapsed > 1000 ) )
//        {
//            Sleep( 1000 - Elapsed );
//        }
//    }
    return 0;
}

// -------------------------------------------------------------------------------- //
// guMagnatuneUpdateThread
// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::guMagnatuneUpdateThread( guMagnatuneLibrary * db, int action, int gaugeid )
{
    m_Db = db;
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    m_Action = action;
    m_GaugeId = gaugeid;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    //m_LastUpdate = Config->ReadNum( wxT( "MagnatuneLastUpdate" ), 0, wxT( "General" ) );
    m_AllowedGenres = Config->ReadANum( wxT( "Genre" ), 0, wxT( "MagnatuneGenres" ) );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::~guMagnatuneUpdateThread()
{
    //
    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( MainFrame, event );

    if( !TestDestroy() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxDateTime Now = wxDateTime::Now();
            Config->WriteNum( wxT( "MagnatuneLastUpdate" ), Now.GetTicks(), wxT( "General" ) );
            Config->Flush();
        }

        event.SetId( ID_MAGNATUNE_UPDATE_FINISHED );
        event.SetEventObject( ( wxObject * ) this );
        wxPostEvent( MainFrame, event );

//        m_MainFrame->GetMagnatunePanel()->EndUpdateThread();
    }
}

#if 0
<AllAlbums>
    <Album>
        <artist>Dr Kuch</artist>
		<artistdesc>Fun electro-poppy dance up and down tempo chill out</artistdesc>
		<cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
        <artistphoto>http://magnatune.com//artists/img/drkuch_cover_small.jpg</artistphoto>
		<albumname>We Can&#39;t Stop Progress</albumname>
		<year>2009</year>
		<album_notes></album_notes>
		<mp3genre>(52)</mp3genre>
		<home>http://magnatune.com/artists/dr_kuch</home>
		<buy>https://magnatune.com/artists/buy_album?artist=Dr+Kuch&amp;album=We+Can%27t+Stop+Progress&amp;genre=Electronica</buy>
		<magnatunegenres>Electronica,Euro-Techno</magnatunegenres>
		<launchdate>2009-10-08</launchdate>
		<albumsku>drkuch-progress</albumsku>
        <Track>
            <artist>Dr Kuch</artist>
            <artistphoto>http://magnatune.com//artists/img/drkuch_cover_small.jpg</artistphoto>
            <artistdesc>Fun electro-poppy dance up and down tempo chill out</artistdesc>
            <albumname>We Can&#39;t Stop Progress</albumname>
            <trackname>Intro</trackname>
            <tracknum>1</tracknum>
            <year>2009</year>
            <mp3genre>(52)</mp3genre>
            <magnatunegenres>Electronica,Euro-Techno</magnatunegenres>
            <license>http://creativecommons.org/licenses/by-nc-sa/1.0/</license>
            <seconds>18</seconds>
            <url>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch.mp3</url>
            <mp3lofi>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch-lofi.mp3</mp3lofi>
            <oggurl>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch.ogg</oggurl>
            <buy>https://magnatune.com/artists/buy_album?artist=Dr+Kuch&amp;album=We+Can%27t+Stop+Progress&amp;genre=Electronica</buy>
            <home>http://magnatune.com/artists/dr_kuch</home>
            <launchdate>2009-10-08</launchdate>
            <cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
            <albumsku>drkuch-progress</albumsku>
        </Track>
        ...
    </Album>
    ...
</AllAlbums>
#endif


// -------------------------------------------------------------------------------- //
void ReadMagnatuneXmlTrack( wxXmlNode * xmlnode, guMagnatuneUpdateThread * thread, guTrack * track, guMagnatuneLibrary * db )
{
    long Id;
    while( xmlnode && !thread->TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "trackname" ) )
        {
            track->m_SongName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
        }
        else if( ItemName == wxT( "tracknum" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_Number = Id;
        }
        else if( ItemName == wxT( "year" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_Year = Id;
        }
        else if( ItemName == wxT( "seconds" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_Length = Id;
        }
        else if( ItemName == wxT( "url" ) )
        {
            track->m_FileName = xmlnode->GetNodeContent();
            track->m_FileName.Replace( wxT( ".mp3" ), wxT( "" ) );
            track->m_FileName.Replace( wxT( ".ogg" ), wxT( "" ) );
        }
        else if( ItemName == wxT( "magnatunegenres" ) )
        {
            track->m_GenreName = xmlnode->GetNodeContent();
            track->m_GenreId = db->GetGenreId( track->m_GenreName );
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadMagnatuneXmlAlbum( wxXmlNode * xmlnode, guMagnatuneUpdateThread * thread, guTrack * track, guMagnatuneLibrary * db )
{
    long Id;
    while( xmlnode && !thread->TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "artist" ) )
        {
            track->m_ArtistName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
            track->m_ArtistId = db->GetArtistId( track->m_ArtistName );
        }
        else if( ItemName == wxT( "albumname" ) )
        {
            track->m_AlbumName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
            track->m_AlbumId = 0;
            guLogMessage( wxT( "%s" ), track->m_AlbumName.c_str() );
        }
        else if( ItemName == wxT( "year" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_Year = Id;
        }
        else if( ItemName == wxT( "magnatunegenres" ) )
        {
            track->m_GenreName = xmlnode->GetNodeContent();
            track->m_GenreId = db->GetGenreId( track->m_GenreName );
        }
        else if( ItemName == wxT( "Track" ) )
        {
            if( !track->m_AlbumId )
            {
                track->m_Path = track->m_GenreName + wxT( "/" ) +
                                track->m_ArtistName + wxT( "/" ) +
                                track->m_AlbumName + wxT( "/" );
                track->m_PathId = db->GetPathId( track->m_Path );
                track->m_AlbumId = db->GetAlbumId( track->m_AlbumName, track->m_ArtistId, track->m_PathId, track->m_Path );
            }
            ReadMagnatuneXmlTrack( xmlnode->GetChildren(), thread, track, db );

            db->CreateNewSong( track );
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guMagnatuneUpdateThread::UpdateDatabase( void )
{
    if( DownloadFile( guMAGNATUNE_DATABASE_DUMP_URL, wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info_xml.gz" ) ) )
    {
        wxFileInputStream Ins( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info_xml.gz" ) );
        if( Ins.IsOk() )
        {
            wxZlibInputStream ZIn( Ins );
            if( ZIn.IsOk() )
            {
                wxFileOutputStream ZOuts( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info.xml" ) );
                if( ZOuts.IsOk() )
                {
                    ZIn.Read( ZOuts );
                    return true;
                }
            }
        }
        else
        {
            guLogError( wxT( "Could not open the Magnatune local database copy" ) );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::ExitCode guMagnatuneUpdateThread::Entry()
{
    wxString query;

    wxCommandEvent evtup( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    if( m_Action == guMAGNATUNE_ACTION_UPGRADE &&
        !wxFileExists( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info.xml" ) ) )
    {
        m_Action = guMAGNATUNE_ACTION_UPDATE;
    }

    guLogMessage( wxT( "Starting the Magnatune Update process..." ) );
    if( !TestDestroy() && ( m_Action == guMAGNATUNE_ACTION_UPGRADE || UpdateDatabase() ) )
    {
        wxFile XmlFile( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info.xml" ), wxFile::read );
        if( XmlFile.IsOpened() )
        {
            evtmax.SetExtraLong( XmlFile.Length() );
            wxPostEvent( wxTheApp->GetTopWindow(), evtmax );

            wxFileOffset CurPos;
            query = wxT( "BEGIN TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            //query = wxT( "DELETE FROM songs" );
            //m_Db->ExecuteUpdate( query );

            wxString AlbumChunk = guGetNextXMLChunk( XmlFile, CurPos, "<Album>", "</Album>" );
            while( !TestDestroy() && !AlbumChunk.IsEmpty() )
            {
                wxHtmlEntitiesParser EntitiesParser;
                AlbumChunk = EntitiesParser.Parse( AlbumChunk );
                AlbumChunk.Replace( wxT( "&" ), wxT( "&amp;" ) );

                wxStringInputStream Ins( AlbumChunk );
                wxXmlDocument XmlDoc( Ins );
                if( XmlDoc.IsOk() )
                {
                    wxXmlNode * XmlNode = XmlDoc.GetRoot();

                    if( XmlNode && XmlNode->GetName() == wxT( "Album" ) )
                    {
                        ReadMagnatuneXmlAlbum( XmlNode->GetChildren(), this, &m_CurrentTrack, m_Db );
                    }
                }
                else
                {
                    guLogMessage( wxT( "Error in album chunk:\n%s" ), AlbumChunk.c_str() );
                }

                AlbumChunk = guGetNextXMLChunk( XmlFile, CurPos, "<Album>", "</Album>" );
                evtup.SetExtraLong( CurPos );
                wxPostEvent( wxTheApp->GetTopWindow(), evtup );
            }

            query = wxT( "END TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            XmlFile.Close();
        }
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
