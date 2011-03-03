// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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

#include "Accelerators.h"
#include "Images.h"
#include "MD5.h"
#include "TagInfo.h"
#include "Transcode.h"
#include "Utils.h"
#include "MediaCtrl.h"

#include <wx/statline.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/arrimpl.cpp>

#include <id3v1genres.h>

WX_DEFINE_OBJARRAY( guCopyToPatternArray );

// -------------------------------------------------------------------------------- //
// guCopyToPattern
// -------------------------------------------------------------------------------- //
guCopyToPattern::guCopyToPattern()
{
    m_Format = guTRANSCODE_FORMAT_KEEP;
    m_Quality = guTRANSCODE_QUALITY_KEEP;
    m_MoveFiles = false;
}

// -------------------------------------------------------------------------------- //
guCopyToPattern::guCopyToPattern( const wxString &pattern )
{
    int Index;
    int Count;
    // Default:{g}/{a}/{b}/{n} - {a} - {t}:0:4:0
    m_Format = guTRANSCODE_FORMAT_KEEP;
    m_Quality = guTRANSCODE_QUALITY_KEEP;
    m_MoveFiles = false;
    wxArrayString Fields = wxStringTokenize( pattern, wxT( ":" ) );
    if( ( Count = Fields.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            switch( Index )
            {
                case 0 : m_Name    = unescape_configlist_str( Fields[ Index ] ); break;
                case 1 : m_Pattern = unescape_configlist_str( Fields[ Index ] ); break;
                case 2 : m_Format  = wxAtoi( Fields[ Index ] ); break;
                case 3 : m_Quality = wxAtoi( Fields[ Index ] ); break;
                case 4 : m_MoveFiles = wxAtoi( Fields[ Index ] ); break;
                default :
                    return;
            }
        }
    }
}


// -------------------------------------------------------------------------------- //
guCopyToPattern::~guCopyToPattern()
{
}

// -------------------------------------------------------------------------------- //
wxString guCopyToPattern::ToString( void )
{
    return wxString::Format( wxT( "%s:%s:%i:%i:%i" ),
        escape_configlist_str( m_Name ).c_str(), escape_configlist_str( m_Pattern ).c_str(), m_Format, m_Quality, m_MoveFiles );
}

// -------------------------------------------------------------------------------- //
// guPrefDialog
// -------------------------------------------------------------------------------- //
guPrefDialog::guPrefDialog( wxWindow* parent, guDbLibrary * db, int pagenum ) //:wxDialog( parent, wxID_ANY, _( "Preferences" ), wxDefaultPosition, wxSize( 600, 530 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
	wxBoxSizer *        MainSizer;

    m_Db = db;
    m_LinkSelected = wxNOT_FOUND;
    m_CmdSelected = wxNOT_FOUND;
    m_CopyToSelected = wxNOT_FOUND;
    m_LibPathsChanged = false;
    m_VisiblePanels = 0;
    m_CopyToOptions = NULL;
    m_LyricSearchEngine = NULL;
    m_LyricSourceSelected = wxNOT_FOUND;

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
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_crossfader ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_record ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_last_fm ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_lyrics ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_online_services ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_podcasts ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_jamendo ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_magnatune ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_links ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_commands ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_copy_to ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_accelerators ) );
    m_MainNotebook->AssignImageList( m_ImageList );


	m_GenPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_GenPanel, _("General"), true, 0 );

	m_LibPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LibPanel, _("Library"), false );
	m_MainNotebook->SetPageImage( 1, 1 );

	m_PlayPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_PlayPanel, _( "Playback" ), false );
	m_MainNotebook->SetPageImage( 2, 2 );

	m_XFadePanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_XFadePanel, _( "Crossfader" ), false );
	m_MainNotebook->SetPageImage( 3, 3 );

	m_RecordPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_RecordPanel, _( "Record" ), false );
	m_MainNotebook->SetPageImage( 4, 4 );

	m_LastFMPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LastFMPanel, _( "AudioScrobble" ), false );
	m_MainNotebook->SetPageImage( 5, 5 );

	m_LyricsPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LyricsPanel, _( "Lyrics" ), false );
	m_MainNotebook->SetPageImage( 6, 6 );

	m_OnlinePanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_OnlinePanel, _( "Online" ), false );
	m_MainNotebook->SetPageImage( 7, 7 );

	m_PodcastPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_PodcastPanel, _("Podcasts"), false );
	m_MainNotebook->SetPageImage( 8, 8 );

	m_JamendoPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_JamendoPanel, wxT("Jamendo"), false );
	m_MainNotebook->SetPageImage( 9, 9 );

	m_MagnatunePanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_MagnatunePanel, wxT("Magnatune"), false );
	m_MainNotebook->SetPageImage( 10, 10 );

	m_LinksPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LinksPanel, _("Links"), false );
	m_MainNotebook->SetPageImage( 11, 11 );

	m_CmdPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_CmdPanel, _( "Commands" ), false );
	m_MainNotebook->SetPageImage( 12, 12 );

	m_CopyPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_CopyPanel, _( "Copy To" ), false );
	m_MainNotebook->SetPageImage( 13, 13 );

	m_AccelPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_AccelPanel, _( "Accelerators" ), false );
	m_MainNotebook->SetPageImage( 14, 14 );

    if( pagenum == guPREFERENCE_PAGE_LASTUSED )
    {
        pagenum = m_Config->ReadNum( wxT( "LasPreferencePage" ), guPREFERENCE_PAGE_GENERAL, wxT( "General" ) );
    }

    switch( pagenum )
    {
        case guPREFERENCE_PAGE_GENERAL :
            BuildGeneralPage();
            break;

        case guPREFERENCE_PAGE_LIBRARY :
            BuildLibraryPage();
            break;

        case guPREFERENCE_PAGE_PLAYBACK :
            BuildPlaybackPage();
            break;

        case guPREFERENCE_PAGE_CROSSFADER :
            BuildCrossfaderPage();
            break;

        case guPREFERENCE_PAGE_RECORD :
            BuildRecordPage();
            break;

        case guPREFERENCE_PAGE_AUDIOSCROBBLE :
            BuildAudioScrobblePage();
            break;

        case guPREFERENCE_PAGE_LYRICS :
            BuildLyricsPage();
            break;

        case guPREFERENCE_PAGE_ONLINE :
            BuildOnlinePage();
            break;

        case guPREFERENCE_PAGE_PODCASTS :
            BuildPodcastsPage();
            break;

        case guPREFERENCE_PAGE_JAMENDO :
            BuildJamendoPage();
            break;

        case guPREFERENCE_PAGE_MAGNATUNE :
            BuildMagnatunePage();
            break;

        case guPREFERENCE_PAGE_LINKS :
            BuildLinksPage();
            break;

        case guPREFERENCE_PAGE_COMMANDS :
            BuildCommandsPage();
            break;

        case guPREFERENCE_PAGE_COPYTO :
            BuildCopyToPage();
            break;

        case guPREFERENCE_PAGE_ACCELERATORS :
            BuildAcceleratorsPage();
            break;
    }

    m_MainNotebook->SetSelection( pagenum );

    //
	MainSizer->Add( m_MainNotebook, 1, wxEXPAND | wxALL, 5 );

    wxStdDialogButtonSizer *    ButtonsSizer;
    wxButton *                  ButtonsSizerOK;
    wxButton *                  ButtonsSizerCancel;

	ButtonsSizer = new wxStdDialogButtonSizer();
	ButtonsSizerOK = new wxButton( this, wxID_OK, _( " Accept " ) );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	ButtonsSizerCancel = new wxButton( this, wxID_CANCEL, _( " Cancel " ) );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	m_MainNotebook->Connect( wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED, wxCommandEventHandler( guPrefDialog::OnPageChanged ), NULL, this );

    //
    //
    //
    m_PathSelected = wxNOT_FOUND;
    m_FilterSelected = wxNOT_FOUND;

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
    m_Config->WriteNum( wxT( "LasPreferencePage" ), m_MainNotebook->GetSelection(), wxT( "General" ) );

    //
	m_MainNotebook->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxCommandEventHandler( guPrefDialog::OnPageChanged ), NULL, this );


    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_GENERAL )
    {
        m_TaskIconChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateTaskBarIcon ), NULL, this );
#ifdef WITH_LIBINDICATE_SUPPORT
        m_SoundMenuChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateSoundMenuIntegration ), NULL, this );
#endif
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LIBRARY )
    {
    	m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxSelected ), NULL, this );
        m_AddPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddPathBtnClick ), NULL, this );
        m_DelPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPathBtnClick ), NULL, this );
        m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxDClicked ), NULL, this );

        m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCoversListBoxSelected ), NULL, this );
        m_AddCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddCoverBtnClick ), NULL, this );
        m_UpCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnUpCoverBtnClick ), NULL, this );
        m_DownCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDownCoverBtnClick ), NULL, this );
        m_DelCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelCoverBtnClick ), NULL, this );
        m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnCoverListBoxDClicked ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PLAYBACK )
    {
        m_RndPlayChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRndPlayClicked ), NULL, this );
        m_DelPlayChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPlayedTracksChecked ), NULL, this );
        m_PlayLevelEnabled->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayLevelEnabled ), NULL, this );
        m_PlayLevelSlider->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnPlayLevelValueChanged ), NULL, this );
        m_PlayEndTimeCheckBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayEndTimeEnabled ), NULL, this );
        m_PlayOutDevChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnPlayOutDevChanged ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_RECORD )
    {
        m_RecordChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecEnableClicked ), NULL, this );
        m_RecDelTracks->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecDelTracksClicked ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LYRICS )
    {
        if( m_LyricSearchEngine )
        {
            delete m_LyricSearchEngine;
        }

        m_LyricsSrcListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSourceSelected ), NULL, this );
        m_LyricsSrcListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSourceDClicked ), NULL, this );
        m_LyricsSrcListBox->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSourceToggled ), NULL, this );
        m_LyricsAddButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricAddBtnClick ), NULL, this );
        m_LyricsUpButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricUpBtnClick ), NULL, this );
        m_LyricsDownButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDownBtnClick ), NULL, this );
        m_LyricsDelButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDelBtnClick ), NULL, this );
        m_LyricsSaveListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSaveSelected ), NULL, this );
        m_LyricsSaveListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDClicked ), NULL, this );
        m_LyricsSaveListBox->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSaveToggled ), NULL, this );
        m_LyricsSaveAddButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveAddBtnClick ), NULL, this );
        m_LyricsSaveUpButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveUpBtnClick ), NULL, this );
        m_LyricsSaveDownButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDownBtnClick ), NULL, this );
        m_LyricsSaveDelButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDelBtnClick ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ONLINE )
    {
        m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
        m_OnlineAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
        m_OnlineDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
        m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
    {
        m_LastFMUserNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );
        m_LastFMPasswdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );

        m_LibreFMUserNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );
        m_LibreFMPasswdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_JAMENDO )
    {
        m_JamSelAllBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectAll ), NULL, this );
        m_JamSelNoneBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectNone ), NULL, this );
        m_JamInvertBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoInvertSelection ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LINKS )
    {
        m_LinksListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
        m_LinksAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
        m_LinksDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
        m_LinksMoveUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
        m_LinksMoveDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
        m_LinksUrlTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
        m_LinksNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
        m_LinksAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COMMANDS )
    {
        m_CmdListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCmdListBoxSelected ), NULL, this );
        m_CmdAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdAddBtnClick ), NULL, this );
        m_CmdDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdDelBtnClick ), NULL, this );
        m_CmdMoveUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveUpBtnClick ), NULL, this );
        m_CmdMoveDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveDownBtnClick ), NULL, this );
        m_CmdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
        m_CmdNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
        m_CmdAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdSaveBtnClick ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COPYTO )
    {
        m_CopyToListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToListBoxSelected ), NULL, this );
        m_CopyToAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToAddBtnClick ), NULL, this );
        m_CopyToDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToDelBtnClick ), NULL, this );
        m_CopyToUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveUpBtnClick ), NULL, this );
        m_CopyToDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveDownBtnClick ), NULL, this );
        m_CopyToPatternTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
        m_CopyToNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
        m_CopyToFormatChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToFormatChanged ), NULL, this );
        m_CopyToQualityChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToQualityChanged ), NULL, this );
        m_CopyToMoveFilesChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveFilesChanged ), NULL, this );
        m_CopyToAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToSaveBtnClick ), NULL, this );

        if( m_CopyToOptions )
        {
            delete m_CopyToOptions;
        }
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_ACCELERATORS )
    {
    }

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPageChanged( wxCommandEvent &event )
{
    switch( m_MainNotebook->GetSelection() )
    {
        case guPREFERENCE_PAGE_GENERAL          : BuildGeneralPage();       break;
        case guPREFERENCE_PAGE_LIBRARY          : BuildLibraryPage();       break;
        case guPREFERENCE_PAGE_PLAYBACK         : BuildPlaybackPage();      break;
        case guPREFERENCE_PAGE_CROSSFADER       : BuildCrossfaderPage();    break;
        case guPREFERENCE_PAGE_RECORD           : BuildRecordPage();        break;
        case guPREFERENCE_PAGE_AUDIOSCROBBLE    : BuildAudioScrobblePage(); break;
        case guPREFERENCE_PAGE_LYRICS           : BuildLyricsPage();        break;
        case guPREFERENCE_PAGE_ONLINE           : BuildOnlinePage();        break;
        case guPREFERENCE_PAGE_PODCASTS         : BuildPodcastsPage();      break;
        case guPREFERENCE_PAGE_JAMENDO          : BuildJamendoPage();       break;
        case guPREFERENCE_PAGE_MAGNATUNE        : BuildMagnatunePage();     break;
        case guPREFERENCE_PAGE_LINKS            : BuildLinksPage();         break;
        case guPREFERENCE_PAGE_COMMANDS         : BuildCommandsPage();      break;
        case guPREFERENCE_PAGE_COPYTO           : BuildCopyToPage();        break;
        case guPREFERENCE_PAGE_ACCELERATORS     : BuildAcceleratorsPage();  break;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildGeneralPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_GENERAL )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_GENERAL;

    //
    // General Preferences Panel
    //
	wxBoxSizer * GenMainSizer = new wxBoxSizer( wxVERTICAL );
	wxStaticBoxSizer * StartSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" On Start ") ), wxVERTICAL );

	m_ShowSplashChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Show splash screen"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowSplashChkBox->SetValue( m_Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "General" ) ) );
	StartSizer->Add( m_ShowSplashChkBox, 0, wxALL, 5 );

	m_MinStartChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Start minimized"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MinStartChkBox->SetValue( m_Config->ReadBool( wxT( "StartMinimized" ), false, wxT( "General" ) ) );
	StartSizer->Add( m_MinStartChkBox, 0, wxALL, 5 );

	wxBoxSizer * StartPlayingSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SavePosCheckBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Restore position for tracks longer than"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SavePosCheckBox->SetValue( m_Config->ReadBool( wxT( "SaveCurrentTrackPos" ), false, wxT( "General" ) ) );

	StartPlayingSizer->Add( m_SavePosCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	m_MinLenSpinCtrl = new wxSpinCtrl( m_GenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 9999, 10 );
	m_MinLenSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MinSavePlayPosLength" ), 10, wxT( "General" ) ) );
	m_MinLenSpinCtrl->SetToolTip( _( "set the minimun length in minutes to save track position" ) );

	StartPlayingSizer->Add( m_MinLenSpinCtrl, 0, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MinLenStaticText = new wxStaticText( m_GenPanel, wxID_ANY, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
	MinLenStaticText->Wrap( -1 );
	StartPlayingSizer->Add( MinLenStaticText, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	StartSizer->Add( StartPlayingSizer, 1, wxEXPAND, 5 );

    m_IgnoreLayoutsChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _( "Load default layouts" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreLayoutsChkBox->SetValue( m_Config->ReadBool( wxT( "LoadDefaultLayouts" ), false, wxT( "General" ) ) );
	StartSizer->Add( m_IgnoreLayoutsChkBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	GenMainSizer->Add( StartSizer, 0, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * BehaviSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" Behaviour ") ), wxVERTICAL );

	m_TaskIconChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Activate task bar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TaskIconChkBox->SetValue( m_Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "General" ) ) );
	BehaviSizer->Add( m_TaskIconChkBox, 0, wxALL, 5 );

    m_SoundMenuChkBox = NULL;
#ifdef WITH_LIBINDICATE_SUPPORT
	m_SoundMenuChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Integrate into SoundMenu"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SoundMenuChkBox->SetValue( m_Config->ReadBool( wxT( "SoundMenuIntegration" ), false, wxT( "General" ) ) );
    m_SoundMenuChkBox->Enable( m_TaskIconChkBox->IsChecked() );
	BehaviSizer->Add( m_SoundMenuChkBox, 0, wxALL, 5 );
#endif

	m_EnqueueChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Enqueue as default action"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnqueueChkBox->SetValue( m_Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) );
	BehaviSizer->Add( m_EnqueueChkBox, 0, wxALL, 5 );

	m_DropFilesChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Drop files clear playlist"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DropFilesChkBox->SetValue( m_Config->ReadBool( wxT( "DropFilesClearPlayList" ), false, wxT( "General" ) ) );
	BehaviSizer->Add( m_DropFilesChkBox, 0, wxALL, 5 );

	GenMainSizer->Add( BehaviSizer, 0, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * OnCloseSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" On Close ") ), wxVERTICAL );

	m_SavePlayListChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Save playlist on close"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SavePlayListChkBox->SetValue( m_Config->ReadBool( wxT( "SavePlayListOnClose" ), true, wxT( "General" ) ) );
	OnCloseSizer->Add( m_SavePlayListChkBox, 0, wxALL, 5 );

	m_CloseTaskBarChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Close to task bar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CloseTaskBarChkBox->SetValue( m_Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) );
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() && ( !m_SoundMenuChkBox || !m_SoundMenuChkBox->IsChecked() ) );
	OnCloseSizer->Add( m_CloseTaskBarChkBox, 0, wxALL, 5 );

	m_ExitConfirmChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Ask confirmation on exit"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExitConfirmChkBox->SetValue( m_Config->ReadBool( wxT( "ShowCloseConfirm" ), true, wxT( "General" ) ) );
	OnCloseSizer->Add( m_ExitConfirmChkBox, 0, wxALL, 5 );

	GenMainSizer->Add( OnCloseSizer, 0, wxEXPAND|wxALL, 5 );

	m_GenPanel->SetSizer( GenMainSizer );
	m_GenPanel->Layout();

	m_TaskIconChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateTaskBarIcon ), NULL, this );
