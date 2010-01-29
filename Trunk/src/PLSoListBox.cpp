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
#include "PLSoListBox.h"

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
guPLSoListBox::guPLSoListBox( wxWindow * parent, guDbLibrary * db, wxString confname, int style ) :
             guSoListBox( parent, db, confname, style | guLISTVIEW_ALLOWDRAG | guLISTVIEW_ALLOWDROP | guLISTVIEW_DRAGSELFITEMS )
{
    m_PLId = wxNOT_FOUND;
    m_PLType = wxNOT_FOUND;

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPLSoListBox::~guPLSoListBox()
{
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::GetItemsList( void )
{
    m_PLSetIds.Empty();
    if( m_PLId > 0 )
    {
        m_Db->GetPlayListSongs( m_PLId, m_PLType, &m_Items );
        m_Db->GetPlayListSetIds( m_PLId, &m_PLSetIds );
    }
    else
    {
        m_Items.Empty();
    }

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::SetPlayList( int plid, int pltype )
{
    m_PLId = plid;
    m_PLType = pltype;
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedItems().Count();

    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_SONG_DELETE, _( "Delete" ), _( "Delete the current selected tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_del ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    guSoListBox::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_SONG_DELETE );
        GetParent()->AddPendingEvent( evt );
        return;
    }

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::OnDropFile( const wxString &filename )
{
    if( m_PLId > 0 && m_PLType == GUPLAYLIST_STATIC )
    {
        guLogMessage( wxT( "Adding file '%s'" ), filename.c_str() );
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
    if( m_PLId > 0 && m_PLType == GUPLAYLIST_STATIC )
    {
        if( m_DropIds.Count() )
        {
            m_Db->AppendStaticPlayList( m_PLId, m_DropIds );

            m_DropIds.Clear();
        }
        ReloadItems();
    }
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::MoveSelection( void )
{
    wxArrayInt   MoveIds;
    wxArrayInt   MoveIndex;
    wxArrayInt   ItemIds;

    if( m_DragOverItem == wxNOT_FOUND ||
        m_PLId != GUPLAYLIST_STATIC )
        return;

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
    int InsertPos = m_DragOverItem;
    if( m_DragOverAfter )
        InsertPos++;

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
    m_Db->UpdateStaticPlayList( m_PLId, ItemIds );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
int guPLSoListBox::GetPlayListSetIds( wxArrayInt * setids ) const
{
    unsigned long cookie;
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        setids->Add( m_PLSetIds[ item ] );
        item = GetNextSelected( cookie );
    }
    return setids->Count();
}

// -------------------------------------------------------------------------------- //
