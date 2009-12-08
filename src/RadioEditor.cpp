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
#include "RadioEditor.h"

// -------------------------------------------------------------------------------- //
guRadioEditor::guRadioEditor( wxWindow* parent, const wxString& title, const wxString &name, const wxString &link ) :
    wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize( 391,135 ), wxDEFAULT_DIALOG_STYLE )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	wxStaticText * NameLabel;
	wxStaticText * LinkLabel;
	wxStdDialogButtonSizer * StdBtnSizer;
	wxButton* StdBtnSizerOK;
	wxButton* StdBtnSizerCancel;

	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer * FlexGridSizer;
	FlexGridSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	FlexGridSizer->AddGrowableCol( 1 );
	FlexGridSizer->SetFlexibleDirection( wxBOTH );
	FlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	NameLabel = new wxStaticText( this, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
	NameLabel->Wrap( -1 );
	FlexGridSizer->Add( NameLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_NameTextCtrl = new wxTextCtrl( this, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, 0 );
	FlexGridSizer->Add( m_NameTextCtrl, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	LinkLabel = new wxStaticText( this, wxID_ANY, _( "Link:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LinkLabel->Wrap( -1 );
	FlexGridSizer->Add( LinkLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_LinkTextCtrl = new wxTextCtrl( this, wxID_ANY, link, wxDefaultPosition, wxDefaultSize, 0 );
	FlexGridSizer->Add( m_LinkTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MainSizer->Add( FlexGridSizer, 1, wxEXPAND, 5 );

	StdBtnSizer = new wxStdDialogButtonSizer();
	StdBtnSizerOK = new wxButton( this, wxID_OK );
	StdBtnSizer->AddButton( StdBtnSizerOK );
	StdBtnSizerCancel = new wxButton( this, wxID_CANCEL );
	StdBtnSizer->AddButton( StdBtnSizerCancel );
	StdBtnSizer->Realize();
	MainSizer->Add( StdBtnSizer, 0, wxEXPAND|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();
}

// -------------------------------------------------------------------------------- //
guRadioEditor::~guRadioEditor()
{
}

// -------------------------------------------------------------------------------- //
