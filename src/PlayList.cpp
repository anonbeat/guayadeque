// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#include "dbus/mpris.h"
#include "Images.h"
#include "LabelEditor.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "PlayerPanel.h"
#include "PlayListAppend.h"
#include "PlayListFile.h"
#include "Shoutcast.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Utils.h"

#include <wx/types.h>
#include <wx/uri.h>

// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
guPlayerPlayList::guPlayerPlayList( wxWindow * parent, guDbLibrary * db ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( -1, -1 ), wxTAB_TRAVERSAL )
{
    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    m_PlayListCtrl = new guPlayList( this, db );
    MainSizer->Add( m_PlayListCtrl, 1, wxEXPAND, 5 );

	SetSizer( MainSizer );
	Layout();
	MainSizer->Fit( this );
}

// -------------------------------------------------------------------------------- //
void guPlayerPlayList::SetPlayerPanel( guPlayerPanel * player )
{
    m_PlayListCtrl->SetPlayerPanel( player );
}

// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
guPlayList::guPlayList( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel ) :
            guListView( parent, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG |
                guLISTVIEW_ALLOWDROP | guLISTVIEW_DRAGSELFITEMS | guLISTVIEW_HIDE_HEADER )
{
    wxArrayString Songs;
    int Count;
    int Index;
    m_ItemHeight = 40;
//    m_PlayedColor = wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT );

    InsertColumn( new guListViewColumn( _( "Now Playing" ), 0 ) );

    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_TotalLen = 0;
    m_CurItem = wxNOT_FOUND;
    m_StartPlaying = false;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_MaxPlayedTracks = Config->ReadNum( wxT( "MaxTracksPlayed" ), 15, wxT( "Playback" ) );
    m_MinPlayListTracks = Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) );
    m_DelTracksPLayed = Config->ReadNum( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) );

    guMainApp * MainApp = ( guMainApp * ) wxTheApp;
    if( MainApp && MainApp->argc > 1 )
    {
        Count = MainApp->argc;
        for( Index = 1; Index < Count; Index++ )
        {
            //guLogMessage( wxT( "%u-%u %s" ), Index, MainApp->argc, MainApp->argv[ Index ] );
            //if( guIsValidAudioFile( MainApp->argv[ Index ] ) ||
            //    guPlayListFile::IsValidPlayList( MainApp->argv[ Index ] ) )
            {
                AddPlayListItem( MainApp->argv[ Index ] );
                m_StartPlaying = true;
            }
        }
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATELIST );
        //event.SetEventObject( ( wxObject * ) this );
        event.SetInt( 1 );
        wxPostEvent( this, event );
    }
    else
    {
        // Load the saved guPlayList
        Songs = Config->ReadAStr( wxT( "PlayListSong" ), wxEmptyString, wxT( "PlayList" ) );
        Count = Songs.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            AddPlayListItem( Songs[ Index ], false );
        }
        //
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATELIST );
        //event.SetEventObject( ( wxObject * ) this );
        event.SetInt( 1 );
        wxPostEvent( this, event );
    }
    m_CurItem = Config->ReadNum( wxT( "PlayerCurItem" ), wxNOT_FOUND, wxT( "General" ) );
    if( ( size_t ) m_CurItem > m_Items.Count() )
        m_CurItem = wxNOT_FOUND;

    m_PlayBitmap = new wxBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    m_NormalStar   = new wxBitmap( guImage( guIMAGE_INDEX_star_normal_tiny ) );
    m_SelectStar = new wxBitmap( guImage( guIMAGE_INDEX_star_highlight_tiny ) );

//    Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guPlayList::OnBeginDrag ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnClearClicked ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnRemoveClicked ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSaveClicked ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCopyToClicked ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnEditLabelsClicked ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnEditTracksClicked ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSearchClicked ), NULL, this );

    Connect( ID_PLAYER_PLAYLIST_SELECT_TITLE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectTrack ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectArtist ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SELECT_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectAlbum ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SELECT_YEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectYear ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SELECT_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectGenre ), NULL, this );

    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSearchLinkClicked ) );
    Connect( ID_PLAYER_PLAYLIST_COMMANDS, ID_PLAYER_PLAYLIST_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCommandClicked ) );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPlayList::OnConfigUpdated ), NULL, this );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPlayList::~guPlayList()
{
    // Save the guPlayList so it can be reload next time
    wxArrayString Songs;
    int Count;
    int Index;
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->UnRegisterObject( this );

        if( Config->ReadBool( wxT( "SavePlayListOnClose" ), true, wxT( "General" ) ) )
        {

            Count = m_Items.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                Songs.Add( m_Items[ Index ].m_FileName );
            }
            Config->WriteNum( wxT( "PlayerCurItem" ), m_CurItem, wxT( "General" ) );
        }
        else
        {
            Config->WriteNum( wxT( "PlayerCurItem" ), wxNOT_FOUND, wxT( "General" ) );
        }
        Config->WriteAStr( wxT( "PlayListSong" ), Songs, wxT( "PlayList" ) );
    }


    if( m_PlayBitmap )
      delete m_PlayBitmap;
    if( m_NormalStar )
      delete m_NormalStar;
    if( m_SelectStar )
      delete m_SelectStar;

