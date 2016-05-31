// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "TVSoListBox.h"

#include "Accelerators.h"
#include "Config.h" // Configuration
#include "EventCommandIds.h"
#include "Images.h"
#include "MainApp.h"
#include "MediaViewer.h"
#include "OnlineLinks.h"
#include "RatingCtrl.h"
#include "TagInfo.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guTVSoListBox::guTVSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, int style ) :
             guSoListBox( parent, mediaviewer, confname, style | guLISTVIEW_ALLOWDRAG )
{
//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    m_TracksOrder = Config->ReadNum( wxT( "TracksOrder" ), 0, confname );
//    m_TracksOrderDesc = Config->ReadBool( wxT( "TracksOrderDesc" ), 0, confname );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guTVSoListBox::~guTVSoListBox()
{
//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    Config->WriteNum( wxT( "TracksOrder" ), m_TracksOrder, m_ConfName );
//    Config->WriteBool( wxT( "TracksOrderDesc" ), m_TracksOrderDesc, m_ConfName );
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
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_TRACKS_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_TRACKS_EDITLABELS );
    RealAccelCmds.Add( ID_TRACKS_EDITTRACKS );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
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
int guTVSoListBox::GetSelectedSongs( guTrackArray * tracks, const bool isdrag ) const
{
    unsigned long cookie;
    guTVSoListBox * self = wxConstCast( this, guTVSoListBox );
    self->m_ItemsMutex.Lock();
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        tracks->Add( new guTrack( m_Items[ item ] ) );
        item = GetNextSelected( cookie );
    }
    self->m_ItemsMutex.Unlock();
    m_MediaViewer->NormalizeTracks( tracks, isdrag );
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
wxString guTVSoListBox::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
            m_Items[ item ].m_ArtistName.c_str(),
            m_Items[ item ].m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
