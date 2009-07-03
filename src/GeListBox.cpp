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
#include "GeListBox.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
void guGeListBox::GetItemsList( void )
{
    m_Items.Add( new guListItem( 0, _( "All" ) ) );
    m_Db->GetGenres( &m_Items );
}

// -------------------------------------------------------------------------------- //
int guGeListBox::GetSelectedSongs( guTrackArray * Songs ) const
{
    return m_Db->GetGenresSongs( GetSelection(), Songs );
}

// -------------------------------------------------------------------------------- //
void guGeListBox::GetContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelection().Count();

    MenuItem = new wxMenuItem( Menu, ID_GENRE_PLAY, _( "Play" ), _( "Play current selected genres" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_GENRE_ENQUEUE, _( "Enqueue" ), _( "Add current selected genres to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_GENRE_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
    }
}

// -------------------------------------------------------------------------------- //
