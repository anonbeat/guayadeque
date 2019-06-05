// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#include "ItemListBox.h"
#include "Utils.h"

#include <wx/dnd.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guListBox::guListBox( wxWindow * parent, guDbLibrary * db, const wxString &label, int flags ) :
             guListView( parent, flags )
{
    m_Db = db;
    m_Items = new guListItems();

    guListViewColumn * Column = new guListViewColumn( label, 0 );
    InsertColumn( Column );
}

// -------------------------------------------------------------------------------- //
guListBox::~guListBox()
{
    if( m_Items )
        delete m_Items;

}

// -------------------------------------------------------------------------------- //
void guListBox::ReloadItems( bool reset )
{
    wxArrayInt Selection;
    int FirstVisible = 0; // = GetVisibleRowsBegin();

    if( reset )
    {
        SetSelection( -1 );
    }
    else
    {
        FirstVisible = GetVisibleRowsBegin();
        Selection = GetSelectedItems( false );
    }

    m_Items->Empty();

    GetItemsList();
    m_Items->Insert( new guListItem( 0, wxString::Format( wxT( "%s (%lu)" ), _( "All" ), m_Items->Count() ) ), 0 );
    SetItemCount( m_Items->Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToRow( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
int guListBox::FindItemId( const int id )
{
    int Index;
    int Count = m_Items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_Items->Item( Index ).m_Id == id )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

}

// -------------------------------------------------------------------------------- //
