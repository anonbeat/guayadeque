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
#include "ListView.h"

#include "Commands.h"
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


#define guLISTVIEW_ITEM_MARGIN      2

#define guLISTVIEW_TIMER_TIMEOUT    500
#define guLISTVIEW_MIN_COL_SIZE     20

WX_DEFINE_OBJARRAY(guListViewColumnArray);


// -------------------------------------------------------------------------------- //
// guListViewHeader
// -------------------------------------------------------------------------------- //
class guListViewHeader : public wxWindow
{
  protected:
    guListView *            m_Owner;
    guListViewClient *      m_ListViewClient;
    wxImageList *           m_ImageList;
    guListViewColumnArray * m_Columns;
    int                     m_Width;
    wxCursor *              m_ResizeCursor;
    int                     m_IsDragging;
    int                     m_DragOfset;

    void OnMouse( wxMouseEvent &event );
    void OnPaint( wxPaintEvent &event );
    void OnSetFocus( wxFocusEvent &event );
    void AdjustDC( wxDC &dc );
    void OnEditColumns( void );

  public:
    guListViewHeader();

    guListViewHeader( wxWindow * parent,
                        guListViewClient * owner,
                        guListViewColumnArray * columns,
                        const wxPoint &pos = wxDefaultPosition,
                        const wxSize &size = wxDefaultSize );

    virtual ~guListViewHeader();

    int     RefreshWidth( void );
    void    SetImageList( wxImageList * imagelist );

private:

    DECLARE_EVENT_TABLE()
};

class guListViewClientTimer;




// -------------------------------------------------------------------------------- //
// guListViewClient
// -------------------------------------------------------------------------------- //
class guListViewClient : public wxVListBox
{
  private :
    guListView *                m_Owner;
    guListViewHeader *          m_Header;
    guListViewColumnArray *     m_Columns;
    guListViewClientTimer *     m_SearchStrTimer;
    wxString                    m_SearchStr;
    guListViewAttr *            m_Attr;
    int                         m_ItemHeight;
    int                         m_HScrollPos;

    wxPoint                     m_DragStart;
    int                         m_DragCount;

    void            OnPaint( wxPaintEvent &event );
    void            AdjustDC( wxDC &dc );
    void            OnDragOver( const wxCoord x, const wxCoord y );
    void            OnKeyDown( wxKeyEvent &event );
    void            OnMouse( wxMouseEvent &event );
    void            OnContextMenu( wxContextMenuEvent& event );
    long            FindItem( long start, const wxString& str, bool partial );

    void            OnSearchLinkClicked( wxCommandEvent &event );
    void            OnCommandClicked( wxCommandEvent &event );
    //wxString        GetSearchText( int Item );

    void            OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const;
    void            DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    void            DoDrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    void            OnDrawBackground( wxDC &dc, const wxRect &rect, size_t n ) const;
    void            DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    void            DoDrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    wxCoord         OnMeasureItem( size_t n ) const;
    virtual wxString GetItemSearchText( const int row );
    void            OnHScroll( wxScrollWinEvent &event );

    DECLARE_EVENT_TABLE()

  public :
    guListViewClient( wxWindow * parent, const int flags, guListViewColumnArray * columns, guListViewAttr * attr );
    ~guListViewClient();

    void SetItemHeigth( const int height );

    void SetHScrollbar( const int width );
    virtual void SetScrollPos( int orientation, int pos, bool refresh = true );
    //int  GetHScrollPosition( void );


