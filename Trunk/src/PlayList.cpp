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
#include "PlayList.h"

#include "Config.h"
#include "Commands.h"
#include "Images.h"
#include "LabelEditor.h"
#include "MainApp.h"
#include "Shoutcast.h"
#include "TagInfo.h"
#include "Utils.h"
#include "dbus/mpris.h"

//#include <id3/tag.h>
//#include <id3/misc_support.h>
#include <wx/types.h>
#include <wx/uri.h>

#define GUPLAYLIST_ITEM_SIZE        40

// -------------------------------------------------------------------------------- //
// guPlayList
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guPlayList,wxVListBox)
  EVT_MOUSE_EVENTS   (guPlayList::OnMouse)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guPlayList::guPlayList( wxWindow * parent, DbLibrary * db ) :
            wxVListBox( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_MULTIPLE|wxSUNKEN_BORDER  )
{
    wxArrayString Songs;
    int Count;
    int Index;
    guConfig * Config;

    m_Db = db;
    m_TotalLen = 0;
    m_CurItem = wxNOT_FOUND;
    m_StartPlaying = false;

    //CurItem = wxNOT_FOUND;
    m_DragOverItem = wxNOT_FOUND;
    m_DragOverAfter = false;
    m_DragSelfItems = false;
    m_DragStart = wxPoint( -1, -1 );
    m_DragCount = 0;

    guMainApp * MainApp = ( guMainApp * ) wxTheApp;
    if( MainApp && MainApp->argc > 1 )
    {
        Count = MainApp->argc;
        for( Index = 1; Index < Count; Index++ )
        {
            //wxMessageBox( wxString::Format( wxT( "%u-%u %s" ), Index, MainApp->argc, MainApp->argv[ Index ] ), wxT( "Song" ) );
            if( wxFileExists( MainApp->argv[ Index ] ) )
            {
                AddPlayListItem( MainApp->argv[ Index ] );
                m_StartPlaying = true;
            }
        }
    }
    else
    {
        // Load the saved guPlayList
        Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            Songs = Config->ReadAStr( wxT( "PlayListSong" ), wxEmptyString, wxT( "PlayList" ) );
            Count = Songs.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                AddPlayListItem( Songs[ Index ], false );
            }
            m_CurItem = Config->ReadNum( wxT( "PlayerCurItem" ), -1l, wxT( "General" ) );
            m_SmartPlayMaxPlayListTracks = Config->ReadNum( wxT( "MaxPlayListTracks" ), 15, wxT( "SmartPlayList" ) );
            //
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATELIST );
            //event.SetEventObject( ( wxObject * ) this );
            wxPostEvent( this, event );
        }
    }

    SetDropTarget( new guPlayListDropTarget( this ) );

//    m_PlayBgColor  = wxColor( 0, 0, 0 ); //SystemSettings.GetColour( wxSYS_COLOUR_HIGHLIGHT );
//    m_PlayFgColor  = wxColor( 255, 255, 255 ); //SystemSettings.GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT );
    m_DragBgColor  = * wxGREY_BRUSH;
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
    //m_SepColor    = SystemSettings.GetColour( wxSYS_COLOUR_WINDOWFRAME );
    //m_PlayBgColor  = m_TextFgColor;
    m_PlayFgColor  = m_SelBgColor;
    //m_RatingEnabled = wxColour( 255, 191, 0 );
    //m_RatingDisabled = wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT );

    SetBackgroundColour( m_EveBgColor );

    m_Font = new wxFont( wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT ) );

    m_PlayBitmap = new wxBitmap( guImage( guIMAGE_INDEX_tiny_playback_start ) );
    m_GreyStar   = new wxBitmap( guImage( guIMAGE_INDEX_grey_star_tiny ) );
    m_YellowStar = new wxBitmap( guImage( guIMAGE_INDEX_yellow_star_tiny ) );

	Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guPlayList::OnKeyDown ), NULL, this );
    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guPlayList::OnBeginDrag ), NULL, this );
    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guPlayList::OnContextMenu ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnClearClicked ) );
    Connect( ID_PLAYER_PLAYLIST_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnRemoveClicked ) );
    Connect( ID_PLAYER_PLAYLIST_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSaveClicked ) );
    Connect( ID_PLAYER_PLAYLIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCopyToClicked ) );
    Connect( ID_PLAYER_PLAYLIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnEditLabelsClicked ) );
    Connect( ID_PLAYER_PLAYLIST_COMMANDS, ID_PLAYER_PLAYLIST_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCommandClicked ) );

}

