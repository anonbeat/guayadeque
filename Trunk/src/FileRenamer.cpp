// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
//    http://www.gnu.org/copyleft/gpl.h"tml
//
// -------------------------------------------------------------------------------- //
#include "FileRenamer.h"

#include "Config.h"
#include "Images.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/uri.h>

// -------------------------------------------------------------------------------- //
guFileRenamer::guFileRenamer( wxWindow * parent, guDbLibrary * db, const wxArrayString &files )
{
    m_Db = db;
    m_Files = files;
    m_CurFile = wxNOT_FOUND;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( wxT( "PosX" ), -1, wxT( "FileRenamer" ) );
    WindowPos.y = Config->ReadNum( wxT( "PosY" ), -1, wxT( "FileRenamer" ) );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( wxT( "SizeWidth" ), 500, wxT( "FileRenamer" ) );
    WindowSize.y = Config->ReadNum( wxT( "SizeHeight" ), 320, wxT( "FileRenamer" ) );

    Create( parent, wxID_ANY, _( "Files Renaming" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * FilesSizer;
	FilesSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _(" Names Preview ") ), wxVERTICAL );

	m_FilesListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED|wxLB_MULTIPLE );
	m_FilesListBox->Append( files );
	FilesSizer->Add( m_FilesListBox, 1, wxEXPAND|wxALL, 5 );

	wxFlexGridSizer * EditSizer;
	EditSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	EditSizer->AddGrowableCol( 1 );
	EditSizer->SetFlexibleDirection( wxBOTH );
	EditSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * NameStaticText = new wxStaticText( this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	NameStaticText->Wrap( -1 );
	EditSizer->Add( NameStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_NameTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	EditSizer->Add( m_NameTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

	wxStaticText * PatternStaticText = new wxStaticText( this, wxID_ANY, _("Pattern:"), wxDefaultPosition, wxDefaultSize, 0 );
	PatternStaticText->Wrap( -1 );
	EditSizer->Add( PatternStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* PatternSizer;
	PatternSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PatTextCtrl = new wxTextCtrl( this, wxID_ANY, Config->ReadStr( wxT( "Pattern" ), wxT( "{n} - {a} -  {t}" ), wxT( "FileRenamer" ) ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PatTextCtrl->SetToolTip( _("Pattern flags:\n{g} \t- Genero\n{a} \t- Artist Name\n{b} \t- Album Name\n{t} \t- Track Title\n{n} \t- Track Number\n{y} \t- Track Year\n{d} \t- Disk\n") );

	PatternSizer->Add( m_PatTextCtrl, 1, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_PatApplyBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PatternSizer->Add( m_PatApplyBtn, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_PatRevertBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_reload ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PatternSizer->Add( m_PatRevertBtn, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	EditSizer->Add( PatternSizer, 1, wxEXPAND, 5 );

	FilesSizer->Add( EditSizer, 0, wxEXPAND, 5 );

	MainSizer->Add( FilesSizer, 1, wxEXPAND|wxALL, 5 );

	wxStdDialogButtonSizer * ButtonsSizer = new wxStdDialogButtonSizer();
	wxButton * ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	wxButton * ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	Connect( wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guFileRenamer::OnOKButton ) );

	m_FilesListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guFileRenamer::OnFileSelected ), NULL, this );
	m_PatTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guFileRenamer::OnPatternChanged ), NULL, this );
	m_PatApplyBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guFileRenamer::OnPatternApply ), NULL, this );
	m_PatRevertBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guFileRenamer::OnPattternRevert ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guFileRenamer::~guFileRenamer()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    // Save the window position and size
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( wxT( "PosX" ), WindowPos.x, wxT( "FileRenamer" ) );
    Config->WriteNum( wxT( "PosY" ), WindowPos.y, wxT( "FileRenamer" ) );
    wxSize WindowSize = GetSize();
    Config->WriteNum( wxT( "SizeWidth" ), WindowSize.x, wxT( "FileRenamer" ) );
    Config->WriteNum( wxT( "SizeHeight" ), WindowSize.y, wxT( "FileRenamer" ) );

	Config->WriteStr( wxT( "Pattern" ), m_PatTextCtrl->GetLineText( 0 ), wxT( "FileRenamer" ) );

	m_FilesListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guFileRenamer::OnFileSelected ), NULL, this );
	m_PatTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guFileRenamer::OnPatternChanged ), NULL, this );
	m_PatApplyBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guFileRenamer::OnPatternApply ), NULL, this );
	m_PatRevertBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guFileRenamer::OnPattternRevert ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guFileRenamer::OnOKButton( wxCommandEvent& event )
{
    if( m_CurFile != wxNOT_FOUND )
    {
        if( m_NameTextCtrl->IsModified() )
        {
            m_FilesListBox->SetString( m_CurFile, m_NameTextCtrl->GetLineText( 0 ) );
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guFileRenamer::OnFileSelected( wxCommandEvent& event )
{
    wxArrayInt Selection;
    int SelCount = m_FilesListBox->GetSelections( Selection );

    if( m_CurFile != wxNOT_FOUND )
    {
        if( m_NameTextCtrl->IsModified() )
        {
            m_FilesListBox->SetString( m_CurFile, m_NameTextCtrl->GetLineText( 0 ) );
        }
    }

    m_NameTextCtrl->Enable( SelCount == 1 );
    if( SelCount == 1 )
    {
        m_CurFile = Selection[ 0 ];
        m_NameTextCtrl->SetValue( m_FilesListBox->GetString( m_CurFile ) );
    }
    else
    {
        m_CurFile = wxNOT_FOUND;
        m_NameTextCtrl->SetValue( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guFileRenamer::OnPatternChanged( wxCommandEvent& event )
{
    bool Enabled = !m_PatTextCtrl->IsEmpty();
    m_PatApplyBtn->Enable( Enabled );
    m_PatRevertBtn->Enable( Enabled );
}

// -------------------------------------------------------------------------------- //
void guFileRenamer::OnPatternApply( wxCommandEvent& event )
{
    wxArrayInt Selection;
    wxArrayString RenameFiles;
    int Index;
    int Count = m_FilesListBox->GetSelections( Selection );
    if( Count )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            RenameFiles.Add( m_Files[ Selection[ Index ] ] );
        }
    }
    else
    {
        Count = m_Files.Count();
        for( Index = 0; Index < Count; Index++ )
            Selection.Add( Index );
        RenameFiles = m_Files;
    }

    if( ( Count = RenameFiles.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            wxString FileName = RenameFiles[ Index ];
            if( guIsValidAudioFile( FileName  ) )
            {
                wxURI Uri( FileName );

                if( Uri.IsReference() )
                {
                    guTrack * Track = new guTrack();
                    Track->m_FileName = FileName;

                    if( !m_Db->FindTrackFile( FileName, Track ) )
                    {
                        guPodcastItem PodcastItem;
                        if( m_Db->GetPodcastItemFile( FileName, &PodcastItem ) )
                        {
                            delete Track;
                            continue;
                        }
                        else
                        {
                            //guLogMessage( wxT( "Reading tags from the file..." ) );
                            if( Track->ReadFromFile( FileName ) )
                            {
                                Track->m_Type = guTRACK_TYPE_NOTDB;
                            }
                            else
                            {
                                guLogError( wxT( "Could not read tags from file '%s'" ), FileName.c_str() );
                                delete Track;
                                continue;
                            }
                        }
                    }

                    if( !m_PatTextCtrl->GetLineText( 0 ).StartsWith( wxT( "/" ) ) )
                    {
                        FileName = wxPathOnly( FileName ) + wxT( "/" ) + m_PatTextCtrl->GetLineText( 0 ) +
                                    wxT( '.' ) + FileName.AfterLast( wxT( '.' ) );
                    }
                    else
                    {
                        FileName = m_PatTextCtrl->GetLineText( 0 ) +
                                    wxT( '.' ) + FileName.AfterLast( wxT( '.' ) );
                    }

                    FileName = guExpandTrackMacros( FileName, Track );

                    m_FilesListBox->SetString( Selection[ Index ], FileName );

                    delete Track;
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileRenamer::OnPattternRevert( wxCommandEvent& event )
{
    m_FilesListBox->Clear();
    m_FilesListBox->Append( m_Files );
}

// -------------------------------------------------------------------------------- //
wxArrayString guFileRenamer::GetRenamedNames( void )
{
    wxArrayString RetVal;
    int Index;
    int Count = m_FilesListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        RetVal.Add( m_FilesListBox->GetString( Index ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //

