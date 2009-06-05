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
#include "AlListBox.h"

#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "Utils.h"

#include <wx/renderer.h>

#define ALLISTBOX_ITEM_SIZE  40

// -------------------------------------------------------------------------------- //
// guALbumListBoxHeader
// -------------------------------------------------------------------------------- //
class guAlbumListBoxHeader : public wxWindow
{
  protected:
    guAlbumListBox *   m_Owner;
    wxString           m_LabelFmt;
    wxString           m_LabelStr;

  public:
    guAlbumListBoxHeader();

    guAlbumListBoxHeader( wxWindow * parent,
                        guAlbumListBox * owner,
                        const wxString &label,
                        const wxPoint &pos = wxDefaultPosition,
                        const wxSize &size = wxDefaultSize );

    virtual ~guAlbumListBoxHeader();

    void OnPaint( wxPaintEvent &event );
    void OnSetFocus( wxFocusEvent &event );
    void UpdateLabel( void );

private:

    DECLARE_EVENT_TABLE()
};

class guAlbumListBoxTimer;

// -------------------------------------------------------------------------------- //
// guAlbumListBox
// -------------------------------------------------------------------------------- //
class guAlbumListBox : public wxVListBox
{
    private :
        guAlbumListBoxHeader *  m_Header;
        guAlbumItems            m_Items;
        DbLibrary *             m_Db;
        guAlbumListBoxTimer *   m_SearchStrTimer;
        wxString                m_SearchStr;

        wxPoint                 m_DragStart;
        int                     m_DragCount;

        wxColor                 m_SelBgColor;
        wxColor                 m_SelFgColor;
        wxColor                 m_OddBgColor;
        wxColor                 m_EveBgColor;
        wxColor                 m_TextFgColor;

        void            OnDragOver( const wxCoord x, const wxCoord y );
        void            OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const;
        wxCoord         OnMeasureItem( size_t n ) const;
        void            OnDrawBackground( wxDC &dc, const wxRect &rect, size_t n ) const;
        void            OnKeyDown( wxKeyEvent &event );
        void            OnBeginDrag( wxMouseEvent &event );
        void            OnMouse( wxMouseEvent &event );
        void            OnContextMenu( wxContextMenuEvent& event );

        void            OnSearchLinkClicked( wxCommandEvent &event );
        wxString        GetSearchText( int Item );

        DECLARE_EVENT_TABLE()

        friend class guAlbumListBoxTimer;

    public :
        guAlbumListBox( wxWindow * parent, DbLibrary * db );
        ~guAlbumListBox();

        wxArrayInt  GetSelection() const;
        void        SetSelectedItems( wxArrayInt selection );
        int         GetSelectedSongs( guTrackArray * songs ) const;
        void        AddDropFile( const wxString &filename, bool addpath = false );

        void        ReloadItems( const bool reset = true );
        void        UpdateView();
        long        FindItem( long start, const wxString& str, bool partial );
        long        FindItem( const long start, const long id );
};

// -------------------------------------------------------------------------------- //
// guAlbumListBoxTimer
// -------------------------------------------------------------------------------- //
class guAlbumListBoxTimer : public wxTimer
{
public:
    //Ctor
    guAlbumListBoxTimer( guAlbumListBox * NewListBox ) { AlbumListBox = NewListBox; }

    //Called each time the timer's timeout expires
    void Notify(); // { ItemListBox->SearchStr = wxEmptyString; };

    guAlbumListBox * AlbumListBox;       //
};


