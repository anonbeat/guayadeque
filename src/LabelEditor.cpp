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
#include "LabelEditor.h"

// -------------------------------------------------------------------------------- //
guLabelEditor::guLabelEditor( wxWindow* parent, const wxString &Title,
                     const guListItems &listitems, const guArrayListItems &selitems ) :
             wxDialog( parent, wxID_ANY, Title, wxDefaultPosition, wxSize( 250,300 ), wxDEFAULT_DIALOG_STYLE )
{
	wxBoxSizer *    LabelEditorSizer;
	wxBoxSizer *    CheckListBoxSizer;
	wxBoxSizer *    OptionsSizer;
	wxArrayString   Choices;
	wxArrayInt      EnabledIds;
	int index;
	int count;

	SetSizeHints( wxDefaultSize, wxDefaultSize );

	LabelEditorSizer = new wxBoxSizer( wxVERTICAL );


	LabelEditorSizer->Add( 0, 15, 0, wxEXPAND, 5 );

	m_LabelsStaticText = new wxStaticText( this, wxID_ANY, _( "Labels" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_LabelsStaticText->Wrap( -1 );
	LabelEditorSizer->Add( m_LabelsStaticText, 0, wxLEFT|wxRIGHT|wxTOP, 5 );

	CheckListBoxSizer = new wxBoxSizer( wxVERTICAL );

    count = listitems.Count();
    for( index = 0; index < count; index++ )
    {
        Choices.Add( listitems[ index ].m_Name );
        m_LabelIds.Add( listitems[ index ].m_Id );
    }

	m_CheckListBox = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, Choices, wxLB_MULTIPLE|wxLB_NEEDED_SB );
	CheckListBoxSizer->Add( m_CheckListBox, 1, wxALL|wxEXPAND, 5 );

	LabelEditorSizer->Add( CheckListBoxSizer, 1, wxEXPAND, 5 );

	OptionsSizer = new wxBoxSizer( wxHORIZONTAL );

	OptionsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_LabelEditorBtnSizer = new wxStdDialogButtonSizer();
	m_LabelEditorBtnSizerOK = new wxButton( this, wxID_OK );
	m_LabelEditorBtnSizer->AddButton( m_LabelEditorBtnSizerOK );
	m_LabelEditorBtnSizerCancel = new wxButton( this, wxID_CANCEL );
	m_LabelEditorBtnSizer->AddButton( m_LabelEditorBtnSizerCancel );
	m_LabelEditorBtnSizer->Realize();
	LabelEditorSizer->Add( m_LabelEditorBtnSizer, 0, wxEXPAND, 5 );

	SetSizer( LabelEditorSizer );
	Layout();

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
}

// -------------------------------------------------------------------------------- //
guLabelEditor::~guLabelEditor()
{
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
