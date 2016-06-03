// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef GULISTVIEW_H
#define GULISTVIEW_H

#include "DbLibrary.h"
#include "Utils.h"

#include <wx/dnd.h>
#include <wx/scrolwin.h>
#include <wx/vlbox.h>
#include <wx/settings.h>
#include <wx/listbase.h>

namespace Guayadeque {

class guListViewClient;
class guListViewHeader;

// Own style flags
#define guLISTVIEW_COLUMN_SELECT        0x01000
#define guLISTVIEW_COLUMN_SORTING       0x02000
#define guLISTVIEW_ALLOWDRAG            0x04000
#define guLISTVIEW_ALLOWDROP            0x08000
#define guLISTVIEW_DRAGSELFITEMS        0x10000
#define guLISTVIEW_COLUMN_CLICK_EVENTS  0x20000
#define guLISTVIEW_HIDE_HEADER          0x40000

DECLARE_EVENT_TYPE( guEVT_LISTBOX_ITEM_COL_CLICKED, wxID_ANY )
#define EVT_LISTBOX_ITEM_COL_CLICKED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( guEVT_LISTBOX_ITEM_COL_CLICKED, winid, wxID_ANY, wxListEventHandler(fn), (wxObject *) NULL ),
DECLARE_EVENT_TYPE( guEVT_LISTBOX_ITEM_COL_RCLICKED, wxID_ANY )
#define EVT_LISTBOX_ITEM_COL_RCLICKED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( guEVT_LISTBOX_ITEM_COL_RCLICKED, winid, wxID_ANY, wxListEventHandler(fn), (wxObject *) NULL ),

extern wxColour wxAuiStepColour( const wxColour & c, int percent );

// -------------------------------------------------------------------------------- //
class guDataObjectComposite : public wxDataObjectComposite
{
  protected :
    guTrackArray *      m_Tracks;

  public :
    guDataObjectComposite() : wxDataObjectComposite() { m_Tracks = NULL; }
    ~guDataObjectComposite() { if( m_Tracks ) delete m_Tracks; }

    void                SetFiles( const wxArrayString &files );
    void                SetTracks( const guTrackArray &tracks );

    wxDataObjectSimple * GetDataObject( const wxDataFormat &format ) const { return GetObject( format ); }
};

// -------------------------------------------------------------------------------- //
// guListViewColumn
// -------------------------------------------------------------------------------- //
class guListViewColumn
{
  public :
    wxString m_Label;
    int      m_Id;
    int      m_Width;
    int      m_ImageIndex;
    bool     m_Enabled;

    guListViewColumn() {};
    ~guListViewColumn() {};
    guListViewColumn( const wxString &label, const int id, const int width = -1, const bool enabled = true )
    {
        m_Label = label;
        m_Id = id;
        m_Width = width;
        m_Enabled = enabled;
        m_ImageIndex = wxNOT_FOUND;
    };

};
WX_DECLARE_OBJARRAY(guListViewColumn, guListViewColumnArray);

// -------------------------------------------------------------------------------- //
// guListViewAttr
// -------------------------------------------------------------------------------- //
class guListViewAttr
{
  public:
    wxColour     m_SelBgColor;
    wxColour     m_SelFgColor;
    wxColour     m_EveBgColor;
    wxColour     m_OddBgColor;
    wxColour     m_TextFgColor;
    //wxColour     m_PlayFgColor;
    wxBrush     m_DragBgColor;
    wxFont *    m_Font;

    guListViewAttr() { LoadSysColors(); };

    ~guListViewAttr()
    {
        if( m_Font )
            delete m_Font;
    };

    guListViewAttr( const wxColour &selbg, const wxColour &selfg,
                    const wxColour &evebg, const wxColour &oddbg,
                    const wxColour &textfg, wxFont * font )
    {
        m_SelBgColor = selbg;
        m_SelFgColor = selfg;
        m_EveBgColor = evebg;
        m_OddBgColor = oddbg;
        m_TextFgColor = textfg;
        m_Font = font;
    };

    void LoadSysColors( void )
    {
        m_SelBgColor  = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );
        //guLogMessage( wxT( "SelBgColor: %s" ), m_SelBgColor.GetAsString( wxC2S_HTML_SYNTAX ).c_str() );
        m_SelFgColor  = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT );
        m_EveBgColor  = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );
        m_OddBgColor  = wxAuiStepColour( m_EveBgColor, m_EveBgColor.Red() + m_EveBgColor.Green() + m_EveBgColor.Blue() > 256 ? 97 : 103 );

        //m_TextFgColor.Set( m_EveBgColor.Red() ^ 0xFF, m_EveBgColor.Green() ^ 0xFF, m_EveBgColor.Blue() ^ 0xFF );
        m_TextFgColor = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOXTEXT );

        //m_PlayFgColor  = m_SelBgColor;
        m_DragBgColor  = * wxGREY_BRUSH;

        m_Font = new wxFont( wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT ) );
    }

};

