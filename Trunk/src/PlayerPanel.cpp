// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#include "PlayerPanel.h"

#include "Commands.h"
#include "CoverFrame.h"
#include "CoverPanel.h"
#include "Config.h"
#include "DbLibrary.h"
#include "Equalizer.h"
#include "FileRenamer.h" // NormalizeField
#include "Images.h"
#include "LastFM.h"
#include "MainFrame.h"
#include "TagInfo.h"
//#include "TrackChangeInfo.h"
#include "Utils.h"
#include "VolumeFrame.h"

#include <wx/gdicmn.h>
#include <wx/regex.h>
#include <wx/utils.h>

#define GUPLAYER_MIN_PREVTRACK_POS      5000

#define guPLAYER_SMART_CACHEITEMS       100
#define guPLAYER_SMART_CACHEARTISTS     20

#define guPLAYER_ICONS_SEPARATOR        3
#define guPLAYER_ICONS_GROUPSEPARATOR   5

#define guPLAYER_FONTSIZE_TRACKNAME     12
#define guPLAYER_FONTSIZE_ALBUMNAME     11
#define guPLAYER_FONTSIZE_ARTISTNAME    11


// -------------------------------------------------------------------------------- //
guPlayerPanel::guPlayerPanel( wxWindow * parent, guDbLibrary * db,
    guPlayList * playlist, guPlayerFilters * filters )
       : wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 310, 170 ), wxTAB_TRAVERSAL )
{
    double SavedVol;

    m_Db = db;
    m_MainFrame = ( guMainFrame * ) parent;
    m_PlayListCtrl = playlist;
    m_NotifySrv = NULL;
    m_PlayerFilters = filters;
    m_BufferGaugeId = wxNOT_FOUND;
    m_PendingNewRecordName = false;
    m_MediaSong.m_SongId = 0;
    m_MediaSong.m_Length = 0;
    m_MediaSong.m_CoverType = GU_SONGCOVER_NONE;

	wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

    // For the Load configuration
    wxArrayString Songs;
    guConfig * Config;

    m_LastVolume = wxNOT_FOUND;
    m_PlayerVumeters = NULL;
    ResetVumeterLevel();

    m_LastCurPos = 0;
    m_LastLength = 0;
    m_LastPlayState = -1; //guMEDIASTATE_STOPPE;
    m_LastTotalLen = -1;
    m_TrackStartPos = 0;

    m_NextTrackId = 0;
    m_CurTrackId = 0;

    wxArrayInt Equalizer;

    m_SilenceDetector = false;
    m_SilenceDetectorLevel = wxNOT_FOUND;
    m_SilenceDetectorTime = 0;

    m_ShowRevTime = false;
    m_PlayRandom = false;
    m_PlayRandomMode = guRANDOM_MODE_TRACK;
    m_DelTracksPlayed = false;
    m_PendingScrob = false;
    m_IsSkipping = false;
    m_ShowNotifications = true;
    m_ShowNotificationsTime = 0;
    m_ErrorFound = false;
    m_SavedPlayedTrack = false;

    m_SilenceDetected = false;
    m_AboutToEndDetected = false;

    // Load configuration
    Config = ( guConfig * ) guConfig::Get();

    Config->RegisterObject( this );

    //guLogMessage( wxT( "Reading PlayerPanel Config" ) );
    SavedVol = Config->ReadNum( wxT( "PlayerCurVol" ), 50, wxT( "General" ) );
    //guLogMessage( wxT( "Current Volume Var : %d" ), ( int ) m_CurVolume );
    m_PlayLoop = Config->ReadNum( wxT( "PlayerLoop" ), 0, wxT( "General" )  );
    m_PlaySmart = Config->ReadBool( wxT( "PlayerSmart" ), m_PlayLoop ? false : true, wxT( "General" )  );
    m_PlayRandom = Config->ReadBool( wxT( "RndPlayOnEmptyPlayList" ), false, wxT( "General" ) );
    m_PlayRandomMode = Config->ReadNum( wxT( "RndModeOnEmptyPlayList" ), guRANDOM_MODE_TRACK, wxT( "General" ) );
    m_ShowNotifications = Config->ReadBool( wxT( "ShowNotifications" ), true, wxT( "General" ) );
    m_ShowNotificationsTime = Config->ReadNum( wxT( "NotificationsTime" ), 0, wxT( "General" ) );

    m_SmartPlayAddTracks = Config->ReadNum( wxT( "NumTracksToAdd" ), 3, wxT( "Playback" ) );
    m_SmartPlayMinTracksToPlay = Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) );
    m_DelTracksPlayed = Config->ReadBool( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) );
	m_SmartMaxArtistsList = Config->ReadNum( wxT( "SmartFilterArtists" ), 20, wxT( "Playback" ) );
	m_SmartMaxTracksList = Config->ReadNum( wxT( "SmartFilterTracks" ), 100, wxT( "Playback" ) );

    m_AudioScrobbleEnabled = Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) ) ||
                             Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LibreFM" ) );
    Equalizer = Config->ReadANum( wxT( "Band" ), 0, wxT( "Equalizer" ) );
    if( Equalizer.Count() != guEQUALIZER_BAND_COUNT )
    {
        Equalizer.Empty();
        Equalizer.Add( 0, guEQUALIZER_BAND_COUNT );
    }

    m_SilenceDetector = Config->ReadBool( wxT( "SilenceDetector" ), false, wxT( "Playback" ) );
    m_SilenceDetectorLevel = Config->ReadNum( wxT( "SilenceLevel" ), -55, wxT( "Playback" ) );
    if( Config->ReadBool( wxT( "SilenceAtEnd" ), false, wxT( "Playback" ) ) )
    {
        m_SilenceDetectorTime = Config->ReadNum( wxT( "SilenceEndTime" ), 45, wxT( "Playback" ) ) * 1000;
    }

    m_ShowRevTime = Config->ReadBool( wxT( "ShowRevTime" ), false, wxT( "General" ) );

    m_ForceGapless = Config->ReadBool( wxT( "ForceGapless"), false, wxT( "Crossfader" ) );
    m_FadeOutTime = Config->ReadNum( wxT( "FadeOutTime" ), 50, wxT( "Crossfader" ) ) * 100;
//        m_ShowFiltersChoices = Config->ReadBool( wxT( "ShowFiltersChoices" ), true, wxT( "Positions" ) );

    m_SliderIsDragged = false;
    m_SmartSearchEnabled = false;
    m_SmartAddTracksThread = NULL;
    m_UpdateCoverThread = NULL;

    // ---------------------------------------------------------------------------- //
    // The player controls
    // ---------------------------------------------------------------------------- //
//    wxPanel * PlayerPanel;
//	PlayerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxBoxSizer * PlayerMainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * PlayerBtnSizer = new wxBoxSizer( wxHORIZONTAL );

	//m_PrevTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_prev ), wxDefaultPosition, wxDefaultSize, 0 ); //wxBU_AUTODRAW );
	m_PrevTrackButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_prev ), guImage( guIMAGE_INDEX_player_highlight_prev ), 0 );
	m_PrevTrackButton->SetToolTip( _( "Go to Previous Track in Playlist" ) );
	PlayerBtnSizer->Add( m_PrevTrackButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_PlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_play ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_play ), guImage( guIMAGE_INDEX_player_highlight_play ), 0 );
	m_PlayButton->SetToolTip( _( "Play or pause current track in Playlist" ) );
	PlayerBtnSizer->Add( m_PlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_NextTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_next ), wxDefaultPosition, wxDefaultSize, 0 );
	m_NextTrackButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_next ), guImage( guIMAGE_INDEX_player_highlight_next ), 0 );
	m_NextTrackButton->SetToolTip( _( "Go to Next Track in Playlist" ) );
	PlayerBtnSizer->Add( m_NextTrackButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_StopButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_stop ), wxDefaultPosition, wxDefaultSize, 0 );
	m_StopButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_stop ), guImage( guIMAGE_INDEX_player_highlight_stop ), 0 );
	m_StopButton->SetToolTip( _( "Stop playing" ) );
	PlayerBtnSizer->Add( m_StopButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM|wxRIGHT, guPLAYER_ICONS_SEPARATOR );

    m_RecordButton = new guToggleRoundButton( this, guImage( guIMAGE_INDEX_player_light_record ), guImage( guIMAGE_INDEX_player_normal_record ), guImage( guIMAGE_INDEX_player_highlight_record ) );
    m_RecordButton->SetToolTip( _( "Record to a file" ) );
    m_RecordButton->Enable( false );
    m_RecordButton->Show( Config->ReadBool( wxT( "Enabled" ), false, wxT( "Record" ) ) );
    PlayerBtnSizer->Add( m_RecordButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, guPLAYER_ICONS_SEPARATOR );

	PlayerBtnSizer->Add( guPLAYER_ICONS_GROUPSEPARATOR, 0, 0, wxEXPAND, 5 );

	//m_VolumeButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_vol_mid ), wxDefaultPosition, wxDefaultSize, 0 );
	m_VolumeButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_vol_mid ), guImage( guIMAGE_INDEX_player_highlight_vol_mid ), 0 );
    m_VolumeButton->SetToolTip( _( "Volume" ) + wxString::Format( wxT( " %i%%" ), ( int ) SavedVol ) );
	PlayerBtnSizer->Add( m_VolumeButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_EqualizerButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_equalizer ), wxDefaultPosition, wxDefaultSize, 0 );
	m_EqualizerButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_equalizer ), guImage( guIMAGE_INDEX_player_highlight_equalizer ), 0 );
	m_EqualizerButton->SetToolTip( _( "Show the equalizer" ) );
	PlayerBtnSizer->Add( m_EqualizerButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM|wxRIGHT, guPLAYER_ICONS_SEPARATOR );

	PlayerBtnSizer->Add( guPLAYER_ICONS_GROUPSEPARATOR, 0, 0, wxEXPAND, 5 );

	//m_SmartPlayButton = new wxToggleBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_smart ), wxDefaultPosition, wxDefaultSize, 0 );
	m_SmartPlayButton = new guToggleRoundButton( this, guImage( guIMAGE_INDEX_player_light_smart ), guImage( guIMAGE_INDEX_player_normal_smart ), guImage( guIMAGE_INDEX_player_highlight_smart ) );
	//m_SmartPlayButton->SetToolTip( _( "Add tracks to the playlist based on LastFM" ) );
    wxString TipText = _( "Smart Mode: " );
    if( !m_PlaySmart )
    {
        TipText += _( "Off" );
    }
    else
    {
        TipText += _( "On" );
    }
    m_SmartPlayButton->SetToolTip( TipText );
	// Get PlayerPanel value from config file
	m_SmartPlayButton->SetValue( m_PlaySmart );
	PlayerBtnSizer->Add( m_SmartPlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	m_RepeatPlayButton = new guToggleRoundButton( this, guImage( guIMAGE_INDEX_player_light_repeat ), guImage( guIMAGE_INDEX_player_normal_repeat ), guImage( guIMAGE_INDEX_player_highlight_repeat ) );
	//m_RepeatPlayButton->SetToolTip( _( "Select the repeat mode" ) );
    TipText = _( "Repeat Mode: " );
    if( !m_PlayLoop )
    {
        TipText += _( "Off" );
    }
    else if( m_PlayLoop == guPLAYER_PLAYLOOP_TRACK )
    {
        TipText += _( "Track" );
    }
    else
    {
        TipText += _( "Playlist" );
    }
    m_RepeatPlayButton->SetToolTip( TipText );
	m_RepeatPlayButton->SetValue( m_PlayLoop );
	PlayerBtnSizer->Add( m_RepeatPlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_RandomPlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_random ), wxDefaultPosition, wxDefaultSize, 0 );
	m_RandomPlayButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_random ), guImage( guIMAGE_INDEX_player_highlight_random ), 0 );
	m_RandomPlayButton->SetToolTip( _( "Randomize the tracks in playlist" ) );
	PlayerBtnSizer->Add( m_RandomPlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	PlayerMainSizer->Add( PlayerBtnSizer, 0, wxEXPAND, 2 );


	wxBoxSizer* PlayerDetailsSizer;
	PlayerDetailsSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayerCoverBitmap = new guStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_no_cover ), wxDefaultPosition, wxSize( 100,100 ), 0 );
	//m_PlayerCoverBitmap->SetToolTip( _( "Shows the current track album cover if available" ) );
	PlayerDetailsSizer->Add( m_PlayerCoverBitmap, 0, wxALL, 2 );

	wxBoxSizer* PlayerLabelsSizer;
	PlayerLabelsSizer = new wxBoxSizer( wxVERTICAL );

	//m_TitleLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TitleLabel = new guAutoScrollText( this, wxEmptyString );
	m_TitleLabel->SetToolTip( _( "Show the name of the current track" ) );
	//m_TitleLabel->Wrap( -1 );
	CurrentFont.SetPointSize( guPLAYER_FONTSIZE_TRACKNAME );
	CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
	m_TitleLabel->SetFont( CurrentFont );

	PlayerLabelsSizer->Add( m_TitleLabel, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 2 );

	//m_AlbumLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumLabel = new guAutoScrollText( this, wxEmptyString );
	m_AlbumLabel->SetToolTip( _( "Show the album name of the current track" ) );
	//m_AlbumLabel->Wrap( -1 );
	CurrentFont.SetPointSize( guPLAYER_FONTSIZE_ALBUMNAME );
	CurrentFont.SetWeight( wxFONTWEIGHT_NORMAL );
	CurrentFont.SetStyle( wxFONTSTYLE_ITALIC );
	m_AlbumLabel->SetFont( CurrentFont );

	PlayerLabelsSizer->Add( m_AlbumLabel, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 2 );

	//m_ArtistLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistLabel = new guAutoScrollText( this, wxEmptyString );
	m_ArtistLabel->SetToolTip( _( "Show the artist name of the current track" ) );
	//m_ArtistLabel->Wrap( -1 );
	CurrentFont.SetPointSize( guPLAYER_FONTSIZE_ARTISTNAME );
	CurrentFont.SetStyle( wxFONTSTYLE_NORMAL );
	m_ArtistLabel->SetFont( CurrentFont );

	PlayerLabelsSizer->Add( m_ArtistLabel, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 2 );

	m_PosLabelSizer = new wxBoxSizer( wxHORIZONTAL );
	//m_PosLabelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	m_YearLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_YearLabel->SetToolTip( _( "Show the year of the current track" ) );
	m_PosLabelSizer->Add( m_YearLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 2 );

	m_PositionLabel = new wxStaticText( this, wxID_ANY, _("00:00 of 00:00"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PositionLabel->SetToolTip( _( "Show the current position and song length of the current track" ) );
	m_PositionLabel->Wrap( -1 );

	m_PosLabelSizer->Add( m_PositionLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2 );

	PlayerLabelsSizer->Add( m_PosLabelSizer, 1, wxEXPAND, 2 );

    m_BitRateSizer = new wxBoxSizer( wxHORIZONTAL );

    m_Rating = new guRating( this, GURATING_STYLE_MID );
	m_BitRateSizer->Add( m_Rating, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 2 );

//	m_LoveBanButton = new guToggleRoundButton( this, guImage( guIMAGE_INDEX_player_light_love ), guImage( guIMAGE_INDEX_player_normal_love ), guImage( guIMAGE_INDEX_player_highlight_love ) );
//    m_LoveBanButton->SetToolTip( _( "Love or Ban a track in AudioScrobble service" ) );
//    m_BitRateSizer->Add( m_LoveBanButton, 0, wxEXPAND|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 2 );

	m_BitRateSizer->Add( 0, 0, 1, wxALL, 2 );

	m_BitRateLabel = new wxStaticText( this, wxID_ANY, wxT( "[kbps]" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_BitRateLabel->SetToolTip( _( "Show the bitrate of the current track" ) );
	CurrentFont.SetPointSize( 8 );
	m_BitRateLabel->SetFont( CurrentFont );

    m_BitRateSizer->Add( m_BitRateLabel, 0, wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 2 );

	PlayerLabelsSizer->Add( m_BitRateSizer, 0, wxEXPAND, 2 );

	PlayerDetailsSizer->Add( PlayerLabelsSizer, 1, wxEXPAND, 5 );

	PlayerMainSizer->Add( PlayerDetailsSizer, 0, wxEXPAND, 5 );

	m_PlayerPositionSlider = new wxSlider( this, wxID_ANY, 0, 0, 1000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	PlayerMainSizer->Add( m_PlayerPositionSlider, 0, wxALL|wxEXPAND, 0 );


//	SetSizer( PlayerMainSizer );
//	Layout();
//	PlayerMainSizer->Fit( this );
//    SetSizeHints( this );
	//PlayerPanel->Layout();


////////////////////////////////////////////////////////////////////////////////////////////////////////



//	m_PlayListCtrl = new guPlayList( this, m_Db );
//    PlayListSizer->Add( m_PlayListCtrl, 1, wxALL|wxEXPAND, 2 );
//	wxPanel * PlayListPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//	wxBoxSizer * PlayListPanelSizer = new wxBoxSizer( wxVERTICAL );

//	m_PlayListCtrl = new guPlayList( this, m_Db, this );
	//PlayListPanelSizer->Add( m_PlayListCtrl, 1, wxALL|wxEXPAND, 2 );
//	PlayerMainSizer->Add( m_PlayListCtrl, 1, wxALL|wxEXPAND, 2 );

//	PlayListPanel->SetSizer( PlayListPanelSizer );
//	PlayListPanel->Layout();
//	PlayListPanelSizer->Fit( PlayListPanel );

//    m_AuiManager.AddPane( PlayListPanel, wxAuiPaneInfo().Name( wxT( "PlayerPlayList" ) ).Caption( _( "Playlist" ) ).
//            Layer( 0 ).Row( 1 ).Position( 0 ).
//            Bottom() );

	SetSizer( PlayerMainSizer );
	Layout();
	PlayerMainSizer->Fit( this );
    PlayerMainSizer->SetSizeHints( this );
	//PlayerPanel->Layout();


    m_MediaCtrl = new guMediaCtrl( this );
    m_MediaRecordCtrl = new guMediaRecordCtrl( this, m_MediaCtrl );
    //m_MediaCtrl->Create( this, wxID_ANY );

    //
    m_PlayListCtrl->ReloadItems();
    TrackListChanged();
    // The SetVolume call dont get set if the volume is the last one
    // so we do it two calls

    m_CurVolume = wxNOT_FOUND;
    SetVolume( SavedVol );
    //guLogMessage( wxT( "CurVol: %i SavedVol: %i" ), int( m_MediaCtrl->GetVolume() * 100.0 ), ( int ) m_CurVolume );

    m_MediaCtrl->SetEqualizer( Equalizer );

    // There was a track passed as argument that we will play
    if( m_PlayListCtrl->StartPlaying() )
    {
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
        OnPlayButtonClick( event );
    }
    else
    {
        if( Config->ReadBool( wxT( "SaveCurrentTrackPos" ), false, wxT( "General" ) ) )
        {
            m_TrackStartPos = wxMax( 0, Config->ReadNum( wxT( "CurrentTrackPos" ), 0, wxT( "General" ) ) - 500 );
            if( m_TrackStartPos > 0 )
            {
                wxCommandEvent event;
                event.SetInt( Config->ReadNum( wxT( "PlayerCurItem" ), 0, wxT( "General" ) ) );
                //OnPlayButtonClick( event );
                OnPlayListDClick( event );
            }
            else
            {
                SetNextTrack( m_PlayListCtrl->GetCurrent() );
            }
        }
        else if( m_PlayListCtrl->GetCurItem() != wxNOT_FOUND )
        {
            SetNextTrack( m_PlayListCtrl->GetCurrent() );
        }
    }

    CheckFiltersEnable();

	// Connect Events
	m_PrevTrackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPrevTrackButtonClick ), NULL, this );
	m_NextTrackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnNextTrackButtonClick ), NULL, this );
	m_PlayButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayButtonClick ), NULL, this );
 	m_StopButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnStopButtonClick ), NULL, this );
    m_RecordButton->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRecordButtonClick ), NULL, this );
	m_VolumeButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnVolumenButtonClick ), NULL, this );
	m_VolumeButton->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guPlayerPanel::OnVolumenMouseWheel ), NULL, this );
	m_SmartPlayButton->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnSmartPlayButtonClick ), NULL, this );
	m_RandomPlayButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ), NULL, this );
	m_RepeatPlayButton->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRepeatPlayButtonClick ), NULL, this );
	m_EqualizerButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnEqualizerButtonClicked ), NULL, this );

    Connect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ) );

    m_TitleLabel->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnTitleNameDClicked ), NULL, this );
	m_AlbumLabel->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnAlbumNameDClicked ), NULL, this );
	m_ArtistLabel->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnArtistNameDClicked ), NULL, this );
	m_YearLabel->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnYearDClicked ), NULL, this );

	m_PositionLabel->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( guPlayerPanel::OnTimeDClicked ), NULL, this );
	m_Rating->Connect( guEVT_RATING_CHANGED, guRatingEventHandler( guPlayerPanel::OnRatingChanged ), NULL, this );
