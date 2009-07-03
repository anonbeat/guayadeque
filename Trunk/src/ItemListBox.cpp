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
#include "ItemListBox.h"
#include "Utils.h"

#include <wx/dnd.h>

BEGIN_EVENT_TABLE(guListBox,wxListCtrl)
    EVT_MOUSE_EVENTS( guListBox::OnMouse)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guListBox::guListBox( wxWindow * parent, DbLibrary * db, wxString label ) :
             wxListCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL|wxLC_AUTOARRANGE|wxSUNKEN_BORDER )
{
    wxListItem ListItem;

    m_Db = db;
    m_SearchStrTimer = new guItemListBoxTimer( this );

    // Create the Columns
    m_Label = label;
    m_SearchStr = wxEmptyString;
    ListItem.SetText( m_Label );
    ListItem.SetImage( wxNOT_FOUND );
    ListItem.SetWidth( 200 );
    InsertColumn( 0, ListItem );

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

    if( parent )
    {
        parent->Connect( wxEVT_SIZE, wxSizeEventHandler( guListBox::OnChangedSize ), NULL, this );
    }

    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guListBox::OnBeginDrag ), NULL, this );
    //Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guListBox::OnContextMenu ), NULL, this );
    Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( guListBox::OnContextMenu ), NULL, this );
    //Connect( wxEVT_COMMAND_LIST_KEY_DOWN, wxListEventHandler( guListBox::OnKeyDown ), NULL, this );
    Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guListBox::OnKeyDown ), NULL, this );

}


// -------------------------------------------------------------------------------- //
guListBox::~guListBox()
{
    if( GetParent() )
        GetParent()->Disconnect( wxEVT_SIZE, wxSizeEventHandler( guListBox::OnChangedSize ), NULL, this );

    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guListBox::OnBeginDrag ), NULL, this );
    //Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guListBox::OnContextMenu ), NULL, this );
    Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( guListBox::OnContextMenu ), NULL, this );
    //Disconnect( wxEVT_COMMAND_LIST_KEY_DOWN, wxListEventHandler( guListBox::OnKeyDown ), NULL, this );
    Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guListBox::OnKeyDown ), NULL, this );

    m_Items.Clear();

    if( m_SearchStrTimer )
      delete m_SearchStrTimer;
}

