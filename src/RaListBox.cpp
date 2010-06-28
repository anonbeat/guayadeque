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
#include "RaListBox.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "OnlineLinks.h"
#include "RatingCtrl.h"
#include "MainApp.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guRaListBox::guRaListBox( wxWindow * parent, guDbLibrary * db, const wxString &label ) :
             guListBox( parent, db, label, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG | guLISTVIEW_HIDE_HEADER )
{
    m_NormalStar   = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_normal_tiny + GURATING_STYLE_MID ) ) );
    m_SelectStar = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_highlight_tiny + GURATING_STYLE_MID ) ) );

    ReloadItems();
};

// -------------------------------------------------------------------------------- //
guRaListBox::~guRaListBox()
{
}

// -------------------------------------------------------------------------------- //
void guRaListBox::GetItemsList( void )
{
    m_Db->GetRatings( m_Items );
}

// -------------------------------------------------------------------------------- //
int guRaListBox::GetSelectedSongs( guTrackArray * songs ) const
{
    wxArrayInt Items = GetSelectedItems();
    int Index;
    int Count = Items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( Items[ Index ] )
            Items[ Index ]--;
    }
    return m_Db->GetRatingsSongs( Items, songs );
}

// -------------------------------------------------------------------------------- //
void guRaListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( row == 0 )
    {
        guListBox::DrawItem( dc, rect, row, col );
    }
    else
    {
        int Rating = GetItemId( row ) - 1;

        dc.SetBackgroundMode( wxTRANSPARENT );
        int x;
        int w = ( ( GURATING_STYLE_MID * 2 ) + GURATING_IMAGE_SIZE );

        for( x = 0; x < 5; x++ )
        {
           dc.DrawBitmap( ( x >= Rating ) ? * m_NormalStar : * m_SelectStar,
                          rect.x + 3 + ( w * x ), rect.y + 3, true );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRaListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    int SelCount = GetSelectedCount();

    MenuItem = new wxMenuItem( Menu, ID_RATING_PLAY, _( "Play" ), _( "Play the selected tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_RATING_ENQUEUE, _( "Enqueue" ), _( "Add current selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_RATING_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Add current selected tracks to playlist as Next Tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_RATING_EDITTRACKS, _( "Edit Songs" ), _( "Edit the selected tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_RATING_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
        Menu->Append( MenuItem );

    }
}

// -------------------------------------------------------------------------------- //
wxString guRaListBox::GetSearchText( int item ) const
{
    return GetItemName( item );
}

// -------------------------------------------------------------------------------- //
int guRaListBox::GetDragFiles( wxFileDataObject * files )
{
    guTrackArray Songs;
    int index;
    int count = GetSelectedSongs( &Songs );
    for( index = 0; index < count; index++ )
    {
       wxString FileName = Songs[ index ].m_FileName;
       FileName.Replace( wxT( "#" ), wxT( "%23" ) );
       //FileName.Replace( wxT( "%" ), wxT( "%25" ) );
       //guLogMessage( wxT( "Adding song '%s'" ), Songs[ index ].m_FileName.c_str() );
       files->AddFile( FileName );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
