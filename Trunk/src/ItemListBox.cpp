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
wxString guListBox::OnGetItemText( const int row, const int col ) const
{
    return GetItemName( row );
}

// -------------------------------------------------------------------------------- //
//wxString inline guListBox::GetItemName( const int row ) const
//{
//    return ( * m_Items )[ row ].m_Name;
//}

// -------------------------------------------------------------------------------- //
//int inline guListBox::GetItemId( const int row ) const
//{
//    return ( * m_Items )[ row ].m_Id;
//}

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
wxString guListBox::GetSearchText( const int item ) const
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void  guListBox::SetSelectedItems( const wxArrayInt &selection )
{
    guListView::SetSelectedItems( selection );

//    wxCommandEvent event( wxEVT_COMMAND_LISTBOX_SELECTED, GetId() );
//    event.SetEventObject( this );
//    event.SetInt( -1 );
//    (void) GetEventHandler()->ProcessEvent( event );
}

// -------------------------------------------------------------------------------- //