class guListViewDropTarget;
class guListViewDropFilesThread;
class guListView;


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
    void OnCaptureLost( wxMouseCaptureLostEvent &event );

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
    bool                        m_MouseWasLeftUp;
    bool                        m_MouseSelecting;
    bool                        m_ColumnClickEvents;

    void            OnPaint( wxPaintEvent &event );
    void            AdjustDC( wxDC &dc );
    void            OnDragOver( const wxCoord x, const wxCoord y );
    void            OnKeyDown( wxKeyEvent &event );
    void            OnMouse( wxMouseEvent &event );
    void            OnContextMenu( wxContextMenuEvent& event );
    long            FindItem( long start, const wxString &str, bool partial, bool atstart = true );

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

    void            SetItemHeigth( const int height );

    void            SetHScrollbar( const int width );
    virtual void    SetScrollPos( int orientation, int pos, bool refresh = true );
    //int  GetHScrollPosition( void );

    friend class guListView;
    friend class guListViewHeader;
    friend class guListViewClientTimer;
};

// -------------------------------------------------------------------------------- //
class guListView : public wxScrolledWindow
{
  protected :
    guListViewClient *      m_ListBox;
    guListViewHeader *      m_Header;

    guListViewAttr          m_Attr;
    bool                    m_ColSelect;
    guListViewColumnArray * m_Columns;
    wxImageList *           m_ImageList;

    wxPoint                 m_DragStart;
    int                     m_DragCount;

    bool                    m_AllowDrop;
    bool                    m_AllowDrag;
    int                     m_DragOverItem;
    int                     m_LastDragOverItem;
    bool                    m_DragOverAfter;
    bool                    m_LastDragOverAfter;
    bool                    m_DragSelfItemsEnabled;
    bool                    m_DragSelfItems;

    //bool                    m_WasLeftUp;

    virtual void        OnKeyDown( wxKeyEvent &event ) { event.Skip(); }

    virtual void        GetItemsList( void ) = 0;
    virtual void        DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void        DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual wxString    OnGetItemText( const int row, const int column ) const { return wxEmptyString; }
    virtual void        CreateContextMenu( wxMenu * menu ) const {}
    virtual wxCoord     OnMeasureItem( size_t row ) const;
    virtual wxString    GetItemSearchText( const int row ) { return GetItemName( row ); }

    virtual void        OnBeginDrag( wxMouseEvent &event );
    virtual void        OnDragOver( const wxCoord x, const wxCoord y );
    virtual void        OnDropBegin( void ) {}
    virtual void        OnDropFile( const wxString &filename ) {}
    virtual void        OnDropTracks( const guTrackArray * tracks ) {}
    virtual void        OnDropEnd( void ) {}
    virtual int         GetDragFiles( guDataObjectComposite * files );

    virtual void        MoveSelection( void ) {}
    //virtual void        OnSysColorChanged( wxSysColourChangedEvent &event );
    virtual void        OnMouse( wxMouseEvent &event );

    void                OnChangedSize( wxSizeEvent &event );
    void                OnContextMenu( wxContextMenuEvent &event );
    void                OnHScroll( wxScrollWinEvent &event );

    virtual void        ItemsLock() {};
    virtual void        ItemsUnlock() {};
    virtual void        ItemsCheckRange( const int start, const int end ) {};

  public :
    guListView( wxWindow * parent, const int flags = wxLB_MULTIPLE, wxWindowID id = wxID_ANY,
                const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                long style = wxHSCROLL|wxVSCROLL|wxNO_BORDER );
    ~guListView();

    void                    SetItemCount( const int count );
    int                     GetItemCount( void ) const { return m_ListBox->GetItemCount(); }
    void                    SetItemHeight( const int height ) { m_ListBox->SetItemHeigth( height ); }
    void                    SetAttr( const guListViewAttr &attr ) { m_Attr = attr; };


    int                     GetFirstSelected( unsigned long &cookie ) const { return m_ListBox->GetFirstSelected( cookie ); }
    int                     GetNextSelected( unsigned long &cookie ) const { return m_ListBox->GetNextSelected( cookie ); }
    int                     GetSelection() const { return m_ListBox->GetSelection(); }
    bool                    Select( size_t item, bool select = true ) { return m_ListBox->Select( item, select ); }

    void                    SetSelection( int selection );

