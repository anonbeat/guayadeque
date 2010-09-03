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
#include "YeListBox.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "OnlineLinks.h"
#include "MainApp.h"
#include "Utils.h"
#include "LibPanel.h"

// -------------------------------------------------------------------------------- //
guYeListBox::guYeListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label ) :
             guListBox( parent, db, label, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG | guLISTVIEW_HIDE_HEADER )
{
    m_LibPanel = libpanel;

    ReloadItems();
};

// -------------------------------------------------------------------------------- //
guYeListBox::~guYeListBox()
{
}

// -------------------------------------------------------------------------------- //
void guYeListBox::GetItemsList( void )
{
    m_Db->GetYears( m_Items );
}

// -------------------------------------------------------------------------------- //
int guYeListBox::GetSelectedSongs( guTrackArray * songs ) const
{
    int Count = m_Db->GetYearsSongs( GetSelectedItems(), songs );
    m_LibPanel->NormalizeTracks( songs );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guYeListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    int SelCount = GetSelectedCount();
    int ContextMenuFlags = m_LibPanel->GetContextMenuFlags();

    MenuItem = new wxMenuItem( Menu, ID_YEAR_PLAY, _( "Play" ), _( "Play current selected artists" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_YEAR_ENQUEUE, _( "Enqueue" ), _( "Add current selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_YEAR_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Add current selected tracks to playlist as Next Tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_EDIT_TRACKS )
        {
            Menu->AppendSeparator();

            MenuItem = new wxMenuItem( Menu, ID_YEAR_EDITTRACKS, _( "Edit Songs" ), _( "Edit the selected tracks" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu->Append( MenuItem );
        }

        if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_COPY_TO )
        {
            Menu->AppendSeparator();

            MenuItem = new wxMenuItem( Menu, ID_YEAR_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
            Menu->Append( MenuItem );
        }
    }

    m_LibPanel->CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
wxString guYeListBox::GetSearchText( int item ) const
{
    return GetItemName( item );
}

// -------------------------------------------------------------------------------- //
int guYeListBox::GetDragFiles( wxFileDataObject * files )
{
    guTrackArray Songs;
    int index;
    int count = GetSelectedSongs( &Songs );
    for( index = 0; index < count; index++ )
    {
       wxString FileName = guFileDnDEncode( Songs[ index ].m_FileName );
       //FileName.Replace( wxT( "#" ), wxT( "%23" ) );
       //FileName.Replace( wxT( "%" ), wxT( "%25" ) );
       //guLogMessage( wxT( "Adding song '%s'" ), Songs[ index ].m_FileName.c_str() );
       files->AddFile( FileName );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
int guYeListBox::FindYear( const int year )
{
    int Index;
    int Count = m_Items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_Items->Item( Index ).m_Id == year )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
