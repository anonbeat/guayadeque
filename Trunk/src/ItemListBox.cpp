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
#include "ItemListBox.h"
#include "Utils.h"

#include <wx/dnd.h>

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
    int FirstVisible = 0; // = GetFirstVisibleLine();

    if( reset )
    {
        SetSelection( -1 );
    }
    else
    {
        FirstVisible = GetFirstVisibleLine();
        Selection = GetSelectedItems( false );
    }

    m_Items->Empty();

    GetItemsList();
    m_Items->Insert( new guListItem( 0, wxString::Format( wxT( "%s (%u)" ), _( "All" ), m_Items->Count() ) ), 0 );
    SetItemCount( m_Items->Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToLine( FirstVisible );
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

// -------------------------------------------------------------------------------- //