// -------------------------------------------------------------------------------- //
void guListBox::OnKeyDown( wxKeyEvent &event )
{
    //event.Skip();
    int KeyCode = event.GetKeyCode();

    //guLogMessage( wxT( "KeyPressed %i" ), KeyCode );
    //if( wxIsalnum( KeyChar ) )
    if( ( KeyCode >= 'a' && KeyCode <= 'z' ) ||
        ( KeyCode >= 'A' && KeyCode <= 'Z' ) ||
        ( KeyCode >= '0' && KeyCode <= '9' ) )
    {
        if( m_SearchStrTimer->IsRunning() )
        {
            m_SearchStrTimer->Stop();
        }
        m_SearchStrTimer->Start( 500, wxTIMER_ONE_SHOT );
        m_SearchStr.Append( KeyCode );
    }
    else if( event.ShiftDown() && ( ( KeyCode == WXK_UP ) || ( KeyCode == WXK_DOWN ) ) )
    {
        wxListEvent le( wxEVT_COMMAND_LIST_ITEM_SELECTED, GetId() );
        le.SetEventObject( this );
        wxPostEvent( this, le );
    }
    else if( KeyCode == WXK_MENU )
    {
        wxMouseEvent me( wxEVT_RIGHT_DOWN );
        me.m_x = me.m_y = -1;
        wxPostEvent( this, me );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guListBox::OnMouse( wxMouseEvent &event )
{
    if( event.LeftDown() && event.ShiftDown() )
    {
        wxPoint Where( event.GetX(), event.GetY() );
        int Flags = wxLIST_HITTEST_ONITEM;
        if( HitTest( Where, Flags, NULL ) != wxNOT_FOUND )
        {
            wxListEvent le( wxEVT_COMMAND_LIST_ITEM_SELECTED, GetId() );
            le.SetEventObject( this );
            wxPostEvent( this, le );
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guListBox::OnBeginDrag( wxMouseEvent &event )
{
    //printf( "Drag started\n" );
    guTrackArray Songs;
    wxFileDataObject Files; // = wxFileDataObject();
    int index;
    int count;
    if( GetSelectedSongs( &Songs ) )
    {
        count = Songs.Count();

        for( index = 0; index < count; index++ )
        {
          Files.AddFile( Songs[ index ].m_FileName );
          //printf( "Added file " ); printf( Songs[ index ].FileName.char_str() ); printf( "\n" );
        }

        wxDropSource source( Files, this );

        wxDragResult Result = source.DoDragDrop();
        if( Result )
        {
        }
        //wxMessageBox( wxT( "DoDragDrop Done" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guListBox::AdjustColumnWidth( int width )
{
    if( GetItemCount() > GetCountPerPage() )
        width -= wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );
    SetColumnWidth( 0, width );
}

// -------------------------------------------------------------------------------- //
void guListBox::OnChangedSize( wxSizeEvent &event )
{
    AdjustColumnWidth( event.GetSize().GetWidth() );
    event.Skip();
}

// -------------------------------------------------------------------------------- //
wxString guListBox::OnGetItemText( long item, long column ) const
{
    //return Items[ item ].AfterFirst( ':' );
    return m_Items[ item ].m_Name;
}


// -------------------------------------------------------------------------------- //
wxListItemAttr * guListBox::OnGetItemAttr( long item ) const
{
    return ( wxListItemAttr *) ( item & 1 ?  &m_OddAttr : &m_EveAttr );
}

// -------------------------------------------------------------------------------- //
wxString guListBox::GetItemText( long item ) const
{
    return m_Items[ item ].m_Name;
}

// -------------------------------------------------------------------------------- //
int guListBox::GetItemData( long item ) const
{
    return m_Items[ item ].m_Id;
}

// -------------------------------------------------------------------------------- //
wxArrayInt guListBox::GetSelection( bool reallist ) const
{
    wxArrayInt  RetVal;
    int         index;
    int         count;
    int         Id;
    long        item = -1;

    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        // this item is selected - do whatever is needed with it
        Id = GetItemData( item );
        RetVal.Add( Id );
        if( Id == 0 )
            break;
    }

    // If All was selected
    if( !reallist && ( RetVal.Index( 0 ) != wxNOT_FOUND ) )
    {
        RetVal.Empty();
        count = m_Items.Count();
        for( index = 0; index < count; index++ )
        {
            RetVal.Add( GetItemData( index ) );
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guListBox::SetSelection( const wxArrayInt selection )
{
    long Item = -1;
    while( ( Item = GetNextItem( Item, wxLIST_NEXT_ALL ) ) != wxNOT_FOUND )
    {
        if( selection.Index( GetItemData( Item ) ) != wxNOT_FOUND )
        {
          SetItemState( Item, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED );
        }

    }
}

// -------------------------------------------------------------------------------- //
void guListBox::ClearSelection()
{
    long item = -1;
    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        // this item is selected - do whatever is needed with it
        //SetItemState( item, false, -1 );
        SetItemState( item, false, wxLIST_STATE_SELECTED );
    }
}

// -------------------------------------------------------------------------------- //
void guListBox::ReloadItems( bool reset )
{
    long ItemCount;
    wxArrayInt SelItems;

    if( !m_Db )
        return;

    int FirstVisible = GetTopItem();

    if( reset )
        ClearSelection();
    else
        SelItems = GetSelection( true );

    if( m_Items.Count() )
        m_Items.Empty();

    GetItemsList();
    ItemCount = m_Items.GetCount();
    SetItemCount( ItemCount );

    if( !reset )
    {
        SetSelection( SelItems );
        EnsureVisible( FirstVisible );
        //RefreshItems( FirstVisible, FirstVisible + GetCountPerPage() );
        RefreshItems( FirstVisible, wxMin( ItemCount, FirstVisible + GetCountPerPage() ) - 1 );
    }
    //guLogMessage( wxT( "ITEMLISTBOX: FirstVisible: %i" ), FirstVisible );
    //guLogMessage( wxT( "ITEMLISTBOX: ItemCount   : %i" ), ItemCount );

    wxListItem ListItem;
    GetColumn( 0, ListItem );
    ListItem.SetText( wxString::Format( m_Label + wxT( " (%u)" ), ItemCount - 1 ) );
    SetColumn( 0, ListItem );

    AdjustColumnWidth( GetSize().GetWidth() );
}

// -------------------------------------------------------------------------------- //
void guListBox::OnContextMenu( wxMouseEvent &event )
{
    //guLogMessage( wxT( "guListBox::OnContextMenu: %i" ), GetId() );
    wxPoint Point = event.GetPosition();
    // If from keyboard
    if( Point.x == -1 && Point.y == -1 )
    {
        wxSize Size = GetSize();
        Point.x = Size.x / 2;
        Point.y = Size.y / 2;
    }
//    else
//    {
//        Point = ScreenToClient( Point );
//    }
    ShowContextMenu( Point );
}

// -------------------------------------------------------------------------------- //
void guListBox::ShowContextMenu( const wxPoint &pos )
{
    wxMenu * Menu = new wxMenu();
    GetContextMenu( Menu );
    PopupMenu( Menu, pos.x, pos.y );
    delete Menu;
}

// -------------------------------------------------------------------------------- //
// wxItemListBoxTimer
// -------------------------------------------------------------------------------- //
void guItemListBoxTimer::Notify()
{
    int index;
    if( m_ItemListBox->m_SearchStr.Len() )
    {
        m_ItemListBox->ClearSelection();
        if( ( index = m_ItemListBox->FindItem( 0, m_ItemListBox->m_SearchStr, true ) ) != wxNOT_FOUND )
        {
            m_ItemListBox->SetItemState( index, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED );
            m_ItemListBox->EnsureVisible( index );
        }
        m_ItemListBox->m_SearchStr = wxEmptyString;
    }
}

// -------------------------------------------------------------------------------- //
