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
#include "TaListBox.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"

// -------------------------------------------------------------------------------- //
guTaListBox::guTaListBox( wxWindow * parent, DbLibrary * NewDb, wxString Label ) : guListBox( parent, NewDb, Label )
{
    Connect( ID_LABEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::AddLabel ) );
    Connect( ID_LABEL_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::DelLabel ) );
    Connect( ID_LABEL_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::EditLabel ) );
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guTaListBox::~guTaListBox()
{
    Disconnect( ID_LABEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::AddLabel ) );
    Disconnect( ID_LABEL_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::DelLabel ) );
    Disconnect( ID_LABEL_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::EditLabel ) );
}

// -------------------------------------------------------------------------------- //
void guTaListBox::GetItemsList( void )
{
    m_Items.Add( new guListItem( 0, _( "All" ) ) );
    m_Db->GetLabels( &m_Items );
}

// -------------------------------------------------------------------------------- //
int guTaListBox::GetSelectedSongs( guTrackArray * Songs ) const
{
    return m_Db->GetLabelsSongs( GetSelection(), Songs );
}

// -------------------------------------------------------------------------------- //
void guTaListBox::GetContextMenu( wxMenu * Menu ) const
{
    //    menu->Append(Menu_Dummy_First, _T("&First item\tCtrl-F1"));
    //    menu->AppendSeparator();
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ADD, _( "Add Label" ), _( "Create a new label" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
    Menu->Append( MenuItem );

    if( GetSelection().Count() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LABEL_EDIT, _( "Edit Label" ), _( "Change selected label" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LABEL_DELETE, _( "Delete label" ), _( "Delete selected labels" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu->Append( MenuItem );
    }

//    Menu->AppendSeparator();
//
//    MenuItem = new wxMenuItem( Menu, ID_LABEL_CLEARSELECTION, _( "Clear selection" ), _( "Unselect all selected labels" ) );
//    //MenuItem->SetBitmap( wxBitmap( GU_CONFIG_IMAGES_DIR + wxT("/images/media-playback-start.png"), wxBITMAP_TYPE_ANY ) );
//    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_LABEL_PLAY, _( "Play" ), _( "Play current selected labels" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ENQUEUE, _( "Enqueue" ), _( "Add current selected labels to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_LABEL_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
    Menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
void guTaListBox::AddLabel( wxCommandEvent &event )
{
    //wxMessageBox( wxT( "AddLabel" ), wxT( "Information" ) );
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ), _( "Please enter the label name" ) );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        //wxMessageBox( EntryDialog->GetValue(), wxT( "Entered..." ) );
        m_Db->AddLabel( EntryDialog->GetValue() );
        ReloadItems();
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guTaListBox::DelLabel( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelection();
    int Count = Selection.Count();
    if( Count )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected labels?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            for( int Index = 0; Index < Count; Index++ )
            {
                m_Db->DelLabel( Selection[ Index ] );
            }
            ReloadItems();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTaListBox::EditLabel( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelection();
    if( Selection.Count() )
    {
        // Get the Index of the First Selected Item
        int item = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ), _( "Enter the new label name" ), m_Items[ item ].m_Name );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            m_Db->SetLabelName( Selection[ 0 ], EntryDialog->GetValue() );
            ReloadItems();
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
