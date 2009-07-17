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

wxString guSONGS_COLUMN_NAMES[] = {
    wxT( "#" ),
    _( "Title" ),
    _( "Artist" ),
    _( "Album" ),
    _( "Length" ),
    _( "Year" ),
    _( "BitRate" ),
    _( "Rating" ),
    _( "PlayCount" ),
    _( "Last Play" ),
    _( "Added Date" )
};

// -------------------------------------------------------------------------------- //
guSoListBox::guSoListBox( wxWindow * parent, DbLibrary * NewDb, wxString confname ) :
             wxListCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL|wxSUNKEN_BORDER )
{
    wxListItem ListItem;
    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_Db = NewDb;
    m_ConfName = confname;

    m_Columns = Config->ReadANum( confname + wxT( "Col" ), 0, confname + wxT( "s" ) );
    if( !m_Columns.Count() )
    {
        m_Columns.Add( guSONGS_COLUMN_NUMBER );
        m_Columns.Add( guSONGS_COLUMN_TITLE );
        m_Columns.Add( guSONGS_COLUMN_ARTIST );
        m_Columns.Add( guSONGS_COLUMN_ALBUM );
        m_Columns.Add( guSONGS_COLUMN_LENGTH );
    }

    int index;
    int count = m_Columns.Count();
    ListItem.SetImage( wxNOT_FOUND );
    for( index = 0; index < count; index++ )
    {
        ListItem.SetText( guSONGS_COLUMN_NAMES[ m_Columns[ index ] ] );
        ListItem.SetWidth( Config->ReadNum( confname + wxString::Format( wxT( "ColSize%u" ), index ), 40, wxT( "Positions" ) ) );
        InsertColumn( index, ListItem );
    }

    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guSoListBox::OnBeginDrag ), NULL, this );
    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guSoListBox::OnContextMenu ), NULL, this );
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnSearchLinkClicked ) );
    Connect( ID_SONGS_COMMANDS, ID_SONGS_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnCommandClicked ) );


    wxColour ListBoxColor = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );
    wxColour ListBoxText;
    ListBoxText.Set( ListBoxColor.Red() ^ 0xFF, ListBoxColor.Green() ^ 0xFF, ListBoxColor.Blue() ^ 0xFF );
    m_EveAttr = wxListItemAttr( ListBoxText,
                                ListBoxColor,
                                wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

    SetBackgroundColour( ListBoxColor );

    if( ListBoxColor.Red() > 0x0A && ListBoxColor.Green() > 0x0A && ListBoxColor.Blue() > 0x0A )
    {
        ListBoxColor.Set( ListBoxColor.Red() - 0xA,
                          ListBoxColor.Green() - 0xA,
                          ListBoxColor.Blue() - 0xA );
    }
    else
    {
        ListBoxColor.Set( ListBoxColor.Red() + 0xA,
                          ListBoxColor.Green() + 0xA,
                          ListBoxColor.Blue() + 0xA );
    }

    m_OddAttr = wxListItemAttr( ListBoxText,
                                ListBoxColor,
                                wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );


    //ReloadItems();
}


// -------------------------------------------------------------------------------- //
guSoListBox::~guSoListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    int index;
    int count = m_Columns.Count();
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( m_ConfName + wxT( "ColSize%u" ), index ), GetColumnWidth( index ), wxT( "Positions" ) );
    }
    Config->WriteANum( m_ConfName + wxT( "Col" ), m_Columns, m_ConfName + wxT( "s" ) );

    m_Songs.Clear();

    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guSoListBox::OnBeginDrag ), NULL, this );
    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guSoListBox::OnContextMenu ), NULL, this );
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnSearchLinkClicked ) );
    Disconnect( ID_SONGS_COMMANDS, ID_SONGS_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSoListBox::OnCommandClicked ) );

}