    friend class guListView;
    friend class guListViewHeader;
    friend class guListViewClientTimer;
};




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
    guListViewClientTimer( guListViewClient * listviewclient ) { m_ListViewClient = listviewclient; }

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
    m_Header = new guListViewHeader( this, m_ListBox, m_Columns, wxPoint( 0, 0 ) );
    m_ColSelect = ( flags & guLISTVIEW_COLUMN_SELECT );

    parent->Connect( wxEVT_SIZE, wxSizeEventHandler( guListView::OnChangedSize ), NULL, this );
	m_ListBox->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guListView::OnKeyDown ), NULL, this );
    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guListView::OnContextMenu ), NULL, this );
    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guListView::OnBeginDrag ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guListView::~guListView()
{
    if( m_Columns )
        delete m_Columns;

    if( m_ImageList )
        delete m_ImageList;

    GetParent()->Disconnect( wxEVT_SIZE, wxSizeEventHandler( guListView::OnChangedSize ), NULL, this );
	m_ListBox->Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guListView::OnKeyDown ), NULL, this );
    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guListView::OnContextMenu ), NULL, this );
    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guListView::OnBeginDrag ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guListView::InsertColumn( guListViewColumn * column )
{
    m_Columns->Add( column );
    m_Header->RefreshWidth();
}

