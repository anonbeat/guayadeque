// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
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
#include "TreeViewFilterEditor.h"

#include "Config.h"
#include "Images.h"
#include "Utils.h"

#include <wx/tokenzr.h>

wxArrayString FilterItemNames;

// -------------------------------------------------------------------------------- //
guTreeViewFilterEditor::guTreeViewFilterEditor( wxWindow * parent, const wxString &filterentry ) :
    wxDialog( parent, wxID_ANY, _( "Filter Editor" ), wxDefaultPosition, wxSize( 410, 310 ), wxDEFAULT_DIALOG_STYLE )
{
    m_CurrentItem = wxNOT_FOUND;

	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

	MainSizer->Add( 0, 10, 0, wxEXPAND, 5 );

	wxBoxSizer * NameSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * NameLabel = new wxStaticText( this, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
	NameLabel->Wrap( -1 );
	NameSizer->Add( NameLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_NameTextCtrl = new wxTextCtrl( this, wxID_ANY, filterentry.BeforeFirst( wxT( ':' ) ), wxDefaultPosition, wxDefaultSize, 0 );
	NameSizer->Add( m_NameTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MainSizer->Add( NameSizer, 0, wxEXPAND, 5 );

	wxStaticBoxSizer * FiltersSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( "Filters" ) ), wxVERTICAL );

	wxBoxSizer * FiltersListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

    if( !FilterItemNames.Count() )
    {
        FilterItemNames.Add( wxT( "Dummy" ) );
        //FilterItemNames.Add( wxT( "Text" ) );
        FilterItemNames.Add( wxT( "Labels" ) );
        FilterItemNames.Add( _( "Genre" ) );
        FilterItemNames.Add( _( "Artist" ) );
        FilterItemNames.Add( _( "Composer" ) );
        FilterItemNames.Add( _( "Album Artist" ) );
        FilterItemNames.Add( _( "Album" ) );
        FilterItemNames.Add( _( "Year" ) );
        FilterItemNames.Add( _( "Rating" ) );
        FilterItemNames.Add( _( "Play Count" ) );
    }

    wxArrayString FilterItems = wxStringTokenize( filterentry.AfterFirst( wxT( ':' ) ), wxT( ':' ) );
    int Index;
    int Count;
    if( ( Count = FilterItems.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            long Value;
            FilterItems[ Index ].ToLong( &Value );
            m_FilterItems.Add( Value );
        }
    }

	m_FiltersListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	Count = m_FilterItems.Count();
	for( Index = 0; Index < Count; Index++ )
	{
	    m_FiltersListBox->Append( FilterItemNames[ m_FilterItems[ Index ] ] );
	}
	FiltersListBoxSizer->Add( m_FiltersListBox, 1, wxEXPAND|wxTOP|wxLEFT, 5 );

	wxBoxSizer * FilterButtonsSizer = new wxBoxSizer( wxVERTICAL );

	m_UpFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_UpFilterButton->Enable( false );

	FilterButtonsSizer->Add( m_UpFilterButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

	m_DownFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DownFilterButton->Enable( false );

	FilterButtonsSizer->Add( m_DownFilterButton, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DelFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelFilterButton->Enable( false );

	FilterButtonsSizer->Add( m_DelFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	FiltersListBoxSizer->Add( FilterButtonsSizer, 0, wxEXPAND, 5 );

	FiltersSizer->Add( FiltersListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer * NewFilterSizer = new wxBoxSizer( wxHORIZONTAL );

	NewFilterSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText * NewFilterLabel = new wxStaticText( this, wxID_ANY, _( "Filter:" ), wxDefaultPosition, wxDefaultSize, 0 );
	NewFilterLabel->Wrap( -1 );
	NewFilterSizer->Add( NewFilterLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxString m_FiltersChoiceChoices[] = { _( "Genres" ), _( "Artist" ), _( "Composer" ), _( "Album Artist" ), _( "Album" ), _( "Year" ), _( "Rating" ), _( "PlayCount" ) };
	int m_FiltersChoiceNChoices = sizeof( m_FiltersChoiceChoices ) / sizeof( wxString );
	m_FiltersChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FiltersChoiceNChoices, m_FiltersChoiceChoices, 0 );
	m_FiltersChoice->SetSelection( 0 );
	NewFilterSizer->Add( m_FiltersChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_AddFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	NewFilterSizer->Add( m_AddFilterButton, 0, wxTOP|wxRIGHT, 5 );

	FiltersSizer->Add( NewFilterSizer, 0, wxEXPAND, 5 );

	MainSizer->Add( FiltersSizer, 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

	wxStdDialogButtonSizer * ButtonsSizer = new wxStdDialogButtonSizer();
	m_AcceptButton = new wxButton( this, wxID_OK );
	m_AcceptButton->Enable( !m_NameTextCtrl->IsEmpty() && m_FiltersListBox->GetCount() );
	ButtonsSizer->AddButton( m_AcceptButton );
	wxButton * ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->SetAffirmativeButton( m_AcceptButton );
	ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	m_AcceptButton->SetDefault();

	// Connect Events
	m_NameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTreeViewFilterEditor::OnCheckAcceptButton ), NULL, this );
	m_FiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guTreeViewFilterEditor::OnFilterListBoxSelected ), NULL, this );
	m_UpFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnUpFilterBtnClick ), NULL, this );
	m_DownFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnDownFilterBtnClick ), NULL, this );
	m_DelFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnDelFilterBtnClick ), NULL, this );
	m_AddFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnAddFilterBtnClick ), NULL, this );

	m_NameTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
