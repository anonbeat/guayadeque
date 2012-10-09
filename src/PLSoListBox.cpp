// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
//	anonbeat@gmail.com
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
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "PLSoListBox.h"

#include "Accelerators.h"
#include "Config.h" // Configuration
#include "Commands.h"
#include "DynamicPlayList.h"
#include "Images.h"
#include "MainApp.h"
#include "OnlineLinks.h"
#include "RatingCtrl.h"
#include "TagInfo.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guPLSoListBox::guPLSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, int style ) :
             guSoListBox( parent, mediaviewer, confname, style | guLISTVIEW_ALLOWDRAG | guLISTVIEW_ALLOWDROP | guLISTVIEW_DRAGSELFITEMS | guLISTVIEW_COLUMN_SORTING )
{
    m_TracksOrder = wxNOT_FOUND;
    m_DisableSorting = false;

    Connect( ID_TRACKS_RANDOMIZE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPLSoListBox::OnRandomizeTracks ), NULL, this );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPLSoListBox::~guPLSoListBox()
{
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_RANDOMPLAY );

    RealAccelCmds.Add( ID_TRACKS_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_TRACKS_EDITLABELS );
    RealAccelCmds.Add( ID_TRACKS_EDITTRACKS );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_PLAYLIST_SEARCH );
    RealAccelCmds.Add( ID_TRACKS_RANDOMIZE );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::GetItemsList( void )
{
    m_PLSetIds.Empty();
    if( m_PLIds.Count() )
    {
        m_Db->GetPlayListSongs( m_PLIds, m_PLTypes, &m_Items, &m_PLSetIds, &m_TracksLength, &m_TracksSize, m_TracksOrder, m_TracksOrderDesc );
        //m_Db->GetPlayListSetIds( m_PLIds, &m_PLSetIds, m_TracksOrder, m_TracksOrderDesc );
    }
    else
    {
        m_Items.Empty();
    }
    SetItemCount( m_Items.Count() );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::SetPlayList( int plid, int pltype )
{
    m_PLIds.Empty();
    m_PLTypes.Empty();
    if( plid != wxNOT_FOUND )
    {
        m_PLIds.Add( plid );
        m_PLTypes.Add( pltype );

        if( pltype == guPLAYLIST_TYPE_DYNAMIC )
        {
            guDynPlayList DynPlayList;
            m_Db->GetDynamicPlayList( plid, &DynPlayList );
            m_DisableSorting = false;
            if( DynPlayList.m_Sorted )
            {
                SetTracksOrder( wxNOT_FOUND );
                m_DisableSorting = true;
                return;
            }
        }
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::SetPlayList( const wxArrayInt &ids, const wxArrayInt &types )
{
    m_PLIds = ids;
    m_PLTypes = types;

    if( m_DisableSorting )
    {
        m_DisableSorting = false;
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedCount();
    if( SelCount )
    {
        guSoListBox::CreateContextMenu( Menu );

        int InsertPosition = 12;

        if( !m_DisableSorting && ( m_TracksOrder == wxNOT_FOUND ) )
        {
            MenuItem = new wxMenuItem( Menu, ID_TRACKS_RANDOMIZE,
                            wxString( _( "Randomize Playlist" ) )  + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_RANDOMPLAY ),
                            _( "Randomize the songs in the playlist" ) );
            Menu->Insert( 9, MenuItem );
            InsertPosition = 13;
        }

        if( ( m_PLTypes.Count() == 1 ) && ( m_PLTypes[ 0 ] == guPLAYLIST_TYPE_STATIC ) )
        {
            MenuItem = new wxMenuItem( Menu, ID_TRACKS_DELETE, _( "Remove from Playlist" ), _( "Delete the current selected tracks" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_del ) );
            Menu->Insert( InsertPosition, MenuItem );
        }
    }

}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_TRACKS_DELETE );
        GetParent()->AddPendingEvent( evt );
        return;
    }

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::OnDropFile( const wxString &filename )
{
    if( ( m_PLIds.Count() == 1 ) && m_PLTypes[ 0 ] == guPLAYLIST_TYPE_STATIC )
    {
        //guLogMessage( wxT( "Adding file '%s'" ), filename.c_str() );
        if( guIsValidAudioFile( filename ) )
        {
            if( wxFileExists( filename ) )
            {
                guTrack Track;
                if( m_Db->FindTrackFile( filename, &Track ) )
                {
                    m_DropIds.Add( Track.m_SongId );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::OnDropEnd( void )
{
    int index;
    int count;
    wxArrayInt ItemIds;

    if( ( m_PLIds.Count() == 1 ) && ( m_PLTypes[ 0 ] == guPLAYLIST_TYPE_STATIC ) )
    {
        if( m_DropIds.Count() )
        {
            count = m_Items.Count();
            for( index = 0; index < count; index++ )
            {
                ItemIds.Add( m_Items[ index ].m_SongId );
            }

            //int InsertPos = m_DragOverItem + m_DragOverAfter;
            int InsertPos = m_DragOverItem + m_DragOverAfter;
            if( m_DragOverItem == wxNOT_FOUND )
                InsertPos = m_Items.Count();
            //guLogMessage( wxT( "Pos: %i + %i  %i of %i " ), m_DragOverItem, m_DragOverAfter, InsertPos, m_Items.Count() );

            count = m_DropIds.Count();
            for( index = 0; index < count; index++ )
            {
                ItemIds.Insert( m_DropIds[ index ], InsertPos + index );
            }

            // Save it to the database
            m_Db->UpdateStaticPlayList( m_PLIds[ 0 ], ItemIds );
            m_Db->UpdateStaticPlayListFile( m_PLIds[ 0 ] );

            m_DropIds.Clear();
        }
        ReloadItems();
    }
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::MoveSelection( void )
{
    if( ( m_TracksOrder != wxNOT_FOUND ) ||
        ( m_PLIds.Count() != 1 ) ||
        ( m_PLTypes[ 0 ] != guPLAYLIST_TYPE_STATIC ) )
        return;

    wxArrayInt   MoveIds;
    wxArrayInt   MoveIndex;
    wxArrayInt   ItemIds;

    // Copy the elements we are going to move
    unsigned long cookie;
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        MoveIndex.Add( item );
        MoveIds.Add( m_Items[ item ].m_SongId );
        item = GetNextSelected( cookie );
    }

    // Get the position where to move it
    int InsertPos;
    if( m_DragOverItem != wxNOT_FOUND )
        InsertPos = m_DragOverItem + m_DragOverAfter;
    else
        InsertPos = m_Items.Count();

    // Remove the elements from the original position
    int index;
    int count = MoveIndex.Count();
    for( index = count - 1; index >= 0; index-- )
    {
        m_Items.RemoveAt( MoveIndex[ index ] );

        if( MoveIndex[ index ] < InsertPos )
            InsertPos--;
    }

    count = m_Items.Count();
    for( index = 0; index < count; index++ )
    {
        ItemIds.Add( m_Items[ index ].m_SongId );
    }

    count = MoveIds.Count();
    for( index = 0; index < count; index++ )
    {
        ItemIds.Insert( MoveIds[ index ], InsertPos + index );
    }

    // Save it to the database
    m_Db->UpdateStaticPlayList( m_PLIds[ 0 ], ItemIds );
    m_Db->UpdateStaticPlayListFile( m_PLIds[ 0 ] );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
int guPLSoListBox::GetPlayListSetIds( wxArrayInt * setids ) const
{
    unsigned long cookie;
    if( m_PLSetIds.Count() )
    {
        int item = GetFirstSelected( cookie );
        while( item != wxNOT_FOUND )
        {
            setids->Add( m_PLSetIds[ item ] );
            item = GetNextSelected( cookie );
        }
    }
    return setids->Count();
}

// -------------------------------------------------------------------------------- //
int guPLSoListBox::GetSelectedSongs( guTrackArray * tracks, const bool isdrag ) const
{
    unsigned long cookie;
    guPLSoListBox * self = wxConstCast( this, guPLSoListBox );
    self->m_ItemsMutex.Lock();
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        tracks->Add( new guTrack( m_Items[ item ] ) );
        item = GetNextSelected( cookie );
    }
    self->m_ItemsMutex.Unlock();
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::GetAllSongs( guTrackArray * tracks )
{
    int index;
    m_ItemsMutex.Lock();
    int count = m_Items.Count();
    for( index = 0; index < count; index++ )
    {
        tracks->Add( new guTrack( m_Items[ index ] ) );
    }
    m_ItemsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
wxString guPLSoListBox::GetItemName( const int row ) const
{
    return m_Items[ row ].m_SongName;
}

// -------------------------------------------------------------------------------- //
int guPLSoListBox::GetItemId( const int row ) const
{
    return m_Items[ row ].m_SongId;
}

// -------------------------------------------------------------------------------- //
wxString guPLSoListBox::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Items[ item - m_ItemsFirst ].m_ArtistName.c_str(),
        m_Items[ item - m_ItemsFirst ].m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::SetTracksOrder( const int order )
{
    if( !m_DisableSorting )
    {
        if( m_TracksOrder != order )
        {
            m_TracksOrder = order;
            if( order == wxNOT_FOUND )
                m_TracksOrderDesc = false;
        }
        else if( order != wxNOT_FOUND )
        {
            m_TracksOrderDesc = !m_TracksOrderDesc;
            if( !m_TracksOrderDesc )
            {
                m_TracksOrder = wxNOT_FOUND;
                m_TracksOrderDesc = false;
            }
        }

        int ColId = m_TracksOrder;

        // Create the Columns
        wxArrayString ColumnNames = GetColumnNames();
        int CurColId;
        int Index;
        int Count = ColumnNames.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            CurColId = GetColumnId( Index );
            SetColumnLabel( Index,
                ColumnNames[ CurColId ]  + ( ( ColId == CurColId ) ? ( m_TracksOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
        }

        ReloadItems();
    }
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::RandomizeTracks( void )
{
    int Index;
    int Pos;
    int NewPos;
    int Count = m_Items.Count();
    guTrack SavedItem;

    if( Count > 2 )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            do {
                Pos = guRandom( Count );
                NewPos = guRandom( Count );
            } while( Pos == NewPos );
            SavedItem = m_Items[ Pos ];
            m_Items[ Pos ] = m_Items[ NewPos ];
            m_Items[ NewPos ] = SavedItem;

            RefreshLine( Pos );
            RefreshLine( NewPos );
        }
        ClearSelectedItems();
    }
}

// -------------------------------------------------------------------------------- //
