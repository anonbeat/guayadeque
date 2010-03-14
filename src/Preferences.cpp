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
guPrefDialog::guPrefDialog( wxWindow* parent, guDbLibrary * db ) //:wxDialog( parent, wxID_ANY, _( "Preferences" ), wxDefaultPosition, wxSize( 600, 530 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
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
	wxStaticBoxSizer *  LibOptionsSizer;
	wxBoxSizer *        ASMainSizer;
	wxStaticBoxSizer *  LastFMASSizer;
	wxFlexGridSizer *   ASLoginSizer;
	wxFlexGridSizer *   SmartPlayListFlexGridSizer;
	wxStaticBoxSizer *  OnlineFiltersSizer;
	wxBoxSizer *        OnlineBtnSizer;
	wxStaticBoxSizer *  BrowserCmdSizer;
    wxPanel *           PodcastPanel;
    wxStaticText *      PodcastPathStaticText;
//	wxBoxSizer *        LinksMainSizer;
//	wxStaticBoxSizer *  LinksSizer;
//	wxBoxSizer *        LinksBtnSizer;
//	wxStaticBoxSizer *  LinksHelpSizer;

    m_Db = db;
    m_LinkSelected = wxNOT_FOUND;
    m_CmdSelected = wxNOT_FOUND;
    m_LibPathsChanged = false;

    m_Config = ( guConfig * ) guConfig::Get();
    if( !m_Config )
        guLogError( wxT( "Invalid m_Config object in preferences dialog" ) );


    wxPoint WindowPos;
    WindowPos.x = m_Config->ReadNum( wxT( "PreferencesPosX" ), -1, wxT( "Positions" ) );
    WindowPos.y = m_Config->ReadNum( wxT( "PreferencesPosY" ), -1, wxT( "Positions" ) );
    wxSize WindowSize;
    WindowSize.x = m_Config->ReadNum( wxT( "PreferencesSizeWidth" ), 600, wxT( "Positions" ) );
    WindowSize.y = m_Config->ReadNum( wxT( "PreferencesSizeHeight" ), 530, wxT( "Positions" ) );

    //wxDialog( parent, wxID_ANY, _( "Songs Editor" ), wxDefaultPosition, wxSize( 625, 440 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
    Create( parent, wxID_ANY, _( "Preferences" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );


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

	m_MainNotebook = new wxListbook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBK_LEFT );

    m_ImageList = new wxImageList( 32, 32 );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_general ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_library ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_playback ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_last_fm ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_online_services ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_podcasts ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_links ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_commands ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_copy_to ) );
    m_MainNotebook->AssignImageList( m_ImageList );


    //
    // General Preferences Panel
    //
	m_GenPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	GenMainSizer = new wxBoxSizer( wxVERTICAL );
	StartSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" On Start ") ), wxVERTICAL );

	m_ShowSplashChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Show splash screen"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowSplashChkBox->SetValue( m_Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "General" ) ) );
	StartSizer->Add( m_ShowSplashChkBox, 0, wxALL, 5 );

	m_MinStartChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Start minimized"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MinStartChkBox->SetValue( m_Config->ReadBool( wxT( "StartMinimized" ), false, wxT( "General" ) ) );
	StartSizer->Add( m_MinStartChkBox, 0, wxALL, 5 );

	wxBoxSizer* StartPlayingSizer;
	StartPlayingSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SavePosCheckBox = new wxCheckBox( m_GenPanel, wxID_ANY, wxT("Restore position for tracks longer than"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SavePosCheckBox->SetValue( m_Config->ReadBool( wxT( "SaveCurrentTrackPos" ), false, wxT( "General" ) ) );

	StartPlayingSizer->Add( m_SavePosCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	m_MinLenSpinCtrl = new wxSpinCtrl( m_GenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 9999, 10 );
	m_MinLenSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MinSavePlayPosLength" ), 10, wxT( "General" ) ) );
	m_MinLenSpinCtrl->SetToolTip( wxT( "set the minimun length in minutes to save track position" ) );

	StartPlayingSizer->Add( m_MinLenSpinCtrl, 0, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MinLenStaticText = new wxStaticText( m_GenPanel, wxID_ANY, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
	MinLenStaticText->Wrap( -1 );
	StartPlayingSizer->Add( MinLenStaticText, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	StartSizer->Add( StartPlayingSizer, 1, wxEXPAND, 5 );

    m_IgnoreLayoutsChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _( "Load default layouts" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreLayoutsChkBox->SetValue( m_Config->ReadBool( wxT( "LoadDefaultLayouts" ), false, wxT( "General" ) ) );
	StartSizer->Add( m_IgnoreLayoutsChkBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

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

////	m_AlYearOrderChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Albums ordered by Year. Default is by Name"), wxDefaultPosition, wxDefaultSize, 0 );
////	m_AlYearOrderChkBox->SetValue( m_Config->ReadNum( wxT( "AlbumYearOrder" ), 0, wxT( "General" ) ) );
////	BehaviSizer->Add( m_AlYearOrderChkBox, 0, wxALL, 5 );
//	wxBoxSizer * YearOrderSizer;
//	YearOrderSizer = new wxBoxSizer( wxHORIZONTAL );
//
//	wxStaticText * YearOrderText = new wxStaticText( m_GenPanel, wxID_ANY, _( "Order Albums  by" ), wxDefaultPosition, wxDefaultSize, 0 );
//	YearOrderText->Wrap( -1 );
//	YearOrderSizer->Add( YearOrderText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );
//
//	wxString m_AlbumOrderChoiceChoices[] = { _("Name"), _("Year"), _("Year Desc.") };
//	int m_AlbumOrderChoiceNChoices = sizeof( m_AlbumOrderChoiceChoices ) / sizeof( wxString );
//	m_AlYearOrderChoice = new wxChoice( m_GenPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_AlbumOrderChoiceNChoices, m_AlbumOrderChoiceChoices, 0 );
//	m_AlYearOrderChoice->SetSelection( m_Config->ReadNum( wxT( "AlbumYearOrder" ), 0, wxT( "General" ) ) );
//	YearOrderSizer->Add( m_AlYearOrderChoice, 0, wxRIGHT, 5 );
//
//	BehaviSizer->Add( YearOrderSizer, 1, wxEXPAND, 5 );

	GenMainSizer->Add( BehaviSizer, 0, wxEXPAND|wxALL, 5 );

	OnCloseSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, wxT(" On Close ") ), wxVERTICAL );

	m_SavePlayListChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Save playlist on close"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SavePlayListChkBox->SetValue( m_Config->ReadBool( wxT( "SavePlayListOnClose" ), true, wxT( "General" ) ) );
	OnCloseSizer->Add( m_SavePlayListChkBox, 0, wxALL, 5 );

	m_CloseTaskBarChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Close to task bar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CloseTaskBarChkBox->SetValue( m_Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) );
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() );
	OnCloseSizer->Add( m_CloseTaskBarChkBox, 0, wxALL, 5 );

	m_ExitConfirmChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Ask confirmation on exit"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExitConfirmChkBox->SetValue( m_Config->ReadBool( wxT( "ShowCloseConfirm" ), true, wxT( "General" ) ) );
	OnCloseSizer->Add( m_ExitConfirmChkBox, 0, wxALL, 5 );

	GenMainSizer->Add( OnCloseSizer, 0, wxEXPAND|wxALL, 5 );

	m_GenPanel->SetSizer( GenMainSizer );
	m_GenPanel->Layout();
	GenMainSizer->Fit( m_GenPanel );
	m_MainNotebook->AddPage( m_GenPanel, _("General"), true, 0 );
	//m_MainNotebook->SetPageImage( 0, 0 );

    //
    // Library Preferences Panel
    //
	m_LibPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	LibMainSizer = new wxBoxSizer( wxVERTICAL );

	PathsSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, _(" Paths ") ), wxHORIZONTAL );

	m_PathsListBox = new wxListBox( m_LibPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_PathsListBox->Append( m_Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) ) );
	PathsSizer->Add( m_PathsListBox, 1, wxALL|wxEXPAND, 5 );

	PathButtonsSizer = new wxBoxSizer( wxVERTICAL );

	m_AddPathButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PathButtonsSizer->Add( m_AddPathButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DelPathButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelPathButton->Disable();
	PathButtonsSizer->Add( m_DelPathButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	PathsSizer->Add( PathButtonsSizer, 0, wxEXPAND, 5 );

	LibMainSizer->Add( PathsSizer, 1, wxEXPAND|wxALL, 5 );

	CoversSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, _(" Words to detect covers ") ), wxHORIZONTAL );

	m_CoversListBox = new wxListBox( m_LibPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	m_CoversListBox->Append( m_Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) ) );
	CoversSizer->Add( m_CoversListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* CoversButtonsSizer;
	CoversButtonsSizer = new wxBoxSizer( wxVERTICAL );

	m_AddCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	CoversButtonsSizer->Add( m_AddCoverButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_UpCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_UpCoverButton->Enable( false );
	CoversButtonsSizer->Add( m_UpCoverButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DownCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DownCoverButton->Enable( false );
	CoversButtonsSizer->Add( m_DownCoverButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DelCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelCoverButton->Enable( false );
	CoversButtonsSizer->Add( m_DelCoverButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CoversSizer->Add( CoversButtonsSizer, 0, wxEXPAND, 5 );

	LibMainSizer->Add( CoversSizer, 1, wxEXPAND|wxALL, 5 );


	LibOptionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	m_UpdateLibChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Update library on application start"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UpdateLibChkBox->SetValue( m_Config->ReadBool( wxT( "UpdateLibOnStart" ), false, wxT( "General" ) ) );
	LibOptionsSizer->Add( m_UpdateLibChkBox, 0, wxALL, 5 );

	m_SaveLyricsChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Save lyrics to audio files"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SaveLyricsChkBox->SetValue( m_Config->ReadBool( wxT( "SaveLyricsToFiles" ), false, wxT( "General" ) ) );
	LibOptionsSizer->Add( m_SaveLyricsChkBox, 0, wxALL, 5 );

	LibMainSizer->Add( LibOptionsSizer, 0, wxEXPAND|wxALL, 5 );

	m_LibPanel->SetSizer( LibMainSizer );
	m_LibPanel->Layout();
	LibMainSizer->Fit( m_LibPanel );
	m_MainNotebook->AddPage( m_LibPanel, _("Library"), false );
	m_MainNotebook->SetPageImage( 1, 1 );

    //
    // Playback Panel
    //
	m_PlayPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxBoxSizer * PlayMainSizer;
	PlayMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * PlayGenSizer;
	PlayGenSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

//	m_RndPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _("Play random track when playlist is empty"), wxDefaultPosition, wxDefaultSize, 0 );
//    m_RndPlayChkBox->SetValue( m_Config->ReadBool( wxT( "RndPlayOnEmptyPlayList" ), false, wxT( "General" ) ) );
//	PlayGenSizer->Add( m_RndPlayChkBox, 0, wxALL, 5 );
//
//	m_DelPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _("Delete played tracks from playlist"), wxDefaultPosition, wxDefaultSize, 0 );
//    m_DelPlayChkBox->SetValue( m_Config->ReadBool( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) ) );
//	PlayGenSizer->Add( m_DelPlayChkBox, 0, wxALL, 5 );
//
//	PlayMainSizer->Add( PlayGenSizer, 0, wxEXPAND|wxALL, 5 );
	wxBoxSizer* RandomPlaySizer;
	RandomPlaySizer = new wxBoxSizer( wxHORIZONTAL );

	m_RndPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Play random" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_RndPlayChkBox->SetValue( m_Config->ReadBool( wxT( "RndPlayOnEmptyPlayList" ), false, wxT( "General" ) ) );
	RandomPlaySizer->Add( m_RndPlayChkBox, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	wxString m_RndModeChoiceChoices[] = { _( "track" ), _( "album" ) };
	int m_RndModeChoiceNChoices = sizeof( m_RndModeChoiceChoices ) / sizeof( wxString );
	m_RndModeChoice = new wxChoice( m_PlayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_RndModeChoiceNChoices, m_RndModeChoiceChoices, 0 );
    m_RndModeChoice->Enable( m_RndPlayChkBox->IsChecked() );
	m_RndModeChoice->SetSelection( m_Config->ReadNum( wxT( "RndModeOnEmptyPlayList" ), 0, wxT( "General" ) ) );
	RandomPlaySizer->Add( m_RndModeChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	wxStaticText * RndTextStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, wxT("when playlist is empty"), wxDefaultPosition, wxDefaultSize, 0 );
	RndTextStaticText->Wrap( -1 );
	RandomPlaySizer->Add( RndTextStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	PlayGenSizer->Add( RandomPlaySizer, 1, wxEXPAND, 5 );

	m_DelPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, wxT("Delete played tracks from playlist"), wxDefaultPosition, wxDefaultSize, 0 );

	PlayGenSizer->Add( m_DelPlayChkBox, 0, wxALL, 5 );

	PlayMainSizer->Add( PlayGenSizer, 0, wxEXPAND|wxALL, 5 );



	wxStaticBoxSizer * PlaySilenceSizer;
	PlaySilenceSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _( " Silence detector " ) ), wxVERTICAL );

	m_PlayLevelEnabled = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Enabled" ), wxDefaultPosition, wxDefaultSize, 0 );
	bool IsPlayLevelEnabled = m_Config->ReadBool( wxT( "SilenceDetector" ), false, wxT( "Playback" ) );
    m_PlayLevelEnabled->SetValue( IsPlayLevelEnabled );

	PlaySilenceSizer->Add( m_PlayLevelEnabled, 0, wxALL|wxEXPAND, 5 );


	wxStaticBoxSizer*   SmartPlayListSizer;
	SmartPlayListSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _( " Random / Smart Play Modes " ) ), wxVERTICAL );

	SmartPlayListFlexGridSizer = new wxFlexGridSizer( 4, 2, 0, 0 );
	SmartPlayListFlexGridSizer->SetFlexibleDirection( wxBOTH );
	SmartPlayListFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_MinTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 10, 4 );
    m_MinTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) ) );
	SmartPlayListFlexGridSizer->Add( m_MinTracksSpinCtrl, 0, wxALL|wxALIGN_RIGHT, 5 );

	wxStaticText * MinTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Tracks left to start search"), wxDefaultPosition, wxDefaultSize, 0 );
	MinTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( MinTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_NumTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 10, 3 );
    m_NumTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "NumTracksToAdd" ), 3, wxT( "Playback" ) ) );
	SmartPlayListFlexGridSizer->Add( m_NumTracksSpinCtrl, 0, wxALL|wxALIGN_RIGHT, 5 );

	wxStaticText * AddTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Tracks added each time"), wxDefaultPosition, wxDefaultSize, 0 );
	AddTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( AddTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_MaxTracksPlayed = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 999, 20 );
    m_MaxTracksPlayed->SetValue( m_Config->ReadNum( wxT( "MaxTracksPlayed" ), 20, wxT( "Playback" ) ) );
    m_MaxTracksPlayed->Enable( !m_DelPlayChkBox->IsChecked() );
	SmartPlayListFlexGridSizer->Add( m_MaxTracksPlayed, 0, wxALL|wxALIGN_RIGHT, 5 );

	wxStaticText * MaxTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Max played tracks kept in playlist"), wxDefaultPosition, wxDefaultSize, 0 );
	MaxTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( MaxTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	SmartPlayListSizer->Add( SmartPlayListFlexGridSizer, 1, wxEXPAND, 5 );

	PlayMainSizer->Add( SmartPlayListSizer, 0, wxALL|wxEXPAND, 5 );


	wxBoxSizer* PlayLevelSizer;
	PlayLevelSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * LevelStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _( "Level(db):" ), wxDefaultPosition, wxDefaultSize, 0 );
	LevelStaticText->Wrap( -1 );

	PlayLevelSizer->Add( LevelStaticText, 0, wxALL|wxALIGN_BOTTOM, 5 );

	m_PlayLevelSlider = new wxSlider( m_PlayPanel, wxID_ANY, m_Config->ReadNum( wxT( "SilenceLevel" ), -50, wxT( "Playback" ) ), -65, 0, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_PlayLevelSlider->Enable( IsPlayLevelEnabled );

	PlayLevelSizer->Add( m_PlayLevelSlider, 1, wxEXPAND|wxALIGN_BOTTOM|wxBOTTOM, 5 );

	PlaySilenceSizer->Add( PlayLevelSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* PlayEndTimeSizer;
	PlayEndTimeSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayEndTimeCheckBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Only in the last" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_PlayEndTimeCheckBox->SetValue( m_Config->ReadBool( wxT( "SilenceAtEnd" ), true, wxT( "Playback" ) ) );
	m_PlayEndTimeCheckBox->Enable( IsPlayLevelEnabled );

	PlayEndTimeSizer->Add( m_PlayEndTimeCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_PlayEndTimeSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 360,
	    m_Config->ReadNum( wxT( "SilenceEndTime" ), 45, wxT( "Playback" ) ) );
	m_PlayEndTimeSpinCtrl->Enable( IsPlayLevelEnabled );

	PlayEndTimeSizer->Add( m_PlayEndTimeSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * PlayEndTimeStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _( "seconds" ), wxDefaultPosition, wxDefaultSize, 0 );
	PlayEndTimeStaticText->Wrap( -1 );

	PlayEndTimeSizer->Add( PlayEndTimeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	PlaySilenceSizer->Add( PlayEndTimeSizer, 0, wxEXPAND, 5 );

	PlayMainSizer->Add( PlaySilenceSizer, 0, wxEXPAND|wxALL, 5 );


	m_PlayPanel->SetSizer( PlayMainSizer );
	m_PlayPanel->Layout();
	PlayMainSizer->Fit( m_PlayPanel );
	m_MainNotebook->AddPage( m_PlayPanel, _( "Playback" ), false );
	m_MainNotebook->SetPageImage( 2, 2 );


    //
    // LastFM Panel
    //
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
	m_PasswdTextCtrl->SetValue( m_Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "LastFM" ) ).IsEmpty() ? wxEmptyString : wxT( "******" ) );
	// Password is saved in md5 form so we cant load it back
	ASLoginSizer->Add( m_PasswdTextCtrl, 0, wxALL, 5 );

	if( m_PasswdTextCtrl->IsEmpty() ||
	    m_UserNameTextCtrl->IsEmpty() )
        m_ASEnableChkBox->Disable();

	LastFMASSizer->Add( ASLoginSizer, 1, wxEXPAND, 5 );

	ASMainSizer->Add( LastFMASSizer, 0, wxEXPAND|wxALL, 5 );

	ASMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxHyperlinkCtrl * m_UserGroupLink = new wxHyperlinkCtrl( m_LastFMPanel, wxID_ANY,
        _( "Join the Guayadeque Last.fm users group" ), wxT("http://www.last.fm/group/Guayadeque"),
        wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	ASMainSizer->Add( m_UserGroupLink, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LastFMPanel->SetSizer( ASMainSizer );
	m_LastFMPanel->Layout();
	ASMainSizer->Fit( m_LastFMPanel );
	m_MainNotebook->AddPage( m_LastFMPanel, wxT( "LastFM" ), false );
	m_MainNotebook->SetPageImage( 3, 3 );

    //
    // Online Services Filter
    //
	m_OnlinePanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* OnlineMainSizer;
	OnlineMainSizer = new wxBoxSizer( wxVERTICAL );

	OnlineFiltersSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _(" Filters ") ), wxHORIZONTAL );

	m_OnlineFiltersListBox = new wxListBox( m_OnlinePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_OnlineFiltersListBox->Append( m_Config->ReadAStr( wxT( "Filter" ), wxEmptyString, wxT( "SearchFilters" ) ) );
	OnlineFiltersSizer->Add( m_OnlineFiltersListBox, 1, wxALL|wxEXPAND, 5 );

	OnlineBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_OnlineAddBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
	OnlineBtnSizer->Add( m_OnlineAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_OnlineDelBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
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
	m_MainNotebook->AddPage( m_OnlinePanel, _( "Online" ), false );
	m_MainNotebook->SetPageImage( 4, 4 );

    //
    // Podcasts
    //
	PodcastPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* PodcastsMainSizer;
	PodcastsMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* PodcastsSizer;
	PodcastsSizer = new wxStaticBoxSizer( new wxStaticBox( PodcastPanel, wxID_ANY, _(" Podcasts ") ), wxVERTICAL );

	wxBoxSizer* PathSizer;
	PathSizer = new wxBoxSizer( wxHORIZONTAL );

	PodcastPathStaticText = new wxStaticText( PodcastPanel, wxID_ANY, _("Destination directory:"), wxDefaultPosition, wxDefaultSize, 0 );
	PodcastPathStaticText->Wrap( -1 );
	PathSizer->Add( PodcastPathStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	m_PodcastPath = new wxDirPickerCtrl( PodcastPanel, wxID_ANY, m_Config->ReadStr( wxT( "Path" ), wxGetHomeDir(), wxT( "Podcasts" ) ), _("Select podcasts folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST );
	PathSizer->Add( m_PodcastPath, 1, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

	PodcastsSizer->Add( PathSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* UpdateSizer;
	UpdateSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PodcastUpdate = new wxCheckBox( PodcastPanel, wxID_ANY, _("Check new podcasts every"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PodcastUpdate->SetValue( m_Config->ReadBool( wxT( "Update" ), true, wxT( "Podcasts" ) ) );

	UpdateSizer->Add( m_PodcastUpdate, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxString m_PodcastUpdatePeriodChoices[] = { _( "Hour" ), _("Day"), _("Week"), _("Month") };
	int m_PodcastUpdatePeriodNChoices = sizeof( m_PodcastUpdatePeriodChoices ) / sizeof( wxString );
	m_PodcastUpdatePeriod = new wxChoice( PodcastPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PodcastUpdatePeriodNChoices, m_PodcastUpdatePeriodChoices, 0 );
	m_PodcastUpdatePeriod->SetSelection( m_Config->ReadNum( wxT( "UpdatePeriod" ), 0, wxT( "Podcasts" ) ) );
	UpdateSizer->Add( m_PodcastUpdatePeriod, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PodcastsSizer->Add( UpdateSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* DeleteSizer;
	DeleteSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PodcastDelete = new wxCheckBox( PodcastPanel, wxID_ANY, _("Delete podcasts after"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PodcastDelete->SetValue( m_Config->ReadBool( wxT( "Delete" ), false, wxT( "Podcasts" ) ) );

	DeleteSizer->Add( m_PodcastDelete, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_PodcastDeleteTime = new wxSpinCtrl( PodcastPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 99,
        m_Config->ReadNum( wxT( "DeleteTime" ), 15, wxT( "Podcasts" ) ) );
	DeleteSizer->Add( m_PodcastDeleteTime, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxString m_PodcastDeletePeriodChoices[] = { _("Days"), _("Weeks"), _("Months") };
	int m_PodcastDeletePeriodNChoices = sizeof( m_PodcastDeletePeriodChoices ) / sizeof( wxString );
	m_PodcastDeletePeriod = new wxChoice( PodcastPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PodcastDeletePeriodNChoices, m_PodcastDeletePeriodChoices, 0 );
	m_PodcastDeletePeriod->SetSelection( m_Config->ReadNum( wxT( "DeletePeriod" ), 0, wxT( "Podcasts" ) ) );
	DeleteSizer->Add( m_PodcastDeletePeriod, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DeleteSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_PodcastDeletePlayed = new wxCheckBox( PodcastPanel, wxID_ANY, _("Only if played"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PodcastDeletePlayed->SetValue( m_Config->ReadBool( wxT( "DeletePlayed" ), false, wxT( "Podcasts" ) ) );

	DeleteSizer->Add( m_PodcastDeletePlayed, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	PodcastsSizer->Add( DeleteSizer, 0, wxEXPAND, 5 );

	PodcastsMainSizer->Add( PodcastsSizer, 0, wxEXPAND|wxALL, 5 );

	PodcastPanel->SetSizer( PodcastsMainSizer );
	PodcastPanel->Layout();
	PodcastsMainSizer->Fit( PodcastPanel );
	m_MainNotebook->AddPage( PodcastPanel, wxT("Podcasts"), false );
	m_MainNotebook->SetPageImage( 5, 5 );

    //
    // Links
    //
	m_LinksPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* LinksMainSizer;
	LinksMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* LinksLabelSizer;
	LinksLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Links ") ), wxVERTICAL );

	wxBoxSizer* LinksListBoxSizer;
	LinksListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_LinksListBox = new wxListBox( m_LinksPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    wxArrayString SearchLinks = m_Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );
	m_LinksListBox->Append( SearchLinks );
	m_LinksNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "SearchLinks" ) );
    int count = m_LinksListBox->GetCount();
	while( ( int ) m_LinksNames.Count() < count )
        m_LinksNames.Add( wxEmptyString );
    int index;
    for( index = 0; index < count; index++ )
    {
        if( m_LinksNames[ index ].IsEmpty() )
        {
            wxURI Uri( SearchLinks[ index ] );
            m_LinksNames[ index ] = Uri.GetServer();
        }
    }

	LinksListBoxSizer->Add( m_LinksListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* LinksBtnSizer;
	LinksBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LinksAddBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
    m_LinksAddBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveUpBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveUpBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveUpBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveDownBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveDownBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveDownBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksDelBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
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

	m_LinksAcceptBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
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
	m_MainNotebook->SetPageImage( 6, 6 );


    //
    // Commands Panel
    //
    wxStaticText * CmdStaticText;
    wxStaticText * CmdNameStaticText;
    wxStaticText * CmdHelpText;

	m_CmdPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* CmdMainSizer;
	CmdMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* CmdLabelSizer;
	CmdLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_CmdPanel, wxID_ANY, _(" Commands ") ), wxVERTICAL );

	wxBoxSizer* CmdListBoxSizer;
	CmdListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CmdListBox = new wxListBox( m_CmdPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    wxArrayString Commands = m_Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
	m_CmdListBox->Append( Commands );
	m_CmdNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "Commands" ) );
    count = m_CmdListBox->GetCount();
	while( ( int ) m_CmdNames.Count() < count )
        m_CmdNames.Add( wxEmptyString );
    if( ( int ) m_CmdNames.Count() > count )
        m_CmdNames.RemoveAt( count, m_CmdNames.Count() - count );
    for( index = 0; index < count; index++ )
    {
        if( m_CmdNames[ index ].IsEmpty() )
        {
            m_CmdNames[ index ] = Commands[ index ].BeforeFirst( ' ' );
        }
    }

	CmdListBoxSizer->Add( m_CmdListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* CmdBtnSizer;
	CmdBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_CmdAddBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
	CmdBtnSizer->Add( m_CmdAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdMoveUpBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdMoveUpBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdMoveUpBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdMoveDownBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdMoveDownBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdMoveDownBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdDelBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdDelBtn->Enable( false );

	CmdBtnSizer->Add( m_CmdDelBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CmdListBoxSizer->Add( CmdBtnSizer, 0, wxEXPAND, 5 );

	CmdLabelSizer->Add( CmdListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer* CmdEditorSizer;
	CmdEditorSizer = new wxBoxSizer( wxHORIZONTAL );

	wxFlexGridSizer* CmdFieldsSizer;
	CmdFieldsSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	CmdFieldsSizer->AddGrowableCol( 1 );
	CmdFieldsSizer->SetFlexibleDirection( wxBOTH );
	CmdFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	CmdStaticText = new wxStaticText( m_CmdPanel, wxID_ANY, _( "Command:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CmdStaticText->Wrap( -1 );
	CmdFieldsSizer->Add( CmdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_CmdTextCtrl = new wxTextCtrl( m_CmdPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CmdFieldsSizer->Add( m_CmdTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	CmdNameStaticText = new wxStaticText( m_CmdPanel, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CmdNameStaticText->Wrap( -1 );
	CmdFieldsSizer->Add( CmdNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_CmdNameTextCtrl = new wxTextCtrl( m_CmdPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CmdFieldsSizer->Add( m_CmdNameTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	CmdEditorSizer->Add( CmdFieldsSizer, 1, wxEXPAND, 5 );

	m_CmdAcceptBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdAcceptBtn->Enable( false );

	CmdEditorSizer->Add( m_CmdAcceptBtn, 0, wxALL, 5 );

	CmdLabelSizer->Add( CmdEditorSizer, 0, wxEXPAND, 5 );

	CmdMainSizer->Add( CmdLabelSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer* CmdHelpSizer;
	CmdHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_CmdPanel, wxID_ANY, _(" Help ") ), wxVERTICAL );

	CmdHelpText = new wxStaticText( m_CmdPanel, wxID_ANY, _("Add commands using :\n{bp}\t: Album path\n{bc}\t: Album cover file path\n{tp}\t: Track path"), wxDefaultPosition, wxDefaultSize, 0 );
	CmdHelpText->Wrap( -1 );
	CmdHelpSizer->Add( CmdHelpText, 0, wxALL, 5 );

	CmdMainSizer->Add( CmdHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_CmdPanel->SetSizer( CmdMainSizer );
	m_CmdPanel->Layout();
	CmdMainSizer->Fit( m_CmdPanel );
	m_MainNotebook->AddPage( m_CmdPanel, _( "Commands" ), false );
	m_MainNotebook->SetPageImage( 7, 7 );


    //
    // Copy To Panel
    //
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

	CopyToMainSizer->Add( CopyToHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_CopyPanel->SetSizer( CopyToMainSizer );
	m_CopyPanel->Layout();
	CopyToMainSizer->Fit( m_CopyPanel );
	m_MainNotebook->AddPage( m_CopyPanel, _( "Copy To" ), false );
	m_MainNotebook->SetPageImage( 8, 8 );


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
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    //
    //
    //
    m_PathSelected = wxNOT_FOUND;
    m_FilterSelected = wxNOT_FOUND;

	m_TaskIconChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateTaskBarIcon ), NULL, this );

	m_PathsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxSelected ), NULL, this );
	m_AddPathButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddPathBtnClick ), NULL, this );
	m_DelPathButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPathBtnClick ), NULL, this );
	m_PathsListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxDClicked ), NULL, this );

	m_RndPlayChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRndPlayClicked ), NULL, this );
	m_DelPlayChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPlayedTracksChecked ), NULL, this );
	m_PlayLevelEnabled->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayLevelEnabled ), NULL, this );
	m_PlayEndTimeCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayEndTimeEnabled ), NULL, this );

	m_CoversListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCoversListBoxSelected ), NULL, this );
	m_AddCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddCoverBtnClick ), NULL, this );
	m_UpCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnUpCoverBtnClick ), NULL, this );
	m_DownCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDownCoverBtnClick ), NULL, this );
	m_DelCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelCoverBtnClick ), NULL, this );
	m_CoversListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnCoverListBoxDClicked ), NULL, this );

	m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
	m_OnlineAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
	m_OnlineDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
	m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );

    m_UserNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnASUserNameChanged ), NULL, this );
    m_PasswdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnASUserNameChanged ), NULL, this );

	m_LinksListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
	m_LinksAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
	m_LinksDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
	m_LinksMoveUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
	m_LinksMoveDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
	m_LinksUrlTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );

	m_CmdListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCmdListBoxSelected ), NULL, this );
	m_CmdAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdAddBtnClick ), NULL, this );
	m_CmdDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdDelBtnClick ), NULL, this );
	m_CmdMoveUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveUpBtnClick ), NULL, this );
	m_CmdMoveDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveDownBtnClick ), NULL, this );
	m_CmdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdSaveBtnClick ), NULL, this );

	m_CopyToFileName->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToFileNameUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guPrefDialog::~guPrefDialog()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    // Save the window position and size
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( wxT( "PreferencesPosX" ), WindowPos.x, wxT( "Positions" ) );
    Config->WriteNum( wxT( "PreferencesPosY" ), WindowPos.y, wxT( "Positions" ) );
    wxSize WindowSize = GetSize();
    Config->WriteNum( wxT( "PreferencesSizeWidth" ), WindowSize.x, wxT( "Positions" ) );
    Config->WriteNum( wxT( "PreferencesSizeHeight" ), WindowSize.y, wxT( "Positions" ) );

    //
	m_TaskIconChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateTaskBarIcon ), NULL, this );

	m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxSelected ), NULL, this );
	m_AddPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddPathBtnClick ), NULL, this );
	m_DelPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPathBtnClick ), NULL, this );
	m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxDClicked ), NULL, this );

	m_RndPlayChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRndPlayClicked ), NULL, this );
	m_DelPlayChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPlayedTracksChecked ), NULL, this );
	m_PlayLevelEnabled->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayLevelEnabled ), NULL, this );
	m_PlayEndTimeCheckBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayEndTimeEnabled ), NULL, this );

	m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCoversListBoxSelected ), NULL, this );
	m_AddCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddCoverBtnClick ), NULL, this );
	m_UpCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnUpCoverBtnClick ), NULL, this );
	m_DownCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDownCoverBtnClick ), NULL, this );
	m_DelCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelCoverBtnClick ), NULL, this );
	m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnCoverListBoxDClicked ), NULL, this );

	m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
	m_OnlineAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
	m_OnlineDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
	m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );

    m_UserNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnASUserNameChanged ), NULL, this );
    m_PasswdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnASUserNameChanged ), NULL, this );

	m_LinksListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
	m_LinksAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
	m_LinksDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
	m_LinksMoveUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
	m_LinksMoveDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
	m_LinksUrlTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );

	m_CmdListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCmdListBoxSelected ), NULL, this );
	m_CmdAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdAddBtnClick ), NULL, this );
	m_CmdDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdDelBtnClick ), NULL, this );
	m_CmdMoveUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveUpBtnClick ), NULL, this );
	m_CmdMoveDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveDownBtnClick ), NULL, this );
	m_CmdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdSaveBtnClick ), NULL, this );

	m_CopyToFileName->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToFileNameUpdated ), NULL, this );
}


