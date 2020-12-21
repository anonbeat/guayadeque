// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#include "LabelEditor.h"

#include "Config.h"
#include "Images.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guLabelEditor::guLabelEditor( wxWindow * parent, guDbLibrary * db, const wxString &title,
        const bool isradiolabel, const guListItems * items, guArrayListItems * labelsets ) //wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize( 500,300 ), wxDEFAULT_DIALOG_STYLE )
{
    m_SelectedItem = wxNOT_FOUND;
    m_SelectedLabel = wxNOT_FOUND;
    m_IsRadioLabel = isradiolabel;
    if( isradiolabel )
    {
        m_Db = NULL;
        m_RaDb = ( guDbRadios * ) db;
    }
    else
    {
        m_Db = db;
        m_RaDb = NULL;
    }
    m_LabelSets = labelsets;


    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( CONFIG_KEY_POSITIONS_LABELEDIT_POSX, -1, CONFIG_PATH_POSITIONS );
    WindowPos.y = Config->ReadNum( CONFIG_KEY_POSITIONS_LABELEDIT_POSY, -1, CONFIG_PATH_POSITIONS );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( CONFIG_KEY_POSITIONS_LABELEDIT_WIDTH, 500, CONFIG_PATH_POSITIONS );
    WindowSize.y = Config->ReadNum( CONFIG_KEY_POSITIONS_LABELEDIT_HEIGHT, 300, CONFIG_PATH_POSITIONS );

    //wxDialog( parent, wxID_ANY, _( "Songs Editor" ), wxDefaultPosition, wxSize( 625, 440 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
    Create( parent, wxID_ANY, title, WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );


	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_Splitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_Splitter->SetMinimumPaneSize( 50 );
	m_ItemsPanel = new wxPanel( m_Splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* ItemsMainSizer;
	ItemsMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* ItemsStaticBox;
	ItemsStaticBox = new wxStaticBoxSizer( new wxStaticBox( m_ItemsPanel, wxID_ANY, _(" Items ") ), wxVERTICAL );

	m_ItemsListBox = new wxListBox( m_ItemsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	int Index;
	int Count = items->Count();
	for( Index = 0; Index < Count; Index++ )
    {
        m_ItemsListBox->Append( items->Item( Index ).m_Name );
    }
	ItemsStaticBox->Add( m_ItemsListBox, 1, wxEXPAND|wxALL, 5 );

	ItemsMainSizer->Add( ItemsStaticBox, 1, wxEXPAND|wxALL, 5 );

	m_ItemsPanel->SetSizer( ItemsMainSizer );
	m_ItemsPanel->Layout();
	ItemsMainSizer->Fit( m_ItemsPanel );
	m_LabelsPanel = new wxPanel( m_Splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* LabelsMainSizer;
	LabelsMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* LabelsStaticBox;
	LabelsStaticBox = new wxStaticBoxSizer( new wxStaticBox( m_LabelsPanel, wxID_ANY, _(" Labels ") ), wxHORIZONTAL );

	if( m_IsRadioLabel )
	{
	    m_RaDb->GetRadioLabels( &m_Labels );
	}
	else
	{
        m_Db->GetLabels( &m_Labels );
	}

	m_LabelsListBox = new wxCheckListBox( m_LabelsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	Count = m_Labels.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_LabelsListBox->Append( m_Labels[ Index ].m_Name );
    }
	LabelsStaticBox->Add( m_LabelsListBox, 1, wxEXPAND|wxALL, 5 );

	LabelsMainSizer->Add( LabelsStaticBox, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer* ButtonsSizer;
	ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );


	ButtonsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_AddButton = new wxBitmapButton( m_LabelsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_AddButton->SetToolTip( _("Add a new label") );

	ButtonsSizer->Add( m_AddButton, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DelButton = new wxBitmapButton( m_LabelsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelButton->Enable( false );
	m_DelButton->SetToolTip( _("Delete the current selected label") );


	ButtonsSizer->Add( m_DelButton, 0, wxBOTTOM|wxRIGHT, 5 );

	m_CopyButton = new wxBitmapButton( m_LabelsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CopyButton->SetToolTip( _("Copy the label selection to all the items") );

	ButtonsSizer->Add( m_CopyButton, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	LabelsMainSizer->Add( ButtonsSizer, 0, wxEXPAND, 5 );

	m_LabelsPanel->SetSizer( LabelsMainSizer );
	m_LabelsPanel->Layout();
	LabelsMainSizer->Fit( m_LabelsPanel );
	m_Splitter->SplitVertically( m_ItemsPanel, m_LabelsPanel, 177 );
	MainSizer->Add( m_Splitter, 1, wxEXPAND, 5 );

	wxStdDialogButtonSizer * DialogButtons = new wxStdDialogButtonSizer();
	wxButton * OkButton = new wxButton( this, wxID_OK );
	DialogButtons->AddButton( OkButton );
	wxButton * CancelButton = new wxButton( this, wxID_CANCEL );
	DialogButtons->AddButton( CancelButton );
	DialogButtons->SetAffirmativeButton( OkButton );
	DialogButtons->SetCancelButton( CancelButton );
	DialogButtons->Realize();
	MainSizer->Add( DialogButtons, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	OkButton->SetDefault();

    // Bind Events
    m_Splitter->Bind( wxEVT_IDLE, &guLabelEditor::OnIdle, this );

    m_ItemsListBox->Bind( wxEVT_LISTBOX, &guLabelEditor::OnItemSelected, this );
    m_LabelsListBox->Bind( wxEVT_LISTBOX, &guLabelEditor::OnLabelSelected, this );
    m_LabelsListBox->Bind( wxEVT_CHECKLISTBOX, &guLabelEditor::OnLabelChecked, this );
    m_LabelsListBox->Bind( wxEVT_LISTBOX_DCLICK, &guLabelEditor::OnLabelDoubleClicked, this );
    m_AddButton->Bind( wxEVT_BUTTON, &guLabelEditor::OnAddLabelClicked, this );
    m_DelButton->Bind( wxEVT_BUTTON, &guLabelEditor::OnDelLabelClicked, this );
    m_CopyButton->Bind( wxEVT_BUTTON, &guLabelEditor::OnCopyLabelsClicked, this );

    m_ItemsListBox->SetSelection( 0 );
    wxCommandEvent event;
    event.SetInt( 0 );
    OnItemSelected( event );

    m_ItemsListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
guLabelEditor::~guLabelEditor()
{
    // Save the window position and size
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( CONFIG_KEY_POSITIONS_LABELEDIT_SASHPOS, m_Splitter->GetSashPosition(), CONFIG_PATH_POSITIONS );
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( CONFIG_KEY_POSITIONS_LABELEDIT_POSX, WindowPos.x, CONFIG_PATH_POSITIONS );
    Config->WriteNum( CONFIG_KEY_POSITIONS_LABELEDIT_POSY, WindowPos.y, CONFIG_PATH_POSITIONS );
    wxSize WindowSize = GetSize();
    Config->WriteNum( CONFIG_KEY_POSITIONS_LABELEDIT_WIDTH, WindowSize.x, CONFIG_PATH_POSITIONS );
    Config->WriteNum( CONFIG_KEY_POSITIONS_LABELEDIT_HEIGHT, WindowSize.y, CONFIG_PATH_POSITIONS );

    // Unbind Events
    m_ItemsListBox->Unbind( wxEVT_LISTBOX, &guLabelEditor::OnItemSelected, this );
    m_LabelsListBox->Unbind( wxEVT_LISTBOX, &guLabelEditor::OnLabelSelected, this );
    m_LabelsListBox->Unbind( wxEVT_CHECKLISTBOX, &guLabelEditor::OnLabelChecked, this );
    m_LabelsListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guLabelEditor::OnLabelDoubleClicked, this );
    m_AddButton->Unbind( wxEVT_BUTTON, &guLabelEditor::OnAddLabelClicked, this );
    m_DelButton->Unbind( wxEVT_BUTTON, &guLabelEditor::OnDelLabelClicked, this );
    m_CopyButton->Unbind( wxEVT_BUTTON, &guLabelEditor::OnCopyLabelsClicked, this );
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::ClearCheckedItems( void )
{
    int Index;
    int Count = m_Labels.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_LabelsListBox->Check( Index, false );
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::CheckLabelItems( const wxArrayInt &checkeditems )
{
    if( checkeditems.Count() )
    {
        int Index;
        int Count = m_Labels.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            if( checkeditems.Index( m_Labels[ Index ].m_Id ) != wxNOT_FOUND )
            {
                m_LabelsListBox->Check( Index, true );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnItemSelected( wxCommandEvent &event )
{
    m_SelectedItem = event.GetInt();
    ClearCheckedItems();
    if( m_SelectedItem >= 0 )
    {
        CheckLabelItems( m_LabelSets->Item( m_SelectedItem ).GetData() );
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnLabelSelected( wxCommandEvent &event )
{
    m_SelectedLabel = event.GetInt();
    m_DelButton->Enable( m_SelectedLabel != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnLabelChecked( wxCommandEvent &event )
{
    m_SelectedLabel = event.GetInt();
    m_DelButton->Enable( m_SelectedLabel != wxNOT_FOUND );

    if( m_SelectedItem == wxNOT_FOUND )
        return;

    if( m_LabelsListBox->IsChecked( m_SelectedLabel ) )
    {
        m_LabelSets->Item( m_SelectedItem ).AddData( m_Labels[ m_SelectedLabel ].m_Id );
    }
    else
    {
        m_LabelSets->Item( m_SelectedItem ).DelData( m_Labels[ m_SelectedLabel ].m_Id );
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnLabelDoubleClicked( wxCommandEvent &event )
{
    int LabelIndex = event.GetInt();
    if( LabelIndex != wxNOT_FOUND )
    {
        bool Enabled = m_LabelsListBox->IsChecked( event.GetInt() );
        int LabelId = m_Labels[ LabelIndex ].m_Id;
        if( Enabled )
        {
            AddToAllItems( LabelId );
        }
        else
        {
            DelToAllItems( LabelId );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnAddLabelClicked( wxCommandEvent &event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ), _( "Please enter the label name" ) );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        int AddedId;
        if( m_IsRadioLabel )
        {
            AddedId = m_RaDb->AddRadioLabel( EntryDialog->GetValue() );
        }
        else
        {
            AddedId = m_Db->AddLabel( EntryDialog->GetValue() );
        }
        m_LabelsListBox->Append( EntryDialog->GetValue() );
        m_Labels.Add( new guListItem( AddedId, EntryDialog->GetValue() ) );
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnDelLabelClicked( wxCommandEvent &event )
{
    if( m_SelectedLabel != wxNOT_FOUND )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected labels?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT, this ) == wxYES )
        {
            int LabelId = m_Labels[ m_SelectedLabel ].m_Id;
            if( m_IsRadioLabel )
            {
                m_RaDb->DelRadioLabel( LabelId );
            }
            else
            {
                m_Db->DelLabel( LabelId );
            }

            m_LabelsListBox->Delete( m_SelectedLabel );

            // Delete the label id from the items enabled labels
            DelToAllItems( LabelId );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnCopyLabelsClicked( wxCommandEvent& event )
{
    int Index;
    int Count = m_Labels.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_LabelsListBox->IsChecked( Index ) )
        {
            AddToAllItems( m_Labels[ Index ].m_Id );
        }
        else
        {
            DelToAllItems( m_Labels[ Index ].m_Id );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnIdle( wxIdleEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_Splitter->SetSashPosition( Config->ReadNum( CONFIG_KEY_POSITIONS_LABELEDIT_SASHPOS, 177, CONFIG_PATH_POSITIONS ) );
    m_Splitter->Unbind( wxEVT_IDLE, &guLabelEditor::OnIdle, this );
}

// -------------------------------------------------------------------------------- //
void  guLabelEditor::AddToAllItems( const int labelid )
{
    int Index;
    int Count = m_LabelSets->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_LabelSets->Item( Index ).Index( labelid ) == wxNOT_FOUND )
            m_LabelSets->Item( Index ).AddData( labelid );
    }
}

// -------------------------------------------------------------------------------- //
void  guLabelEditor::DelToAllItems( const int labelid )
{
    int Index;
    int Count = m_LabelSets->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_LabelSets->Item( Index ).DelData( labelid );
    }
}

}

// -------------------------------------------------------------------------------- //
