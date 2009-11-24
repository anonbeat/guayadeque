// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
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
#include "SoListBox.h"

#include "Config.h" // Configuration
#include "Commands.h"
#include "Images.h"
#include "MainApp.h"
#include "OnlineLinks.h"
#include "PlayList.h" // LenToString
#include "Utils.h"
#include "RatingCtrl.h"

#include <wx/imaglist.h>

wxString guSONGS_COLUMN_NAMES[ guSONGS_COLUMN_ADDEDDATE + 1 ] = {
    wxT( "#" ),
    _( "Title" ),
    _( "Artist" ),
    _( "Album" ),
    _( "Genre" ),
    _( "Length" ),
    _( "Year" ),
    _( "BitRate" ),
    _( "Rating" ),
    _( "PlayCount" ),
    _( "Last Play" ),
    _( "Added Date" )
};

// -------------------------------------------------------------------------------- //
guSoListBox::guSoListBox( wxWindow * parent, DbLibrary * NewDb, wxString confname, long style ) :
             guListView( parent, style|wxLB_MULTIPLE, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_Db = NewDb;
    m_ConfName = confname;

    int ColOrder = Config->ReadNum( wxT( "TracksOrder" ), 0, wxT( "General" ) );
    bool ColOrderDesc = Config->ReadBool( wxT( "TracksOrderDesc" ), 0, wxT( "General" ) );

    int ColId;
    wxString ColName;
    int index;
    int count = sizeof( guSONGS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( m_ConfName + wxString::Format( wxT( "Col%u" ), index ), index, m_ConfName + wxT( "Columns" ) );

        ColName = guSONGS_COLUMN_NAMES[ ColId ];

        if( style & guLISTVIEW_COLUMN_SORTING )
            ColName += ( ( ColId == ColOrder ) ? ( ColOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( m_ConfName + wxString::Format( wxT( "ColWidth%u" ), index ), 80, m_ConfName + wxT( "Columns" ) ),
            Config->ReadBool( m_ConfName + wxString::Format( wxT( "ColShow%u" ), index ), true, m_ConfName + wxT( "Columns" ) )
            );
        InsertColumn( Column );
    }

    m_GreyStar   = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_grey_star_tiny + GURATING_STYLE_MID ) ) );
    m_YellowStar = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_yellow_star_tiny + GURATING_STYLE_MID ) ) );

    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnSearchLinkClicked ) );
    Connect( ID_SONGS_COMMANDS, ID_SONGS_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnCommandClicked ) );
    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guSoListBox::~guSoListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    int ColId;
    int index;
    int count = sizeof( guSONGS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( m_ConfName + wxString::Format( wxT( "Col%u" ), index ),
                          ( * m_Columns )[ index ].m_Id,
                          m_ConfName + wxT( "Columns" ) );
        Config->WriteNum( m_ConfName + wxString::Format( wxT( "ColWidth%u" ), index ),
                          ( * m_Columns )[ index ].m_Width,
                          m_ConfName + wxT( "Columns" ) );
        Config->WriteBool( m_ConfName + wxString::Format( wxT( "ColShow%u" ), index ),
                           ( * m_Columns )[ index ].m_Enabled,
                           m_ConfName + wxT( "Columns" ) );
    }

    if( m_GreyStar )
        delete m_GreyStar;
    if( m_YellowStar )
        delete m_YellowStar;

    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnSearchLinkClicked ) );
    Disconnect( ID_SONGS_COMMANDS, ID_SONGS_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnCommandClicked ) );
}

