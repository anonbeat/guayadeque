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
#ifndef GULISTVIEW_H
#define GULISTVIEW_H

#include "DbLibrary.h"
#include "Utils.h"

#include <wx/dnd.h>
#include <wx/scrolwin.h>
#include <wx/vlbox.h>
#include <wx/settings.h>

class guListViewClient;
class guListViewHeader;

// Own style flags
#define guLISTVIEW_COLUMN_SELECT        0x01000
#define guLISTVIEW_COLUMN_SORTING       0x02000
#define guLISTVIEW_ALLOWDRAG            0x04000
#define guLISTVIEW_ALLOWDROP            0x08000
#define guLISTVIEW_DRAGSELFITEMS        0x10000
#define guLISTVIEW_COLUMN_CLICK_EVENTS  0x20000

DECLARE_EVENT_TYPE( guEVT_LISTBOX_ITEM_COL_CLICKED, wxID_ANY )
#define EVT_LISTBOX_ITEM_COL_CLICKED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( guEVT_LISTBOX_ITEM_COL_CLICKED, winid, wxID_ANY, wxListEventHandler(fn), (wxObject *) NULL ),
DECLARE_EVENT_TYPE( guEVT_LISTBOX_ITEM_COL_RCLICKED, wxID_ANY )
#define EVT_LISTBOX_ITEM_COL_RCLICKED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( guEVT_LISTBOX_ITEM_COL_RCLICKED, winid, wxID_ANY, wxListEventHandler(fn), (wxObject *) NULL ),

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
    wxColor     m_SelBgColor;
    wxColor     m_SelFgColor;
    wxColor     m_EveBgColor;
    wxColor     m_OddBgColor;
    wxColor     m_TextFgColor;
    //wxColor     m_PlayFgColor;
    wxBrush     m_DragBgColor;
    wxFont *    m_Font;

    guListViewAttr() { LoadSysColors(); };

    ~guListViewAttr()
    {
        if( m_Font )
            delete m_Font;
    };

    guListViewAttr( const wxColor &selbg, const wxColor &selfg,
                    const wxColor &evebg, const wxColor &oddbg,
                    const wxColor &textfg, wxFont * font )
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
        if( m_EveBgColor.Red() > 0x0A && m_EveBgColor.Green() > 0x0A && m_EveBgColor.Blue() > 0x0A )
        {
            m_OddBgColor.Set( m_EveBgColor.Red() - 0xA, m_EveBgColor.Green() - 0x0A, m_EveBgColor.Blue() - 0x0A );
        }
        else
        {
            m_OddBgColor.Set( m_EveBgColor.Red() + 0xA, m_EveBgColor.Green() + 0x0A, m_EveBgColor.Blue() + 0x0A );
        }
        m_TextFgColor.Set( m_EveBgColor.Red() ^ 0xFF, m_EveBgColor.Green() ^ 0xFF, m_EveBgColor.Blue() ^ 0xFF );

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


    virtual void        OnKeyDown( wxKeyEvent &event );
    virtual void        GetItemsList( void ) = 0;
    virtual void        DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void        DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual wxString    OnGetItemText( const int row, const int column ) const;
    virtual void        CreateContextMenu( wxMenu * menu ) const;
    virtual wxCoord     OnMeasureItem( size_t row ) const;

    virtual void        OnBeginDrag( wxMouseEvent &event );
    virtual void        OnDragOver( const wxCoord x, const wxCoord y );
    virtual void        OnDropBegin( void );
    virtual void        OnDropFile( const wxString &filename );
    virtual void        OnDropEnd( void );
    virtual int         GetDragFiles( wxFileDataObject * files );
    virtual void        MoveSelection( void );
    //virtual void        OnSysColorChanged( wxSysColourChangedEvent &event );
    virtual void        OnMouse( wxMouseEvent &event );

    void                OnChangedSize( wxSizeEvent &event );
    void                OnContextMenu( wxContextMenuEvent &event );
    void                OnHScroll( wxScrollWinEvent &event );

  public :
    guListView( wxWindow * parent, const int flags = wxLB_MULTIPLE, wxWindowID id = wxID_ANY,
                const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                long style = wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER );
    ~guListView();

    void                    SetItemCount( const int count );
    int                     GetItemCount( void ) const;
    void                    SetItemHeight( const int height );
    void                    SetAttr( const guListViewAttr &attr ) { m_Attr = attr; };

    int                     GetFirstSelected( unsigned long &cookie ) const;
    int                     GetNextSelected( unsigned long &cookie ) const;
    int                     GetSelection() const;
    bool                    Select( size_t item, bool select = true );
    void                    SetSelection( int selection );

    size_t                  GetFirstVisibleLine() const;
    size_t                  GetLastVisibleLine() const;
    bool                    ScrollLines( int lines );

    bool                    ScrollToLine( size_t line );
    void                    RefreshAll( int scroll = wxNOT_FOUND );
    void                    RefreshLines( const int from, const int to );
    void                    RefreshLine( const int line );
    bool                    IsSelected( size_t row ) const;
    virtual int             GetSelectedSongs( guTrackArray * Songs ) const;
    int                     HitTest( wxCoord x, wxCoord y ) const;

    virtual void            ReloadItems( bool reset = true ) = 0;

    virtual wxArrayInt      GetSelectedItems( bool reallist = true ) const;
    virtual void            SetSelectedItems( const wxArrayInt &selection );
    virtual void            ClearSelectedItems();

    virtual wxString        GetItemName( const int item ) const = 0;
    virtual int             GetItemId( const int item ) const = 0;
    long                    FindItem( long start, const wxString &str, bool partial );

    void                    SetImageList( wxImageList * imagelist );

    void                    InsertColumn( guListViewColumn * column );
    void                    SetColumnWidth( const int col, const int width );
    int                     GetColumnWidth( const int col ) const;
    wxString                GetColumnLabel( const int col ) const;
    void                    SetColumnLabel( const int col, const wxString &label );
    int                     GetColumnId( const int col ) const;
    void                    SetColumnImage( const int col, const int imageindex );

    bool                    IsAllowedColumnSelect( void ) const;

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
class guListViewDropTarget : public wxFileDropTarget
{
  private:
    guListView *                    m_ListView;
    guListViewDropFilesThread *     m_ListViewDropFilesThread;

    void ClearPlayListFilesThread( void ) { m_ListViewDropFilesThread = NULL; };

  public:
    guListViewDropTarget( guListView * listview );
    ~guListViewDropTarget();

    virtual bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &files );

    virtual wxDragResult OnDragOver( wxCoord x, wxCoord y, wxDragResult def );


    friend class guListViewDropFilesThread;
};


#endif
// -------------------------------------------------------------------------------- //
