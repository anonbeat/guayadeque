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
#include "ImportFiles.h"

#include "Config.h"
#include "Images.h"
#include "MediaViewer.h"
#include "TagInfo.h"

#include <wx/filedlg.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guImportFiles::guImportFiles( wxWindow * parent, guMediaViewer * mediaviewer, guTrackArray * tracks )
//    wxDialog( parent, wxID_ANY, _( "Import Files" ), wxDefaultPosition, wxSize( 480,340 ), wxDEFAULT_DIALOG_STYLE )
{
    m_MediaViewer = mediaviewer;
    m_Tracks = tracks;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( CONFIG_KEY_IMPORT_FILES_POS_X, -1, CONFIG_PATH_IMPORT_FILES_POSITION );
    WindowPos.y = Config->ReadNum( CONFIG_KEY_IMPORT_FILES_POS_Y, -1, CONFIG_PATH_IMPORT_FILES_POSITION );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( CONFIG_KEY_IMPORT_FILES_WIDTH, 480, CONFIG_PATH_IMPORT_FILES_POSITION );
    WindowSize.y = Config->ReadNum( CONFIG_KEY_IMPORT_FILES_HEIGHT, 340, CONFIG_PATH_IMPORT_FILES_POSITION );

    Create( parent, wxID_ANY, _( "Import Files" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

    CreateControls();
}

// -------------------------------------------------------------------------------- //
guImportFiles::~guImportFiles()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    wxPoint WindowPos = GetPosition();
    Config->WriteNum( CONFIG_KEY_IMPORT_FILES_POS_X, WindowPos.x, CONFIG_PATH_IMPORT_FILES_POSITION );
    Config->WriteNum( CONFIG_KEY_IMPORT_FILES_POS_Y, WindowPos.y, CONFIG_PATH_IMPORT_FILES_POSITION );
    wxSize WindowSize = GetSize();
    Config->WriteNum( CONFIG_KEY_IMPORT_FILES_WIDTH, WindowSize.x, CONFIG_PATH_IMPORT_FILES_POSITION );
    Config->WriteNum( CONFIG_KEY_IMPORT_FILES_HEIGHT, WindowSize.y, CONFIG_PATH_IMPORT_FILES_POSITION );

    m_CopyToSetupBtn->Unbind( wxEVT_BUTTON, &guImportFiles::OnCopyToSetupClicked, this );
    m_FilesListBox->Unbind( wxEVT_LISTBOX, &guImportFiles::OnFileSelected, this );
    m_AddFilesBtn->Unbind( wxEVT_BUTTON, &guImportFiles::OnAddFilesClicked, this );
    m_DelFilesBtn->Unbind( wxEVT_BUTTON, &guImportFiles::OnDelFilesClicked, this );

    Unbind( guConfigUpdatedEvent, &guImportFiles::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guImportFiles::CreateControls( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * MainStaticBoxSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxVERTICAL );

    wxFlexGridSizer * CopyToSizer = new wxFlexGridSizer( 2, 0, 0 );
	CopyToSizer->AddGrowableCol( 1 );
	CopyToSizer->SetFlexibleDirection( wxBOTH );
	CopyToSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * CopyToStaticText = new wxStaticText( this, wxID_ANY, _( "Copy to Option:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToStaticText->Wrap( -1 );
	CopyToSizer->Add( CopyToStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    wxBoxSizer * CopyToChoiceSIzer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString CopyToOptions = Config->ReadAStr( CONFIG_KEY_COPYTO_OPTION, wxEmptyString, CONFIG_PATH_COPYTO );
    int Index;
    int Count;
    if( ( Count = CopyToOptions.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            CopyToOptions[ Index ] = CopyToOptions[ Index ].BeforeFirst( wxT( ':' ) );
        }
    }

    guMediaCollection * MediaCollection = m_MediaViewer->GetMediaCollection();
    int CopyToIndex = CopyToOptions.Index( MediaCollection->m_DefaultCopyAction );

	m_CopyToChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, CopyToOptions, 0 );
	m_CopyToChoice->SetSelection( CopyToIndex );
	CopyToChoiceSIzer->Add( m_CopyToChoice, 1, wxTOP|wxBOTTOM|wxEXPAND, 5 );

	m_CopyToSetupBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search_engine ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	CopyToChoiceSIzer->Add( m_CopyToSetupBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	CopyToSizer->Add( CopyToChoiceSIzer, 1, wxEXPAND, 5 );

	wxStaticText * DestPathStaticText = new wxStaticText( this, wxID_ANY, _( "Destination Folder:" ), wxDefaultPosition, wxDefaultSize, 0 );
	DestPathStaticText->Wrap( -1 );
	CopyToSizer->Add( DestPathStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DestPathDirPicker = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, _( "Select a folder" ), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_DIR_MUST_EXIST );
    m_DestPathDirPicker->SetPath( m_MediaViewer->AudioPath() );
	CopyToSizer->Add( m_DestPathDirPicker, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	MainStaticBoxSizer->Add( CopyToSizer, 0, wxEXPAND, 5 );

	wxStaticBoxSizer * FilesSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( " Files " ) ), wxVERTICAL );

	m_FilesListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE );
	Count = m_Tracks->Count();
	for( Index = 0; Index < Count; Index++ )
	{
	    const guTrack & CurTrack = m_Tracks->Item( Index );
	    wxString CurFile = CurTrack.m_FileName.AfterLast( wxT( '/' ) );
	    if( CurTrack.m_Offset )
	    {
            CurFile += wxT( "@" ) + LenToString( CurTrack.m_Offset );
            CurFile += wxT( " / " ) + LenToString( CurTrack.m_Length );
	    }

	    m_FilesListBox->Append( CurFile );
	}
	FilesSizer->Add( m_FilesListBox, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer * AddFilesSizer = new wxBoxSizer( wxHORIZONTAL );

	m_AddFilesBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	AddFilesSizer->Add( m_AddFilesBtn, 0, wxLEFT, 5 );

	m_DelFilesBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelFilesBtn->Enable( false );
	AddFilesSizer->Add( m_DelFilesBtn, 0, wxRIGHT, 5 );


	AddFilesSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_FilesLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_FilesLabel->Wrap( -1 );
	AddFilesSizer->Add( m_FilesLabel, 0, wxRIGHT|wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	FilesSizer->Add( AddFilesSizer, 0, wxEXPAND, 5 );

	MainStaticBoxSizer->Add( FilesSizer, 1, wxEXPAND, 5 );

	MainSizer->Add( MainStaticBoxSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxStdDialogButtonSizer * DlgButtons = new wxStdDialogButtonSizer();
	m_DlgButtonsOK = new wxButton( this, wxID_OK );
	m_DlgButtonsOK->Enable( !m_CopyToChoice->GetStringSelection().IsEmpty() && !m_DestPathDirPicker->GetPath().IsEmpty() && !m_FilesListBox->IsEmpty() );
	DlgButtons->AddButton( m_DlgButtonsOK );
	wxButton * DlgButtonsCancel = new wxButton( this, wxID_CANCEL );
	DlgButtons->AddButton( DlgButtonsCancel );
	DlgButtons->SetAffirmativeButton( m_DlgButtonsOK );
	DlgButtons->SetCancelButton( DlgButtonsCancel );
	DlgButtons->Realize();
	MainSizer->Add( DlgButtons, 0, wxEXPAND|wxALL, 5 );

	SetSizer( MainSizer );
	Layout();

	m_DlgButtonsOK->SetDefault();

	UpdateCounters();

    m_CopyToSetupBtn->Bind( wxEVT_BUTTON, &guImportFiles::OnCopyToSetupClicked, this );
    m_FilesListBox->Bind( wxEVT_LISTBOX, &guImportFiles::OnFileSelected, this );
    m_AddFilesBtn->Bind( wxEVT_BUTTON, &guImportFiles::OnAddFilesClicked, this );
    m_DelFilesBtn->Bind( wxEVT_BUTTON, &guImportFiles::OnDelFilesClicked, this );

    Bind( guConfigUpdatedEvent, &guImportFiles::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guImportFiles::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_COPYTO )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString CopyToOptions = Config->ReadAStr( CONFIG_KEY_COPYTO_OPTION, wxEmptyString, CONFIG_PATH_COPYTO );
        int Index;
        int Count;
        if( ( Count = CopyToOptions.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                CopyToOptions[ Index ] = CopyToOptions[ Index ].BeforeFirst( wxT( ':' ) );
            }
        }
        wxString CurSelected = m_CopyToChoice->GetStringSelection();
        m_CopyToChoice->Clear();
        m_CopyToChoice->Append( CopyToOptions );
        m_CopyToChoice->SetStringSelection( CurSelected );
    }
}

// -------------------------------------------------------------------------------- //
void guImportFiles::OnCopyToSetupClicked( wxCommandEvent &event )
{
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_COPYTO );
    wxPostEvent( ( wxEvtHandler * ) m_MediaViewer->GetMainFrame(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guImportFiles::OnFileSelected( wxCommandEvent &event )
{
    m_DelFilesBtn->Enable( event.GetInt() != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guImportFiles::OnAddFilesClicked( wxCommandEvent &event )
{
    wxFileDialog * FileDialog = new wxFileDialog( this, _( "Select files" ), wxGetHomeDir(), wxEmptyString,
                wxString( _( "Audio files" ) ) + wxT( "|*.mp3;*.ogg;*.oga;*.flac;*.mp4;*.m4a;*.m4b;*.m4p;*.aac;*.wma;*.asf;*.ape;*.wav;*.aif;*.wv;*.tta;*.mpc;*.dsf;*.3gp;*.webm;*.ra;*.8svx;*.ac3;*.dts;*.maud;*.mp2;*.snd;*.voc;*.mpg;*.mov;*.avi;*.mkv;*.mka;*.mid;*.au;*.amr;*.aiff;*.flv|" ) +
                wxString( _( "mp3 files" ) ) + wxT( " (*.mp3)|*.mp3|" ) +
                wxString( _( "ogg files" ) ) + wxT( " (*.ogg;*.oga)|*.ogg;*.oga|" ) +
                wxString( _( "flac files" ) ) + wxT( " (*.flac)|*.flac|" ) +
                wxString( _( "mp4 files" ) ) + wxT( " (*.mp4;*.m4a)|*.mp4;*.m4a;*.m4b;*.m4p;*.aac|" ) +
                wxString( _( "wma files" ) ) + wxT( " (*.wma;*.asf)|*.wma;*.asf|" ) +
                wxString( _( "ape files" ) ) + wxT( " (*.ape)|*.ape|" ) +
                wxString( _( "waf files" ) ) + wxT( " (*.wav)|*.wav|" ) +
                wxString( _( "aif files" ) ) + wxT( " (*.aif)|*.aif|" ) +
                wxString( _( "wv files" ) ) + wxT( " (*.wv)|*.wv|" ) +
                wxString( _( "tta files" ) ) + wxT( " (*.tta)|*.tta|" ) +
                wxString( _( "mpc files" ) ) + wxT( " (*.mpc)|*.mpc" ) +
                wxString( _( "3gp files" ) ) + wxT( " (*.3gp)|*.3gp" ) +
                wxString( _( "webm files" ) ) + wxT( " (*.webm)|*.webm" ) +
                wxString( _( "ra files" ) ) + wxT( " (*.ra)|*.ra" ) +
                wxString( _( "8svx files" ) ) + wxT( " (*.8svx)|*.8svx" ) +
                wxString( _( "ac3 files" ) ) + wxT( " (*.ac3)|*.ac3" ) +
                wxString( _( "dts files" ) ) + wxT( " (*.dts)|*.dts" ) +
                wxString( _( "maud files" ) ) + wxT( " (*.maud)|*.maud" ) +
                wxString( _( "mp2 files" ) ) + wxT( " (*.mp2)|*.mp2" ) +
                wxString( _( "snd files" ) ) + wxT( " (*.snd)|*.snd" ) +
                wxString( _( "voc files" ) ) + wxT( " (*.voc)|*.voc" ) +
                wxString( _( "mpg files" ) ) + wxT( " (*.mpg)|*.mpg" ) +
                wxString( _( "mov files" ) ) + wxT( " (*.mov)|*.mov" ) +
                wxString( _( "avi files" ) ) + wxT( " (*.avi)|*.avi" ) +
                wxString( _( "mkv files" ) ) + wxT( " (*.mkv)|*.mkv" ) +
                wxString( _( "mka files" ) ) + wxT( " (*.mka)|*.mka" ) +
                wxString( _( "mid files" ) ) + wxT( " (*.mid)|*.mid" ) +
                wxString( _( "au files" ) ) + wxT( " (*.au)|*.au" ) +
                wxString( _( "amr files" ) ) + wxT( " (*.amr)|*.amr" ) +
                wxString( _( "aiff files" ) ) + wxT( " (*.aiff)|*.aiff" ) +
                wxString( _( "flv files" ) ) + wxT( " (*.flv)|*.flv" ) +
                wxString( _( "dsd files" ) ) + wxT( " (*.dsf)|*.dsf" ),
                wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE|wxFD_PREVIEW );
    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            wxArrayString NewFiles;
            FileDialog->GetPaths( NewFiles );
            int Index;
            int Count = NewFiles.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                wxString NewFile = NewFiles[ Index ];
                //guLogMessage( wxT( "Openning file '%s'" ), NewFile.c_str() );
                if( wxFileExists( NewFile ) && guIsValidAudioFile( NewFile.Lower() ) )
                {
                    guTrack * Track = new guTrack();
                    if( Track->ReadFromFile( NewFile ) )
                    {
                        Track->m_Type = guTRACK_TYPE_NOTDB;
                        m_Tracks->Add( Track );
                        m_FilesListBox->Append( NewFile.AfterLast( wxT( '/' ) ) );

                    }
                    else
                    {
                        delete Track;
                    }
                }
            }

            UpdateCounters();
            CheckButtons();
        }
        FileDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guImportFiles::OnDelFilesClicked( wxCommandEvent &event )
{
    wxArrayInt Selected;
    m_FilesListBox->GetSelections( Selected );
    int Index;
    int Count = Selected.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_FilesListBox->Delete( Selected[ Index ] );
        m_Tracks->RemoveAt( Selected[ Index ] );
    }
    UpdateCounters();
    CheckButtons();
}

// -------------------------------------------------------------------------------- //
void guImportFiles::CheckButtons( void )
{
	m_DlgButtonsOK->Enable( !m_CopyToChoice->GetStringSelection().IsEmpty() && !m_DestPathDirPicker->GetPath().IsEmpty() && !m_FilesListBox->IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guImportFiles::UpdateCounters( void )
{
    wxLongLong Size = 0;
    wxLongLong Length = 0;
    int Index;
    int Count = m_Tracks->Count();
    if( Count )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack &Track = m_Tracks->Item( Index );
            Size += Track.m_FileSize;
            Length += Track.m_Length;
        }

        wxString SelInfo = wxString::Format( wxT( "%u " ), Count );
        SelInfo += Count == 1 ? _( "track" ) : _( "tracks" );
        SelInfo += wxString::Format( wxT( ",  %s,  %s" ),
            LenToString( Length.GetValue() ).c_str(),
            SizeToString( Size.GetValue() ).c_str() );

        m_FilesLabel->SetLabel( SelInfo );
        Layout();
    }
    else
    {
        m_FilesLabel->SetLabel( _( "No files added" ) );
        Layout();
    }
}

}

// -------------------------------------------------------------------------------- //