// -------------------------------------------------------------------------------- //
wxString guSoListBox::OnGetItemText( const int row, const int col ) const
{
    guTrack * Song;

    Song = &m_Items[ row ];

    switch( ( * m_Columns )[ col ].m_Id )
    {
        case guSONGS_COLUMN_NUMBER :
          return wxString::Format( wxT( "%02u" ), Song->m_Number );

        case guSONGS_COLUMN_TITLE  :
          return Song->m_SongName;

        case guSONGS_COLUMN_ARTIST :
          return Song->m_ArtistName;

        case guSONGS_COLUMN_ALBUM :
          return Song->m_AlbumName;

        case guSONGS_COLUMN_GENRE :
          return Song->m_GenreName;

        case guSONGS_COLUMN_LENGTH :
          return LenToString( Song->m_Length );

        case guSONGS_COLUMN_YEAR :
            if( Song->m_Year )
                return wxString::Format( wxT( "%u" ), Song->m_Year );
            else
                return wxEmptyString;

        case guSONGS_COLUMN_BITRATE :
            return wxString::Format( wxT( "%u" ), Song->m_Bitrate );

        case guSONGS_COLUMN_RATING :
            return wxString::Format( wxT( "%i" ), Song->m_Rating );

        case guSONGS_COLUMN_PLAYCOUNT :
            return wxString::Format( wxT( "%u" ), Song->m_PlayCount );

        case guSONGS_COLUMN_LASTPLAY :
        {
            if( Song->m_LastPlay )
            {
                wxDateTime LastPlay;
                LastPlay.Set( ( time_t ) Song->m_LastPlay );
                return LastPlay.FormatDate();
            }
            else
                return _( "Never" );
        }

        case guSONGS_COLUMN_ADDEDDATE :
        {
            wxDateTime AddedDate;
            AddedDate.Set( ( time_t ) Song->m_AddedTime );
            return AddedDate.FormatDate();
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guSoListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( ( * m_Columns )[ col ].m_Id == guSONGS_COLUMN_RATING )
    {
        dc.SetBackgroundMode( wxTRANSPARENT );
        int x;
        int w = ( ( GURATING_STYLE_MID * 2 ) + GURATING_IMAGE_SIZE );

        for( x = 0; x < 5; x++ )
        {
           dc.DrawBitmap( ( x >= m_Items[ row ].m_Rating ) ? * m_GreyStar : * m_YellowStar,
                          rect.x + 3 + ( w * x ), rect.y + 6, true );
        }
    }
    else
    {
        guListView::DrawItem( dc, rect, row, col );
    }
}


// -------------------------------------------------------------------------------- //
void guSoListBox::GetItemsList( void )
{
    m_Db->GetSongs( &m_Items );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::ReloadItems( bool reset )
{
    //
    wxArrayInt Selection;
    int FirstVisible = GetFirstVisibleLine();

    if( reset )
        SetSelection( -1 );
    else
        Selection = GetSelectedItems( false );

    m_Items.Empty();

    GetItemsList();

    SetItemCount( m_Items.Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToLine( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
int guSoListBox::GetSelectedSongs( guTrackArray * tracks ) const
{
    unsigned long cookie;
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        tracks->Add( new guTrack( m_Items[ item ] ) );
        item = GetNextSelected( cookie );
    }
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
void guSoListBox::GetAllSongs( guTrackArray * tracks ) const
{
    int index;
    int count = m_Items.Count();
    for( index = 0; index < count; index++ )
    {
        tracks->Add( new guTrack( m_Items[ index ] ) );
    }
}

// -------------------------------------------------------------------------------- //
void AddSongsCommands( wxMenu * Menu, int SelCount )
{
    wxMenu * SubMenu;
    int index;
    int count;
    wxMenuItem * MenuItem;
    if( Menu )
    {
        SubMenu = new wxMenu();
        wxASSERT( SubMenu );

        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
        wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "Commands" ) );
        if( ( count = Commands.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                if( ( ( Commands[ index ].Find( wxT( "{bp}" ) ) != wxNOT_FOUND ) ||
                      ( Commands[ index ].Find( wxT( "{bc}" ) ) != wxNOT_FOUND ) )
                    && ( SelCount != 1 ) )
                {
                    continue;
                }
                MenuItem = new wxMenuItem( Menu, ID_SONGS_COMMANDS + index, Names[ index ], Commands[ index ] );
                SubMenu->Append( MenuItem );
            }
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, -1, _( "No commands defined" ), _( "Add commands in preferences" ) );
            SubMenu->Append( MenuItem );
        }
        Menu->AppendSubMenu( SubMenu, _( "Commands" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guSoListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedItems().Count();

    MenuItem = new wxMenuItem( Menu, ID_SONG_PLAY, _( "Play" ), _( "Play current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_SONG_PLAYALL, _( "Play All" ), _( "Play all songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_SONG_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_SONG_ENQUEUEALL, _( "Enqueue All" ), _( "Add all songs to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_SONG_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_SONG_EDITTRACKS, _( "Edit Songs" ), _( "Edit the songs selected" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_SONG_SAVEPLAYLIST, _( "Save Playlist" ), _( "Save all selected tracks as a playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_SONG_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_SONG_BROWSE_GENRE, _( "Select Genre" ), _( "Selects the genre of the current song" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_search ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_SONG_BROWSE_ARTIST, _( "Select Artist" ), _( "Selects the artist of the current song" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_search ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_SONG_BROWSE_ALBUM, _( "Select Album" ), _( "Select the album of the current song" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_search ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        if( SelCount == 1 )
        {
            AddOnlineLinksMenu( Menu );
        }

        AddSongsCommands( Menu, SelCount );
    }
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    unsigned long cookie;
    int Item = GetFirstSelected( cookie );
    if( Item != wxNOT_FOUND )
    {
        int index = event.GetId();

        guConfig * Config = ( guConfig * ) Config->Get();
        if( Config )
        {
            wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );
            wxASSERT( Links.Count() > 0 );

            index -= ID_LASTFM_SEARCH_LINK;
            wxString SearchLink = Links[ index ];
            wxString Lang = Config->ReadStr( wxT( "Language" ), wxT( "en" ), wxT( "LastFM" ) );
            if( Lang.IsEmpty() )
            {
                Lang = ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().Mid( 0, 2 );
                //guLogMessage( wxT( "Locale: %s" ), ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().c_str() );
            }
            SearchLink.Replace( wxT( "{lang}" ), Lang );
            SearchLink.Replace( wxT( "{text}" ), guURLEncode( GetSearchText( Item ) ) );
            guWebExecute( SearchLink );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnCommandClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        index = event.GetId();

        guConfig * Config = ( guConfig * ) Config->Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
            wxASSERT( Commands.Count() > 0 );

            index -= ID_SONGS_COMMANDS;
            wxString CurCmd = Commands[ index ];

            if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                GetSelectedSongs( &Songs );
                wxArrayInt AlbumList;
                AlbumList.Add( Songs[ 0 ].m_AlbumId );
                wxArrayString AlbumPaths = m_Db->GetAlbumsPaths( AlbumList );
                wxString Paths = wxEmptyString;
                count = AlbumPaths.Count();
                for( index = 0; index < count; index++ )
                {
                    Paths += wxT( " \"" ) + AlbumPaths[ index ] + wxT( "\"" );
                }
                CurCmd.Replace( wxT( "{bp}" ), Paths.Trim( false ) );
            }

            if( CurCmd.Find( wxT( "{bc}" ) ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                GetSelectedSongs( &Songs );
                wxArrayInt AlbumList;
                AlbumList.Add( Songs[ 0 ].m_AlbumId );
                int CoverId = m_Db->GetAlbumCoverId( AlbumList[ 0 ] );
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                {
                    CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
                }
                CurCmd.Replace( wxT( "{bc}" ), CoverPath );
            }

            if( CurCmd.Find( wxT( "{tp}" ) ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                GetSelectedSongs( &Songs );
                wxString SongList = wxEmptyString;
                count = Songs.Count();
                if( count )
                {
                    for( index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + Songs[ index ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( wxT( "{tp}" ), SongList.Trim( false ) );
                }
            }

            //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
            guExecute( CurCmd );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guSoListBox::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Items[ item ].m_ArtistName.c_str(),
        m_Items[ item ].m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
wxString guSoListBox::GetItemName( const int row ) const
{
    return m_Items[ row ].m_SongName;
}

// -------------------------------------------------------------------------------- //
int guSoListBox::GetItemId( const int row ) const
{
    return m_Items[ row ].m_SongId;
}

// -------------------------------------------------------------------------------- //
