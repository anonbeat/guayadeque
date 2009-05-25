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
#include <wx/types.h>
#include <wx/uri.h>

#include "Config.h"
#include "Commands.h"
#include "Images.h"
#include "MainApp.h"
#include "Shoutcast.h"
#include "TagInfo.h"
#include "Utils.h"

//#include <id3/tag.h>
//#include <id3/misc_support.h>

#define GUPLAYLIST_ITEM_SIZE        40

// -------------------------------------------------------------------------------- //
// guPlayList
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guPlayList,wxVListBox)
  EVT_MOUSE_EVENTS   (guPlayList::OnMouse)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guPlayList::guPlayList( wxWindow * parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style ) :
            wxVListBox( parent, id, pos, size, style | wxLB_MULTIPLE )
{
    wxArrayString Songs;
    int Count;
    int Index;
    guConfig * Config;

    m_TotalLen = 0;
    m_CurItem = wxNOT_FOUND;

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
            m_SmartPlayMaxPlayListTracks = Config->ReadNum( wxT( "SmartPlayMaxPlayListTracks" ), 15, wxT( "SmartPlayList" ) );
            //
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATELIST );
            //event.SetEventObject( ( wxObject * ) this );
            wxPostEvent( this, event );
        }
    }

    SetDropTarget( new guPlayListDropTarget( this ) );

    m_PlayBgColor  = wxColor( 0, 0, 0 ); //SystemSettings.GetColour( wxSYS_COLOUR_HIGHLIGHT );
    m_PlayFgColor  = wxColor( 255, 255, 255 ); //SystemSettings.GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT );
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

    SetBackgroundColour( m_EveBgColor );

    m_PlayBitmap = new wxBitmap( guImage_media_playback_start );

	Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guPlayList::OnKeyDown ), NULL, this );
    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guPlayList::OnBeginDrag ), NULL, this );
    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guPlayList::OnContextMenu ), NULL, this );
    Connect( ID_PLAYLIST_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnClearClicked ) );
    Connect( ID_PLAYLIST_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnRemoveClicked ) );
    Connect( ID_PLAYLIST_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSaveClicked ) );
    Connect( ID_PLAYLIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCopyToClicked ) );

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

    //Disconnect( wxID_ANY, EVT_UPDATEDPLAYLIST_EVENT, UpdatedPlayListEventHandler( guPlayList::OnUpdatedItems ), NULL, this );
	Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guPlayList::OnKeyDown ), NULL, this );
    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guPlayList::OnBeginDrag ), NULL, this );
    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guPlayList::OnContextMenu ), NULL, this );
    Disconnect( ID_PLAYLIST_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnClearClicked ) );
    Disconnect( ID_PLAYLIST_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnRemoveClicked ) );
    Disconnect( ID_PLAYLIST_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSaveClicked ) );
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
void guPlayList::RemoveSelected()
{
    int Index;
    int item;
    wxArrayInt Selection = GetSelectedItems();
    for( Index = Selection.Count() - 1; Index >= 0; Index-- )
    {
        item = Selection[ Index ];
        m_TotalLen -= m_Items[ item ].m_Length;
        m_Items.RemoveAt( item );
        if( item == m_CurItem )
            m_CurItem = wxNOT_FOUND;
        else if( item < m_CurItem )
            m_CurItem--;
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
void guPlayList::AddToPlayList( const guTrackArray &NewItems, const bool DeleteOld )
{
    int Index;
    int Count;
    Count = NewItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
      m_Items.Add( NewItems[ Index ] );
      m_TotalLen += NewItems[ Index ].m_Length;
      if( DeleteOld && ( m_CurItem != 0 ) && m_Items.Count() > ( size_t ) m_SmartPlayMaxPlayListTracks )
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
        m_DragOverAfter = ( y > ( ( ( int ) m_DragOverItem + 1 ) * GUPLAYLIST_ITEM_SIZE ) - ( GUPLAYLIST_ITEM_SIZE / 2 ) );
        RefreshLines( wxMax( ( int ) m_DragOverItem - 1, 0 ), wxMin( ( ( int ) m_DragOverItem + 3 ), GetCount() ) );
    }
    //printf( "DragOverItem: %d ( %d, %d )\n", DragOverItem, x, y );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const
{
    guTrack Item;
    wxRect CutRect;
    wxSize TimeLen;
    wxString TimeStr;
//    wxArrayInt Selection;
    wxFont Font( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );

    if( ( int ) n == wxNOT_FOUND )
        return;

    Item = m_Items[ n ];
    dc.SetFont( Font  );
    dc.SetBackgroundMode( wxTRANSPARENT );
    if( n == ( size_t ) m_CurItem )
    {
        dc.SetTextForeground( m_PlayFgColor );
    }
    else if( IsSelected( n ) )
    {
        dc.SetTextForeground( m_SelFgColor );
    }
    else
    {
        dc.SetTextForeground( m_TextFgColor );
    }

    if( Item.m_SongId != guPLAYLIST_RADIOSTATION )
    {
        //y = dc.GetCharHeight();
        dc.DrawText( Item.m_SongName, rect.x + 5, rect.y + 5 );
        //Font.SetPointSize( 7 );
        Font.SetStyle( wxFONTSTYLE_ITALIC );
        Font.SetWeight( wxFONTWEIGHT_NORMAL );
        dc.SetFont( Font );
        dc.DrawText( Item.m_ArtistName + wxT( " - " ) + Item.m_AlbumName, rect.x + 5, rect.y + ( dc.GetCharHeight() + 10 ) );
        //
        TimeStr = LenToString( Item.m_Length );
        TimeLen = dc.GetTextExtent( TimeStr );
        CutRect = rect;
        CutRect.x += CutRect.width - ( TimeLen.GetWidth() + 6 );
        CutRect.width -= CutRect.width - ( TimeLen.GetWidth() + 6 );
        OnDrawBackground( dc, CutRect, n );
        dc.DrawText( TimeStr, CutRect.x + 3, CutRect.y + 5 );
        if( n == ( size_t ) m_CurItem && m_PlayBitmap )
        {
            // Draw Play bitmap
            dc.DrawBitmap( * m_PlayBitmap, CutRect.x + 8, CutRect.y + 15, true );
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
    else if( n == ( size_t ) m_CurItem )
      dc.SetBrush( wxBrush( m_PlayBgColor ) );
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
void guPlayList::UpdateView()
{
    if( !IsVisible( m_CurItem ) )
    {
        ScrollToLine( m_CurItem );
    }
    //RefreshLines( GetVisibleBegin(), GetVisibleEnd() );
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

    if( count )
    {
        for( index = 0; index < count; index++ )
        {
            do {
                pos = guRandom( count );
                newpos = guRandom( count );
            } while( pos == newpos );
            //guLogMessage( wxT( "<--Current: %u  Pos : %u  NewPos : %u" ), CurItem, pos, newpos );
            SavedItem = m_Items[ pos ];
            m_Items[ pos ] = m_Items[ newpos ];
            m_Items[ newpos ] = SavedItem;
            if( pos == m_CurItem )
                m_CurItem = newpos;
            else if( newpos == m_CurItem )
                m_CurItem = pos;
            //wxMilliSleep( 1 );
            //guLogMessage( wxT( "-->Current: %u  Pos : %u  NewPos : %u" ), CurItem, pos, newpos );
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
//    ID3_Tag tag;
//    char * pStr;

    //guTrack PlayListSong;
    guTrack Song;
    wxString Len;
    wxURI UriPath( FileName );
    if( UriPath.IsReference() )
    {
        //guLogMessage( wxT( "AddPlaylistItem: '%s'" ), FileName.c_str() );
        //
        Song.m_FileName = wxEmptyString;
        Song.m_CoverId = 0;
        //Song.m_Number = -1;

        Song.m_SongId = 1;

        if( AddPath )
        {
            Song.m_FileName = wxGetCwd() + wxT( "/" );
        }

        Song.m_FileName +=  FileName; //.AfterLast( '/' );

        Info.ReadID3Tags( FileName );

        Song.m_ArtistName = Info.m_ArtistName;
        Song.m_AlbumName = Info.m_AlbumName;
        Song.m_SongName = Info.m_TrackName;
        Song.m_Number = Info.m_Track;
        Song.m_GenreName = Info.m_GenreName;
        Song.m_Length = Info.m_Length;
        m_TotalLen += Info.m_Length;

        AddItem( Song );
    }
    else
    {
        Song.m_SongId   = guPLAYLIST_RADIOSTATION;
        Song.m_CoverId  = 0;
        Song.m_FileName = FileName;
        Song.m_SongName = FileName;
        Song.m_Length   = 0;
        AddItem( Song );
        //guLogMessage( wxT( "Added a radio stream" ) );
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

    MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_CLEAR, _( "Clear PlayList" ), _( "Remove all songs from PlayList" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_edit_clear ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_REMOVE, _( "Remove selected songs" ), _( "Remove selected songs from PlayList" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_edit_delete ) );
    Menu.Append( MenuItem );

//    MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_SAVE, _( "Save PlayList" ), _( "Save the PlayList" ) );
//    MenuItem->SetBitmap( wxBitmap( guImage_document_save ) );
//    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_COPYTO, _( "Copy to..." ), _( "Copy the current playlist to a directory or device" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_edit_copy ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_RANDOMPLAY, _( "Randomize PlayList" ), _( "Randomize the songs in the PlayList" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_media_playlist_shuffle ) );
    Menu.Append( MenuItem );

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
    // Use constants to avoid include mpris.h only for this
    int Caps = 0x0000;
    if( m_Items.Count() )
    {
        if( m_CurItem < m_Items.Count() )
            Caps |= 0x0001;
        if( m_CurItem > 0 )
            Caps |= 0x0010;
        Caps |= ( 0x0004 | 0x0008 | 0x0010 | 0x0020 );
    }
    Caps |= 0x0040;
    return Caps;
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
                            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATELIST );
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
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATELIST );
        //event.SetEventObject( ( wxObject * ) this );
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
        m_PlayList->UpdateView();
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
