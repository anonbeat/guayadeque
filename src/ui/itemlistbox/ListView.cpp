// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#include "ListView.h"

#include "EventCommandIds.h"
#include "Config.h"
#include "Images.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "Utils.h"

#include <wx/dcbuffer.h>
#include <wx/event.h>
#include <wx/renderer.h>

#include <wx/string.h>
#include <wx/checklst.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/dialog.h>

namespace Guayadeque {

#define guLISTVIEW_ITEM_MARGIN      2

#define guLISTVIEW_TIMER_TIMEOUT    1000
#define guLISTVIEW_MIN_COL_SIZE     20
#define guLISTVIEWHEADER_IMAGE_WIDTH  18



wxDEFINE_EVENT( guEVT_LISTBOX_ITEM_COL_CLICKED, wxListEvent );
wxDEFINE_EVENT( guEVT_LISTBOX_ITEM_COL_RCLICKED, wxListEvent );

WX_DEFINE_OBJARRAY( guListViewColumnArray )

static wxString FindChars = wxT( "abcdefghijklmnñopqrstuvwxyzABCDEFGHIJKLMNÑOPQRSTUVWXYZ0123456789 @&*+-_=/\\ºª:.,;çÇ<>$%()'\"|" );

// -------------------------------------------------------------------------------- //
// guListViewColEdit
// -------------------------------------------------------------------------------- //
class guListViewColEdit : public wxDialog
{
  //private:

  protected:
    int                         m_SelItem;
    guListView *                m_Owner;
    guListViewColumnArray *     m_Columns;
    wxArrayString               m_ItemsText;
    wxArrayInt                  m_ItemsData;

    wxCheckListBox *            m_ColumnsListBox;
    wxBitmapButton *            m_UpBitmapBtn;
    wxBitmapButton *            m_DownBitmapBtn;

    void    OnColumnSelected( wxCommandEvent &event );
    void    OnUpBtnClick( wxCommandEvent &event );
    void    OnDownBtnClick( wxCommandEvent &event );

  public:
    guListViewColEdit( wxWindow * parent, guListViewColumnArray * columns );
    ~guListViewColEdit();

    void    UpdateColumns( void );
};

// -------------------------------------------------------------------------------- //
// guListViewClientTimer
// -------------------------------------------------------------------------------- //
class guListViewClientTimer : public wxTimer
{
  public:
    guListViewClient * m_ListViewClient;
    //
    guListViewClientTimer( guListViewClient * listviewclient )
    {
        m_ListViewClient = listviewclient;
    }

    //Called each time the timer's timeout expires
    void Notify();
};


// -------------------------------------------------------------------------------- //
// guListView
// -------------------------------------------------------------------------------- //
guListView::guListView( wxWindow * parent, const int flags, wxWindowID id, const wxPoint &pos, const wxSize &size, long style ) :
    wxScrolledWindow( parent, id, pos, size, style )
{
    m_Columns = new guListViewColumnArray();
    m_ImageList = ( wxImageList * ) NULL;
    m_ListBox = new guListViewClient( this, flags, m_Columns, &m_Attr );
    if( !( flags & guLISTVIEW_HIDE_HEADER ) )
        m_Header = new guListViewHeader( this, m_ListBox, m_Columns, wxPoint( 0, 0 ) );
    else
        m_Header = NULL;

    m_ColSelect = ( flags & guLISTVIEW_COLUMN_SELECT );
    m_AllowDrag = ( flags & guLISTVIEW_ALLOWDRAG );
    m_AllowDrop = ( flags & guLISTVIEW_ALLOWDROP );
    m_DragSelfItemsEnabled = ( flags & guLISTVIEW_DRAGSELFITEMS );

    m_DragOverItem = wxNOT_FOUND;
    m_LastDragOverItem = wxNOT_FOUND;
    m_DragOverAfter = false;
    m_LastDragOverAfter = false;
    m_DragSelfItems = false;

    if( m_AllowDrop )
    {
        SetDropTarget( new guListViewDropTarget( this ) );
    }

    parent->Bind( wxEVT_SIZE, &guListView::OnChangedSize, this );
    m_ListBox->Bind( wxEVT_KEY_DOWN, &guListView::OnKeyDown, this );
    Bind( wxEVT_CONTEXT_MENU, &guListView::OnContextMenu, this );
    if( m_AllowDrag )
    {
        Bind( wxEVT_LIST_BEGIN_DRAG, &guListView::OnBeginDrag, this );
    }

    m_ListBox->Bind( wxEVT_LEFT_DOWN, &guListView::OnMouse, this );
    m_ListBox->Bind( wxEVT_LEFT_UP, &guListView::OnMouse, this );
    m_ListBox->Bind( wxEVT_MOTION, &guListView::OnMouse, this );
}

// -------------------------------------------------------------------------------- //
guListView::~guListView()
{
    if( m_Columns )
        delete m_Columns;

    if( m_ImageList )
        delete m_ImageList;

    GetParent()->Unbind( wxEVT_SIZE, &guListView::OnChangedSize, this );
    m_ListBox->Unbind( wxEVT_KEY_DOWN, &guListView::OnKeyDown, this );
    Unbind( wxEVT_CONTEXT_MENU, &guListView::OnContextMenu, this );
    if( m_AllowDrag )
    {
        Unbind( wxEVT_LIST_BEGIN_DRAG, &guListView::OnBeginDrag, this );
    }

    m_ListBox->Unbind( wxEVT_LEFT_DOWN, &guListView::OnMouse, this );
    m_ListBox->Unbind( wxEVT_LEFT_UP, &guListView::OnMouse, this );
    m_ListBox->Unbind( wxEVT_MOTION, &guListView::OnMouse, this );
}

// -------------------------------------------------------------------------------- //
void guListView::InsertColumn( guListViewColumn * column )
{
    m_Columns->Add( column );
    if( m_Header )
        m_Header->RefreshWidth();
}

// -------------------------------------------------------------------------------- //
void guListView::OnChangedSize( wxSizeEvent &event )
{
    int w;
    int h = 0;
    int d;
    wxSize Size = event.GetSize();
    //Size.x -= 4;
    //Size.y -= 4;
    //guLogMessage( wxT( "ListView SetSize %i,%i" ), Size.x, Size.y );
    if( m_Header )
    {
        // Calculate the Height
        GetTextExtent( wxT("Hg"), &w, &h, &d );
        h += d + 4;
        // Only change size if its different
        if( m_Header->GetSize().GetWidth() != Size.x )
        {
            m_Header->SetSize( Size.x, h );
        }
        m_Header->RefreshWidth();
    }
    if( m_ListBox )
    {
        m_ListBox->SetSize( Size.x, Size.y - h );
        m_ListBox->Move( 0, h );
    }
    // continue with default behaivor
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guListView::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( ( row == wxNOT_FOUND ) || ( row >= GetItemCount() ) )
        return;
    m_ListBox->DoDrawItem( dc, rect, row, col );
}

// -------------------------------------------------------------------------------- //
void guListView::DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( row == wxNOT_FOUND )
        return;
    m_ListBox->DoDrawBackground( dc, rect, row, col );
}

// -------------------------------------------------------------------------------- //
void guListView::OnContextMenu( wxContextMenuEvent& event )
{
    wxMenu Menu;
    CreateContextMenu( &Menu );
    if( Menu.GetMenuItemCount() )
    {
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
        PopupMenu( &Menu, Point.x, Point.y );
    }
}


