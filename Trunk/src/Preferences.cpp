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
#include "Preferences.h"

#include "Images.h"
#include "MD5.h"
#include "Utils.h"

#include <wx/statline.h>
#include <wx/uri.h>

wxString PatternToExample( const wxString &Pattern );

// -------------------------------------------------------------------------------- //
guPrefDialog::guPrefDialog( wxWindow* parent ) :
    wxDialog( parent, wxID_ANY, _( "Preferences" ), wxDefaultPosition, wxSize( 500, 515 ), wxDEFAULT_DIALOG_STYLE )
{
	wxBoxSizer *        MainSizer;
	wxBoxSizer *        GenMainSizer;
	wxStaticBoxSizer *  StartSizer;
	wxStaticBoxSizer *  BehaviSizer;
	wxStaticBoxSizer *  OnCloseSizer;
	wxBoxSizer *        LibMainSizer;
	wxStaticBoxSizer *  PathsSizer;
	wxBoxSizer *        PathButtonsSizer;
	wxStaticBoxSizer*   CoversSizer;
	wxStaticBoxSizer *  LibOptionsSIzer;
	wxBoxSizer *        ASMainSizer;
	wxStaticBoxSizer *  LastFMASSizer;
	wxFlexGridSizer *   ASLoginSizer;
	wxStaticBoxSizer*   SmartPlayListSizer;
	wxFlexGridSizer *   SmartPlayListFlexGridSizer;
	wxStaticBoxSizer *  OnlineFiltersSizer;
	wxBoxSizer *        OnlineBtnSizer;
	wxStaticBoxSizer *  BrowserCmdSizer;
//	wxBoxSizer *        LinksMainSizer;
//	wxStaticBoxSizer *  LinksSizer;
//	wxBoxSizer *        LinksBtnSizer;
//	wxStaticBoxSizer *  LinksHelpSizer;

    m_LinkSelected = wxNOT_FOUND;

    m_Config = ( guConfig * ) guConfig::Get();
    if( !m_Config )
        guLogError( wxT( "Invalid m_Config object in preferences dialog" ) );

	m_RadioMinBitRateRadBoxChoices.Add( wxT(   "0" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT(  "16" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT(  "32" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT(  "64" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT(  "96" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT( "128" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT( "160" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT( "192" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT( "256" ) );
	m_RadioMinBitRateRadBoxChoices.Add( wxT( "320" ) );

    m_LangNames.Add( _( "Default" ) );        m_LangIds.Add( wxEmptyString );
    m_LangNames.Add( _( "English" ) );        m_LangIds.Add( wxT( "en" ) );
    m_LangNames.Add( _( "French" ) );         m_LangIds.Add( wxT( "fr" ) );
    m_LangNames.Add( _( "German" ) );         m_LangIds.Add( wxT( "de" ) );
    m_LangNames.Add( _( "Italian" ) );        m_LangIds.Add( wxT( "it" ) );
    m_LangNames.Add( _( "Portuguese" ) );     m_LangIds.Add( wxT( "pt" ) );
    m_LangNames.Add( _( "Spanish" ) );        m_LangIds.Add( wxT( "es" ) );

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_MainNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

    // General Preferences Panel
	m_GenPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	GenMainSizer = new wxBoxSizer( wxVERTICAL );
	StartSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" On Start ") ), wxVERTICAL );

	m_ShowSplashChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Show splash screen"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowSplashChkBox->SetValue( m_Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "General" ) ) );
	StartSizer->Add( m_ShowSplashChkBox, 0, wxALL, 5 );

	m_MinStartChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Start minimized"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MinStartChkBox->SetValue( m_Config->ReadBool( wxT( "StartMinimized" ), false, wxT( "General" ) ) );
	StartSizer->Add( m_MinStartChkBox, 0, wxALL, 5 );

	GenMainSizer->Add( StartSizer, 0, wxEXPAND|wxALL, 5 );

	BehaviSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" Behaviour ") ), wxVERTICAL );

	m_TaskIconChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Activate task bar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TaskIconChkBox->SetValue( m_Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "General" ) ) );
	BehaviSizer->Add( m_TaskIconChkBox, 0, wxALL, 5 );

	m_EnqueueChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Enqueue as default action"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnqueueChkBox->SetValue( m_Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) );
	BehaviSizer->Add( m_EnqueueChkBox, 0, wxALL, 5 );

	m_DropFilesChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Drop files clear playlist"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DropFilesChkBox->SetValue( m_Config->ReadBool( wxT( "DropFilesClearPlayList" ), false, wxT( "General" ) ) );
	BehaviSizer->Add( m_DropFilesChkBox, 0, wxALL, 5 );

	m_RndPlayChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Play random track when playlist is empty"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RndPlayChkBox->SetValue( m_Config->ReadBool( wxT( "RndTrackOnEmptyPlayList" ), false, wxT( "General" ) ) );
	BehaviSizer->Add( m_RndPlayChkBox, 0, wxALL, 5 );

	m_AlYearOrderChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Albums ordered by Year. Default is by Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlYearOrderChkBox->SetValue( m_Config->ReadNum( wxT( "AlbumYearOrder" ), 0, wxT( "General" ) ) );
	BehaviSizer->Add( m_AlYearOrderChkBox, 0, wxALL, 5 );

	GenMainSizer->Add( BehaviSizer, 0, wxEXPAND|wxALL, 5 );

	OnCloseSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, wxT(" On Close ") ), wxVERTICAL );

	m_SavePlayListChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Save playlist on close"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SavePlayListChkBox->SetValue( m_Config->ReadBool( wxT( "SavePlayListOnClose" ), true, wxT( "General" ) ) );
	OnCloseSizer->Add( m_SavePlayListChkBox, 0, wxALL, 5 );

	m_CloseTaskBarChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Close to task bar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CloseTaskBarChkBox->SetValue( m_Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) );
	OnCloseSizer->Add( m_CloseTaskBarChkBox, 0, wxALL, 5 );

	m_ExitConfirmChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Ask confirmation on exit"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExitConfirmChkBox->SetValue( m_Config->ReadBool( wxT( "ShowCloseConfirm" ), true, wxT( "General" ) ) );
	OnCloseSizer->Add( m_ExitConfirmChkBox, 0, wxALL, 5 );

	GenMainSizer->Add( OnCloseSizer, 1, wxEXPAND|wxALL, 5 );

	m_GenPanel->SetSizer( GenMainSizer );
	m_GenPanel->Layout();
	GenMainSizer->Fit( m_GenPanel );
	m_MainNotebook->AddPage( m_GenPanel, _("General"), true );

    // Library Preferences Panel
	m_LibPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	LibMainSizer = new wxBoxSizer( wxVERTICAL );

	PathsSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, _(" Paths ") ), wxHORIZONTAL );

	m_PathsListBox = new wxListBox( m_LibPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_PathsListBox->Append( m_Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) ) );
	PathsSizer->Add( m_PathsListBox, 1, wxALL|wxEXPAND, 5 );

	PathButtonsSizer = new wxBoxSizer( wxVERTICAL );

	m_AddPathButton = new wxBitmapButton( m_LibPanel, wxID_ANY, wxBitmap( guImage_vol_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PathButtonsSizer->Add( m_AddPathButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DelPathButton = new wxBitmapButton( m_LibPanel, wxID_ANY, wxBitmap( guImage_vol_remove ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelPathButton->Disable();
	PathButtonsSizer->Add( m_DelPathButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	PathsSizer->Add( PathButtonsSizer, 0, wxEXPAND, 5 );

	LibMainSizer->Add( PathsSizer, 1, wxEXPAND|wxALL, 5 );

	CoversSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, _(" Words to detect covers ") ), wxHORIZONTAL );

	m_CoversListBox = new wxListBox( m_LibPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE );
	m_CoversListBox->Append( m_Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) ) );
	CoversSizer->Add( m_CoversListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* CoversButtonsSizer;
	CoversButtonsSizer = new wxBoxSizer( wxVERTICAL );

	m_AddCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, wxBitmap( guImage_vol_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	CoversButtonsSizer->Add( m_AddCoverButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DelCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, wxBitmap( guImage_vol_remove ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelCoverButton->Enable( false );
	CoversButtonsSizer->Add( m_DelCoverButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CoversSizer->Add( CoversButtonsSizer, 0, wxEXPAND, 5 );

	LibMainSizer->Add( CoversSizer, 1, wxEXPAND|wxALL, 5 );


	LibOptionsSIzer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	m_UpdateLibChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Update library on application start"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UpdateLibChkBox->SetValue( m_Config->ReadBool( wxT( "UpdateLibOnStart" ), false, wxT( "General" ) ) );
	LibOptionsSIzer->Add( m_UpdateLibChkBox, 0, wxALL, 5 );

	m_CoverSearchChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Search for missing covers on application start"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CoverSearchChkBox->SetValue( m_Config->ReadBool( wxT( "CoverSearchOnStart" ), false, wxT( "General" ) ) );
	LibOptionsSIzer->Add( m_CoverSearchChkBox, 0, wxALL, 5 );

	LibMainSizer->Add( LibOptionsSIzer, 0, wxEXPAND|wxALL, 5 );

	m_LibPanel->SetSizer( LibMainSizer );
	m_LibPanel->Layout();
	LibMainSizer->Fit( m_LibPanel );
	m_MainNotebook->AddPage( m_LibPanel, _("Library"), false );

    // LastFM Panel
	m_LastFMPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	ASMainSizer = new wxBoxSizer( wxVERTICAL );

	LastFMASSizer = new wxStaticBoxSizer( new wxStaticBox( m_LastFMPanel, wxID_ANY, _(" LastFM Audioscrobble ") ), wxVERTICAL );

	m_ASEnableChkBox = new wxCheckBox( m_LastFMPanel, wxID_ANY, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ASEnableChkBox->SetValue( m_Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) ) );
	LastFMASSizer->Add( m_ASEnableChkBox, 0, wxALL, 5 );

	ASLoginSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	ASLoginSizer->SetFlexibleDirection( wxBOTH );
	ASLoginSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_UserNameStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_UserNameStaticText->Wrap( -1 );
	ASLoginSizer->Add( m_UserNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_UserNameTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
	m_UserNameTextCtrl->SetValue( m_Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "LastFM" ) ) );
	ASLoginSizer->Add( m_UserNameTextCtrl, 0, wxALL, 5 );

	m_PasswdStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PasswdStaticText->Wrap( -1 );
	ASLoginSizer->Add( m_PasswdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_PasswdTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), wxTE_PASSWORD );
	// Password is saved in md5 form so we cant load it back
	ASLoginSizer->Add( m_PasswdTextCtrl, 0, wxALL, 5 );

	LastFMASSizer->Add( ASLoginSizer, 1, wxEXPAND, 5 );

	ASMainSizer->Add( LastFMASSizer, 1, wxEXPAND|wxALL, 5 );

	SmartPlayListSizer = new wxStaticBoxSizer( new wxStaticBox( m_LastFMPanel, wxID_ANY, _( " Smart Playlists " ) ), wxVERTICAL );

	SmartPlayListFlexGridSizer = new wxFlexGridSizer( 4, 2, 0, 0 );
	SmartPlayListFlexGridSizer->SetFlexibleDirection( wxBOTH );
	SmartPlayListFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_SmartPlayListMinTracksSpinCtrl = new wxSpinCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 10, 4 );
    m_SmartPlayListMinTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MinPlayTracks" ), 4, wxT( "SmartPlayList" ) ) );
	SmartPlayListFlexGridSizer->Add( m_SmartPlayListMinTracksSpinCtrl, 0, wxALL|wxALIGN_RIGHT, 5 );

	m_SmartPlayListMinTracksStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Tracks left to start search"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SmartPlayListMinTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( m_SmartPlayListMinTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_SmartPlayListAddTracksSpinCtrl = new wxSpinCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 10, 3 );
    m_SmartPlayListAddTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "AddPlayTracks" ), 3, wxT( "SmartPlayList" ) ) );
	SmartPlayListFlexGridSizer->Add( m_SmartPlayListAddTracksSpinCtrl, 0, wxALL|wxALIGN_RIGHT, 5 );

	m_SmartPlayListAddTracksStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Tracks added each time"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SmartPlayListAddTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( m_SmartPlayListAddTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_SmartPlayListMaxTracksSpinCtrl = new wxSpinCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 5, 99, 20 );
    m_SmartPlayListMaxTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MaxPlayTracks" ), 20, wxT( "SmartPlayList" ) ) );
	SmartPlayListFlexGridSizer->Add( m_SmartPlayListMaxTracksSpinCtrl, 0, wxALL|wxALIGN_RIGHT, 5 );

	m_SmartPlayListMaxTracksStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Max tracks kept in playlist"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SmartPlayListMaxTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( m_SmartPlayListMaxTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	SmartPlayListSizer->Add( SmartPlayListFlexGridSizer, 1, wxEXPAND, 5 );

	ASMainSizer->Add( SmartPlayListSizer, 1, wxALL|wxEXPAND, 5 );

	m_LastFMPanel->SetSizer( ASMainSizer );
	m_LastFMPanel->Layout();
	ASMainSizer->Fit( m_LastFMPanel );
	m_MainNotebook->AddPage( m_LastFMPanel, wxT( "LastFM" ), false );

    // Online Services Filter
	m_OnlinePanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* OnlineMainSizer;
	OnlineMainSizer = new wxBoxSizer( wxVERTICAL );

	OnlineFiltersSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _(" Filters ") ), wxHORIZONTAL );

	m_OnlineFiltersListBox = new wxListBox( m_OnlinePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_OnlineFiltersListBox->Append( m_Config->ReadAStr( wxT( "Filter" ), wxEmptyString, wxT( "SearchFilters" ) ) );
	OnlineFiltersSizer->Add( m_OnlineFiltersListBox, 1, wxALL|wxEXPAND, 5 );

	OnlineBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_OnlineAddBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, wxBitmap( guImage_vol_add ), wxDefaultPosition, wxDefaultSize, 0 );
	OnlineBtnSizer->Add( m_OnlineAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_OnlineDelBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, wxBitmap( guImage_vol_remove ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_OnlineDelBtn->Disable();
	OnlineBtnSizer->Add( m_OnlineDelBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	OnlineFiltersSizer->Add( OnlineBtnSizer, 0, wxEXPAND, 5 );

//	OnlineFiltersSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	OnlineMainSizer->Add( OnlineFiltersSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * OnlineLangSizer;
	OnlineLangSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _( " Language " ) ), wxHORIZONTAL );

	m_LangStaticText = new wxStaticText( m_OnlinePanel, wxID_ANY, _("Language:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_LangStaticText->Wrap( -1 );
	OnlineLangSizer->Add( m_LangStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LangChoice = new wxChoice( m_OnlinePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_LangNames, 0 );
    int LangIndex = m_LangIds.Index( m_Config->ReadStr( wxT( "Language" ), wxEmptyString, wxT( "LastFM" ) ) );
    if( LangIndex == wxNOT_FOUND )
        m_LangChoice->SetSelection( 0 );
    else
        m_LangChoice->SetSelection( LangIndex );
	OnlineLangSizer->Add( m_LangChoice, 0, wxALL, 5 );

	OnlineMainSizer->Add( OnlineLangSizer, 0, wxEXPAND|wxALL, 5 );

	BrowserCmdSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _(" Browser command ") ), wxHORIZONTAL );

	m_BrowserCmdTextCtrl = new wxTextCtrl( m_OnlinePanel, wxID_ANY, m_Config->ReadStr( wxT( "BrowserCommand" ), wxT( "firefox --new-tab" ), wxT( "General" ) ), wxDefaultPosition, wxDefaultSize, 0 );
	BrowserCmdSizer->Add( m_BrowserCmdTextCtrl, 1, wxALL, 5 );

	OnlineMainSizer->Add( BrowserCmdSizer, 0, wxEXPAND|wxALL, 5 );

	m_RadioMinBitRateRadBox = new wxRadioBox( m_OnlinePanel, wxID_ANY, _( "Radio Min. Allowed Bitrate "), wxDefaultPosition, wxDefaultSize, m_RadioMinBitRateRadBoxChoices, 10, wxRA_SPECIFY_COLS );
	wxString MinBitRate = m_Config->ReadStr( wxT( "RadioMinBitRate" ), wxT( "128" ), wxT( "Radios" ) );
	int RadioMinBitRateIndex = m_RadioMinBitRateRadBoxChoices.Index( MinBitRate );
    if( RadioMinBitRateIndex != wxNOT_FOUND )
    {
        m_RadioMinBitRateRadBox->SetSelection( RadioMinBitRateIndex );
    }
    else
    {
        m_RadioMinBitRateRadBox->SetSelection( 4 );
    }
	OnlineMainSizer->Add( m_RadioMinBitRateRadBox, 0, wxALL|wxEXPAND, 5 );

	m_OnlinePanel->SetSizer( OnlineMainSizer );
	m_OnlinePanel->Layout();
	OnlineMainSizer->Fit( m_OnlinePanel );
	m_MainNotebook->AddPage( m_OnlinePanel, _( "Online Services" ), false );


    // Links
//	m_LinksPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//	LinksMainSizer = new wxBoxSizer( wxVERTICAL );
//
//	LinksSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, wxT(" Links ") ), wxHORIZONTAL );
//
//	m_LinksListBox = new wxListBox( m_LinksPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
//	m_LinksListBox->Append( m_Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) ) );
//	LinksSizer->Add( m_LinksListBox, 1, wxALL|wxEXPAND, 5 );
//
//	LinksBtnSizer = new wxBoxSizer( wxVERTICAL );
//
//	m_LinksAddBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_vol_add ), wxDefaultPosition, wxDefaultSize, 0 );
//	LinksBtnSizer->Add( m_LinksAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
//
//	m_LinksMoveUpBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_go_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	m_LinksMoveUpBtn->Enable( false );
//	LinksBtnSizer->Add( m_LinksMoveUpBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
//
//	m_LinksMoveDownBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_go_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	m_LinksMoveDownBtn->Enable( false );
//	LinksBtnSizer->Add( m_LinksMoveDownBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
//
//	m_LinksDelBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_vol_remove ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	m_LinksDelBtn->Enable( false );
//	LinksBtnSizer->Add( m_LinksDelBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
//
//	LinksSizer->Add( LinksBtnSizer, 0, wxEXPAND, 5 );
//
//	LinksMainSizer->Add( LinksSizer, 1, wxEXPAND|wxALL, 5 );
//
//	LinksHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Help ") ), wxVERTICAL );
//
//	wxStaticText * LinksHelpText;
//	LinksHelpText = new wxStaticText( m_LinksPanel, wxID_ANY, _( "Add urls using :\n{lang} : 2 lettes language code.\n{text} : Text to search." ), wxDefaultPosition, wxDefaultSize, 0 );
//	LinksHelpText->Wrap( -1 );
//	LinksHelpSizer->Add( LinksHelpText, 0, wxALL, 5 );
//
//	LinksMainSizer->Add( LinksHelpSizer, 0, wxEXPAND|wxALL, 5 );
//
//	m_LinksPanel->SetSizer( LinksMainSizer );
//	m_LinksPanel->Layout();
//	LinksMainSizer->Fit( m_LinksPanel );
//	m_MainNotebook->AddPage( m_LinksPanel, _( "Links" ), false );

	m_LinksPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* LinksMainSizer;
	LinksMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* LinksLabelSizer;
	LinksLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Links ") ), wxVERTICAL );

	wxBoxSizer* LinksListBoxSizer;
	LinksListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_LinksListBox = new wxListBox( m_LinksPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_LinksListBox->Append( m_Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) ) );
	m_LinksNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "SearchLinks" ) );
	while( m_LinksNames.Count() < m_LinksListBox->GetCount() )
        m_LinksNames.Add( wxEmptyString );
	LinksListBoxSizer->Add( m_LinksListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* LinksBtnSizer;
	LinksBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LinksAddBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_vol_add ), wxDefaultPosition, wxDefaultSize, 0 );
    m_LinksAddBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveUpBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_go_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveUpBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveUpBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveDownBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_go_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveDownBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveDownBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksDelBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_vol_remove ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksDelBtn->Enable( false );

	LinksBtnSizer->Add( m_LinksDelBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	LinksListBoxSizer->Add( LinksBtnSizer, 0, wxEXPAND, 5 );

	LinksLabelSizer->Add( LinksListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer* LinksEditorSizer;
	LinksEditorSizer = new wxBoxSizer( wxHORIZONTAL );

	wxFlexGridSizer* LinksFieldsSizer;
	LinksFieldsSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	LinksFieldsSizer->AddGrowableCol( 1 );
	LinksFieldsSizer->SetFlexibleDirection( wxBOTH );
	LinksFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText * LinkUrlStaticText;
	LinkUrlStaticText = new wxStaticText( m_LinksPanel, wxID_ANY, _("Url:"), wxDefaultPosition, wxDefaultSize, 0 );
	LinkUrlStaticText->Wrap( -1 );
	LinksFieldsSizer->Add( LinkUrlStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_LinksUrlTextCtrl = new wxTextCtrl( m_LinksPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LinksFieldsSizer->Add( m_LinksUrlTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

    wxStaticText * LinkNameStaticText;
	LinkNameStaticText = new wxStaticText( m_LinksPanel, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	LinkNameStaticText->Wrap( -1 );
	LinksFieldsSizer->Add( LinkNameStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_LinksNameTextCtrl = new wxTextCtrl( m_LinksPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LinksFieldsSizer->Add( m_LinksNameTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

	LinksEditorSizer->Add( LinksFieldsSizer, 1, wxEXPAND, 5 );

	m_LinksAcceptBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, wxBitmap( guImage_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksAcceptBtn->Enable( false );

	LinksEditorSizer->Add( m_LinksAcceptBtn, 0, wxALL, 5 );

	LinksLabelSizer->Add( LinksEditorSizer, 0, wxEXPAND, 5 );

	LinksMainSizer->Add( LinksLabelSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer* LinksHelpSizer;
	LinksHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Help ") ), wxVERTICAL );

	wxStaticText * LinksHelpText;
	LinksHelpText = new wxStaticText( m_LinksPanel, wxID_ANY, _("Add urls using :\n{lang} : 2 lettes language code.\n{text} : Text to search."), wxDefaultPosition, wxDefaultSize, 0 );
	LinksHelpText->Wrap( -1 );
	LinksHelpSizer->Add( LinksHelpText, 0, wxALL, 5 );

	LinksMainSizer->Add( LinksHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_LinksPanel->SetSizer( LinksMainSizer );
	m_LinksPanel->Layout();
	LinksMainSizer->Fit( m_LinksPanel );
	m_MainNotebook->AddPage( m_LinksPanel, _("Links"), false );

    // Copy To Panel
	m_CopyPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* CopyToMainSizer;
	CopyToMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* CopyToFileNameSizer;
	CopyToFileNameSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _( " FileName Pattern " ) ), wxVERTICAL );

    wxString CopyToPattern = m_Config->ReadStr( wxT( "CopyToPattern" ), wxT("{g}/{a}/{b}/{n} - {a} - {t}"), wxT( "General" ) );
	m_CopyToFileName = new wxTextCtrl( m_CopyPanel, wxID_ANY, CopyToPattern, wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFileNameSizer->Add( m_CopyToFileName, 1, wxEXPAND|wxALL, 5 );

	CopyToMainSizer->Add( CopyToFileNameSizer, 0, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer* CopyToHelpSizer;
	CopyToHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Help ") ), wxVERTICAL );

	wxStaticText * CopyToHelpText;
	CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "{a}\t- Artist\t\t\t('U2')\n{b}\t- Album\t\t\t('The Josua Tree')\n{f}\t- Original Filename\t( 'With or without you.mp3')\n{g}\t- Genre\t\t\t('Pop')\n{n}\t- Number\t\t\t('3')\n{t}\t- Title\t\t\t('With or Without You')\n{y}\t- Year\t\t\t('1987')\n" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 0, wxALL, 5 );

	wxStaticBoxSizer * CopyToExampleSizer;
	CopyToExampleSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Example ") ), wxVERTICAL );

	m_CopyToExampleTextCtrl = new wxTextCtrl( m_CopyPanel, wxID_ANY, PatternToExample( CopyToPattern ), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	CopyToExampleSizer->Add( m_CopyToExampleTextCtrl, 0, wxALL|wxEXPAND, 5 );

	CopyToHelpSizer->Add( CopyToExampleSizer, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );

	CopyToMainSizer->Add( CopyToHelpSizer, 1, wxEXPAND|wxALL, 5 );

	m_CopyPanel->SetSizer( CopyToMainSizer );
	m_CopyPanel->Layout();
	CopyToMainSizer->Fit( m_CopyPanel );
	m_MainNotebook->AddPage( m_CopyPanel, _( "Copy To" ), false );


    //
	MainSizer->Add( m_MainNotebook, 1, wxEXPAND | wxALL, 5 );

    wxStdDialogButtonSizer *    ButtonsSizer;
    wxButton *                  ButtonsSizerOK;
    wxButton *                  ButtonsSizerCancel;

	ButtonsSizer = new wxStdDialogButtonSizer();
	ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    //
    m_PathSelected = wxNOT_FOUND;
    m_FilterSelected = wxNOT_FOUND;

	m_PathsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxSelected ), NULL, this );
	m_AddPathButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddPathBtnClick ), NULL, this );
	m_DelPathButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPathBtnClick ), NULL, this );
	m_PathsListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxDClicked ), NULL, this );

	m_CoversListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCoversListBoxSelected ), NULL, this );
	m_AddCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddCoverBtnClick ), NULL, this );
	m_DelCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelCoverBtnClick ), NULL, this );
	m_CoversListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnCoverListBoxDClicked ), NULL, this );

	m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
	m_OnlineAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
	m_OnlineDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
	m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );

	m_LinksListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
	//m_LinksListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLinkListBoxDClicked ), NULL, this );
	m_LinksAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
	m_LinksDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
	m_LinksMoveUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
	m_LinksMoveDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
	m_LinksUrlTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );

	m_CopyToFileName->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToFileNameUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guPrefDialog::~guPrefDialog()
{
    if( GetReturnCode() == wxID_OK )
    {
        m_Config = ( guConfig * ) guConfig::Get();
        if( !m_Config )
            guLogError( wxT( "Invalid m_Config object in preferences dialog" ) );

        // Save all configurations
        m_Config->WriteBool( wxT( "ShowSplashScreen" ), m_ShowSplashChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "StartMinimized" ), m_MinStartChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "ShowTaskBarIcon" ), m_TaskIconChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "DefaultActionEnqueue" ), m_EnqueueChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "DropFilesClearPlaylist" ), m_DropFilesChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "RndTrackOnEmptyPlayList" ), m_RndPlayChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteNum( wxT( "AlbumYearOrder" ), m_AlYearOrderChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "SavePlayListOnClose" ), m_SavePlayListChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "CloseToTaskBar" ), m_CloseTaskBarChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "ShowCloseConfirm" ), m_ExitConfirmChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteAStr( wxT( "LibPath" ), m_PathsListBox->GetStrings(), wxT( "LibPaths" ) );
        m_Config->WriteAStr( wxT( "Word" ), m_CoversListBox->GetStrings(), wxT( "CoverSearch" ) );
        m_Config->WriteBool( wxT( "UpdateLibOnStart" ), m_UpdateLibChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "CoverSearchOnStart" ), m_CoverSearchChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "SubmitEnabled" ), m_ASEnableChkBox->GetValue(), wxT( "LastFM" ) );
        m_Config->WriteStr( wxT( "UserName" ), m_UserNameTextCtrl->GetValue(), wxT( "LastFM" ) );
        if( !m_PasswdTextCtrl->GetValue().IsEmpty() )
        {
            guMD5 MD5;
            m_Config->WriteStr( wxT( "Password" ), MD5.MD5( m_PasswdTextCtrl->GetValue() ), wxT( "LastFM" ) );
            //guLogMessage( wxT( "Pass: %s" ), PasswdTextCtrl->GetValue().c_str() );
            //guLogMessage( wxT( "MD5 : %s" ), MD5.MD5( PasswdTextCtrl->GetValue() ).c_str() );
        }
        // LastFM Panel Info language
        m_Config->WriteStr( wxT( "Language" ), m_LangIds[ m_LangChoice->GetSelection() ], wxT( "LastFM" ) );
        m_Config->WriteNum( wxT( "MinPlayTracks" ), m_SmartPlayListMinTracksSpinCtrl->GetValue(), wxT( "SmartPlayList" ) );
        m_Config->WriteNum( wxT( "AddPlayTracks" ), m_SmartPlayListAddTracksSpinCtrl->GetValue(), wxT( "SmartPlayList" ) );
        m_Config->WriteNum( wxT( "MaxPlayTracks" ), m_SmartPlayListMaxTracksSpinCtrl->GetValue(), wxT( "SmartPlayList" ) );
        m_Config->WriteAStr( wxT( "Filter" ), m_OnlineFiltersListBox->GetStrings(), wxT( "SearchFilters" ) );
        m_Config->WriteStr( wxT( "BrowserCommand" ), m_BrowserCmdTextCtrl->GetValue(), wxT( "General" ) );
        m_Config->WriteStr( wxT( "RadioMinBitRate" ), m_RadioMinBitRateRadBoxChoices[ m_RadioMinBitRateRadBox->GetSelection() ], wxT( "Radios" ) );
        wxArrayString SearchLinks = m_LinksListBox->GetStrings();
        m_Config->WriteAStr( wxT( "Link" ), SearchLinks, wxT( "SearchLinks" ) );
        m_Config->WriteAStr( wxT( "Name" ), m_LinksNames, wxT( "SearchLinks" ), false );
        m_Config->WriteStr( wxT( "CopyToPattern" ), m_CopyToFileName->GetValue(), wxT( "General" ) );

        // TODO : Make this process in a thread
        int index;
        int count = SearchLinks.Count();
        for( index = 0; index < count; index++ )
        {
            wxURI Uri( SearchLinks[ index ] );
            wxString IconFile = wxGetHomeDir() + wxT( "/.guayadeque/LinkIcons/" ) + Uri.GetServer() + wxT( ".ico" );
            if( !wxFileExists( IconFile ) )
            {
                if( DownloadFile( Uri.GetServer() + wxT( "/favicon.ico" ), IconFile ) )
                {
                    wxImage Image( IconFile, wxBITMAP_TYPE_ANY );
                    if( Image.IsOk() )
                    {
                        if( Image.GetWidth() > 25 || Image.GetHeight() > 25 )
                        {
                            Image.Rescale( 25, 25, wxIMAGE_QUALITY_HIGH );
                        }
                        if( Image.IsOk() )
                        {
                            Image.SaveFile( IconFile, wxBITMAP_TYPE_ICO );
                        }
                    }
                }
                else
                {
                    guLogError( wxT( "Coult not get the icon from SearchLink server '%s'" ), Uri.GetServer().c_str() );
                }
            }
        }

    }
    //
	m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxSelected ), NULL, this );
	m_AddPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddPathBtnClick ), NULL, this );
	m_DelPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPathBtnClick ), NULL, this );
	m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxDClicked ), NULL, this );

	m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCoversListBoxSelected ), NULL, this );
	m_AddCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddCoverBtnClick ), NULL, this );
	m_DelCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelCoverBtnClick ), NULL, this );
	m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnCoverListBoxDClicked ), NULL, this );

	m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
	m_OnlineAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
	m_OnlineDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
	m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );

	m_LinksListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
	//m_LinksListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLinkListBoxDClicked ), NULL, this );
	m_LinksAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
	m_LinksDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
	m_LinksMoveUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
	m_LinksMoveDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
	m_LinksUrlTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );

	m_CopyToFileName->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToFileNameUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPathsListBoxSelected( wxCommandEvent& event )
{
    m_PathSelected = event.GetInt();
    if( m_PathSelected != wxNOT_FOUND )
    {
        m_DelPathButton->Enable();
    }
    else
    {
        m_DelPathButton->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnAddPathBtnClick( wxCommandEvent& event )
{
    wxDirDialog * DirDialog = new wxDirDialog( this, _( "Select library path" ), wxGetHomeDir() );
    if( DirDialog )
    {
        if( DirDialog->ShowModal() == wxID_OK )
        {
            if( m_PathsListBox->FindString( DirDialog->GetPath(), true ) == wxNOT_FOUND )
            {
                m_PathsListBox->Append( DirDialog->GetPath() );
            }
        }
        DirDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnDelPathBtnClick( wxCommandEvent& event )
{
    if( m_PathSelected != wxNOT_FOUND )
    {
        m_PathsListBox->Delete( m_PathSelected );
        m_PathSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPathsListBoxDClicked( wxCommandEvent &event )
{
    int index = event.GetInt();
    if( index != wxNOT_FOUND )
    {
        wxDirDialog * DirDialog = new wxDirDialog( this, _( "Change library path" ), m_PathsListBox->GetString( index ) );
        if( DirDialog )
        {
            if( DirDialog->ShowModal() == wxID_OK )
            {
                if( m_PathsListBox->FindString( DirDialog->GetPath(), true ) == wxNOT_FOUND )
                {
                    m_PathsListBox->SetString( index, DirDialog->GetPath() );
                }
            }
            DirDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCoversListBoxSelected( wxCommandEvent& event )
{
    m_CoverSelected = event.GetInt();
    if( m_CoverSelected != wxNOT_FOUND )
    {
        m_DelCoverButton->Enable();
    }
    else
    {
        m_DelCoverButton->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnAddCoverBtnClick( wxCommandEvent& event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Word: " ), _( "Enter the text to find covers" ), wxEmptyString );
    if( EntryDialog )
    {
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            if( m_CoversListBox->FindString( EntryDialog->GetValue().Lower() ) == wxNOT_FOUND )
            {
                m_CoversListBox->Append( EntryDialog->GetValue().Lower() );
            }
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnDelCoverBtnClick( wxCommandEvent& event )
{
    if( m_CoverSelected != wxNOT_FOUND )
    {
        m_CoversListBox->Delete( m_CoverSelected );
        m_CoverSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCoverListBoxDClicked( wxCommandEvent &event )
{
    int index = event.GetInt();
    if( index != wxNOT_FOUND )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Word: " ), _( "Edit the text to find covers" ), m_CoversListBox->GetString( index ) );
        if( EntryDialog )
        {
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                if( m_CoversListBox->FindString( EntryDialog->GetValue().Lower() ) == wxNOT_FOUND )
                {
                    m_CoversListBox->SetString( index, EntryDialog->GetValue() );
                }
            }
            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnFiltersListBoxSelected( wxCommandEvent &event )
{
    m_FilterSelected = event.GetInt();
    if( m_FilterSelected != wxNOT_FOUND )
    {
        m_OnlineDelBtn->Enable();
    }
    else
    {
        m_OnlineDelBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnOnlineAddBtnClick( wxCommandEvent& event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Filter: " ), _( "Enter the text to filter" ), wxEmptyString );
    if( EntryDialog )
    {
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            if( m_OnlineFiltersListBox->FindString( EntryDialog->GetValue(), true ) == wxNOT_FOUND )
            {
                m_OnlineFiltersListBox->Append( EntryDialog->GetValue() );
            }
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnOnlineDelBtnClick( wxCommandEvent& event )
{
    if( m_FilterSelected != wxNOT_FOUND )
    {
        m_OnlineFiltersListBox->Delete( m_FilterSelected );
        m_FilterSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnOnlineListBoxDClicked( wxCommandEvent &event )
{
    int index = event.GetInt();
    if( index != wxNOT_FOUND )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Filter: " ), _( "Edit the text to filter" ), m_OnlineFiltersListBox->GetString( index ) );
        if( EntryDialog )
        {
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                if( m_OnlineFiltersListBox->FindString( EntryDialog->GetValue(), true ) == wxNOT_FOUND )
                {
                    m_OnlineFiltersListBox->SetString( index, EntryDialog->GetValue() );
                }
            }
            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksListBoxSelected( wxCommandEvent &event )
{
    m_LinkSelected = event.GetInt();
    if( m_LinkSelected != wxNOT_FOUND )
    {
        m_LinksDelBtn->Enable();

        if( m_LinkSelected > 0 )
            m_LinksMoveUpBtn->Enable();
        else
            m_LinksMoveUpBtn->Disable();

        if( m_LinkSelected < ( int ) ( m_LinksListBox->GetCount() - 1 ) )
            m_LinksMoveDownBtn->Enable();
        else
            m_LinksMoveDownBtn->Disable();

        m_LinksUrlTextCtrl->SetValue( m_LinksListBox->GetString( m_LinkSelected ) );
        m_LinksNameTextCtrl->SetValue( m_LinksNames[ m_LinkSelected ] );
        m_LinksAcceptBtn->Disable();
    }
    else
    {
        m_LinksDelBtn->Disable();
        m_LinksMoveUpBtn->Disable();
        m_LinksMoveDownBtn->Disable();
        m_LinksAcceptBtn->Disable();
        m_LinksUrlTextCtrl->SetValue( wxEmptyString );
        m_LinksNameTextCtrl->SetValue( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksAddBtnClick( wxCommandEvent& event )
{
    wxString Url = m_LinksUrlTextCtrl->GetValue();
    if( !Url.IsEmpty() )
    {
        m_LinksListBox->Append( m_LinksUrlTextCtrl->GetValue() );
        m_LinksNames.Add( m_LinksNameTextCtrl->GetValue() );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksDelBtnClick( wxCommandEvent& event )
{
    if( m_LinkSelected != wxNOT_FOUND )
    {
        m_LinksListBox->Delete( m_LinkSelected );
        m_LinksNames.RemoveAt( m_LinkSelected );
        m_LinkSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinkMoveUpBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_LinksListBox->GetString( m_LinkSelected );
    wxString CurName = m_LinksNames[ m_LinkSelected ];
    m_LinksListBox->SetString( m_LinkSelected, m_LinksListBox->GetString( m_LinkSelected - 1 ) );
    m_LinksNames[ m_LinkSelected ] = m_LinksNames[ m_LinkSelected - 1 ];
    m_LinkSelected--;
    m_LinksListBox->SetString( m_LinkSelected, CurUrl );
    m_LinksNames[ m_LinkSelected ] = CurName;
    m_LinksListBox->SetSelection( m_LinkSelected );

    event.SetInt( m_LinkSelected );
    OnLinksListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinkMoveDownBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_LinksListBox->GetString( m_LinkSelected );
    wxString CurName = m_LinksNames[ m_LinkSelected ];
    m_LinksListBox->SetString( m_LinkSelected, m_LinksListBox->GetString( m_LinkSelected + 1 ) );
    m_LinksNames[ m_LinkSelected ] = m_LinksNames[ m_LinkSelected + 1 ];
    m_LinkSelected++;
    m_LinksListBox->SetString( m_LinkSelected, CurUrl );
    m_LinksNames[ m_LinkSelected ] = CurName;
    m_LinksListBox->SetSelection( m_LinkSelected );

    event.SetInt( m_LinkSelected );
    OnLinksListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksTextChanged( wxCommandEvent &event )
{
    if( !m_LinksUrlTextCtrl->GetValue().IsEmpty() )
    {
        m_LinksAddBtn->Enable();
        if( m_LinkSelected != wxNOT_FOUND )
            m_LinksAcceptBtn->Enable();
    }
    else
    {
        m_LinksAddBtn->Disable();
        if( m_LinkSelected != wxNOT_FOUND )
            m_LinksAcceptBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksSaveBtnClick( wxCommandEvent &event )
{
    m_LinksListBox->SetString( m_LinkSelected, m_LinksUrlTextCtrl->GetValue() );
    m_LinksNames[ m_LinkSelected ] = m_LinksNameTextCtrl->GetValue();
}

// -------------------------------------------------------------------------------- //
wxString PatternToExample( const wxString &Pattern )
{
    wxString RetVal = Pattern;
    RetVal.Replace( wxT( "{a}" ), wxT( "U2" ) );
    RetVal.Replace( wxT( "{b}" ), wxT( "The Josua Tree" ) );
    RetVal.Replace( wxT( "{f}" ), wxT( "With or without you.mp3" ) );
    RetVal.Replace( wxT( "{g}" ), wxT( "Pop" ) );
    RetVal.Replace( wxT( "{n}" ), wxT( "03" ) );
    RetVal.Replace( wxT( "{t}" ), wxT( "With or Without You" ) );
    RetVal.Replace( wxT( "{y}" ), wxT( "1987" ) );
    RetVal.Append( wxT( ".mp3" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToFileNameUpdated( wxCommandEvent &event )
{
    m_CopyToExampleTextCtrl->SetValue( PatternToExample( m_CopyToFileName->GetValue() ) );
}