//	m_LoveBanButton->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnLoveBanButtonClick ), NULL, this );

    //
	//m_PlayerCoverBitmap->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnLeftDClickPlayerCoverBitmap ), NULL, this );
    Connect( guEVT_STATICBITMAP_MOUSE_OVER, guStaticBitmapMouseOverEvent, wxCommandEventHandler( guPlayerPanel::OnPlayerCoverBitmapMouseOver ), NULL, this );

	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderBeginSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_CHANGED	, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderChanged ), NULL, this );
//	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
    m_PlayerPositionSlider->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guPlayerPanel::OnPlayerPositionSliderMouseWheel ), NULL, this );

    m_PlayListCtrl->Connect( ID_PLAYER_PLAYLIST_UPDATELIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnPlayListUpdated ), NULL, this );
    m_PlayListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayListDClick ), NULL, this );

    Connect( guEVT_MEDIA_LOADED, guMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    Connect( guEVT_MEDIA_FINISHED, guMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    Connect( guEVT_MEDIA_FADEOUT_FINISHED, guMediaEventHandler( guPlayerPanel::OnMediaFadeOutFinished ), NULL, this );
    Connect( guEVT_MEDIA_FADEIN_STARTED, guMediaEventHandler( guPlayerPanel::OnMediaFadeInStarted ), NULL, this );
    Connect( guEVT_MEDIA_TAGINFO, guMediaEventHandler( guPlayerPanel::OnMediaTags ), NULL, this );
    Connect( guEVT_MEDIA_CHANGED_BITRATE, guMediaEventHandler( guPlayerPanel::OnMediaBitrate ), NULL, this );
    Connect( guEVT_MEDIA_BUFFERING, guMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );
    Connect( guEVT_MEDIA_LEVELINFO, guMediaEventHandler( guPlayerPanel::OnMediaLevel ), NULL, this );
    Connect( guEVT_MEDIA_ERROR, guMediaEventHandler( guPlayerPanel::OnMediaError ), NULL, this );
    Connect( guEVT_MEDIA_CHANGED_STATE, guMediaEventHandler( guPlayerPanel::OnMediaState ), NULL, this );
    Connect( guEVT_MEDIA_CHANGED_POSITION, guMediaEventHandler( guPlayerPanel::OnMediaPosition ), NULL, this );
    Connect( guEVT_MEDIA_CHANGED_LENGTH, guMediaEventHandler( guPlayerPanel::OnMediaLength ), NULL, this );

    Connect( ID_PLAYER_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracks ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPlayerPanel::OnConfigUpdated ), NULL, this );

    Connect( ID_PLAYERPANEL_COVERUPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnCoverUpdated ), NULL, this );

    Connect( ID_PLAYERPANEL_ADDTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnAddTracks ), NULL, this );
    Connect( ID_PLAYERPANEL_REMOVETRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRemoveTrack ), NULL, this );
    Connect( ID_PLAYERPANEL_SETREPEAT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRepeat ), NULL, this );
    Connect( ID_PLAYERPANEL_SETLOOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnLoop ), NULL, this );
    Connect( ID_PLAYERPANEL_SETRANDOM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandom ), NULL, this );
    Connect( ID_PLAYERPANEL_SETVOLUME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSetVolume ), NULL, this );

    //
    m_AudioScrobble = NULL;
    if( m_AudioScrobbleEnabled )
    {
        m_AudioScrobble = new guAudioScrobble( m_Db );
    }
}

// -------------------------------------------------------------------------------- //
guPlayerPanel::~guPlayerPanel()
{
    if( m_SmartAddTracksThread )
    {
        m_SmartAddTracksThread->Pause();
        m_SmartAddTracksThread->Delete();
    }

    if( m_UpdateCoverThread )
    {
        m_UpdateCoverThread->Pause();
        m_UpdateCoverThread->Delete();
    }

    if( m_AudioScrobble )
        delete m_AudioScrobble;

    if( m_MediaRecordCtrl )
        delete m_MediaRecordCtrl;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->UnRegisterObject( this );

        //printf( "guPlayerPanel::guConfig Save\n" );
        Config->WriteBool( wxT( "PlayerStopped" ), m_MediaCtrl->GetState() != guMEDIASTATE_PLAYING, wxT( "General" ) );
        Config->WriteNum( wxT( "PlayerCurVol" ), m_CurVolume, wxT( "General" ) );
        Config->WriteNum( wxT( "PlayerLoop" ), m_PlayLoop, wxT( "General" ) );
        Config->WriteBool( wxT( "PlayerSmart" ), m_PlaySmart, wxT( "General" ) );
        // If the track length is at least the configured minimun track length save the pos offset
        if( Config->ReadBool( wxT( "SaveCurrentTrackPos" ), false, wxT( "General" ) ) )
        {
            if( ( m_LastPlayState != guMEDIASTATE_STOPPED ) &&
                ( m_MediaSong.m_Length >= ( unsigned int ) ( Config->ReadNum( wxT( "MinSavePlayPosLength" ), 10, wxT( "General" ) ) * 60 ) ) )
            {
                Config->WriteNum( wxT( "CurrentTrackPos" ), m_LastCurPos, wxT( "General" ) );
            }
            else
            {
                Config->WriteNum( wxT( "CurrentTrackPos" ), 0, wxT( "General" ) );
            }
        }
        //printf( PlaySmart ? "Smart Enabled" : "Smart Disabled" );  printf( "\n" );

        int index;
        wxArrayInt Equalizer;
        for( index = 0; index < guEQUALIZER_BAND_COUNT; index++ )
        {
            Equalizer.Add( m_MediaCtrl->GetEqualizerBand( index ) );
        }
        Config->WriteANum( wxT( "Band" ), Equalizer, wxT( "Equalizer" ) );

        Config->WriteBool( wxT( "ShowRevTime" ), m_ShowRevTime, wxT( "General" ) );
    }

	// Disconnect Events
	m_PrevTrackButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPrevTrackButtonClick ), NULL, this );
	m_NextTrackButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnNextTrackButtonClick ), NULL, this );
	m_PlayButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayButtonClick ), NULL, this );
 	m_StopButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnStopButtonClick ), NULL, this );
    m_RecordButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRecordButtonClick ), NULL, this );
	m_VolumeButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnVolumenButtonClick ), NULL, this );
	m_VolumeButton->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guPlayerPanel::OnVolumenMouseWheel ), NULL, this );
	m_SmartPlayButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnSmartPlayButtonClick ), NULL, this );
	m_RandomPlayButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ), NULL, this );
	m_RepeatPlayButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRepeatPlayButtonClick ), NULL, this );
	m_EqualizerButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnEqualizerButtonClicked ), NULL, this );

    Disconnect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ) );

    m_TitleLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnTitleNameDClicked ), NULL, this );
	m_AlbumLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnAlbumNameDClicked ), NULL, this );
	m_ArtistLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnArtistNameDClicked ), NULL, this );
	m_YearLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnYearDClicked ), NULL, this );

	m_PositionLabel->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( guPlayerPanel::OnTimeDClicked ), NULL, this );
	m_Rating->Disconnect( guEVT_RATING_CHANGED, guRatingEventHandler( guPlayerPanel::OnRatingChanged ), NULL, this );
