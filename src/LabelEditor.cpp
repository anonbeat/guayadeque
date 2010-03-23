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
#include "LabelEditor.h"
#include "Images.h"

// -------------------------------------------------------------------------------- //
guLabelEditor::guLabelEditor( wxWindow* parent, guDbLibrary * db, const wxString &Title,
        const bool isradiolabel, const guListItems &listitems, const guArrayListItems &selitems ) :
             wxDialog( parent, wxID_ANY, Title, wxDefaultPosition, wxSize( 250,300 ), wxDEFAULT_DIALOG_STYLE )
{
    m_Db = db;
    m_SelectedItem = wxNOT_FOUND;
    m_IsRadioLabel = isradiolabel;

	wxArrayString   Choices;
	wxArrayInt      EnabledIds;
	int index;
	int count;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* TopSizer;
	TopSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * LabelStaticText = new wxStaticText( this, wxID_ANY, _( "Labels" ), wxDefaultPosition, wxDefaultSize, 0 );
	LabelStaticText->Wrap( -1 );
	TopSizer->Add( LabelStaticText, 1, wxALIGN_BOTTOM|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_AddLabelBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_AddLabelBtn->SetToolTip( _( "Add a new label" ) );

	TopSizer->Add( m_AddLabelBtn, 0, wxALL, 5 );

	m_DelLabelBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelLabelBtn->SetToolTip( _( "Delete the selected labels" ) );
	m_DelLabelBtn->Enable( false );

	TopSizer->Add( m_DelLabelBtn, 0, wxALL, 5 );

	MainSizer->Add( TopSizer, 0, wxEXPAND, 5 );

    count = listitems.Count();
    for( index = 0; index < count; index++ )
    {
        Choices.Add( listitems[ index ].m_Name );
        m_LabelIds.Add( listitems[ index ].m_Id );
    }

	m_CheckListBox = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, Choices, wxLB_MULTIPLE|wxLB_NEEDED_SB );
	MainSizer->Add( m_CheckListBox, 1, wxALL|wxEXPAND, 5 );

    wxStdDialogButtonSizer * LabelEditorBtnSizer;
    wxButton * LabelEditorBtnSizerOK;
    wxButton * LabelEditorBtnSizerCancel;

	LabelEditorBtnSizer = new wxStdDialogButtonSizer();
	LabelEditorBtnSizerOK = new wxButton( this, wxID_OK );
	LabelEditorBtnSizer->AddButton( LabelEditorBtnSizerOK );
	LabelEditorBtnSizerCancel = new wxButton( this, wxID_CANCEL );
	LabelEditorBtnSizer->AddButton( LabelEditorBtnSizerCancel );
	LabelEditorBtnSizer->Realize();
	MainSizer->Add( LabelEditorBtnSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    //
    count = selitems.Count();
    EnabledIds = m_LabelIds;
    for( index = 0; index < count; index++ )
    {
        EnabledIds = GetArraySameItems( EnabledIds, selitems[ index ].GetData() );
        if( !EnabledIds.Count() )
            break;
    }

    if( ( count = EnabledIds.Count() ) )
    {
        wxArrayInt EnabledIndexs;
        for( index = 0; index < count; index++ )
        {
            EnabledIndexs.Add( guListItemSearch( listitems, 0, listitems.Count() - 1, EnabledIds[ index ] ) );
        }
        SetCheckedItems( EnabledIndexs );
    }

	m_CheckListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guLabelEditor::OnCheckListBoxSelected ), NULL, this );
	m_AddLabelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLabelEditor::OnAddLabelBtnClick ), NULL, this );
	m_DelLabelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLabelEditor::OnDelLabelBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guLabelEditor::~guLabelEditor()
{
	m_CheckListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guLabelEditor::OnCheckListBoxSelected ), NULL, this );
	m_AddLabelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLabelEditor::OnAddLabelBtnClick ), NULL, this );
	m_DelLabelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLabelEditor::OnDelLabelBtnClick ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::SetCheckedItems( const wxArrayInt &Checked )
{
    int index;
    int count = Checked.GetCount();
    for( index = 0; index < count; index++ )
    {
        m_CheckListBox->Check( Checked[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
wxArrayInt guLabelEditor::GetCheckedIds( void )
{
    wxArrayInt RetVal;
    int index;
    int count = m_CheckListBox->GetCount();
    for( index = 0; index < count; index++ )
    {
        if( m_CheckListBox->IsChecked( index ) )
            RetVal.Add( m_LabelIds[ index ] );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnAddLabelBtnClick( wxCommandEvent &event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ), _( "Please enter the label name" ) );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        int AddedId;
        if( m_IsRadioLabel )
        {
            AddedId = m_Db->AddRadioLabel( EntryDialog->GetValue() );
        }
        else
        {
            AddedId = m_Db->AddLabel( EntryDialog->GetValue() );
        }
        m_CheckListBox->Append( EntryDialog->GetValue() );
        m_LabelIds.Add( AddedId );
    }
    EntryDialog->Destroy();

}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnDelLabelBtnClick( wxCommandEvent &event )
{
    if( m_SelectedItem != wxNOT_FOUND )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected labels?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            if( m_IsRadioLabel )
            {
                m_Db->DelRadioLabel( m_LabelIds[ m_SelectedItem ] );
            }
            else
            {
                m_Db->DelLabel( m_LabelIds[ m_SelectedItem ] );
            }
            m_CheckListBox->Delete( m_SelectedItem );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLabelEditor::OnCheckListBoxSelected( wxCommandEvent& event )
{
    m_SelectedItem = event.GetInt();
    if( m_SelectedItem != wxNOT_FOUND )
    {
        m_DelLabelBtn->Enable();
    }
    else
    {
        m_DelLabelBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