// -------------------------------------------------------------------------------- //
void guListView::OnChangedSize( wxSizeEvent &event )
{
    int w, h, d;
    wxSize Size = event.GetSize();
    Size.x -= 6;
    Size.y -= 6;
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
wxString guListView::OnGetItemText( const int row, const int col ) const
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guListView::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( row == wxNOT_FOUND )
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
void guListView::CreateContextMenu( wxMenu * menu ) const
{
}

// -------------------------------------------------------------------------------- //
void guListView::OnContextMenu( wxContextMenuEvent& event )
{
    wxMenu Menu;
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
    CreateContextMenu( &Menu );
    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
int guListView::GetItemCount( void ) const
{
    return m_ListBox->GetItemCount();
}

// -------------------------------------------------------------------------------- //
void guListView::SetItemCount( const int count )
{
    m_ListBox->SetItemCount( count );
    m_Header->RefreshWidth();
}

// -------------------------------------------------------------------------------- //
void guListView::SetItemHeight( const int height )
{
    m_ListBox->SetItemHeigth( height );
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
int guListView::GetFirstSelected( unsigned long &cookie ) const
{
    return m_ListBox->GetFirstSelected( cookie );
}

// -------------------------------------------------------------------------------- //
int guListView::GetNextSelected( unsigned long &cookie ) const
{
    return m_ListBox->GetNextSelected( cookie );
}

// -------------------------------------------------------------------------------- //
bool guListView::Select( size_t item, bool select )
{
    return m_ListBox->Select( item, select );
}

// -------------------------------------------------------------------------------- //
void guListView::SetSelection( int selection )
{
    m_ListBox->SetSelection( selection );

    wxCommandEvent event( wxEVT_COMMAND_LISTBOX_SELECTED, m_ListBox->GetId() );
    event.SetEventObject( m_ListBox );
    event.SetInt( selection );
    (void) GetEventHandler()->ProcessEvent( event );
}

// -------------------------------------------------------------------------------- //
size_t guListView::GetFirstVisibleLine() const
{
    return m_ListBox->GetFirstVisibleLine();
}

// -------------------------------------------------------------------------------- //
size_t guListView::GetLastVisibleLine() const
{
    return m_ListBox->GetLastVisibleLine();
}

// -------------------------------------------------------------------------------- //
bool guListView::ScrollLines( int lines )
{
    return m_ListBox->ScrollLines( lines );
}

// -------------------------------------------------------------------------------- //
bool guListView::ScrollToLine( size_t line )
{
    return m_ListBox->ScrollToLine( line );
}

// -------------------------------------------------------------------------------- //
void guListView::RefreshAll( int scroll )
{
    if( scroll != wxNOT_FOUND )
    {
        if( !m_ListBox->IsVisible( scroll ) )
            m_ListBox->ScrollToLine( scroll );
    }
    m_ListBox->RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guListView::RefreshLines( const int from, const int to )
{
    m_ListBox->RefreshLines( from, to );
}

// -------------------------------------------------------------------------------- //
bool guListView::IsSelected( size_t row ) const
{
    return m_ListBox->IsSelected( row );
}

// -------------------------------------------------------------------------------- //
void guListView::OnBeginDrag( wxMouseEvent &event )
{
    guTrackArray Songs;
    wxFileDataObject Files; // = wxFileDataObject();
    int index;
    int count;
    // TODO : Virtualice the function to add the files to the FileDataObject
    if( ( count = GetSelectedSongs( &Songs ) ) )
    {
        //count = Songs.Count();
        for( index = 0; index < count; index++ )
        {
          Files.AddFile( Songs[ index ].m_FileName );
          guLogMessage( wxT( "Added file %s" ), Songs[ index ].m_FileName.c_str() );
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
int guListView::GetSelectedSongs( guTrackArray * tracks ) const
{
    return 0;
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
void guListView::SetSelectedItems( const wxArrayInt &selection )
{
    int index;
    int count = GetItemCount();

    ClearSelectedItems();
    if( selection.Count() )
    {
        bool IsMultiple = m_ListBox->HasMultipleSelection();
        for( index = 0; index < count; index++ )
        {
            if( selection.Index( GetItemId( index ) ) != wxNOT_FOUND )
            {
                if( IsMultiple )
                    Select( index );
                else
                    SetSelection( index );
            }
        }
        wxCommandEvent event( wxEVT_COMMAND_LISTBOX_SELECTED, m_ListBox->GetId() );
        event.SetEventObject( m_ListBox );
        event.SetInt( selection[ 0 ] );
        (void) GetEventHandler()->ProcessEvent( event );
    }
}

// -------------------------------------------------------------------------------- //
void guListView::ClearSelectedItems()
{
    SetSelection( wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
long guListView::FindItem( long start, const wxString &str, bool partial )
{
    return m_ListBox->FindItem( start, str, partial );
}

// -------------------------------------------------------------------------------- //
void guListView::SetColumnWidth( const int col, const int width )
{
    ( * m_Columns )[ col ].m_Width = width;
    m_Header->RefreshWidth();
}

// -------------------------------------------------------------------------------- //
int guListView::GetColumnWidth( const int col ) const
{
    return m_Columns->Item( col ).m_Width;
}

// -------------------------------------------------------------------------------- //
bool guListView::IsAllowedColumnSelect( void ) const
{
    return m_ColSelect;
}

// -------------------------------------------------------------------------------- //
wxString guListView::GetColumnLabel( const int col ) const
{
    return m_Columns->Item( col ).m_Label;
}

// -------------------------------------------------------------------------------- //
int guListView::GetColumnId( const int col ) const
{
    return m_Columns->Item( col ).m_Id;
}

// -------------------------------------------------------------------------------- //
void  guListView::SetColumnLabel( const int col, const wxString &label )
{
    m_Columns->Item( col ).m_Label = label;
    m_Header->Refresh();
}

// -------------------------------------------------------------------------------- //
void guListView::SetColumnImage( const int col, const int imageindex )
{
    m_Columns->Item( col ).m_ImageIndex = imageindex;
    m_Header->Refresh();
}

// -------------------------------------------------------------------------------- //
void guListView::OnKeyDown( wxKeyEvent &event )
{
    event.Skip();
}

// -------------------------------------------------------------------------------- //
int guListView::HitTest( wxCoord x, wxCoord y ) const
{
    return m_ListBox->HitTest( x, y );
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
    wxVListBox( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags )
{
    m_Owner = ( guListView * ) parent;
    m_SearchStrTimer = new guListViewClientTimer( this );
    m_Attr = attr;
    m_Columns = columns;
    m_SearchStr = wxEmptyString;
    m_ItemHeight = wxNOT_FOUND;
    m_HScrollPos = 0;

    SetBackgroundColour( m_Attr->m_EveBgColor );

    Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guListViewClient::OnKeyDown ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guListViewClient::~guListViewClient()
{
    if( m_SearchStrTimer )
      delete m_SearchStrTimer;

    Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guListViewClient::OnKeyDown ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::OnKeyDown( wxKeyEvent &event )
{
    wxChar KeyChar = event.GetKeyCode();

    //printf( "KeyPressed %i\n", KeyChar );
    //if( wxIsalnum( KeyChar ) )
    if( ( KeyChar >= 'a' && KeyChar <= 'z' ) ||
        ( KeyChar >= 'A' && KeyChar <= 'Z' ) ||
        ( KeyChar >= '0' && KeyChar <= '9' ) )
    {
        if( m_SearchStrTimer->IsRunning() )
        {
            m_SearchStrTimer->Stop();
        }
        m_SearchStrTimer->Start( guLISTVIEW_TIMER_TIMEOUT, wxTIMER_ONE_SHOT );
        m_SearchStr.Append( KeyChar );
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
    dc.SetBackground(GetBackgroundColour());
    dc.Clear();

    AdjustDC( dc );

    // the bounding rectangle of the current line
    wxRect rectLine;
    rectLine.width = clientSize.x;

    // iterate over all visible lines
    const size_t lineMax = GetVisibleEnd();
    for( size_t line = GetFirstVisibleLine(); line < lineMax; line++ )
    {
        const wxCoord hLine = OnGetLineHeight(line);

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
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DoDrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    dc.SetFont( * m_Attr->m_Font );
    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.SetTextForeground( IsSelected( row ) ? m_Attr->m_SelFgColor : m_Attr->m_TextFgColor );
    dc.DrawText( ( ( guListView * ) GetParent() )->OnGetItemText( row, col ),
                  rect.x + guLISTVIEW_ITEM_MARGIN, rect.y + guLISTVIEW_ITEM_MARGIN );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    ( ( guListView * ) GetParent() )->DrawItem( dc, rect, row, col );
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
wxCoord guListViewClient::OnMeasureItem( size_t n ) const
{
    if( m_ItemHeight != wxNOT_FOUND )
    {
        return m_ItemHeight;
    }
    else
    {
        return ( ( guListView * ) GetParent() )->OnMeasureItem( n );
    }
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DoDrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( IsSelected( row ) )
      dc.SetBrush( wxBrush( m_Attr->m_SelBgColor ) );
    else
      dc.SetBrush( wxBrush( row & 1 ? m_Attr->m_OddBgColor : m_Attr->m_EveBgColor ) );

    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.DrawRectangle( rect );
}

// -------------------------------------------------------------------------------- //
void guListViewClient::DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    ( ( guListView * ) GetParent() )->DrawBackground( dc, rect, row, col );
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
void guListViewClient::OnMouse( wxMouseEvent &event )
{
    if( event.Dragging() )
    {
        if( !m_DragCount )
        {
            m_DragStart = event.GetPosition();
        }

        if( ++m_DragCount == 3 )
        {
            wxListEvent le( wxEVT_COMMAND_LIST_BEGIN_DRAG, GetId() );
            le.SetEventObject( this );
            //le.m_itemIndex = m_lineLastClicked;
            le.m_pointDrag = m_DragStart;
            GetEventHandler()->ProcessEvent( le );
        }
        return;
    }
    else
      m_DragCount = 0;

    event.Skip();
}

// -------------------------------------------------------------------------------- //
wxString guListViewClient::GetItemSearchText( const int row )
{
    return ( ( guListView * ) GetParent() )->GetItemName( row );
}

// -------------------------------------------------------------------------------- //
long guListViewClient::FindItem( long start, const wxString& str, bool partial )
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
        else
        {
            if( line_upper.find( str_upper ) == 0 )
                return index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guListViewClient::SetItemHeigth( const int height )
{
    m_ItemHeight = height;
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
        SetScrollbar( wxHORIZONTAL, m_HScrollPos, 1, width );
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
    EVT_PAINT         (guListViewHeader::OnPaint)
    EVT_MOUSE_EVENTS  (guListViewHeader::OnMouse)
    EVT_SET_FOCUS     (guListViewHeader::OnSetFocus)
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
                cRect.width -= 18;
            dc.SetClippingRegion( cRect );
            dc.DrawText( CurCol->m_Label, cRect.x + 2, hLabel );
            dc.DestroyClippingRegion();

            if( CurCol->m_ImageIndex >= 0 )
            {
                dc.DrawBitmap( m_ImageList->GetBitmap( CurCol->m_ImageIndex ), cRect.x + cRect.width + 1, (cRect.height/2)-8, true );
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
            wxListEvent le( event.LeftDown() ? wxEVT_COMMAND_LIST_COL_CLICK :
                                               wxEVT_COMMAND_LIST_COL_RIGHT_CLICK, m_Owner->GetId() );
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
// guListViewClientTimer
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
	ColumnsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT(" Columns ") ), wxHORIZONTAL );

    int index;
    int count = columns->Count();
    for( index = 0; index < count; index++ )
    {
        m_ItemsText.Add( ( * columns )[ index ].m_Label );
        m_ItemsData.Add( ( * columns )[ index ].m_Id );
    }

	m_ColumnsListBox = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ItemsText, wxLB_SINGLE );
	ColumnsSizer->Add( m_ColumnsListBox, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer * BtnsSizer;
	BtnsSizer = new wxBoxSizer( wxVERTICAL );

	m_UpBitmapBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_UpBitmapBtn->Enable( false );
	BtnsSizer->Add( m_UpBitmapBtn, 0, wxTOP|wxBOTTOM, 5 );

	m_DownBitmapBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DownBitmapBtn->Enable( false );
	BtnsSizer->Add( m_DownBitmapBtn, 0, wxTOP|wxBOTTOM, 5 );

	ColumnsSizer->Add( BtnsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

    wxStdDialogButtonSizer * StdBtnSizer;
    wxButton * StdBtnSizerOK;
    wxButton * StdBtnSizerCancel;
	MainSizer->Add( ColumnsSizer, 1, wxEXPAND|wxALL, 5 );

	StdBtnSizer = new wxStdDialogButtonSizer();
	StdBtnSizerOK = new wxButton( this, wxID_OK );
	StdBtnSizer->AddButton( StdBtnSizerOK );
	StdBtnSizerCancel = new wxButton( this, wxID_CANCEL );
	StdBtnSizer->AddButton( StdBtnSizerCancel );
	StdBtnSizer->Realize();
	MainSizer->Add( StdBtnSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();


    count = columns->Count();
    for( index = 0; index < count; index++ )
    {
        if( ( * columns )[ index ].m_Enabled )
        {
            m_ColumnsListBox->Check( index );
        }
    }

	// Connect Events
	m_ColumnsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guListViewColEdit::OnColumnSelected ), NULL, this );
	m_UpBitmapBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guListViewColEdit::OnUpBtnClick ), NULL, this );
	m_DownBitmapBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guListViewColEdit::OnDownBtnClick ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guListViewColEdit::~guListViewColEdit()
{

	// Disconnect Events
	m_ColumnsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guListViewColEdit::OnColumnSelected ), NULL, this );
	m_UpBitmapBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guListViewColEdit::OnUpBtnClick ), NULL, this );
	m_DownBitmapBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guListViewColEdit::OnDownBtnClick ), NULL, this );
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
        m_DownBitmapBtn->Enable( m_SelItem < ( m_ItemsData.Count() - 1 ) );
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

        wxCommandEvent event( wxEVT_COMMAND_LISTBOX_SELECTED, m_ListViewClient->GetId() );
        event.SetEventObject( m_ListViewClient );
        event.SetInt( index );
        m_ListViewClient->ProcessEvent( event );

        m_ListViewClient->m_SearchStr = wxEmptyString;
    }
}

// -------------------------------------------------------------------------------- //
