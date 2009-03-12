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
#include "ArListBox.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"

// -------------------------------------------------------------------------------- //
void guArListBox::GetItemsList( void )
{
    m_Items.Add( new guListItem( 0, _( "All" ) ) );
    m_Db->GetArtists( &m_Items );
}

// -------------------------------------------------------------------------------- //
int guArListBox::GetSelectedSongs( guTrackArray * songs ) const
{
    return m_Db->GetArtistsSongs( GetSelection(), songs );
}

// -------------------------------------------------------------------------------- //
void guArListBox::GetContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_PLAY, _( "Play" ), _( "Play current selected artists" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_media_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_ENQUEUE, _( "Enqueue" ), _( "Add current selected artists to playlist" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected artists" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_EDITTRACKS, _( "Edit Songs" ), _( "Edit the songs from the selected artists" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_edit_copy ) );
    Menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
