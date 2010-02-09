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
#include "ConfirmExit.h"
#include "Images.h"

// -------------------------------------------------------------------------------- //
guExitConfirmDlg::guExitConfirmDlg( wxWindow * parent ) :
  wxDialog( parent, wxID_ANY, _("Please confirm"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE )
{
	wxBoxSizer* MainSizer;
	wxBoxSizer* TopSizer;
    wxStaticBitmap * ExitBitmap;
    wxStaticText * MessageString;
    wxStdDialogButtonSizer * ButtonsSizer;
    wxButton * ButtonsSizerOK;
    wxButton * ButtonsSizerCancel;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainSizer = new wxBoxSizer( wxVERTICAL );

	TopSizer = new wxBoxSizer( wxHORIZONTAL );

	ExitBitmap = new wxStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_exit ), wxDefaultPosition, wxDefaultSize, 0 );
	TopSizer->Add( ExitBitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	MessageString = new wxStaticText( this, wxID_ANY, _("Are you sure you want to exit the application?"), wxDefaultPosition, wxDefaultSize, 0 );
	MessageString->Wrap( -1 );
	TopSizer->Add( MessageString, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	MainSizer->Add( TopSizer, 1, wxEXPAND, 5 );

	m_AskAgainCheckBox = new wxCheckBox( this, wxID_ANY, _("Don't ask again"), wxDefaultPosition, wxDefaultSize, 0 );

	MainSizer->Add( m_AskAgainCheckBox, 0, wxALL, 5 );

	ButtonsSizer = new wxStdDialogButtonSizer();
	ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND | wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();
	MainSizer->Fit( this );
}

// -------------------------------------------------------------------------------- //
guExitConfirmDlg::~guExitConfirmDlg()
{
}

// -------------------------------------------------------------------------------- //
bool guExitConfirmDlg::GetConfirmChecked( void )
{
    return m_AskAgainCheckBox->IsChecked();
}

// -------------------------------------------------------------------------------- //