guTreeViewFilterEditor::~guTreeViewFilterEditor()
{
	m_NameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTreeViewFilterEditor::OnCheckAcceptButton ), NULL, this );
	m_FiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guTreeViewFilterEditor::OnFilterListBoxSelected ), NULL, this );
	m_UpFilterButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnUpFilterBtnClick ), NULL, this );
	m_DownFilterButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnDownFilterBtnClick ), NULL, this );
	m_DelFilterButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnDelFilterBtnClick ), NULL, this );
	m_AddFilterButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTreeViewFilterEditor::OnAddFilterBtnClick ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guTreeViewFilterEditor::OnFilterListBoxSelected( wxCommandEvent& event )
{
    m_CurrentItem = event.GetInt();
    if( m_CurrentItem != wxNOT_FOUND )
    {
        m_UpFilterButton->Enable( m_CurrentItem > 0 );
        m_DownFilterButton->Enable( m_CurrentItem < ( int ) ( m_FiltersListBox->GetCount() - 1 ) );
        m_DelFilterButton->Enable( true );
    }
    else
    {
        m_UpFilterButton->Enable( false );
        m_DownFilterButton->Enable( false );
        m_DelFilterButton->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewFilterEditor::OnUpFilterBtnClick( wxCommandEvent& event )
{
    wxString OldItemStr = m_FiltersListBox->GetString( m_CurrentItem );
    int      OldItemVal = m_FilterItems[ m_CurrentItem ];

    m_FiltersListBox->SetString( m_CurrentItem, m_FiltersListBox->GetString( m_CurrentItem - 1 ) );
    m_FilterItems[ m_CurrentItem ] = m_FilterItems[ m_CurrentItem - 1 ];
    m_CurrentItem--;
    m_FiltersListBox->SetString( m_CurrentItem, OldItemStr );
    m_FilterItems[ m_CurrentItem ] = OldItemVal;

    m_FiltersListBox->SetSelection( m_CurrentItem );
    event.SetInt( m_CurrentItem );
    OnFilterListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewFilterEditor::OnDownFilterBtnClick( wxCommandEvent& event )
{
    wxString OldItemStr = m_FiltersListBox->GetString( m_CurrentItem );
    int      OldItemVal = m_FilterItems[ m_CurrentItem ];

    m_FiltersListBox->SetString( m_CurrentItem, m_FiltersListBox->GetString( m_CurrentItem + 1 ) );
    m_FilterItems[ m_CurrentItem ] = m_FilterItems[ m_CurrentItem + 1 ];
    m_CurrentItem++;
    m_FiltersListBox->SetString( m_CurrentItem, OldItemStr );
    m_FilterItems[ m_CurrentItem ] = OldItemVal;

    m_FiltersListBox->SetSelection( m_CurrentItem );
    event.SetInt( m_CurrentItem );
    OnFilterListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewFilterEditor::OnDelFilterBtnClick( wxCommandEvent& event )
{
    m_FilterItems.RemoveAt( m_CurrentItem );
    m_FiltersListBox->Delete( m_CurrentItem );

    OnCheckAcceptButton( event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewFilterEditor::OnAddFilterBtnClick( wxCommandEvent& event )
{
    int SelectedItem = m_FiltersChoice->GetCurrentSelection() + 2;
    if( m_FilterItems.Index( SelectedItem ) == wxNOT_FOUND )
    {
        m_FilterItems.Add( SelectedItem );
        m_FiltersListBox->Append( FilterItemNames[ SelectedItem ] );

        OnCheckAcceptButton( event );
    }
}

// -------------------------------------------------------------------------------- //
wxString guTreeViewFilterEditor::GetTreeViewFilterEntry( void )
{
    wxString RetVal = m_NameTextCtrl->GetValue();
    int Index;
    int Count = m_FilterItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        RetVal += wxString::Format( wxT( ":%i" ), m_FilterItems[ Index ] );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guTreeViewFilterEditor::OnCheckAcceptButton( wxCommandEvent &event )
{
	m_AcceptButton->Enable( !m_NameTextCtrl->IsEmpty() && m_FiltersListBox->GetCount() );
}

// -------------------------------------------------------------------------------- //
