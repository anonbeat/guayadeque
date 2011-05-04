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
void guTVSoListBox::GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
    m_Db->GetSongsCounters( m_Filters, m_TextFilters, count, len, size );
}

// -------------------------------------------------------------------------------- //
