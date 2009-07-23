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

#include <wx/scrolwin.h>
#include <wx/vlbox.h>
#include <wx/settings.h>

class guListViewClient;
class guListViewHeader;

// Own style flags
#define guLISTVIEW_COLUMN_SELECT    0x1000

// -------------------------------------------------------------------------------- //
// guListViewColumn
// -------------------------------------------------------------------------------- //
class guListViewColumn
{
  public :
    wxString m_Label;
    int      m_Id;
    int      m_Width;
    bool     m_Enabled;

    guListViewColumn() {};
    ~guListViewColumn() {};
    guListViewColumn( const wxString &label, const int id, const int width = -1, const bool enabled = true )
    {
        m_Label = label;
        m_Id = id;
        m_Width = width;
        m_Enabled = enabled;
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
    wxFont *    m_Font;

    guListViewAttr() {
        m_SelBgColor  = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );
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

        m_Font = new wxFont( wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT ) );
    };

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

};

// -------------------------------------------------------------------------------- //
class guListView : public wxScrolledWindow
{
  protected :
    guListViewAttr          m_Attr;
    bool                    m_ColSelect;
    int                     m_ItemHeight;
    guListViewColumnArray * m_Columns;

    virtual void        GetItemsList( void ) = 0;
    virtual void        DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void        DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const;

  private :
    guListViewClient *      m_ListBox;
    guListViewHeader *      m_Header;

    void                OnChangedSize( wxSizeEvent &event );

    virtual wxString    OnGetItemText( const int row, const int column );
    virtual void        CreateContextMenu( wxMenu * menu ) const;
    void                OnContextMenu( wxContextMenuEvent &event );
    virtual wxCoord     OnMeasureItem( size_t row ) const;
    void                OnBeginDrag( wxMouseEvent &event );

    void                OnHScroll( wxScrollWinEvent &event );

  public :
    guListView( wxWindow * parent, const int flags = wxLB_MULTIPLE, wxWindowID id = wxID_ANY,
                const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                long style = wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER );
    ~guListView();

    void                    InsertColumn( guListViewColumn * column );
    void                    SetItemCount( const int count );
    int                     GetItemCount( void ) const;
    void                    SetItemHeight( const int height );
    void                    SetAttr( const guListViewAttr &attr ) { m_Attr = attr; };

    int                     GetFirstSelected( unsigned long &cookie ) const;
    int                     GetNextSelected( unsigned long &cookie ) const;
    bool                    Select( size_t item, bool select = true );
    void                    SetSelection( int selection );
    size_t                  GetFirstVisibleLine() const;
    bool                    ScrollToLine( size_t line );
    void                    RefreshAll();
    bool                    IsSelected( size_t row ) const;
    virtual int             GetSelectedSongs( guTrackArray * Songs ) const;

    virtual void            ReloadItems( bool reset = true ) = 0;

    wxArrayInt              GetSelectedItems( bool reallist = true ) const;
    void                    SetSelectedItems( const wxArrayInt &selection );
    void                    ClearSelectedItems();
    virtual wxString inline GetItemName( const int item ) const = 0;
    virtual int inline      GetItemId( const int item ) const = 0;
    long                    FindItem( long start, const wxString &str, bool partial );

    void                    SetColumnWidth( const int col, const int width );
    int                     GetColumnWidth( const int col ) const;

    bool                    IsAllowedColumnSelect( void ) const;

    friend class guListViewClient;

};

#endif
// -------------------------------------------------------------------------------- //
