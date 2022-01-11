// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
#include "EditWithOptions.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guEditWithOptions::guEditWithOptions( wxWindow * parent, const wxString &title, const wxString &label, const wxString &defval, const wxArrayString &items ) :
    wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize( 370, 175 ), wxDEFAULT_DIALOG_STYLE )
{
    SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer * MainFrame;
    MainFrame = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer * TopFrame;
    TopFrame = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText * EditLabel = new wxStaticText( this, wxID_ANY, label + wxT( ":" ), wxDefaultPosition, wxDefaultSize, 0 );
    EditLabel->Wrap( -1 );
    TopFrame->Add( EditLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_EditComboBox = new wxComboBox( this, wxID_ANY, defval, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    m_EditComboBox->Append( items );
    TopFrame->Add( m_EditComboBox, 1, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    MainFrame->Add( TopFrame, 1, wxEXPAND, 5 );

    wxStdDialogButtonSizer * ButtonFrame = new wxStdDialogButtonSizer();
    wxButton * ButtonOK = new wxButton( this, wxID_OK );
    ButtonFrame->AddButton( ButtonOK );
    wxButton * ButtonCancel = new wxButton( this, wxID_CANCEL );
    ButtonFrame->AddButton( ButtonCancel );
    ButtonFrame->SetAffirmativeButton( ButtonOK );
    ButtonFrame->SetCancelButton( ButtonCancel );
    ButtonFrame->Realize();

    MainFrame->Add( ButtonFrame, 0, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    SetSizer( MainFrame );
    Layout();

    ButtonOK->SetDefault();
//  SetEscapeId( wxID_CANCEL );

    m_EditComboBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
guEditWithOptions::~guEditWithOptions()
{
}

// -------------------------------------------------------------------------------- //
wxString guEditWithOptions::GetData( void )
{
    int Selected = m_EditComboBox->GetCurrentSelection();
    if( Selected != wxNOT_FOUND )
        return m_EditComboBox->GetString( Selected );

    return m_EditComboBox->GetValue();
}

}

// -------------------------------------------------------------------------------- //