// -------------------------------------------------------------------------------- //
// guAlListBox
// -------------------------------------------------------------------------------- //
guAlListBox::guAlListBox( wxWindow * parent, DbLibrary * db, const wxString &label ) :
    wxScrolledWindow( parent )
{
    m_ListBox = new guAlbumListBox( this, db );
    m_Header = new guAlbumListBoxHeader( this, m_ListBox, label, wxPoint( 0, 0 ) );

    parent->Connect( wxEVT_SIZE, wxSizeEventHandler( guAlListBox::OnChangedSize ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guAlListBox::~guAlListBox()
{
    GetParent()->Disconnect( wxEVT_SIZE, wxSizeEventHandler( guAlListBox::OnChangedSize ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnChangedSize( wxSizeEvent &event )
{
    int w, h, d;
    wxSize Size = event.GetSize();
    if( m_Header )
    {
        // Calculate the Height
        GetTextExtent( wxT("Hg"), &w, &h, &d );
        h += d + 4;
        // Calculate the Width
        //
        w = Size.x;
        if( m_ListBox )
        {
            // If we need to show a scrollbar
            if( ( int ) m_ListBox->GetItemCount() > ( Size.GetHeight() / ALLISTBOX_ITEM_SIZE ) )
            {
                w -= wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );
            }
        }
        // Only change size if its different
        if( m_Header->GetSize().GetWidth() != w )
        {
            m_Header->SetSize( w, h );
        }
//        guLogMessage( wxT( "Header SetSize %i,%i" ), w, h );
    }
    if( m_ListBox )
    {
        m_ListBox->SetSize( Size.GetWidth(), Size.GetHeight() - h );
        m_ListBox->Move( 0, h );
    }
    // continue with default behaivor
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guAlListBox::ReloadItems( const bool reset )
{
    m_ListBox->ReloadItems( reset );
    m_Header->UpdateLabel();
}

// -------------------------------------------------------------------------------- //
wxArrayInt guAlListBox::GetSelection() const
{
    return m_ListBox->GetSelection();
}

// -------------------------------------------------------------------------------- //
int guAlListBox::GetSelectedSongs( guTrackArray * tracks ) const
{
    return m_ListBox->GetSelectedSongs( tracks );
}

// -------------------------------------------------------------------------------- //
// guAlbumListBox
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guAlbumListBox,wxVListBox)
  EVT_MOUSE_EVENTS   (guAlbumListBox::OnMouse)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guAlbumListBox::guAlbumListBox( wxWindow * parent, DbLibrary * db ) :
    wxVListBox( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_MULTIPLE )
{
    m_Db = db;
    m_Items.Empty();
    m_SearchStrTimer = new guAlbumListBoxTimer( this );
    m_SearchStr = wxEmptyString;

    wxSystemSettings SystemSettings;

    m_SelBgColor  = SystemSettings.GetColour( wxSYS_COLOUR_HIGHLIGHT );
    m_SelFgColor  = SystemSettings.GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT );
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
    SetBackgroundColour( m_EveBgColor );

    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guAlbumListBox::OnContextMenu ), NULL, this );
    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guAlbumListBox::OnBeginDrag ), NULL, this );
    Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guAlbumListBox::OnKeyDown ), NULL, this );
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumListBox::OnSearchLinkClicked ) );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guAlbumListBox::~guAlbumListBox()
{
    m_Items.Clear();

    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guAlbumListBox::OnContextMenu ), NULL, this );
    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guAlbumListBox::OnBeginDrag ), NULL, this );
    Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guAlbumListBox::OnKeyDown ), NULL, this );
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumListBox::OnSearchLinkClicked ) );

    if( m_SearchStrTimer )
      delete m_SearchStrTimer;
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::OnKeyDown( wxKeyEvent &event )
{
    //event.Skip();
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
        m_SearchStrTimer->Start( 500, wxTIMER_ONE_SHOT );
        m_SearchStr.Append( KeyChar );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const
{
    wxString CoverPath;
//    wxLongLong time = wxGetLocalTimeMillis();

    if( ( int ) n == wxNOT_FOUND )
        return;

    wxFont Font( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

    guAlbumItem * Item = &m_Items[ n ];

    dc.SetFont( Font );
    dc.SetBackgroundMode( wxTRANSPARENT );

    dc.SetTextForeground( IsSelected( n ) ? m_SelFgColor : m_TextFgColor );

    dc.DrawText( Item->m_Name, rect.x + 45, rect.y + 8 );

    if( Item->m_Year > 0 )
    {
        Font.SetPointSize( 7 );
        dc.SetFont( Font );
        dc.DrawText( wxString::Format( wxT( "%04u" ), Item->m_Year ), rect.x + 45, rect.y + 26 );
    }

    if( Item->m_Thumb && Item->m_Thumb->IsOk() )
    {
        dc.DrawBitmap( * Item->m_Thumb, rect.x + 1, rect.y + 1, false );
    }
    else if( Item->m_Thumb )
    {
        guLogError( wxT( "Thumb image corrupt or not correctly loaded" ) );
    }

//    time = wxGetLocalTimeMillis() - time;
//    guLogWarning( _T( "OnDrawItem %i (%ums)" ), n, time.GetLo() );
}

// -------------------------------------------------------------------------------- //
wxCoord guAlbumListBox::OnMeasureItem( size_t n ) const
{
    return wxCoord( ALLISTBOX_ITEM_SIZE );
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::OnDrawBackground( wxDC &dc, const wxRect &rect, size_t n ) const
{
    if( ( int ) n == wxNOT_FOUND )
        return;

    if( IsSelected( n ) )
      dc.SetBrush( wxBrush( m_SelBgColor ) );
    else
      dc.SetBrush( wxBrush( n & 1 ? m_OddBgColor : m_EveBgColor ) );

    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.DrawRectangle( rect );

}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::UpdateView()
{
    RefreshLines( GetVisibleBegin(), GetVisibleEnd() );
}

// -------------------------------------------------------------------------------- //
wxArrayInt guAlbumListBox::GetSelection() const
{
    wxArrayInt RetVal;
    unsigned long cookie;
    int item;
    int index;
    int count;
    int Id;
    item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        Id = m_Items[ item  ].m_Id;
        RetVal.Add(  Id );
        if( Id == 0 )
            break;
        item = GetNextSelected( cookie );
    }
    //
    if( RetVal.Index( 0 ) != wxNOT_FOUND )
    {
        RetVal.Empty();
        count = m_Items.Count();
        for( index = 0; index < count; index++ )
        {
            RetVal.Add( m_Items[ index ].m_Id );
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::SetSelectedItems( wxArrayInt Selection )
{
    int index;
    int count = m_Items.Count();
    for( index = 0; index < count; index++ )
    {
        if( Selection.Index( m_Items[ index ].m_Id ) != wxNOT_FOUND )
            Select( index );
    }
}

// -------------------------------------------------------------------------------- //
int guAlbumListBox::GetSelectedSongs( guTrackArray * Songs ) const
{
    return m_Db->GetAlbumsSongs( GetSelection(), Songs );
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::ReloadItems( const bool Reset )
{
    guAlbumItem * AlbumItem;
    wxArrayInt Selection;
    int FirstVisible = GetFirstVisibleLine();

    if( Reset )
        SetSelection( -1 );
    else
        Selection = GetSelection();

    m_Items.Empty();
    AlbumItem = new guAlbumItem;
    if( AlbumItem )
    {
        AlbumItem->m_Id = 0;
        AlbumItem->m_Name = _( "All" );
        AlbumItem->m_CoverId = 0;
        AlbumItem->m_Thumb = NULL;
        m_Items.Add( AlbumItem );
    }

    m_Db->GetAlbums( &m_Items );
    SetItemCount( m_Items.Count() );

    if( !Reset )
    {
      SetSelectedItems( Selection );
      ScrollToLine( FirstVisible );
    }
    UpdateView();
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::OnContextMenu( wxContextMenuEvent& event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;
    int SelCount;

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

    MenuItem = new wxMenuItem( &Menu, ID_ALBUM_PLAY, _( "Play" ), _( "Play current selected albums" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_media_playback_start ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_ALBUM_ENQUEUE, _( "Enqueue" ), _( "Add current selected albums to the Playlist" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
    Menu.Append( MenuItem );

    SelCount = GetSelectedCount();
    if( SelCount )
    {
        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUM_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected albums" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_tags ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_ALBUM_EDITTRACKS, _( "Edit Album songs" ), _( "Edit the selected albums songs" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
        Menu.Append( MenuItem );

        if( SelCount == 1 )
        {
            Menu.AppendSeparator();

            MenuItem = new wxMenuItem( &Menu, ID_ALBUM_MANUALCOVER, _( "Download Album cover" ), _( "Download cover for the current selected album" ) );
            MenuItem->SetBitmap( wxBitmap( guImage_download_covers ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_ALBUM_COVER_DELETE, _( "Delete Album cover" ), _( "Delete the cover for the selected album" ) );
            MenuItem->SetBitmap( wxBitmap( guImage_edit_delete ) );
            Menu.Append( MenuItem );
        }

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUM_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_edit_copy ) );
        Menu.Append( MenuItem );

        if( SelCount == 1 )
        {
            Menu.AppendSeparator();

            AddOnlineLinksMenu( &Menu );
        }
    }

    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::OnMouse( wxMouseEvent &event )
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
void guAlbumListBox::OnBeginDrag( wxMouseEvent &event )
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
long guAlbumListBox::FindItem( long start, const wxString& str, bool partial )
{
    if( str.empty() )
        return wxNOT_FOUND;

    long pos = start;
    wxString str_upper = str.Upper();
    if( pos < 0 )
        pos = 0;

    size_t count = GetItemCount();
    for( size_t i = ( size_t )pos; i < count; i++ )
    {
        guAlbumItem * Item = &m_Items[ i ];
        wxString line_upper = Item->m_Name.Upper();
        if( !partial )
        {
            if( line_upper == str_upper )
                return i;
        }
        else
        {
            if( line_upper.find( str_upper ) == 0 )
                return i;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
long guAlbumListBox::FindItem( const long start, const long Id )
{
    long pos = start;
    if( pos < 0 )
        pos = 0;
    size_t count = GetItemCount();
    for( size_t i = ( size_t )pos; i < count; i++ )
    {
        guAlbumItem * Item = &m_Items[ i ];
        if( Item->m_Id == Id )
        {
            return i;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guAlbumListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    int Item;
    unsigned long cookie;
    Item = GetFirstSelected( cookie );
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
wxString guAlbumListBox::GetSearchText( int item )
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Db->GetArtistName( m_Items[ item ].m_ArtistId ).c_str(),
        m_Items[ item ].m_Name.c_str() );
}

// -------------------------------------------------------------------------------- //
//  guAlbumListBoxHeader
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guAlbumListBoxHeader,wxWindow)
    EVT_PAINT         (guAlbumListBoxHeader::OnPaint)
    EVT_SET_FOCUS     (guAlbumListBoxHeader::OnSetFocus)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guAlbumListBoxHeader::guAlbumListBoxHeader()
{
    m_Owner = (guAlbumListBox *) NULL;
    m_LabelStr = wxEmptyString;
    m_LabelFmt = wxEmptyString;
}

// -------------------------------------------------------------------------------- //
guAlbumListBoxHeader::guAlbumListBoxHeader( wxWindow * parent, guAlbumListBox *newowner, const wxString &newlabel,
                                              const wxPoint& pos, const wxSize& size )
                  : wxWindow( parent, wxID_ANY, pos, size )
{
    m_Owner = newowner;
    m_LabelFmt = newlabel + wxT( " (%u)" );
    UpdateLabel();
}

// -------------------------------------------------------------------------------- //
guAlbumListBoxHeader::~guAlbumListBoxHeader()
{
}

// -------------------------------------------------------------------------------- //
void guAlbumListBoxHeader::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );

    PrepareDC( dc );
//    AdjustDC( dc );

    dc.SetFont( GetFont() );

    // width and height of the entire header window
    int w, h;
    GetClientSize( &w, &h );
//    m_owner->CalcUnscrolledPosition(w, 0, &w, NULL);

    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.SetTextForeground( GetForegroundColour() );

    int flags = 0;
    if (!m_parent->IsEnabled())
        flags |= wxCONTROL_DISABLED;

    wxRendererNative::Get().DrawHeaderButton( this, dc, wxRect( 0, 0, w, h ), flags );

    wxCoord wLabel;
    wxCoord hLabel;
    dc.GetTextExtent( m_LabelStr, &wLabel, &hLabel);

    dc.DrawText( m_LabelStr, 4, h / 2 - hLabel / 2 );
//    guLogMessage( wxT( "OnPaint (%i,%i)" ), w, h );
}

// -------------------------------------------------------------------------------- //
void guAlbumListBoxHeader::OnSetFocus( wxFocusEvent &WXUNUSED(event) )
{
    if( m_Owner )
    {
        m_Owner->SetFocus();
        m_Owner->Update();
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumListBoxHeader::UpdateLabel( void )
{
    int h;
    int w;
    wxSize Size;
    wxSize CurSize;
    if( m_Owner )
    {
        Size = m_Owner->GetSize();
        w = Size.GetWidth();
        CurSize = GetSize();
        h = CurSize.GetHeight();
        if( ( int ) m_Owner->GetItemCount() > ( Size.GetHeight() / ALLISTBOX_ITEM_SIZE ) )
        {
            w -= wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );
        }
        if( CurSize.GetWidth() != w )
        {
            SetSize( w, GetSize().GetHeight() );
        }
    }
    m_LabelStr = wxString::Format( m_LabelFmt, m_Owner->GetItemCount() - 1 );
    Refresh();
//    Update();
//    guLogMessage( wxT( "Header::UpdateLabel SetSize %i,%i" ), w, h );
}

// -------------------------------------------------------------------------------- //
// guAlbumListBoxTimer
// -------------------------------------------------------------------------------- //
void guAlbumListBoxTimer::Notify()
{
    int index;
    if( AlbumListBox->m_SearchStr.Len() )
    {
        AlbumListBox->SetSelection( -1 );
        if( ( index = AlbumListBox->FindItem( 0, AlbumListBox->m_SearchStr, true ) ) != wxNOT_FOUND )
        {
            AlbumListBox->SetSelection( index );
        }
        AlbumListBox->m_SearchStr = wxEmptyString;
    }
}

// -------------------------------------------------------------------------------- //