// -------------------------------------------------------------------------------- //
void guPrefDialog::SaveSettings( void )
{
    m_Config = ( guConfig * ) guConfig::Get();
    if( !m_Config )
        guLogError( wxT( "Invalid m_Config object in preferences dialog" ) );


    // Save all configurations
    m_Config->WriteBool( wxT( "ShowSplashScreen" ), m_ShowSplashChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "StartMinimized" ), m_MinStartChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "LoadDefaultLayouts" ), m_IgnoreLayoutsChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "ShowTaskBarIcon" ), m_TaskIconChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "DefaultActionEnqueue" ), m_EnqueueChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "DropFilesClearPlaylist" ), m_DropFilesChkBox->GetValue(), wxT( "General" ) );
    //m_Config->WriteNum( wxT( "AlbumYearOrder" ), m_AlYearOrderChoice->GetSelection(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "SavePlayListOnClose" ), m_SavePlayListChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "SaveCurrentTrackPos" ), m_SavePosCheckBox->GetValue(), wxT( "General" ) );
    m_Config->WriteNum( wxT( "MinSavePlayPosLength" ), m_MinLenSpinCtrl->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "CloseToTaskBar" ), m_TaskIconChkBox->IsChecked() && m_CloseTaskBarChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "ShowCloseConfirm" ), m_ExitConfirmChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteAStr( wxT( "LibPath" ), m_PathsListBox->GetStrings(), wxT( "LibPaths" ) );
    if( m_LibPathsChanged )
    {
        m_Config->WriteStr( wxT( "LastUpdate" ), wxEmptyString, wxT( "General" ) );
    }

    m_Config->WriteAStr( wxT( "Word" ), m_CoversListBox->GetStrings(), wxT( "CoverSearch" ) );
    m_Config->WriteBool( wxT( "UpdateLibOnStart" ), m_UpdateLibChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "SaveLyricsToFiles" ), m_SaveLyricsChkBox->GetValue(), wxT( "General" ) );

    m_Config->WriteBool( wxT( "RndPlayOnEmptyPlayList" ), m_RndPlayChkBox->GetValue(), wxT( "General" ) );
    m_Config->WriteNum( wxT( "RndModeOnEmptyPlayList" ), m_RndModeChoice->GetSelection(), wxT( "General" ) );
    m_Config->WriteBool( wxT( "DelTracksPlayed" ), m_DelPlayChkBox->GetValue(), wxT( "Playback" ) );

    m_Config->WriteBool( wxT( "SilenceDetector" ), m_PlayLevelEnabled->GetValue(), wxT( "Playback" ) );
    m_Config->WriteNum( wxT( "SilenceLevel" ), m_PlayLevelSlider->GetValue(), wxT( "Playback" ) );
    m_Config->WriteBool( wxT( "SilenceAtEnd" ), m_PlayEndTimeCheckBox->GetValue(), wxT( "Playback" ) );
    m_Config->WriteNum( wxT( "SilenceEndTime" ), m_PlayEndTimeSpinCtrl->GetValue(), wxT( "Playback" ) );


    m_Config->WriteBool( wxT( "SubmitEnabled" ), m_ASEnableChkBox->IsEnabled() && m_ASEnableChkBox->GetValue(), wxT( "LastFM" ) );
    m_Config->WriteStr( wxT( "UserName" ), m_UserNameTextCtrl->GetValue(), wxT( "LastFM" ) );
    if( !m_PasswdTextCtrl->IsEmpty() && m_PasswdTextCtrl->GetValue() != wxT( "******" ) )
    {
        guMD5 MD5;
        m_Config->WriteStr( wxT( "Password" ), MD5.MD5( m_PasswdTextCtrl->GetValue() ), wxT( "LastFM" ) );
        //guLogMessage( wxT( "Pass: %s" ), PasswdTextCtrl->GetValue().c_str() );
        //guLogMessage( wxT( "MD5 : %s" ), MD5.MD5( PasswdTextCtrl->GetValue() ).c_str() );
    }
    // LastFM Panel Info language
    m_Config->WriteStr( wxT( "Language" ), m_LangIds[ m_LangChoice->GetSelection() ], wxT( "LastFM" ) );
    m_Config->WriteNum( wxT( "MinTracksToPlay" ), m_MinTracksSpinCtrl->GetValue(), wxT( "Playback" ) );
    m_Config->WriteNum( wxT( "NumTracksToAdd" ), m_NumTracksSpinCtrl->GetValue(), wxT( "Playback" ) );
    m_Config->WriteNum( wxT( "MaxTracksPlayed" ), m_MaxTracksPlayed->GetValue(), wxT( "Playback" ) );
    m_Config->WriteAStr( wxT( "Filter" ), m_OnlineFiltersListBox->GetStrings(), wxT( "SearchFilters" ) );
    m_Config->WriteStr( wxT( "BrowserCommand" ), m_BrowserCmdTextCtrl->GetValue(), wxT( "General" ) );
    m_Config->WriteStr( wxT( "RadioMinBitRate" ), m_RadioMinBitRateRadBoxChoices[ m_RadioMinBitRateRadBox->GetSelection() ], wxT( "Radios" ) );
