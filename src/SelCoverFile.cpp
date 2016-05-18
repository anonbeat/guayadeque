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
#include "SelCoverFile.h"

#include "Config.h"
#include "Commands.h"

#include <wx/filedlg.h>

// -------------------------------------------------------------------------------- //
guSelCoverFile::guSelCoverFile( wxWindow * parent, guDbLibrary * db, const int albumid ) :
    wxDialog( parent, wxID_ANY, _( "Select Cover File" ), wxDefaultPosition, wxSize( 400, 132 ), wxDEFAULT_DIALOG_STYLE )
{
    m_Db = db;
    if( albumid != wxNOT_FOUND )
    {
        wxString AlbumName;
        wxString ArtistName;
        if( !m_Db->GetAlbumInfo( albumid, &AlbumName, &ArtistName, &m_AlbumPath ) )
        {
            wxMessageBox( _( "Could not find the Album in the songs library.\n"\
                             "You should update the library." ), _( "Error" ), wxICON_ERROR | wxOK );
        }
    }

    guConfig * Config = ( guConfig * ) guConfig::Get();

    // GUI
	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * ControlsSizer;
	ControlsSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * CoverLabel = new wxStaticText( this, wxID_ANY, _( "Cover:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CoverLabel->Wrap( -1 );
	ControlsSizer->Add( CoverLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_FileLink = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	ControlsSizer->Add( m_FileLink, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );

	m_SelFileBtn = new wxButton( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 25,-1 ), 0 );
	ControlsSizer->Add( m_SelFileBtn, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MainSizer->Add( ControlsSizer, 1, wxEXPAND, 5 );

	m_EmbedToFilesChkBox = new wxCheckBox( this, wxID_ANY, _( "Embed into tracks" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_EmbedToFilesChkBox->SetValue( Config->ReadBool( wxT( "EmbedToFiles" ), false, wxT( "general" ) ) );
	MainSizer->Add( m_EmbedToFilesChkBox, 0, wxRIGHT|wxLEFT, 5 );

	wxStdDialogButtonSizer * StdBtnSizer = new wxStdDialogButtonSizer();
	m_StdBtnOk = new wxButton( this, wxID_OK );
	m_StdBtnOk->Enable( false );
	StdBtnSizer->AddButton( m_StdBtnOk );
	wxButton * StdBtnSizerCancel = new wxButton( this, wxID_CANCEL );
	StdBtnSizer->AddButton( StdBtnSizerCancel );
	StdBtnSizer->SetAffirmativeButton( m_StdBtnOk );
	StdBtnSizer->SetCancelButton( StdBtnSizerCancel );
	StdBtnSizer->Realize();
	MainSizer->Add( StdBtnSizer, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	m_StdBtnOk->SetDefault();

	m_SelFileBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guSelCoverFile::OnSelFileClicked ), NULL, this );
	m_FileLink->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guSelCoverFile::OnPathChanged ), NULL, this );

	Connect( ID_SELCOVERDIALOG_FINISH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSelCoverFile::OnCoverFinish ), NULL, this );

	m_FileLink->SetFocus();
}

// -------------------------------------------------------------------------------- //
guSelCoverFile::~guSelCoverFile()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( wxT( "EmbedToFiles" ), m_EmbedToFilesChkBox->GetValue(), wxT( "general" ) );

	m_SelFileBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guSelCoverFile::OnSelFileClicked ), NULL, this );
	m_FileLink->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guSelCoverFile::OnPathChanged ), NULL, this );

	Disconnect( ID_SELCOVERDIALOG_FINISH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSelCoverFile::OnCoverFinish ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guSelCoverFile::OnSelFileClicked( wxCommandEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "coversearch" ) );
    wxString CoverName = ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );

    wxFileDialog * FileDialog = new wxFileDialog( this, _( "Select the cover filename" ),
        m_AlbumPath, CoverName, wxT( "*.jpg;*.JPG;*.jpeg;*.JPEG;*.png;*,PNG" ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_PREVIEW );

    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            m_FileLink->SetValue( FileDialog->GetPath() );

            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_SELCOVERDIALOG_FINISH );
            AddPendingEvent( event );
        }
        FileDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guSelCoverFile::OnCoverFinish( wxCommandEvent& event )
{
    EndModal( wxID_OK );
}

// -------------------------------------------------------------------------------- //
wxString guSelCoverFile::GetSelFile( void )
{
    return m_FileLink->GetValue();
}

// -------------------------------------------------------------------------------- //
void guSelCoverFile::OnPathChanged( wxCommandEvent &event )
{
	m_StdBtnOk->Enable( !m_FileLink->IsEmpty() );
}

// -------------------------------------------------------------------------------- //
