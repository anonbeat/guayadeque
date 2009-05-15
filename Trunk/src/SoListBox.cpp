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
#include "PlayList.h" // LenToString
#include "Config.h" // Configuration
#include "Commands.h"
#include "Images.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guSoListBox::guSoListBox( wxWindow * parent, DbLibrary * NewDb ) :
             wxListCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL )
{
    wxListItem ListItem;
    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_Db = NewDb;


    // Create the Columns
    ListItem.SetText( wxT( "#" ) );
    ListItem.SetImage( wxNOT_FOUND );
    ListItem.SetWidth( Config->ReadNum( wxT( "SongColSize0" ), 40, wxT( "Positions" ) ) );
    InsertColumn( 0, ListItem );

    ListItem.SetText( _( "Title" ) );
    ListItem.SetWidth( Config->ReadNum( wxT( "SongColSize1" ), 200, wxT( "Positions" ) ) );
    InsertColumn( 1, ListItem );

    ListItem.SetText( _( "Artist" ) );
    ListItem.SetWidth( Config->ReadNum( wxT( "SongColSize2" ), 200, wxT( "Positions" ) ) );
    InsertColumn( 2, ListItem );

    ListItem.SetText( _( "Album" ) );
    ListItem.SetWidth( Config->ReadNum( wxT( "SongColSize3" ), 200, wxT( "Positions" ) ) );
    InsertColumn( 3, ListItem );

    ListItem.SetText( _( "Length" ) );
    ListItem.SetWidth( Config->ReadNum( wxT( "SongColSize4" ), 100, wxT( "Positions" ) ) );
    InsertColumn( 4, ListItem );

    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guSoListBox::OnBeginDrag ), NULL, this );
    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guSoListBox::OnContextMenu ), NULL, this );

//    m_EveAttr = wxListItemAttr( wxColor( 0, 0, 0 ),
//                              wxColor( 250, 250, 250 ),
//                              wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );
//
//    m_OddAttr = wxListItemAttr( wxColor( 0, 0, 0 ),
//                              wxColor( 240, 240, 240 ),
//                              wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );
//
//    SetBackgroundColour( wxColor( 250, 250, 250 ) );
    wxColour ListBoxColor = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );
    m_EveAttr = wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_MENUTEXT ),
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

    m_OddAttr = wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_MENUTEXT ),
                                ListBoxColor,
                                wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );


    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guSoListBox::~guSoListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    int Index;
    for( Index = 0; Index < 5; Index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "SongColSize%u" ), Index ), GetColumnWidth( Index ), wxT( "Positions" ) );
    }

    m_Songs.Clear();

    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guSoListBox::OnBeginDrag ), NULL, this );

}

// -------------------------------------------------------------------------------- //
wxString guSoListBox::OnGetItemText( long item, long column ) const
{
    guTrack * Song;

    Song = &m_Songs[ item ];

    switch( column )
    {
        case 0 :
          return wxString::Format( wxT( "%02u" ), Song->m_Number );
          break;
        case 1 :
          return Song->m_SongName;

        case 2 :
          return Song->m_ArtistName;

        case 3 :
          return Song->m_AlbumName;

        case 4 :
          return LenToString( Song->m_Length );
          break;
    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
wxListItemAttr * guSoListBox::OnGetItemAttr( long item ) const
{
    return ( wxListItemAttr * ) ( item & 1 ? &m_EveAttr : &m_OddAttr );
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
    ItemCount = m_Db->GetSongs( &m_Songs );

    SetItemCount( ItemCount );

    wxListItem ListItem;
    GetColumn( 1, ListItem );
    ListItem.SetText( wxString::Format( _( "Title (%u)" ), ItemCount ) );
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
    MenuItem = new wxMenuItem( &Menu, ID_SONG_PLAY, _( "Play" ), _( "Play current selected songs" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_media_playback_start ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_SONG_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_SONG_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected songs" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_SONG_EDITTRACKS, _( "Edit Songs" ), _( "Edit the songs selected" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_SONG_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_edit_copy ) );
    Menu.Append( MenuItem );

    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