//    m_Config->WriteNum( wxT( "LyricSearchEngine" ), m_LyricsChoice->GetSelection(), wxT( "General" ) );
    m_Config->WriteStr( wxT( "Path" ), m_PodcastPath->GetPath(), wxT( "Podcasts" ) );
    m_Config->WriteBool( wxT( "Update" ), m_PodcastUpdate->GetValue(), wxT( "Podcasts" ) );
    m_Config->WriteNum( wxT( "UpdatePeriod" ), m_PodcastUpdatePeriod->GetSelection(), wxT( "Podcasts" ) );
    m_Config->WriteBool( wxT( "Delete" ), m_PodcastDelete->GetValue(), wxT( "Podcasts" ) );
    m_Config->WriteNum( wxT( "DeleteTime" ), m_PodcastDeleteTime->GetValue(), wxT( "Podcasts" ) );
    m_Config->WriteNum( wxT( "DeletePeriod" ), m_PodcastDeletePeriod->GetSelection(), wxT( "Podcasts" ) );
    m_Config->WriteBool( wxT( "DeletePlayed" ), m_PodcastDeletePlayed->GetValue(), wxT( "Podcasts" ) );

    wxArrayString SearchLinks = m_LinksListBox->GetStrings();
    m_Config->WriteAStr( wxT( "Link" ), SearchLinks, wxT( "SearchLinks" ) );
    m_Config->WriteAStr( wxT( "Name" ), m_LinksNames, wxT( "SearchLinks" ), false );
    wxArrayString Commands = m_CmdListBox->GetStrings();
    m_Config->WriteAStr( wxT( "Cmd" ), Commands, wxT( "Commands" ) );
    m_Config->WriteAStr( wxT( "Name" ), m_CmdNames, wxT( "Commands" ), false );
    m_Config->WriteStr( wxT( "CopyToPattern" ), m_CopyToFileName->GetValue(), wxT( "General" ) );

    // TODO : Make this process in a thread
    int index;
    int count = SearchLinks.Count();
    for( index = 0; index < count; index++ )
    {
        wxURI Uri( SearchLinks[ index ] );
        if( !wxDirExists( wxGetHomeDir() + wxT( "/.guayadeque/LinkIcons/" ) ) )
        {
            wxMkdir( wxGetHomeDir() + wxT( "/.guayadeque/LinkIcons" ), 0770 );
        }
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
    m_Config->Flush();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnActivateTaskBarIcon( wxCommandEvent& event )
{
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnRndPlayClicked( wxCommandEvent& event )
{
    m_RndModeChoice->Enable( m_RndPlayChkBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnDelPlayedTracksChecked( wxCommandEvent& event )
{
    m_MaxTracksPlayed->Enable( !m_DelPlayChkBox->IsChecked() );
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
                m_PathsListBox->Append( DirDialog->GetPath() + wxT( "/" ) );
                m_LibPathsChanged = true;
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
        m_LibPathsChanged = true;
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
                    m_PathsListBox->SetString( index, DirDialog->GetPath() + wxT( "/" ) );
                    m_LibPathsChanged = true;
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
        m_UpCoverButton->Enable( m_CoverSelected > 0 );
        m_DownCoverButton->Enable( m_CoverSelected < int( m_CoversListBox->GetCount() - 1 ) );
        m_DelCoverButton->Enable();
    }
    else
    {
        m_UpCoverButton->Disable();
        m_DownCoverButton->Disable();
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
void guPrefDialog::OnUpCoverBtnClick( wxCommandEvent& event )
{
    wxString CoverWord = m_CoversListBox->GetString( m_CoverSelected );
    m_CoversListBox->SetString( m_CoverSelected, m_CoversListBox->GetString( m_CoverSelected - 1 ) );
    m_CoverSelected--;
    m_CoversListBox->SetString( m_CoverSelected, CoverWord );
    m_CoversListBox->SetSelection( m_CoverSelected );

    event.SetInt( m_CoverSelected );
    OnCoversListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnDownCoverBtnClick( wxCommandEvent& event )
{
    wxString CoverWord = m_CoversListBox->GetString( m_CoverSelected );
    m_CoversListBox->SetString( m_CoverSelected, m_CoversListBox->GetString( m_CoverSelected + 1 ) );
    m_CoverSelected++;
    m_CoversListBox->SetString( m_CoverSelected, CoverWord );
    m_CoversListBox->SetSelection( m_CoverSelected );

    event.SetInt( m_CoverSelected );
    OnCoversListBoxSelected( event );
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
void guPrefDialog::OnPlayLevelEnabled( wxCommandEvent& event )
{
	m_PlayLevelSlider->Enable( event.IsChecked() );
	m_PlayEndTimeCheckBox->Enable( event.IsChecked() );
	m_PlayEndTimeSpinCtrl->Enable( event.IsChecked() && m_PlayEndTimeCheckBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayEndTimeEnabled( wxCommandEvent& event )
{
	m_PlayEndTimeSpinCtrl->Enable( event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnASUserNameChanged( wxCommandEvent &event )
{
    if( m_UserNameTextCtrl->IsEmpty() || m_PasswdTextCtrl->IsEmpty() )
    {
        m_ASEnableChkBox->Disable();
    }
    else
    {
        m_ASEnableChkBox->Enable();
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
        m_LinksNames.RemoveAt( m_LinkSelected );
        m_LinksListBox->Delete( m_LinkSelected );
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
    if( !m_LinksUrlTextCtrl->IsEmpty() )
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
    if( m_LinksNames[ m_LinkSelected ].IsEmpty() )
    {
        wxURI Uri( m_LinksUrlTextCtrl->GetValue() );
        m_LinksNames[ m_LinkSelected ] = Uri.GetServer();
        m_LinksNameTextCtrl->SetValue( Uri.GetServer() );
    }
    m_LinksAcceptBtn->Disable();
}


// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdListBoxSelected( wxCommandEvent &event )
{
    m_CmdSelected = event.GetInt();
    if( m_CmdSelected != wxNOT_FOUND )
    {
        m_CmdDelBtn->Enable();

        if( m_CmdSelected > 0 )
            m_CmdMoveUpBtn->Enable();
        else
            m_CmdMoveUpBtn->Disable();

        if( m_CmdSelected < ( int ) ( m_CmdListBox->GetCount() - 1 ) )
            m_CmdMoveDownBtn->Enable();
        else
            m_CmdMoveDownBtn->Disable();

        m_CmdTextCtrl->SetValue( m_CmdListBox->GetString( m_CmdSelected ) );
        m_CmdNameTextCtrl->SetValue( m_CmdNames[ m_CmdSelected ] );
        m_CmdAcceptBtn->Disable();
    }
    else
    {
        m_CmdDelBtn->Disable();
        m_CmdMoveUpBtn->Disable();
        m_CmdMoveDownBtn->Disable();
        m_CmdAcceptBtn->Disable();
        m_CmdTextCtrl->SetValue( wxEmptyString );
        m_CmdNameTextCtrl->SetValue( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdAddBtnClick( wxCommandEvent& event )
{
    wxString Cmd = m_CmdTextCtrl->GetValue();
    if( !Cmd.IsEmpty() )
    {
        m_CmdListBox->Append( m_CmdTextCtrl->GetValue() );
        m_CmdNames.Add( m_CmdNameTextCtrl->GetValue() );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdDelBtnClick( wxCommandEvent& event )
{
    if( m_CmdSelected != wxNOT_FOUND )
    {
        m_CmdNames.RemoveAt( m_CmdSelected );
        m_CmdListBox->Delete( m_CmdSelected );
        m_CmdSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdMoveUpBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_CmdListBox->GetString( m_CmdSelected );
    wxString CurName = m_CmdNames[ m_CmdSelected ];
    m_CmdListBox->SetString( m_CmdSelected, m_CmdListBox->GetString( m_CmdSelected - 1 ) );
    m_CmdNames[ m_CmdSelected ] = m_CmdNames[ m_CmdSelected - 1 ];
    m_CmdSelected--;
    m_CmdListBox->SetString( m_CmdSelected, CurUrl );
    m_CmdNames[ m_CmdSelected ] = CurName;
    m_CmdListBox->SetSelection( m_CmdSelected );

    event.SetInt( m_CmdSelected );
    OnCmdListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdMoveDownBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_CmdListBox->GetString( m_CmdSelected );
    wxString CurName = m_CmdNames[ m_CmdSelected ];
    m_CmdListBox->SetString( m_CmdSelected, m_CmdListBox->GetString( m_CmdSelected + 1 ) );
    m_CmdNames[ m_CmdSelected ] = m_CmdNames[ m_CmdSelected + 1 ];
    m_CmdSelected++;
    m_CmdListBox->SetString( m_CmdSelected, CurUrl );
    m_CmdNames[ m_CmdSelected ] = CurName;
    m_CmdListBox->SetSelection( m_CmdSelected );

    event.SetInt( m_CmdSelected );
    OnCmdListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdTextChanged( wxCommandEvent &event )
{
    if( !m_CmdTextCtrl->IsEmpty() )
    {
        m_CmdAddBtn->Enable();
        if( m_CmdSelected != wxNOT_FOUND )
            m_CmdAcceptBtn->Enable();
    }
    else
    {
        m_CmdAddBtn->Disable();
        if( m_CmdSelected != wxNOT_FOUND )
            m_CmdAcceptBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdSaveBtnClick( wxCommandEvent &event )
{
    m_CmdListBox->SetString( m_CmdSelected, m_CmdTextCtrl->GetValue() );
    m_CmdNames[ m_CmdSelected ] = m_CmdNameTextCtrl->GetValue();
    if( m_CmdNames[ m_CmdSelected ].IsEmpty() )
    {
        m_CmdNames[ m_CmdSelected ] = m_CmdTextCtrl->GetValue().BeforeFirst( ' ' );
        m_CmdNameTextCtrl->SetValue( m_CmdNames[ m_CmdSelected ] );
    }
    m_CmdAcceptBtn->Disable();
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
