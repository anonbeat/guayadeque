// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#include "PlayList.h"

#include "Accelerators.h"
#include "Config.h"
#include "EventCommandIds.h"
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
#include <wx/busyinfo.h>

#define SAVE_PLAYLIST_TIMEOUT   60000

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
guPlayerPlayList::guPlayerPlayList( wxWindow * parent, guDbLibrary * db, wxAuiManager * manager ) :
    guAuiManagedPanel( parent, manager )
{
    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    m_PlayListCtrl = new guPlayList( this, db, NULL, ( guMainFrame * ) parent );
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
// guPlayList
// -------------------------------------------------------------------------------- //
guPlayList::guPlayList( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel, guMainFrame * mainframe ) :
            guListView( parent, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG |
                guLISTVIEW_ALLOWDROP | guLISTVIEW_DRAGSELFITEMS | guLISTVIEW_HIDE_HEADER )
{
    wxArrayString Songs;
    m_ItemHeight = 40;

    InsertColumn( new guListViewColumn( _( "Now Playing" ), 0 ) );

    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_MainFrame = mainframe;
    m_TotalLen = 0;
    m_CurItem = wxNOT_FOUND;
    m_StartPlaying = false;
    m_SavePlaylistTimer = NULL;
    m_SysFontPointSize = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT ).GetPointSize();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_MaxPlayedTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MAX_TRACKS_PLAYED, 15, CONFIG_PATH_PLAYBACK );
    m_MinPlayListTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MIN_TRACKS_PLAY, 4, CONFIG_PATH_PLAYBACK );
    m_DelTracksPLayed = Config->ReadNum( CONFIG_KEY_PLAYBACK_DEL_TRACKS_PLAYED, false, CONFIG_PATH_PLAYBACK );

    m_PlayBitmap = new wxBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    m_StopBitmap = new wxBitmap( guImage( guIMAGE_INDEX_player_tiny_red_stop ) );
    m_NormalStar   = new wxBitmap( guImage( guIMAGE_INDEX_star_normal_tiny ) );
    m_SelectStar = new wxBitmap( guImage( guIMAGE_INDEX_star_highlight_tiny ) );

    Bind( wxEVT_MENU, &guPlayList::OnClearClicked, this, ID_PLAYER_PLAYLIST_CLEAR );
    Bind( wxEVT_MENU, &guPlayList::OnRemoveClicked, this, ID_PLAYER_PLAYLIST_REMOVE );
    Bind( wxEVT_MENU, &guPlayList::OnSaveClicked, this, ID_PLAYER_PLAYLIST_SAVE );
    Bind( wxEVT_MENU, &guPlayList::OnEditLabelsClicked, this, ID_PLAYER_PLAYLIST_EDITLABELS );
    Bind( wxEVT_MENU, &guPlayList::OnEditTracksClicked, this, ID_PLAYER_PLAYLIST_EDITTRACKS );
    Bind( wxEVT_MENU, &guPlayList::OnSearchClicked, this, ID_PLAYER_PLAYLIST_SEARCH );
    Bind( wxEVT_MENU, &guPlayList::OnCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Bind( wxEVT_MENU, &guPlayList::OnStopAtEnd, this, ID_PLAYER_PLAYLIST_STOP_ATEND );
    Bind( wxEVT_MENU, &guPlayList::SetNextTracks, this, ID_PLAYER_PLAYLIST_SET_NEXT_TRACK );

    Bind( wxEVT_MENU, &guPlayList::OnSelectTrack, this, ID_PLAYER_PLAYLIST_SELECT_TITLE );
    Bind( wxEVT_MENU, &guPlayList::OnSelectArtist, this, ID_PLAYER_PLAYLIST_SELECT_ARTIST );
    Bind( wxEVT_MENU, &guPlayList::OnSelectAlbum, this, ID_PLAYER_PLAYLIST_SELECT_ALBUM );
    Bind( wxEVT_MENU, &guPlayList::OnSelectAlbumArtist, this, ID_PLAYER_PLAYLIST_SELECT_ALBUMARTIST );
    Bind( wxEVT_MENU, &guPlayList::OnSelectComposer, this, ID_PLAYER_PLAYLIST_SELECT_COMPOSER );
    Bind( wxEVT_MENU, &guPlayList::OnSelectYear, this, ID_PLAYER_PLAYLIST_SELECT_YEAR );
    Bind( wxEVT_MENU, &guPlayList::OnSelectGenre, this, ID_PLAYER_PLAYLIST_SELECT_GENRE );

    Bind( wxEVT_MENU, &guPlayList::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Bind( wxEVT_MENU, &guPlayList::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    Bind( guConfigUpdatedEvent, &guPlayList::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_MENU, &guPlayList::OnDeleteFromLibrary, this, ID_PLAYER_PLAYLIST_DELETE_LIBRARY );
    Bind( wxEVT_MENU, &guPlayList::OnDeleteFromDrive, this, ID_PLAYER_PLAYLIST_DELETE_DRIVE );

    Bind( wxEVT_MENU, &guPlayList::OnSetRating, this, ID_PLAYERPANEL_SETRATING_0, ID_PLAYERPANEL_SETRATING_5 );

    Bind( wxEVT_MENU, &guPlayList::OnCreateSmartPlaylist, this, ID_PLAYLIST_SMART_PLAYLIST );

    Bind( wxEVT_MENU, &guPlayList::StartSavePlaylistTimer, this, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    Bind( wxEVT_TIMER, &guPlayList::OnSavePlaylistTimer, this );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPlayList::~guPlayList()
{
    // Save the guPlayList so it can be reload next time
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    SavePlaylistTracks();

    if( m_SavePlaylistTimer )
        delete m_SavePlaylistTimer;

    if( m_PlayBitmap )
      delete m_PlayBitmap;

    if( m_StopBitmap )
      delete m_StopBitmap;

    if( m_NormalStar )
      delete m_NormalStar;

    if( m_SelectStar )
      delete m_SelectStar;

    Unbind( wxEVT_MENU, &guPlayList::OnClearClicked, this, ID_PLAYER_PLAYLIST_CLEAR );
    Unbind( wxEVT_MENU, &guPlayList::OnRemoveClicked, this, ID_PLAYER_PLAYLIST_REMOVE );
    Unbind( wxEVT_MENU, &guPlayList::OnSaveClicked, this, ID_PLAYER_PLAYLIST_SAVE );
    Unbind( wxEVT_MENU, &guPlayList::OnEditLabelsClicked, this, ID_PLAYER_PLAYLIST_EDITLABELS );
    Unbind( wxEVT_MENU, &guPlayList::OnEditTracksClicked, this, ID_PLAYER_PLAYLIST_EDITTRACKS );
    Unbind( wxEVT_MENU, &guPlayList::OnSearchClicked, this, ID_PLAYER_PLAYLIST_SEARCH );
    Unbind( wxEVT_MENU, &guPlayList::OnCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Unbind( wxEVT_MENU, &guPlayList::OnStopAtEnd, this, ID_PLAYER_PLAYLIST_STOP_ATEND );
    Unbind( wxEVT_MENU, &guPlayList::SetNextTracks, this, ID_PLAYER_PLAYLIST_SET_NEXT_TRACK );

    Unbind( wxEVT_MENU, &guPlayList::OnSelectTrack, this, ID_PLAYER_PLAYLIST_SELECT_TITLE );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectArtist, this, ID_PLAYER_PLAYLIST_SELECT_ARTIST );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectAlbum, this, ID_PLAYER_PLAYLIST_SELECT_ALBUM );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectAlbumArtist, this, ID_PLAYER_PLAYLIST_SELECT_ALBUMARTIST );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectComposer, this, ID_PLAYER_PLAYLIST_SELECT_COMPOSER );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectYear, this, ID_PLAYER_PLAYLIST_SELECT_YEAR );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectGenre, this, ID_PLAYER_PLAYLIST_SELECT_GENRE );

    Unbind( wxEVT_MENU, &guPlayList::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guPlayList::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    Unbind( guConfigUpdatedEvent, &guPlayList::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Unbind( wxEVT_MENU, &guPlayList::OnDeleteFromLibrary, this, ID_PLAYER_PLAYLIST_DELETE_LIBRARY );
    Unbind( wxEVT_MENU, &guPlayList::OnDeleteFromDrive, this, ID_PLAYER_PLAYLIST_DELETE_DRIVE );

    Unbind( wxEVT_MENU, &guPlayList::OnSetRating, this, ID_PLAYERPANEL_SETRATING_0, ID_PLAYERPANEL_SETRATING_5 );

    Unbind( wxEVT_MENU, &guPlayList::OnCreateSmartPlaylist, this, ID_PLAYLIST_SMART_PLAYLIST );

    Unbind( wxEVT_MENU, &guPlayList::StartSavePlaylistTimer, this, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    Unbind( wxEVT_TIMER, &guPlayList::OnSavePlaylistTimer, this );
}

// -------------------------------------------------------------------------------- //
void guPlayList::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AccelCmds;
    AccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );
    AccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_0 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_1 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_2 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_3 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_4 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_5 );

    if( guAccelDoAcceleratorTable( AccelCmds, AccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_PLAYBACK )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            m_MaxPlayedTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MAX_TRACKS_PLAYED, 15, CONFIG_PATH_PLAYBACK );
            m_MinPlayListTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MIN_TRACKS_PLAY, 4, CONFIG_PATH_PLAYBACK );
            m_DelTracksPLayed = Config->ReadNum( CONFIG_KEY_PLAYBACK_DEL_TRACKS_PLAYED, false, CONFIG_PATH_PLAYBACK );
        }
    }

    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropBegin( void )
{
    if( GetItemCount() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config->ReadBool( CONFIG_KEY_GENERAL_DROP_FILES_CLEAR_PLAYLIST, false, CONFIG_PATH_GENERAL ) )
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
bool inline guIsJamendoFile( const wxString &filename )
{
    return filename.Find( wxT( "/api.jamendo.com/get2/" ) ) != wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
bool inline guIsMagnatuneFile( const wxString &filename )
{
    return filename.Find( wxT( ".magnatune.com/all/" ) ) != wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropFile( const wxString &filename )
{
    guLogMessage( wxT( "Dropping '%s'" ), filename.c_str() );
    if( guIsJamendoFile( filename ) || guIsMagnatuneFile( filename ) )
    {
        AddPlayListItem( wxT( "http:/" ) + filename, guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND );
    }
    else if( guIsValidAudioFile( filename ) ||
             guPlaylistFile::IsValidPlayList( filename ) ||
             guCuePlaylistFile::IsValidFile( filename ) )
    {
        AddPlayListItem( filename, guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropTracks( const guTrackArray * tracks )
{
    guLogMessage( wxT( "Dropping tracks" ) );
    AddToPlayList( * tracks );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropEnd( void )
{
    m_DragOverItem = wxNOT_FOUND;
    // Once finished send the update guPlayList event to the guPlayList object
    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config->ReadBool( CONFIG_KEY_GENERAL_DROP_FILES_CLEAR_PLAYLIST, false, CONFIG_PATH_GENERAL ) )
    {
        event.SetExtraLong( 1 );
    }
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
int guPlayList::GetSelectedSongs( guTrackArray * Songs, const bool isdrag ) const
{
    int Index;
    int Count;
    wxArrayInt Selection = GetSelectedItems( false );
    Count = Selection.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        Songs->Add( new guTrack( m_Items[ Selection[ Index ] ] ) );
    }
    return Count;
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
    //guLogMessage( wxT( "MoveSelection %i" ), m_DragOverItem );
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

    if( !Selection.Count() )
    {
        return;
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
    m_DragOverItem = wxNOT_FOUND;
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
        wxCommandEvent CmdEvent( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
        CmdEvent.SetInt( 0 );
        CmdEvent.SetExtraLong( 0 );
        wxPostEvent( this, CmdEvent );
        return;
    }
    event.Skip();
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
    m_Attr.m_Font->SetPointSize( m_SysFontPointSize );
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
    if( Item.m_Type != guTRACK_TYPE_RADIOSTATION )
    {
        CutRect.width -= ( 50 + 6 + 2 );

        dc.SetClippingRegion( CutRect );

        //dc.DrawText( ( Item.m_Number ? wxString::Format( wxT( "%02u - " ), Item.m_Number ) :
        //                  wxT( "" ) ) + Item.m_SongName, CutRect.x + 4, CutRect.y + 4 );
        dc.DrawText( Item.m_SongName, CutRect.x + 4, CutRect.y + 2 );

        //m_Attr.m_Font->SetPointSize( m_SysFontPointSize - 3 );
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

        //m_Attr.m_Font->SetPointSize( m_SysFontPointSize - 2 );
        //m_Attr.m_Font->SetStyle( wxFONTSTYLE_NORMAL );
        //dc.SetFont( * m_Attr.m_Font );

        int TimeWidth = 56;

        if( Item.m_Type & guTRACK_TYPE_STOP_HERE )
        {
            dc.DrawBitmap( * m_StopBitmap, CutRect.x + 40, CutRect.y + 2, true );
            TimeWidth -= 16;
        }

        TimeStr = LenToString( Item.m_Length );
        TextSize = dc.GetTextExtent( TimeStr );
        TimeWidth -= TextSize.GetWidth();
        if( TimeWidth < 0 )
            TimeWidth = 0;
        dc.DrawText( TimeStr, CutRect.x + ( TimeWidth / 2 ), CutRect.y + 4 );
        //guLogMessage( wxT( "%i - %i" ), TextSize.GetWidth(), TextSize.GetHeight() );


//        if( Item.m_Type != guTRACK_TYPE_RADIOSTATION )
//        {
            // Draw the rating
            int index;
            //OffsetSecLine += 2;
            CutRect.x += 1;
            CutRect.y += 2;
            for( index = 0; index < 5; index++ )
            {
               dc.DrawBitmap( ( index >= Item.m_Rating ) ? * m_NormalStar : * m_SelectStar,
                              CutRect.x + ( 11 * index ), CutRect.y + m_SecondLineOffset + 2, true );
            }
//        }
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
            //if( Item != wxNOT_FOUND && m_Items[ Item ].m_Type == guTRACK_TYPE_DB )
            if( Item != wxNOT_FOUND &&
               ( m_Items[ Item ].m_Type != guTRACK_TYPE_PODCAST ) &&
               ( m_Items[ Item ].m_Type != guTRACK_TYPE_RADIOSTATION ) )
            {
                if( ( size_t ) y > ( ( Item - GetVisibleRowsBegin() ) * m_ItemHeight ) + m_SecondLineOffset + 2 )
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
                        RefreshRow( Item );

                        SetTrackRating( m_Items[ Item ], Rating );
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
    Font.SetPointSize( m_SysFontPointSize - 2 );
    dc.SetFont( Font );

    wxCoord y;
    dc.GetTextExtent( wxT( "Hg" ), NULL, &y );
    Height += y;
    self->m_SecondLineOffset = Height;

//    Font.SetPointSize( m_SysFontPointSize - 3 );
//    dc.SetFont( Font );
//    dc.GetTextExtent( wxT( "Hg" ), NULL, &y );
    Height += y + 6;

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

    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, event );
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
guTrack * guPlayList::GetNext( const int playmode, const bool forceskip )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( !forceskip && ( playmode == guPLAYER_PLAYMODE_REPEAT_TRACK ) )
        {
            return &m_Items[ m_CurItem ];
        }
        else if( ( m_CurItem < ( ( int ) m_Items.Count() - 1 ) ) )
        {
            if( m_DelTracksPLayed && ( playmode <= guPLAYER_PLAYMODE_SMART ) )
            {
                m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                m_Items.RemoveAt( m_CurItem );
                ReloadItems();
            }
            else
                m_CurItem++;

            return &m_Items[ m_CurItem ];

        }
        else if( playmode == guPLAYER_PLAYMODE_REPEAT_PLAYLIST )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetPrev( const int playmode, const bool forceskip )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( !forceskip && playmode == guPLAYER_PLAYMODE_REPEAT_TRACK )
        {
            return &m_Items[ m_CurItem ];
        }
        else if( m_CurItem > 0 )
        {
            if( m_DelTracksPLayed && !playmode )
            {
                m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                m_Items.RemoveAt( m_CurItem );
                ReloadItems();
            }
            m_CurItem--;
            return &m_Items[ m_CurItem ];
        }
        else if( playmode == guPLAYER_PLAYMODE_REPEAT_PLAYLIST )
        {
            m_CurItem = m_Items.Count() - 1;
            return &m_Items[ m_CurItem ];
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetNextAlbum( const int playmode, const bool forceskip )
{
    int SaveCurItem = m_CurItem;
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
//        else if( !forceskip && playloop == guPLAYER_PLAYLOOP_TRACK )
//        {
//            return &m_Items[ m_CurItem ];
//        }
        else if( ( ( size_t ) m_CurItem < ( m_Items.Count() - 1 ) ) )
        {
            int CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;

            while( ( size_t ) m_CurItem < ( m_Items.Count() - 1 ) )
            {
                if( m_DelTracksPLayed && !playmode )
                {
                    m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                    m_Items.RemoveAt( m_CurItem );
                    ReloadItems();
                }
                else
                    m_CurItem++;
                if( m_Items[ m_CurItem ].m_AlbumId != CurAlbumId )
                    break;
            }

            if( ( size_t ) m_CurItem < ( m_Items.Count() - 1 ) )
                return &m_Items[ m_CurItem ];

        }
//        else if( playloop == guPLAYER_PLAYLOOP_PLAYLIST )
//        {
//            m_CurItem = 0;
//            return &m_Items[ m_CurItem ];
//        }
    }
    m_CurItem = SaveCurItem;
    return NULL;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetPrevAlbum( const int playmode, const bool forceskip )
{
    int SaveCurItem = m_CurItem;
    if( m_Items.Count() )
    {
        //guLogMessage( wxT( "GetPrevAlbum... %i" ), m_CurItem );
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
//        else if( !forceskip && playloop == guPLAYER_PLAYLOOP_TRACK )
//        {
//            return &m_Items[ m_CurItem ];
//        }
        else if( m_CurItem > 0 )
        {
            int CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;

            //guLogMessage( wxT( "CurrentAlbum: %i" ), CurAlbumId );
            while( m_CurItem > 0 )
            {
                if( m_DelTracksPLayed && !playmode )
                {
                    m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                    m_Items.RemoveAt( m_CurItem );
                    ReloadItems();
                }
                m_CurItem--;

                //guLogMessage( wxT( "Album %i:  %i" ), m_CurItem, m_Items[ m_CurItem ].m_AlbumId );
                if( m_Items[ m_CurItem ].m_AlbumId != CurAlbumId )
                {
                    CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;
                    while( m_CurItem > 0 && m_Items[ m_CurItem ].m_AlbumId == CurAlbumId )
                    {
                        //guLogMessage( wxT( "New Album %i:  %i" ), m_CurItem, m_Items[ m_CurItem ].m_AlbumId );
                        m_CurItem--;
                    }
                    if( m_Items[ m_CurItem ].m_AlbumId != CurAlbumId )
                        m_CurItem++;
                    break;
                }
            }

            if( m_CurItem >= 0 )
                return &m_Items[ m_CurItem ];
        }
//        else if( playloop == guPLAYER_PLAYLOOP_PLAYLIST )
//        {
//            m_CurItem = m_Items.Count() - 1;
//            return &m_Items[ m_CurItem ];
//        }
    }
    m_CurItem = SaveCurItem;
    return NULL;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetItem( size_t item )
{
    size_t ItemsCount = m_Items.Count();
    if( ItemsCount && item < ItemsCount )
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
    m_PendingLoadIds.Empty();
    ClearSelectedItems();
    ReloadItems();
    //PlayerPanel->UpdateTotalLength();
    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
    event.SetInt( 0 );
    event.SetExtraLong( 0 );
    wxPostEvent( this, event );

    event.SetId( ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
void guPlayList::Randomize( const bool isplaying )
{
    int index;
    int pos;
    int newpos;
    int count = m_Items.Count();
    guTrack SavedItem;

    if( count > 2 )
    {
        if( isplaying && m_CurItem > 0 )
        {
            SavedItem = m_Items[ 0 ];
            m_Items[ 0 ] = m_Items[ m_CurItem ];
            m_Items[ m_CurItem ] = SavedItem;
            m_CurItem = 0;
        }
        else if( !isplaying )
        {
            if( m_CurItem != wxNOT_FOUND )
                m_CurItem = wxNOT_FOUND;
        }
        for( index = 0; index < count; index++ )
        {
            do {
                pos = guRandom( count );
                newpos = guRandom( count );
            } while( ( pos == newpos ) || ( isplaying && ( m_CurItem == 0 ) && ( !pos || !newpos ) ) );
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

        wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
        wxPostEvent( this, event );
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

    // Get the SearchCoverWords array
    m_MainFrame->GetCollectionsCoverNames( CoverSearchWords );

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
                    if( guIsValidImageFile( CurFile ) )
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
void guPlayList::AddToPlayList( const guTrackArray &items, const bool deleteold, const int aftercurrent )
{
    wxMutexLocker Lock( m_ItemsMutex );
    int Index;
    int Count;

    int InsertPosition = 0;

    switch( aftercurrent )
    {
        case guINSERT_AFTER_CURRENT_NONE :
            InsertPosition = m_Items.Count();
            break;

        case guINSERT_AFTER_CURRENT_TRACK :
        {
            if( m_CurItem != wxNOT_FOUND && m_CurItem < ( int ) m_Items.Count() )
            {
                wxString CurFileName = m_Items[ m_CurItem ].m_FileName;
                InsertPosition = m_CurItem + 1;
                while( ( InsertPosition < ( int ) m_Items.Count() ) &&
                       m_Items[ InsertPosition ].m_FileName == CurFileName )
                {
                    InsertPosition++;
                }
            }
            break;
        }

        case guINSERT_AFTER_CURRENT_ALBUM :
        {
            if( m_CurItem != wxNOT_FOUND && m_CurItem < ( int ) m_Items.Count() )
            {
                int CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;
                InsertPosition = m_CurItem + 1;
                if( CurAlbumId )
                {
                    while( ( InsertPosition < ( int ) m_Items.Count() ) &&
                           m_Items[ InsertPosition ].m_AlbumId == CurAlbumId )
                    {
                        InsertPosition++;
                    }
                }
            }
            break;
        }

        case guINSERT_AFTER_CURRENT_ARTIST :
        {
            if( m_CurItem != wxNOT_FOUND && m_CurItem < ( int ) m_Items.Count() )
            {
                int CurArtistId = m_Items[ m_CurItem ].m_ArtistId;
                InsertPosition = m_CurItem + 1;
                if( CurArtistId )
                {
                    while( ( InsertPosition < ( int ) m_Items.Count() ) &&
                           m_Items[ InsertPosition ].m_ArtistId == CurArtistId )
                    {
                        InsertPosition++;
                    }
                }
            }
            break;
        }
    }

    Count = items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        AddItem( items[ Index ], InsertPosition + Index );
        m_TotalLen += items[ Index ].m_Length;
    }

    while( deleteold && ( m_CurItem != 0 ) && ( ( m_CurItem ) > m_MaxPlayedTracks ) )
    {
        m_TotalLen -= m_Items[ 0 ].m_Length;
        m_Items.RemoveAt( 0 );
        m_CurItem--;
    }
    ReloadItems();

    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddPlayListItem( const wxString &filename, const int aftercurrent, const int pos )
{
    // Check if its a uri or a filename
    int Index;
    int Count;
    wxString FileName;
    guTrack Track;
    guPodcastItem PodcastItem;

    wxURI Uri( filename );

    int InsertPosition = 0;

    switch( aftercurrent )
    {
        case guINSERT_AFTER_CURRENT_NONE :
            InsertPosition = m_Items.Count();
            break;

        case guINSERT_AFTER_CURRENT_TRACK :
        {
            if( m_CurItem != wxNOT_FOUND && m_CurItem < ( int ) m_Items.Count() )
            {
                wxString CurFileName = m_Items[ m_CurItem ].m_FileName;
                InsertPosition = m_CurItem + 1;
                while( ( InsertPosition < ( int ) m_Items.Count() ) &&
                       m_Items[ InsertPosition ].m_FileName == CurFileName )
                {
                    InsertPosition++;
                }
            }
            break;
        }

        case guINSERT_AFTER_CURRENT_ALBUM :
        {
            if( m_CurItem != wxNOT_FOUND && m_CurItem < ( int ) m_Items.Count() )
            {
                int CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;
                InsertPosition = m_CurItem + 1;
                if( CurAlbumId )
                {
                    while( ( InsertPosition < ( int ) m_Items.Count() ) &&
                           m_Items[ InsertPosition ].m_AlbumId == CurAlbumId )
                    {
                        InsertPosition++;
                    }
                }
            }
            break;
        }

        case guINSERT_AFTER_CURRENT_ARTIST :
        {
            if( m_CurItem != wxNOT_FOUND && m_CurItem < ( int ) m_Items.Count() )
            {
                int CurArtistId = m_Items[ m_CurItem ].m_ArtistId;
                InsertPosition = m_CurItem + 1;
                if( CurArtistId )
                {
                    while( ( InsertPosition < ( int ) m_Items.Count() ) &&
                           m_Items[ InsertPosition ].m_ArtistId == CurArtistId )
                    {
                        InsertPosition++;
                    }
                }
            }
            break;
        }
    }

    //guLogMessage( wxT( "Loading %i %i => %i '%s'" ), aftercurrent, pos, InsertPosition, filename.c_str() );

    if( guCuePlaylistFile::IsValidFile( Uri.GetPath() ) )   // If its a cue playlist
    {
        int InsertPos = wxMax( pos, 0 );
        guCuePlaylistFile CuePlaylistFile( filename );
        if( ( Count = CuePlaylistFile.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                guCuePlaylistItem &CueItem = CuePlaylistFile.GetItem( Index );
                Track.m_SongId = wxNOT_FOUND;
                Track.m_SongName = CueItem.m_Name;
                Track.m_ArtistName = CueItem.m_ArtistName;
                Track.m_Composer = CueItem.m_Composer;
                Track.m_Comments = CueItem.m_Comment;
                Track.m_AlbumId = 0;
                Track.m_AlbumName = CueItem.m_AlbumName;
                Track.m_Offset = CueItem.m_Start;
                Track.m_Length = CueItem.m_Length;
                Track.m_FileName = CueItem.m_TrackPath;
                Track.m_Number = Index;
                long Year;
                if( CueItem.m_Year.ToLong( &Year ) )
                    Track.m_Year = Year;
                Track.m_Rating = wxNOT_FOUND;

                AddItem( Track, InsertPosition + InsertPos );
                InsertPos++;
            }
        }
    }
    else if( guPlaylistFile::IsValidPlayList( Uri.GetPath() ) )   // If its a playlist
    {
        int InsertPos = wxMax( pos, 0 );
        guPlaylistFile PlayList( filename );
        if( ( Count = PlayList.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                AddPlayListItem( PlayList.GetItem( Index ).m_Location, aftercurrent, InsertPos++ );
            }
        }
    }
    else if( Uri.IsReference() )    // Its a file
    {
        if( filename.StartsWith( wxT( "file://" ) ) )
        {
            FileName = wxURI::Unescape( Uri.GetPath() );
        }
        else
        {
            FileName = filename;
        }
        guLogMessage( wxT( "Loading '%s'" ), FileName.c_str() );
        if( wxFileExists( FileName ) )
        {
            if( guIsValidAudioFile( FileName ) )
            {
                Track.m_FileName = FileName;

                if( !m_Db->FindTrackFile( FileName, &Track ) )
                {
                    guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                    if( DbPodcasts->GetPodcastItemFile( FileName, &PodcastItem ) )
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
                        if( Track.ReadFromFile( FileName ) )
                        {
                            Track.m_Type = guTRACK_TYPE_NOTDB;
                        }
                        else
                        {
                            guLogError( wxT( "Could not read tags from file '%s'" ), FileName.c_str() );
                        }
                    }
                }

                m_TotalLen += Track.m_Length;

                AddItem( Track, InsertPosition + wxMax( 0, pos ) );
            }
            else
            {
                guLogError( wxT( "Could not open the file '%s'" ), filename.c_str() );
            }
        }
        else if( wxDirExists( FileName ) )
        {
            wxString DirName = FileName;
            wxDir Dir;
            if( !DirName.EndsWith( wxT( "/" ) ) )
                DirName += wxT( "/" );

            int InsertPos = pos;
            Dir.Open( DirName );
            if( Dir.IsOpened() )
            {
                if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
                {
                    do {
                        if( ( FileName[ 0 ] != '.' ) )
                        {
                            AddPlayListItem( DirName + FileName, aftercurrent, InsertPos++ );
                        }
                    } while( Dir.GetNext( &FileName ) );
                }
            }
        }
        else
        {
            guLogError( wxT( "File doesnt exist '%s'" ), filename.c_str() );
        }
    }
    else if( guIsJamendoFile( filename ) )
    {
        //http://api.jamendo.com/get2/stream/track/redirect/?id=594731&streamencoding=ogg2
        Track.m_CoverId  = 0;
        Track.m_SongName = filename;
        //Track.m_AlbumName = FileName;
        Track.m_Length   = 0;
        Track.m_Year     = 0;
        Track.m_Bitrate  = 0;
        Track.m_Rating   = wxNOT_FOUND;

        long Id;
        FileName = filename;
        wxString IdStr = FileName.Mid( FileName.Find( wxT( "/?id=" ) ) + 5 );
        IdStr = IdStr.Mid( 0, IdStr.Find( wxT( "&" ) ) );
        IdStr.ToLong( &Id );
        if( Id )
        {
            guMediaViewer * JamendoMediaViewer = m_MainFrame->FindCollectionMediaViewer( wxT( "Jamendo" ) );
            if( JamendoMediaViewer )
            {
                guJamendoLibrary * JamendoDb = ( guJamendoLibrary *  ) JamendoMediaViewer->GetDb();
                if( JamendoDb )
                {
                    JamendoDb->FindTrackId( Id, &Track );
                }
            }
            else if( m_PendingLoadIds.Index( wxT( "Jamendo" ) ) == wxNOT_FOUND )
            {
                m_PendingLoadIds.Add( wxT( "Jamendo" ) );
            }
        }

        Track.m_Type     = guTRACK_TYPE_JAMENDO;
        Track.m_FileName = FileName;
        AddItem( Track, InsertPosition + wxMax( 0, pos ) );
    }
    else if( guIsMagnatuneFile( filename ) )
    {
        FileName = filename;
        FileName.Replace( wxT( " " ), wxT( "%20" ) );
        wxString SearchStr = FileName;
        int FoundPos;
        if( ( FoundPos = SearchStr.Find( wxT( "@stream.magnatune" ) ) ) != wxNOT_FOUND )
        {
            SearchStr = SearchStr.Mid( FoundPos );
            SearchStr.Replace( wxT( "@stream." ), wxT( "http://he3." ) );
            SearchStr.Replace( wxT( "_nospeech" ), wxEmptyString );
        }
        else if( ( FoundPos = SearchStr.Find( wxT( "@download.magnatune" ) ) ) != wxNOT_FOUND )
        {
            SearchStr = SearchStr.Mid( FoundPos );
            SearchStr.Replace( wxT( "@download." ), wxT( "http://he3." ) );
            SearchStr.Replace( wxT( "_nospeech" ), wxEmptyString );
        }

        guLogMessage( wxT( "Searching for track '%s'" ), SearchStr.c_str() );

        guMediaViewer * MagnatuneMediaViewer = m_MainFrame->FindCollectionMediaViewer( wxT( "Magnatune" ) );
        guMagnatuneLibrary * MagnatuneDb = NULL;
        if( MagnatuneMediaViewer )
        {
            MagnatuneDb = ( guMagnatuneLibrary * ) MagnatuneMediaViewer->GetDb();
        }
        else if( m_PendingLoadIds.Index( wxT( "Magnatune" ) ) == wxNOT_FOUND )
        {
            m_PendingLoadIds.Add( wxT( "Magnatune" ) );
        }

        if( !MagnatuneDb || ( MagnatuneDb->GetTrackId( SearchStr, &Track ) == wxNOT_FOUND ) )
        {
            Track.m_CoverId  = 0;
            Track.m_SongName = FileName;
            //Track.m_AlbumName = FileName;
            Track.m_Length   = 0;
            Track.m_Year     = 0;
            Track.m_Bitrate  = 0;
            Track.m_Rating   = wxNOT_FOUND;
        }
        Track.m_Type     = guTRACK_TYPE_MAGNATUNE;
        Track.m_FileName = FileName;
        AddItem( Track, InsertPosition + wxMax( 0, pos ) );
    }
    else    // This should be a radiostation
    {
        Track.m_Type     = guTRACK_TYPE_RADIOSTATION;
        Track.m_CoverId  = 0;
        Track.m_FileName = filename;
        Track.m_SongName = filename;
        //Track.m_AlbumName = FileName;
        Track.m_Length   = 0;
        Track.m_Year     = 0;
        Track.m_Bitrate  = 0;
        Track.m_Rating   = wxNOT_FOUND;
        AddItem( Track, InsertPosition + wxMax( 0, pos ) );
    }

    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, event );
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
        wxArrayString Commands = Config->ReadAStr( CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, CONFIG_PATH_COMMANDS_EXECS );
        wxArrayString Names = Config->ReadAStr( CONFIG_KEY_COMMANDS_NAME, wxEmptyString, CONFIG_PATH_COMMANDS_NAMES );
        if( ( count = Commands.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                if( ( ( Commands[ index ].Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND ) ||
                      ( Commands[ index ].Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND ) )
                    && ( SelCount != 1 ) )
                {
                    continue;
                }
                MenuItem = new wxMenuItem( Menu, ID_COMMANDS_BASE + index, Names[ index ], Commands[ index ] );
                SubMenu->Append( MenuItem );
            }

            SubMenu->AppendSeparator();
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, ID_MENU_PREFERENCES_COMMANDS, _( "Preferences" ), _( "Add commands in preferences" ) );
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
        MenuItem = new wxMenuItem( Menu, wxNOT_FOUND, _( "Empty Playlist" ), _( "The playlist is empty" ) );
        Menu->Append( MenuItem );
        return;
    }

    wxArrayInt SelectedItems = GetSelectedItems( false );
    int SelCount = SelectedItems.Count();

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_EDITLABELS,
                            wxString( _( "Edit Labels" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITLABELS ),
                            _( "Edit the current selected tracks labels" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_EDITTRACKS,
                            wxString( _( "Edit Songs" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                            _( "Edit the current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SET_NEXT_TRACK,
                                _( "Set as Next Track" ),
                                _( "Move the selected tracks to be played next" ) );
        Menu->Append( MenuItem );

        wxMenu * RatingMenu = new wxMenu();

        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_0, wxT( "☆☆☆☆☆" ), _( "Set the rating to 0" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_1, wxT( "★☆☆☆☆" ), _( "Set the rating to 1" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_2, wxT( "★★☆☆☆" ), _( "Set the rating to 2" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_3, wxT( "★★★☆☆" ), _( "Set the rating to 3" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_4, wxT( "★★★★☆" ), _( "Set the rating to 4" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_5, wxT( "★★★★★" ), _( "Set the rating to 5" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );

        Menu->AppendSubMenu( RatingMenu, _( "Rating" ), _( "Set the current selected tracks rating" ) );

        Menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SEARCH,
                            wxString( _( "Search" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SEARCH ),
                            _( "Search a track in the playlist by name" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

//    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_STOP_ATEND,
//                            wxString( _( "Stop at end" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_STOP_ATEND),
//                            _( "Stop after current playing or selected track" ) );
//    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_stop ) );
//    Menu->Append( MenuItem );
//
//    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SAVE,
                        wxString( _( "Save to Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                        _( "Save the selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
    Menu->Append( MenuItem );

    if( SelCount == 1 )
    {
        MenuItem = new wxMenuItem( Menu, ID_PLAYLIST_SMART_PLAYLIST, _( "Create Smart Playlist" ), _( "Create a smart playlist from this track" ) );
        Menu->Append( MenuItem );
    }

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY,
                            wxString( _( "Randomize Playlist" ) )  + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_RANDOMPLAY ),
                            _( "Randomize the songs in the playlist" ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_CLEAR,
                            wxString( _( "Clear Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_CLEAR ),
                            _( "Remove all tracks from playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_clear ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_REMOVE,
                            _( "Remove from Playlist" ),
                            _( "Remove the selected tracks from playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_DELETE_LIBRARY,
                            _( "Remove from Library" ),
                            _( "Remove the selected tracks from library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_DELETE_DRIVE,
                            _( "Delete from Drive" ),
                            _( "Remove the selected tracks from drive" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    if( SelCount == 1 )
    {
        wxMenu *     SubMenu;
        SubMenu = new wxMenu();

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_TITLE, _( "Track" ), _( "Selects the current selected track in the library" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_ARTIST, _( "Artist" ), _( "Selects the artist of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_ALBUMARTIST, _( "Album Artist" ), _( "Select the album artist of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_COMPOSER, _( "Composer" ), _( "Select the composer of the current song" ) );
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

    m_MainFrame->CreateCopyToMenu( Menu );

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
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
    CmdEvent.SetInt( 0 );
    CmdEvent.SetExtraLong( 0 );
    wxPostEvent( this, CmdEvent );

    CmdEvent.SetId( ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, CmdEvent );
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
            Tracks->Add( new guTrack( m_Items[ SelectedItems[ index ] ] ) );
        }
    }
    else
    {
        Tracks = new guTrackArray( m_Items );
    }

    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
    {
        event.SetId( ID_MAINFRAME_COPYTO );
    }

    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void inline UpdateTracks( const guTrackArray &tracks, const wxArrayInt &changedflags )
{
    wxArrayPtrVoid MediaViewerPtrs;
    GetMediaViewersList( tracks, MediaViewerPtrs );

    guTrackArray CurrentTracks;
    wxArrayInt   CurrentFlags;
    int Index;
    int Count = MediaViewerPtrs.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        CurrentTracks.Empty();
        CurrentFlags.Empty();
        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];
        GetMediaViewerTracks( tracks, changedflags, MediaViewer, CurrentTracks, CurrentFlags );
        if( CurrentTracks.Count() )
        {
            guDbLibrary * Db = MediaViewer->GetDb();
            Db->UpdateSongs( &CurrentTracks, CurrentFlags );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDeleteFromLibrary( wxCommandEvent &event )
{
    if( GetSelectedCount() )
    {
        if( wxMessageBox( wxT( "Are you sure to remove the selected tracks from your library?" ),
            wxT( "Remove tracks from library" ), wxICON_QUESTION|wxYES|wxNO|wxNO_DEFAULT ) == wxYES )
        {
            guTrackArray SelectedTracks;
            wxArrayInt PodcastsIds;

            int Index;
            int Count;
            wxArrayInt Selected = GetSelectedItems( false );
            Count = Selected.Count();
            for( Index = Count - 1; Index >= 0; Index-- )
            {
                const guTrack & Track = m_Items[ Selected[ Index ] ];

                if( Track.m_Type == guTRACK_TYPE_PODCAST )
                {
                    PodcastsIds.Add( Track.m_SongId );
                }
                else if( Track.m_MediaViewer )
                {
                    SelectedTracks.Add( new guTrack( Track ) );
                }

                if( Selected[ Index ] == m_CurItem )
                {
                    m_CurItem--;
                    event.SetId( ID_PLAYERPANEL_NEXTTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                }

                RemoveItem( Selected[ Index ] );
            }

            if( SelectedTracks.Count() )
            {
                wxArrayPtrVoid MediaViewerPtrs;
                GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

                if( ( Count = MediaViewerPtrs.Count() ) )
                {
                    for( Index = 0; Index < Count; Index++ )
                    {
                        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];
                        guTrackArray MediaViewerTracks;
                        GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );

                        guDbLibrary * Db = MediaViewer->GetDb();

                        Db->DeleteLibraryTracks( &MediaViewerTracks, true );
                    }
                }
            }

            if( ( Count = PodcastsIds.Count() ) )
            {
                guPodcastItemArray Podcasts;
                guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                DbPodcasts->GetPodcastItems( &Podcasts, PodcastsIds, 0, 0 );

                m_MainFrame->RemovePodcastDownloadItems( &Podcasts );

                for( Index = 0; Index < Count; Index++ )
                {
                    DbPodcasts->SetPodcastItemStatus( PodcastsIds[ Index ], guPODCAST_STATUS_DELETED );
                }
            }

            ClearSelectedItems();
            ReloadItems();

            wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
            wxPostEvent( this, event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDeleteFromDrive( wxCommandEvent &event )
{
    if( GetSelectedCount() )
    {
        if( wxMessageBox( wxT( "Are you sure to delete the selected tracks from your drive?\nThis will permanently erase the selected tracks." ),
            wxT( "Remove tracks from drive" ), wxICON_QUESTION|wxYES|wxNO|wxNO_DEFAULT ) == wxYES )
        {
            guTrackArray SelectedTracks;
            wxArrayInt PodcastsIds;
            int Index;
            int Count;
            wxArrayInt Selected = GetSelectedItems( false );
            Count = Selected.Count();
            for( Index = Count - 1; Index >= 0; Index-- )
            {
                const guTrack & Track = m_Items[ Selected[ Index ] ];

                if( Track.m_Type == guTRACK_TYPE_PODCAST )
                {
                    PodcastsIds.Add( Track.m_SongId );
                }
                else if( Track.m_MediaViewer )
                {
                    SelectedTracks.Add( new guTrack( Track ) );
                }

                if( Track.m_Type != guTRACK_TYPE_RADIOSTATION &&
                    Track.m_Type != guTRACK_TYPE_JAMENDO &&
                    Track.m_Type != guTRACK_TYPE_MAGNATUNE )
                {
                    if( !wxRemoveFile( Track.m_FileName ) )
                    {
                        guLogMessage( wxT( "Error deleting '%s'" ), Track.m_FileName.c_str() );
                    }
                }

                if( Selected[ Index ] == m_CurItem )
                {
                    m_CurItem--;
                    event.SetId( ID_PLAYERPANEL_NEXTTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                }
                RemoveItem( Selected[ Index ] );
            }

            if( SelectedTracks.Count() )
            {
                wxArrayPtrVoid MediaViewerPtrs;
                GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

                if( ( Count = MediaViewerPtrs.Count() ) )
                {
                    for( Index = 0; Index < Count; Index++ )
                    {
                        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];
                        guTrackArray MediaViewerTracks;
                        GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );

                        guDbLibrary * Db = MediaViewer->GetDb();

                        Db->DeleteLibraryTracks( &MediaViewerTracks, true );
                    }
                }
            }

            if( ( Count = PodcastsIds.Count() ) )
            {
                guPodcastItemArray Podcasts;
                guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                DbPodcasts->GetPodcastItems( &Podcasts, PodcastsIds, 0, 0 );

                m_MainFrame->RemovePodcastDownloadItems( &Podcasts );

                for( Index = 0; Index < Count; Index++ )
                {
                    DbPodcasts->SetPodcastItemStatus( PodcastsIds[ Index ], guPODCAST_STATUS_DELETED );
                }
            }

            ClearSelectedItems();
            ReloadItems();

            wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
            wxPostEvent( this, event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSaveClicked( wxCommandEvent &event )
{
    int Index;
    int Count;
    wxArrayInt SelectedItems = GetSelectedItems( false );
    guTrackArray SelectedTracks;

    if( ( Count = SelectedItems.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ SelectedItems[ Index ] ];
            if( Track.m_MediaViewer )
                SelectedTracks.Add( new guTrack( Track ) );
        }
    }
    else
    {
        Count = m_Items.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ Index ];
            if( Track.m_MediaViewer )
                SelectedTracks.Add( new guTrack( Track ) );
        }
    }

    if( SelectedTracks.Count() )
    {
        wxArrayPtrVoid MediaViewerPtrs;
        GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

        if( MediaViewerPtrs.Count() )
        {
            guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ 0 ];
            guTrackArray MediaViewerTracks;
            GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );

            wxArrayInt TrackIds;
            Count = MediaViewerTracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                TrackIds.Add( MediaViewerTracks[ Index ].m_SongId );
            }

            guDbLibrary * Db = MediaViewer->GetDb();
            guListItems PlayLists;

            Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );
            guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( m_MainFrame, Db, &TrackIds, &PlayLists );
            if( PlayListAppendDlg->ShowModal() == wxID_OK )
            {
                int Selected = PlayListAppendDlg->GetSelectedPlayList();
                if( Selected == wxNOT_FOUND )
                {
                    wxString PLName = PlayListAppendDlg->GetPlaylistName();
                    if( PLName.IsEmpty() )
                    {
                        PLName = _( "UnNamed" );
                    }
                    Db->CreateStaticPlayList( PLName, TrackIds );
                }
                else
                {
                    int PLId = PlayLists[ Selected ].m_Id;
                    wxArrayInt OldSongs;
                    Db->GetPlayListSongIds( PLId, &OldSongs );
                    if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                    {
                        Db->UpdateStaticPlayList( PLId, TrackIds );
                        Db->AppendStaticPlayList( PLId, OldSongs );
                    }
                    else                                                // END
                    {
                        Db->AppendStaticPlayList( PLId, TrackIds );
                    }
                }

                MediaViewer->UpdatePlaylists();
            }
            PlayListAppendDlg->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

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
            if( !Track->m_Offset && ( Track->m_Type < guTRACK_TYPE_RADIOSTATION ) )
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
            if( !Track->m_Offset && ( Track->m_Type < guTRACK_TYPE_RADIOSTATION ) )
            {
                Songs.Add( new guTrack( * Track ) );
            }
        }
    }

    if( !Songs.Count() )
        return;

    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Songs, &Images, &Lyrics, &ChangedFlags );

    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            guUpdateTracks( Songs, Images, Lyrics, ChangedFlags );

            UpdateTracks( Songs, ChangedFlags );

            // Update the track in database, playlist, etc
            m_MainFrame->UpdatedTracks( guUPDATED_TRACKS_NONE, &Songs );
        }
        guImagePtrArrayClean( &Images );
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnEditLabelsClicked( wxCommandEvent &event )
{
    guTrackArray SelectedTracks;

    //
    wxArrayInt SelectedItems = GetSelectedItems( false );
    int Index;
    int Count = SelectedItems.Count();
    if( Count )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ SelectedItems[ Index ] ];
            if( Track.m_MediaViewer )
            {
                SelectedTracks.Add( new guTrack( Track ) );
            }
        }
    }
    else
    {
        // If there is no selection then use all songs that are
        // recognized in the database.
        Count = m_Items.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ Index ];
            if( Track.m_MediaViewer )
            {
                SelectedTracks.Add( new guTrack( Track ) );
            }
        }
    }

    if( SelectedTracks.Count() )
    {
        wxArrayPtrVoid MediaViewerPtrs;
        GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

        if( MediaViewerPtrs.Count() )
        {
            guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ 0 ];
            guTrackArray MediaViewerTracks;
            GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );


            guListItems Tracks;
            wxArrayInt  TrackIds;

            for( Index = 0; Index < Count; Index++ )
            {
                const guTrack &Track = MediaViewerTracks[ Index ];
                Tracks.Add( new guListItem( Track.m_SongId, Track.m_SongName ) );
                TrackIds.Add( Track.m_SongId );
            }

            guDbLibrary * Db = MediaViewer->GetDb();
            guArrayListItems LabelSets = Db->GetSongsLabels( TrackIds );

            guLabelEditor * LabelEditor = new guLabelEditor( this, Db, _( "Tracks Labels Editor" ), false, &Tracks, &LabelSets );
            if( LabelEditor )
            {
                if( LabelEditor->ShowModal() == wxID_OK )
                {
                    // Update the labels in the files
                    Db->UpdateSongsLabels( LabelSets );
                }

                LabelEditor->Destroy();

                wxCommandEvent event( wxEVT_MENU, ID_LABEL_UPDATELABELS );
                wxPostEvent( MediaViewer, event );
            }
        }
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
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_TRACK );
        evt.SetInt( m_Items[ SelItem ].m_SongId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectArtist( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( m_Items[ SelItem ].m_ArtistId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectAlbum( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ALBUM );
        evt.SetInt( m_Items[ SelItem ].m_AlbumId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectAlbumArtist( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ALBUMARTIST );
        evt.SetInt( m_Items[ SelItem ].m_AlbumArtistId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectComposer( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_COMPOSER );
        evt.SetInt( m_Items[ SelItem ].m_ComposerId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
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
            guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

            wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_YEAR );
            evt.SetInt( SelYear );
            evt.SetClientData( MediaViewer );
            evt.SetExtraLong( m_Items[ SelItem ].m_Type );
            wxPostEvent( m_MainFrame, evt );
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
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_GENRE );
        evt.SetInt( m_Items[ SelItem ].m_GenreId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
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
void guPlayList::SetNextTracks( wxCommandEvent &event )
{
    m_DragOverItem = ( m_CurItem == wxNOT_FOUND ) ? 0 : m_CurItem;
    m_DragOverAfter = true;
    MoveSelection();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSearchLinkClicked( wxCommandEvent &event )
{
    unsigned long cookie;
    int Item = GetFirstSelected( cookie );
    if( Item != wxNOT_FOUND )
    {
        ExecuteOnlineLink( event.GetId(), GetSearchText( Item ) );
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

        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, CONFIG_PATH_COMMANDS_EXECS );
            wxASSERT( Commands.Count() > 0 );

            index -= ID_COMMANDS_BASE;
            wxString CurCmd = Commands[ index ];

            const guTrack &Track = m_Items[ Selection[ 0 ] ];

            if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
            {
                //wxString Path = wxT( "\"" ) + wxPathOnly( m_Items[ Selection[ 0 ] ].m_FileName ) + wxT( "\"" );
                wxString Path = wxPathOnly( Track.m_FileName );
                Path.Replace( wxT( " " ), wxT( "\\ " ) );
                CurCmd.Replace( guCOMMAND_ALBUMPATH, Path );
            }

            if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
            {
                int CoverId = Track.m_CoverId;
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 && Track.m_MediaViewer )
                {
                    CoverPath = Track.m_MediaViewer->GetDb()->GetCoverPath( CoverId );
                }
                else
                {
                    CoverPath = FindCoverFile( wxPathOnly( Track.m_FileName ) );
                }

                if( !CoverPath.IsEmpty() )
                {
                    CurCmd.Replace( guCOMMAND_COVERPATH, wxT( "\"" ) + CoverPath + wxT( "\"" ) );
                }
            }

            if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
            {
                wxString SongList = wxEmptyString;
                count = Selection.Count();
                if( count )
                {
                    for( index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + m_Items[ Selection[ index ] ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
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
            if( ( m_Items[ item ].m_FileName == tracks->Item( index ).m_FileName ) &&
                ( m_Items[ item ].m_Offset == tracks->Item( index ).m_Offset ) )
            {
                m_Items[ item ] = tracks->Item( index );
                found = true;
            }
        }
    }
    if( found )
    {
        RefreshAll();

        wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
        wxPostEvent( this, event );
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
        if( ( m_Items[ item ].m_FileName == track->m_FileName ) &&
            ( m_Items[ item ].m_Offset == track->m_Offset ) )
        {
            m_Items[ item ] = * track;
            found = true;
        }
    }
    if( found )
    {
        RefreshAll();

        wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
        wxPostEvent( this, event );
    }
}

//// -------------------------------------------------------------------------------- //
//wxString inline guPlayList::GetItemName( const int row ) const
//{
//    return m_Items[ row ].m_SongName;
//}
//
//// -------------------------------------------------------------------------------- //
//int inline guPlayList::GetItemId( const int row ) const
//{
//    return row;
//}

// -------------------------------------------------------------------------------- //
wxString guPlayList::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Items[ item ].m_ArtistName.c_str(),
        m_Items[ item ].m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
void guPlayList::StopAtEnd( void )
{
    int ItemToFlag = wxNOT_FOUND;
    int Index;
    int Count;
    wxArrayInt Selection = GetSelectedItems( false );
    if( ( Count = Selection.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( Selection[ Index ] > ItemToFlag )
                ItemToFlag = Selection[ Index ];
        }
        //if( ( ItemToFlag != wxNOT_FOUND ) && ( ItemToFlag < ( int ) m_Items.Count() ) ) )
        //    ItemToFlag;
    }
    else
    {
        if( ( m_CurItem >= 0 ) && ( m_CurItem < ( int ) m_Items.Count() ) )
            ItemToFlag = m_CurItem;
    }

    if( ( ItemToFlag >= 0 ) && ( ItemToFlag < ( int ) m_Items.Count() ) )
    {
        m_Items[ ItemToFlag ].m_Type = guTrackType( ( int ) m_Items[ ItemToFlag ].m_Type ^ guTRACK_TYPE_STOP_HERE );
        RefreshAll();
        if( ItemToFlag == m_CurItem )
        {
            m_PlayerPanel->StopAtEnd();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::ClearStopAtEnd( void )
{
    if( ( m_CurItem >= 0 ) && ( m_CurItem < ( int ) m_Items.Count() ) )
    {
        m_Items[ m_CurItem ].m_Type = guTrackType( ( int ) m_Items[ m_CurItem ].m_Type & 0x7FFFFFFF );
        RefreshAll();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSetRating( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnSetRating( %i )" ), event.GetId() - ID_PLAYERPANEL_SETRATING_0 );
    int Index;
    int Count;
    wxArrayInt Selected = GetSelectedItems( false );
    Count = Selected.Count();
    if( Count )
    {
        int Rating = event.GetId() - ID_PLAYERPANEL_SETRATING_0;
        guTrackArray UpdatedTracks;

        for( Index = 0; Index < Count; Index++ )
        {
            int ItemNum = Selected[ Index ];
            m_Items[ ItemNum ].m_Rating = Rating;
            RefreshRow( ItemNum );

            UpdatedTracks.Add( m_Items[ ItemNum ] );
        }

        SetTracksRating( UpdatedTracks, Rating );
    }
}


// -------------------------------------------------------------------------------- //
void guPlayList::SetTrackRating( const guTrack &track, const int rating )
{
    guTrackArray Tracks;
    Tracks.Add( track );
    SetTracksRating( Tracks, rating );
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetTracksRating( const guTrackArray &tracks, const int rating )
{
    wxArrayPtrVoid MediaViewerPtrs;
    GetMediaViewersList( tracks, MediaViewerPtrs );

    guTrackArray CurrentTracks;
    wxArrayInt   CurrentFlags;
    int Index;
    int Count = MediaViewerPtrs.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        CurrentTracks.Empty();
        CurrentFlags.Empty();

        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];

        GetMediaViewerTracks( tracks, guTRACK_CHANGED_DATA_RATING, MediaViewer, CurrentTracks, CurrentFlags );

        if( MediaViewer->GetEmbeddMetadata() )
        {
            guImagePtrArray Images;
            wxArrayString Lyrics;
            guUpdateTracks( CurrentTracks, Images, Lyrics, CurrentFlags );
        }

        guDbLibrary * Db = MediaViewer->GetDb();
        Db->SetTracksRating( &CurrentTracks, rating );

        MediaViewer->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &CurrentTracks );

        m_PlayerPanel->UpdatedTracks( &CurrentTracks );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::MediaViewerCreated( const wxString &uniqueid, guMediaViewer * mediaviewer )
{
    if( m_PendingLoadIds.Index( uniqueid ) != wxNOT_FOUND )
    {
        CheckPendingLoadItems( uniqueid, mediaviewer );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::MediaViewerClosed( guMediaViewer * mediaviewer )
{
    int Index;
    int Count = m_Items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack & Track = m_Items[ Index ];
        if( Track.m_MediaViewer == mediaviewer )
        {
            Track.m_MediaViewer = NULL;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::CheckPendingLoadItems( const wxString &uniqueid, guMediaViewer * mediaviewer )
{
    int Type;
    guDbLibrary * Db = NULL;

    if( !mediaviewer )
        return;

    if( uniqueid == wxT( "Jamendo" ) )
    {
        Type = guTRACK_TYPE_JAMENDO;
    }
    else if( uniqueid == wxT( "Magnatune" ) )
    {
        Type = guTRACK_TYPE_MAGNATUNE;
    }
    else
        return;

    Db = mediaviewer->GetDb();
    if( !Db )
        return;

    wxString FileName;
    int Index;
    int Count = m_Items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack & Track = m_Items[ Index ];
        if( Track.m_Type == Type )
        {
            //guLogMessage( wxT( "Trying: '%s'" ), Track.m_FileName.c_str() );
            if( Type == guTRACK_TYPE_JAMENDO )
            {
                long Id;
                FileName = Track.m_FileName;
                wxString IdStr = FileName.Mid( FileName.Find( wxT( "/?id=" ) ) + 5 );
                IdStr = IdStr.Mid( 0, IdStr.Find( wxT( "&" ) ) );
                IdStr.ToLong( &Id );
                if( Id )
                {
                    Db->FindTrackId( Id, &Track );
                    Track.m_Type     = guTRACK_TYPE_JAMENDO;
                    Track.m_FileName = FileName;
                    Track.m_MediaViewer = mediaviewer;
                }
            }
            else
            {
                FileName = Track.m_FileName;
                FileName.Replace( wxT( " " ), wxT( "%20" ) );
                wxString SearchStr = FileName;
                int FoundPos;
                if( ( FoundPos = SearchStr.Find( wxT( "@stream.magnatune" ) ) ) != wxNOT_FOUND )
                {
                    SearchStr = SearchStr.Mid( FoundPos );
                    SearchStr.Replace( wxT( "@stream." ), wxT( "http://he3." ) );
                    SearchStr.Replace( wxT( "_nospeech" ), wxEmptyString );
                }
                else if( ( FoundPos = SearchStr.Find( wxT( "@download.magnatune" ) ) ) != wxNOT_FOUND )
                {
                    SearchStr = SearchStr.Mid( FoundPos );
                    SearchStr.Replace( wxT( "@download." ), wxT( "http://he3." ) );
                    SearchStr.Replace( wxT( "_nospeech" ), wxEmptyString );
                }

                guLogMessage( wxT( "Searching for track '%s'" ), SearchStr.c_str() );

                ( ( guMagnatuneLibrary * ) Db )->GetTrackId( SearchStr, &Track );
                Track.m_Type     = guTRACK_TYPE_MAGNATUNE;
                Track.m_FileName = FileName;
                Track.m_MediaViewer = mediaviewer;
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCreateSmartPlaylist( wxCommandEvent &event )
{
    wxArrayInt Selected = GetSelectedItems( false );
    if( Selected.Count() )
    {
        const guTrack &Track = m_Items[ Selected[ 0 ] ];
        if( Track.m_MediaViewer )
        {
            Track.m_MediaViewer->CreateSmartPlaylist( Track.m_ArtistName, Track.m_SongName );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::SavePlaylistTracks( void )
{
    guTrackArray Tracks;
    int Index;
    int Count;
    guConfig * Config = ( guConfig * ) guConfig::Get();

    if( Config->ReadBool( CONFIG_KEY_PLAYLIST_SAVE_ON_CLOSE, true, CONFIG_PATH_PLAYLIST ) )
    {
        Count = m_Items.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_Items[ Index ].m_Type < guTRACK_TYPE_IPOD )
                Tracks.Add( new guTrack( m_Items[ Index ] ) );
        }
        Config->SavePlaylistTracks( Tracks, m_CurItem );
    }
    else
    {
        Config->SavePlaylistTracks( Tracks, wxNOT_FOUND );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::LoadPlaylistTracks( void )
{
    wxBusyInfo BusyInfo( _( "Loading tracks. Please wait" ) );
    wxTheApp->Yield();

    int Index;
    int Count;

    guMainApp * MainApp = ( guMainApp * ) wxTheApp;
    if( MainApp && MainApp->argc > 1 )
    {
        Count = MainApp->argc;
        for( Index = 1; Index < Count; Index++ )
        {
            //guLogMessage( wxT( "%u-%u %s" ), Index, MainApp->argc, MainApp->argv[ Index ] );
            AddPlayListItem( MainApp->argv[ Index ], guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND );
        }
        m_CurItem = wxNOT_FOUND;
        m_StartPlaying = true;
    }
    else
    {
        guTrackArray Tracks;
        guConfig * Config = ( guConfig * ) guConfig::Get();
        int CurItem = Config->LoadPlaylistTracks( Tracks );
        if( ( size_t ) m_CurItem > Tracks.Count() )
            m_CurItem = wxNOT_FOUND;

        wxTheApp->Yield();

        Count = Tracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            if( Tracks[ Index ].m_Offset ||
                ( Tracks[ Index ].m_Type == guTRACK_TYPE_RADIOSTATION ) )
            {
                m_Items.Add( new guTrack( Tracks[ Index ] ) );
            }
            else
            {
                AddPlayListItem( Tracks[ Index ].m_FileName, guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND );
            }
        }
        m_CurItem = CurItem;
    }

    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
    event.SetInt( 1 );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
void guPlayList::StartSavePlaylistTimer( wxCommandEvent &event )
{
    if( !m_SavePlaylistTimer )
    {
        m_SavePlaylistTimer = new wxTimer( this );
    }

    m_SavePlaylistTimer->Start( SAVE_PLAYLIST_TIMEOUT, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSavePlaylistTimer( wxTimerEvent & )
{
    SavePlaylistTracks();
}

}

// -------------------------------------------------------------------------------- //