// -------------------------------------------------------------------------------- //
guPlayList::~guPlayList()
{
    // Save the guPlayList so it can be reload next time
    wxArrayString Songs;
    int Count;
    int Index;
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config && Config->ReadBool( wxT( "SavePlayListOnClose" ), true, wxT( "General" ) ) )
    {
        Count = m_Items.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            Songs.Add( m_Items[ Index ].m_FileName );
        }
        Config->WriteAStr( wxT( "PlayListSong" ), Songs, wxT( "PlayList" ) );
        Config->WriteNum( wxT( "PlayerCurItem" ), m_CurItem, wxT( "General" ) );
    }
    // Destroy all items
    m_Items.Clear();

    if( m_PlayBitmap )
      delete m_PlayBitmap;
    if( m_GreyStar )
      delete m_GreyStar;
    if( m_YellowStar )
      delete m_YellowStar;

    if( m_Font )
        delete m_Font;

	Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guPlayList::OnKeyDown ), NULL, this );
    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guPlayList::OnBeginDrag ), NULL, this );
    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guPlayList::OnContextMenu ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnClearClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnRemoveClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSaveClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCopyToClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnEditLabelsClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_COMMANDS, ID_PLAYER_PLAYLIST_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCommandClicked ) );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnMouse( wxMouseEvent &event )
{
    //printf( "guPlayList::OnMouse\n" );
    if( event.Dragging() )
    {
        //printf( "guPlayList::OnMouseDragging\n" );
        if( !m_DragCount )
        {
            m_DragStart = event.GetPosition();
        }

        if( ++m_DragCount == 3 )
        {
            //printf( "guPlayList::OnMouseDragging (Fired!) \n" );
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
void guPlayList::OnBeginDrag( wxMouseEvent &event )
{
    //printf( "Drag started\n" );
    wxFileDataObject Files; // = wxFileDataObject();
    int index;
    int count;
    wxArrayInt Selection = GetSelectedItems();

    count = Selection.Count();
    //printf( "OnBeginDraw: %d\n", count );

    for( index = 0; index < count; index++ )
    {
       Files.AddFile( m_Items[ Selection[ index ] ].m_FileName );
       //printf( Items[ Selection[ index ] ].FileName.char_str() ); printf( "\n" );
    }

    wxDropSource source( Files, this );

    m_DragSelfItems = true;
    wxDragResult Result = source.DoDragDrop();
    if( Result )
    {

    }
    m_DragSelfItems = false;
    m_DragOverItem = wxNOT_FOUND;
    //wxMessageBox( wxT( "DoDragDrop Done" ) );
}

// -------------------------------------------------------------------------------- //
wxArrayInt guPlayList::GetSelectedItems()
{
    wxArrayInt RetVal;
    unsigned long cookie;
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        RetVal.Add( item );
        item = GetNextSelected( cookie );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayList::RemoveItem( int itemnum )
{
    int count = m_Items.Count();
    if( count && ( itemnum < count ) )
    {
        m_TotalLen -= m_Items[ itemnum ].m_Length;
        m_Items.RemoveAt( itemnum );
        if( itemnum == m_CurItem )
            m_CurItem = wxNOT_FOUND;
        else if( itemnum < m_CurItem )
            m_CurItem--;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::RemoveSelected()
{
    int Index;
    int item;
    wxArrayInt Selection = GetSelectedItems();
    for( Index = Selection.Count() - 1; Index >= 0; Index-- )
    {
        item = Selection[ Index ];
        RemoveItem( item );
    }
    DoSelectAll( false );
}

//static void PrintItems( guTrackArray Songs, int IP, int SI, int CI )
//{
//    int Index;
//    int Count = Songs.Count();
//    printf( "SI: %d  IP: %d  CI: %d\n", SI, IP, CI );
//    for( Index = 0; Index < Count; Index++ )
//    {
//        printf( "%02d ", Songs[ Index ].Number );
//    }
//    printf( "\n" );
//}

// -------------------------------------------------------------------------------- //
void guPlayList::MoveSelected()
{
    //
    // Move the Selected Items to the DragOverItem and DragOverFirst
    //
    int     InsertPos;
    int     Index;
    int     Count;
    bool    CurItemSet = false;
    guTrackArray MoveItems;
    wxArrayInt Selection = GetSelectedItems();
    if( m_DragOverItem != ( size_t ) wxNOT_FOUND )
    {
        // Where is the Items to be moved
        InsertPos = m_DragOverAfter ? m_DragOverItem + 1 : m_DragOverItem;
        // How Many elements to move
        Count = Selection.Count();
//        PrintItems( Items, InsertPos, Selection[ 0 ], CurItem );
        // Get a copy of every element to move
        for( Index = 0; Index < Count; Index++ )
        {
            MoveItems.Add( m_Items[ Selection[ Index ] ] );
        }
        // Remove the Items and move CurItem and InsertPos
        // We move from last (bigger) to first
        for( Index = Count - 1; Index >= 0; Index-- )
        {
            m_Items.RemoveAt( Selection[ Index ] );
            if( Selection[ Index ] < InsertPos )
                InsertPos--;
            if( Selection[ Index ] < m_CurItem )
                m_CurItem--;
            else if( Selection[ Index ] == m_CurItem )
            {
                m_CurItem = InsertPos + Index;
                CurItemSet = true;
            }
        }

//        PrintItems( Items, InsertPos, Selection[ 0 ], CurItem );

        // Insert every element at the InsertPos
        for( Index = 0; Index < Count; Index++ )
        {
            m_Items.Insert( MoveItems[ Index ], InsertPos );
            if( !CurItemSet && ( InsertPos <= m_CurItem ) )
                m_CurItem++;
            InsertPos++;
        }
//        PrintItems( Items, InsertPos, Selection[ 0 ], CurItem );
    }
    DoSelectAll( false );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        //wxMessageBox( wxT( "Delete" ) );
        RemoveSelected();
        RefreshItems();
        //RefreshAll();
        return;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddToPlayList( const guTrackArray &items, const bool deleteold )
{
    int Index;
    int Count;
    if( m_CurItem == wxNOT_FOUND )
        m_CurItem = 0;

    Count = items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
      m_Items.Add( items[ Index ] );
      m_TotalLen += items[ Index ].m_Length;
      if( deleteold && ( m_CurItem != 0 ) && ( m_Items.Count() > ( size_t ) m_SmartPlayMaxPlayListTracks ) )
      {
        m_TotalLen -= m_Items[ 0 ].m_Length;
        m_Items.RemoveAt( 0 );
        m_CurItem--;
      }
    }
    RefreshItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetPlayList( const guTrackArray &NewItems )
{
    int Index;
    int Count;
    m_Items = NewItems;

    m_CurItem = 0;
    Count = m_Items.Count();
    m_TotalLen = 0;
    for( Index = 0; Index < Count; Index++ )
    {
      m_TotalLen += m_Items[ Index ].m_Length;
    }

    RefreshItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDragOver( const wxCoord x, const wxCoord y )
{
    m_DragOverItem = HitTest( x, y );
    // Check if its over a item if its in the upper or lower part
    // to determine if will be inserted before or after
    if( ( int ) m_DragOverItem != wxNOT_FOUND )
    {
        m_DragOverAfter = ( y > ( ( ( ( int ) m_DragOverItem - GetFirstVisibleLine() + 1 ) * GUPLAYLIST_ITEM_SIZE ) - ( GUPLAYLIST_ITEM_SIZE / 2 ) ) );
        RefreshLines( wxMax( ( int ) m_DragOverItem - 1, 0 ), wxMin( ( ( int ) m_DragOverItem + 3 ), GetCount() ) );
    }
    int Width;
    int Height;
    GetSize( &Width, &Height );
    if( ( y > ( Height - 10 ) ) && GetLastVisibleLine() != GetCount() )
    {
        ScrollLines( 1 );
    }
    else
    {
        if( ( y < 10 ) && GetFirstVisibleLine() > 0 )
        {
            ScrollLines( -1 );
        }
    }
    //printf( "DragOverItem: %d ( %d, %d )\n", DragOverItem, x, y );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const
{
    guTrack Item;
    wxRect CutRect;
    wxSize TextSize;
    wxString TimeStr;
//    wxArrayInt Selection;

    if( ( int ) n == wxNOT_FOUND )
        return;

    Item = m_Items[ n ];
    m_Font->SetPointSize( 8 );
    m_Font->SetStyle( wxFONTSTYLE_NORMAL );
    m_Font->SetWeight( wxFONTWEIGHT_BOLD );

    dc.SetFont( * m_Font );
    dc.SetBackgroundMode( wxTRANSPARENT );
    if( IsSelected( n ) )
    {
        dc.SetTextForeground( m_SelFgColor );
    }
    else if( n == ( size_t ) m_CurItem )
    {
        dc.SetTextForeground( m_PlayFgColor );
    }
    else
    {
        dc.SetTextForeground( m_TextFgColor );
    }

    if( Item.m_SongId != guPLAYLIST_RADIOSTATION )
    {
        //y = dc.GetCharHeight();
        int OffsetSecLine;
        dc.DrawText( Item.m_SongName, rect.x + 5, rect.y + 5 );
        //Font.SetPointSize( 7 );
        m_Font->SetStyle( wxFONTSTYLE_ITALIC );
        m_Font->SetWeight( wxFONTWEIGHT_NORMAL );
        dc.SetFont( * m_Font );

        OffsetSecLine = rect.y + ( dc.GetCharHeight() + 10 );

        dc.DrawText( Item.m_ArtistName + wxT( " - " ) + Item.m_AlbumName, rect.x + 5, OffsetSecLine );

        // Get the area where the length will be writen
        TimeStr = LenToString( Item.m_Length );
        TextSize = dc.GetTextExtent( TimeStr );
        CutRect = rect;
        CutRect.x += CutRect.width - ( 50 + 6 );
        CutRect.width -= CutRect.x;
        OnDrawBackground( dc, CutRect, n );

        // Draw Play bitmap
        if( n == ( size_t ) m_CurItem && m_PlayBitmap )
        {
            dc.DrawBitmap( * m_PlayBitmap, CutRect.x + 21, CutRect.y + 14, true );
        }

        // Draw the Length string
        dc.DrawText( TimeStr, CutRect.x + ( ( 56 - TextSize.GetWidth() ) / 2 ), CutRect.y + 5 );
        //guLogMessage( wxT( "%i - %i" ), TextSize.GetWidth(), TextSize.GetHeight() );

        // Draw the rating
        int index;
        OffsetSecLine += 2;
        CutRect.x += 3;
        for( index = 0; index < 5; index++ )
        {
           dc.DrawBitmap( ( index >= Item.m_Rating ) ? * m_GreyStar : * m_YellowStar,
                          CutRect.x + ( 10 * index ), OffsetSecLine, true );
        }
    }
    else
    {
        dc.DrawText( Item.m_SongName, rect.x + 25, rect.y + 5 );
        CutRect = rect;
        CutRect.x += CutRect.width - 30;
        CutRect.width = 30;
        OnDrawBackground( dc, CutRect, n );
        //dc.DrawText( TimeStr, CutRect.x + 3, CutRect.y + 5 );
        if( n == ( size_t ) m_CurItem && m_PlayBitmap )
        {
            // Draw Play bitmap
            dc.DrawBitmap( * m_PlayBitmap, CutRect.x + 8, CutRect.y + 8, true );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxCoord guPlayList::OnMeasureItem( size_t n ) const
{
    return wxCoord( GUPLAYLIST_ITEM_SIZE );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDrawBackground( wxDC &dc, const wxRect &rect, size_t n ) const
{
    wxRect LineRect;

    if( ( int ) n == wxNOT_FOUND )
        return;

    if( n == m_DragOverItem )
      dc.SetBrush( m_DragBgColor );
    //else if( n == ( size_t ) GetSelection() )
    else if( IsSelected( n ) )
      dc.SetBrush( wxBrush( m_SelBgColor ) );
//    else if( n == ( size_t ) m_CurItem )
//      dc.SetBrush( wxBrush( m_PlayBgColor ) );
    else
      dc.SetBrush( wxBrush( n & 1 ? m_OddBgColor : m_EveBgColor ) );

    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.DrawRectangle( rect );

    if( n == m_DragOverItem )
    {
        LineRect = rect;
        if( m_DragOverAfter )
            LineRect.y += ( LineRect.height - 2 );
        LineRect.height = 2;
        dc.SetBrush( * wxBLACK_BRUSH );
        dc.DrawRectangle( LineRect );
    }
    //printf( "DrawBackground: %d\n", ( int ) n );
}

// -------------------------------------------------------------------------------- //
long guPlayList::GetCount()
{
    return m_Items.GetCount();
}

// -------------------------------------------------------------------------------- //
void guPlayList::UpdateView( bool Scroll )
{
    if( Scroll && !IsVisible( m_CurItem ) )
    {
        ScrollToLine( m_CurItem );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guPlayList::RefreshItems()
{
    SetItemCount( GetCount() );
    UpdateView();
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddItem( const guTrack &NewItem )
{
    int InsertPos;
    if( m_DragOverItem != ( size_t ) wxNOT_FOUND )
    {
        InsertPos = m_DragOverAfter ? m_DragOverItem + 1 : m_DragOverItem;
        if( InsertPos <= m_CurItem )
            m_CurItem++;
        //printf( "Inserted at %d\n", DragOverItem );
        m_Items.Insert( NewItem, InsertPos );
    }
    else
    {
        //printf( "Added at %d\n", DragOverItem );
        m_Items.Add( NewItem );
    }
    if( m_CurItem == wxNOT_FOUND )
        m_CurItem = 0;
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddItem( const guTrack * NewItem )
{
    AddItem( * NewItem );
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetCurrent( const int NewCurItem )
{
    if( NewCurItem >= 0 && NewCurItem <= GetCount() )
        m_CurItem = NewCurItem;
    else
        m_CurItem = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guPlayList::GetCurItem( void )
{
    return m_CurItem;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetCurrent( void )
{
//    if( ( CurItem == wxNOT_FOUND ) && Items.Count() )
//        CurItem = 0;
    return GetItem( m_CurItem );
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetNext( const bool PlayLoop )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( ( m_CurItem < ( ( int ) m_Items.Count() - 1 ) ) )
        {
            m_CurItem++;
            return &m_Items[ m_CurItem ];
        }
        else if( PlayLoop )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetPrev( const bool bLoop )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( m_CurItem > 0 )
        {
            m_CurItem--;
            return &m_Items[ m_CurItem ];
        }
        else if( bLoop )
        {
            m_CurItem = m_Items.Count() - 1;
            return &m_Items[ m_CurItem ];
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetItem( size_t item )
{
    size_t ItemsCount = m_Items.Count();
    if( ItemsCount && item >= 0 && item < ItemsCount )
    {
      return &m_Items[ item ];
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
long guPlayList::GetLength( void ) const
{
    return m_TotalLen;
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::GetLengthStr() const
{
    return LenToString( m_TotalLen );
}

// -------------------------------------------------------------------------------- //
void guPlayList::ClearItems()
{
    int Index;
    for( Index = m_Items.Count() - 1; Index >= 0; Index-- )
    {
        m_Items.RemoveAt( Index );
    }
    m_CurItem = wxNOT_FOUND;
    m_TotalLen = 0;
    DoSelectAll( false );
    RefreshItems();
    //PlayerPanel->UpdateTotalLength();
}

// -------------------------------------------------------------------------------- //
void guPlayList::Randomize( void )
{
    int index;
    int pos;
    int newpos;
    int count = m_Items.Count();
    guTrack SavedItem;

    if( m_CurItem > 0 )
    {
        SavedItem = m_Items[ 0 ];
        m_Items[ 0 ] = m_Items[ m_CurItem ];
        m_Items[ m_CurItem ] = SavedItem;
        m_CurItem = 0;
    }
    if( count > 2 )
    {
        for( index = 0; index < count; index++ )
        {
            do {
                pos = guRandom( count );
                newpos = guRandom( count );
            } while( ( pos == newpos ) || !pos || !newpos );
            SavedItem = m_Items[ pos ];
            m_Items[ pos ] = m_Items[ newpos ];
            m_Items[ newpos ] = SavedItem;
//            if( pos == m_CurItem )
//                m_CurItem = newpos;
//            else if( newpos == m_CurItem )
//                m_CurItem = pos;
            //wxMilliSleep( 1 );
           //guLogMessage( wxT( "%u -> %u" ), pos, newpos );
        }
        DoSelectAll( false );
        UpdateView();
    }
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::FindCoverFile( const wxString &DirName )
{
    wxDir           Dir;
    wxString        FileName;
    wxString        CurFile;
    wxString        SavedDir = wxGetCwd();
    wxString        RetVal = wxEmptyString;
    wxArrayString   CoverSearchWords;

    // Refresh the SearchCoverWords array
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
    }

    Dir.Open( DirName );
    wxSetWorkingDirectory( DirName );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES ) )
        {
            do {
                CurFile = FileName.Lower();

                if( SearchCoverWords( CurFile, CoverSearchWords ) )
                {
                    if( CurFile.EndsWith( wxT( ".jpg" ) ) ||
                        CurFile.EndsWith( wxT( ".png" ) ) ||
                        CurFile.EndsWith( wxT( ".bmp" ) ) ||
                        CurFile.EndsWith( wxT( ".gif" ) ) )
                    {
                        //printf( "Found Cover: " ); printf( CurFile.char_str() ); printf( "\n" );
                        RetVal = DirName + wxT( '/' ) + FileName;
                        break;
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
    }
    wxSetWorkingDirectory( SavedDir );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddPlayListItem( const wxString &FileName, bool AddPath )
{
    wxListItem ListItem;
    TagInfo Info;

    guTrack Song;
    wxString Len;
    wxURI UriPath( FileName );
    if( UriPath.IsReference() )
    {
        //guLogMessage( wxT( "AddPlaylistItem: '%s'" ), FileName.c_str() );

        //
        Song.m_FileName = wxEmptyString;

        if( AddPath )
        {
            Song.m_FileName = wxGetCwd() + wxT( "/" );
        }

        //Song.m_FileName +=  UriPath.Unescape( UriPath.GetPath() ); //.AfterLast( '/' );
        Song.m_FileName += FileName;

        if( wxFileExists( Song.m_FileName ) )
        {
            Song.m_SongId = 0;
            Song.m_CoverId = 0;
            //Song.m_Number = -1;

            //Song.m_SongId = 1;
            //guLogMessage( wxT( "Loading : %s" ), Song.m_FileName.c_str() );
            m_Db->FindTrackFile( Song.m_FileName, &Song );

            if( !Song.m_SongId )
            {
                Song.m_SongId = guPLAYLIST_NOTDBTRACK;

                Info.ReadID3Tags( Song.m_FileName );

                Song.m_ArtistName = Info.m_ArtistName;
                Song.m_AlbumName = Info.m_AlbumName;
                Song.m_SongName = Info.m_TrackName;
                Song.m_Number = Info.m_Track;
                Song.m_GenreName = Info.m_GenreName;
                Song.m_Length = Info.m_Length;
                Song.m_Year = Info.m_Year;
                Song.m_Rating = wxNOT_FOUND;
            }

            m_TotalLen += Song.m_Length;

            AddItem( Song );
        }
        else
        {
            guLogWarning( wxT( "Could not open the file '%s'" ), Song.m_FileName.c_str() );
        }
    }
    else
    {
        Song.m_SongId   = guPLAYLIST_RADIOSTATION;
        Song.m_CoverId  = 0;
        Song.m_FileName = FileName;
        Song.m_SongName = FileName;
        Song.m_Length   = 0;
        Song.m_Year     = 0;
        Song.m_Rating   = -1;
        AddItem( Song );
        //guLogMessage( wxT( "Added a radio stream" ) );
    }
}

// -------------------------------------------------------------------------------- //
void AddPlayListCommands( wxMenu * Menu, int SelCount )
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
                MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_COMMANDS + index, Names[ index ], Commands[ index ] );
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
void guPlayList::OnContextMenu( wxContextMenuEvent& event )
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

    wxArrayInt SelectedItems = GetSelectedItems();
    int SelCount = SelectedItems.Count();

    MenuItem = new wxMenuItem( &Menu, ID_PLAYER_PLAYLIST_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels of the current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_PLAYER_PLAYLIST_CLEAR, _( "Clear PlayList" ), _( "Remove all songs from PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_clear ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_PLAYER_PLAYLIST_SAVE, _( "Save PlayList" ), _( "Save the PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_PLAYER_PLAYLIST_REMOVE, _( "Remove selected songs" ), _( "Remove selected songs from PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY, _( "Randomize PlayList" ), _( "Randomize the songs in the PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playlist_shuffle ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_PLAYER_PLAYLIST_COPYTO, _( "Copy to..." ), _( "Copy the current playlist to a directory or device" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
    Menu.Append( MenuItem );

    AddPlayListCommands( &Menu, SelCount );

    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnClearClicked( wxCommandEvent &event )
{
    ClearItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnRemoveClicked( wxCommandEvent &event )
{
    RemoveSelected();
    RefreshItems();
    //PlayerPanel->UpdateTotalLength();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSaveClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt SelectedItems = GetSelectedItems();
    wxArrayInt Songs;

    if( ( count = SelectedItems.Count() ) )
    {
        for( index = 0; index < count; index++ )
        {
            if( m_Items[ SelectedItems[ index ] ].m_SongId > 0 )
                Songs.Add( m_Items[ SelectedItems[ index ] ].m_SongId );
        }
    }
    else
    {
        count = m_Items.Count();
        for( index = 0; index < count; index++ )
        {
            if( m_Items[ index ].m_SongId > 0 )
                Songs.Add( m_Items[ index ].m_SongId );
        }
    }

    if( Songs.Count() )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( wxTheApp->GetTopWindow(), _( "PlayList Name: " ), _( "Enter the new playlist name" ) );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            m_Db->CreateStaticPlayList( EntryDialog->GetValue(), Songs );
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks;
    wxArrayInt SelectedItems = GetSelectedItems();
    int index;
    int count = SelectedItems.Count();
    if( count )
    {
        Tracks = new guTrackArray();
        for( index = 0; index < count; index++ )
        {
            Tracks->Add( m_Items[ SelectedItems[ index ] ] );
        }
    }
    else
    {
        Tracks = new guTrackArray( m_Items );
    }

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Labels;
    wxArrayInt SongIds;
    //
    guTrack * Track;
    wxArrayInt SelectedItems = GetSelectedItems();
    int index;

    int count = SelectedItems.Count();
    if( count )
    {
        for( index = 0; index < count; index++ )
        {
            Track = &m_Items[ SelectedItems[ index ] ];
            if( Track->m_SongId > 0 )
            {
                SongIds.Add( Track->m_SongId );
            }
        }
    }
    else
    {
        // If there is no selection then use all songs that are
        // recognized in the database.
        count = m_Items.Count();
        for( index = 0; index < count; index++ )
        {
            Track = &m_Items[ index ];
            if( Track->m_SongId > 0 )
            {
                SongIds.Add( Track->m_SongId );
            }
        }
    }

    if( SongIds.Count() )
    {
        m_Db->GetLabels( &Labels, true );

        //SongIds = m_SongListCtrl->GetSelection();
        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Songs Labels Editor" ),
                             Labels, m_Db->GetSongsLabels( SongIds ) );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                m_Db->UpdateSongsLabels( SongIds, LabelEditor->GetCheckedIds() );
            }
            LabelEditor->Destroy();
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LABEL_UPDATELABELS );
            wxPostEvent( wxTheApp->GetTopWindow(), event );
        }
    }
}

// -------------------------------------------------------------------------------- //
int guPlayList::GetCaps()
{
//    NONE                  = 0x0000
//    CAN_GO_NEXT           = 0x0001
//    CAN_GO_PREV           = 0x0002
//   *CAN_PAUSE             = 0x0004
//   *CAN_PLAY              = 0x0008
//   *CAN_SEEK              = 0x0010
//    CAN_PROVIDE_METADATA  = 0x0020
//    CAN_HAS_TRACKLIST     = 0x0040
    int Caps = MPRIS_CAPS_NONE;
    if( m_Items.Count() )
    {
        if( m_CurItem < m_Items.Count() )
            Caps |= MPRIS_CAPS_CAN_GO_NEXT;
        if( m_CurItem > 0 )
            Caps |= MPRIS_CAPS_CAN_GO_PREV;
        Caps |= ( MPRIS_CAPS_CAN_PAUSE | MPRIS_CAPS_CAN_PLAY | MPRIS_CAPS_CAN_SEEK | MPRIS_CAPS_CAN_PROVIDE_METADATA );
    }
    Caps |= MPRIS_CAPS_CAN_HAS_TRACKLIST;
    return Caps;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCommandClicked( wxCommandEvent &event )
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

            index -= ID_PLAYER_PLAYLIST_COMMANDS;
            wxString CurCmd = Commands[ index ];

            if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
            {
                wxString Path = wxT( "\"" ) + wxPathOnly( m_Items[ Selection[ 0 ] ].m_FileName ) + wxT( "\"" );
                CurCmd.Replace( wxT( "{bp}" ), Path );
            }

            if( CurCmd.Find( wxT( "{bc}" ) ) != wxNOT_FOUND )
            {
                int CoverId = m_Items[ Selection[ 0 ] ].m_CoverId;
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                {
                    CoverPath = m_Db->GetCoverPath( CoverId );
                }
                else
                {
                    CoverPath = FindCoverFile( wxPathOnly( m_Items[ Selection[ 0 ] ].m_FileName ) );
                }

                if( !CoverPath.IsEmpty() )
                {
                    CurCmd.Replace( wxT( "{bc}" ), wxT( "\"" ) + CoverPath ) + wxT( "\"" );
                }
            }

            if( CurCmd.Find( wxT( "{tp}" ) ) != wxNOT_FOUND )
            {
                wxString SongList = wxEmptyString;
                count = Selection.Count();
                if( count )
                {
                    for( index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + m_Items[ Selection[ index ] ].m_FileName + wxT( "\"" );
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
void guPlayList::UpdatedTracks( const guTrackArray * tracks )
{
    // If there are no items in the playlist there is nothing to do
    if( !m_Items.Count() )
        return;

    bool found = false;
    int index;
    int count = tracks->Count();
    for( index = 0; index < count; index++ )
    {
        int item;
        int itemcnt = m_Items.Count();
        for( item = 0; item < itemcnt; item++ )
        {
            if( m_Items[ item ].m_SongId == ( * tracks )[ index ].m_SongId )
            {
                m_Items[ item ] = ( * tracks )[ index ];
                found = true;
                break;
            }
        }
    }
    if( found )
    {
        UpdateView();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::UpdatedTrack( const guTrack * track )
{
    // If there are no items in the playlist there is nothing to do
    if( !m_Items.Count() )
        return;

    bool found = false;
    int item;
    int itemcnt = m_Items.Count();
    for( item = 0; item < itemcnt; item++ )
    {
        if( m_Items[ item ].m_SongId == track->m_SongId )
        {
            m_Items[ item ] = * track;
            found = true;
            break;
        }
    }
    if( found )
    {
        UpdateView();
    }
}

// -------------------------------------------------------------------------------- //
// guAddDropFilesThread
// -------------------------------------------------------------------------------- //
guAddDropFilesThread::guAddDropFilesThread( guPlayListDropTarget * playlistdroptarget,
                             guPlayList * playlist, const wxArrayString &files ) :
    wxThread()
{
    m_PlayList = playlist;
    m_Files = files;
    m_PlayListDropTarget = playlistdroptarget;
}

// -------------------------------------------------------------------------------- //
guAddDropFilesThread::~guAddDropFilesThread()
{
//    printf( "guAddDropFilesThread Object destroyed\n" );
    if( !TestDestroy() )
    {
        m_PlayListDropTarget->ClearAddDropFilesThread();
    }
}

// -------------------------------------------------------------------------------- //
void guAddDropFilesThread::AddDropFiles( const wxString &DirName )
{
    wxDir Dir;
    wxString FileName;
    wxString SavedDir( wxGetCwd() );

    //printf( "Entering Dir : " ); printf( ( char * ) DirName.char_str() );  ; printf( "\n" );
    if( wxDirExists( DirName ) )
    {
        //wxMessageBox( DirName, wxT( "DirName" ) );
        Dir.Open( DirName );
        wxSetWorkingDirectory( DirName );
        if( Dir.IsOpened() )
        {
            if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
            {
                do {
                    if( ( FileName[ 0 ] != '.' ) )
                    {
                        if( Dir.Exists( FileName ) )
                        {
                            AddDropFiles( FileName );
                            // Update the guPlayList Object on every dir
                            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATELIST );
                            wxPostEvent( m_PlayList, event );
                        }
                        else
                        {
                            if( FileName.Lower().EndsWith( wxT( ".mp3" ) ) )
                            {
                                //guLogMessage( wxT( "Adding file %i '%s'" ), m_PlayList->GetCount(), FileName.c_str() );
                                m_PlayList->AddPlayListItem( FileName, true );
                            }
                        }
                    }
                } while( Dir.GetNext( &FileName ) && !TestDestroy() );
            }
        }
    }
    else
    {
        if( DirName.Lower().EndsWith( wxT( ".mp3" ) ) )
        {
            //guLogMessage( wxT( "Adding file %i '%s'" ), m_PlayList->GetCount(), FileName.c_str() );
            m_PlayList->AddPlayListItem( DirName, false );
        }
    }
    wxSetWorkingDirectory( SavedDir );
}

// -------------------------------------------------------------------------------- //
guAddDropFilesThread::ExitCode guAddDropFilesThread::Entry()
{
    int index;
    int Count = m_Files.Count();
    for( index = 0; index < Count; ++index )
    {
        if( TestDestroy() )
            break;
        AddDropFiles( m_Files[ index ] );
    }

    if( !TestDestroy() )
    {
        // Once finished send the update guPlayList event to the guPlayList object
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATELIST );
        //event.SetEventObject( ( wxObject * ) this );
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config->ReadBool( wxT( "DropFilesClearPlayList" ), false, wxT( "General" ) ) )
        {
            event.SetExtraLong( 1 );
        }
        wxPostEvent( m_PlayList, event );
    }
    //
    m_PlayList->m_DragOverItem = wxNOT_FOUND;


    return 0;
}

// -------------------------------------------------------------------------------- //
// guPlayListDropTarget
// -------------------------------------------------------------------------------- //
guPlayListDropTarget::guPlayListDropTarget( guPlayList * playlist )
{
    m_PlayList = playlist;
    m_AddDropFilesThread = NULL;
}

// -------------------------------------------------------------------------------- //
guPlayListDropTarget::~guPlayListDropTarget()
{
//    printf( "guPlayListDropTarget Object destroyed\n" );
}

// -------------------------------------------------------------------------------- //
bool guPlayListDropTarget::OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &files )
{
    // We are moving items inside this object.
    if( m_PlayList->m_DragSelfItems )
    {
        m_PlayList->MoveSelected();
        m_PlayList->UpdateView( false );
    }
    else
    {
        if( m_PlayList->GetCount() )
        {
            guConfig * Config = ( guConfig * ) guConfig::Get();
            if( Config->ReadBool( wxT( "DropFilesClearPlayList" ), false, wxT( "General" ) ) )
            {
                m_PlayList->ClearItems();
                m_PlayList->RefreshAll();
                m_PlayList->m_DragOverItem = wxNOT_FOUND;
                m_PlayList->m_CurItem = 0;
            }
            //guLogMessage( wxT( "ClearPlaylist set on config. Playlist cleared" ) );
        }

        //
        if( m_AddDropFilesThread )
        {
            m_AddDropFilesThread->Pause();
            m_AddDropFilesThread->Delete();
        }
        m_AddDropFilesThread = new guAddDropFilesThread( this, m_PlayList, files );
        if( m_AddDropFilesThread )
        {
            m_AddDropFilesThread->Create();
            m_AddDropFilesThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            m_AddDropFilesThread->Run();
        }
        else
        {
            guLogError( wxT( "Could not create the add files thread." ) );
        }
    }
    return true;
}

// -------------------------------------------------------------------------------- //
wxDragResult guPlayListDropTarget::OnDragOver( wxCoord x, wxCoord y, wxDragResult def )
{
    //printf( "guPlayListDropTarget::OnDragOver... %d - %d\n", x, y );
    m_PlayList->OnDragOver( x, y );
    return wxDragCopy;
}

// -------------------------------------------------------------------------------- //