// -------------------------------------------------------------------------------- //
wxString guSoListBox::OnGetItemText( long item, long column ) const
{
    guTrack * Song;

    Song = &m_Songs[ item ];

    switch( m_Columns[ column ] )
    {
        case guSONGS_COLUMN_NUMBER :
          return wxString::Format( wxT( "%02u" ), Song->m_Number );

        case guSONGS_COLUMN_TITLE  :
          return Song->m_SongName;

        case guSONGS_COLUMN_ARTIST :
          return Song->m_ArtistName;

        case guSONGS_COLUMN_ALBUM :
          return Song->m_AlbumName;

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
wxListItemAttr * guSoListBox::OnGetItemAttr( long item ) const
{
    return ( wxListItemAttr * ) ( item & 1 ? &m_OddAttr : &m_EveAttr );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::FillTracks( void )
{
    SetItemCount( m_Db->GetSongs( &m_Songs ) );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::ReloadItems()
{
    long ItemCount;
    if( !m_Db )
        return;

    ClearSelection();

    if( m_Songs.Count() )
        m_Songs.Empty();

    //Songs.Add( new guTrack( 0, _( "All" ) ) );
    //ItemCount = m_Db->GetSongs( &m_Songs );
    //SetItemCount( ItemCount );
    FillTracks();

    wxListItem ListItem;
    GetColumn( 1, ListItem );
    ListItem.SetText( wxString::Format( _( "Title (%u)" ), m_Songs.Count() ) );
    SetColumn( 1, ListItem );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnBeginDrag( wxMouseEvent &event )
{
    //printf( "Drag started\n" );
    int index;
    int count;
    wxFileDataObject Files; // = wxFileDataObject();
    guTrackArray Songs = GetSelectedSongs();

    count = Songs.Count();

    for( index = 0; index < count; index++ )
    {
      Files.AddFile( Songs[ index ].m_FileName );
      //printf( "Added file " ); printf( Songs[ index ].FileName.char_str() ); printf( "\n" );
    }

    wxDropSource source( Files, this );

    source.DoDragDrop();
    //wxMessageBox( wxT( "DoDragDrop Done" ) );
}

// -------------------------------------------------------------------------------- //
wxArrayInt guSoListBox::GetSelection( void ) const
{
    wxArrayInt RetVal;
    long item = -1;
    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        RetVal.Add( m_Songs[ item ].m_SongId );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guTrackArray guSoListBox::GetSelectedSongs() const
{
    guTrackArray RetVal;
    long item = -1;
    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        // this item is selected - do whatever is needed with it
        RetVal.Add( new guTrack( m_Songs[ item ] ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guTrackArray guSoListBox::GetAllSongs() const
{
    guTrackArray RetVal;
    RetVal = m_Songs;
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guSoListBox::ClearSelection()
{
    long item = -1;
    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        // this item is selected - do whatever is needed with it
        SetItemState( item, false, -1 );
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
void guSoListBox::OnContextMenu( wxContextMenuEvent& event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPosition();
    // If from keyboard
    if( Point.x == -1 && Point.y == -1 )
    {
        wxSize Size = GetSize();
        Point.x = Size.x / 2;
        Point.y = Size.y / 2;
    }
    else
    {
        Point = ScreenToClient( Point );
    }

    int SelCount = GetSelection().Count();

    MenuItem = new wxMenuItem( &Menu, ID_SONG_PLAY, _( "Play" ), _( "Play current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_SONG_PLAYALL, _( "Play All" ), _( "Play all songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_SONG_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_SONG_ENQUEUEALL, _( "Enqueue All" ), _( "Add all songs to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu.Append( MenuItem );

    if( SelCount )
    {
        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_SONG_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_SONG_EDITTRACKS, _( "Edit Songs" ), _( "Edit the songs selected" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu.Append( MenuItem );

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_SONG_SAVEPLAYLIST, _( "Save Playlist" ), _( "Save all selected tracks as a playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_SONG_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu.Append( MenuItem );

        if( SelCount == 1 )
        {
            Menu.AppendSeparator();

            AddOnlineLinksMenu( &Menu );
        }

        AddSongsCommands( &Menu, SelCount );
    }

    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    int Item = wxNOT_FOUND;
    Item = GetNextItem( Item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
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
    wxArrayInt Selection = GetSelection();
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
                guTrackArray Songs = GetSelectedSongs();
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
                guTrackArray Songs = GetSelectedSongs();
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
                guTrackArray Songs = GetSelectedSongs();
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
wxString guSoListBox::GetSearchText( int item )
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Songs[ item ].m_ArtistName.c_str(),
        m_Songs[ item ].m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