//    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guPlayList::OnBeginDrag ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnClearClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnRemoveClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSaveClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCopyToClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnEditLabelsClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnEditTracksClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_SEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSearchClicked ) );

    Disconnect( ID_PLAYER_PLAYLIST_SELECT_TITLE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectTrack ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectArtist ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_SELECT_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectAlbum ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_SELECT_YEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectYear ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_SELECT_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSelectGenre ), NULL, this );

    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnSearchLinkClicked ) );
    Disconnect( ID_PLAYER_PLAYLIST_COMMANDS, ID_PLAYER_PLAYLIST_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayList::OnCommandClicked ) );

    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPlayList::OnConfigUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnConfigUpdated( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_MaxPlayedTracks = Config->ReadNum( wxT( "MaxTracksPlayed" ), 15, wxT( "Playback" ) );
        m_MinPlayListTracks = Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) );
        m_DelTracksPLayed = Config->ReadNum( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropBegin( void )
{
    if( GetItemCount() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config->ReadBool( wxT( "DropFilesClearPlayList" ), false, wxT( "General" ) ) )
        {
            ClearItems();
            RefreshAll();
            m_DragOverItem = wxNOT_FOUND;
            //m_CurItem = wxNOT_FOUND;
            //guLogMessage( wxT( "ClearPlaylist set on config. Playlist cleared" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropFile( const wxString &filename )
{
    if( guIsValidAudioFile( filename ) ||
        guPlayListFile::IsValidPlayList( filename ) )
    {
        //guLogMessage( wxT( "Adding file '%s'" ), filename.c_str() );
        AddPlayListItem( filename, false );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropEnd( void )
{
    // Once finished send the update guPlayList event to the guPlayList object
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATELIST );
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config->ReadBool( wxT( "DropFilesClearPlayList" ), false, wxT( "General" ) ) )
    {
        event.SetExtraLong( 1 );
    }
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
int  guPlayList::GetDragFiles( wxFileDataObject * files )
{
    int index;
    int count;
    wxArrayInt Selection = GetSelectedItems( false );
    count = Selection.Count();
    for( index = 0; index < count; index++ )
    {
        wxString FileName = m_Items[ Selection[ index ] ].m_FileName;
        FileName.Replace( wxT( "#" ), wxT( "%23" ) );
        files->AddFile( FileName );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
void guPlayList::RemoveItem( int itemnum )
{
    wxMutexLocker Lock( m_ItemsMutex );
    int count = m_Items.Count();
    if( count && ( itemnum >= 0 ) && ( itemnum < count ) )
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
    int index;
    int count;
    wxArrayInt Selected = GetSelectedItems( false );
    count = Selected.Count();
    for( index = count - 1; index >= 0; index-- )
    {
        RemoveItem( Selected[ index ] );
    }
    ClearSelectedItems();
}

//// -------------------------------------------------------------------------------- //
//static void PrintItems( const guTrackArray &Songs, int IP, int SI, int CI )
//{
//    int Index;
//    int Count = Songs.Count();
//    printf( "SI: %d  IP: %d  CI: %d\n", SI, IP, CI );
//    for( Index = 0; Index < Count; Index++ )
//    {
//        printf( "%02d ", Songs[ Index ].m_Number );
//    }
//    printf( "\n" );
//}

// -------------------------------------------------------------------------------- //
void guPlayList::MoveSelection( void )
{
    //
    // Move the Selected Items to the DragOverItem and DragOverFirst
    //
    int     InsertPos;
    int     Index;
    int     Count;
    bool    CurItemSet = false;
    guTrackArray MoveItems;
    wxArrayInt Selection = GetSelectedItems( false );
    if( m_DragOverItem == wxNOT_FOUND )
    {
        m_DragOverItem = m_Items.Count() - 1;
        m_DragOverAfter = true;
    }

        m_ItemsMutex.Lock();

        // Where is the Items to be moved
        InsertPos = m_DragOverAfter ? m_DragOverItem + 1 : m_DragOverItem;
        // How Many elements to move
        Count = Selection.Count();
        //PrintItems( m_Items, InsertPos, Selection[ 0 ], m_CurItem );
        // Get a copy of every element to move
        for( Index = 0; Index < Count; Index++ )
        {
            MoveItems.Add( m_Items[ Selection[ Index ] ] );
        }

        // Remove the Items and move CurItem and InsertPos
        // We move from last (bigger) to first
        for( Index = Count - 1; Index >= 0; Index-- )
        {
            //guLogMessage( wxT( "%i) ci:%i ip:%i" ), Index, m_CurItem, InsertPos );
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

        //PrintItems( m_Items, InsertPos, Selection[ 0 ], m_CurItem );

        // Insert every element at the InsertPos
        for( Index = 0; Index < Count; Index++ )
        {
            m_Items.Insert( MoveItems[ Index ], InsertPos );
            if( !CurItemSet && ( InsertPos <= m_CurItem ) )
                m_CurItem++;
            InsertPos++;
        }

        //PrintItems( m_Items, InsertPos, Selection[ 0 ], m_CurItem );
        m_ItemsMutex.Unlock();
    ClearSelectedItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        RemoveSelected();
        ReloadItems();
        return;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddToPlayList( const guTrackArray &items, const bool deleteold, const bool aftercurrent )
{
    wxMutexLocker Lock( m_ItemsMutex );
    int Index;
    int Count;
//    if( m_CurItem == wxNOT_FOUND )
//        m_CurItem = 0;

    Count = items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
      if( !aftercurrent )
      {
        m_Items.Add( items[ Index ] );
      }
      else
      {
        m_Items.Insert( items[ Index ], m_CurItem + 1 + Index );
      }
      m_TotalLen += items[ Index ].m_Length;

      while( deleteold && ( m_CurItem != 0 ) && ( ( m_CurItem ) > m_MaxPlayedTracks ) )
      {
        m_TotalLen -= m_Items[ 0 ].m_Length;
        m_Items.RemoveAt( 0 );
        m_CurItem--;
      }
    }
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetPlayList( const guTrackArray &NewItems )
{
    wxMutexLocker Lock( m_ItemsMutex );
    int Index;
    int Count;
    m_Items = NewItems;

    SetSelection( -1 );

    m_CurItem = 0;
    Count = m_Items.Count();
    m_TotalLen = 0;
    for( Index = 0; Index < Count; Index++ )
    {
      m_TotalLen += m_Items[ Index ].m_Length;
    }
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    guTrack Item;
    wxRect CutRect;
    wxSize TextSize;
    wxString TimeStr;
//    int OffsetSecLine;
//    wxArrayInt Selection;

    Item = m_Items[ row ];
    m_Attr.m_Font->SetPointSize( 8 );
    m_Attr.m_Font->SetStyle( wxFONTSTYLE_NORMAL );
    m_Attr.m_Font->SetWeight( wxFONTWEIGHT_BOLD );

    dc.SetFont( * m_Attr.m_Font );
    dc.SetBackgroundMode( wxTRANSPARENT );
    if( IsSelected( row ) )
    {
        dc.SetTextForeground( m_Attr.m_SelFgColor );
    }
    else if( row == m_CurItem )
    {
        dc.SetTextForeground( m_Attr.m_SelBgColor );
    }
    else
    {
        //dc.SetTextForeground( row > m_CurItem ? m_Attr.m_TextFgColor : m_PlayedColor );
        dc.SetTextForeground( m_Attr.m_TextFgColor );
    }


    // Draw the Items Texts
    CutRect = rect;

    // Draw Play bitmap
    if( row == m_CurItem && m_PlayBitmap )
    {
        dc.DrawBitmap( * m_PlayBitmap, CutRect.x + 2, CutRect.y + 10, true );
        CutRect.x += 16;
        CutRect.width -= 16;
    }

    // The DB or NODB Tracks
    if( Item.m_Type < guTRACK_TYPE_RADIOSTATION ||
        Item.m_Type == guTRACK_TYPE_PODCAST )
    {
        CutRect.width -= ( 50 + 6 + 2 );

        dc.SetClippingRegion( CutRect );

        //dc.DrawText( ( Item.m_Number ? wxString::Format( wxT( "%02u - " ), Item.m_Number ) :
        //                  wxT( "" ) ) + Item.m_SongName, CutRect.x + 4, CutRect.y + 4 );
        dc.DrawText( Item.m_SongName, CutRect.x + 4, CutRect.y + 4 );

        //m_Attr.m_Font->SetPointSize( 7 );
        //m_Attr.m_Font->SetStyle( wxFONTSTYLE_ITALIC );
        m_Attr.m_Font->SetWeight( wxFONTWEIGHT_NORMAL );
        dc.SetFont( * m_Attr.m_Font );

        dc.DrawText( Item.m_ArtistName + wxT( " - " ) + Item.m_AlbumName, CutRect.x + 4, CutRect.y + m_SecondLineOffset );

        dc.DestroyClippingRegion();

        // Draw the length and rating
        CutRect = rect;
        CutRect.x += ( CutRect.width - ( 50 + 6 ) );
        CutRect.width = ( 50 + 6 );

        dc.SetClippingRegion( CutRect );

        //m_Attr.m_Font->SetPointSize( 8 );
        //m_Attr.m_Font->SetStyle( wxFONTSTYLE_NORMAL );
        //dc.SetFont( * m_Attr.m_Font );

        TimeStr = LenToString( Item.m_Length );
        TextSize = dc.GetTextExtent( TimeStr );
        dc.DrawText( TimeStr, CutRect.x + ( ( 56 - TextSize.GetWidth() ) / 2 ), CutRect.y + 4 );
        //guLogMessage( wxT( "%i - %i" ), TextSize.GetWidth(), TextSize.GetHeight() );


        if( Item.m_Type < guTRACK_TYPE_RADIOSTATION )
        {
            // Draw the rating
            int index;
            //OffsetSecLine += 2;
            CutRect.x += 1;
            CutRect.y += 2;
            for( index = 0; index < 5; index++ )
            {
               dc.DrawBitmap( ( index >= Item.m_Rating ) ? * m_NormalStar : * m_SelectStar,
                              CutRect.x + ( 11 * index ), CutRect.y + m_SecondLineOffset, true );
            }
        }
    }
    else
    {
        dc.DrawText( Item.m_SongName, CutRect.x + 4, CutRect.y + 13 );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnMouse( wxMouseEvent &event )
{
    if( event.LeftDown() || event.LeftUp() )
    {
        int x = event.m_x;
        int y = event.m_y;
        wxSize Size = m_ListBox->GetClientSize();
        if( x >= ( Size.GetWidth() - ( 50 + 6 ) ) )
        {
            int Item = HitTest( x, y );
            if( Item != wxNOT_FOUND && m_Items[ Item ].m_Type == guTRACK_TYPE_DB )
            {
                if( ( size_t ) y > ( ( Item - GetFirstVisibleLine() ) * m_ItemHeight ) + m_SecondLineOffset )
                {
                    if( event.LeftDown() )
                    {
                        int Rating;
                        x -= ( Size.GetWidth() - ( 50 + 6 ) );

                        if( x < 3 )
                            Rating = 0;
                        else
                            Rating = wxMin( 5, ( wxMax( 0, x ) / 11 ) + 1 );

                        if( m_Items[ Item ].m_Rating == Rating )
                        {
                            Rating = 0;
                        }
                        m_Items[ Item ].m_Rating = Rating;
                        RefreshLine( Item );
//                        if( Item == m_CurItem )
//                        {
//                            m_PlayerPanel->SetRating( Rating );
//                        }
                        m_Db->SetTrackRating( m_Items[ Item ].m_SongId, Rating );

                        // Update the track in database, playlist, etc
                        ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTrack( guUPDATED_TRACKS_PLAYER_PLAYLIST, &m_Items[ Item ] );
                    }
                    return;
                }
            }
        }
    }

    // Do the inherited procedure
    guListView::OnMouse( event );
}

// -------------------------------------------------------------------------------- //
wxCoord guPlayList::OnMeasureItem( size_t n ) const
{
    int Height = 4;
    // Code taken from the generic/listctrl.cpp file
    guPlayList * self = wxConstCast( this, guPlayList );

    wxClientDC dc( self );
    wxFont Font = GetFont();
    Font.SetPointSize( 8 );
    dc.SetFont( Font );

    wxCoord y;
    dc.GetTextExtent( wxT( "Hg" ), NULL, &y );
    Height += y + 2;
    self->m_SecondLineOffset = Height;

//    Font.SetPointSize( 7 );
//    dc.SetFont( Font );
//    dc.GetTextExtent( wxT( "Hg" ), NULL, &y );
    Height += y + 4;

    self->SetItemHeight( Height );
    self->m_ItemHeight = Height;

//    guLogMessage( wxT( "PlayList::OnMeasureItem %i  %i" ), m_SecondLineOffset, Height );

    return Height;
}

// -------------------------------------------------------------------------------- //
void guPlayList::DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    wxRect LineRect;

    if( row == ( int ) m_DragOverItem )
      dc.SetBrush( m_Attr.m_DragBgColor );
    //else if( n == ( size_t ) GetSelection() )
    else if( IsSelected( row ) )
      dc.SetBrush( wxBrush( m_Attr.m_SelBgColor ) );
//    else if( n == ( size_t ) m_CurItem )
//      dc.SetBrush( wxBrush( m_PlayBgColor ) );
    else
      dc.SetBrush( wxBrush( row & 1 ? m_Attr.m_OddBgColor : m_Attr.m_EveBgColor ) );

    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.DrawRectangle( rect );

    if( row == ( int ) m_DragOverItem )
    {
        LineRect = rect;
        if( m_DragOverAfter )
            LineRect.y += ( LineRect.height - 2 );
        LineRect.height = 2;
        dc.SetBrush( * wxBLACK_BRUSH );
        dc.DrawRectangle( LineRect );
    }
}

// -------------------------------------------------------------------------------- //
long guPlayList::GetCount()
{
    return m_Items.GetCount();
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::OnGetItemText( const int row, const int col ) const
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guPlayList::GetItemsList( void )
{
}

// -------------------------------------------------------------------------------- //
void guPlayList::ReloadItems( bool reset )
{
    SetItemCount( GetCount() );
    RefreshAll( m_CurItem );
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddItem( const guTrack &NewItem, const int pos )
{
    int InsertPos;
    if( m_DragOverItem != wxNOT_FOUND )
    {
        InsertPos = m_DragOverAfter ? m_DragOverItem + 1 : m_DragOverItem;
        if( InsertPos <= m_CurItem )
            m_CurItem++;
        //guLogMessage( wxT( "Inserted at %i %i" ), m_DragOverItem, m_DragOverAfter );
        m_Items.Insert( NewItem, InsertPos );
        if( m_DragOverAfter )
            m_DragOverItem++;
        m_DragOverAfter = true;
    }
    else
    {
        InsertPos = pos;
        if( InsertPos < 0 || InsertPos > ( int ) m_Items.Count() )
            InsertPos = wxNOT_FOUND;
        if( InsertPos != wxNOT_FOUND )
        {
            if( InsertPos <= m_CurItem )
                m_CurItem++;
            m_Items.Insert( NewItem, InsertPos );
        }
        else
        {
            m_Items.Add( NewItem );
        }
    }
}

//// -------------------------------------------------------------------------------- //
//void guPlayList::AddItem( const guTrack * NewItem )
//{
//    AddItem( * NewItem );
//}
//
// -------------------------------------------------------------------------------- //
void guPlayList::SetCurrent( int curitem, bool delold )
{
    if( delold && ( curitem != m_CurItem ) && ( m_CurItem != wxNOT_FOUND ) &&
        ( ( size_t ) m_CurItem < m_Items.Count() ) )
    {
        m_TotalLen -= m_Items[ m_CurItem ].m_Length;
        m_Items.RemoveAt( m_CurItem );
        if( m_CurItem < curitem )
            curitem--;
        ReloadItems();
    }

    if( curitem >= 0 && curitem <= GetCount() )
        m_CurItem = curitem;
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
guTrack * guPlayList::GetNext( const int playloop, const bool forceskip )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( !forceskip && playloop == guPLAYER_PLAYLOOP_TRACK )
        {
            return &m_Items[ m_CurItem ];
        }
        else if( ( m_CurItem < ( ( int ) m_Items.Count() - 1 ) ) )
        {
            if( m_DelTracksPLayed && !playloop )
            {
                m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                m_Items.RemoveAt( m_CurItem );
                ReloadItems();
            }
            else
                m_CurItem++;
            return &m_Items[ m_CurItem ];

        }
        else if( playloop == guPLAYER_PLAYLOOP_PLAYLIST )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetPrev( const int playloop, const bool forceskip )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( !forceskip && playloop == guPLAYER_PLAYLOOP_TRACK )
        {
            return &m_Items[ m_CurItem ];
        }
        else if( m_CurItem > 0 )
        {
            if( m_DelTracksPLayed && !playloop )
            {
                m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                m_Items.RemoveAt( m_CurItem );
                ReloadItems();
            }
            m_CurItem--;
            return &m_Items[ m_CurItem ];
        }
        else if( playloop == guPLAYER_PLAYLOOP_PLAYLIST )
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
    ClearSelectedItems();
    ReloadItems();
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

    if( count > 2 )
    {
        if( m_CurItem > 0 )
        {
            SavedItem = m_Items[ 0 ];
            m_Items[ 0 ] = m_Items[ m_CurItem ];
            m_Items[ m_CurItem ] = SavedItem;
            m_CurItem = 0;
        }
        for( index = 0; index < count; index++ )
        {
            do {
                pos = guRandom( count );
                newpos = guRandom( count );
            } while( ( pos == newpos ) || ( ( m_CurItem == 0 ) && ( !pos || !newpos ) ) );
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
        ClearSelectedItems();
        Refresh( m_CurItem );
    }
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::FindCoverFile( const wxString &dirname )
{
    wxDir           Dir;
    wxString        DirName;
    wxString        FileName;
    wxString        CurFile;
    wxString        RetVal = wxEmptyString;
    wxArrayString   CoverSearchWords;

    DirName = dirname;
    if( !DirName.EndsWith( wxT( "/" ) ) )
        DirName += wxT( '/' );

    // Refresh the SearchCoverWords array
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
    }

    Dir.Open( DirName );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES ) )
        {
            do {
                CurFile = FileName.Lower();
                //guLogMessage( wxT( "Searching %s : %s" ), DirName.c_str(), FileName.c_str() );

                if( SearchCoverWords( CurFile, CoverSearchWords ) )
                {
                    if( CurFile.EndsWith( wxT( ".jpg" ) ) ||
                        CurFile.EndsWith( wxT( ".jpeg" ) ) ||
                        CurFile.EndsWith( wxT( ".png" ) ) ||
                        CurFile.EndsWith( wxT( ".bmp" ) ) ||
                        CurFile.EndsWith( wxT( ".gif" ) ) )
                    {
                        //printf( "Found Cover: " ); printf( CurFile.char_str() ); printf( "\n" );
                        RetVal = DirName + FileName;
                        break;
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddPlayListItem( const wxString &filename, bool addpath, const int pos )
{
    // Check if its a uri or a filename
    int Index;
    int Count;
    wxString FileName;
    guTrack Track;
    guPodcastItem PodcastItem;
    FileName = ( addpath ? wxGetCwd() + wxT( "/" ) : wxT( "" ) ) + filename;

    wxURI Uri( FileName );

    guLogMessage( wxT( "Loading %s" ), FileName.c_str() );

    // If its a playlist
    if( guPlayListFile::IsValidPlayList( Uri.GetPath() ) )
    {
        int InsertPos = pos;
        guPlayListFile PlayList( FileName );
        if( ( Count = PlayList.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                AddPlayListItem( PlayList.GetItem( Index ).m_Location, InsertPos++ );
            }
        }
    }
    else if( Uri.IsReference() )    // Its a file
    {
        if( wxFileExists( FileName ) )
        {
            if( guIsValidAudioFile( FileName ) )
            {
                Track.m_FileName = FileName;

                if( !m_Db->FindTrackFile( FileName, &Track ) )
                {
                    if( m_Db->GetPodcastItemFile( FileName, &PodcastItem ) )
                    {
                        Track.m_Type = guTRACK_TYPE_PODCAST;
                        Track.m_SongId = PodcastItem.m_Id;
                        Track.m_SongName = PodcastItem.m_Title;
                        Track.m_ArtistName = PodcastItem.m_Author;
                        Track.m_AlbumId = PodcastItem.m_ChId;
                        Track.m_AlbumName = PodcastItem.m_Channel;
                        Track.m_Length = PodcastItem.m_Length;
                        Track.m_PlayCount = PodcastItem.m_PlayCount;
                        Track.m_Year = 0;
                        Track.m_Rating = wxNOT_FOUND;
                    }
                    else
                    {
                        //guLogMessage( wxT( "Reading tags from the file..." ) );
                        guTagInfo * TagInfo = guGetTagInfoHandler( FileName );
                        if( TagInfo )
                        {
                            Track.m_Type = guTRACK_TYPE_NOTDB;

                            TagInfo->Read();

                            Track.m_ArtistName  = TagInfo->m_ArtistName;
                            Track.m_AlbumName   = TagInfo->m_AlbumName;
                            Track.m_SongName    = TagInfo->m_TrackName;
                            Track.m_Number      = TagInfo->m_Track;
                            Track.m_GenreName   = TagInfo->m_GenreName;
                            Track.m_Length      = TagInfo->m_Length;
                            Track.m_Year        = TagInfo->m_Year;
                            Track.m_Rating      = wxNOT_FOUND;
                            Track.m_CoverId     = 0;

                            delete TagInfo;
                        }
                        else
                        {
                            guLogError( wxT( "Could not read tags from file '%s'" ), FileName.c_str() );
                        }
                    }
                }

                m_TotalLen += Track.m_Length;

                AddItem( Track, pos );
            }
            else
            {
                guLogError( wxT( "Could not open the file '%s'" ), FileName.c_str() );
            }
        }
        else if( wxDirExists( FileName ) )
        {
            wxString DirName = FileName;
            wxDir Dir;
            if( !DirName.EndsWith( wxT( "/" ) ) )
                DirName += wxT( "/" );

            int InsertPos = pos;
            Dir.Open( FileName );
            if( Dir.IsOpened() )
            {
                if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
                {
                    do {
                        if( ( FileName[ 0 ] != '.' ) )
                        {
                            AddPlayListItem( DirName + FileName, addpath, InsertPos++ );
                        }
                    } while( Dir.GetNext( &FileName ) );
                }
            }
        }
        else
        {
            guLogError( wxT( "File doesnt exist '%s'" ), FileName.c_str() );
        }
    }
    else    // This should be a radiostation
    {
        Track.m_Type     = guTRACK_TYPE_RADIOSTATION;
        Track.m_CoverId  = 0;
        Track.m_FileName = FileName;
        Track.m_SongName = FileName;
        //Track.m_AlbumName = FileName;
        Track.m_Length   = 0;
        Track.m_Year     = 0;
        Track.m_Bitrate  = 0;
        Track.m_Rating   = wxNOT_FOUND;
        AddItem( Track, pos );
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
void guPlayList::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int TrackCount = m_Items.Count();
    if( !TrackCount )
    {
        MenuItem = new wxMenuItem( Menu, wxNOT_FOUND, _( "The playlist is empty" ), _( "The playlist is empty" ) );
        Menu->Append( MenuItem );
        return;
    }

    wxArrayInt SelectedItems = GetSelectedItems( false );
    int SelCount = SelectedItems.Count();

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_EDITTRACKS, _( "Edit Songs" ), _( "Edit the current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels of the current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SEARCH, _( "Search" ), _( "Search a track in the playlist by name" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_CLEAR, _( "Clear PlayList" ), _( "Remove all songs from PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_clear ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_REMOVE, _( "Remove selected songs" ), _( "Remove selected songs from PlayList" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu->Append( MenuItem );
    }

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SAVE, _( "Save to PlayList" ), _( "Save the selected tracks to PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY, _( "Randomize PlayList" ), _( "Randomize the songs in the PlayList" ) );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_random ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_COPYTO, _( "Copy to..." ), _( "Copy the current playlist to a directory or device" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    if( SelCount == 1 )
    {
        wxMenu *     SubMenu;
        SubMenu = new wxMenu();

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_TITLE, _( "Track" ), _( "Selects the current selected track in the library" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_ARTIST, _( "Artist" ), _( "Selects the artist of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_ALBUM, _( "Album" ), _( "Select the album of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_YEAR, _( "Year" ), _( "Select the year of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_GENRE, _( "Genre" ), _( "Select the genre of the current song" ) );
        SubMenu->Append( MenuItem );

        Menu->AppendSubMenu( SubMenu, _( "Select" ), _( "Search in the library" ) );

        Menu->AppendSeparator();
    }

    if( SelCount == 1 && ( m_Items[ SelectedItems[ 0 ] ].m_Type < guTRACK_TYPE_RADIOSTATION ) )
    {
        AddOnlineLinksMenu( Menu );
    }
    AddPlayListCommands( Menu, SelCount );

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
    ReloadItems();
    //PlayerPanel->UpdateTotalLength();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnAppendToPlaylistClicked( wxCommandEvent &event )
{
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSaveClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt SelectedItems = GetSelectedItems( false );
    wxArrayInt NewSongs;

    if( ( count = SelectedItems.Count() ) )
    {
        for( index = 0; index < count; index++ )
        {
            if( m_Items[ SelectedItems[ index ] ].m_SongId > 0 )
                NewSongs.Add( m_Items[ SelectedItems[ index ] ].m_SongId );
        }
    }
    else
    {
        count = m_Items.Count();
        for( index = 0; index < count; index++ )
        {
            if( m_Items[ index ].m_SongId > 0 )
                NewSongs.Add( m_Items[ index ].m_SongId );
        }
    }

    if( NewSongs.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists,GUPLAYLIST_STATIC );
        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( wxTheApp->GetTopWindow(), m_Db, &NewSongs, &PlayLists );
        if( PlayListAppendDlg->ShowModal() == wxID_OK )
        {
            int Selected = PlayListAppendDlg->GetSelectedPlayList();
            if( Selected == -1 )
            {
                wxString PLName = PlayListAppendDlg->GetPlaylistName();
                if( PLName.IsEmpty() )
                {
                    PLName = _( "UnNamed" );
                }
                m_Db->CreateStaticPlayList( PLName, NewSongs );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, NewSongs );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                {
                    m_Db->AppendStaticPlayList( PLId, NewSongs );
                }
            }
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks;
    wxArrayInt SelectedItems = GetSelectedItems( false );
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
    wxArrayInt SelectedItems = GetSelectedItems( false );
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
        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Songs Labels Editor" ), false,
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
void guPlayList::OnEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    guImagePtrArray Images;
    wxArrayString Lyrics;

    guListItems Labels;
    wxArrayInt SongIds;
    //
    guTrack * Track;
    wxArrayInt SelectedItems = GetSelectedItems( false );
    int index;

    int count = SelectedItems.Count();
    if( count )
    {
        for( index = 0; index < count; index++ )
        {
            Track = &m_Items[ SelectedItems[ index ] ];
            if( Track->m_Type < guTRACK_TYPE_RADIOSTATION )
            {
                Songs.Add( new guTrack( * Track ) );
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
            if( Track->m_Type < guTRACK_TYPE_RADIOSTATION )
            {
                Songs.Add( new guTrack( * Track ) );
            }
        }
    }

    if( !Songs.Count() )
        return;

    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Songs, &Images, &Lyrics );

    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Songs );
            UpdateImages( Songs, Images );
            UpdateLyrics( Songs, Lyrics );

            // Update the track in database, playlist, etc
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Songs );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::GetItemSearchText( const int row )
{
    return m_Items[ row ].m_SongName +
           m_Items[ row ].m_ArtistName +
           m_Items[ row ].m_AlbumName;

}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSearchClicked( wxCommandEvent &event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Search: " ), _( "Please enter the search term" ), m_LastSearch );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        m_LastSearch = EntryDialog->GetValue();
        wxArrayInt Selection = GetSelectedItems();
        long StartItem = 0;
        if( Selection.Count() )
            StartItem = Selection[ 0 ];
        int LastItemFound = FindItem( StartItem, m_LastSearch, true, false );
        SetSelection( LastItemFound );
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectTrack( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        int SelType = m_Items[ SelItem ].m_Type;
        if( ( SelType == guTRACK_TYPE_DB ) ||
            ( SelType == guTRACK_TYPE_PODCAST ) )
        {
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_TRACK );
            evt.SetInt( m_Items[ SelItem ].m_SongId );
            evt.SetExtraLong( SelType );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectArtist( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        int SelType = m_Items[ SelItem ].m_Type;
        if( ( SelType == guTRACK_TYPE_DB ) )
        {
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ARTIST );
            evt.SetInt( m_Items[ SelItem ].m_ArtistId );
            evt.SetExtraLong( SelType );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectAlbum( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        int SelType = m_Items[ SelItem ].m_Type;
        if( ( SelType == guTRACK_TYPE_DB ) ||
            ( SelType == guTRACK_TYPE_PODCAST ) )
        {
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ALBUM );
            evt.SetInt( m_Items[ SelItem ].m_AlbumId );
            evt.SetExtraLong( SelType );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectYear( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        int SelYear = m_Items[ SelItem ].m_Year;
        if( SelYear )
        {
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_YEAR );
            evt.SetInt( SelYear );
            //evt.SetExtraLong( SelType );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectGenre( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        int SelType = m_Items[ SelItem ].m_Type;
        if( ( SelType == guTRACK_TYPE_DB ) )
        {
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_GENRE );
            evt.SetInt( m_Items[ SelItem ].m_GenreId );
            //evt.SetExtraLong( SelType );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
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
        if( m_CurItem < ( int ) m_Items.Count() )
            Caps |= MPRIS_CAPS_CAN_GO_NEXT;
        if( m_CurItem > 0 )
            Caps |= MPRIS_CAPS_CAN_GO_PREV;
        Caps |= ( MPRIS_CAPS_CAN_PAUSE | MPRIS_CAPS_CAN_PLAY | MPRIS_CAPS_CAN_SEEK | MPRIS_CAPS_CAN_PROVIDE_METADATA );
    }
    Caps |= MPRIS_CAPS_CAN_HAS_TRACKLIST;
    return Caps;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSearchLinkClicked( wxCommandEvent &event )
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
            guLogMessage( wxT( "Launching link: %s" ), SearchLink.c_str() );
            guWebExecute( SearchLink );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCommandClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt Selection = GetSelectedItems( false );
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
                //wxString Path = wxT( "\"" ) + wxPathOnly( m_Items[ Selection[ 0 ] ].m_FileName ) + wxT( "\"" );
                wxString Path = wxPathOnly( m_Items[ Selection[ 0 ] ].m_FileName );
                Path.Replace( wxT( " " ), wxT( "\\ " ) );
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
                    CurCmd.Replace( wxT( "{bc}" ), wxT( "\"" ) + CoverPath + wxT( "\"" ) );
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
            if( m_Items[ item ].m_FileName == tracks->Item( index ).m_FileName )
            {
                m_Items[ item ] = tracks->Item( index );
                found = true;
            }
        }
    }
    if( found )
    {
        RefreshAll();
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
        if( m_Items[ item ].m_FileName == track->m_FileName )
        {
            m_Items[ item ] = * track;
            found = true;
        }
    }
    if( found )
    {
        RefreshAll();
    }
}

// -------------------------------------------------------------------------------- //
wxString inline guPlayList::GetItemName( const int row ) const
{
    return m_Items[ row ].m_SongName;
}

// -------------------------------------------------------------------------------- //
int inline guPlayList::GetItemId( const int row ) const
{
    return row;
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Items[ item ].m_ArtistName.c_str(),
        m_Items[ item ].m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