    size_t                  GetVisibleBegin( void ) const { return m_ListBox->GetVisibleBegin(); }
    size_t                  GetVisibleEnd( void ) const { return m_ListBox->GetVisibleEnd(); }
    size_t                  GetVisibleRowsBegin() const { return m_ListBox->GetVisibleRowsBegin(); }
    size_t                  GetVisibleRowsEnd() const { return m_ListBox->GetVisibleRowsEnd(); }
    bool                    ScrollLines( int lines ) { return m_ListBox->wxWindow::ScrollLines( lines ); }

    bool                    ScrollToRow( size_t line ) { return m_ListBox->ScrollToRow( line ); }

    void                    RefreshAll( int scroll = wxNOT_FOUND );
    void                    RefreshRows( const int from, const int to ) { m_ListBox->RefreshRows( from, to ); }
    void                    RefreshRow( const int line ) { m_ListBox->RefreshRow( line ); }
    bool                    IsSelected( size_t row ) const { return m_ListBox->IsSelected( row ); }
    virtual int             GetSelectedSongs( guTrackArray * Songs, const bool isdrag = false ) const { return 0; }

    int                     HitTest( wxCoord x, wxCoord y ) const { return m_ListBox->HitTest( x, y ); }

    virtual void            ReloadItems( bool reset = true ) = 0;

    virtual wxArrayInt      GetSelectedItems( bool reallist = true ) const;
    virtual wxArrayInt      GetSelectedIndexs( bool reallist = true ) const;
    virtual void            GetSelectedItems( guListItems * items, bool convertall = true ) const;
    virtual void            SetSelectedItems( const wxArrayInt &selection );
    virtual void            SetSelectedIndexs( const wxArrayInt &selection );
    virtual size_t          GetSelectedCount( void ) const { return m_ListBox->GetSelectedCount(); }
    virtual void            ClearSelectedItems( void ) { SetSelection( wxNOT_FOUND ); }


    virtual wxString        GetItemName( const int item ) const = 0;
    virtual int             GetItemId( const int item ) const = 0;
    long                    FindItem( long start, const wxString &str, bool partial, bool atstart = true ) { return m_ListBox->FindItem( start, str, partial, atstart ); }

    void                    SetImageList( wxImageList * imagelist );

    void                    InsertColumn( guListViewColumn * column );
    void                    SetColumnWidth( const int col, const int width );
    int                     GetColumnWidth( const int col ) const { return m_Columns->Item( col ).m_Width; }
    wxString                GetColumnLabel( const int col ) const { return m_Columns->Item( col ).m_Label; }
    void                    SetColumnLabel( const int col, const wxString &label );
    int                     GetColumnId( const int col ) const { return m_Columns->Item( col ).m_Id; }
    void                    SetColumnImage( const int col, const int imageindex );

    bool                    GetColumnData( const int id, int * index, int * width, bool * enabled );
    bool                    SetColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false );

    wxRect                  GetClientScreenRect( void ) { return m_ListBox->GetScreenRect(); }

    bool                    IsAllowedColumnSelect( void ) const { return m_ColSelect; }


    friend class guListViewClient;
    friend class guListViewDropTarget;
    friend class guListViewDropFilesThread;

};

// -------------------------------------------------------------------------------- //
class guListViewDropFilesThread : public wxThread
{
  protected :
    guListView *            m_ListView;                 // To add the files
    guListViewDropTarget *  m_ListViewDropTarget;       // To clear the thread pointer once its finished
    wxArrayString           m_Files;

    void AddDropFiles( const wxString &DirName );

  public :
    guListViewDropFilesThread( guListViewDropTarget * playlistdroptarget,
                                 guListView * listview, const wxArrayString &files );
    ~guListViewDropFilesThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guListViewDropTarget : public wxDropTarget
{
  private:
    guListView *                    m_ListView;
    guListViewDropFilesThread *     m_ListViewDropFilesThread;
    wxMutex                         m_DropFilesThreadMutex;

    void ClearPlayListFilesThread( void )
    {
        m_DropFilesThreadMutex.Lock();
        m_ListViewDropFilesThread = NULL;
        m_DropFilesThreadMutex.Unlock();
    }

  public:
    guListViewDropTarget( guListView * listview );
    ~guListViewDropTarget();

    virtual bool OnDrop( wxCoord x, wxCoord y );
    virtual wxDragResult OnData( wxCoord x, wxCoord y, wxDragResult def );

    virtual void OnLeave();

    virtual wxDragResult OnDragOver( wxCoord x, wxCoord y, wxDragResult def );


    friend class guListViewDropFilesThread;
};

}

#endif
// -------------------------------------------------------------------------------- //
