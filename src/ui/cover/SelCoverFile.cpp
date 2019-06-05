// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#include "SelCoverFile.h"

#include "Config.h"
#include "EventCommandIds.h"
#include "MediaViewer.h"

#include <wx/filedlg.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guSelCoverFile::guSelCoverFile( wxWindow * parent, guMediaViewer * mediaviewer, const int albumid ) :
    wxDialog( parent, wxID_ANY, _( "Select Cover File" ), wxDefaultPosition, wxSize( 400, 132 ), wxDEFAULT_DIALOG_STYLE )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer->GetDb();
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
    m_EmbedToFilesChkBox->SetValue( Config->ReadBool( CONFIG_KEY_GENERAL_EMBED_TO_FILES, false, CONFIG_PATH_GENERAL ) );
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

    m_SelFileBtn->Bind( wxEVT_BUTTON, &guSelCoverFile::OnSelFileClicked, this );
    m_FileLink->Bind( wxEVT_TEXT, &guSelCoverFile::OnPathChanged, this );

    Bind( wxEVT_MENU, &guSelCoverFile::OnCoverFinish, this, ID_SELCOVERDIALOG_FINISH );

	m_FileLink->SetFocus();
}

// -------------------------------------------------------------------------------- //
guSelCoverFile::~guSelCoverFile()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( CONFIG_KEY_GENERAL_EMBED_TO_FILES, m_EmbedToFilesChkBox->GetValue(), CONFIG_PATH_GENERAL );

    m_SelFileBtn->Unbind( wxEVT_BUTTON, &guSelCoverFile::OnSelFileClicked, this );
    m_FileLink->Unbind( wxEVT_TEXT, &guSelCoverFile::OnPathChanged, this );

    Unbind( wxEVT_MENU, &guSelCoverFile::OnCoverFinish, this, ID_SELCOVERDIALOG_FINISH );
}

// -------------------------------------------------------------------------------- //
void guSelCoverFile::OnSelFileClicked( wxCommandEvent& event )
{
    wxString CoverName = m_MediaViewer->CoverName();
    if( CoverName.IsEmpty() )
    {
        CoverName = wxT( "cover" );
    }
    CoverName += wxT( ".jpg " );

    wxFileDialog * FileDialog = new wxFileDialog( this, _( "Select the cover filename" ),
        m_AlbumPath, CoverName, wxT( "*.jpg;*.JPG;*.jpeg;*.JPEG;*.png;*,PNG" ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_PREVIEW );

    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            m_FileLink->SetValue( FileDialog->GetPath() );

            wxCommandEvent event( wxEVT_MENU, ID_SELCOVERDIALOG_FINISH );
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
    m_StdBtnOk->Enable( !event.GetString().IsEmpty() );
}

}

// -------------------------------------------------------------------------------- //
