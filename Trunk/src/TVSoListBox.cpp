// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "TVSoListBox.h"

#include "Accelerators.h"
#include "Config.h" // Configuration
#include "Commands.h"
#include "Images.h"
#include "MainApp.h"
#include "OnlineLinks.h"
#include "PlayList.h" // LenToString
#include "RatingCtrl.h"
#include "TagInfo.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guTVSoListBox::guTVSoListBox( wxWindow * parent, guDbLibrary * db, wxString confname, int style ) :
             guSoListBox( parent, NULL, db, confname, style | guLISTVIEW_ALLOWDRAG | guLISTVIEW_ALLOWDROP | guLISTVIEW_DRAGSELFITEMS )
{
    m_LibPanel = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_TracksOrder = Config->ReadNum( wxT( "TracksOrder" ), 0, wxT( "General" ) );
    m_TracksOrderDesc = Config->ReadBool( wxT( "TracksOrderDesc" ), 0, wxT( "General" ) );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guTVSoListBox::~guTVSoListBox()
{
}

// -------------------------------------------------------------------------------- //
void guTVSoListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_SONG_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_SONG_EDITLABELS );
    RealAccelCmds.Add( ID_SONG_EDITTRACKS );
    RealAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_PLAYLIST_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guTVSoListBox::GetItemsList( void )
{
    m_Items.Empty();

    if( m_Filters.Count() )
    {
        m_Db->GetSongs( m_Filters, &m_Items, m_TextFilters, m_TracksOrder, m_TracksOrderDesc );
    }

    SetItemCount( m_Items.Count() );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guTVSoListBox::SetFilters( guTreeViewFilterArray &filters )
{
    m_Filters = filters;
    ReloadItems();
}

//// -------------------------------------------------------------------------------- //
//void guTVSoListBox::OnDropFile( const wxString &filename )
//{
////    if( ( m_PLIds.Count() == 1 ) && m_PLTypes[ 0 ] == guTVAYLIST_TYPE_STATIC )
////    {
////        //guLogMessage( wxT( "Adding file '%s'" ), filename.c_str() );
////        if( guIsValidAudioFile( filename ) )
////        {
////            if( wxFileExists( filename ) )
////            {
////                guTrack Track;
////                if( m_Db->FindTrackFile( filename, &Track ) )
////                {
////                    m_DropIds.Add( Track.m_SongId );
////                }
////            }
////        }
////    }
//}
//
//// -------------------------------------------------------------------------------- //
//void guTVSoListBox::OnDropEnd( void )
//{
////    int index;
////    int count;
////    wxArrayInt ItemIds;
////
////    if( ( m_PLIds.Count() == 1 ) && ( m_PLTypes[ 0 ] == guTVAYLIST_TYPE_STATIC ) )
////    {
////        if( m_DropIds.Count() )
////        {
////            count = m_Items.Count();
////            for( index = 0; index < count; index++ )
////            {
////                ItemIds.Add( m_Items[ index ].m_SongId );
////            }
////
////            //int InsertPos = m_DragOverItem + m_DragOverAfter;
////            int InsertPos = m_DragOverItem + m_DragOverAfter;
////            if( m_DragOverItem == wxNOT_FOUND )
////                InsertPos = m_Items.Count();
////            //guLogMessage( wxT( "Pos: %i + %i  %i of %i " ), m_DragOverItem, m_DragOverAfter, InsertPos, m_Items.Count() );
////
////            count = m_DropIds.Count();
////            for( index = 0; index < count; index++ )
////            {
////                ItemIds.Insert( m_DropIds[ index ], InsertPos + index );
////            }
////
////            // Save it to the database
////            m_Db->UpdateStaticPlayList( m_PLIds[ 0 ], ItemIds );
////            m_Db->UpdateStaticPlayListFile( m_PLIds[ 0 ] );
////
////            m_DropIds.Clear();
////        }
////        ReloadItems();
////    }
//}
//
//// -------------------------------------------------------------------------------- //
//void guTVSoListBox::MoveSelection( void )
//{
////    wxArrayInt   MoveIds;
////    wxArrayInt   MoveIndex;
////    wxArrayInt   ItemIds;
////
////    if( ( m_PLIds.Count() != 1 ) || ( m_PLTypes[ 0 ] != guTVAYLIST_TYPE_STATIC ) )
////        return;
////
////    // Copy the elements we are going to move
////    unsigned long cookie;
////    int item = GetFirstSelected( cookie );
////    while( item != wxNOT_FOUND )
////    {
////        MoveIndex.Add( item );
////        MoveIds.Add( m_Items[ item ].m_SongId );
////        item = GetNextSelected( cookie );
////    }
////
////    // Get the position where to move it
////    int InsertPos;
////    if( m_DragOverItem != wxNOT_FOUND )
////        InsertPos = m_DragOverItem + m_DragOverAfter;
////    else
////        InsertPos = m_Items.Count();
////
////    // Remove the elements from the original position
////    int index;
////    int count = MoveIndex.Count();
////    for( index = count - 1; index >= 0; index-- )
////    {
////        m_Items.RemoveAt( MoveIndex[ index ] );
////
////        if( MoveIndex[ index ] < InsertPos )
////            InsertPos--;
////    }
////
////    count = m_Items.Count();
////    for( index = 0; index < count; index++ )
////    {
////        ItemIds.Add( m_Items[ index ].m_SongId );
////    }
////
////    count = MoveIds.Count();
////    for( index = 0; index < count; index++ )
////    {
////        ItemIds.Insert( MoveIds[ index ], InsertPos + index );
////    }
////
////    // Save it to the database
////    m_Db->UpdateStaticPlayList( m_PLIds[ 0 ], ItemIds );
////    m_Db->UpdateStaticPlayListFile( m_PLIds[ 0 ] );
////
////    ReloadItems();
//}

// -------------------------------------------------------------------------------- //
int guTVSoListBox::GetSelectedSongs( guTrackArray * tracks )
{
    unsigned long cookie;
    m_ItemsMutex.Lock();
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        tracks->Add( new guTrack( m_Items[ item ] ) );
        item = GetNextSelected( cookie );
    }
    m_ItemsMutex.Unlock();
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
void guTVSoListBox::GetAllSongs( guTrackArray * tracks )
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
wxString guTVSoListBox::GetItemName( const int row ) const
{
    return m_Items[ row ].m_SongName;
}

// -------------------------------------------------------------------------------- //
int guTVSoListBox::GetItemId( const int row ) const
{
    return m_Items[ row ].m_SongId;
}

// -------------------------------------------------------------------------------- //