//	m_LoveBanButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnLoveBanButtonClick ), NULL, this );

    //
	//m_PlayerCoverBitmap->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnLeftDClickPlayerCoverBitmap ), NULL, this );
    Disconnect( guEVT_STATICBITMAP_MOUSE_OVER, guStaticBitmapMouseOverEvent, wxCommandEventHandler( guPlayerPanel::OnPlayerCoverBitmapMouseOver ), NULL, this );

	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderBeginSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_CHANGED	, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderChanged ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
    m_PlayerPositionSlider->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guPlayerPanel::OnPlayerPositionSliderMouseWheel ), NULL, this );

    m_PlayListCtrl->Disconnect( ID_PLAYER_PLAYLIST_UPDATELIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnPlayListUpdated ), NULL, this );
    m_PlayListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayListDClick ), NULL, this );

    Disconnect( guEVT_MEDIA_LOADED, guMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    Disconnect( guEVT_MEDIA_FINISHED, guMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    Disconnect( guEVT_MEDIA_FADEOUT_FINISHED, guMediaEventHandler( guPlayerPanel::OnMediaFadeOutFinished ), NULL, this );
    Disconnect( guEVT_MEDIA_FADEIN_STARTED, guMediaEventHandler( guPlayerPanel::OnMediaFadeInStarted ), NULL, this );
    Disconnect( guEVT_MEDIA_TAGINFO, guMediaEventHandler( guPlayerPanel::OnMediaTags ), NULL, this );
    Disconnect( guEVT_MEDIA_CHANGED_BITRATE, guMediaEventHandler( guPlayerPanel::OnMediaBitrate ), NULL, this );
    Disconnect( guEVT_MEDIA_BUFFERING, guMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );
    Disconnect( guEVT_MEDIA_LEVELINFO, guMediaEventHandler( guPlayerPanel::OnMediaLevel ), NULL, this );
    Disconnect( guEVT_MEDIA_ERROR, guMediaEventHandler( guPlayerPanel::OnMediaError ), NULL, this );
    Disconnect( guEVT_MEDIA_CHANGED_STATE, guMediaEventHandler( guPlayerPanel::OnMediaState ), NULL, this );
    Disconnect( guEVT_MEDIA_CHANGED_POSITION, guMediaEventHandler( guPlayerPanel::OnMediaPosition ), NULL, this );
    Disconnect( guEVT_MEDIA_CHANGED_LENGTH, guMediaEventHandler( guPlayerPanel::OnMediaLength ), NULL, this );

    Disconnect( ID_PLAYER_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracks ), NULL, this );

    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPlayerPanel::OnConfigUpdated ), NULL, this );

    Disconnect( ID_PLAYERPANEL_COVERUPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnCoverUpdated ), NULL, this );


    if( m_MediaCtrl )
        delete m_MediaCtrl;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnConfigUpdated( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        //guLogMessage( wxT( "Reading PlayerPanel Config Updated" ) );
        m_PlayRandom = Config->ReadBool( wxT( "RndPlayOnEmptyPlayList" ), false, wxT( "General" ) );
        m_PlayRandomMode = Config->ReadNum( wxT( "RndModeOnEmptyPlayList" ), guRANDOM_MODE_TRACK, wxT( "General" ) );
        m_ShowNotifications = Config->ReadBool( wxT( "ShowNotifications" ), true, wxT( "General" ) );
        m_ShowNotificationsTime = Config->ReadNum( wxT( "NotificationsTime" ), 0, wxT( "General" ) );

        m_RecordButton->Show( Config->ReadBool( wxT( "Enabled" ), false, wxT( "Record" ) ) );
        Layout();

        m_SmartPlayAddTracks = Config->ReadNum( wxT( "NumTracksToAdd" ), 3, wxT( "Playback" ) );
        m_SmartPlayMinTracksToPlay = Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) );
        m_DelTracksPlayed = Config->ReadBool( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) );
        m_SmartMaxArtistsList = Config->ReadNum( wxT( "SmartFilterArtists" ), 20, wxT( "Playback" ) );
        m_SmartMaxTracksList = Config->ReadNum( wxT( "SmartFilterTracks" ), 100, wxT( "Playback" ) );

        // We only insert the last CACHEITEMS as the rest should be forgiven
        while( ( int ) m_SmartAddedTracks.Count() > m_SmartMaxTracksList )
            m_SmartAddedTracks.RemoveAt( 0 );

        while( ( int ) m_SmartAddedArtists.Count() > m_SmartMaxArtistsList )
            m_SmartAddedArtists.RemoveAt( 0 );

        m_AudioScrobbleEnabled = Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) ) ||
                                 Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LibreFM" ) );

        m_SilenceDetector = Config->ReadBool( wxT( "SilenceDetector" ), false, wxT( "Playback" ) );
        m_SilenceDetectorLevel = Config->ReadNum( wxT( "SilenceLevel" ), -55, wxT( "Playback" ) );
        if( Config->ReadBool( wxT( "SilenceAtEnd" ), false, wxT( "Playback" ) ) )
        {
            m_SilenceDetectorTime = Config->ReadNum( wxT( "SilenceEndTime" ), 45, wxT( "Playback" ) ) * 1000;
        }

        m_ForceGapless = Config->ReadBool( wxT( "ForceGapless"), false, wxT( "Crossfader" ) );
        m_FadeOutTime = Config->ReadNum( wxT( "FadeOutTime" ), 50, wxT( "Crossfader" ) ) * 100;

        if( !m_PlaySmart )
            CheckFiltersEnable();

        if( m_AudioScrobbleEnabled )
        {
            if( !m_AudioScrobble )
                m_AudioScrobble = new guAudioScrobble( m_Db );
            else
                m_AudioScrobble->OnConfigUpdated();
        }
        else
        {
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_AUDIOSCROBBLE_UPDATED );
            event.SetInt( 1 );
            m_MainFrame->AddPendingEvent( event );
        }

        if( m_MediaCtrl )
        {
            m_MediaCtrl->UpdatedConfig();
        }

        if( m_MediaRecordCtrl )
        {
            m_MediaRecordCtrl->UpdatedConfig();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetArtistLabel( const wxString &artistname )
{
//    wxString Label = artistname;
//    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_ArtistLabel->SetLabel( artistname );
    m_ArtistLabel->SetToolTip( artistname );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetAlbumLabel( const wxString &albumname )
{
//    wxString Label = albumname;
//    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_AlbumLabel->SetLabel( albumname );
    m_AlbumLabel->SetToolTip( albumname );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetTitleLabel( const wxString &trackname )
{
//    wxString Label = trackname;
//    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_TitleLabel->SetLabel( trackname );
    m_TitleLabel->SetToolTip( trackname );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetRatingLabel( const int rating )
{
    m_Rating->SetRating( rating );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetRating( const int rating )
{
    //m_MediaSong.m_Rating = rating;
    m_Rating->SetRating( rating );
    guRatingEvent Event;
    Event.SetInt( rating );
    OnRatingChanged( Event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdatePositionLabel( const unsigned int curpos )
{
    wxString Label;
    unsigned int CurLen = m_LastLength / 1000;
    //if( !m_ShowRevTime || !m_MediaSong.m_Length )
    if( !m_ShowRevTime || !CurLen )
    {
        Label = LenToString( curpos );
    }
    else if( CurLen )
    {
        // The Gapless playback can produce than while we are listenning to the finish of
        // a track the player already changed to the next track.
        // To avoid weird time less results we check if the length of the track is
        // bigger than the current pos.
        //if( curpos > m_MediaSong.m_Length )
        if( curpos > CurLen )
            //Label = wxT( "-" ) + LenToString( m_MediaSong.m_Length );
            Label = wxT( "-" ) + LenToString( CurLen );
        else
            Label = wxT( "-" ) + LenToString( CurLen - curpos );
    }

    //if( m_MediaSong.m_Length )
    if( m_LastLength )
    {
        //Label += wxT( " / " ) + LenToString( m_MediaSong.m_Length );
        Label += wxT( " / " ) + LenToString( CurLen );
    }

    m_PositionLabel->SetLabel( Label );

    m_PosLabelSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetBitRateLabel( int bitrate )
{
    //guLogMessage( wxT( "SetBitRateLabel( %i )" ), bitrate );
    if( bitrate )
    {
        m_BitRateLabel->SetLabel( wxString::Format( wxT( "[%ukbps]" ), bitrate ) );
    }
    else
        m_BitRateLabel->SetLabel( wxT( "[kbps]" ) );
    m_BitRateSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetBitRate( int bitrate )
{
    if( bitrate )
    {
        bitrate = bitrate / 1000;
        //guLogMessage( wxT( "Bitrate: %u" ), bitrate );
        m_BitRateLabel->SetLabel( wxString::Format( wxT( "[%ukbps]" ), bitrate ) );
        if( ( m_MediaSong.m_Bitrate < bitrate ) && ( GetState() == guMEDIASTATE_PLAYING ) )
        {
            m_MediaSong.m_Bitrate = bitrate;

            if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
            {
                guDbLibrary * Db = m_MediaSong.m_LibPanel ? m_MediaSong.m_LibPanel->GetDb() : m_Db;
                Db->UpdateTrackBitRate( m_MediaSong.m_SongId, bitrate );
            }

            // Update the track in database, playlist, etc
            m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
        }
    }
    else
        m_BitRateLabel->SetLabel( wxT( "[kbps]" ) );
    m_BitRateSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetPlayList( const guTrackArray &SongList )
{
    wxCommandEvent event;
    m_PlayListCtrl->SetPlayList( SongList );

    SetNextTrack( m_PlayListCtrl->GetCurrent() );

    LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );
    TrackListChanged();

    if( m_PlaySmart )
    {
        // Reset the Smart played items
        m_SmartAddedTracks.Empty();
        m_SmartAddedArtists.Empty();

        //guLogMessage( wxT( "SetPlayList adding track to smart cache..." ) );
        int Count;
        int Index = 0;
        Count = SongList.Count();

        // We only insert the last CACHEITEMS as the rest should be forgiven
        if( Count > m_SmartMaxTracksList )
            Index = Count - m_SmartMaxTracksList;
        for( ; Index < Count; Index++ )
        {
            m_SmartAddedTracks.Add( SongList[ Index ].m_SongId );
        }

        Index = 0;
        if( Count > m_SmartMaxArtistsList )
            Index = Count - m_SmartMaxArtistsList;
        for( ; Index < Count; Index++ )
        {
            m_SmartAddedArtists.Add( SongList[ Index ].m_ArtistName.Upper() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetPlayList( const wxArrayString &files )
{
    m_PlayListCtrl->ClearItems();

    int Index;
    int Count = files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_PlayListCtrl->AddPlayListItem( files[ Index ] );
    }
    m_PlayListCtrl->ReloadItems();
    if( m_PlayListCtrl->GetItemCount() )
    {
        m_PlayListCtrl->SetCurrent( 0 );
        SetNextTrack( m_PlayListCtrl->GetCurrent() );

        LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );

        TrackListChanged();

        // Add the added track to the smart cache
        if( m_PlaySmart )
        {
            // Reset the Smart played items
            m_SmartAddedTracks.Empty();
            m_SmartAddedArtists.Empty();

            //guLogMessage( wxT( "SetPlayList adding track to smart cache..." ) );
            int Count;
            int Index = 0;
            Count = m_PlayListCtrl->GetItemCount();
            // We only insert the last CACHEITEMS as the rest should be forgiven
            if( Count > m_SmartMaxTracksList )
                Index = Count - m_SmartMaxTracksList;
            for( ; Index < Count; Index++ )
            {
                guTrack * Track = m_PlayListCtrl->GetItem( Index );
                m_SmartAddedTracks.Add( Track->m_SongId );
            }

            Index = 0;
            if( Count > m_SmartMaxArtistsList )
                Index = Count - m_SmartMaxArtistsList;
            for( ; Index < Count; Index++ )
            {
                guTrack * Track = m_PlayListCtrl->GetItem( Index );
                m_SmartAddedArtists.Add( Track->m_ArtistName.Upper() );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const guTrackArray &tracks, const bool allowplay, const bool aftercurrent )
{
    int PrevTrackCount = m_PlayListCtrl->GetCount();
    if( tracks.Count() )
    {
        guTrack * Track = &tracks[ 0 ];
        bool ClearPlayList = ( Track->m_TrackMode == guTRACK_MODE_RANDOM ||
                               Track->m_TrackMode == guTRACK_MODE_SMART ) && !m_DelTracksPlayed;

        m_PlayListCtrl->AddToPlayList( tracks, ClearPlayList, aftercurrent );

        TrackListChanged();

        if( m_PlaySmart )
        {
            int Count;
            int Index;
            Count = tracks.Count();
            // We only insert the last CACHEITEMS as the rest should be forgiven
            for( Index = 0; Index < Count; Index++ )
            {
                if( tracks[ Index ].m_TrackMode != guTRACK_MODE_SMART )
                {
                    m_SmartAddedTracks.Add( tracks[ Index ].m_SongId );
                    m_SmartAddedArtists.Add( tracks[ Index ].m_ArtistName.Upper() );
                }
            }

            if( ( Count = m_SmartAddedTracks.Count() ) > m_SmartMaxTracksList )
                m_SmartAddedTracks.RemoveAt( 0, Count - m_SmartMaxTracksList );

            if( ( Count = m_SmartAddedArtists.Count() ) > m_SmartMaxArtistsList )
                m_SmartAddedArtists.RemoveAt( 0, Count - m_SmartMaxArtistsList );
        }

        if( allowplay && !PrevTrackCount && ( GetState() != guMEDIASTATE_PLAYING ) )
        {
            wxCommandEvent CmdEvent;
            OnNextTrackButtonClick( CmdEvent );
            OnPlayButtonClick( CmdEvent );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const wxString &FileName, const bool aftercurrent  )
{
    int PrevTrackCount = m_PlayListCtrl->GetCount();

    m_PlayListCtrl->AddPlayListItem( FileName, false, aftercurrent ? m_PlayListCtrl->GetCurItem() + 1 : wxNOT_FOUND );
    m_PlayListCtrl->ReloadItems();
    TrackListChanged();
    // Add the added track to the smart cache
    if( m_PlaySmart )
    {
        int Count = m_PlayListCtrl->GetCount();

        // TODO : Check if the track was really added or not
        if( Count > PrevTrackCount )
        {
            guTrack * Track = m_PlayListCtrl->GetItem( Count - 1 );

            m_SmartAddedTracks.Add( Track->m_SongId );
            m_SmartAddedArtists.Add( Track->m_ArtistName.Upper() );

            if( ( int ) m_SmartAddedTracks.Count() > m_SmartMaxTracksList )
                m_SmartAddedTracks.RemoveAt( 0 );

            if( ( int ) m_SmartAddedArtists.Count() > m_SmartMaxArtistsList )
                m_SmartAddedArtists.RemoveAt( 0 );
        }
    }

    if( !PrevTrackCount && m_PlayListCtrl->GetCount() && ( GetState() != guMEDIASTATE_PLAYING ) )
    {
        wxCommandEvent CmdEvent;
        OnNextTrackButtonClick( CmdEvent );
        OnPlayButtonClick( CmdEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const wxArrayString &files, const bool aftercurrent  )
{
    int PrevTrackCount = m_PlayListCtrl->GetItemCount();

    int Index;
    int Count = files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_PlayListCtrl->AddPlayListItem( files[ Index ], false, aftercurrent ? m_PlayListCtrl->GetCurItem() + 1 + Index : wxNOT_FOUND );
    }

    m_PlayListCtrl->ReloadItems();
    TrackListChanged();

    // Add the added track to the smart cache
    if( m_PlaySmart )
    {
        Count = m_PlayListCtrl->GetItemCount();
        // We only insert the last CACHEITEMS as the rest should be forgiven
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = m_PlayListCtrl->GetItem( Index );

            if( m_SmartAddedTracks.Index( Track->m_SongId ) == wxNOT_FOUND )
                m_SmartAddedTracks.Add( Track->m_SongId );

            if( m_SmartAddedArtists.Index( Track->m_ArtistName.Upper() ) == wxNOT_FOUND )
                m_SmartAddedArtists.Add( Track->m_ArtistName.Upper() );

        }

        if( ( Count = m_SmartAddedTracks.Count() ) > m_SmartMaxTracksList )
            m_SmartAddedTracks.RemoveAt( 0, Count - m_SmartMaxTracksList );

        if( ( Count = m_SmartAddedArtists.Count() ) > m_SmartMaxArtistsList )
            m_SmartAddedArtists.RemoveAt( 0, Count - m_SmartMaxArtistsList );
    }

    if( !PrevTrackCount && m_PlayListCtrl->GetItemCount() && ( GetState() != guMEDIASTATE_PLAYING ) )
    {
        wxCommandEvent CmdEvent;
        OnNextTrackButtonClick( CmdEvent );
        OnPlayButtonClick( CmdEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::TrackListChanged( void )
{
//    m_PlayListLenStaticText->SetLabel( m_PlayListCtrl->GetLengthStr() );
//   	m_PlayListLabelsSizer->Layout();
//    m_PlayListCtrl->SetColumnLabel( 0, _( "Now Playing" ) +
//        wxString::Format( wxT( ":  %i / %i    ( %s )" ),
//            m_PlayListCtrl->GetCurItem() + 1,
//            m_PlayListCtrl->GetCount(),
//            m_PlayListCtrl->GetLengthStr().c_str() ) );
    wxCommandEvent TitleEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATETITLE );
    wxPostEvent( m_MainFrame, TitleEvent );

    wxCommandEvent TracksChangedEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, TracksChangedEvent );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListUpdated( wxCommandEvent &event )
{
    m_PlayListCtrl->ReloadItems();
    //SetNextTrack( m_PlayListCtrl->GetCurrent() );
    m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );

    // If a Player reset is needed
    if( event.GetExtraLong() && ( m_MediaCtrl->GetState() != guMEDIASTATE_PLAYING ) )
    {
        //OnStopButtonClick( event );
        //OnPlayButtonClick( event );
        if( m_PlayListCtrl->GetCount() )
        {
            event.SetInt( 0 );
            OnPlayListDClick( event );
        }
    }
    if( ( event.GetExtraLong() || event.GetInt() ) && m_PlaySmart )
    {
        // Reset the Smart added songs cache
        m_SmartAddedTracks.Empty();
        m_SmartAddedArtists.Empty();

        int Count;
        int Index = 0;
        Count = m_PlayListCtrl->GetCount();
        // We only insert the last CACHEITEMS as the rest should be forgiven
        if( Count > m_SmartMaxTracksList )
            Index = Count - m_SmartMaxTracksList;
        for( ; Index < Count; Index++ )
        {
            guTrack * Track = m_PlayListCtrl->GetItem( Index );
            m_SmartAddedTracks.Add( Track->m_SongId );
        }

        Index = 0;
        Count = m_PlayListCtrl->GetCount();
        if( Count > m_SmartMaxArtistsList )
            Index = Count - m_SmartMaxArtistsList;
        for( ; Index < Count; Index++ )
        {
            guTrack * Track = m_PlayListCtrl->GetItem( Index );
            m_SmartAddedArtists.Add( Track->m_ArtistName.Upper() );
        }
    }
    TrackListChanged();
}

//// -------------------------------------------------------------------------------- //
//void guPlayerPanel::UpdateStatus()
//{
//    guMediaState State;
//    //wxFileOffset CurPos;
//    static bool IsUpdatingStatus = false;
//
//    if( IsUpdatingStatus )
//        return;
//
//    IsUpdatingStatus = true;
//
//    State = m_MediaCtrl->GetState();
//
//    if( State == guMEDIASTATE_PLAYING )
//    {
//        // Some track lengths are not correctly read by taglib so
//        // we try to find the length from gstreamer and update the database
//        // We need to not do this for radiostations or online streams
//        if( m_MediaSong.m_Length == 0 &&
//            m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
//        {
//            m_MediaSong.m_Length = m_MediaCtrl->GetLength();
//
//            if( m_MediaSong.m_SongId )
//            {
//                if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
//                    m_Db->UpdateTrackLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
//                else if( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST )
//                    m_Db->UpdatePodcastItemLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
//            }
//
//            // Update the track in database, playlist, etc
//            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
//        }
//
//        // When tags are received while buffering the rename gets pending till the track start playing again
//        // To avoid get the stream paused.
//        if( m_PendingNewRecordName && ( m_BufferGaugeId == wxNOT_FOUND ) )
//        {
//            m_PendingNewRecordName = false;
//            m_MediaRecordCtrl->SplitTrack();
//        }
//    }
//
//    // Total Length
//    if( m_LastTotalLen != m_PlayListCtrl->GetLength() )
//    {
//        m_LastTotalLen = m_PlayListCtrl->GetLength();
//        TrackListChanged();
//    }
//
//    IsUpdatingStatus = false;
//}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnSmartAddTracks( wxCommandEvent &event )
{
    guTrackArray * Songs = ( guTrackArray * ) event.GetClientData();
    if( Songs )
    {
        if( Songs->Count() )
        {
            AddToPlayList( * Songs );
        }
        delete Songs;
    }
    m_SmartSearchEnabled = false;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SmartAddTracks( const guTrack &CurSong )
{
    // if its already searching
    if( m_SmartSearchEnabled )
        return;

    m_SmartSearchEnabled = true;
    m_SmartAddTracksThread = new guSmartAddTracksThread( m_Db, this, &CurSong,
        &m_SmartAddedTracks, &m_SmartAddedArtists, m_SmartPlayAddTracks,
        m_SmartMaxTracksList, m_SmartMaxArtistsList,
        m_PlayerFilters->GetAllowFilterId(),
        m_PlayerFilters->GetDenyFilterId() );
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetCurrentItem()
{
    return m_PlayListCtrl->GetCurItem();
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetItemCount()
{
    return m_PlayListCtrl->GetItemCount();
}

// -------------------------------------------------------------------------------- //
const guTrack * guPlayerPanel::GetTrack( int index )
{
    return m_PlayListCtrl->GetItem( index );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::RemoveItem( int itemnum )
{
    m_PlayListCtrl->RemoveItem( itemnum );
    m_PlayListCtrl->ReloadItems();
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetCaps()
{
    return m_PlayListCtrl->GetCaps();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetNextTrack( const guTrack * Song )
{
    //guLogMessage( wxT( "SetNextTrack: %i" ), m_PlayListCtrl->GetCurItem() );

    if( !Song )
        return;

    m_NextSong = * Song;
    //m_NextTrackId = true;

}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListDClick( wxCommandEvent &event )
{
    int item = event.GetInt();
    m_PlayListCtrl->SetCurrent( item, m_DelTracksPlayed && !m_PlayLoop );

    SetNextTrack( m_PlayListCtrl->GetCurrent() );
    //wxLogMessage( wxT( "Selected %i : %s - %s" ), m_MediaSong.SongId, m_MediaSong.ArtistName.c_str(), m_MediaSong.SongName.c_str() );

    LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );
}

// -------------------------------------------------------------------------------- //
wxString inline FileNameEncode( const wxString filename )
{
    static const wxChar NumChars[] = wxT( "0123456789" );

    wxString RetVal = filename;
    int Pos;
    while( ( Pos = RetVal.Find( wxT( "%" ) ) ) != wxNOT_FOUND &&
            !( wxStrchr( NumChars, RetVal[ Pos + 1 ] ) &&
               wxStrchr( NumChars, RetVal[ Pos + 2 ] ) ) )
    {
        RetVal = RetVal.Mid( 0, Pos ) + wxT( "%25" ) + RetVal.Mid( Pos + 1 );
    }
    RetVal.Replace( wxT( "#" ), wxT( "%23" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::LoadMedia( guFADERPLAYBIN_PLAYTYPE playtype )
{
    if( m_MediaSong.m_Type & guTRACK_TYPE_STOP_HERE )
    {
        m_MediaSong.m_Type = guTrackType( int( m_MediaSong.m_Type ) ^ guTRACK_TYPE_STOP_HERE );
        m_PlayListCtrl->ClearStopAfterCurrent();
        return;
    }
    //guLogMessage( wxT( "LoadMedia Cur: %i  %i" ), m_PlayListCtrl->GetCurItem(), playtype );
    //m_MediaCtrl->Load( NextItem->FileName );
    wxURI UriPath( m_NextSong.m_FileName );
    wxString Uri;
    try {
        if( !UriPath.HasScheme() )
            Uri = wxT( "file://" ) + m_NextSong.m_FileName;
        else
            Uri = m_NextSong.m_FileName;

        //guLogMessage( wxT( "'%s'\n'%s'" ), FileName.c_str(), FileNameEncode( Uri ).c_str() );
        m_NextTrackId = m_MediaCtrl->Load( FileNameEncode( Uri ), playtype );
        if( !m_NextTrackId )
        {
            guLogError( wxT( "ee: Failed load of file '%s'" ), Uri.c_str() );
            //guLogError( wxT( "ee: The filename was '%s'" ), FileName.c_str() );
            //m_MediaCtrl->CleanPlayBins();

            int CurItem = m_PlayListCtrl->GetCurItem();
            m_IsSkipping = false;

            wxCommandEvent event;
            event.SetInt( playtype );
            OnNextTrackButtonClick( event );

            RemoveItem( CurItem );
            //m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
        }
    }
    catch(...)
    {
        guLogError( wxT( "Error loading '%s'" ), m_NextSong.m_FileName.c_str() );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaBuffering( guMediaEvent &event )
{
    wxCommandEvent GaugeEvent( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
    int Percent = event.GetInt();
//    printf( "Buffering: %d%%\n", Percent );
    if( Percent == 100 )
    {
        if( m_BufferGaugeId != wxNOT_FOUND )
        {
            GaugeEvent.SetId( ID_STATUSBAR_GAUGE_REMOVE );
            GaugeEvent.SetInt( m_BufferGaugeId );
            wxPostEvent( m_MainFrame, GaugeEvent );
            m_BufferGaugeId = wxNOT_FOUND;
        }
    }
    else
    {
        if( m_BufferGaugeId == wxNOT_FOUND )
        {
              m_BufferGaugeId = ( ( guStatusBar * ) m_MainFrame->GetStatusBar() )->AddGauge( _( "Buffering..." ) );
              GaugeEvent.SetId( ID_STATUSBAR_GAUGE_SETMAX );
              GaugeEvent.SetInt( m_BufferGaugeId );
              GaugeEvent.SetExtraLong( 100 );
              wxPostEvent( m_MainFrame, GaugeEvent );
        }

        if( m_BufferGaugeId != wxNOT_FOUND )
        {
            GaugeEvent.SetId( ID_STATUSBAR_GAUGE_UPDATE );
            GaugeEvent.SetInt( m_BufferGaugeId );
            GaugeEvent.SetExtraLong( Percent );
            wxPostEvent( m_MainFrame, GaugeEvent );
        }
    }
}


// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaLevel( guMediaEvent &event )
{
    guLevelInfo * LevelInfo = ( guLevelInfo * ) event.GetClientObject();

    // We only enable to check if :
    // * Already not detected
    // * Its enabled in preferences
    // * Its not a radiostation
    // * Not enabled a time range or
    // * TrackLength bigger in 10 secs of the minimun time range
    if( !m_SilenceDetected && m_SilenceDetector &&
        ( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION ) &&
        ( !m_SilenceDetectorTime ||
          ( ( m_MediaSong.m_Length * 1000 ) > ( unsigned int ) ( m_SilenceDetectorTime + 10000 ) ) ) )
    {
        //guLogMessage( wxT( "Decay Level: %0.2f /  %02i  %s  %li > %li" ), LevelInfo->m_Decay_L, m_SilenceDetectorLevel, LenToString( ( unsigned int ) ( m_LastCurPos / 1000 ) ).c_str(), m_MediaSong.m_Length * 1000, ( unsigned int ) ( m_SilenceDetectorTime + 10000 ) );
        if( LevelInfo->m_Decay_L < double( m_SilenceDetectorLevel ) )
        {
            unsigned long EventTime = m_LastCurPos; //LevelInfo->m_EndTime;
            if( EventTime > 10000 )
            {
                unsigned long TrackLength = m_MediaSong.m_Length * 1000;
                //guLogMessage( wxT( "The level is now lower than triger level" ) );
                //guLogMessage( wxT( "(%f) %02i : %li , %i, %i" ), LevelInfo->m_Decay_L, m_SilenceDetectorLevel, EventTime, TrackLength - EventTime, m_SilenceDetectorTime );
                //guLogMessage( wxT( "(%li) %f %02i : %li , %i, %i" ), event.GetExtraLong(), LevelInfo->m_Decay_L, m_SilenceDetectorLevel, EventTime, TrackLength - EventTime, m_SilenceDetectorTime );


                // We only skip to next track if the level is lower than the triger one and also if
                // we are at the end time period (if configured this way) and the time left is more than 500msecs
                if( !m_TrackChanged && !m_NextTrackId && ( m_CurTrackId == event.GetExtraLong() ) &&
                    ( !m_SilenceDetectorTime || ( ( ( unsigned int ) m_SilenceDetectorTime > ( TrackLength - EventTime ) ) &&
                      ( ( EventTime + 500 ) < TrackLength ) ) ) )
                {
                    m_SilenceDetected = true;
                    //guLogMessage( wxT( "Silence detected. Changed to next track %i" ), m_PlayListCtrl->GetCurItem() );
                    wxCommandEvent evt;
                    evt.SetId( ID_PLAYERPANEL_NEXTTRACK );
                    OnNextTrackButtonClick( evt );
                }
            }
        }
    }

    if( m_PlayerVumeters )
    {
        //guLogMessage( wxT( "%lli %lli   (%lli)" ), LevelInfo->m_EndTime, LevelInfo->m_OutTime, LevelInfo->m_OutTime - LevelInfo->m_EndTime );
        m_PlayerVumeters->SetLevels( * LevelInfo );

//        guLogMessage( wxT( "L:%02.02f  R:%02.02f" ),
//            event.m_LevelInfo.m_Peak_L,
//            event.m_LevelInfo.m_Peak_R );
    }

    delete LevelInfo;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaError( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaError: %i" ), m_PlayListCtrl->GetCurItem() );
    wxString * ErrorStr = ( wxString * ) event.GetClientData();
    if( ErrorStr )
    {
        m_NotifySrv->Notify( wxEmptyString, wxT( "Guayadeque: GStreamer Error" ), * ErrorStr, NULL );

        delete ErrorStr;
    }
    else
    {
        m_NotifySrv->Notify( wxEmptyString, wxT( "Guayadeque: GStreamer Error" ), _( "Unknown" ), NULL );
    }

    if( m_MediaCtrl->GetState() == guMEDIASTATE_PLAYING )
        m_ErrorFound = true;

    m_MediaCtrl->ClearError();

    int CurItem = m_PlayListCtrl->GetCurItem();

    m_IsSkipping = false;
    m_NextTrackId = 0;

    wxCommandEvent CmdEvent;
    CmdEvent.SetInt( guFADERPLAYBIN_PLAYTYPE_REPLACE );
    OnNextTrackButtonClick( CmdEvent );

    RemoveItem( CurItem );

//    m_MediaCtrl->SetCurrentState( GST_STATE_READY );

//    m_MediaCtrl->CleanPlayBins();

//    OnNextTrackButtonClick( CmdEvent );
//
//    OnPlayButtonClick( CmdEvent );

//    // Be sure it will not try to play the track again
//    event.SetInt( 0 );
//    // Simulate the track loaded correctly.
//    OnMediaLoaded( event );
//
//    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STOP );
//    AddPendingEvent( CmdEvent );
//
//    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_NEXTTRACK );
//    AddPendingEvent( CmdEvent );

//    CmdEvent.SetId( ID_PLAYERPANEL_STOP );
//    AddPendingEvent( CmdEvent );
//
//    CmdEvent.SetId( ID_PLAYERPANEL_PLAY );
//    AddPendingEvent( CmdEvent );

//    wxCommandEvent CmdEvent;
//    OnNextTrackButtonClick( CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaState( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaState: %i %li %li" ), event.GetInt(), m_CurTrackId, m_NextTrackId );
    GstState State = ( GstState ) event.GetInt();

    if( State == GST_STATE_PLAYING && m_NextTrackId )
    {
        OnMediaPlayStarted();
    }


    if( State != m_LastPlayState )
    {
        if( State == GST_STATE_PLAYING ) //guMEDIASTATE_PLAYING )
        {
            m_PlayButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_pause ) );
            m_PlayButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_pause ) );
        }
        else
        {
            m_PlayButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_play ) );
            m_PlayButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_play ) );

            if( State == GST_STATE_READY )
            {
                UpdatePositionLabel( 0 );
                m_PlayerPositionSlider->SetValue( 0 );
            }
        }
        m_PlayButton->Refresh();
        m_LastPlayState = State;
        //
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
        wxPostEvent( m_MainFrame, event );

    }

    if( m_ErrorFound )
    {
        m_ErrorFound = false;

        wxCommandEvent CmdEvent;
        OnNextTrackButtonClick( CmdEvent );
        OnPlayButtonClick( CmdEvent );
    }
}

// -------------------------------------------------------------------------------- //
void  guPlayerPanel::OnMediaPosition( guMediaEvent &event )
{
    //guLogMessage( wxT( "OnMediaPosition... %i - %li" ), event.GetInt(), event.GetExtraLong() );

    if( event.GetInt() < 0 )
        return;

#if __WORDSIZE == 64
     int EventId = ( gint64 ) event.GetClientData();
#else
     int EventId = ( int ) event.GetClientData();
#endif
    if( EventId != m_CurTrackId )
        return;

    wxFileOffset CurPos = event.GetInt();
    wxFileOffset CurLen = event.GetExtraLong();
    if( CurLen && ( CurLen != m_LastLength ) )
    {
        m_LastLength = CurLen;

//        if( !m_LastLength )
//            m_PlayerPositionSlider->SetValue( 0 );

        // Some track lengths are not correctly read by taglib so
        // we try to find the length from gstreamer and update the database
        // We need to not do this for radiostations or online streams
        if( CurLen > 0 && m_MediaSong.m_Length == 0 &&
            m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
        {
            m_MediaSong.m_Length = CurLen / 1000;

            if( m_MediaSong.m_SongId )
            {
                if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
                {
                    guDbLibrary * Db = m_MediaSong.m_LibPanel ? m_MediaSong.m_LibPanel->GetDb() : m_Db;
                    Db->UpdateTrackLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
                }
                else if( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST )
                {
                    m_Db->UpdatePodcastItemLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
                }
            }

            // Update the track in database, playlist, etc
            m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
        }
    }

    if( ( ( CurPos / 1000 ) != ( m_LastCurPos / 1000 ) ) && !m_SliderIsDragged )
    {
        guLogMessage( wxT( "OnMediaPosition... %i - %li   %li %li" ), event.GetInt(), event.GetExtraLong(), m_CurTrackId, m_NextTrackId );
        m_LastCurPos = CurPos;

        if( m_TrackChanged )
            m_TrackChanged = false;

        UpdatePositionLabel( CurPos / 1000 );

        if( m_LastLength )
        {
            m_PlayerPositionSlider->SetValue( event.GetInt() / ( m_LastLength / 1000 ) );
        }

        m_MediaSong.m_PlayTime = CurPos / 1000;

        if( !m_AboutToEndDetected && !m_NextTrackId && ( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION ) &&
            ( CurPos > 0 ) && ( m_LastLength > 0 ) && ( CurPos + m_FadeOutTime + 5000 >= m_LastLength ) )
        {
            if( GetState() == guMEDIASTATE_PLAYING )
            {
                m_AboutToEndDetected = true;

                guLogMessage( wxT( "Detected about to finish track... Trying to load the next track..." ) );
                wxCommandEvent evt;
                OnNextTrackButtonClick( evt );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void  guPlayerPanel::OnMediaLength( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaLength... %i" ), event.GetInt() );
    wxFileOffset CurLen = event.GetInt() / 1000;

    if( CurLen != m_LastLength )
    {
        m_LastLength = CurLen;

        UpdatePositionLabel( m_LastCurPos );
        m_MediaSong.m_Length = CurLen;
    }
}

// -------------------------------------------------------------------------------- //
void ExtractMetaData( wxString &title, wxString &artist, wxString &trackname )
{
    int FindPos;
    if( !title.IsEmpty() )
    {
        FindPos = title.Find( wxT( " - " ) );
        if( FindPos == wxNOT_FOUND )
            FindPos = title.Find( wxT( "_-_" ) );
        if( FindPos != wxNOT_FOUND )
        {
            artist = title.Mid( 0, FindPos );
            title = title.Mid( FindPos + 3 );
            FindPos = title.Find( wxT( " - " ) );
            if( FindPos == wxNOT_FOUND )
                FindPos = title.Find( wxT( "_-_" ) );
            if( FindPos != wxNOT_FOUND )
            {
                trackname = title.Mid( 0, FindPos );
            }
            else
            {
                trackname = title;
            }
        }
        else
        {
            trackname = title;
        }
    }
}


// -------------------------------------------------------------------------------- //
void guPlayerPanel::SendRecordSplitEvent( void )
{
    // If its buffering
    if( m_BufferGaugeId != wxNOT_FOUND )
    {
        m_PendingNewRecordName = true;
        //guLogMessage( wxT( "Player is buffering. Will rename recording once its finished" ) );
        return;
    }

    m_MediaRecordCtrl->SplitTrack();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaTags( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaTags..." ) );
    guRadioTagInfo * RadioTag = ( guRadioTagInfo * ) event.GetClientData();
    if( RadioTag )
    {
        if( ( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION ) ||
            ( m_NextTrackId && ( m_NextSong.m_Type == guTRACK_TYPE_RADIOSTATION ) ) )
        {
            //guLogMessage( wxT( "Radio Name: %s" ), wxString( RadioTag->m_Organization, wxConvUTF8 ).c_str() );
            if( RadioTag->m_Organization )
            {
                if( m_NextTrackId )
                {
                    m_NextSong.m_AlbumName = wxString( RadioTag->m_Organization, wxConvUTF8 );
                    while( m_NextSong.m_AlbumName.StartsWith( wxT( "." ) ) )
                        m_NextSong.m_AlbumName = m_NextSong.m_AlbumName.Mid( 1 );
                    if( m_MediaRecordCtrl && m_MediaRecordCtrl->IsRecording() )
                    {
                        m_MediaRecordCtrl->SetStation( m_NextSong.m_AlbumName );
                    }
                }
                else
                {
                    m_MediaSong.m_AlbumName = wxString( RadioTag->m_Organization, wxConvUTF8 );
                    SetAlbumLabel( m_MediaSong.m_AlbumName );
                    if( m_MediaRecordCtrl && m_MediaRecordCtrl->IsRecording() )
                    {
                        m_MediaRecordCtrl->SetStation( m_MediaSong.m_AlbumName );
                    }
                }
            }


            if( RadioTag->m_Genre )
            {
                //guLogMessage( wxT( "Radio Genre: %s" ), wxString( RadioTag->m_Genre, wxConvUTF8 ).c_str() );
                if( m_NextTrackId )
                {
                    m_NextSong.m_GenreName = wxString( RadioTag->m_Genre, wxConvUTF8 );
                    if( m_MediaRecordCtrl && m_MediaRecordCtrl->IsRecording() )
                    {
                        m_MediaRecordCtrl->SetGenre( m_NextSong.m_GenreName );
                    }
                }
                else
                {
                    m_MediaSong.m_GenreName = wxString( RadioTag->m_Genre, wxConvUTF8 );
                    if( m_MediaRecordCtrl && m_MediaRecordCtrl->IsRecording() )
                    {
                        m_MediaRecordCtrl->SetGenre( m_MediaSong.m_GenreName );
                    }
                }
            }

            //guLogMessage( wxT( "MediaTag:'%s'" ), TagStr->c_str() );
            if( RadioTag->m_Title )
            {
                wxString Title( RadioTag->m_Title, wxConvUTF8 );
                //guLogMessage( wxT( "Radio Title: %s" ), Title.c_str() );
                if( m_NextTrackId )
                {
                    ExtractMetaData( Title,
                            m_NextSong.m_ArtistName,
                            m_NextSong.m_SongName );
                }
                else
                {
                    ExtractMetaData( Title,
                            m_MediaSong.m_ArtistName,
                            m_MediaSong.m_SongName );
                    SetTitleLabel( m_MediaSong.m_SongName );
                    SetArtistLabel( m_MediaSong.m_ArtistName );
                }

                //guLogMessage( wxT( "AlbumName: '%s'" ), m_MediaSong.m_AlbumName.c_str() );

                if( m_MediaRecordCtrl && m_MediaRecordCtrl->IsRecording() )
                {
                    if( m_NextTrackId )
                        m_MediaRecordCtrl->SetTrackName( m_NextSong.m_ArtistName, m_NextSong.m_SongName );
                    else
                        m_MediaRecordCtrl->SetTrackName( m_MediaSong.m_ArtistName, m_MediaSong.m_SongName );

                    SendRecordSplitEvent();
                }

                //guLogMessage( wxT( "Sending LastFMPanel::UpdateTrack event" ) );
                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
                event.SetClientData( new guTrack( m_MediaSong ) );
                wxPostEvent( m_MainFrame, event );

                wxImage Image( guImage( guIMAGE_INDEX_net_radio ) );
                SendNotifyInfo( &Image );
            }
            GetSizer()->Layout();
        }
        delete RadioTag;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaBitrate( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaBitrate... (%li) %i" ), event.GetExtraLong(), event.GetInt() );
//
//    if( m_NextSong.m_Bitrate != BitRate )
//    {
//        m_NextSong.m_Bitrate = BitRate;
//
//        if( m_NextSong.m_Type == guTRACK_TYPE_DB )
//        {
//            m_Db->UpdateTrackBitRate( m_NextSong.m_SongId, BitRate );
//
//        }
//
//        // Update the track in database, playlist, etc
//        m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_NextSong );
//    }
    //SetBitRateLabel( BitRate );
    if( event.GetExtraLong() == m_CurTrackId )
    {
        SetBitRate( event.GetInt() );
    }
    else
    {
        int BitRate = ( event.GetInt() / 1000 );
        if( event.GetExtraLong() == m_NextTrackId )
        {
            if( m_NextSong.m_Bitrate != BitRate )
            {
                m_NextSong.m_Bitrate = BitRate;
            }
        }
    }

}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaLoaded( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaLoaded Cur: %i %i   %li" ), m_PlayListCtrl->GetCurItem(), event.GetInt(), m_NextTrackId );

    if( m_IsSkipping )
        m_IsSkipping = false;

    try {

        if( event.GetInt() )
        {
            //
            m_MediaCtrl->Play();
        }

        if( m_TrackStartPos )
        {
            //guLogMessage( wxT( "Try to set saved position %i" ), m_TrackStartPos );
            SetPosition( m_TrackStartPos );
            m_TrackStartPos = 0;
        }

        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
    catch(...)
    {
        OnNextTrackButtonClick( event );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaPlayStarted( void )
{
    guLogMessage( wxT( "OnMediaPlayStarted  %li" ), m_NextTrackId );

    SavePlayedTrack();

    // Enable or disables the record button. Only enabled for radio stations
    m_RecordButton->Enable( ( m_NextSong.m_Type == guTRACK_TYPE_RADIOSTATION ) );
    if( m_RecordButton->GetValue() )
    {
        m_RecordButton->SetValue( ( m_NextSong.m_Type == guTRACK_TYPE_RADIOSTATION ) );
        if( !m_RecordButton->GetValue() )
        {
            m_MediaRecordCtrl->Stop();
        }
        else
        {
            m_MediaRecordCtrl->Start( &m_NextSong );
            //m_MediaRecordCtrl->SetTrack( m_NextSong );
        }
    }

    // Set the Current Song
    m_CurTrackId = m_NextTrackId;
    m_NextTrackId = 0;
    m_MediaSong = m_NextSong;
    m_TrackChanged = true;
    m_SavedPlayedTrack = false;
    m_SilenceDetected = false;
    m_AboutToEndDetected = false;
    m_LastLength = m_MediaSong.m_Length * 1000;
    if( !m_LastLength )
        m_PlayerPositionSlider->SetValue( 0 );

    // Update the Current Playing Song Info
    UpdateLabels();
    //UpdatePositionLabel( 0 );

//    m_PlayListCtrl->SetColumnLabel( 0, _( "Now Playing" ) +
//        wxString::Format( wxT( ":  %i / %i    ( %s )" ),
//            m_PlayListCtrl->GetCurItem() + 1,
//            m_PlayListCtrl->GetCount(),
//            m_PlayListCtrl->GetLengthStr().c_str() ) );
    wxCommandEvent TitleEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATETITLE );
    wxPostEvent( m_MainFrame, TitleEvent );


    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
    event.SetClientData( new guTrack( m_MediaSong ) );
    wxPostEvent( m_MainFrame, event );

    // Update the cover
    UpdateCover();

    // Check if Smart is enabled
    if( ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) && m_PlaySmart &&
        ( ( m_PlayListCtrl->GetCurItem() + m_SmartPlayMinTracksToPlay ) >= m_PlayListCtrl->GetCount() ) )
    {
        SmartAddTracks( m_MediaSong );
    }

    // If its a Radio disable PositionSlider
    //m_PlayerPositionSlider->SetValue( 0 );
    if( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION )
    {
        m_PlayerPositionSlider->Disable();
    }
    else if( !m_PlayerPositionSlider->IsEnabled() )
    {
        m_PlayerPositionSlider->Enable();
    }

    // Send the CapsChanged Event
    //wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_CAPSCHANGED );
    event.SetId( ID_PLAYERPANEL_CAPSCHANGED );
    wxPostEvent( m_MainFrame, event );


    if( m_AudioScrobbleEnabled && m_AudioScrobble && m_AudioScrobble->IsOk() )
    {
        m_AudioScrobble->SendNowPlayingTrack( m_MediaSong );
        //m_PendingScrob = false;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetCurrentCoverImage( wxImage * coverimage, const guSongCoverType covertype, const wxString &coverpath )
{
    if( coverimage && coverimage->IsOk() )
    {
        m_MediaSong.SetCoverImage( coverimage );
        m_MediaSong.m_CoverType = covertype;
        m_MediaSong.m_CoverPath = coverpath;
    }
    else //if( covertype == GU_SONGCOVER_NONE )
    {
        m_MediaSong.SetCoverImage( new wxImage( guImage( guIMAGE_INDEX_no_cover ) ) );
        m_MediaSong.m_CoverType = GU_SONGCOVER_NONE;
        m_MediaSong.m_CoverPath = wxEmptyString;
    }
    UpdateCoverImage();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdateCoverImage( const bool shownotify )
{
    m_PlayerCoverBitmap->SetBitmap( wxBitmap( * m_MediaSong.m_CoverImage ) );
    m_PlayerCoverBitmap->Refresh();

    if( shownotify )
        SendNotifyInfo( m_MediaSong.m_CoverImage );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_COVERUPDATED );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdateCover( const bool shownotify )
{
    if( m_UpdateCoverThread )
    {
        m_UpdateCoverThread->Pause();
        m_UpdateCoverThread->Delete();
    }

    m_UpdateCoverThread = new guUpdatePlayerCoverThread( m_Db, m_MainFrame, this, &m_MediaSong, shownotify );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnCoverUpdated( wxCommandEvent &event )
{
    m_UpdateCoverThread = NULL;

    UpdateCoverImage( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SavePlayedTrack( void )
{
    guLogMessage( wxT( "SavePlayedTrack %i     %li" ), m_SavedPlayedTrack, m_NextTrackId );

    if( m_SavedPlayedTrack )
        return;

    m_SavedPlayedTrack = true;

    // Check if the Current Song have played more than the half and if so add it to
    // The CachedPlayedSong database to be submitted to LastFM AudioScrobbling
    //if( m_AudioScrobbleEnabled && ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) ) // If its not a radiostation
    if( m_AudioScrobbleEnabled && ( m_MediaSong.m_Type != guTRACK_TYPE_PODCAST ) ) // If its not a podcast
    {
        guLogMessage( wxT( "PlayTime: %u Length: %u" ), m_MediaSong.m_PlayTime, m_MediaSong.m_Length );
        if( ( ( m_MediaSong.m_PlayTime > guAS_MIN_PLAYTIME ) || // If have played more than the min amount of time
            ( m_MediaSong.m_PlayTime >= ( m_MediaSong.m_Length / 2 ) ) ) && // If have played at least the half
            ( m_MediaSong.m_PlayTime > guAS_MIN_TRACKLEN ) )    // If the Length is more than 30 secs
        {
            if( !m_MediaSong.m_SongName.IsEmpty() &&    // Check if we have no missing data
                !m_MediaSong.m_ArtistName.IsEmpty() )
            {
                //
                m_AudioScrobble->SendPlayedTrack( m_MediaSong );
            }
        }
    }

    // Update the play count if it has player at least the half of the track
    if( m_MediaSong.m_Loaded &&
        ( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
          ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) ) )  // If its a song from the library
    {
        if( m_MediaSong.m_PlayTime >= ( m_MediaSong.m_Length / 2 ) )  // If have played at least the half
        {
            m_MediaSong.m_PlayCount++;

            if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
            {
                guDbLibrary * Db = m_MediaSong.m_LibPanel ? m_MediaSong.m_LibPanel->GetDb() : m_Db;
                Db->SetTrackPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount );
            }
            else
            {
                m_Db->SetPodcastItemPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount );
            }

            // Update the track in database, playlist, etc
            m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
        }
    }

}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFinished( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaFinished (%li) Cur: %i  %li" ), event.GetExtraLong(), m_PlayListCtrl->GetCurItem(), m_NextTrackId );

    ResetVumeterLevel();
    SavePlayedTrack();

    if( m_SilenceDetected || m_AboutToEndDetected || m_NextTrackId || ( m_CurTrackId != event.GetExtraLong() ) )
    {
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
        guLogMessage( wxT( "Media Finished Cancelled... %li %li" ), m_CurTrackId, m_NextTrackId );
        return;
    }

    guTrack * NextItem = m_PlayListCtrl->GetNext( m_PlayLoop );
    if( NextItem )
    {
        //m_MediaSong = * NextItem;
        SetNextTrack( NextItem );
        LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
    else
    {
        // If the option to play a random track is set
        if( m_PlayRandom )
        {
            // If Repeat was enabled disable it
            if( m_PlayLoop )
                SetPlayLoop( guPLAYER_PLAYLOOP_NONE );

            //guLogMessage( wxT( "Getting Random Tracks..." ) );
            guTrackArray Tracks;
            if( m_Db->GetRandomTracks( &Tracks, m_SmartPlayAddTracks, m_PlayRandomMode,
                    m_PlayerFilters->GetAllowFilterId(),
                    m_PlayerFilters->GetDenyFilterId() ) )
            {
                if( Tracks.Count() )
                {
                    AddToPlayList( Tracks, false );

                    OnMediaFinished( event );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFadeOutFinished( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaFadeOutFinished (%li) Cur: %i  %li" ), event.GetExtraLong(), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFadeInStarted( guMediaEvent &event )
{
    guLogMessage( wxT( "OnMediaFadeInStarted Cur: %i  %li" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
}

// -------------------------------------------------------------------------------- //
const guMediaState guPlayerPanel::GetState( void )
{
    return m_MediaCtrl->GetState();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::CheckFiltersEnable( void )
{
    bool IsEnable = m_PlaySmart || ( !m_PlayLoop && m_PlayRandom );
    m_PlayerFilters->EnableFilters( IsEnable );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetPlaySmart( bool playsmart )
{
    m_PlaySmart = playsmart;
    m_SmartPlayButton->SetValue( m_PlaySmart );
    if( m_PlaySmart && GetPlayLoop() )
    {
        SetPlayLoop( guPLAYER_PLAYLOOP_NONE );
    }
    CheckFiltersEnable();

    // Send a notification
    wxString TipText = _( "Smart Mode: " );
    if( !playsmart )
    {
        TipText += _( "Off" );
    }
    else
    {
        TipText += _( "On" );
    }
    m_SmartPlayButton->SetToolTip( TipText );

    // Send Notification for the mpris interface
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::GetPlaySmart()
{
    return m_PlaySmart;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetPlayLoop( int playloop )
{
    m_PlayLoop = playloop;

    m_RepeatPlayButton->SetBitmapLabel( m_PlayLoop < guPLAYER_PLAYLOOP_TRACK ?
        guImage( guIMAGE_INDEX_player_normal_repeat ) :
        guImage( guIMAGE_INDEX_player_normal_repeat_single ) );

    m_RepeatPlayButton->SetBitmapDisabled( m_PlayLoop < guPLAYER_PLAYLOOP_TRACK ?
        guImage( guIMAGE_INDEX_player_light_repeat ) :
        guImage( guIMAGE_INDEX_player_light_repeat_single ) );

    m_RepeatPlayButton->SetBitmapHover( m_PlayLoop < guPLAYER_PLAYLOOP_TRACK ?
        guImage( guIMAGE_INDEX_player_highlight_repeat ) :
        guImage( guIMAGE_INDEX_player_highlight_repeat_single ) );

    m_RepeatPlayButton->SetValue( m_PlayLoop );
    if( m_PlayLoop && GetPlaySmart() )
    {
        SetPlaySmart( false );
    }

    CheckFiltersEnable();

    // Send a notification
    wxString TipText = _( "Repeat Mode: " );
    if( !m_PlayLoop )
    {
        TipText += _( "Off" );
    }
    else if( m_PlayLoop == guPLAYER_PLAYLOOP_TRACK )
    {
        TipText += _( "Track" );
    }
    else
    {
        TipText += _( "Playlist" );
    }
    m_RepeatPlayButton->SetToolTip( TipText );

    // Send Notification for the mpris interface
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetPlayLoop()
{
    return m_PlayLoop;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPrevTrackButtonClick( wxCommandEvent& event )
{
    if( wxGetKeyState( WXK_SHIFT ) )
    {
        OnPrevAlbumButtonClick( event );
        return;
    }
    guMediaState State;
//    wxFileOffset CurPos;
    guTrack * PrevItem;

    if( m_IsSkipping )
        return;

    // If we are already in the first Item start again the song from the begining
    State = m_MediaCtrl->GetState();
    //CurPos = m_MediaCtrl->Tell();
    int CurItem = m_PlayListCtrl->GetCurItem();
    if( ( ( CurItem == 0 ) && ( State == guMEDIASTATE_PLAYING ) ) ||
        ( ( State != guMEDIASTATE_STOPPED ) && ( m_LastCurPos  > GUPLAYER_MIN_PREVTRACK_POS ) ) )
    {
//        m_MediaCtrl->UnsetCrossfader();
        SetPosition( 0 );
        return;
    }

    bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_PREVTRACK ) ||
                      ( event.GetEventObject() == m_PrevTrackButton );

    PrevItem = m_PlayListCtrl->GetPrev( m_PlayLoop, ForceSkip );
    if( PrevItem )
    {
        SetNextTrack( PrevItem );

        if( State == guMEDIASTATE_PLAYING )
        {
            if( State == guMEDIASTATE_PLAYING )
            {
                m_IsSkipping = true;
                LoadMedia( ( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE :
                    ( ForceSkip ? guFADERPLAYBIN_PLAYTYPE_REPLACE : guFADERPLAYBIN_PLAYTYPE_AFTER_EOS ) ) );
            }
        }
        else
        {
            if( State == guMEDIASTATE_PAUSED )
            {
                m_MediaCtrl->Stop();
            }
            if( m_MediaSong.m_Loaded )
            {
                guCurrentTrack CurrentTrack;
                CurrentTrack.m_Rating = 0;
                CurrentTrack.m_Year = 0;
                m_MediaSong = CurrentTrack;
                m_LastCurPos = 0;
                m_LastLength = 0;
                UpdateLabels();
                UpdatePositionLabel( 0 );

                m_MediaSong.m_CoverType = GU_SONGCOVER_NONE;
                m_MediaSong.m_CoverPath = wxEmptyString;
                wxImage * CoverImage = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
                // Cover
                if( CoverImage )
                {
                    if( CoverImage->IsOk() )
                    {
                        m_PlayerCoverBitmap->SetBitmap( wxBitmap( *CoverImage ) );
                        m_PlayerCoverBitmap->Refresh();
                    }
                    delete CoverImage;
                }
            }
            //guLogMessage( wxT( "Prev Track when not playing.." ) );
//            m_MediaCtrl->SetCurrentState( GST_STATE_READY );
        }
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
//    //event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnNextTrackButtonClick( wxCommandEvent& event )
{
    if( wxGetKeyState( WXK_SHIFT ) )
    {
        OnNextAlbumButtonClick( event );
        return;
    }
    //guLogMessage( wxT( "OnNextTrackButtonClick Cur: %i    %li   %i" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId, event.GetInt() );
    guMediaState State;
    guTrack * NextItem;

    if( m_IsSkipping )
        return;

    bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_NEXTTRACK ) ||
                      ( event.GetEventObject() == m_NextTrackButton );

    NextItem = m_PlayListCtrl->GetNext( m_PlayLoop, ForceSkip );
    if( NextItem )
    {
        State = m_MediaCtrl->GetState();
        //guLogMessage( wxT( "OnNextTrackButtonClick : State = %i" ), State );

        SetNextTrack( NextItem );

        if( State == guMEDIASTATE_PLAYING || State == guMEDIASTATE_ERROR )
        {
            m_IsSkipping = true;
            LoadMedia( ( event.GetInt() ? ( guFADERPLAYBIN_PLAYTYPE ) event.GetInt() :
                ( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE :
                    ( ForceSkip ? guFADERPLAYBIN_PLAYTYPE_REPLACE : guFADERPLAYBIN_PLAYTYPE_AFTER_EOS ) ) ) );
        }
        else
        {
            if( State == guMEDIASTATE_PAUSED )
            {
                m_MediaCtrl->Stop();
            }
            if( m_MediaSong.m_Loaded )
            {
                guCurrentTrack CurrentTrack;
                CurrentTrack.m_Rating = 0;
                CurrentTrack.m_Year = 0;
                CurrentTrack.m_Length = 0;
                m_MediaSong = CurrentTrack;
                m_LastCurPos = 0;
                m_LastLength = 0;
                UpdateLabels();
                UpdatePositionLabel( 0 );

                m_MediaSong.m_CoverType = GU_SONGCOVER_NONE;
                m_MediaSong.m_CoverPath = wxEmptyString;
                wxImage * CoverImage = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
                // Cover
                if( CoverImage )
                {
                    if( CoverImage->IsOk() )
                    {
                        m_PlayerCoverBitmap->SetBitmap( wxBitmap( *CoverImage ) );
                        m_PlayerCoverBitmap->Refresh();
                    }
                    delete CoverImage;
                }
            }
            //guLogMessage( wxT( "Next Track when not playing.." ) );
//            m_MediaCtrl->SetCurrentState( GST_STATE_READY );
        }

        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );

    }
    else
    {
        // If the option to play a random track is set
        if( m_PlayRandom )
        {
            // If Repeat was enabled disable it
            if( m_PlayLoop )
                SetPlayLoop( guPLAYER_PLAYLOOP_NONE );

            //guLogMessage( wxT( "Getting Random Tracks..." ) );
            guTrackArray Tracks;
            if( m_Db->GetRandomTracks( &Tracks, m_SmartPlayAddTracks, m_PlayRandomMode,
                    m_PlayerFilters->GetAllowFilterId(),
                    m_PlayerFilters->GetDenyFilterId() ) )
            {
                AddToPlayList( Tracks, false );

                OnNextTrackButtonClick( event );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnNextAlbumButtonClick( wxCommandEvent& event )
{
    //guLogMessage( wxT( "OnNextAlbumButtonClick Cur: %i    %li" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
    guMediaState State;

    if( m_IsSkipping )
        return;

    guTrack * NextAlbumTrack = m_PlayListCtrl->GetNextAlbum( m_PlayLoop, true );
    if( NextAlbumTrack )
    {
        State = m_MediaCtrl->GetState();

        SetNextTrack( NextAlbumTrack );

        if( State == guMEDIASTATE_PLAYING )
        {
            m_IsSkipping = true;
            LoadMedia( ( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE ) );
        }
        //else
        //{
            //guLogMessage( wxT( "Next ALbum Track when not playing.." ) );
//            m_MediaCtrl->SetCurrentState( GST_STATE_READY );
        //}

        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
    else
    {
        // If the option to play a random track is set
        if( m_PlayRandom )
        {
            // If Repeat was enabled disable it
            if( m_PlayLoop )
                SetPlayLoop( guPLAYER_PLAYLOOP_NONE );

            //guLogMessage( wxT( "Getting Random Tracks..." ) );
            guTrackArray Tracks;
            if( m_Db->GetRandomTracks( &Tracks, m_SmartPlayAddTracks, m_PlayRandomMode,
                    m_PlayerFilters->GetAllowFilterId(),
                    m_PlayerFilters->GetDenyFilterId() ) )
            {
                AddToPlayList( Tracks, false );

                OnNextAlbumButtonClick( event );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPrevAlbumButtonClick( wxCommandEvent& event )
{
    //guLogMessage( wxT( "OnPrevAlbumButtonClick Cur: %i    %li" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
    guMediaState State;

    if( m_IsSkipping )
        return;

    guTrack * NextAlbumTrack = m_PlayListCtrl->GetPrevAlbum( m_PlayLoop, true );
    if( NextAlbumTrack )
    {
        State = m_MediaCtrl->GetState();

        SetNextTrack( NextAlbumTrack );

        if( State == guMEDIASTATE_PLAYING )
        {
            m_IsSkipping = true;
            LoadMedia( ( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE ) );
        }
        //else
        //{
            //guLogMessage( wxT( "Prev Album Track when not playing.." ) );
//            m_MediaCtrl->SetCurrentState( GST_STATE_READY );
        //}

        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayButtonClick( wxCommandEvent& event )
{
    guLogMessage( wxT( "OnPlayButtonClick Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    guMediaState State;

    if( m_PendingNewRecordName )
        m_PendingNewRecordName = false;

    // Get The Current Song From m_PlayListCtrl
    //guTrack * CurItem = m_PlayListCtrl->GetCurrent();
    //if( !m_MediaSong.m_SongId && m_PlayListCtrl->GetItemCount() )
    if( !m_MediaSong.m_Loaded && m_PlayListCtrl->GetItemCount() )
    {
        //guLogMessage( wxT( "Going to load the track..." ) );
        if( m_PlayListCtrl->GetCurItem() == wxNOT_FOUND )
            m_PlayListCtrl->SetCurrent( 0, m_DelTracksPlayed && !m_PlayLoop );
        //m_MediaSong = * m_PlayListCtrl->GetCurrent();
        SetNextTrack( m_PlayListCtrl->GetCurrent() );

        LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );
        return;
    }

    if( m_MediaSong.m_Loaded )
    {
        //State = m_MediaCtrl->GetState();
        State = GetState();
        guLogMessage( wxT( ">>>> PlayButtonClick State: %i" ), State );
        if( State == guMEDIASTATE_PLAYING )
        {
            if( m_SilenceDetector )
                m_SilenceDetected = true;

            if( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
                m_MediaCtrl->Pause();
            else
                m_MediaCtrl->Stop();

            //ResetVumeterLevel();
        }
        else if( State == guMEDIASTATE_PAUSED )
        {
            m_MediaCtrl->Play();
        }
        else if( State == guMEDIASTATE_STOPPED )
        {
            m_SavedPlayedTrack = false;
            //guLogMessage( wxT( "Loading '%s'" ), m_NextSong.m_FileName.c_str() );
            LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );
            return;
        }
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
    else
    {
        // If the option to play a random track is set
        if( m_PlayRandom )
        {
            // If Repeat was enabled disable it
            if( m_PlayLoop )
                SetPlayLoop( guPLAYER_PLAYLOOP_NONE );

            //guLogMessage( wxT( "Getting Random Tracks..." ) );
            guTrackArray Tracks;
            if( m_Db->GetRandomTracks( &Tracks, m_SmartPlayAddTracks, m_PlayRandomMode,
                    m_PlayerFilters->GetAllowFilterId(),
                    m_PlayerFilters->GetDenyFilterId() ) )
            {
                AddToPlayList( Tracks, false );

                OnPlayButtonClick( event );
            }
        }
    }
    //wxLogMessage( wxT( "OnPlayButtonClick Id : %i" ), m_MediaSong.SongId );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnStopButtonClick( wxCommandEvent& event )
{
    //guLogMessage( wxT( "OnStopButtonClick Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    if( wxGetKeyState( WXK_SHIFT ) )
    {
        OnStopAfterCurrentTrack( event );
        return;
    }

    //guMediaState State;
    //State = m_MediaCtrl->GetState();
    //guLogMessage( wxT( "State: %i" ), State );
    //if( State != guMEDIASTATE_STOPPED )
    //{
    m_MediaCtrl->Stop();
        //UpdatePositionLabel( 0 );
//        if( m_MediaSong.m_Length )
//    m_PlayerPositionSlider->SetValue( 0 );
//    ResetVumeterLevel();
    //}
    SavePlayedTrack();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnStopAfterCurrentTrack( wxCommandEvent &event )
{
    //m_MediaSong.m_Type = guTrackType( int( m_MediaSong.m_Type ) ^ guTRACK_TYPE_STOP_HERE );
    m_PlayListCtrl->StopAfterCurrent();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRecordButtonClick( wxCommandEvent& event )
{
    bool IsEnabled = event.IsChecked();
    if( IsEnabled )
    {
        m_MediaRecordCtrl->Start( &m_MediaSong );
    }
    else
    {
        m_MediaRecordCtrl->Stop();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnVolumenButtonClick( wxCommandEvent& event )
{
    //wxMessageBox( wxT("OnVolumenButtonClick"), wxT("Event") );
    wxPoint Pos;
    //Pos = VolumenButton->GetPosition();
    Pos = ClientToScreen( m_VolumeButton->GetPosition() );
    Pos.x += 1;
    guVolumeFrame * VolFrame = new guVolumeFrame( this, this, wxID_ANY, wxEmptyString, Pos );
    //
    if( VolFrame )
    {
        VolFrame->Show();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnSmartPlayButtonClick( wxCommandEvent &event )
{
    SetPlaySmart( !GetPlaySmart() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRandomPlayButtonClick( wxCommandEvent &event )
{
    m_PlayListCtrl->Randomize( ( GetState() == guMEDIASTATE_PLAYING ) );
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, evt );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRepeatPlayButtonClick( wxCommandEvent &event )
{
    m_PlayLoop = m_PlayLoop++;
    if( m_PlayLoop > guPLAYER_PLAYLOOP_TRACK )
        m_PlayLoop = guPLAYER_PLAYLOOP_NONE;
    SetPlayLoop( m_PlayLoop );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnLoveBanButtonClick( wxCommandEvent &event )
{
    if( m_MediaSong.m_Loaded )
    {
        if( m_MediaSong.m_ASRating == guAS_RATING_NONE )
            m_MediaSong.m_ASRating = guAS_RATING_LOVE;
        else if( m_MediaSong.m_ASRating == guAS_RATING_LOVE )
            m_MediaSong.m_ASRating = guAS_RATING_BAN;
        else
            m_MediaSong.m_ASRating = guAS_RATING_NONE;

        m_LoveBanButton->SetBitmapLabel( m_MediaSong.m_ASRating < guAS_RATING_BAN ?
            guImage( guIMAGE_INDEX_player_normal_love ) :
            guImage( guIMAGE_INDEX_player_normal_ban ) );

        m_LoveBanButton->SetBitmapDisabled( m_MediaSong.m_ASRating < guAS_RATING_BAN ?
            guImage( guIMAGE_INDEX_player_light_love ) :
            guImage( guIMAGE_INDEX_player_light_ban ) );

        m_LoveBanButton->SetBitmapHover( m_MediaSong.m_ASRating < guAS_RATING_BAN ?
            guImage( guIMAGE_INDEX_player_highlight_love ) :
            guImage( guIMAGE_INDEX_player_highlight_ban ) );

        m_LoveBanButton->SetValue( m_MediaSong.m_ASRating );
    }
    else
        m_LoveBanButton->SetValue( false );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnEqualizerButtonClicked( wxCommandEvent &event )
{
    guEq10Band * Eq10Band = new guEq10Band( this, m_MediaCtrl );
    if( Eq10Band )
    {
        if( Eq10Band->ShowModal() == wxID_OK )
        {
        }
        Eq10Band->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerCoverBitmapMouseOver( wxCommandEvent &event )
{
    if( m_MediaSong.m_CoverType == GU_SONGCOVER_NONE ||
        m_MediaSong.m_CoverType == GU_SONGCOVER_RADIO ||
        m_MediaSong.m_CoverType == GU_SONGCOVER_PODCAST )
        return;

    wxPoint Pos;
    Pos = ClientToScreen( m_PlayerCoverBitmap->GetPosition() );
    guCoverFrame * BigCover = new guCoverFrame( this, wxID_ANY, wxEmptyString, Pos );
    if( BigCover )
    {
//        if( m_MediaSong.m_CoverType == GU_SONGCOVER_ID3TAG )
//            BigCover->SetBitmap( m_MediaSong.m_CoverType, m_MediaSong.m_FileName );
//        else
        BigCover->SetBitmap( m_MediaSong.m_CoverType, m_MediaSong.m_CoverPath );
        BigCover->Show();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderBeginSeek( wxScrollEvent &event )
{
    m_SliderIsDragged = true;
    //
    if( m_LastLength )
    {
        int CurPos = event.GetPosition() * ( m_LastLength / 1000 );

//        m_PositionLabel->SetLabel( LenToString( CurPos / 1000 ) + _( " of " ) + LenToString( m_MediaSong.m_Length ) );
//        m_PosLabelSizer->Layout();
        UpdatePositionLabel( CurPos / 1000 );
        //printf( "Slider Tracking %d\n", CurPos );

        //m_MediaCtrl->Seek( CurPos * m_MediaSong.Length );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderEndSeek( wxScrollEvent &event )
{
    wxFileOffset NewPos;
    if( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
    {
        NewPos = event.GetPosition();
//        guLogMessage( wxT( "Slider EndSeek Set Pos to %i Of %i" ), ( int ) NewPos, 1000 );
//        //printf( "SetPos: %llu\n", ( long long int ) NewPos * m_MediaSong.Length );
//        //m_MediaCtrl->Seek( NewPos * m_MediaSong.m_Length );
//        SetPosition( NewPos * m_MediaSong.m_Length );
    }
    m_SliderIsDragged = false;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderChanged( wxScrollEvent &event )
{
    wxFileOffset NewPos;
    if( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
    {
        NewPos = event.GetPosition();
        //guLogMessage( wxT( "Slider Changed Set Pos to %i Of %i" ), ( int ) NewPos, 1000 );
        SetPosition( NewPos * ( m_LastLength / 1000 ) );
    }
    m_SliderIsDragged = false;

//    m_MediaCtrl->UnsetCrossfader();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderMouseWheel( wxMouseEvent &event )
{
    if( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
    {
        int Rotation = event.GetWheelRotation() / event.GetWheelDelta();
        //guLogMessage( wxT( "Pos : %i -> %i" ), GetPosition(), GetPosition() + ( Rotation * 7000 ) );
        SetPosition( wxMax( 0, wxMin( ( int ) m_LastLength, GetPosition() + ( Rotation * 7000 ) ) ) );
    }
    m_SliderIsDragged = false;

//    m_MediaCtrl->UnsetCrossfader();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetVolume( double volume )
{
    if( volume == m_CurVolume )
        return;

    if( volume < 0 )
        volume = 0;
    else if( volume > 100 )
        volume = 100;

    m_CurVolume = volume;

    if( m_CurVolume > 75 )
    {
        m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_vol_hi ) );
        m_VolumeButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_vol_hi ) );
    }
    else if( m_CurVolume > 50 )
    {
        m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_vol_mid ) );
        m_VolumeButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_vol_mid ) );
    }
    else if( m_CurVolume == 0 )
    {
        m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_muted ) );
        m_VolumeButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_muted ) );
    }
    else
    {
        m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_vol_low ) );
        m_VolumeButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_vol_low ) );
    }
    m_VolumeButton->Refresh();
    m_LastVolume = m_CurVolume;

    m_MediaCtrl->SetVolume(  volume / ( double ) 100.0 );
    m_VolumeButton->SetToolTip( _( "Volume" ) + wxString::Format( wxT( " %u%%" ), ( int ) volume ) );

    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_VOLUMECHANGED );
    wxPostEvent( m_MainFrame, evt );
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::SetPosition( int pos )
{
    return m_MediaCtrl->Seek( pos );
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetPosition()
{
    return m_LastCurPos; //m_MediaCtrl->Tell();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnTitleNameDClicked( wxMouseEvent &event )
{
    int TrackId = wxNOT_FOUND;
    if( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_JAMENDO ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_MAGNATUNE ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        TrackId = m_MediaSong.m_SongId;
    }
    else
    {
        guDbLibrary * Db = m_MediaSong.m_LibPanel ? m_MediaSong.m_LibPanel->GetDb() : m_Db;
        TrackId = Db->FindTrack( m_MediaSong.m_ArtistName, m_MediaSong.m_SongName );
    }

    if( TrackId != wxNOT_FOUND )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_TRACK );
        evt.SetInt( TrackId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        evt.SetClientData( ( void * ) m_MediaSong.m_LibPanel );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnAlbumNameDClicked( wxMouseEvent &event )
{
    int AlbumId = wxNOT_FOUND;
    if( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_JAMENDO ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_MAGNATUNE ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        AlbumId = m_MediaSong.m_AlbumId;
    }
    else
    {
        guDbLibrary * Db = m_MediaSong.m_LibPanel ? m_MediaSong.m_LibPanel->GetDb() : m_Db;
        AlbumId = Db->FindAlbum( m_MediaSong.m_ArtistName, m_MediaSong.m_AlbumName );
    }

    if( AlbumId != wxNOT_FOUND )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ALBUM );
        evt.SetInt( AlbumId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        evt.SetClientData( ( void * ) m_MediaSong.m_LibPanel );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnArtistNameDClicked( wxMouseEvent &event )
{
    int ArtistId = wxNOT_FOUND;
    if( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_JAMENDO ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_MAGNATUNE ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        ArtistId = m_MediaSong.m_ArtistId;
    }
    else
    {
        guDbLibrary * Db = m_MediaSong.m_LibPanel ? m_MediaSong.m_LibPanel->GetDb() : m_Db;
        ArtistId = Db->FindArtist( m_MediaSong.m_ArtistName );
    }

    if( ArtistId != wxNOT_FOUND )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( ArtistId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        evt.SetClientData( ( void * ) m_MediaSong.m_LibPanel );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnYearDClicked( wxMouseEvent &event )
{
    int Year = m_MediaSong.m_Year;

    if( Year )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_YEAR );
        evt.SetInt( Year );
        evt.SetExtraLong( m_MediaSong.m_Type );
        evt.SetClientData( ( void * ) m_MediaSong.m_LibPanel );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRatingChanged( guRatingEvent &event )
{
    m_MediaSong.m_Rating = event.GetInt();
    if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
    {
        guDbLibrary * Db = m_MediaSong.m_LibPanel ? m_MediaSong.m_LibPanel->GetDb() : m_Db;
        Db->SetTrackRating( m_MediaSong.m_SongId, m_MediaSong.m_Rating );

        // Update the track in database, playlist, etc
        m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
    }
    else
    {
        m_MediaSong.m_Rating = -1;
        m_Rating->SetRating( -1 );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdatedTracks( const guTrackArray * tracks )
{
    wxASSERT( tracks );
    int index;
    int count = tracks->Count();
    for( index = 0; index < count; index++ )
    {
        if( tracks->Item( index ).m_FileName == m_MediaSong.m_FileName )
        {
            //m_MediaSong = tracks->Item( index );
            m_MediaSong.Update( tracks->Item( index ) );
            // Update the Current Playing Song Info
            UpdateLabels();
            UpdateCover( false );
            break;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdatedTrack( const guTrack * track )
{
    wxASSERT( track );
    if( track->m_FileName == m_MediaSong.m_FileName )
    {
        //m_MediaSong = * track;
        m_MediaSong.Update( * track );
        // Update the Current Playing Song Info
        UpdateLabels();
        UpdateCover();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdateLabels( void )
{
    // Update the Current Playing Song Info
    SetTitleLabel( m_MediaSong.m_SongName );
    SetAlbumLabel( m_MediaSong.m_AlbumName );
    SetArtistLabel( m_MediaSong.m_ArtistName );
    SetRatingLabel( m_MediaSong.m_Rating );
//    m_LoveBanButton->SetValue( m_MediaSong.m_ASRating );

    if( m_MediaSong.m_Year > 0 )
    {
        m_YearLabel->SetLabel( wxString::Format( wxT( "%u" ), m_MediaSong.m_Year ) );
    }
    else
        m_YearLabel->SetLabel( wxEmptyString );

    SetBitRateLabel( m_MediaSong.m_Bitrate );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::ResetVumeterLevel( void )
{
    guLevelInfo LevelInfo;
    LevelInfo.m_Decay_L = -INFINITY;
    LevelInfo.m_Decay_R = -INFINITY;
    LevelInfo.m_Peak_L  = -INFINITY;
    LevelInfo.m_Peak_R  = -INFINITY;
    if( m_PlayerVumeters )
    {
        m_PlayerVumeters->SetLevels( LevelInfo );
    }
}

// -------------------------------------------------------------------------------- //
void Notify_Normalize_String( wxString &text )
{
    text.Replace( wxT( "&" ), wxT( "&amp;" ) );
    text.Replace( wxT( "<" ), wxT( "&lt;" ) );
    text.Replace( wxT( ">" ), wxT( "&gt;" ) );
    text.Replace( wxT( "'" ), wxT( "&apos;" ) );
    text.Replace( wxT( "\"" ), wxT( "&quot;" ) );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SendNotifyInfo( wxImage * image )
{
    if( m_ShowNotifications && m_NotifySrv )
    {
        image->Rescale( 60, 60, wxIMAGE_QUALITY_HIGH );

        wxString Body;
        if( m_MediaSong.m_Length )
        {
            Body = LenToString( m_MediaSong.m_Length );
        }
        if( m_MediaSong.m_Rating > 0 )
        {
            Body += wxT( "  " );
            Body.Append( wxT( '★' ), m_MediaSong.m_Rating );
        }
        Body +=  wxT( "\n" ) + m_MediaSong.m_ArtistName;
        if( !m_MediaSong.m_AlbumName.IsEmpty() )
        {
            Body += wxT( " / " ) + m_MediaSong.m_AlbumName;
        }
        Notify_Normalize_String( Body );

        m_NotifySrv->Notify( wxEmptyString, m_MediaSong.m_SongName, Body, image );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnVolumenMouseWheel( wxMouseEvent &event )
{
    int Rotation = ( event.GetWheelRotation() / event.GetWheelDelta() ) * ( event.ShiftDown() ? 1 : 4 );
    //guLogMessage( wxT( "CurVol: %u  Rotations:%i" ), m_CurVolume, Rotation );
    SetVolume( m_CurVolume + Rotation );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnAddTracks( wxCommandEvent &event )
{
    //bool PlayTracks = event.GetInt();
    wxArrayString * TrackList = ( wxArrayString * ) event.GetClientData();

    if( TrackList )
    {
        SetPlayList( * TrackList );
        delete TrackList;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRemoveTrack( wxCommandEvent &event )
{
    RemoveItem( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRepeat( wxCommandEvent &event )
{
    OnRepeatPlayButtonClick( event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnLoop( wxCommandEvent &event )
{
    SetPlayLoop( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRandom( wxCommandEvent &event )
{
    OnRandomPlayButtonClick( event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnSetVolume( wxCommandEvent &event )
{
    SetVolume( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
// guSmartAddTracksThread
// -------------------------------------------------------------------------------- //
guSmartAddTracksThread::guSmartAddTracksThread( guDbLibrary * db,
        guPlayerPanel * playerpanel, const guTrack * track,
        wxArrayInt * smartaddedtracks, wxArrayString * smartaddedartists,
        const int maxtracks, const int maxartists,
        const int trackcount, const int filterallow, const int filterdeny ) : wxThread()
{
    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_CurSong = track;
    m_SmartAddedTracks = smartaddedtracks;
    m_SmartAddedArtists = smartaddedartists;
    m_SmartMaxTracksList = maxtracks;
    m_SmartMaxArtistsList = maxartists;
    m_TrackCount = trackcount;
    m_FilterAllowPlayList = filterallow;
    m_FilterDenyPlayList = filterdeny;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guSmartAddTracksThread::~guSmartAddTracksThread()
{
//    printf( "guSmartAddTracksThread Object destroyed\n" );
  if( !TestDestroy() )
    m_PlayerPanel->m_SmartAddTracksThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guSmartAddTracksThread::AddSimilarTracks( const wxString &artist, const wxString &track, guTrackArray * songs )
{
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        guSimilarTrackInfoArray SimilarTracks = LastFM->TrackGetSimilar( artist, track );
        if( SimilarTracks.Count() )
        {
            int Index;
            int Count = SimilarTracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
              //guLogMessage( wxT( "Similar: '%s' - '%s'" ), SimilarTracks[ index ].ArtistName.c_str(), SimilarTracks[ index ].TrackName.c_str() );
              guTrack * Song = m_Db->FindSong( SimilarTracks[ Index ].m_ArtistName,
                                               SimilarTracks[ Index ].m_TrackName,
                                               m_FilterAllowPlayList,
                                               m_FilterDenyPlayList );
              if( Song &&
                  ( m_SmartAddedTracks->Index( Song->m_SongId ) == wxNOT_FOUND ) &&
                  ( m_SmartAddedArtists->Index( Song->m_ArtistName.Upper() ) == wxNOT_FOUND ) )
              {
                  Song->m_TrackMode = guTRACK_MODE_SMART;
                  //guLogMessage( wxT( "Found this song in the Songs Library" ) );
                  songs->Add( Song );
              }
            }
        }
        delete LastFM;
    }
}

// -------------------------------------------------------------------------------- //
guSmartAddTracksThread::ExitCode guSmartAddTracksThread::Entry()
{
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        guTrackArray FoundTracks;
        guTrackArray * Songs = new guTrackArray();
        //guTrack * Song;
        int Index;
        int Count;
        if( !TestDestroy() && Songs )
        {
            guTrackArray FoundTracks;
            guTrack      AddedTrack;

            AddSimilarTracks( m_CurSong->m_ArtistName, m_CurSong->m_SongName, &FoundTracks );

            int CurAddedTrack = m_SmartAddedTracks->Count();
            if( CurAddedTrack )
            {
                CurAddedTrack--;
                while( FoundTracks.Count() < 25 && CurAddedTrack )
                {
                    if( m_Db->GetSong( ( * m_SmartAddedTracks )[ CurAddedTrack ], &AddedTrack ) )
                    {
                        AddSimilarTracks( AddedTrack.m_ArtistName, AddedTrack.m_SongName, &FoundTracks );
                    }
                    CurAddedTrack--;
                    Sleep( 20 );
                }
            }

            // Aleatorize tracks
            Count = FoundTracks.Count();
            if( Count )
            {
                for( Index = 0; Index < m_TrackCount; Index++ )
                {
                    if( !Count )
                        break;

                    if( Count > 1 )
                    {
                        do {
                            int Selected = guRandom( Count );
                            //guLogMessage( wxT( "%i (%i) %s" ), Selected, Count, FoundTracks[ Selected ].m_SongName.c_str() );
                            if( m_SmartAddedArtists->Index( FoundTracks[ Selected ].m_ArtistName.Upper() ) != wxNOT_FOUND )
                            {
                                continue;
                            }
                            Songs->Add( new guTrack( FoundTracks[ Selected ] ) );
                            m_SmartAddedTracks->Add( FoundTracks[ Selected ].m_SongId );
                            m_SmartAddedArtists->Add( FoundTracks[ Selected ].m_ArtistName.Upper() );
                            FoundTracks.RemoveAt( Selected );
                            Count--;
                        } while( false );
                    }
                    else
                    {
                        if( m_SmartAddedArtists->Index( FoundTracks[ 0 ].m_ArtistName.Upper() ) == wxNOT_FOUND )
                        {
                            //guLogMessage( wxT( "%i (%i) %s" ), 0, Count, FoundTracks[ 0 ].m_SongName.c_str() );
                            Songs->Add( new guTrack( FoundTracks[ 0 ] ) );
                            m_SmartAddedTracks->Add( FoundTracks[ 0 ].m_SongId );
                            m_SmartAddedArtists->Add( FoundTracks[ 0 ].m_ArtistName.Upper() );
                        }
                        break;
                    }
                }
            }
        }

        if( !TestDestroy() && Songs && ( ( int ) Songs->Count() < m_TrackCount ) )
        {
            guSimilarArtistInfoArray SimilarArtists = LastFM->ArtistGetSimilar( m_CurSong->m_ArtistName );
            if( SimilarArtists.Count() && !TestDestroy() )
            {
                int ArtistId;
                Count = SimilarArtists.Count();
                wxArrayInt Artists;
                guTrackArray ArtistsTracks;
                for( Index = 0; Index < Count; Index++ )
                {
                    if( TestDestroy() )
                        break;

                    ArtistId = m_Db->GetArtistId( SimilarArtists[ Index ].m_Name, false );
                    if( ArtistId != wxNOT_FOUND )
                    {
                        Artists.Add( ArtistId );
                    }
                }

                m_Db->GetArtistsSongs( Artists, &ArtistsTracks, 25, m_FilterAllowPlayList, m_FilterDenyPlayList );

                Count = ArtistsTracks.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    if( ( m_SmartAddedTracks->Index( ArtistsTracks[ Index ].m_SongId ) == wxNOT_FOUND ) &&
                        ( m_SmartAddedArtists->Index( ArtistsTracks[ Index ].m_ArtistName.Upper() ) == wxNOT_FOUND ) )
                    {
                        Songs->Add( new guTrack( ArtistsTracks[ Index ] ) );
                        m_SmartAddedTracks->Add( ArtistsTracks[ Index ].m_SongId );
                        m_SmartAddedArtists->Add( ArtistsTracks[ Index ].m_ArtistName.Upper() );
                        if( ( int ) Songs->Count() == m_TrackCount )
                            break;
                    }
                }
            }
        }

        if( ( int ) m_SmartAddedTracks->Count() > m_SmartMaxTracksList )
            m_SmartAddedTracks->RemoveAt( 0, m_SmartAddedTracks->Count() - m_SmartMaxTracksList );

        if( ( int ) m_SmartAddedArtists->Count() > m_SmartMaxArtistsList )
            m_SmartAddedArtists->RemoveAt( 0, m_SmartAddedArtists->Count() - m_SmartMaxArtistsList );

        //guLogMessage( wxT( "========" ) );
        //for( Index = 0; Index < m_SmartAddedArtists->Count(); Index++ )
        //    guLogMessage( wxT( "Artist: '%s'" ), ( * m_SmartAddedArtists )[ Index ].c_str() );

        if( !TestDestroy() )
        {
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_SMART_ADDTRACK );
            //event.SetEventObject( ( wxObject * ) this );
            event.SetClientData( ( void * ) Songs );
            m_PlayerPanel->AddPendingEvent( event );
        }
        else
        {
            if( Songs )
            {
                delete Songs;
            }
        }
        delete LastFM;
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
// guUpdatePlayerCoverThread
// -------------------------------------------------------------------------------- //
guUpdatePlayerCoverThread::guUpdatePlayerCoverThread( guDbLibrary * db,
    guMainFrame * mainframe, guPlayerPanel * playerpanel, guCurrentTrack * currenttrack, const bool shownotify ) : wxThread()
{
    m_Db = db;
    m_MainFrame = mainframe;
    m_PlayerPanel = playerpanel;
    m_CurrentTrack = currenttrack;
    m_ShowNotify = shownotify;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guUpdatePlayerCoverThread::~guUpdatePlayerCoverThread()
{
}

// -------------------------------------------------------------------------------- //
guUpdatePlayerCoverThread::ExitCode guUpdatePlayerCoverThread::Entry()
{
    if( TestDestroy() )
        return 0;

    wxImage * CoverImage = NULL;

    if( m_CurrentTrack->m_Type == guTRACK_TYPE_RADIOSTATION )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_net_radio ) );
        m_CurrentTrack->m_CoverType = GU_SONGCOVER_RADIO;
    }
    else if( m_CurrentTrack->m_Type == guTRACK_TYPE_PODCAST )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_podcast ) );
        m_CurrentTrack->m_CoverType = GU_SONGCOVER_PODCAST;
    }
    else if( m_CurrentTrack->m_Type == guTRACK_TYPE_JAMENDO )
    {
        guJamendoPanel * JamendoPanel = m_MainFrame->GetJamendoPanel();
        if( JamendoPanel )
        {
            //guLogMessage( wxT( "Tried to get the CoverImage for the jamendo album %i" ), m_CurrentTrack->m_AlbumId );
            CoverImage = JamendoPanel->GetAlbumCover( m_CurrentTrack->m_AlbumId, m_CurrentTrack->m_CoverPath );
            m_CurrentTrack->m_CoverType = GU_SONGCOVER_FILE;
        }
    }
    else if( m_CurrentTrack->m_Type == guTRACK_TYPE_MAGNATUNE )
    {
        guMagnatunePanel * MagnatunePanel = m_MainFrame->GetMagnatunePanel();
        if( MagnatunePanel )
        {
            //guLogMessage( wxT( "Tried to get the CoverImage for the magnatune album %i" ), m_CurrentTrack->m_AlbumId );
            CoverImage = MagnatunePanel->GetAlbumCover( m_CurrentTrack->m_AlbumId, m_CurrentTrack->m_ArtistName,
                                                        m_CurrentTrack->m_AlbumName, m_CurrentTrack->m_CoverPath );
            m_CurrentTrack->m_CoverType = GU_SONGCOVER_FILE;
        }
    }
    else if( ( CoverImage = guTagGetPicture( m_CurrentTrack->m_FileName ) ) )
    {
        m_CurrentTrack->m_CoverType = GU_SONGCOVER_ID3TAG;
        m_CurrentTrack->m_CoverPath = m_CurrentTrack->m_FileName;
    }
    else if( m_CurrentTrack->m_Type == guTRACK_TYPE_IPOD )
    {
    }
    else if( m_CurrentTrack->m_CoverId )
    {
        //guLogMessage( wxT( "CoverId %i" ), m_CurrentTrack->m_CoverId );
        guDbLibrary * Db = m_CurrentTrack->m_LibPanel ? m_CurrentTrack->m_LibPanel->GetDb() : m_Db;
        m_CurrentTrack->m_CoverPath = Db->GetCoverPath( m_CurrentTrack->m_CoverId );
        m_CurrentTrack->m_CoverType = GU_SONGCOVER_FILE;
    }
    else
    {
        //guLogWarning( wxT( "Trying to find covers in %s" ), wxPathOnly( m_CurrentTrack->m_FileName ).c_str() );
        m_CurrentTrack->m_CoverPath = m_PlayerPanel->PlayListCtrl()->FindCoverFile( wxPathOnly( m_CurrentTrack->m_FileName ) );
    }

    if( TestDestroy() )
        return 0;


    if( !CoverImage )
    {
        if( m_CurrentTrack->m_CoverPath.IsEmpty() || !wxFileExists( m_CurrentTrack->m_CoverPath ) )
        {
            //printf( "No coverpath set\n" );
            CoverImage = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
            m_CurrentTrack->m_CoverType = GU_SONGCOVER_NONE;
            m_CurrentTrack->m_CoverPath = wxEmptyString;
        }
        else
        {
            CoverImage = new wxImage( m_CurrentTrack->m_CoverPath );
            m_CurrentTrack->m_CoverType = GU_SONGCOVER_FILE;
            //m_CurrentTrack->CoverPath = CoverPath;
        }
    }

    if( TestDestroy() )
        return 0;

    CoverImage->Rescale( 100, 100, wxIMAGE_QUALITY_HIGH );

//    guLogMessage( wxT( "   File : %s" ), m_CurrentTrack->m_FileName.c_str() );
//    guLogMessage( wxT( " Loaded : %i" ), m_CurrentTrack->m_Loaded );
//    guLogMessage( wxT( "   Type : %i" ), m_CurrentTrack->m_Type );
//    guLogMessage( wxT( " SongId : %i" ), m_CurrentTrack->m_SongId );
//    guLogMessage( wxT( "CoverId : %i" ), m_CurrentTrack->m_CoverId );
//    guLogMessage( wxT( "Co.Type : %i" ), m_CurrentTrack->m_CoverType );
//    guLogMessage( wxT( "  Cover : '%s'" ), m_CurrentTrack->m_CoverPath.c_str() );
//    guLogMessage( wxT( "  Width : %u" ), CoverImage->GetWidth() );
//    guLogMessage( wxT( " Height : %u" ), CoverImage->GetHeight() );
//    guLogMessage( wxT( "===========================================" ) );

    if( ( ( m_CurrentTrack->m_CoverType != GU_SONGCOVER_NONE ) && m_CurrentTrack->m_CoverPath.IsEmpty() ) ||
        ( m_CurrentTrack->m_CoverType == GU_SONGCOVER_ID3TAG ) )
    {
        if( CoverImage )
        {
            wxString LastTmpCoverFile = m_PlayerPanel->LastTmpCoverFile();
            if( !LastTmpCoverFile.IsEmpty() )
                wxRemoveFile( LastTmpCoverFile );

            if( LastTmpCoverFile.EndsWith( wxT( "1.png" ) ) )
                LastTmpCoverFile = wxFileName::GetTempDir() + wxT( "/" ) + guTEMPORARY_COVER_FILENAME + wxT( "2.png");
            else
                LastTmpCoverFile = wxFileName::GetTempDir() + wxT( "/" ) + guTEMPORARY_COVER_FILENAME + wxT( "1.png");

            //guLogMessage( wxT( "Saving temp cover file to '%s'" ), LastTmpCoverFile.c_str() );
            if( CoverImage->SaveFile( LastTmpCoverFile, wxBITMAP_TYPE_PNG ) )
            {
                if( m_CurrentTrack->m_CoverPath.IsEmpty() )
                    m_CurrentTrack->m_CoverPath = LastTmpCoverFile;

                m_PlayerPanel->SetLastTmpCoverFile( LastTmpCoverFile );
            }
        }
    }

    if( TestDestroy() )
        return 0;

    if( m_CurrentTrack->m_CoverImage )
        delete m_CurrentTrack->m_CoverImage;
    m_CurrentTrack->m_CoverImage = CoverImage;

    wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_COVERUPDATED );
    Event.SetInt( m_ShowNotify );
    wxPostEvent( m_PlayerPanel, Event );

    return 0;
}

// -------------------------------------------------------------------------------- //