#ifdef WITH_LIBINDICATE_SUPPORT
	m_SoundMenuChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateSoundMenuIntegration ), NULL, this );
#endif

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildLibraryPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LIBRARY )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_LIBRARY;

    //
    // Library Preferences Panel
    //
	wxBoxSizer * LibMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * PathsSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, _(" Paths ") ), wxHORIZONTAL );

	m_PathsListBox = new wxListBox( m_LibPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_PathsListBox->Append( m_Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) ) );
	PathsSizer->Add( m_PathsListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * PathButtonsSizer = new wxBoxSizer( wxVERTICAL );

	m_AddPathButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PathButtonsSizer->Add( m_AddPathButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DelPathButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelPathButton->Disable();
	PathButtonsSizer->Add( m_DelPathButton, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	PathsSizer->Add( PathButtonsSizer, 0, wxEXPAND, 5 );

	LibMainSizer->Add( PathsSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * CoversSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, _(" Words to detect covers ") ), wxHORIZONTAL );

	m_CoversListBox = new wxListBox( m_LibPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	m_CoversListBox->Append( m_Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) ) );
	CoversSizer->Add( m_CoversListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * CoversButtonsSizer = new wxBoxSizer( wxVERTICAL );

	m_AddCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	CoversButtonsSizer->Add( m_AddCoverButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_UpCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_UpCoverButton->Enable( false );
	CoversButtonsSizer->Add( m_UpCoverButton, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DownCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DownCoverButton->Enable( false );
	CoversButtonsSizer->Add( m_DownCoverButton, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_DelCoverButton = new wxBitmapButton( m_LibPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelCoverButton->Enable( false );
	CoversButtonsSizer->Add( m_DelCoverButton, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CoversSizer->Add( CoversButtonsSizer, 0, wxEXPAND, 5 );

	LibMainSizer->Add( CoversSizer, 0, wxEXPAND|wxALL, 5 );


	wxStaticBoxSizer * LibOptionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	m_UpdateLibChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Update library on application start"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UpdateLibChkBox->SetValue( m_Config->ReadBool( wxT( "UpdateLibOnStart" ), false, wxT( "General" ) ) );
	LibOptionsSizer->Add( m_UpdateLibChkBox, 0, wxALL, 5 );

	m_LibScanPlayListChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Create Playlists on library scan"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LibScanPlayListChkBox->SetValue( m_Config->ReadBool( wxT( "ScanAddPlayLists" ), true, wxT( "General" ) ) );
	LibOptionsSizer->Add( m_LibScanPlayListChkBox, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_LibScanSymlinksChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Follow symbolic links on library scan"), wxDefaultPosition, wxDefaultSize, 0 );
	m_LibScanSymlinksChkBox->SetValue( m_Config->ReadBool( wxT( "ScanSymlinks" ), false, wxT( "General" ) ) );
	LibOptionsSizer->Add( m_LibScanSymlinksChkBox, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_LibScanEmbCoversChkBox = new wxCheckBox( m_LibPanel, wxID_ANY, _("Scan embedded covers in audio files"), wxDefaultPosition, wxDefaultSize, 0 );
	m_LibScanEmbCoversChkBox->SetValue( m_Config->ReadBool( wxT( "ScanEmbeddedCovers" ), true, wxT( "General" ) ) );
	LibOptionsSizer->Add( m_LibScanEmbCoversChkBox, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	LibMainSizer->Add( LibOptionsSizer, 0, wxEXPAND|wxALL, 5 );

	m_LibPanel->SetSizer( LibMainSizer );
	m_LibPanel->Layout();


    //
    //
    //
	m_PathsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxSelected ), NULL, this );
	m_AddPathButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddPathBtnClick ), NULL, this );
	m_DelPathButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPathBtnClick ), NULL, this );
	m_PathsListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxDClicked ), NULL, this );

	m_CoversListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCoversListBoxSelected ), NULL, this );
	m_AddCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddCoverBtnClick ), NULL, this );
	m_UpCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnUpCoverBtnClick ), NULL, this );
	m_DownCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDownCoverBtnClick ), NULL, this );
	m_DelCoverButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelCoverBtnClick ), NULL, this );
	m_CoversListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnCoverListBoxDClicked ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildPlaybackPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PLAYBACK )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_PLAYBACK;

    //
    // Playback Panel
    //
	wxBoxSizer * PlayMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * PlayGenSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	wxBoxSizer * RandomPlaySizer = new wxBoxSizer( wxHORIZONTAL );

	m_RndPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Play random" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_RndPlayChkBox->SetValue( m_Config->ReadBool( wxT( "RndPlayOnEmptyPlayList" ), false, wxT( "General" ) ) );
	RandomPlaySizer->Add( m_RndPlayChkBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxString m_RndModeChoiceChoices[] = { _( "track" ), _( "album" ) };
	int m_RndModeChoiceNChoices = sizeof( m_RndModeChoiceChoices ) / sizeof( wxString );
	m_RndModeChoice = new wxChoice( m_PlayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_RndModeChoiceNChoices, m_RndModeChoiceChoices, 0 );
    m_RndModeChoice->Enable( m_RndPlayChkBox->IsChecked() );
	m_RndModeChoice->SetSelection( m_Config->ReadNum( wxT( "RndModeOnEmptyPlayList" ), 0, wxT( "General" ) ) );
	//m_RndModeChoice->SetMinSize( wxSize( 150,-1 ) );
	RandomPlaySizer->Add( m_RndModeChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * RndTextStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _( "when playlist is empty" ), wxDefaultPosition, wxDefaultSize, 0 );
	RndTextStaticText->Wrap( -1 );
	RandomPlaySizer->Add( RndTextStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	PlayGenSizer->Add( RandomPlaySizer, 1, wxEXPAND, 5 );

	m_DelPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Delete played tracks from playlist" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_DelPlayChkBox->SetValue( m_Config->ReadBool( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) ) );
	PlayGenSizer->Add( m_DelPlayChkBox, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	m_NotifyChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Show Notifications" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_NotifyChkBox->SetValue( m_Config->ReadBool( wxT( "ShowNotifications" ), true, wxT( "General" ) ) );
	PlayGenSizer->Add( m_NotifyChkBox, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer * PlayReplaySizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * PlayReplayLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("Replaygain Mode :"), wxDefaultPosition, wxDefaultSize, 0 );
	PlayReplayLabel->Wrap( -1 );
	PlayReplaySizer->Add( PlayReplayLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxString m_PlayReplayModeChoiceChoices[] = { _( "Disabled" ), _("Track"), _("Album") };
	int m_PlayReplayModeChoiceNChoices = sizeof( m_PlayReplayModeChoiceChoices ) / sizeof( wxString );
	m_PlayReplayModeChoice = new wxChoice( m_PlayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PlayReplayModeChoiceNChoices, m_PlayReplayModeChoiceChoices, 0 );
	m_PlayReplayModeChoice->SetSelection( m_Config->ReadNum( wxT( "ReplayGainMode"), 0, wxT( "General" ) ) );
	PlayReplaySizer->Add( m_PlayReplayModeChoice, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	PlayGenSizer->Add( PlayReplaySizer, 1, wxEXPAND, 5 );

	PlayMainSizer->Add( PlayGenSizer, 0, wxEXPAND|wxALL, 5 );


    wxStaticBoxSizer * SmartPlayListSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _( " Random / Smart Play Modes " ) ), wxVERTICAL );

	wxFlexGridSizer * SmartPlayListFlexGridSizer = new wxFlexGridSizer( 4, 2, 0, 0 );
	SmartPlayListFlexGridSizer->SetFlexibleDirection( wxBOTH );
	SmartPlayListFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_MinTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 10, 4 );
    m_MinTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) ) );
	SmartPlayListFlexGridSizer->Add( m_MinTracksSpinCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MinTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Tracks left to start search"), wxDefaultPosition, wxDefaultSize, 0 );
	MinTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( MinTracksStaticText, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_NumTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 10, 3 );
    m_NumTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "NumTracksToAdd" ), 3, wxT( "Playback" ) ) );
	SmartPlayListFlexGridSizer->Add( m_NumTracksSpinCtrl, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * AddTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Tracks added each time"), wxDefaultPosition, wxDefaultSize, 0 );
	AddTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( AddTracksStaticText, 0, wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

	m_MaxTracksPlayed = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 999, 20 );
    m_MaxTracksPlayed->SetValue( m_Config->ReadNum( wxT( "MaxTracksPlayed" ), 20, wxT( "Playback" ) ) );
    m_MaxTracksPlayed->Enable( !m_DelPlayChkBox->IsChecked() );
	SmartPlayListFlexGridSizer->Add( m_MaxTracksPlayed, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MaxTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Max played tracks kept in playlist"), wxDefaultPosition, wxDefaultSize, 0 );
	MaxTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( MaxTracksStaticText, 0, wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

	SmartPlayListSizer->Add( SmartPlayListFlexGridSizer, 1, wxEXPAND, 5 );

	wxBoxSizer * SmartPlayFilterSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * SmartPlayFilterLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("Don't repeat last"), wxDefaultPosition, wxDefaultSize, 0 );
	SmartPlayFilterLabel->Wrap( -1 );
	SmartPlayFilterSizer->Add( SmartPlayFilterLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_SmartPlayArtistsSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 50, 20 );
    m_SmartPlayArtistsSpinCtrl->SetValue( m_Config->ReadNum( wxT( "SmartFilterArtists" ), 20, wxT( "Playback" ) ) );
	SmartPlayFilterSizer->Add( m_SmartPlayArtistsSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * SmartPlayArtistLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("artists or"), wxDefaultPosition, wxDefaultSize, 0 );
	SmartPlayArtistLabel->Wrap( -1 );
	SmartPlayFilterSizer->Add( SmartPlayArtistLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_SmartPlayTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 55,-1 ), wxSP_ARROW_KEYS, 0, 200, 100 );
    m_SmartPlayTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "SmartFilterTracks" ), 100, wxT( "Playback" ) ) );
	SmartPlayFilterSizer->Add( m_SmartPlayTracksSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * SmartPlayTracksLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	SmartPlayTracksLabel->Wrap( -1 );
	SmartPlayFilterSizer->Add( SmartPlayTracksLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	SmartPlayListSizer->Add( SmartPlayFilterSizer, 0, wxEXPAND, 5 );

	PlayMainSizer->Add( SmartPlayListSizer, 0, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer * PlaySilenceSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _(" Silence detector ") ), wxVERTICAL );

	wxBoxSizer * PlayLevelSizer = new wxBoxSizer( wxHORIZONTAL );

	bool IsPlayLevelEnabled = m_Config->ReadBool( wxT( "SilenceDetector" ), false, wxT( "Playback" ) );
	m_PlayLevelEnabled = new wxCheckBox( m_PlayPanel, wxID_ANY, _("Skip at"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PlayLevelEnabled->SetValue( IsPlayLevelEnabled );
	PlayLevelSizer->Add( m_PlayLevelEnabled, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    int PlayLevelValue = m_Config->ReadNum( wxT( "SilenceLevel" ), -500, wxT( "Playback" ) );
	m_PlayLevelVal = new wxStaticText( m_PlayPanel, wxID_ANY, wxString::Format( wxT("%02idb"), PlayLevelValue ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayLevelVal->Wrap( -1 );
	m_PlayLevelVal->Enable( IsPlayLevelEnabled );
	PlayLevelSizer->Add( m_PlayLevelVal, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT, 5 );

	m_PlayLevelSlider = new wxSlider( m_PlayPanel, wxID_ANY, PlayLevelValue, -65, 0, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_PlayLevelSlider->Enable( IsPlayLevelEnabled );
	PlayLevelSizer->Add( m_PlayLevelSlider, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PlaySilenceSizer->Add( PlayLevelSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* PlayEndTimeSizer;
	PlayEndTimeSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayEndTimeCheckBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _("In the last"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PlayEndTimeCheckBox->SetValue( m_Config->ReadBool( wxT( "SilenceAtEnd" ), true, wxT( "Playback" ) ) );
	m_PlayEndTimeCheckBox->Enable( IsPlayLevelEnabled );
	PlayEndTimeSizer->Add( m_PlayEndTimeCheckBox, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	m_PlayEndTimeSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5, 360,
	    m_Config->ReadNum( wxT( "SilenceEndTime" ), 45, wxT( "Playback" ) ) );
	m_PlayEndTimeSpinCtrl->Enable( IsPlayLevelEnabled && m_PlayEndTimeCheckBox->IsChecked() );
	PlayEndTimeSizer->Add( m_PlayEndTimeSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * PlayEndTimeStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _( "seconds" ), wxDefaultPosition, wxDefaultSize, 0 );
	PlayEndTimeStaticText->Wrap( -1 );
	PlayEndTimeSizer->Add( PlayEndTimeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PlaySilenceSizer->Add( PlayEndTimeSizer, 0, wxEXPAND, 5 );

	PlayMainSizer->Add( PlaySilenceSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxStaticBoxSizer* PlayOutDeviceSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _(" Output Device ") ), wxHORIZONTAL );

    wxArrayString OutputDeviceOptions;
    OutputDeviceOptions.Add( _( "Automatic" ) );
    OutputDeviceOptions.Add( _( "GConf defined" ) );
    OutputDeviceOptions.Add( wxT( "Alsa" ) );
    OutputDeviceOptions.Add( wxT( "PulseAudio" ) );
    OutputDeviceOptions.Add( wxT( "OSS" ) );

	m_PlayOutDevChoice = new wxChoice( m_PlayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, OutputDeviceOptions, 0 );
	int OutDevice = m_Config->ReadNum( wxT( "OutputDevice" ), guOUTPUT_DEVICE_AUTOMATIC, wxT( "Playback" ) );
	m_PlayOutDevChoice->SetSelection( OutDevice );
	PlayOutDeviceSizer->Add( m_PlayOutDevChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_PlayOutDevName = new wxTextCtrl( m_PlayPanel, wxID_ANY, m_Config->ReadStr( wxT( "OutputDeviceName" ), wxEmptyString, wxT( "Playback" ) ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayOutDevName->Enable( OutDevice > guOUTPUT_DEVICE_GCONF );
	PlayOutDeviceSizer->Add( m_PlayOutDevName, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PlayMainSizer->Add( PlayOutDeviceSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );


	m_PlayPanel->SetSizer( PlayMainSizer );
	m_PlayPanel->Layout();

    //
    //
    //
	m_RndPlayChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRndPlayClicked ), NULL, this );
	m_DelPlayChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPlayedTracksChecked ), NULL, this );
	m_PlayLevelEnabled->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayLevelEnabled ), NULL, this );
    m_PlayLevelSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnPlayLevelValueChanged ), NULL, this );
	m_PlayEndTimeCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayEndTimeEnabled ), NULL, this );
	m_PlayOutDevChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnPlayOutDevChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildCrossfaderPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_CROSSFADER )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_CROSSFADER;

    //
    // Crossfader Panel
    //
	wxBoxSizer * XFadeMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * XFadesbSizer = new wxStaticBoxSizer( new wxStaticBox( m_XFadePanel, wxID_ANY, _(" Crossfader ") ), wxVERTICAL );

	wxFlexGridSizer * XFadeFlexSizer = new wxFlexGridSizer( 4, 3, 0, 0 );
	XFadeFlexSizer->AddGrowableCol( 2 );
	XFadeFlexSizer->SetFlexibleDirection( wxBOTH );
	XFadeFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * XFadeOutLenLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("Out length:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeOutLenLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeOutLenLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeOutLenVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeOutLenVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeOutLenVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeOutLenSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeOutTime" ), 50, wxT( "Crossfader" ) ), 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeOutLenSlider->SetToolTip( _( "Select the length of the fade out. 0 for gapless playback" ) );
	XFadeFlexSizer->Add( m_XFadeOutLenSlider, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * XFadeInLenLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("In length:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeInLenLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeInLenLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInLenVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeInLenVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeInLenVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInLenSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeInTime" ), 10, wxT( "Crossfader" ) ), 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeInLenSlider->SetToolTip( _( "Select the length of the fade in" ) );
	XFadeFlexSizer->Add( m_XFadeInLenSlider, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * XFadeInStartLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("In vol. Start:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeInStartLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeInStartLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInStartVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeInStartVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeInStartVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInStartSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeInVolStart" ), 80, wxT( "Crossfader" ) ), 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeInStartSlider->SetToolTip( _( "Select the initial volume of the fade in" ) );
	XFadeFlexSizer->Add( m_XFadeInStartSlider, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * XFadeTrigerLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("In start:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeTrigerLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeTrigerLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeTrigerVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeTrigerVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeTrigerVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInTrigerSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeInVolTriger" ), 50, wxT( "Crossfader" ) ), 10, 90, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeInTrigerSlider->SetToolTip( _( "Select at which volume of the fade out the fade in starts" ) );
	XFadeFlexSizer->Add( m_XFadeInTrigerSlider, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	XFadesbSizer->Add( XFadeFlexSizer, 1, wxEXPAND, 5 );

	XFadeMainSizer->Add( XFadesbSizer, 0, wxEXPAND|wxALL, 5 );

	m_FadeBitmap = new wxStaticBitmap( m_XFadePanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 400,200 ), 0 );
	XFadeMainSizer->Add( m_FadeBitmap, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_XFadePanel->SetSizer( XFadeMainSizer );
	m_XFadePanel->Layout();

    //
    //
    //
    m_XFadeOutLenSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInLenSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInStartSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInTrigerSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeOutLenSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInLenSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInStartSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInTrigerSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );

    //
    wxScrollEvent ScrollEvent;
    OnCrossFadeChanged( ScrollEvent );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildRecordPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_RECORD )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_RECORD;

    //
    // Record Panel
    //
	wxBoxSizer * RecMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * RecordSizer = new wxStaticBoxSizer( new wxStaticBox( m_RecordPanel, wxID_ANY, _(" Record ") ), wxVERTICAL );

	m_RecordChkBox = new wxCheckBox( m_RecordPanel, wxID_ANY, _("Enable recording"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RecordChkBox->SetValue( m_Config->ReadBool( wxT( "Enabled" ), false, wxT( "Record" ) ) );
	RecordSizer->Add( m_RecordChkBox, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer * RecSelDirSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * RecSelDirLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("Save to:"), wxDefaultPosition, wxDefaultSize, 0 );
	RecSelDirLabel->Wrap( -1 );
	RecSelDirSizer->Add( RecSelDirLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_RecSelDirPicker = new wxDirPickerCtrl( m_RecordPanel, wxID_ANY, wxEmptyString, _("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_DIR_MUST_EXIST );
    m_RecSelDirPicker->SetPath( m_Config->ReadStr( wxT( "Path" ), wxGetHomeDir() + wxT( "/Records" ), wxT( "Record" ) ) );
    m_RecSelDirPicker->Enable( m_RecordChkBox->IsChecked() );
	RecSelDirSizer->Add( m_RecSelDirPicker, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	RecordSizer->Add( RecSelDirSizer, 0, wxEXPAND, 5 );

	wxStaticBoxSizer * RecPropSizer = new wxStaticBoxSizer( new wxStaticBox( m_RecordPanel, wxID_ANY, _(" Properties ") ), wxVERTICAL );

	wxFlexGridSizer * RecPropFlexSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	RecPropFlexSizer->AddGrowableCol( 1 );
	RecPropFlexSizer->SetFlexibleDirection( wxBOTH );
	RecPropFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * RecFormatLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("Format:"), wxDefaultPosition, wxDefaultSize, 0 );
	RecFormatLabel->Wrap( -1 );
	RecPropFlexSizer->Add( RecFormatLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	wxString m_RecFormatChoiceChoices[] = { wxT("mp3"), wxT("ogg"), wxT("flac") };
	int m_RecFormatChoiceNChoices = sizeof( m_RecFormatChoiceChoices ) / sizeof( wxString );
	m_RecFormatChoice = new wxChoice( m_RecordPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_RecFormatChoiceNChoices, m_RecFormatChoiceChoices, 0 );
	m_RecFormatChoice->SetSelection( m_Config->ReadNum( wxT( "Format" ), 0, wxT( "Record" ) ) );
    m_RecFormatChoice->Enable( m_RecordChkBox->IsChecked() );
	m_RecFormatChoice->SetMinSize( wxSize( 150,-1 ) );
	RecPropFlexSizer->Add( m_RecFormatChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * RecQualityLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("Quality:"), wxDefaultPosition, wxDefaultSize, 0 );
	RecQualityLabel->Wrap( -1 );
	RecPropFlexSizer->Add( RecQualityLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	wxArrayString RecQualityChoiceChoices;
	RecQualityChoiceChoices.Add( _( "Very High" ) );
	RecQualityChoiceChoices.Add( _( "High" ) );
	RecQualityChoiceChoices.Add( _( "Normal" ) );
	RecQualityChoiceChoices.Add( _( "Low" ) );
	RecQualityChoiceChoices.Add( _( "Very Low" ) );

	m_RecQualityChoice = new wxChoice( m_RecordPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, RecQualityChoiceChoices, 0 );
	m_RecQualityChoice->SetSelection( m_Config->ReadNum( wxT( "Quality" ), 2, wxT( "Record" ) ) );
    m_RecQualityChoice->Enable( m_RecordChkBox->IsChecked() );
	m_RecQualityChoice->SetMinSize( wxSize( 150,-1 ) );
	RecPropFlexSizer->Add( m_RecQualityChoice, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	RecPropSizer->Add( RecPropFlexSizer, 1, wxEXPAND, 5 );

	RecordSizer->Add( RecPropSizer, 1, wxEXPAND|wxALL, 5 );

	m_RecSplitChkBox = new wxCheckBox( m_RecordPanel, wxID_ANY, _( "Split tracks" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_RecSplitChkBox->SetValue( m_Config->ReadBool( wxT( "Split" ), false, wxT( "Record" ) ) );
    m_RecSplitChkBox->Enable( m_RecordChkBox->IsChecked() );
	RecordSizer->Add( m_RecSplitChkBox, 0, wxALL, 5 );

	wxBoxSizer * RecDelSizer = new wxBoxSizer( wxHORIZONTAL );

	m_RecDelTracks = new wxCheckBox( m_RecordPanel, wxID_ANY, _("Delete Tracks shorter than"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RecDelTracks->SetValue( m_Config->ReadBool( wxT( "DeleteTracks" ), false, wxT( "Record" ) ) );

	RecDelSizer->Add( m_RecDelTracks, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_RecDelTime = new wxSpinCtrl( m_RecordPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 999,
                                    m_Config->ReadNum( wxT( "DeleteTime" ), 50, wxT( "Record" ) ) );
	m_RecDelTime->Enable( m_RecDelTracks->IsChecked() );
	RecDelSizer->Add( m_RecDelTime, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * RecDelSecLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	RecDelSecLabel->Wrap( -1 );
	RecDelSizer->Add( RecDelSecLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	RecordSizer->Add( RecDelSizer, 0, wxEXPAND, 5 );

	RecMainSizer->Add( RecordSizer, 0, wxEXPAND|wxALL, 5 );

	m_RecordPanel->SetSizer( RecMainSizer );
	m_RecordPanel->Layout();

    //
    //
    //
	m_RecordChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecEnableClicked ), NULL, this );
	m_RecDelTracks->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecDelTracksClicked ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildAudioScrobblePage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE;

    //
    // LastFM Panel
    //
	wxBoxSizer * ASMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * LastFMASSizer = new wxStaticBoxSizer( new wxStaticBox( m_LastFMPanel, wxID_ANY, _(" Last.fm Audioscrobble ") ), wxVERTICAL );

	m_LastFMASEnableChkBox = new wxCheckBox( m_LastFMPanel, wxID_ANY, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LastFMASEnableChkBox->SetValue( m_Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) ) );
	LastFMASSizer->Add( m_LastFMASEnableChkBox, 0, wxALL, 5 );

	wxFlexGridSizer * ASLoginSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	ASLoginSizer->SetFlexibleDirection( wxBOTH );
	ASLoginSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * UserNameStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
	UserNameStaticText->Wrap( -1 );
	ASLoginSizer->Add( UserNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LastFMUserNameTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
	m_LastFMUserNameTextCtrl->SetValue( m_Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "LastFM" ) ) );
	ASLoginSizer->Add( m_LastFMUserNameTextCtrl, 0, wxALL, 5 );

	wxStaticText * PasswdStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	PasswdStaticText->Wrap( -1 );
	ASLoginSizer->Add( PasswdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LastFMPasswdTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), wxTE_PASSWORD );
	m_LastFMPasswdTextCtrl->SetValue( m_Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "LastFM" ) ).IsEmpty() ? wxEmptyString : wxT( "******" ) );
	// Password is saved in md5 form so we cant load it back
	ASLoginSizer->Add( m_LastFMPasswdTextCtrl, 0, wxALL, 5 );

	if( m_LastFMPasswdTextCtrl->IsEmpty() ||
	    m_LastFMUserNameTextCtrl->IsEmpty() )
        m_LastFMASEnableChkBox->Disable();

	LastFMASSizer->Add( ASLoginSizer, 1, wxEXPAND, 5 );

	ASMainSizer->Add( LastFMASSizer, 0, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * LibreFMASSizer = new wxStaticBoxSizer( new wxStaticBox( m_LastFMPanel, wxID_ANY, _(" Libre.fm Audioscrobble ") ), wxVERTICAL );

	m_LibreFMASEnableChkBox = new wxCheckBox( m_LastFMPanel, wxID_ANY, _( "Enabled" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_LibreFMASEnableChkBox->SetValue( m_Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LibreFM" ) ) );
	LibreFMASSizer->Add( m_LibreFMASEnableChkBox, 0, wxALL, 5 );

	wxFlexGridSizer * LibreFMASLoginSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	LibreFMASLoginSizer->SetFlexibleDirection( wxBOTH );
	LibreFMASLoginSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * LibreFMUserNameStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _( "Username:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LibreFMUserNameStaticText->Wrap( -1 );
	LibreFMASLoginSizer->Add( LibreFMUserNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LibreFMUserNameTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
	m_LibreFMUserNameTextCtrl->SetValue( m_Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "LibreFM" ) ) );
	LibreFMASLoginSizer->Add( m_LibreFMUserNameTextCtrl, 0, wxALL, 5 );

	wxStaticText * LibreFMPasswdStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	LibreFMPasswdStaticText->Wrap( -1 );
	LibreFMASLoginSizer->Add( LibreFMPasswdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LibreFMPasswdTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), wxTE_PASSWORD );
	m_LibreFMPasswdTextCtrl->SetValue( m_Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "LibreFM" ) ).IsEmpty() ? wxEmptyString : wxT( "******" ) );
	// Password is saved in md5 form so we cant load it back
	LibreFMASLoginSizer->Add( m_LibreFMPasswdTextCtrl, 0, wxALL, 5 );

	if( m_LibreFMPasswdTextCtrl->IsEmpty() ||
	    m_LibreFMUserNameTextCtrl->IsEmpty() )
        m_LibreFMASEnableChkBox->Disable();

	LibreFMASSizer->Add( LibreFMASLoginSizer, 0, wxEXPAND, 5 );

	ASMainSizer->Add( LibreFMASSizer, 0, wxEXPAND|wxALL, 5 );


	ASMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxHyperlinkCtrl * m_UserGroupLink = new wxHyperlinkCtrl( m_LastFMPanel, wxID_ANY,
        _( "Join the Guayadeque Last.fm users group" ), wxT("http://www.last.fm/group/Guayadeque"),
        wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	ASMainSizer->Add( m_UserGroupLink, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LastFMPanel->SetSizer( ASMainSizer );
	m_LastFMPanel->Layout();

    //
    //
    //
    m_LastFMUserNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );
    m_LastFMPasswdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );

    m_LibreFMUserNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );
    m_LibreFMPasswdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildLyricsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LYRICS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_LYRICS;

    //
    // Lyrics
    //
	wxBoxSizer * LyricsMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * LyricsSrcSizer = new wxStaticBoxSizer( new wxStaticBox( m_LyricsPanel, wxID_ANY, _(" Sources " ) ), wxHORIZONTAL );

    m_LyricSearchEngine = new guLyricSearchEngine();

    wxArrayString LyricSourcesNames;
    wxArrayInt    LyricSourcesEnabled;
    int Index;
    int Count = m_LyricSearchEngine->SourcesCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSource * LyricSource = m_LyricSearchEngine->GetSource( Index );
        LyricSourcesNames.Add( LyricSource->Name() );
        LyricSourcesEnabled.Add( LyricSource->Enabled() );
    }

	m_LyricsSrcListBox = new wxCheckListBox( m_LyricsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, LyricSourcesNames, 0 );
	LyricsSrcSizer->Add( m_LyricsSrcListBox, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	for( Index = 0; Index < Count; Index++ )
	{
        m_LyricsSrcListBox->Check( Index, LyricSourcesEnabled[ Index ] );
	}

	wxBoxSizer * LyricsSrcBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LyricsAddButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	//m_LyricsAddButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsAddButton, 0, wxTOP, 5 );

	m_LyricsUpButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsUpButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsUpButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsDownButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsDownButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsDownButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsDelButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsDelButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsDelButton, 0, wxTOP|wxBOTTOM, 5 );

	LyricsSrcSizer->Add( LyricsSrcBtnSizer, 0, wxEXPAND, 5 );

	LyricsMainSizer->Add( LyricsSrcSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer* LyricsSaveSizer = new wxStaticBoxSizer( new wxStaticBox( m_LyricsPanel, wxID_ANY, _( " Targets " ) ), wxHORIZONTAL );

    wxArrayString LyricTargetsNames;
    wxArrayInt    LyricTargetsEnabled;
    Count = m_LyricSearchEngine->TargetsCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSource * LyricTarget = m_LyricSearchEngine->GetTarget( Index );
        LyricTargetsNames.Add( LyricTarget->Name() );
        LyricTargetsEnabled.Add( LyricTarget->Enabled() );
    }
	m_LyricsSaveListBox = new wxCheckListBox( m_LyricsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, LyricTargetsNames, 0 );
	for( Index = 0; Index < Count; Index++ )
	{
        m_LyricsSaveListBox->Check( Index, LyricTargetsEnabled[ Index ] );
	}
	LyricsSaveSizer->Add( m_LyricsSaveListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * LyricsSaveBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LyricsSaveAddButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	//m_LyricsSaveAddButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveAddButton, 0, wxTOP|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LyricsSaveUpButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsSaveUpButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveUpButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsSaveDownButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsSaveDownButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveDownButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsSaveDelButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsSaveDelButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveDelButton, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	LyricsSaveSizer->Add( LyricsSaveBtnSizer, 0, wxEXPAND, 5 );

	LyricsMainSizer->Add( LyricsSaveSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxStaticBoxSizer * LyricsFontSizer = new wxStaticBoxSizer( new wxStaticBox( m_LyricsPanel, wxID_ANY, _(" Font ") ), wxHORIZONTAL );

	wxStaticText * LyricsFontLabel = new wxStaticText( m_LyricsPanel, wxID_ANY, _( "Font:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LyricsFontLabel->Wrap( -1 );
	LyricsFontSizer->Add( LyricsFontLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxRIGHT|wxLEFT, 5 );

	wxFont LyricFont;
	LyricFont.SetNativeFontInfo( m_Config->ReadStr( wxT( "Font" ), wxEmptyString, wxT( "Lyrics" ) ) );
	if( !LyricFont.IsOk() )
        LyricFont = GetFont();
	m_LyricFontPicker = new wxFontPickerCtrl( m_LyricsPanel, wxID_ANY, LyricFont, wxDefaultPosition, wxDefaultSize, wxFNTP_DEFAULT_STYLE );
	m_LyricFontPicker->SetMaxPointSize( 100 );
	LyricsFontSizer->Add( m_LyricFontPicker, 2, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * LyricsAlignLabel = new wxStaticText( m_LyricsPanel, wxID_ANY, _( "Align:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LyricsAlignLabel->Wrap( -1 );
	LyricsFontSizer->Add( LyricsAlignLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxRIGHT|wxLEFT, 5 );

	wxArrayString LyricsAlignChoices;
	LyricsAlignChoices.Add( _( "Left" ) );
	LyricsAlignChoices.Add( _( "Center" ) );
	LyricsAlignChoices.Add( _( "Right" ) );
	m_LyricsAlignChoice = new wxChoice( m_LyricsPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), LyricsAlignChoices, 0 );
	m_LyricsAlignChoice->SetSelection( m_Config->ReadNum( wxT( "TextAlign" ), 1, wxT( "Lyrics" ) ) );
	LyricsFontSizer->Add( m_LyricsAlignChoice, 1, wxRIGHT, 5 );

	LyricsMainSizer->Add( LyricsFontSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_LyricsPanel->SetSizer( LyricsMainSizer );
	m_LyricsPanel->Layout();

	m_LyricsSrcListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSourceSelected ), NULL, this );
	m_LyricsSrcListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSourceDClicked ), NULL, this );
	m_LyricsSrcListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSourceToggled ), NULL, this );
	m_LyricsAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricAddBtnClick ), NULL, this );
	m_LyricsUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricUpBtnClick ), NULL, this );
	m_LyricsDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDownBtnClick ), NULL, this );
	m_LyricsDelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDelBtnClick ), NULL, this );
	m_LyricsSaveListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSaveSelected ), NULL, this );
	m_LyricsSaveListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDClicked ), NULL, this );
	m_LyricsSaveListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSaveToggled ), NULL, this );
	m_LyricsSaveAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveAddBtnClick ), NULL, this );
	m_LyricsSaveUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveUpBtnClick ), NULL, this );
	m_LyricsSaveDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDownBtnClick ), NULL, this );
	m_LyricsSaveDelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDelBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildOnlinePage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ONLINE )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_ONLINE;

    //
    // Online Services Filter
    //
	wxBoxSizer * OnlineMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * OnlineFiltersSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _(" Filters ") ), wxHORIZONTAL );

	m_OnlineFiltersListBox = new wxListBox( m_OnlinePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_OnlineFiltersListBox->Append( m_Config->ReadAStr( wxT( "Filter" ), wxEmptyString, wxT( "SearchFilters" ) ) );
	OnlineFiltersSizer->Add( m_OnlineFiltersListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * OnlineBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_OnlineAddBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
	OnlineBtnSizer->Add( m_OnlineAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_OnlineDelBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_OnlineDelBtn->Disable();
	OnlineBtnSizer->Add( m_OnlineDelBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	OnlineFiltersSizer->Add( OnlineBtnSizer, 0, wxEXPAND, 5 );

	OnlineMainSizer->Add( OnlineFiltersSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * OnlineLangSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _( " Language " ) ), wxHORIZONTAL );

	m_LangStaticText = new wxStaticText( m_OnlinePanel, wxID_ANY, _("Language:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_LangStaticText->Wrap( -1 );
	OnlineLangSizer->Add( m_LangStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LangChoice = new wxChoice( m_OnlinePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_LangNames, 0 );
    int LangIndex = m_LangIds.Index( m_Config->ReadStr( wxT( "Language" ), wxEmptyString, wxT( "LastFM" ) ) );
    if( LangIndex == wxNOT_FOUND )
        m_LangChoice->SetSelection( 0 );
    else
        m_LangChoice->SetSelection( LangIndex );
	m_LangChoice->SetMinSize( wxSize( 250,-1 ) );
	OnlineLangSizer->Add( m_LangChoice, 0, wxALL, 5 );

	OnlineMainSizer->Add( OnlineLangSizer, 0, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * BrowserCmdSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _(" Browser command ") ), wxHORIZONTAL );

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

	wxStaticBoxSizer * BufferSizeSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _( " Player online buffer size ") ), wxHORIZONTAL );

	m_BufferSizeSlider = new wxSlider( m_OnlinePanel, wxID_ANY, m_Config->ReadNum( wxT( "BufferSize" ), 64, wxT( "General" ) ), 32, 1024, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	BufferSizeSizer->Add( m_BufferSizeSlider, 1, wxEXPAND, 5 );

	OnlineMainSizer->Add( BufferSizeSizer, 0, wxEXPAND|wxALL, 5 );

	m_OnlinePanel->SetSizer( OnlineMainSizer );
	m_OnlinePanel->Layout();

    //
    //
    //
	m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
	m_OnlineAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
	m_OnlineDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
	m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildPodcastsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PODCASTS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_PODCASTS;

    //
    // Podcasts
    //
	wxBoxSizer * PodcastsMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * PodcastsSizer = new wxStaticBoxSizer( new wxStaticBox( m_PodcastPanel, wxID_ANY, _(" Podcasts ") ), wxVERTICAL );

	wxBoxSizer * PathSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * PodcastPathStaticText = new wxStaticText( m_PodcastPanel, wxID_ANY, _("Destination directory:"), wxDefaultPosition, wxDefaultSize, 0 );
	PodcastPathStaticText->Wrap( -1 );
	PathSizer->Add( PodcastPathStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	m_PodcastPath = new wxDirPickerCtrl( m_PodcastPanel, wxID_ANY, wxEmptyString, _("Select podcasts folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST );
    m_PodcastPath->SetPath( m_Config->ReadStr( wxT( "Path" ), wxGetHomeDir(), wxT( "Podcasts" ) ) );
	PathSizer->Add( m_PodcastPath, 1, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

	PodcastsSizer->Add( PathSizer, 0, wxEXPAND, 5 );

	wxBoxSizer * UpdateSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PodcastUpdate = new wxCheckBox( m_PodcastPanel, wxID_ANY, _("Check every"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PodcastUpdate->SetValue( m_Config->ReadBool( wxT( "Update" ), true, wxT( "Podcasts" ) ) );

	UpdateSizer->Add( m_PodcastUpdate, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxString m_PodcastUpdatePeriodChoices[] = { _( "Hour" ), _("Day"), _("Week"), _("Month") };
	int m_PodcastUpdatePeriodNChoices = sizeof( m_PodcastUpdatePeriodChoices ) / sizeof( wxString );
	m_PodcastUpdatePeriod = new wxChoice( m_PodcastPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PodcastUpdatePeriodNChoices, m_PodcastUpdatePeriodChoices, 0 );
	m_PodcastUpdatePeriod->SetSelection( m_Config->ReadNum( wxT( "UpdatePeriod" ), 0, wxT( "Podcasts" ) ) );
	m_PodcastUpdatePeriod->SetMinSize( wxSize( 150,-1 ) );
	UpdateSizer->Add( m_PodcastUpdatePeriod, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PodcastsSizer->Add( UpdateSizer, 0, wxEXPAND, 5 );

	wxBoxSizer * DeleteSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PodcastDelete = new wxCheckBox( m_PodcastPanel, wxID_ANY, _("Delete after"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PodcastDelete->SetValue( m_Config->ReadBool( wxT( "Delete" ), false, wxT( "Podcasts" ) ) );

	DeleteSizer->Add( m_PodcastDelete, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_PodcastDeleteTime = new wxSpinCtrl( m_PodcastPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 99,
        m_Config->ReadNum( wxT( "DeleteTime" ), 15, wxT( "Podcasts" ) ) );
	DeleteSizer->Add( m_PodcastDeleteTime, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxString m_PodcastDeletePeriodChoices[] = { _("Days"), _("Weeks"), _("Months") };
	int m_PodcastDeletePeriodNChoices = sizeof( m_PodcastDeletePeriodChoices ) / sizeof( wxString );
	m_PodcastDeletePeriod = new wxChoice( m_PodcastPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PodcastDeletePeriodNChoices, m_PodcastDeletePeriodChoices, 0 );
	m_PodcastDeletePeriod->SetSelection( m_Config->ReadNum( wxT( "DeletePeriod" ), 0, wxT( "Podcasts" ) ) );
	m_PodcastDeletePeriod->SetMinSize( wxSize( 150,-1 ) );
	DeleteSizer->Add( m_PodcastDeletePeriod, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DeleteSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_PodcastDeletePlayed = new wxCheckBox( m_PodcastPanel, wxID_ANY, _("Only if played"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PodcastDeletePlayed->SetValue( m_Config->ReadBool( wxT( "DeletePlayed" ), false, wxT( "Podcasts" ) ) );

	DeleteSizer->Add( m_PodcastDeletePlayed, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	PodcastsSizer->Add( DeleteSizer, 0, wxEXPAND, 5 );

	PodcastsMainSizer->Add( PodcastsSizer, 0, wxEXPAND|wxALL, 5 );

	m_PodcastPanel->SetSizer( PodcastsMainSizer );
	m_PodcastPanel->Layout();

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildJamendoPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_JAMENDO )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_JAMENDO;

    //
    // Jamendo
    //
	wxBoxSizer * JamMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * JamGenresSizer = new wxStaticBoxSizer( new wxStaticBox( m_JamendoPanel, wxID_ANY, _( " Genres " ) ), wxHORIZONTAL );

	wxArrayString JamendoGenres;
	int Index = 0;
    do {
        wxString GenreName = TStringTowxString( TagLib::ID3v1::genre( Index++ ) );

        if( !GenreName.IsEmpty() )
            JamendoGenres.Add( GenreName );
        else
            break;

    } while( true );

    m_LastJamendoGenres = m_Config->ReadANum( wxT( "Genre" ), 0, wxT( "JamendoGenres" ) );

	m_JamGenresListBox = new wxCheckListBox( m_JamendoPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, JamendoGenres, 0 );
	int Count = m_LastJamendoGenres.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        m_JamGenresListBox->Check( m_LastJamendoGenres[ Index ] );
	}
	JamGenresSizer->Add( m_JamGenresListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * JamGenresBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_JamSelAllBtn = new wxButton( m_JamendoPanel, wxID_ANY, _( "All" ), wxDefaultPosition, wxDefaultSize, 0 );
	JamGenresBtnSizer->Add( m_JamSelAllBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_JamSelNoneBtn = new wxButton( m_JamendoPanel, wxID_ANY, _( "None" ), wxDefaultPosition, wxDefaultSize, 0 );
	JamGenresBtnSizer->Add( m_JamSelNoneBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_JamInvertBtn = new wxButton( m_JamendoPanel, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
	JamGenresBtnSizer->Add( m_JamInvertBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	JamGenresSizer->Add( JamGenresBtnSizer, 0, wxEXPAND, 5 );

	JamMainSizer->Add( JamGenresSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * JamOtherSizer = new wxStaticBoxSizer( new wxStaticBox( m_JamendoPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	wxBoxSizer * JamFormatSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * JamFormatLabel = new wxStaticText( m_JamendoPanel, wxID_ANY, _( "Audio format:" ), wxDefaultPosition, wxDefaultSize, 0 );
	JamFormatLabel->Wrap( -1 );
	JamFormatSizer->Add( JamFormatLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	wxArrayString m_JamFormatChoices;
	m_JamFormatChoices.Add( wxT( "mp3" ) );
	m_JamFormatChoices.Add( wxT( "ogg" ) );
	m_JamFormatChoice = new wxChoice( m_JamendoPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_JamFormatChoices, 0 );
	m_JamFormatChoice->SetSelection( m_Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Jamendo" ) ) );
	JamFormatSizer->Add( m_JamFormatChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	JamOtherSizer->Add( JamFormatSizer, 0, wxEXPAND, 5 );

	wxBoxSizer * JamBTCmdSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * JamBTCmdLabel = new wxStaticText( m_JamendoPanel, wxID_ANY, _( "Torrent command:" ), wxDefaultPosition, wxDefaultSize, 0 );
	JamBTCmdLabel->Wrap( -1 );
	JamBTCmdSizer->Add( JamBTCmdLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_JamBTCmd = new wxTextCtrl( m_JamendoPanel, wxID_ANY, m_Config->ReadStr( wxT( "TorrentCommand" ), wxT( "transmission" ), wxT( "Jamendo" ) ), wxDefaultPosition, wxDefaultSize, 0 );
	JamBTCmdSizer->Add( m_JamBTCmd, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	JamOtherSizer->Add( JamBTCmdSizer, 1, wxEXPAND, 5 );

	JamMainSizer->Add( JamOtherSizer, 0, wxEXPAND|wxALL, 5 );

	m_JamendoPanel->SetSizer( JamMainSizer );
	m_JamendoPanel->Layout();

	m_JamSelAllBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectAll ), NULL, this );
	m_JamSelNoneBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectNone ), NULL, this );
	m_JamInvertBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoInvertSelection ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildMagnatunePage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_MAGNATUNE )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_MAGNATUNE;

    //
    // Magnatune
    //
	wxBoxSizer * MagMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * MagGenresSizer = new wxStaticBoxSizer( new wxStaticBox( m_MagnatunePanel, wxID_ANY, _(" Genres ") ), wxHORIZONTAL );

	wxArrayString MagnatuneGenres = m_Config->ReadAStr( wxT( "Genre" ), wxEmptyString, wxT( "MagnatuneGenreList" ) );
	if( !MagnatuneGenres.Count() )
	{
        int Index = 0;
        do {
            wxString GenreName = TStringTowxString( TagLib::ID3v1::genre( Index++ ) );

            if( !GenreName.IsEmpty() )
                MagnatuneGenres.Add( GenreName );
            else
                break;

        } while( true );
	}

    m_LastMagnatuneGenres = m_Config->ReadAStr( wxT( "Genre" ), wxEmptyString, wxT( "MagnatuneGenres" ) );

	m_MagGenresListBox = new wxCheckListBox( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, MagnatuneGenres, 0 );
	int Index;
	int Count = m_LastMagnatuneGenres.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        int Pos = MagnatuneGenres.Index( m_LastMagnatuneGenres[ Index ] );
        if( Pos != wxNOT_FOUND )
            m_MagGenresListBox->Check( Pos );
	}
	MagGenresSizer->Add( m_MagGenresListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * MagGenresBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_MagSelAllBtn = new wxButton( m_MagnatunePanel, wxID_ANY, _( "All" ), wxDefaultPosition, wxDefaultSize, 0 );
	MagGenresBtnSizer->Add( m_MagSelAllBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_MagSelNoneBtn = new wxButton( m_MagnatunePanel, wxID_ANY, _("None"), wxDefaultPosition, wxDefaultSize, 0 );
	MagGenresBtnSizer->Add( m_MagSelNoneBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_MagInvertBtn = new wxButton( m_MagnatunePanel, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
	MagGenresBtnSizer->Add( m_MagInvertBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	MagGenresSizer->Add( MagGenresBtnSizer, 0, wxEXPAND, 5 );

	MagMainSizer->Add( MagGenresSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * MagOtherSizer = new wxStaticBoxSizer( new wxStaticBox( m_MagnatunePanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	wxFlexGridSizer * MagFlexSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	MagFlexSizer->AddGrowableCol( 1 );
	MagFlexSizer->SetFlexibleDirection( wxBOTH );
	MagFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * MagMemberLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Membership :" ), wxDefaultPosition, wxDefaultSize, 0 );
	MagMemberLabel->Wrap( -1 );
	MagFlexSizer->Add( MagMemberLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxBoxSizer * MagMembSizer = new wxBoxSizer( wxHORIZONTAL );

	m_MagNoRadioItem = new wxRadioButton( m_MagnatunePanel, wxID_ANY, _( "None" ), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_MagStRadioItem = new wxRadioButton( m_MagnatunePanel, wxID_ANY, _( "Streaming" ), wxDefaultPosition, wxDefaultSize );
	m_MagDlRadioItem = new wxRadioButton( m_MagnatunePanel, wxID_ANY, _( "Downloading" ), wxDefaultPosition, wxDefaultSize );
	int Membership = m_Config->ReadNum( wxT( "Membership" ), 0, wxT( "Magnatune" ) );
	if( Membership == 1 )
        m_MagStRadioItem->SetValue( true );
    else if( Membership == 2 )
        m_MagDlRadioItem->SetValue( true );
    else
        m_MagNoRadioItem->SetValue( true );
	MagMembSizer->Add( m_MagNoRadioItem, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	MagMembSizer->Add( m_MagStRadioItem, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	MagMembSizer->Add( m_MagDlRadioItem, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	MagFlexSizer->Add( MagMembSizer, 1, wxEXPAND, 5 );

	wxStaticText * MagUserLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Username :" ), wxDefaultPosition, wxDefaultSize, 0 );
	MagUserLabel->Wrap( -1 );
	MagFlexSizer->Add( MagUserLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_MagUserTextCtrl = new wxTextCtrl( m_MagnatunePanel, wxID_ANY, m_Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "Magnatune" ) ), wxDefaultPosition, wxDefaultSize, 0 );
    m_MagUserTextCtrl->Enable( !m_MagNoRadioItem->GetValue() );
	MagFlexSizer->Add( m_MagUserTextCtrl, 0, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MagPassLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Password :" ), wxDefaultPosition, wxDefaultSize, 0 );
	MagPassLabel->Wrap( -1 );
	MagFlexSizer->Add( MagPassLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_RIGHT, 5 );

	m_MagPassTextCtrl = new wxTextCtrl( m_MagnatunePanel, wxID_ANY, m_Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "Magnatune" ) ), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_MagPassTextCtrl->Enable( !m_MagNoRadioItem->GetValue() );
	MagFlexSizer->Add( m_MagPassTextCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

//	wxStaticText * MagFormatLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Format :" ), wxDefaultPosition, wxDefaultSize, 0 );
//	MagFormatLabel->Wrap( -1 );
//	MagFlexSizer->Add( MagFormatLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
//
//	wxArrayString MagFormatChoices;
//	MagFormatChoices.Add( wxT( "mp3" ) );
//	MagFormatChoices.Add( wxT( "ogg" ) );
//	m_MagFormatChoice = new wxChoice( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, MagFormatChoices, 0 );
//	m_MagFormatChoice->SetSelection( m_Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Magnatune" ) ) );
//	MagFlexSizer->Add( m_MagFormatChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	wxStaticText * MagFormatLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Stream as :" ), wxDefaultPosition, wxDefaultSize, 0 );
	MagFormatLabel->Wrap( -1 );
	MagFlexSizer->Add( MagFormatLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxBoxSizer * FormatSizer = new wxBoxSizer( wxHORIZONTAL );

	wxString m_MagFormatChoiceChoices[] = { wxT("mp3"), wxT("ogg") };
	int m_MagFormatChoiceNChoices = sizeof( m_MagFormatChoiceChoices ) / sizeof( wxString );
	m_MagFormatChoice = new wxChoice( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MagFormatChoiceNChoices, m_MagFormatChoiceChoices, 0 );
	m_MagFormatChoice->SetSelection( m_Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Magnatune" ) ) );
	m_MagFormatChoice->SetMinSize( wxSize( 100,-1 ) );

	FormatSizer->Add( m_MagFormatChoice, 1, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

	wxStaticText * MagDownLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _("Download as :"), wxDefaultPosition, wxDefaultSize, 0 );
	MagDownLabel->Wrap( -1 );
	FormatSizer->Add( MagDownLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxString m_MagDownFormatChoiceChoices[] = { _( "Download Page" ), wxT("mp3 (VBR)"), wxT("mp3 (128Kbits)"), wxT("ogg"), wxT("flac"), wxT("wav") };
	int m_MagDownFormatChoiceNChoices = sizeof( m_MagDownFormatChoiceChoices ) / sizeof( wxString );
	m_MagDownFormatChoice = new wxChoice( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MagDownFormatChoiceNChoices, m_MagDownFormatChoiceChoices, 0 );
	m_MagDownFormatChoice->SetSelection( m_Config->ReadNum( wxT( "DownloadFormat" ), 0, wxT( "Magnatune" ) ) );
	m_MagDownFormatChoice->Enable( Membership == 2 );
	m_MagDownFormatChoice->SetMinSize( wxSize( 100,-1 ) );

	FormatSizer->Add( m_MagDownFormatChoice, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	MagFlexSizer->Add( FormatSizer, 1, wxEXPAND, 5 );

	MagOtherSizer->Add( MagFlexSizer, 1, wxEXPAND, 5 );

	MagMainSizer->Add( MagOtherSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_MagnatunePanel->SetSizer( MagMainSizer );
	m_MagnatunePanel->Layout();

	m_MagSelAllBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnMagnatuneSelectAll ), NULL, this );
	m_MagSelNoneBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnMagnatuneSelectNone ), NULL, this );
	m_MagInvertBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnMagnatuneInvertSelection ), NULL, this );
	m_MagNoRadioItem->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( guPrefDialog::OnMagNoRadioItemChanged ), NULL, this );
	m_MagStRadioItem->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( guPrefDialog::OnMagNoRadioItemChanged ), NULL, this );
	m_MagDlRadioItem->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( guPrefDialog::OnMagNoRadioItemChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildLinksPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LINKS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_LINKS;

    //
    // Links
    //
	wxBoxSizer * LinksMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * LinksLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Links ") ), wxVERTICAL );

	wxBoxSizer * LinksListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_LinksListBox = new wxListBox( m_LinksPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    wxArrayString SearchLinks = m_Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );
	m_LinksListBox->Append( SearchLinks );
	m_LinksNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "SearchLinks" ) );
    int count = m_LinksListBox->GetCount();
	while( ( int ) m_LinksNames.Count() < count )
        m_LinksNames.Add( wxEmptyString );
	while( ( int ) m_LinksNames.Count() > count )
        m_LinksNames.RemoveAt( count );

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

	wxBoxSizer * LinksBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LinksAddBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
    m_LinksAddBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveUpBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveUpBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveUpBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveDownBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveDownBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveDownBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksDelBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksDelBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksDelBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	LinksListBoxSizer->Add( LinksBtnSizer, 0, wxEXPAND, 5 );

	LinksLabelSizer->Add( LinksListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer * LinksEditorSizer = new wxBoxSizer( wxHORIZONTAL );

	wxFlexGridSizer * LinksFieldsSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	LinksFieldsSizer->AddGrowableCol( 1 );
	LinksFieldsSizer->SetFlexibleDirection( wxBOTH );
	LinksFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * LinkUrlStaticText = new wxStaticText( m_LinksPanel, wxID_ANY, _("Url:"), wxDefaultPosition, wxDefaultSize, 0 );
	LinkUrlStaticText->Wrap( -1 );
	LinksFieldsSizer->Add( LinkUrlStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_LinksUrlTextCtrl = new wxTextCtrl( m_LinksPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LinksFieldsSizer->Add( m_LinksUrlTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

	wxStaticText * LinkNameStaticText = new wxStaticText( m_LinksPanel, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
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

	wxStaticBoxSizer * LinksHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Help ") ), wxVERTICAL );

	wxStaticText * LinksHelpText = new wxStaticText( m_LinksPanel, wxID_ANY, _("Add urls using :\n{lang} : 2 lettes language code.\n{text} : Text to search."), wxDefaultPosition, wxDefaultSize, 0 );
	LinksHelpText->Wrap( -1 );
	LinksHelpSizer->Add( LinksHelpText, 0, wxALL, 5 );

	LinksMainSizer->Add( LinksHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_LinksPanel->SetSizer( LinksMainSizer );
	m_LinksPanel->Layout();

    //
    //
    //
	m_LinksListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
	m_LinksAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
	m_LinksDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
	m_LinksMoveUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
	m_LinksMoveDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
	m_LinksUrlTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildCommandsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COMMANDS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_COMMANDS;

    //
    // Commands Panel
    //
	wxBoxSizer * CmdMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * CmdLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_CmdPanel, wxID_ANY, _(" Commands ") ), wxVERTICAL );

	wxBoxSizer * CmdListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CmdListBox = new wxListBox( m_CmdPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    wxArrayString Commands = m_Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
	m_CmdListBox->Append( Commands );
	m_CmdNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "Commands" ) );
	int index;
    int count = m_CmdListBox->GetCount();
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

	wxBoxSizer * CmdBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_CmdAddBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CmdAddBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdMoveUpBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdMoveUpBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdMoveUpBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdMoveDownBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdMoveDownBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdMoveDownBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdDelBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdDelBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdDelBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CmdListBoxSizer->Add( CmdBtnSizer, 0, wxEXPAND, 5 );

	CmdLabelSizer->Add( CmdListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer * CmdEditorSizer = new wxBoxSizer( wxHORIZONTAL );

	wxFlexGridSizer * CmdFieldsSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	CmdFieldsSizer->AddGrowableCol( 1 );
	CmdFieldsSizer->SetFlexibleDirection( wxBOTH );
	CmdFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * CmdStaticText = new wxStaticText( m_CmdPanel, wxID_ANY, _( "Command:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CmdStaticText->Wrap( -1 );
	CmdFieldsSizer->Add( CmdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_CmdTextCtrl = new wxTextCtrl( m_CmdPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CmdFieldsSizer->Add( m_CmdTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * CmdNameStaticText = new wxStaticText( m_CmdPanel, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
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

	wxStaticBoxSizer * CmdHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_CmdPanel, wxID_ANY, _(" Help ") ), wxVERTICAL );

	wxStaticText * CmdHelpText = new wxStaticText( m_CmdPanel, wxID_ANY, _("Add commands using :\n{bp}\t: Album path\n{bc}\t: Album cover file path\n{tp}\t: Track path"), wxDefaultPosition, wxDefaultSize, 0 );
	CmdHelpText->Wrap( -1 );
	CmdHelpSizer->Add( CmdHelpText, 0, wxALL, 5 );

	CmdMainSizer->Add( CmdHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_CmdPanel->SetSizer( CmdMainSizer );
	m_CmdPanel->Layout();

    //
    //
    //
	m_CmdListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCmdListBoxSelected ), NULL, this );
	m_CmdAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdAddBtnClick ), NULL, this );
	m_CmdDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdDelBtnClick ), NULL, this );
	m_CmdMoveUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveUpBtnClick ), NULL, this );
	m_CmdMoveDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveDownBtnClick ), NULL, this );
	m_CmdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdSaveBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildCopyToPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COPYTO )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_COPYTO;

    //
    // Copy To patterns
    //
	wxBoxSizer * CopyToMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * CopyToLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Patterns ") ), wxVERTICAL );

	wxBoxSizer * CopyToListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CopyToListBox = new wxListBox( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );

	m_CopyToOptions = new guCopyToPatternArray();

	wxArrayString Options = m_Config->ReadAStr( wxT( "Option" ), wxEmptyString, wxT( "CopyTo" ) );
	wxArrayString Names;
    int Index;
    int Count;
	if( !Options.Count() )
	{
        wxArrayString Patterns = m_Config->ReadAStr( wxT( "Pattern" ), wxEmptyString, wxT( "CopyTo" ) );
        wxArrayString PatNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "CopyTo" ) );
        Count = wxMin( Patterns.Count(), PatNames.Count() );
        for( Index = 0; Index < Count; Index++ )
        {
            Options.Add( wxString::Format( wxT( "%s:%s:%i:%i:%i" ),
                escape_configlist_str( PatNames[ Index ] ).c_str(),
                escape_configlist_str( Patterns[ Index ] ).c_str(),
                guTRANSCODE_FORMAT_KEEP, guTRANSCODE_QUALITY_KEEP, false ) );
        }
	}

	if( ( Count = Options.Count() ) )
	{
	    for( Index = 0; Index < Count; Index++ )
	    {
	        guCopyToPattern * CopyToPattern = new guCopyToPattern( Options[ Index ] );
	        if( CopyToPattern )
	        {
	            m_CopyToOptions->Add( CopyToPattern );
	            Names.Add( CopyToPattern->m_Name );
	        }
	    }
	}

	m_CopyToListBox->Append( Names );

	CopyToListBoxSizer->Add( m_CopyToListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * CopyToBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_CopyToAddBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CopyToAddBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CopyToUpBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CopyToUpBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToUpBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CopyToDownBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CopyToDownBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToDownBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CopyToDelBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CopyToDelBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToDelBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CopyToListBoxSizer->Add( CopyToBtnSizer, 0, wxEXPAND, 5 );

	CopyToLabelSizer->Add( CopyToListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer* CopyToEditorSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer * CopyToOptionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Options ") ), wxHORIZONTAL );

	wxFlexGridSizer * CopyToFieldsSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	CopyToFieldsSizer->AddGrowableCol( 1 );
	CopyToFieldsSizer->SetFlexibleDirection( wxBOTH );
	CopyToFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * CopyToNameStaticText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToNameStaticText->Wrap( -1 );
	CopyToFieldsSizer->Add( CopyToNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_CopyToNameTextCtrl = new wxTextCtrl( m_CopyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFieldsSizer->Add( m_CopyToNameTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * CopyToPatternStaticText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Pattern:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToPatternStaticText->Wrap( -1 );
	CopyToFieldsSizer->Add( CopyToPatternStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_CopyToPatternTextCtrl = new wxTextCtrl( m_CopyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFieldsSizer->Add( m_CopyToPatternTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * CopyToFormatLabel = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Format:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFormatLabel->Wrap( -1 );
	CopyToFieldsSizer->Add( CopyToFormatLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

//	m_CopyToFormatChoice = new wxChoice( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeFormatStrings(), 0 );
//	m_CopyToFormatChoice->SetSelection( 0 );
//	CopyToFieldsSizer->Add( m_CopyToFormatChoice, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );
//
//	wxStaticText * CopyToQualityLabel = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Quality:" ), wxDefaultPosition, wxDefaultSize, 0 );
//	CopyToQualityLabel->Wrap( -1 );
//	CopyToFieldsSizer->Add( CopyToQualityLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
//
//	m_CopyToQualityChoice = new wxChoice( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeQualityStrings(), 0 );
//	m_CopyToQualityChoice->SetSelection( guTRANSCODE_QUALITY_KEEP );
//	m_CopyToQualityChoice->Enable( false );
//	CopyToFieldsSizer->Add( m_CopyToQualityChoice, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxBoxSizer* CopyToFormatSizer;
	CopyToFormatSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CopyToFormatChoice = new wxChoice( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeFormatStrings(), 0 );
	m_CopyToFormatChoice->SetSelection( 0 );
	CopyToFormatSizer->Add( m_CopyToFormatChoice, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	m_CopyToQualityChoice = new wxChoice( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeQualityStrings(), 0 );
	m_CopyToQualityChoice->SetSelection( guTRANSCODE_QUALITY_KEEP );
	m_CopyToQualityChoice->Enable( false );
	CopyToFormatSizer->Add( m_CopyToQualityChoice, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	CopyToFieldsSizer->Add( CopyToFormatSizer, 1, wxEXPAND, 5 );

	CopyToFieldsSizer->Add( 0, 0, 0, wxEXPAND, 5 );

	m_CopyToMoveFilesChkBox = new wxCheckBox( m_CopyPanel, wxID_ANY, _("Remove source files"), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFieldsSizer->Add( m_CopyToMoveFilesChkBox, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	CopyToOptionsSizer->Add( CopyToFieldsSizer, 1, wxEXPAND, 5 );

	CopyToEditorSizer->Add( CopyToOptionsSizer, 1, wxEXPAND|wxRIGHT, 5 );

	m_CopyToAcceptBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CopyToAcceptBtn->Enable( false );

	CopyToEditorSizer->Add( m_CopyToAcceptBtn, 0, wxALL|wxALIGN_BOTTOM, 5 );

	CopyToLabelSizer->Add( CopyToEditorSizer, 0, wxEXPAND, 5 );

	CopyToMainSizer->Add( CopyToLabelSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer* CopyToHelpSizer;
	CopyToHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Help ") ), wxVERTICAL );

	wxStaticText * CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, _("{a}\t: Artist\t\t\t{aa} : Album Artist\t\t{b}\t: Album\n{d}\t: Disk  \t\t\t{f}\t: Filename\t\t\t{g}\t: Genre\n{n}\t: Number\t\t\t{t}\t: Title\t\t\t\t{y}\t: Year"), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 0, wxALL, 5 );

	CopyToMainSizer->Add( CopyToHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_CopyPanel->SetSizer( CopyToMainSizer );
	m_CopyPanel->Layout();

	m_CopyToListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToListBoxSelected ), NULL, this );
	m_CopyToAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToAddBtnClick ), NULL, this );
	m_CopyToDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToDelBtnClick ), NULL, this );
	m_CopyToUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveUpBtnClick ), NULL, this );
	m_CopyToDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveDownBtnClick ), NULL, this );
	m_CopyToPatternTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
	m_CopyToNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
	m_CopyToFormatChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToFormatChanged ), NULL, this );
	m_CopyToQualityChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToQualityChanged ), NULL, this );
	m_CopyToMoveFilesChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveFilesChanged ), NULL, this );
	m_CopyToAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToSaveBtnClick ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildAcceleratorsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_ACCELERATORS;

    guAccelGetActionNames( m_AccelActionNames );

	m_AccelKeys = m_Config->ReadANum( wxT( "AccelKey"), 0, wxT( "Accelerators" ) );
	if( !m_AccelKeys.Count() )
	{
	    guAccelGetDefaultKeys( m_AccelKeys );
	}

	while( m_AccelKeys.Count() < m_AccelActionNames.Count() )
        m_AccelKeys.Add( 0 );

    int Index;
    int Count = m_AccelActionNames.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_AccelKeyNames.Add( guAccelGetKeyCodeString( m_AccelKeys[ Index ] ) );
    }

    //
    // Accelerators Panel
    //
	wxBoxSizer * AccelMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * AccelActionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_AccelPanel, wxID_ANY, _(" Accelerators ") ), wxVERTICAL );

	m_AccelListCtrl = new wxListCtrl( m_AccelPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_VRULES );

    wxListItem AccelColumn;
    AccelColumn.SetText( _( "Action" ) );
    AccelColumn.SetImage( wxNOT_FOUND );
    m_AccelListCtrl->InsertColumn( 0, AccelColumn );

    AccelColumn.SetText( _( "Key" ) );
    AccelColumn.SetAlign( wxLIST_FORMAT_RIGHT );
    m_AccelListCtrl->InsertColumn( 1, AccelColumn );

    m_AccelListCtrl->Hide();

    wxColour EveBgColor  = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );
    wxColour OddBgColor;
    if( EveBgColor.Red() > 0x0A && EveBgColor.Green() > 0x0A && EveBgColor.Blue() > 0x0A )
    {
        OddBgColor.Set( EveBgColor.Red() - 0xA, EveBgColor.Green() - 0x0A, EveBgColor.Blue() - 0x0A );
    }
    else
    {
        OddBgColor.Set( EveBgColor.Red() + 0xA, EveBgColor.Green() + 0x0A, EveBgColor.Blue() + 0x0A );
    }
    for( Index = 0; Index < Count; Index++ )
    {
        long NewItem = m_AccelListCtrl->InsertItem( Index, m_AccelActionNames[ Index ], 0 );
        m_AccelListCtrl->SetItemData( NewItem, m_AccelKeys[ Index ] );
        m_AccelListCtrl->SetItemBackgroundColour( NewItem, Index & 1 ? OddBgColor : EveBgColor );
        m_AccelListCtrl->SetItem( NewItem, 1, m_AccelKeyNames[ Index ] );
    }

    m_AccelListCtrl->Show();

    m_AccelListCtrl->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_AccelListCtrl->SetColumnWidth( 1, 200 );

	AccelActionsSizer->Add( m_AccelListCtrl, 1, wxEXPAND|wxALL, 5 );

	AccelMainSizer->Add( AccelActionsSizer, 1, wxEXPAND|wxALL, 5 );

	m_AccelPanel->SetSizer( AccelMainSizer );
	m_AccelPanel->Layout();

	m_AccelCurIndex = wxNOT_FOUND;
	m_AccelItemNeedClear = false;

	m_AccelListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( guPrefDialog::OnAccelSelected ), NULL, this );
	m_AccelListCtrl->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guPrefDialog::OnAccelKeyDown ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::SaveSettings( void )
{
    m_Config = ( guConfig * ) guConfig::Get();
    if( !m_Config )
        guLogError( wxT( "Invalid m_Config object in preferences dialog" ) );


    // Save all configurations
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_GENERAL )
    {
        m_Config->WriteBool( wxT( "ShowSplashScreen" ), m_ShowSplashChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "StartMinimized" ), m_MinStartChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "LoadDefaultLayouts" ), m_IgnoreLayoutsChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "ShowTaskBarIcon" ), m_TaskIconChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "CloseToTaskBar" ), m_TaskIconChkBox->IsChecked() && m_CloseTaskBarChkBox->GetValue(), wxT( "General" ) );
#ifdef WITH_LIBINDICATE_SUPPORT
        m_Config->WriteBool( wxT( "SoundMenuIntegration" ), m_SoundMenuChkBox->GetValue(), wxT( "General" ) );
#endif
        m_Config->WriteBool( wxT( "DefaultActionEnqueue" ), m_EnqueueChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "DropFilesClearPlaylist" ), m_DropFilesChkBox->GetValue(), wxT( "General" ) );
        //m_Config->WriteNum( wxT( "AlbumYearOrder" ), m_AlYearOrderChoice->GetSelection(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "SavePlayListOnClose" ), m_SavePlayListChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "SaveCurrentTrackPos" ), m_SavePosCheckBox->GetValue(), wxT( "General" ) );
        m_Config->WriteNum( wxT( "MinSavePlayPosLength" ), m_MinLenSpinCtrl->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "ShowCloseConfirm" ), m_ExitConfirmChkBox->GetValue(), wxT( "General" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LIBRARY )
    {
        m_Config->WriteAStr( wxT( "LibPath" ), m_PathsListBox->GetStrings(), wxT( "LibPaths" ) );
        if( m_LibPathsChanged )
        {
            m_Config->WriteStr( wxT( "LastUpdate" ), wxEmptyString, wxT( "General" ) );
        }

        m_Config->WriteAStr( wxT( "Word" ), m_CoversListBox->GetStrings(), wxT( "CoverSearch" ) );
        m_Config->WriteBool( wxT( "UpdateLibOnStart" ), m_UpdateLibChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "ScanAddPlayLists" ), m_LibScanPlayListChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "ScanSymlinks" ), m_LibScanSymlinksChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "ScanEmbeddedCovers" ), m_LibScanEmbCoversChkBox->GetValue(), wxT( "General" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PLAYBACK )
    {
        m_Config->WriteBool( wxT( "RndPlayOnEmptyPlayList" ), m_RndPlayChkBox->GetValue(), wxT( "General" ) );
        m_Config->WriteNum( wxT( "RndModeOnEmptyPlayList" ), m_RndModeChoice->GetSelection(), wxT( "General" ) );
        m_Config->WriteBool( wxT( "DelTracksPlayed" ), m_DelPlayChkBox->GetValue(), wxT( "Playback" ) );
        m_Config->WriteNum( wxT( "ReplayGainMode" ), m_PlayReplayModeChoice->GetSelection(), wxT( "General" ) );

        m_Config->WriteBool( wxT( "SilenceDetector" ), m_PlayLevelEnabled->GetValue(), wxT( "Playback" ) );
        m_Config->WriteNum( wxT( "SilenceLevel" ), m_PlayLevelSlider->GetValue(), wxT( "Playback" ) );
        m_Config->WriteBool( wxT( "SilenceAtEnd" ), m_PlayEndTimeCheckBox->GetValue(), wxT( "Playback" ) );
        m_Config->WriteNum( wxT( "SilenceEndTime" ), m_PlayEndTimeSpinCtrl->GetValue(), wxT( "Playback" ) );
        m_Config->WriteBool( wxT( "ShowNotifications" ), m_NotifyChkBox->GetValue(), wxT( "General" ) );

        m_Config->WriteNum( wxT( "MinTracksToPlay" ), m_MinTracksSpinCtrl->GetValue(), wxT( "Playback" ) );
        m_Config->WriteNum( wxT( "NumTracksToAdd" ), m_NumTracksSpinCtrl->GetValue(), wxT( "Playback" ) );
        m_Config->WriteNum( wxT( "MaxTracksPlayed" ), m_MaxTracksPlayed->GetValue(), wxT( "Playback" ) );
        m_Config->WriteNum( wxT( "SmartFilterArtists" ), m_SmartPlayArtistsSpinCtrl->GetValue(), wxT( "Playback" ) );
        m_Config->WriteNum( wxT( "SmartFilterTracks" ), m_SmartPlayTracksSpinCtrl->GetValue(), wxT( "Playback" ) );

        m_Config->WriteNum( wxT( "OutputDevice" ), m_PlayOutDevChoice->GetSelection(), wxT( "Playback" ) );
        m_Config->WriteStr( wxT( "OutputDeviceName" ), m_PlayOutDevName->GetValue(), wxT( "Playback" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_CROSSFADER )
    {
        m_Config->WriteNum( wxT( "FadeOutTime" ), m_XFadeOutLenSlider->GetValue(), wxT( "Crossfader" ) );
        m_Config->WriteNum( wxT( "FadeInTime" ), m_XFadeInLenSlider->GetValue(), wxT( "Crossfader" ) );
        m_Config->WriteNum( wxT( "FadeInVolStart" ), m_XFadeInStartSlider->GetValue(), wxT( "Crossfader" ) );
        m_Config->WriteNum( wxT( "FadeInVolTriger" ), m_XFadeInTrigerSlider->GetValue(), wxT( "Crossfader" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
    {
        m_Config->WriteBool( wxT( "SubmitEnabled" ), m_LastFMASEnableChkBox->IsEnabled() && m_LastFMASEnableChkBox->GetValue(), wxT( "LastFM" ) );
        m_Config->WriteStr( wxT( "UserName" ), m_LastFMUserNameTextCtrl->GetValue(), wxT( "LastFM" ) );
        if( !m_LastFMPasswdTextCtrl->IsEmpty() && m_LastFMPasswdTextCtrl->GetValue() != wxT( "******" ) )
        {
            guMD5 MD5;
            m_Config->WriteStr( wxT( "Password" ), MD5.MD5( m_LastFMPasswdTextCtrl->GetValue() ), wxT( "LastFM" ) );
            //guLogMessage( wxT( "Pass: %s" ), PasswdTextCtrl->GetValue().c_str() );
            //guLogMessage( wxT( "MD5 : %s" ), MD5.MD5( PasswdTextCtrl->GetValue() ).c_str() );
        }
        m_Config->WriteBool( wxT( "SubmitEnabled" ), m_LibreFMASEnableChkBox->IsEnabled() && m_LibreFMASEnableChkBox->GetValue(), wxT( "LibreFM" ) );
        m_Config->WriteStr( wxT( "UserName" ), m_LibreFMUserNameTextCtrl->GetValue(), wxT( "LibreFM" ) );
        if( !m_LibreFMPasswdTextCtrl->IsEmpty() && m_LibreFMPasswdTextCtrl->GetValue() != wxT( "******" ) )
        {
            guMD5 MD5;
            m_Config->WriteStr( wxT( "Password" ), MD5.MD5( m_LibreFMPasswdTextCtrl->GetValue() ), wxT( "LibreFM" ) );
        }
    }

    // LastFM Panel Info language
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ONLINE )
    {
        m_Config->WriteStr( wxT( "Language" ), m_LangIds[ m_LangChoice->GetSelection() ], wxT( "LastFM" ) );

        m_Config->WriteAStr( wxT( "Filter" ), m_OnlineFiltersListBox->GetStrings(), wxT( "SearchFilters" ) );

        m_Config->WriteStr( wxT( "BrowserCommand" ), m_BrowserCmdTextCtrl->GetValue(), wxT( "General" ) );
        m_Config->WriteStr( wxT( "RadioMinBitRate" ), m_RadioMinBitRateRadBoxChoices[ m_RadioMinBitRateRadBox->GetSelection() ], wxT( "Radios" ) );
        m_Config->WriteNum( wxT( "BufferSize" ), m_BufferSizeSlider->GetValue(), wxT( "General" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_RECORD )
    {
        m_Config->WriteBool( wxT( "Enabled" ), m_RecordChkBox->GetValue(), wxT( "Record" ) );
        m_Config->WriteStr( wxT( "Path" ), m_RecSelDirPicker->GetPath(), wxT( "Record" ) );
        m_Config->WriteNum( wxT( "Format" ), m_RecFormatChoice->GetSelection(), wxT( "Record" ) );
        m_Config->WriteNum( wxT( "Quality" ), m_RecQualityChoice->GetSelection(), wxT( "Record" ) );
        m_Config->WriteBool( wxT( "Split" ), m_RecSplitChkBox->GetValue(), wxT( "Record" ) );
        m_Config->WriteBool( wxT( "DeleteTracks" ), m_RecDelTracks->GetValue(), wxT( "Record" ) );
        m_Config->WriteNum( wxT( "DeleteTime" ), m_RecDelTime->GetValue(), wxT( "Record" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PODCASTS )
    {
        m_Config->WriteStr( wxT( "Path" ), m_PodcastPath->GetPath(), wxT( "Podcasts" ) );
        m_Config->WriteBool( wxT( "Update" ), m_PodcastUpdate->GetValue(), wxT( "Podcasts" ) );
        m_Config->WriteNum( wxT( "UpdatePeriod" ), m_PodcastUpdatePeriod->GetSelection(), wxT( "Podcasts" ) );
        m_Config->WriteBool( wxT( "Delete" ), m_PodcastDelete->GetValue(), wxT( "Podcasts" ) );
        m_Config->WriteNum( wxT( "DeleteTime" ), m_PodcastDeleteTime->GetValue(), wxT( "Podcasts" ) );
        m_Config->WriteNum( wxT( "DeletePeriod" ), m_PodcastDeletePeriod->GetSelection(), wxT( "Podcasts" ) );
        m_Config->WriteBool( wxT( "DeletePlayed" ), m_PodcastDeletePlayed->GetValue(), wxT( "Podcasts" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LYRICS )
    {
        m_LyricSearchEngine->Save();
//        m_Config->WriteBool( wxT( "EmbedToFiles" ), m_LyricsEmbedChkBox->GetValue(), wxT( "Lyrics" ) );
//        m_Config->WriteBool( wxT( "SaveToFiles" ), m_LyricsSaveToFileChkBox->GetValue(), wxT( "Lyrics" ) );
//        m_Config->WriteStr( wxT( "SaveToFilesPath" ), m_LyricsDirSavePicker->GetPath(), wxT( "Lyrics" ) );
//        m_Config->WriteStr( wxT( "SaveToFilesPattern" ), m_LyricsPatternTextCtrl->GetValue(), wxT( "Lyrics" ) );
//        m_Config->WriteBool( wxT( "SaveCommandEnabled" ), m_LyricsEmbedChkBox->GetValue(), wxT( "Lyrics" ) );
//        m_Config->WriteStr( wxT( "SaveCommandText" ), m_LyricCmdTextCtrl->GetValue(), wxT( "Lyrics" ) );
        m_Config->WriteStr( wxT( "Font" ), m_LyricFontPicker->GetSelectedFont().GetNativeFontInfoDesc(), wxT( "Lyrics" ) );
        m_Config->WriteNum( wxT( "TextAlign" ), m_LyricsAlignChoice->GetSelection(), wxT( "Lyrics" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_JAMENDO )
    {
        wxArrayInt EnabledGenres;
        int Index;
        int Count = m_JamGenresListBox->GetCount();
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_JamGenresListBox->IsChecked( Index ) )
                EnabledGenres.Add( Index );
        }

        m_Config->WriteANum( wxT( "Genre" ), EnabledGenres, wxT( "JamendoGenres" ) );
        bool DoUpgrade = ( EnabledGenres.Count() != m_LastJamendoGenres.Count() );
        if( !DoUpgrade )
        {
            Count = EnabledGenres.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( m_LastJamendoGenres.Index( EnabledGenres[ Index ] ) == wxNOT_FOUND )
                {
                    DoUpgrade = true;
                    break;
                }
            }
        }

        m_Config->WriteBool( wxT( "NeedUpgrade" ), DoUpgrade, wxT( "Jamendo" ) );
        m_Config->WriteNum( wxT( "AudioFormat" ), m_JamFormatChoice->GetSelection(), wxT( "Jamendo" ) );
        m_Config->WriteStr( wxT( "TorrentCommand" ), m_JamBTCmd->GetValue(), wxT( "Jamendo" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_MAGNATUNE )
    {
        wxArrayString EnabledGenres;
        int Index;
        int Count = m_MagGenresListBox->GetCount();
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_MagGenresListBox->IsChecked( Index ) )
                EnabledGenres.Add( m_MagGenresListBox->GetString( Index ) );
        }
        m_Config->WriteAStr( wxT( "Genre" ), EnabledGenres, wxT( "MagnatuneGenres" ) );

        bool DoUpgrade = ( EnabledGenres.Count() != m_LastMagnatuneGenres.Count() );
        if( !DoUpgrade )
        {
            Count = EnabledGenres.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( m_LastMagnatuneGenres.Index( EnabledGenres[ Index ] ) == wxNOT_FOUND )
                {
                    DoUpgrade = true;
                    break;
                }
            }
        }

        m_Config->WriteBool( wxT( "NeedUpgrade" ), DoUpgrade, wxT( "Magnatune" ) );
        if( m_MagNoRadioItem->GetValue() )
            m_Config->WriteNum( wxT( "Membership" ), 0, wxT( "Magnatune" ) );
        else if( m_MagStRadioItem->GetValue() )
            m_Config->WriteNum( wxT( "Membership" ), 1, wxT( "Magnatune" ) );
        else
            m_Config->WriteNum( wxT( "Membership" ), 2, wxT( "Magnatune" ) );
        m_Config->WriteStr( wxT( "UserName" ), m_MagUserTextCtrl->GetValue(), wxT( "Magnatune" ) );
        m_Config->WriteStr( wxT( "Password" ), m_MagPassTextCtrl->GetValue(), wxT( "Magnatune" ) );
        m_Config->WriteNum( wxT( "AudioFormat" ), m_MagFormatChoice->GetSelection(), wxT( "Magnatune" ) );
        m_Config->WriteNum( wxT( "DownloadFormat" ), m_MagDownFormatChoice->GetSelection(), wxT( "Magnatune" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LINKS )
    {
        wxArrayString SearchLinks = m_LinksListBox->GetStrings();
        m_Config->WriteAStr( wxT( "Link" ), SearchLinks, wxT( "SearchLinks" ) );
        m_Config->WriteAStr( wxT( "Name" ), m_LinksNames, wxT( "SearchLinks" ), false );

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
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COMMANDS )
    {
        wxArrayString Commands = m_CmdListBox->GetStrings();
        m_Config->WriteAStr( wxT( "Cmd" ), Commands, wxT( "Commands" ) );
        m_Config->WriteAStr( wxT( "Name" ), m_CmdNames, wxT( "Commands" ), false );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COPYTO )
    {
        wxArrayString Options;
        int Index;
        int Count = m_CopyToOptions->Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guCopyToPattern &CopyToPattern = m_CopyToOptions->Item( Index );
            Options.Add( CopyToPattern.ToString() );
        }

        m_Config->WriteAStr( wxT( "Option" ), Options, wxT( "CopyTo" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        m_Config->WriteANum( wxT( "AccelKey" ), m_AccelKeys, wxT( "Accelerators" ) );
    }

    m_Config->Flush();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnActivateTaskBarIcon( wxCommandEvent& event )
{
    if( m_SoundMenuChkBox )
        m_SoundMenuChkBox->Enable( m_TaskIconChkBox->IsChecked() );
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() && ( !m_SoundMenuChkBox || !m_SoundMenuChkBox->IsChecked() ) );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnActivateSoundMenuIntegration( wxCommandEvent& event )
{
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() && !m_SoundMenuChkBox->IsChecked() );
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
void guPrefDialog::OnLyricSourceSelected( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Selected %i" ), event.GetInt() );
    m_LyricSourceSelected = event.GetInt();
    m_LyricsUpButton->Enable( m_LyricSourceSelected > 0 );
    m_LyricsDownButton->Enable( m_LyricSourceSelected >= 0 && ( m_LyricSourceSelected < ( ( int ) m_LyricSearchEngine->SourcesCount() - 1 ) ) );
    m_LyricsDelButton->Enable( m_LyricSourceSelected != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSourceDClicked( wxCommandEvent &event )
{
    guLyricSource * CurrentLyricSource = m_LyricSearchEngine->GetSource( m_LyricSourceSelected );
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, CurrentLyricSource );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            bool WasActive = m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected );
            m_LyricsSrcListBox->SetString( m_LyricSourceSelected, CurrentLyricSource->Name() );
            m_LyricsSrcListBox->Check( m_LyricSourceSelected, WasActive );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSourceToggled( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Toggled %i %i" ), event.GetInt(), m_LyricsSrcListBox->IsChecked( event.GetInt() ) );
    m_LyricSourceSelected = event.GetInt();
    guLyricSource * LyricSource = m_LyricSearchEngine->GetSource( m_LyricSourceSelected );
    LyricSource->Enabled( m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected ) );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricAddBtnClick( wxCommandEvent &event )
{
    guLyricSource LyricSource;
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, &LyricSource );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            m_LyricsSrcListBox->Append( LyricSource.Name() );
            m_LyricSearchEngine->SourceAdd( new guLyricSource( LyricSource ) );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricUpBtnClick( wxCommandEvent &event )
{
    wxString LyricSource = m_LyricsSrcListBox->GetString( m_LyricSourceSelected );
    bool     LyricChecked = m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected );

    m_LyricSearchEngine->SourceMoveUp( m_LyricSourceSelected );

    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, m_LyricsSrcListBox->GetString( m_LyricSourceSelected - 1 ) );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected - 1 ) );
    m_LyricSourceSelected--;
    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, LyricSource );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, LyricChecked );
    m_LyricsSrcListBox->SetSelection( m_LyricSourceSelected );


    event.SetInt( m_LyricSourceSelected );
    OnLyricSourceSelected( event );

}
// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricDownBtnClick( wxCommandEvent &event )
{
    wxString LyricSource = m_LyricsSrcListBox->GetString( m_LyricSourceSelected );
    bool     LyricChecked = m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected );

    m_LyricSearchEngine->SourceMoveDown( m_LyricSourceSelected );

    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, m_LyricsSrcListBox->GetString( m_LyricSourceSelected + 1 ) );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected + 1 ) );
    m_LyricSourceSelected++;
    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, LyricSource );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, LyricChecked );
    m_LyricsSrcListBox->SetSelection( m_LyricSourceSelected );

    event.SetInt( m_LyricSourceSelected );
    OnLyricSourceSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricDelBtnClick( wxCommandEvent &event )
{
    if( m_LyricSourceSelected != wxNOT_FOUND )
    {
        m_LyricSearchEngine->SourceRemoveAt( m_LyricSourceSelected );
        m_LyricsSrcListBox->Delete( m_LyricSourceSelected );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveSelected( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Selected %i" ), event.GetInt() );
    m_LyricTargetSelected = event.GetInt();
    m_LyricsSaveUpButton->Enable( m_LyricTargetSelected > 0 );
    m_LyricsSaveDownButton->Enable( m_LyricTargetSelected >= 0 && ( m_LyricTargetSelected < ( ( int ) m_LyricSearchEngine->TargetsCount() - 1 ) ) );
    m_LyricsSaveDelButton->Enable( m_LyricTargetSelected != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveDClicked( wxCommandEvent &event )
{
    guLyricSource * CurrentLyricTarget = m_LyricSearchEngine->GetTarget( m_LyricTargetSelected );
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, CurrentLyricTarget, true );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            bool WasActive = m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected );
            m_LyricsSaveListBox->SetString( m_LyricTargetSelected, CurrentLyricTarget->Name() );
            m_LyricsSaveListBox->Check( m_LyricTargetSelected, WasActive );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveToggled( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Toggled %i %i" ), event.GetInt(), m_LyricsSaveListBox->IsChecked( event.GetInt() ) );
    m_LyricTargetSelected = event.GetInt();
    guLyricSource * LyricTarget = m_LyricSearchEngine->GetTarget( m_LyricTargetSelected );
    LyricTarget->Enabled( m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected ) );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveAddBtnClick( wxCommandEvent &event )
{
    guLyricSource LyricTarget;
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, &LyricTarget, true );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            m_LyricsSaveListBox->Append( LyricTarget.Name() );
            m_LyricSearchEngine->TargetAdd( new guLyricSource( LyricTarget ) );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveUpBtnClick( wxCommandEvent &event )
{
    wxString LyricTarget = m_LyricsSaveListBox->GetString( m_LyricTargetSelected );
    bool     LyricChecked = m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected );

    m_LyricSearchEngine->TargetMoveUp( m_LyricTargetSelected );

    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, m_LyricsSaveListBox->GetString( m_LyricTargetSelected - 1 ) );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected - 1 ) );
    m_LyricTargetSelected--;
    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, LyricTarget );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, LyricChecked );
    m_LyricsSaveListBox->SetSelection( m_LyricTargetSelected );


    event.SetInt( m_LyricTargetSelected );
    OnLyricSourceSelected( event );

}
// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveDownBtnClick( wxCommandEvent &event )
{
    wxString LyricTarget = m_LyricsSaveListBox->GetString( m_LyricTargetSelected );
    bool     LyricChecked = m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected );

    m_LyricSearchEngine->TargetMoveDown( m_LyricTargetSelected );

    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, m_LyricsSaveListBox->GetString( m_LyricTargetSelected + 1 ) );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected + 1 ) );
    m_LyricTargetSelected++;
    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, LyricTarget );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, LyricChecked );
    m_LyricsSaveListBox->SetSelection( m_LyricTargetSelected );

    event.SetInt( m_LyricTargetSelected );
    OnLyricSourceSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveDelBtnClick( wxCommandEvent &event )
{
    if( m_LyricTargetSelected != wxNOT_FOUND )
    {
        m_LyricSearchEngine->TargetRemoveAt( m_LyricTargetSelected );
        m_LyricsSaveListBox->Delete( m_LyricTargetSelected );
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
	m_PlayLevelVal->Enable( event.IsChecked() );
	m_PlayEndTimeCheckBox->Enable( event.IsChecked() );
	m_PlayEndTimeSpinCtrl->Enable( event.IsChecked() && m_PlayEndTimeCheckBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayLevelValueChanged( wxScrollEvent &event )
{
    int Value = m_PlayLevelSlider->GetValue();
    m_PlayLevelVal->SetLabel( wxString::Format( wxT( "%02idb" ), Value ) );
    m_PlayLevelVal->GetParent()->GetSizer()->Layout();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayEndTimeEnabled( wxCommandEvent& event )
{
	m_PlayEndTimeSpinCtrl->Enable( event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayOutDevChanged( wxCommandEvent& event )
{
    m_PlayOutDevName->Enable( event.GetInt() > guOUTPUT_DEVICE_GCONF );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCrossFadeChanged( wxScrollEvent& event )
{
    bool IsEnabled = m_XFadeOutLenSlider->GetValue();
    m_XFadeInLenSlider->Enable( IsEnabled );
    m_XFadeInStartSlider->Enable( IsEnabled );
    m_XFadeInTrigerSlider->Enable( IsEnabled );

    double FadeOutLen = double( m_XFadeOutLenSlider->GetValue() ) / 10.0;
    double FadeInLen = double( m_XFadeInLenSlider->GetValue() ) / 10.0;
    double FadeInTriger = double( m_XFadeInTrigerSlider->GetValue() ) / 10.0;
    double FadeInVolStart = double( m_XFadeInStartSlider->GetValue() ) / 10.0;

    m_XFadeOutLenVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeOutLen ) );
    m_XFadeInLenVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeInLen ) );
    m_XFadeInStartVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeInVolStart ) );
    m_XFadeTrigerVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeInTriger ) );

    m_XFadeInLenVal->Enable( IsEnabled );
    m_XFadeInStartVal->Enable( IsEnabled );
    m_XFadeTrigerVal->Enable( IsEnabled );

    m_XFadeTrigerVal->GetParent()->GetSizer()->Layout();

    wxBitmap * FadeBitmap = new wxBitmap( 400, 200 );
    if( FadeBitmap )
    {
        if( FadeBitmap->IsOk() )
        {
            wxMemoryDC MemDC;
            MemDC.SelectObject( * FadeBitmap );
            MemDC.Clear();

            wxRect Rect( 0, 0, 400, 200 );

            wxPoint FadeOutPoints[ 4 ];
            FadeOutPoints[ 0 ] = wxPoint( 0, 0 );
            FadeOutPoints[ 1 ] = wxPoint( 0, 200 );
            FadeOutPoints[ 2 ] = wxPoint( ( FadeOutLen + 1 ) * 20, 200 );
            FadeOutPoints[ 3 ] = wxPoint( 20, 0 );
            wxRegion OutRegion( WXSIZEOF( FadeOutPoints ), FadeOutPoints );

            wxPoint FadeInPoints[ 5 ];
            int FadeInStartX = FadeOutPoints[ 2 ].x - ( FadeInTriger * ( ( FadeOutPoints[ 2 ].x - 20 ) / 10 ) );
            int FadeInStartY = IsEnabled ? ( 200 - ( FadeInVolStart * 20 ) ) : 0;
            FadeInPoints[ 0 ] = wxPoint( FadeInStartX, 200 );
            FadeInPoints[ 1 ] = wxPoint( FadeInStartX, FadeInStartY );
            FadeInPoints[ 2 ] = wxPoint( FadeInStartX + ( FadeInLen * 20 ), 0 );
            FadeInPoints[ 3 ] = wxPoint( 400, 0 );
            FadeInPoints[ 4 ] = wxPoint( 400, 200 );
            wxRegion InRegion( WXSIZEOF( FadeInPoints ), FadeInPoints );
            MemDC.SetClippingRegion( InRegion );
            MemDC.GradientFillLinear( Rect, * wxLIGHT_GREY, * wxGREEN, wxRIGHT );
            MemDC.DestroyClippingRegion();

            MemDC.SetClippingRegion( OutRegion );
            MemDC.GradientFillLinear( Rect, wxColour( 0, 200, 200 ), wxColour( 0, 128, 128 ), wxRIGHT );
            MemDC.DestroyClippingRegion();

            OutRegion.Subtract( InRegion );
            MemDC.SetClippingRegion( OutRegion );
            MemDC.GradientFillLinear( Rect, * wxBLUE, * wxLIGHT_GREY, wxRIGHT );
            MemDC.DestroyClippingRegion();

            m_FadeBitmap->SetBitmap( * FadeBitmap );
            m_FadeBitmap->Refresh();
        }
        delete FadeBitmap;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnRecEnableClicked( wxCommandEvent& event )
{
	m_RecSelDirPicker->Enable( event.IsChecked() );
	m_RecFormatChoice->Enable( event.IsChecked() );
	m_RecQualityChoice->Enable( event.IsChecked() );
	m_RecSplitChkBox->Enable( event.IsChecked() );
	m_RecDelTracks->Enable( event.IsChecked() );
	m_RecDelTime->Enable( event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnRecDelTracksClicked( wxCommandEvent& event )
{
	m_RecDelTime->Enable( event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLastFMASUserNameChanged( wxCommandEvent &event )
{
    if( m_LastFMUserNameTextCtrl->IsEmpty() || m_LastFMPasswdTextCtrl->IsEmpty() )
    {
        m_LastFMASEnableChkBox->Disable();
    }
    else
    {
        m_LastFMASEnableChkBox->Enable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibreFMASUserNameChanged( wxCommandEvent &event )
{
    if( m_LibreFMUserNameTextCtrl->IsEmpty() || m_LibreFMPasswdTextCtrl->IsEmpty() )
    {
        m_LibreFMASEnableChkBox->Disable();
    }
    else
    {
        m_LibreFMASEnableChkBox->Enable();
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
void guPrefDialog::OnJamendoSelectAll( wxCommandEvent& event )
{
    int Index;
    int Count = m_JamGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_JamGenresListBox->Check( Index );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnJamendoSelectNone( wxCommandEvent& event )
{
    int Index;
    int Count = m_JamGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_JamGenresListBox->Check( Index, false );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnJamendoInvertSelection( wxCommandEvent& event )
{
    int Index;
    int Count = m_JamGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_JamGenresListBox->Check( Index, !m_JamGenresListBox->IsChecked( Index ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagnatuneSelectAll( wxCommandEvent& event )
{
    int Index;
    int Count = m_MagGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_MagGenresListBox->Check( Index );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagnatuneSelectNone( wxCommandEvent& event )
{
    int Index;
    int Count = m_MagGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_MagGenresListBox->Check( Index, false );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagnatuneInvertSelection( wxCommandEvent& event )
{
    int Index;
    int Count = m_MagGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_MagGenresListBox->Check( Index, !m_MagGenresListBox->IsChecked( Index ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagNoRadioItemChanged( wxCommandEvent& event )
{
    bool Enabled = !m_MagNoRadioItem->GetValue();
    m_MagUserTextCtrl->Enable( Enabled );
    m_MagPassTextCtrl->Enable( Enabled );
    m_MagDownFormatChoice->Enable( m_MagDlRadioItem->GetValue() );
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
void guPrefDialog::OnCopyToListBoxSelected( wxCommandEvent &event )
{
    m_CopyToSelected = event.GetInt();
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        m_CopyToDelBtn->Enable();

        if( m_CopyToSelected > 0 )
            m_CopyToUpBtn->Enable();
        else
            m_CopyToUpBtn->Disable();

        if( m_CopyToSelected < ( int ) ( m_CopyToListBox->GetCount() - 1 ) )
            m_CopyToDownBtn->Enable();
        else
            m_CopyToDownBtn->Disable();

        guCopyToPattern &CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
        m_CopyToNameTextCtrl->SetValue( CopyToPattern.m_Name );
        m_CopyToPatternTextCtrl->SetValue( CopyToPattern.m_Pattern );
        m_CopyToFormatChoice->SetSelection( CopyToPattern.m_Format );
        m_CopyToQualityChoice->SetSelection( CopyToPattern.m_Quality );
        m_CopyToMoveFilesChkBox->SetValue( CopyToPattern.m_MoveFiles );

        m_CopyToAcceptBtn->Disable();
    }
    else
    {
        m_CopyToDelBtn->Disable();
        m_CopyToUpBtn->Disable();
        m_CopyToDownBtn->Disable();
        m_CopyToAcceptBtn->Disable();
        m_CopyToPatternTextCtrl->SetValue( wxEmptyString );
        m_CopyToNameTextCtrl->SetValue( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToAddBtnClick( wxCommandEvent& event )
{
    wxString Cmd = m_CopyToPatternTextCtrl->GetValue();
    if( !Cmd.IsEmpty() )
    {
        m_CopyToListBox->Append( m_CopyToNameTextCtrl->GetValue() );
        guCopyToPattern * CopyToPattern = new guCopyToPattern();
        CopyToPattern->m_Name = m_CopyToNameTextCtrl->GetValue();
        CopyToPattern->m_Pattern = m_CopyToPatternTextCtrl->GetValue();
        CopyToPattern->m_Format = m_CopyToFormatChoice->GetSelection();
        CopyToPattern->m_Quality = m_CopyToQualityChoice->GetSelection();
        CopyToPattern->m_MoveFiles = m_CopyToMoveFilesChkBox->GetValue();
        m_CopyToOptions->Add( CopyToPattern );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToDelBtnClick( wxCommandEvent& event )
{
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        m_CopyToOptions->RemoveAt( m_CopyToSelected );
        m_CopyToListBox->Delete( m_CopyToSelected );
        m_CopyToSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToMoveUpBtnClick( wxCommandEvent &event )
{
    wxString CurName = m_CopyToListBox->GetString( m_CopyToSelected );
    guCopyToPattern CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
    m_CopyToListBox->SetString( m_CopyToSelected, m_CopyToListBox->GetString( m_CopyToSelected - 1 ) );
    m_CopyToOptions->Item( m_CopyToSelected ) = m_CopyToOptions->Item( m_CopyToSelected - 1 );
    m_CopyToSelected--;
    m_CopyToListBox->SetString( m_CopyToSelected, CurName );
    m_CopyToOptions->Item( m_CopyToSelected ) = CopyToPattern;
    m_CopyToListBox->SetSelection( m_CopyToSelected );

    event.SetInt( m_CopyToSelected );
    OnCopyToListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToMoveDownBtnClick( wxCommandEvent &event )
{
    wxString CurName = m_CopyToListBox->GetString( m_CopyToSelected );
    guCopyToPattern CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
    m_CopyToListBox->SetString( m_CopyToSelected, m_CopyToListBox->GetString( m_CopyToSelected + 1 ) );
    m_CopyToOptions->Item( m_CopyToSelected ) = m_CopyToOptions->Item( m_CopyToSelected + 1 );
    m_CopyToSelected++;
    m_CopyToListBox->SetString( m_CopyToSelected, CurName );
    m_CopyToOptions->Item( m_CopyToSelected ) = CopyToPattern;
    m_CopyToListBox->SetSelection( m_CopyToSelected );

    event.SetInt( m_CopyToSelected );
    OnCopyToListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToTextChanged( wxCommandEvent &event )
{
    if( !m_CopyToPatternTextCtrl->IsEmpty() )
    {
        m_CopyToAddBtn->Enable();
        if( m_CopyToSelected != wxNOT_FOUND )
            m_CopyToAcceptBtn->Enable();
    }
    else
    {
        m_CopyToAddBtn->Disable();
        if( m_CopyToSelected != wxNOT_FOUND )
            m_CopyToAcceptBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToFormatChanged( wxCommandEvent &event )
{
    m_CopyToQualityChoice->Enable( m_CopyToFormatChoice->GetSelection() );
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        if( !m_CopyToPatternTextCtrl->IsEmpty() )
        {
            m_CopyToAcceptBtn->Enable();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToQualityChanged( wxCommandEvent &event )
{
    m_CopyToQualityChoice->Enable( m_CopyToFormatChoice->GetSelection() );
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        if( !m_CopyToPatternTextCtrl->IsEmpty() )
        {
            m_CopyToAcceptBtn->Enable();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToMoveFilesChanged( wxCommandEvent &event )
{
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        if( !m_CopyToPatternTextCtrl->IsEmpty() )
        {
            m_CopyToAcceptBtn->Enable();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToSaveBtnClick( wxCommandEvent &event )
{
    m_CopyToListBox->SetString( m_CopyToSelected, m_CopyToNameTextCtrl->GetValue() );
    guCopyToPattern &CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
    CopyToPattern.m_Name = m_CopyToNameTextCtrl->GetValue();
    CopyToPattern.m_Pattern = m_CopyToPatternTextCtrl->GetValue();
    CopyToPattern.m_Format = m_CopyToFormatChoice->GetSelection();
    CopyToPattern.m_Quality = m_CopyToQualityChoice->GetSelection();
    CopyToPattern.m_MoveFiles = m_CopyToMoveFilesChkBox->GetValue();

    if( CopyToPattern.m_Name.IsEmpty() )
    {
        CopyToPattern.m_Name = CopyToPattern.m_Pattern;
        m_CopyToNameTextCtrl->SetValue( CopyToPattern.m_Pattern );
    }
    m_CopyToAcceptBtn->Disable();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnAccelSelected( wxListEvent &event )
{
    if( m_AccelItemNeedClear )
    {
        m_AccelItemNeedClear = false;
        m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, wxEmptyString );
    }

    m_AccelCurIndex = event.GetIndex();
    //guLogMessage( wxT( "Selected Accel %i" ), m_AccelCurIndex );
    if( m_AccelCurIndex != wxNOT_FOUND )
    {
        m_AccelLastKey = m_AccelKeys[ m_AccelCurIndex ];
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnAccelKeyDown( wxKeyEvent &event )
{
    if( m_AccelCurIndex != wxNOT_FOUND )
    {
        //guLogMessage( wxT( "Mod : %08X Key: %08X %i %c" ), event.GetModifiers(), event.GetKeyCode(), event.GetKeyCode(), event.GetKeyCode() );
        int Modifiers = event.GetModifiers();
        int KeyCode = event.GetKeyCode();

        switch( KeyCode )
        {
            case WXK_SHIFT :
            case WXK_ALT :
            case WXK_CONTROL :
            case WXK_MENU :
            case WXK_PAUSE :
            case WXK_CAPITAL :
                event.Skip();
                return;

            default :
                break;
        }

        if( Modifiers == 0 )
        {
            switch( KeyCode )
            {
                case WXK_HOME :
                case WXK_END :
                case WXK_PAGEUP :
                case WXK_PAGEDOWN :
                case WXK_UP :
                case WXK_DOWN :
                    event.Skip();
                    return;

                case WXK_ESCAPE :
                    if( m_AccelLastKey != m_AccelKeys[ m_AccelCurIndex ] )
                    {
                        m_AccelKeys[ m_AccelCurIndex ] = m_AccelLastKey;
                        m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, guAccelGetKeyCodeString( m_AccelLastKey ) );
                    }
                    return;

                case WXK_DELETE :
                    m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, wxEmptyString );
                    m_AccelKeys[ m_AccelCurIndex ] = 0;
                    return;

                default :
                    break;
            }

            if( wxIsalnum( KeyCode ) || wxIsprint( KeyCode ) )
            {
                return;
            }
        }

        int AccelCurKey = ( Modifiers << 16 ) | KeyCode;
        int KeyIndex = m_AccelKeys.Index( AccelCurKey );
        if( ( KeyIndex == wxNOT_FOUND ) || ( KeyIndex == m_AccelCurIndex ) )
        {
            if( m_AccelItemNeedClear )
            {
                m_AccelItemNeedClear = false;
            }
            m_AccelKeys[ m_AccelCurIndex ] = AccelCurKey;
            m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, guAccelGetKeyCodeString( AccelCurKey ) );
        }
        else
        {
            if( !m_AccelItemNeedClear )
            {
                m_AccelItemNeedClear = true;
                m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, wxString( _( "Key used by '" ) ) + m_AccelActionNames[ KeyIndex ] + wxT( "'") );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