// -------------------------------------------------------------------------------- //
void guListView::SetItemCount( const int count )
{
    m_ListBox->SetItemCount( count );
    if( m_Header )
        m_Header->RefreshWidth();
}

// -------------------------------------------------------------------------------- //
wxCoord guListView::OnMeasureItem( size_t n ) const
{
    // Code taken from the generic/listctrl.cpp file
    guListView * self = wxConstCast( this, guListView );

    wxClientDC dc( self );
    dc.SetFont( GetFont() );

    wxCoord y;
    dc.GetTextExtent( wxT( "Hg" ), NULL, &y );

    self->SetItemHeight( y + 4 ); // 2 up, 2 down
    return y + 4;
}

// -------------------------------------------------------------------------------- //
void guListView::SetSelection( int selection )
{
    m_ListBox->SetSelection( selection );

    wxCommandEvent event( wxEVT_LISTBOX, m_ListBox->GetId() );
    event.SetEventObject( m_ListBox );
    event.SetInt( selection );
    (void) GetEventHandler()->ProcessEvent( event );
}

// -------------------------------------------------------------------------------- //
void guListView::RefreshAll( int scrollto )
{
    if( scrollto != wxNOT_FOUND )
    {
        if( !m_ListBox->IsVisible( scrollto ) )
        {
            m_ListBox->ScrollToRow( scrollto );
        }
    }
    m_ListBox->RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guListView::OnBeginDrag( wxCommandEvent &event )
{
    guDataObjectComposite Files;

    if( GetDragFiles( &Files ) )
    {
        wxDropSource source( Files, this );

        m_DragSelfItems = true;
        wxDragResult Result = source.DoDragDrop();
        if( Result )
        {
        }

        m_DragSelfItems = false;
        m_DragOverItem = wxNOT_FOUND;

        RefreshAll();
    }
}

// -------------------------------------------------------------------------------- //
wxArrayInt guListView::GetSelectedItems( const bool convertall ) const
{
    wxArrayInt RetVal;
    unsigned long cookie;
    int item;
    int index;
    int count;
    int ItemId;
    if( GetItemCount() )
    {
        if( m_ListBox->HasMultipleSelection() )
        {
            item = GetFirstSelected( cookie );
            while( item != wxNOT_FOUND )
            {
                ItemId = GetItemId( item );
                RetVal.Add( ItemId );
                if( convertall && ( ItemId == 0 ) )
                    break;
                item = GetNextSelected( cookie );
            }
        }
        else
        {
            item = m_ListBox->GetSelection();
            if( item != wxNOT_FOUND )
            {
                ItemId = GetItemId( item );
                RetVal.Add( ItemId );
            }
        }

        //
        if( convertall && ( RetVal.Index( 0 ) != wxNOT_FOUND ) )
        {
            RetVal.Empty();
            count = GetItemCount();
            for( index = 0; index < count; index++ )
            {
                RetVal.Add( GetItemId( index ) );
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayInt guListView::GetSelectedIndexs( const bool convertall ) const
{
    wxArrayInt RetVal;
    unsigned long cookie;
    int item;
    int index;
    int count;
    if( GetItemCount() )
    {
        if( m_ListBox->HasMultipleSelection() )
        {
            item = GetFirstSelected( cookie );
            while( item != wxNOT_FOUND )
            {
                RetVal.Add( item );
                if( convertall && ( item == 0 ) )
                    break;
                item = GetNextSelected( cookie );
            }
        }
        else
        {
            item = m_ListBox->GetSelection();
            if( item != wxNOT_FOUND )
            {
                RetVal.Add( item );
            }
        }

        //
        if( convertall && ( RetVal.Index( 0 ) != wxNOT_FOUND ) )
        {
            RetVal.Empty();
            count = GetItemCount();
            for( index = 0; index < count; index++ )
            {
                RetVal.Add( index );
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guListView::SetSelectedItems( const wxArrayInt &selection )
{
    int Count = GetItemCount();

    // TODO Need to speed up this
    ClearSelectedItems();

    if( Count && selection.Count() )
    {
        bool IsMultiple = m_ListBox->HasMultipleSelection();
        for( int Index = 0; Index < Count; Index++ )
        {
            if( selection.Index( GetItemId( Index ) ) != wxNOT_FOUND )
            {
                if( IsMultiple )
                    Select( Index );
                else
                    SetSelection( Index );
            }
        }
        wxCommandEvent event( wxEVT_LISTBOX, m_ListBox->GetId() );
        event.SetEventObject( m_ListBox );
        event.SetInt( selection[ 0 ] );
        (void) GetEventHandler()->ProcessEvent( event );
    }
}

// -------------------------------------------------------------------------------- //
void guListView::SetSelectedIndexs( const wxArrayInt &selection )
{
    // TODO Need to speed up this
    ClearSelectedItems();
    int ItemCount = GetItemCount();
    if( ItemCount )
    {
        int Count;
        if( ( Count = selection.Count() ) )
        {
            bool IsMultiple = m_ListBox->HasMultipleSelection();
            for( int Index = 0; Index < Count; Index++ )
            {
                if( selection[ Index ] < ItemCount )
                {
                    if( IsMultiple )
                        Select( selection[ Index ] );
                    else
                        SetSelection( selection[ Index ] );
                }
            }
            wxCommandEvent event( wxEVT_LISTBOX, m_ListBox->GetId() );
            event.SetEventObject( m_ListBox );
            event.SetInt( selection[ 0 ] );
            (void) GetEventHandler()->ProcessEvent( event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guListView::GetSelectedItems( guListItems * items, const bool convertall ) const
{
    unsigned long cookie;
    int item;
    int index;
    int count;
    bool AppendAll = false;
    if( ( count = GetItemCount() ) )
    {
        if( m_ListBox->HasMultipleSelection() )
        {
            item = GetFirstSelected( cookie );
            while( item != wxNOT_FOUND )
            {
                if( convertall && ( item == 0 ) )
                {
                    AppendAll = true;
                    break;
                }
                items->Add( new guListItem( GetItemId( item ), GetItemName( item ) ) );
                item = GetNextSelected( cookie );
            }
        }
        else
        {
            item = m_ListBox->GetSelection();
            if( item != wxNOT_FOUND )
            {
                items->Add( new guListItem( GetItemId( item ), GetItemName( item ) ) );
            }
        }

        //
        if( convertall && AppendAll )
        {
            for( index = 0; index < count; index++ )
            {
                items->Add( new guListItem( GetItemId( index ), GetItemName( index ) ) );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guListView::SetColumnWidth( const int col, const int width )
{
    ( * m_Columns )[ col ].m_Width = width;
    if( m_Header )
        m_Header->RefreshWidth();
}

// -------------------------------------------------------------------------------- //
void  guListView::SetColumnLabel( const int col, const wxString &label )
{
    m_Columns->Item( col ).m_Label = label;
    if( m_Header )
        m_Header->Refresh();
}

// -------------------------------------------------------------------------------- //
void guListView::SetColumnImage( const int col, const int imageindex )
{
    m_Columns->Item( col ).m_ImageIndex = imageindex;
    if( m_Header )
        m_Header->Refresh();
}


// -------------------------------------------------------------------------------- //
void guListView::SetImageList( wxImageList * imagelist )
{
    if( m_ImageList )
        delete m_ImageList;
    m_ImageList = imagelist;

    if( m_Header )
        m_Header->SetImageList( imagelist );
}

// -------------------------------------------------------------------------------- //
int guListView::GetDragFiles( guDataObjectComposite * files )
{
    guTrackArray Tracks;
    wxArrayString Filenames;
    int Index;
    int Count = GetSelectedSongs( &Tracks, true );
    for( Index = 0; Index < Count; Index++ )
    {
        if( Tracks[ Index ].m_Offset )
            continue;
        Filenames.Add( guFileDnDEncode( Tracks[ Index ].m_FileName ) );
    }
    files->SetTracks( Tracks );
    files->SetFiles( Filenames );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guListView::OnDragOver( wxCoord x, wxCoord y )
{
    //guLogMessage( wxT( ">>guListView::OnDragOver( %u, %u )" ), x, y );
    int HeaderHeight = 0;
    if( m_Header )
    {
        HeaderHeight = m_Header->GetSize().GetHeight();
    }

    y -= HeaderHeight;

    m_DragOverItem = HitTest( x, y );

    if( ( int ) m_DragOverItem != wxNOT_FOUND )
    {
        int ItemHeight = m_ListBox->OnMeasureItem( m_DragOverItem );
        m_DragOverAfter = ( y % ItemHeight ) > ( ItemHeight / 2 );
    }

    //guLogMessage( wxT( "Current: %i   Last: %i" ), m_DragOverItem, m_LastDragOverItem );

    if( ( m_DragOverItem != m_LastDragOverItem ) )
    {
        if( m_DragOverItem != wxNOT_FOUND )
            RefreshRows( m_DragOverItem, m_DragOverItem );
        if( m_LastDragOverItem != wxNOT_FOUND )
            RefreshRows( m_LastDragOverItem, m_LastDragOverItem );
        m_LastDragOverItem = m_DragOverItem;
    }
    else if( m_DragOverAfter != m_LastDragOverAfter )
    {
        RefreshRows( m_DragOverItem, m_DragOverItem );
        m_LastDragOverAfter = m_DragOverAfter;
    }


    // Scroll items if we are in the top or bottom borders
    int Width;
    int Height;
    GetSize( &Width, &Height );
    Height -= HeaderHeight;

    int Rows = 0;
    if( ( y > ( Height - 10 ) ) && ( int ) GetVisibleRowsEnd() != GetItemCount() )
    {
        Rows = 1;
    }
    else
    {
        if( ( y < 10 ) && ( GetVisibleRowsBegin() > 0 ) )
        {
            Rows = -1;
        }
    }
    //guLogMessage( wxT( "ScrollLines %i, %i, %i" ), Height, (int) y, Rows );
    //guLogMessage( wxT( "End: %li  Items:%i" ), GetVisibleRowsEnd(), GetItemCount() );
    if( Rows )
        ScrollRows( Rows );
}

// -------------------------------------------------------------------------------- //
void guListView::OnMouse( wxMouseEvent &event )
{
    if( event.Dragging() )
    {
        if( !m_DragCount )
        {
            m_DragStart = event.GetPosition();
        }

        if( ++m_DragCount == 3 )
        {
            wxListEvent le( wxEVT_LIST_BEGIN_DRAG, GetId() );
            le.SetEventObject( this );
            le.m_pointDrag = m_DragStart;
            GetEventHandler()->ProcessEvent( le );
        }
        return;
    }
    else
    {
      m_DragCount = 0;
    }

    event.Skip();
}

// -------------------------------------------------------------------------------- //
int FindColumnId( const guListViewColumnArray * columns, const int id )
{
    int index;
    int count = columns->Count();
    for( index = 0; index < count; index++ )
    {
        if( ( * columns )[ index ].m_Id == id )
            return index;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
bool  guListView::GetColumnData( const int id, int * index, int * width, bool * enabled )
{
    int ColPos = FindColumnId( m_Columns, id );
    if( ColPos != wxNOT_FOUND )
    {
        * index   = ColPos;
        * width   = ( * m_Columns )[ ColPos ].m_Width;
        * enabled = ( * m_Columns )[ ColPos ].m_Enabled;
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool  guListView::SetColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh )
{
    int ColPos = FindColumnId( m_Columns, id );
    if( ColPos != wxNOT_FOUND )
    {
        if( ColPos != index )
        {
            guListViewColumn * Column = m_Columns->Detach( ColPos );
            m_Columns->Insert( Column, index );
        }
        ( * m_Columns )[ index ].m_Width = width;
        ( * m_Columns )[ index ].m_Enabled = enabled;
        if( refresh )
        {
            m_Header->RefreshWidth();
            Refresh();
        }
        return true;
    }
    return false;
}




// -------------------------------------------------------------------------------- //
// guListViewClient
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guListViewClient,wxVListBox)
  EVT_PAINT          (guListViewClient::OnPaint)
  EVT_MOUSE_EVENTS   (guListViewClient::OnMouse)
  EVT_SCROLLWIN      (guListViewClient::OnHScroll)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guListViewClient::guListViewClient( wxWindow * parent, const int flags,
                            guListViewColumnArray * columns, guListViewAttr * attr ) :
    wxVListBox( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags|wxHSCROLL|wxVSCROLL|wxNO_BORDER )
{
    m_Owner = ( guListView * ) parent;
    m_SearchStrTimer = new guListViewClientTimer( this );
    m_Attr = attr;
    m_Columns = columns;
    m_SearchStr = wxEmptyString;
    m_ItemHeight = wxNOT_FOUND;
    m_HScrollPos = 0;
    m_MouseWasLeftUp = false;
    m_MouseSelecting = false;
    m_ColumnClickEvents = ( flags & guLISTVIEW_COLUMN_CLICK_EVENTS );

    SetBackgroundColour( m_Attr->m_EveBgColor );

    Bind( wxEVT_KEY_DOWN, &guListViewClient::OnKeyDown, this );
}

// -------------------------------------------------------------------------------- //
guListViewClient::~guListViewClient()
{
    if( m_SearchStrTimer )
      delete m_SearchStrTimer;

    Unbind( wxEVT_KEY_DOWN, &guListViewClient::OnKeyDown, this );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::OnMouse( wxMouseEvent &event )
{
    int MouseX = event.m_x;
    int MouseY = event.m_y;
    int Item = HitTest( MouseX, MouseY );
    bool ResetVals = false;

    // We want to get a better experience for dragging as before
    // when you click over selected items the items was unselected
    // even when you tried to drag then.
    // Here we check if the item was selected and if so wait for the button up
    // to unselecte the item
    //guLogMessage( wxT( "ID: %u LD: %i LU: %i SD: %i CD: %i WasLeftUp: %i  Selecting: %i " ), event.GetId(), event.LeftDown(), event.LeftUp(), event.ShiftDown(), event.ControlDown(), m_MouseWasLeftUp, m_MouseSelecting );
    if( !m_MouseWasLeftUp && !event.ShiftDown() && !event.ControlDown() )
    {
        m_MouseWasLeftUp = event.LeftUp();
        if( ( event.LeftDown() || m_MouseWasLeftUp ) )
        {
            if( Item != wxNOT_FOUND )
            {
                bool Selected = IsSelected( Item );
                if( Selected )
                {
                    if( !m_MouseSelecting && event.LeftUp() )
                    {
                        // Its a LeftUp event
                        event.SetEventType( wxEVT_LEFT_DOWN );
                        event.m_leftDown = true;
                        AddPendingEvent( event );
                    }
                    return;
                }
                m_MouseSelecting = event.LeftDown();
            }
        }
        else
        {
            ResetVals = true;
        }
    }
    else
    {
        ResetVals = true;
    }

    // Only when the left or right is down and the click events are enabled
    if( m_ColumnClickEvents &&
        ( Item != wxNOT_FOUND ) &&
        ( ( event.LeftDown() && !( ResetVals && m_MouseSelecting ) ) || event.RightDown() ) )
    {
        // We want get the left click events, calculate which column clicked on and
        // send a Column_Clicked event
        int Col_Num = wxNOT_FOUND;

        MouseX += GetScrollPos( wxHORIZONTAL );

        int Index;
        int Col_Start = 0;
        int Col_Border = 0;
        int Count = m_Columns->Count();
        for( Index = 0; Index < Count; Index++ )
        {
            if( ( * m_Columns )[ Index ].m_Enabled )
            {
                Col_Border += ( * m_Columns )[ Index ].m_Width;
                if( MouseX < Col_Border )
                {
                    Col_Num = Index;
                    break;
                }
                Col_Start = Col_Border;
            }
        }

        if( Col_Num != wxNOT_FOUND )
        {
            wxListEvent le( event.LeftDown() ? guEVT_LISTBOX_ITEM_COL_CLICKED :
                                               guEVT_LISTBOX_ITEM_COL_RCLICKED, m_Owner->GetId() );
            le.SetEventObject( m_Owner );
            le.m_pointDrag.x = MouseX - Col_Start;
            le.m_pointDrag.y = MouseY;
            le.SetInt( Item );
            le.m_col = Col_Num;
            m_Owner->GetEventHandler()->ProcessEvent( le );
            //guLogMessage( wxT( "Col %i have been clicked (%i-%i) %i-%i" ), Col_Num, Col_Start, Col_Border, MouseX - Col_Start, MouseY );
            //return;
        }
    }

    // The wxVListBox dont handle the right click to select items. We add this functionality
    if( event.RightDown() && ( Item != wxNOT_FOUND ) && !IsSelected( Item ) )
        OnLeftDown( event );

    if( ResetVals )
    {
        m_MouseWasLeftUp = false;
        m_MouseSelecting = false;
    }

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guListViewClient::OnKeyDown( wxKeyEvent &event )
{
    wxChar KeyChar = event.GetKeyCode();

    int KeyMod = event.GetModifiers();
    if( KeyMod == wxMOD_NONE || KeyMod == wxMOD_SHIFT )
    {
        if( FindChars.Find( KeyChar ) != wxNOT_FOUND )
        {
            if( m_SearchStrTimer->IsRunning() )
            {
                m_SearchStrTimer->Stop();
            }
            m_SearchStrTimer->Start( guLISTVIEW_TIMER_TIMEOUT, wxTIMER_ONE_SHOT );
            m_SearchStr.Append( KeyChar );
            return;
        }
        else if( ( event.GetKeyCode() == WXK_RETURN ) || ( event.GetKeyCode() == WXK_NUMPAD_ENTER ) )
        {
            if( GetSelectedCount() )
            {
                unsigned long Cookie;
                wxCommandEvent DCEvent( wxEVT_LISTBOX_DCLICK, GetId() );
                DCEvent.SetEventObject( this );
                DCEvent.SetInt( GetFirstSelected( Cookie ) );
                ( void ) GetEventHandler()->ProcessEvent( DCEvent );
            }
        }
    }
    else if( KeyMod == wxMOD_CONTROL )
    {
        if( KeyChar == 'a' || KeyChar == 'A' )
        {
            SelectAll();
            wxCommandEvent evt( wxEVT_LISTBOX, GetId() );
            evt.SetEventObject( this );
            evt.SetInt( 0 );
            (void) GetEventHandler()->ProcessEvent( evt );
            return;
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guListViewClient::OnPaint( wxPaintEvent &event )
{
    // CODE FROM THE wxVListBox class
    wxSize clientSize = GetClientSize();

    wxAutoBufferedPaintDC dc( this );

    // the update rectangle
    wxRect rectUpdate = GetUpdateClientRect();

    // fill it with background colour
    dc.SetBackground( GetBackgroundColour() );
    dc.Clear();

    AdjustDC( dc );

    // the bounding rectangle of the current line
    wxRect rectLine;
    rectLine.width = clientSize.x;

    m_Owner->ItemsLock();
    const size_t lineMax = GetVisibleEnd();
    size_t line = GetVisibleRowsBegin();
    m_Owner->ItemsCheckRange( line, lineMax );
    // iterate over all visible lines
    for( ; line < lineMax; line++ )
    {
        const wxCoord hLine = OnGetRowHeight( line );

        rectLine.height = hLine;

        // and draw the ones which intersect the update rect
        if( rectLine.Intersects( rectUpdate ) )
        {
            // don't allow drawing outside of the lines rectangle
            wxDCClipper clip(dc, rectLine);

            wxRect rect = rectLine;
            OnDrawBackground(dc, rect, line);

            OnDrawSeparator(dc, rect, line);

            //rect.Deflate(m_ptMargins.x, m_ptMargins.y);
            OnDrawItem( dc, rect, line );
        }
        else // no intersection
        {
            if ( rectLine.GetTop() > rectUpdate.GetBottom() )
            {
                // we are already below the update rect, no need to continue
                // further
                break;
            }
            //else: the next line may intersect the update rect
        }

        rectLine.y += hLine;
    }
    m_Owner->ItemsUnlock();
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DoDrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    dc.SetFont( * m_Attr->m_Font );
    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.SetTextForeground( IsSelected( row ) ? m_Attr->m_SelFgColor : m_Attr->m_TextFgColor );
    dc.DrawText( m_Owner->OnGetItemText( row, col ),
                  rect.x + guLISTVIEW_ITEM_MARGIN, rect.y + guLISTVIEW_ITEM_MARGIN );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    m_Owner->DrawItem( dc, rect, row, col );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const
{
    if( ( int ) n == wxNOT_FOUND )
        return;

    wxRect cRect = rect;

    int StartOfs = rect.x;
    int index;
    int count = m_Columns->Count();
    for( index = 0; index < count; index++ )
    {
        if( ( * m_Columns )[ index ].m_Enabled )
        {
            int w = ( * m_Columns )[ index ].m_Width;
            if( w == wxNOT_FOUND )
                w = rect.width;

            cRect.x = StartOfs;
            cRect.width = w - guLISTVIEW_ITEM_MARGIN;

            {
                wxDCClipper clip( dc, cRect );

                DrawItem( dc, cRect, n, index );
            }
            StartOfs += w;
            if( StartOfs > ( m_HScrollPos + rect.width ) )
                break;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guListViewClient::SetItemHeigth( const int height )
{
    m_ItemHeight = height;
}

// -------------------------------------------------------------------------------- //
wxCoord guListViewClient::OnMeasureItem( size_t n ) const
{
    if( m_ItemHeight != wxNOT_FOUND )
    {
        return m_ItemHeight;
    }
    else
    {
        return m_Owner->OnMeasureItem( n );
    }
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DoDrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    wxRect LineRect;

    if( IsSelected( row ) )
      dc.SetBrush( wxBrush( m_Attr->m_SelBgColor ) );
    else if( row == ( int ) m_Owner->m_DragOverItem )
      dc.SetBrush( m_Attr->m_DragBgColor );
    else
      dc.SetBrush( wxBrush( row & 1 ? m_Attr->m_OddBgColor : m_Attr->m_EveBgColor ) );

    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.DrawRectangle( rect );

    if( row == ( int ) m_Owner->m_DragOverItem )
    {
        LineRect = rect;
        if( m_Owner->m_DragOverAfter )
            LineRect.y += ( LineRect.height - 2 );
        LineRect.height = 2;
        dc.SetBrush( * wxBLACK_BRUSH );
        dc.DrawRectangle( LineRect );
    }
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    m_Owner->DrawBackground( dc, rect, row, col );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::OnDrawBackground( wxDC &dc, const wxRect &rect, size_t n ) const
{
    if( ( int ) n == wxNOT_FOUND )
        return;

    wxRect cRect = rect;
    int StartOfs = rect.x;
    int index;
    int count = m_Columns->Count();
    for( index = 0; index < count; index++ )
    {
        if( ( * m_Columns )[ index ].m_Enabled )
        {
            cRect.x     = StartOfs;
            int w = ( * m_Columns )[ index ].m_Width;
            if( w != wxNOT_FOUND )
                cRect.width = w;

            {
                wxDCClipper clip( dc, cRect );

                DrawBackground( dc, cRect, n, index );
            }
            StartOfs += cRect.width;
            if( StartOfs > ( m_HScrollPos + rect.width ) )
                break;
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guListViewClient::GetItemSearchText( const int row )
{
    return m_Owner->GetItemSearchText( row );
}

// -------------------------------------------------------------------------------- //
long guListViewClient::FindItem( long start, const wxString& str, bool partial, bool atstart )
{
    if( str.empty() )
        return wxNOT_FOUND;

    long pos = start;
    wxString str_upper = str.Upper();
    if( pos < 0 )
        pos = 0;

    int index;
    int count = GetItemCount();
    for( index = pos; index < count; index++ )
    {
        wxString line_upper = GetItemSearchText( index ).Upper();
        if( !partial )
        {
            if( line_upper == str_upper )
                return index;
        }
        else if( atstart )
        {
            if( line_upper.Find( str_upper ) == 0 )
                return index;
        }
        else
        {
            if( line_upper.Find( str_upper ) != wxNOT_FOUND )
                return index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guListViewClient::OnHScroll( wxScrollWinEvent &event )
{
    if( event.GetOrientation() == wxHORIZONTAL )
    {
        m_HScrollPos = event.GetPosition();
        m_Owner->Update();
        m_Owner->Refresh();
    }
    else
    {
        event.Skip();
    }
}

// -------------------------------------------------------------------------------- //
void guListViewClient::SetHScrollbar( const int width )
{
    if( width )
        SetScrollbar( wxHORIZONTAL, m_HScrollPos, 125, width + 125 );
    else
        SetScrollbar( wxHORIZONTAL, 0, 0, 0 );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::SetScrollPos( int orientation, int pos, bool refresh )
{
    if( orientation == wxHORIZONTAL )
    {
        m_HScrollPos = pos;
    }
    wxVListBox::SetScrollPos( orientation, pos, refresh );
}

// -------------------------------------------------------------------------------- //
//int guListViewClient::GetHScrollPosition( void )
//{
//    return m_HScrollPos;
//}

// -------------------------------------------------------------------------------- //
void guListViewClient::AdjustDC( wxDC &dc )
{
    int org_x = 0;
    int org_y = 0;
    dc.GetDeviceOrigin( &org_x, &org_y );

    dc.SetDeviceOrigin( org_x - m_HScrollPos, org_y );
}




// -------------------------------------------------------------------------------- //
//  guListViewHeader
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guListViewHeader,wxWindow)
    EVT_PAINT               (guListViewHeader::OnPaint)
    EVT_MOUSE_EVENTS        (guListViewHeader::OnMouse)
    EVT_SET_FOCUS           (guListViewHeader::OnSetFocus)
    EVT_MOUSE_CAPTURE_LOST  (guListViewHeader::OnCaptureLost)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guListViewHeader::guListViewHeader()
{
    m_Owner = (guListView *) NULL;
    m_ListViewClient = ( guListViewClient * ) NULL;
    m_Columns = (guListViewColumnArray*) NULL;
    m_ResizeCursor = new wxCursor( wxCURSOR_SIZEWE );
    m_IsDragging = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
guListViewHeader::guListViewHeader( wxWindow * parent, guListViewClient * owner,
             guListViewColumnArray * columns, const wxPoint& pos, const wxSize& size )
    : wxWindow( parent, wxID_ANY, pos, size )
{
    m_Owner = ( guListView * ) parent;
    m_ListViewClient = owner;
    m_Columns = columns;
    m_ResizeCursor = new wxCursor( wxCURSOR_SIZEWE );
    m_IsDragging = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
guListViewHeader::~guListViewHeader()
{
    if( m_ResizeCursor )
        delete m_ResizeCursor;
}

// -------------------------------------------------------------------------------- //
void guListViewHeader::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    AdjustDC( dc );

    dc.SetFont( GetFont() );

    // width and height of the entire header window
    int w, h;
    GetClientSize( &w, &h );

    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.SetTextForeground( GetForegroundColour() );

    int flags = 0;
    if( !m_parent->IsEnabled() )
        flags |= wxCONTROL_DISABLED;

    wxCoord wLabel;
    wxCoord hLabel;
    dc.GetTextExtent( wxT( "hg" ), &wLabel, &hLabel );
    hLabel = ( h / 2 ) - ( hLabel / 2 );

    int index;
    int count = m_Columns->Count();
    wxRect cRect = wxRect( 0, 0, w, h );
    int StartOfs = 0;
    int ScrollPos = m_ListViewClient->GetScrollPos( wxHORIZONTAL );
    for( index = 0; index < count; index++ )
    {
        guListViewColumn * CurCol = &m_Columns->Item( index );
        if( CurCol->m_Enabled )
        {
            cRect.x = StartOfs;
            int cw = CurCol->m_Width;
            if( cw != wxNOT_FOUND )
                cRect.width = cw;

            //guLogMessage( wxT( "Pinting header %u at %u %u '%s'" ), index, cRect.x, cRect.width, ( * m_Columns )[ index ].m_Label.c_str() );

            wxRendererNative::Get().DrawHeaderButton( this, dc, cRect, flags );

            if( CurCol->m_ImageIndex >= 0 )
                cRect.width -= guLISTVIEWHEADER_IMAGE_WIDTH;
            dc.SetClippingRegion( cRect );

            dc.DrawText( CurCol->m_Label, cRect.x + 4, hLabel );
            dc.DestroyClippingRegion();

            if( CurCol->m_ImageIndex >= 0 )
            {
                dc.DrawBitmap( m_ImageList->GetBitmap( CurCol->m_ImageIndex ), cRect.x + cRect.width + 1, ( cRect.height / 2 ) - 8, true );
            }

            StartOfs += CurCol->m_Width;
            if( StartOfs > w + ScrollPos )
                break;
        }
    }

    if( m_Width < w )
    {
        cRect.x = m_Width;
        cRect.width = w - m_Width;
        wxRendererNative::Get().DrawHeaderButton( this, dc, cRect, flags );
    }
}

// -------------------------------------------------------------------------------- //
void guListViewHeader::OnCaptureLost( wxMouseCaptureLostEvent &event )
{
    m_IsDragging = false;
}

// -------------------------------------------------------------------------------- //
void guListViewHeader::OnMouse( wxMouseEvent &event )
{
    // we want to work with logical coords
    int mx;
    int hit_border = wxNOT_FOUND;
    int col_num = wxNOT_FOUND;

    int ScrollPos = m_ListViewClient->GetScrollPos( wxHORIZONTAL );
    mx = event.GetX() + ScrollPos;

    int my = event.GetY();

    int index;
    int col_border = 0;
    int col_start = 0;
    int count = m_Columns->Count();
    for( index = 0; index < count; index++ )
    {
        if( ( * m_Columns )[ index ].m_Enabled )
        {
            col_border += ( * m_Columns )[ index ].m_Width;
            if( ( my < 22 ) && abs( mx - col_border ) < 3 )
            {
                hit_border = index;
                break;
            }

            if( mx < col_border + 3 )
            {
                col_num = index;
                break;
            }
            col_start = col_border;
        }
    }

//    guLogMessage( wxT( "cn:%i  cs:%i  cb:%i  id:%i  mx:%i  my:%i" ), col_num, col_start, col_border, m_IsDragging, mx, my );

    //
    if( m_IsDragging >= 0 )
    {
        if( event.LeftUp() )
        {
            m_IsDragging = wxNOT_FOUND;
            ReleaseMouse();
        }
        else
        {
            //guLogMessage( wxT( "sp: %i  mx:%i  do:%i" ), ScrollPos, mx, m_DragOfset );
            if( mx > ( m_DragOfset + guLISTVIEW_MIN_COL_SIZE ) )
            {
                m_Owner->SetColumnWidth( m_IsDragging, mx - m_DragOfset );
                m_Owner->Refresh();
            }
        }
    }
    else if( event.LeftDown() || event.RightDown() )
    {
        if( event.LeftDown() && ( hit_border >= 0 ) ) // hit a border
        {
            m_IsDragging = hit_border;
            m_DragOfset = col_start;
            CaptureMouse();
        }
        else if( m_Owner->IsAllowedColumnSelect() && event.RightDown() )
        {
            OnEditColumns();
        }
        else if( col_num >= 0 )
        {
            wxListEvent le( event.LeftDown() ? wxEVT_LIST_COL_CLICK :
                                               wxEVT_LIST_COL_RIGHT_CLICK, m_Owner->GetId() );
            le.SetEventObject( m_Owner );
            le.m_pointDrag = event.GetPosition();
            le.m_pointDrag.y -= GetSize().y;
            le.m_col = col_num;
            m_Owner->GetEventHandler()->ProcessEvent( le );
            //guLogMessage( wxT( "Col %i have been clicked %i" ), col_num,
            //                                      ( * m_Columns )[ col_num ].m_Width );
        }
    }
    else if( event.Moving() )
    {
        m_IsDragging = wxNOT_FOUND;
        if( hit_border >= 0 )
        {
            SetCursor( * m_ResizeCursor );
        }
        else
        {
            SetCursor( * wxSTANDARD_CURSOR );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guListViewHeader::OnSetFocus( wxFocusEvent &WXUNUSED(event) )
{
    if( m_ListViewClient )
    {
        m_ListViewClient->SetFocus();
        m_ListViewClient->Update();
    }
}

// -------------------------------------------------------------------------------- //
int guListViewHeader::RefreshWidth( void )
{
    // width and height of the entire header window
    int w, h;
    m_ListViewClient->GetClientSize( &w, &h );

    int index;
    int count = m_Columns->Count();
    if( count > 1 )
    {
        m_Width = 0;
        for( index = 0; index < count; index++ )
        {
            if( ( * m_Columns )[ index ].m_Enabled )
            {
                m_Width += ( * m_Columns )[ index ].m_Width;
            }
        }

        // Checkif its needed the horizontal scroll bar
        if( m_Width > w )
        {
            //guLogMessage( wxT( "Vertical ScrollBar... %i %i  %i   %i    %i" ), m_Width, w, m_Width - w,
            //   m_ListViewClient->GetScrollPos( wxHORIZONTAL ), ( h / m_ListViewClient->OnGetLineHeight( 0 ) ) );
            if( m_ListViewClient->GetScrollPos( wxHORIZONTAL ) > m_Width - w )
            {
                m_ListViewClient->SetScrollPos( wxHORIZONTAL, m_Width - w );
            }
            m_ListViewClient->SetHScrollbar( m_Width - w );
        }
        else
        {
            m_ListViewClient->SetHScrollbar( 0 );
        }
    }
    else
    {
        m_Width = w;
    }
    return m_Width;
}

// -------------------------------------------------------------------------------- //
void guListViewHeader::AdjustDC( wxDC &dc )
{
    int ScrollPos = m_ListViewClient->GetScrollPos( wxHORIZONTAL );
    int org_x = 0;
    int org_y = 0;
    dc.GetDeviceOrigin( &org_x, &org_y );

    dc.SetDeviceOrigin( org_x - ScrollPos, org_y );
}

// -------------------------------------------------------------------------------- //
void guListViewHeader::OnEditColumns( void )
{
    guListViewColEdit * ListViewColEdit = new guListViewColEdit( m_Owner, m_Columns );
    if( ListViewColEdit )
    {
        if( ListViewColEdit->ShowModal() == wxID_OK )
        {
            ListViewColEdit->UpdateColumns();
            RefreshWidth();
            m_Owner->Refresh();
        }
        ListViewColEdit->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guListViewHeader::SetImageList( wxImageList * imagelist )
{
    m_ImageList = imagelist;
}

// -------------------------------------------------------------------------------- //
// guListViewColEdit
// -------------------------------------------------------------------------------- //
guListViewColEdit::guListViewColEdit( wxWindow * parent, guListViewColumnArray * columns ) :
    wxDialog( parent, wxID_ANY, _( "Select Columns" ), wxDefaultPosition, wxSize( 246,340 ), wxDEFAULT_DIALOG_STYLE )
{
    m_Owner = ( guListView * ) parent;
    m_Columns = columns;
    m_SelItem = wxNOT_FOUND;

	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * ColumnsSizer;
	ColumnsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _(" Columns ") ), wxHORIZONTAL );

    int index;
    int count = columns->Count();
    for( index = 0; index < count; index++ )
    {
        m_ItemsText.Add( ( * columns )[ index ].m_Label );
        m_ItemsData.Add( ( * columns )[ index ].m_Id );
    }

	m_ColumnsListBox = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ItemsText, wxLB_SINGLE );
	ColumnsSizer->Add( m_ColumnsListBox, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer * BtnsSizer;
	BtnsSizer = new wxBoxSizer( wxVERTICAL );

	m_UpBitmapBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_UpBitmapBtn->Enable( false );
	BtnsSizer->Add( m_UpBitmapBtn, 0, wxTOP|wxBOTTOM, 5 );

	m_DownBitmapBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DownBitmapBtn->Enable( false );
	BtnsSizer->Add( m_DownBitmapBtn, 0, wxTOP|wxBOTTOM, 5 );

	ColumnsSizer->Add( BtnsSizer, 0, wxEXPAND, 5 );

    wxStdDialogButtonSizer * StdBtnSizer;
    wxButton * StdBtnSizerOK;
    wxButton * StdBtnSizerCancel;
	MainSizer->Add( ColumnsSizer, 1, wxEXPAND|wxALL, 5 );

	StdBtnSizer = new wxStdDialogButtonSizer();
	StdBtnSizerOK = new wxButton( this, wxID_OK );
	StdBtnSizer->AddButton( StdBtnSizerOK );
	StdBtnSizerCancel = new wxButton( this, wxID_CANCEL );
	StdBtnSizer->AddButton( StdBtnSizerCancel );
	StdBtnSizer->SetAffirmativeButton( StdBtnSizerOK );
	StdBtnSizer->SetCancelButton( StdBtnSizerCancel );
	StdBtnSizer->Realize();
	MainSizer->Add( StdBtnSizer, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	StdBtnSizerOK->SetDefault();

    count = columns->Count();
    for( index = 0; index < count; index++ )
    {
        if( ( * columns )[ index ].m_Enabled )
        {
            m_ColumnsListBox->Check( index );
        }
    }

    // Bind Events
    m_ColumnsListBox->Bind( wxEVT_LISTBOX, &guListViewColEdit::OnColumnSelected, this );
    m_UpBitmapBtn->Bind( wxEVT_BUTTON, &guListViewColEdit::OnUpBtnClick, this );
    m_DownBitmapBtn->Bind( wxEVT_BUTTON, &guListViewColEdit::OnDownBtnClick, this );

    m_ColumnsListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
guListViewColEdit::~guListViewColEdit()
{

    // Unbind Events
    m_ColumnsListBox->Unbind( wxEVT_LISTBOX, &guListViewColEdit::OnColumnSelected, this );
    m_UpBitmapBtn->Unbind( wxEVT_BUTTON, &guListViewColEdit::OnUpBtnClick, this );
    m_DownBitmapBtn->Unbind( wxEVT_BUTTON, &guListViewColEdit::OnDownBtnClick, this );
}

// -------------------------------------------------------------------------------- //
void guListViewColEdit::UpdateColumns( void )
{
    int ColId;
    int ColPos;
    int index;
    int count = m_ItemsData.Count();
    for( index = 0; index < count; index++ )
    {
        ColId = m_ItemsData[ index ];
        ColPos = FindColumnId( m_Columns, ColId );
        if( ColPos != index )
        {
            guListViewColumn * Column = m_Columns->Detach( ColPos );
            m_Columns->Insert( Column, index );
        }
        ( * m_Columns )[ index ].m_Enabled = m_ColumnsListBox->IsChecked( index );
    }
}

// -------------------------------------------------------------------------------- //
void guListViewColEdit::OnColumnSelected( wxCommandEvent &event )
{
    m_SelItem = event.GetInt();
    if( m_SelItem != wxNOT_FOUND )
    {
        m_UpBitmapBtn->Enable( m_SelItem > 0 );
        m_DownBitmapBtn->Enable( m_SelItem < ( int ) ( m_ItemsData.Count() - 1 ) );
    }
    else
    {
        m_UpBitmapBtn->Enable( false );
        m_DownBitmapBtn->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guListViewColEdit::OnUpBtnClick( wxCommandEvent &event )
{
    wxString CurLabel = m_ItemsText[ m_SelItem ];
    int      CurData = m_ItemsData[ m_SelItem ];
    bool     CurCheck = m_ColumnsListBox->IsChecked( m_SelItem );

    m_ColumnsListBox->SetString( m_SelItem, m_ColumnsListBox->GetString( m_SelItem - 1 ) );
    m_ColumnsListBox->Check( m_SelItem, m_ColumnsListBox->IsChecked( m_SelItem - 1 ) );
    m_ItemsText[ m_SelItem ] = m_ItemsText[ m_SelItem - 1 ];
    m_ItemsData[ m_SelItem ] = m_ItemsData[ m_SelItem - 1 ];
    m_SelItem--;
    m_ColumnsListBox->SetString( m_SelItem, CurLabel );
    if( CurCheck )
        m_ColumnsListBox->Check( m_SelItem );
    m_ItemsText[ m_SelItem ] = CurLabel;
    m_ItemsData[ m_SelItem ] = CurData;
    m_ColumnsListBox->SetSelection( m_SelItem );

    event.SetInt( m_SelItem );
    OnColumnSelected( event );
}

// -------------------------------------------------------------------------------- //
void guListViewColEdit::OnDownBtnClick( wxCommandEvent &event )
{
    wxString CurLabel = m_ItemsText[ m_SelItem ];
    int      CurData = m_ItemsData[ m_SelItem ];
    bool     CurCheck = m_ColumnsListBox->IsChecked( m_SelItem );

    m_ColumnsListBox->SetString( m_SelItem, m_ColumnsListBox->GetString( m_SelItem + 1 ) );
    m_ColumnsListBox->Check( m_SelItem, m_ColumnsListBox->IsChecked( m_SelItem + 1 ) );
    m_ItemsText[ m_SelItem ] = m_ItemsText[ m_SelItem + 1 ];
    m_ItemsData[ m_SelItem ] = m_ItemsData[ m_SelItem + 1 ];
    m_SelItem++;
    m_ColumnsListBox->SetString( m_SelItem, CurLabel );
    if( CurCheck )
        m_ColumnsListBox->Check( m_SelItem );
    m_ItemsText[ m_SelItem ] = CurLabel;
    m_ItemsData[ m_SelItem ] = CurData;
    m_ColumnsListBox->SetSelection( m_SelItem );

    event.SetInt( m_SelItem );
    OnColumnSelected( event );
}




// -------------------------------------------------------------------------------- //
// guListViewClientTimer
// -------------------------------------------------------------------------------- //
void guListViewClientTimer::Notify()
{
    int index;
    if( m_ListViewClient->m_SearchStr.Len() )
    {
        m_ListViewClient->SetSelection( -1 );
        index = m_ListViewClient->FindItem( 0, m_ListViewClient->m_SearchStr, true );
        m_ListViewClient->SetSelection( index );

        wxCommandEvent event( wxEVT_LISTBOX, m_ListViewClient->GetId() );
        event.SetEventObject( m_ListViewClient );
        event.SetInt( index );
        m_ListViewClient->ProcessEvent( event );

        m_ListViewClient->m_SearchStr = wxEmptyString;
    }
}




// -------------------------------------------------------------------------------- //
// guListViewDropFilesThread
// -------------------------------------------------------------------------------- //
guListViewDropFilesThread::guListViewDropFilesThread( guListViewDropTarget * playlistdroptarget,
                             guListView * listview, const wxArrayString &files ) :
    wxThread()
{
    m_ListView = listview;
    m_Files = files;
    m_ListViewDropTarget = playlistdroptarget;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guListViewDropFilesThread::~guListViewDropFilesThread()
{
//    printf( "guListViewDropFilesThread Object destroyed\n" );
    if( !TestDestroy() )
    {
        m_ListViewDropTarget->ClearPlayListFilesThread();
    }
}

// -------------------------------------------------------------------------------- //
void guListViewDropFilesThread::AddDropFiles( const wxString &dirname )
{
    wxDir Dir;
    wxString FileName;
    //wxString SavedDir( wxGetCwd() );

    if( wxDirExists( dirname ) )
    {
        wxString DirName = dirname;
        if( !DirName.EndsWith( wxT( "/" ) ) )
            DirName += wxT( "/" );

        Dir.Open( DirName );
        if( Dir.IsOpened() )
        {
            if( !TestDestroy() && Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
            {
                do {
                    if( ( FileName[ 0 ] != '.' ) )
                    {
                        if( Dir.Exists( DirName + FileName ) )
                        {
                            AddDropFiles( DirName + FileName );
                        }
                        else
                        {
                            m_ListView->OnDropFile( DirName + FileName );
                        }
                    }
                } while( Dir.GetNext( &FileName ) && !TestDestroy() );
            }
        }
    }
    else
    {
        m_ListView->OnDropFile( dirname );
    }
}

// -------------------------------------------------------------------------------- //
guListViewDropFilesThread::ExitCode guListViewDropFilesThread::Entry()
{
    int index;
    int Count = m_Files.Count();
    for( index = 0; index < Count; ++index )
    {
        if( TestDestroy() )
            return 0;
        AddDropFiles( m_Files[ index ] );
    }

    m_ListView->OnDropEnd();
    //
    m_ListView->m_DragOverItem = wxNOT_FOUND;

    return 0;
}

// -------------------------------------------------------------------------------- //
// guListViewDropTarget
// -------------------------------------------------------------------------------- //
guListViewDropTarget::guListViewDropTarget( guListView * listview ) : wxDropTarget()
{
    m_ListView = listview;
    m_ListViewDropFilesThread = NULL;

    wxDataObjectComposite * DataObject = new wxDataObjectComposite();
    wxCustomDataObject * TracksDataObject = new wxCustomDataObject( wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) );
    DataObject->Add( TracksDataObject, true );
    wxFileDataObject * FileDataObject = new wxFileDataObject();
    DataObject->Add( FileDataObject, false );
    SetDataObject( DataObject );
}

// -------------------------------------------------------------------------------- //
guListViewDropTarget::~guListViewDropTarget()
{
//    printf( "guListViewDropTarget Object destroyed\n" );
}

// -------------------------------------------------------------------------------- //
wxDragResult guListViewDropTarget::OnData( wxCoord x, wxCoord y, wxDragResult def )
{
    //guLogMessage( wxT( "guListViewDropTarget::OnData" ) );

    if( def == wxDragError || def == wxDragNone || def == wxDragCancel )
        return def;

    if( m_ListView->m_DragSelfItems )
    {
        return wxDragNone;
    }

    if( !GetData() )
    {
        guLogMessage( wxT( "Error getting drop data" ) );
        return wxDragError;
    }

    guDataObjectComposite * DataObject = ( guDataObjectComposite * ) m_dataObject;

    wxDataFormat ReceivedFormat = DataObject->GetReceivedFormat();
    //guLogMessage( wxT( "ReceivedFormat: '%s'" ), ReceivedFormat.GetId().c_str() );
    if( ReceivedFormat == wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) )
    {
        guTrackArray * Tracks;
        if( !DataObject->GetDataHere( ReceivedFormat, &Tracks ) )
        {
          guLogMessage( wxT( "Error getting tracks data..." ) );
        }
        else
        {
            m_ListView->OnDropTracks( Tracks );
            m_ListView->OnDropEnd();
        }
    }
    else if( ReceivedFormat == wxDataFormat( wxDF_FILENAME ) )
    {
        m_DropFilesThreadMutex.Lock();
        if( m_ListViewDropFilesThread )
        {
            m_ListViewDropFilesThread->Pause();
            m_ListViewDropFilesThread->Delete();
            m_ListViewDropFilesThread = NULL;
        }
        wxFileDataObject * FileDataObject = ( wxFileDataObject * ) DataObject->GetDataObject( wxDataFormat( wxDF_FILENAME ) );
        if( FileDataObject )
        {
            m_ListViewDropFilesThread = new guListViewDropFilesThread( this, m_ListView, FileDataObject->GetFilenames() );
            if( !m_ListViewDropFilesThread )
            {
                guLogError( wxT( "Could not create the add files thread." ) );
            }
        }
        m_DropFilesThreadMutex.Unlock();
    }

    return def;
}

// -------------------------------------------------------------------------------- //
bool guListViewDropTarget::OnDrop( wxCoord x, wxCoord y )
{
    //guLogMessage( wxT( "OnDrop %i / %i" ), m_ListView->m_DragSelfItems, m_ListView->m_DragSelfItemsEnabled );
    // We are moving items inside this object.
    if( m_ListView->m_DragSelfItems )
    {
        if( m_ListView->m_DragSelfItemsEnabled )
        {
            m_ListView->MoveSelection();
        }
        m_ListView->RefreshAll();
    }
    else
    {
        m_ListView->OnDropBegin();
    }
    return true;
}

// -------------------------------------------------------------------------------- //
void guListViewDropTarget::OnLeave()
{
    int MouseX, MouseY;
    wxGetMousePosition( &MouseX, &MouseY );
    //m_ListView->ScreenToClient( &MouseX, &MouseY );
    wxRect ScreenRect = m_ListView->GetClientScreenRect();
    //guLogMessage( wxT( "guListViewDropTarget::OnLeave  %i, %i -> ( %i, %i, %i, %i )" ), MouseX, MouseY, ScreenRect.x, ScreenRect.y, ScreenRect.x + ScreenRect.width, ScreenRect.y + ScreenRect.height  );
    if( !ScreenRect.Contains( MouseX, MouseY ) ) //( m_ListView->HitTest( MouseX, MouseY ) == wxNOT_FOUND ) )
    {
        m_ListView->m_DragOverItem = wxNOT_FOUND;
        //m_ListView->m_DragSelfItems = false;
        m_ListView->RefreshAll();
    }
}

// -------------------------------------------------------------------------------- //
wxDragResult guListViewDropTarget::OnDragOver( wxCoord x, wxCoord y, wxDragResult def )
{
    //printf( "guListViewDropTarget::OnDragOver... %d - %d\n", x, y );
    m_ListView->OnDragOver( x, y );
    return wxDragCopy;
}




// -------------------------------------------------------------------------------- //
// guDataObjectComposite
// -------------------------------------------------------------------------------- //
void guDataObjectComposite::SetFiles( const wxArrayString &files )
{
    wxFileDataObject * FileDataObject = new wxFileDataObject();

    int Index;
    int Count = files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        FileDataObject->AddFile( files[ Index ] );
    }
    Add( FileDataObject );
}

// -------------------------------------------------------------------------------- //
void guDataObjectComposite::SetTracks( const guTrackArray &tracks )
{
    wxCustomDataObject * TracksObject = new wxCustomDataObject( wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) );
    m_Tracks = new guTrackArray();

    int Index;
    int Count = tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_Tracks->Add( new guTrack( tracks[ Index ] ) );
    }
    TracksObject->SetData( sizeof( m_Tracks ), &m_Tracks );
    Add( TracksObject );
}

}

// -------------------------------------------------------------------------------- //
