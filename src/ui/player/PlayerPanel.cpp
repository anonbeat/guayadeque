// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
#include "PlayerPanel.h"

#include "EventCommandIds.h"
#include "CoverFrame.h"
#include "CoverPanel.h"
#include "Config.h"
#include "DbLibrary.h"
#include "Equalizer.h"
#include "FileRenamer.h" // NormalizeField
#include "Images.h"
#include "LevelInfo.h"
#include "MainFrame.h"
#include "RadioTagInfo.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Utils.h"

#include <wx/gdicmn.h>
#include <wx/regex.h>
#include <wx/utils.h>

namespace Guayadeque {

//#define guLogDebug(...)  guLogMessage(__VA_ARGS__)
#define guLogDebug(...)


#define GUPLAYER_MIN_PREVTRACK_POS      5000

#define guPLAYER_SMART_CACHEITEMS       100
#define guPLAYER_SMART_CACHEARTISTS     20

#define guPLAYER_ICONS_SEPARATOR        2
#define guPLAYER_ICONS_GROUPSEPARATOR   4

#define guPLAYER_FONTSIZE_TRACKNAME     12
#define guPLAYER_FONTSIZE_ALBUMNAME     11
#define guPLAYER_FONTSIZE_ARTISTNAME    11

wxArrayInt SupportedPlayCountTypes;

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
    //m_PendingNewRecordName = false;
    m_MediaSong.m_Type = guTRACK_TYPE_NOTDB;
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
    m_ShowNotifications = true;
    m_ShowNotificationsTime = 0;
    m_ErrorFound = false;
    m_SavedPlayedTrack = false;

    m_SilenceDetected = false;
    m_AboutToEndDetected = false;

    // Load configuration
    Config = ( guConfig * ) guConfig::Get();

    Config->RegisterObject( this );

    //guLogDebug( wxT( "Reading PlayerPanel Config" ) );
    SavedVol = Config->ReadNum( CONFIG_KEY_GENERAL_PLAYER_VOLUME, 50, CONFIG_PATH_GENERAL );
    //guLogDebug( wxT( "Current Volume Var : %d" ), ( int ) m_CurVolume );
    //m_PlayLoop = Config->ReadNum( CONFIG_KEY_GENERAL_PLAYER_LOOP, 0, CONFIG_PATH_GENERAL  );
    //m_PlaySmart = Config->ReadBool( CONFIG_KEY_GENERAL_PLAYER_SMART, m_PlayLoop ? false : true, CONFIG_PATH_GENERAL  );
    m_PlayMode = Config->ReadNum( CONFIG_KEY_GENERAL_PLAYER_PLAYMODE, guPLAYER_PLAYMODE_SMART, CONFIG_PATH_GENERAL );
    m_PlayRandom = Config->ReadBool( CONFIG_KEY_GENERAL_RANDOM_PLAY_ON_EMPTY_PLAYLIST, false, CONFIG_PATH_GENERAL );
    m_PlayRandomMode = Config->ReadNum( CONFIG_KEY_GENERAL_RANDOM_MODE_ON_EMPTY_PLAYLIST, guRANDOM_MODE_TRACK, CONFIG_PATH_GENERAL );
    m_ShowNotifications = Config->ReadBool( CONFIG_KEY_GENERAL_SHOW_NOTIFICATIONS, true, CONFIG_PATH_GENERAL );
    m_ShowNotificationsTime = Config->ReadNum( CONFIG_KEY_GENERAL_NOTIFICATION_TIME, 0, CONFIG_PATH_GENERAL );

    m_SmartPlayAddTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_NUM_TRACKS_TO_ADD, 3, CONFIG_PATH_PLAYBACK );
    m_SmartPlayMinTracksToPlay = Config->ReadNum( CONFIG_KEY_PLAYBACK_MIN_TRACKS_PLAY, 4, CONFIG_PATH_PLAYBACK );
    m_DelTracksPlayed = Config->ReadBool( CONFIG_KEY_PLAYBACK_DEL_TRACKS_PLAYED, false, CONFIG_PATH_PLAYBACK );
    m_SmartMaxArtistsList = Config->ReadNum( CONFIG_KEY_PLAYBACK_SMART_FILTER_ARTISTS, 20, CONFIG_PATH_PLAYBACK );
    m_SmartMaxTracksList = Config->ReadNum( CONFIG_KEY_PLAYBACK_SMART_FILTER_TRACKS, 100, CONFIG_PATH_PLAYBACK );

    m_AudioScrobbleEnabled = Config->ReadBool( CONFIG_KEY_LASTFM_ENABLED, false, CONFIG_PATH_LASTFM ) ||
                             Config->ReadBool( CONFIG_KEY_LIBREFM_ENABLED, false, CONFIG_PATH_LIBREFM );
    Equalizer = Config->ReadANum( CONFIG_KEY_EQUALIZER_BAND, 0, CONFIG_PATH_EQUALIZER );
    if( Equalizer.Count() != guEQUALIZER_BAND_COUNT )
    {
        Equalizer.Empty();
        Equalizer.Add( 0, guEQUALIZER_BAND_COUNT );
    }

    m_SilenceDetector = Config->ReadBool( CONFIG_KEY_PLAYBCK_SILENCE_DETECTOR, false, CONFIG_PATH_PLAYBACK );
    m_SilenceDetectorLevel = Config->ReadNum( CONFIG_KEY_PLAYBCK_SILENCE_LEVEL, -55, CONFIG_PATH_PLAYBACK );
    if( Config->ReadBool( CONFIG_KEY_PLAYBCK_SILENCE_AT_END, false, CONFIG_PATH_PLAYBACK ) )
    {
        m_SilenceDetectorTime = Config->ReadNum( CONFIG_KEY_PLAYBCK_SILENCE_END_TIME, 45, CONFIG_PATH_PLAYBACK ) * 1000;
    }

    m_ShowRevTime = Config->ReadBool( CONFIG_KEY_GENERAL_SHOW_REV_TIME, false, CONFIG_PATH_GENERAL );

    m_ForceGapless = Config->ReadBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, false, CONFIG_PATH_CROSSFADER );
    m_FadeOutTime = Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEOUT_TIME, 50, CONFIG_PATH_CROSSFADER ) * 100;

    m_SliderIsDragged = false;
    m_SmartSearchEnabled = false;
    m_SmartAddTracksThread = NULL;
//    m_UpdateCoverThread = NULL;

    // ---------------------------------------------------------------------------- //
    // The player controls
    // ---------------------------------------------------------------------------- //
//    wxPanel * PlayerPanel;
//	PlayerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxBoxSizer * PlayerMainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * PlayerBtnSizer = new wxBoxSizer( wxHORIZONTAL );

	//m_PrevTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_prev ), wxDefaultPosition, wxDefaultSize, 0 ); //wxBU_AUTODRAW );
    m_PrevTrackButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_prev ), guImage( guIMAGE_INDEX_player_highlight_prev ) );
	m_PrevTrackButton->SetToolTip( _( "Go to the playlist previous track" ) );
	PlayerBtnSizer->Add( m_PrevTrackButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_PlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_play ), wxDefaultPosition, wxDefaultSize, 0 );
    m_PlayButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_play ), guImage( guIMAGE_INDEX_player_highlight_play ) );
	m_PlayButton->SetToolTip( _( "Play or pause the current track" ) );
	PlayerBtnSizer->Add( m_PlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_NextTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_next ), wxDefaultPosition, wxDefaultSize, 0 );
    m_NextTrackButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_next ), guImage( guIMAGE_INDEX_player_highlight_next ) );
	m_NextTrackButton->SetToolTip( _( "Go to the playlist next track" ) );
	PlayerBtnSizer->Add( m_NextTrackButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

    m_StopButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_stop ), guImage( guIMAGE_INDEX_player_highlight_stop ) );
    m_StopButton->SetToolTip( _( "Stops the current track" ) );
    PlayerBtnSizer->Add( m_StopButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

    m_RecordButton = new guToggleRoundButton( this, guImage( guIMAGE_INDEX_player_light_record ), guImage( guIMAGE_INDEX_player_normal_record ), guImage( guIMAGE_INDEX_player_highlight_record ) );
    m_RecordButton->SetToolTip( _( "Record to file" ) );
    m_RecordButton->Enable( false );
    m_RecordButton->Show( Config->ReadBool( CONFIG_KEY_RECORD_ENABLED, false, CONFIG_PATH_RECORD ) );
    PlayerBtnSizer->Add( m_RecordButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT|wxRIGHT, guPLAYER_ICONS_SEPARATOR );

	PlayerBtnSizer->Add( guPLAYER_ICONS_GROUPSEPARATOR, 0, 0, wxEXPAND, 5 );

    m_PlayModeButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_smart ), guImage( guIMAGE_INDEX_player_highlight_smart ) );
    UpdatePlayModeButton();
    PlayerBtnSizer->Add( m_PlayModeButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

    //m_RandomPlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_random ), wxDefaultPosition, wxDefaultSize, 0 );
    m_RandomPlayButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_random ), guImage( guIMAGE_INDEX_player_highlight_random ) );
	m_RandomPlayButton->SetToolTip( _( "Randomize the tracks in playlist" ) );
	PlayerBtnSizer->Add( m_RandomPlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

    m_ForceGaplessButton = new guRoundButton( this,
                                guImage( m_ForceGapless ? guIMAGE_INDEX_player_normal_gapless : guIMAGE_INDEX_player_normal_crossfading ),
                                guImage( m_ForceGapless ? guIMAGE_INDEX_player_highlight_gapless : guIMAGE_INDEX_player_highlight_crossfading ) );
    m_ForceGaplessButton->SetToolTip( m_ForceGapless ? _( "Enable crossfading" ) : _( "Disable crossfading" ) );
    PlayerBtnSizer->Add( m_ForceGaplessButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT|wxRIGHT, guPLAYER_ICONS_SEPARATOR );

    PlayerBtnSizer->Add( guPLAYER_ICONS_GROUPSEPARATOR, 0, 0, wxEXPAND, 5 );

    //m_EqualizerButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_equalizer ), wxDefaultPosition, wxDefaultSize, 0 );
    m_EqualizerButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_equalizer ), guImage( guIMAGE_INDEX_player_highlight_equalizer ) );
    m_EqualizerButton->SetToolTip( _( "Show the equalizer" ) );
    PlayerBtnSizer->Add( m_EqualizerButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

    //m_VolumeButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_vol_mid ), wxDefaultPosition, wxDefaultSize, 0 );
    m_VolumeButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_vol_mid ), guImage( guIMAGE_INDEX_player_highlight_vol_mid ) );
    m_VolumeButton->SetToolTip( _( "Volume" ) + wxString::Format( wxT( " %i%%" ), ( int ) SavedVol ) );
    PlayerBtnSizer->Add( m_VolumeButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

    m_VolumeBar = new wxSlider( this, wxID_ANY, SavedVol, 0, 100, wxDefaultPosition, wxSize( 100, -1 ) );
    m_VolumeBar->SetMinSize( wxSize( 100, 20 ) );
    PlayerBtnSizer->Add( m_VolumeBar, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );
    if( Config->ReadBool( CONFIG_KEY_GENERAL_PLAYER_VOLUME_VISIBLE, true, CONFIG_PATH_GENERAL ) )
        m_VolumeBar->Show();
    else
        m_VolumeBar->Hide();
    PlayerMainSizer->Add( PlayerBtnSizer, 0, wxEXPAND, 2 );


    wxBoxSizer * PlayerDetailsSizer;
	PlayerDetailsSizer = new wxBoxSizer( wxHORIZONTAL );

    m_PlayerCoverBitmap = new wxStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_no_cover ), wxDefaultPosition, wxSize( 100, 100 ), 0 );
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

    PlayerLabelsSizer->Add( m_TitleLabel, 1, wxEXPAND|wxLEFT, 2 );

	//m_AlbumLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumLabel = new guAutoScrollText( this, wxEmptyString );
	m_AlbumLabel->SetToolTip( _( "Show the album name of the current track" ) );
	//m_AlbumLabel->Wrap( -1 );
	CurrentFont.SetPointSize( guPLAYER_FONTSIZE_ALBUMNAME );
	CurrentFont.SetWeight( wxFONTWEIGHT_NORMAL );
	CurrentFont.SetStyle( wxFONTSTYLE_ITALIC );
	m_AlbumLabel->SetFont( CurrentFont );

    PlayerLabelsSizer->Add( m_AlbumLabel, 1, wxEXPAND|wxLEFT, 2 );

	//m_ArtistLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistLabel = new guAutoScrollText( this, wxEmptyString );
	m_ArtistLabel->SetToolTip( _( "Show the artist name of the current track" ) );
	//m_ArtistLabel->Wrap( -1 );
	CurrentFont.SetPointSize( guPLAYER_FONTSIZE_ARTISTNAME );
	CurrentFont.SetStyle( wxFONTSTYLE_NORMAL );
	m_ArtistLabel->SetFont( CurrentFont );

    PlayerLabelsSizer->Add( m_ArtistLabel, 1, wxEXPAND|wxLEFT, 2 );

	m_PosLabelSizer = new wxBoxSizer( wxHORIZONTAL );
	//m_PosLabelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	m_YearLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_YearLabel->SetToolTip( _( "Show the year of the current track" ) );
    m_PosLabelSizer->Add( m_YearLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT, 2 );

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

    m_FormatLabel = new wxStaticText( this, wxID_ANY, "[]", wxDefaultPosition, wxDefaultSize, 0 );
    m_FormatLabel->SetToolTip( _( "Show the file format of the current track" ) );
    CurrentFont.SetPointSize( 8 );
    m_FormatLabel->SetFont( CurrentFont );

    m_BitRateSizer->Add( m_FormatLabel, 0, wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 2 );

    m_BitRateLabel = new wxStaticText( this, wxID_ANY, "[kbps]", wxDefaultPosition, wxDefaultSize, 0 );
	m_BitRateLabel->SetToolTip( _( "Show the bit rate of the current track" ) );
	m_BitRateLabel->SetFont( CurrentFont );

    m_BitRateSizer->Add( m_BitRateLabel, 0, wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 2 );

	PlayerLabelsSizer->Add( m_BitRateSizer, 0, wxEXPAND, 2 );

	PlayerDetailsSizer->Add( PlayerLabelsSizer, 1, wxEXPAND, 5 );

	PlayerMainSizer->Add( PlayerDetailsSizer, 0, wxEXPAND, 5 );

    m_PlayerPositionSlider = new wxSlider( this, wxID_ANY, 0, 0, 1000 );
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
    //guLogDebug( wxT( "CurVol: %i SavedVol: %i" ), int( m_MediaCtrl->GetVolume() * 100.0 ), ( int ) m_CurVolume );

    m_MediaCtrl->SetEqualizer( Equalizer );

    CheckFiltersEnable();

    // Bind Events
    m_PrevTrackButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnPrevTrackButtonClick, this );
    m_NextTrackButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnNextTrackButtonClick, this );
    m_PlayButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnPlayButtonClick, this );
    m_StopButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnStopButtonClick, this );
    m_RecordButton->Bind( wxEVT_TOGGLEBUTTON, &guPlayerPanel::OnRecordButtonClick, this );
    m_ForceGaplessButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnForceGaplessClick, this );
    m_VolumeButton->Bind( wxEVT_MOUSEWHEEL, &guPlayerPanel::OnVolumeMouseWheel, this );
    m_VolumeButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnVolumeClicked, this );
    m_VolumeBar->Bind( wxEVT_SCROLL_CHANGED	, &guPlayerPanel::OnVolumeChanged, this );
    m_VolumeBar->Bind( wxEVT_SCROLL_THUMBTRACK, &guPlayerPanel::OnVolumeChanged, this );
    m_VolumeBar->Bind( wxEVT_MOUSEWHEEL, &guPlayerPanel::OnVolumeMouseWheel, this );
    //m_SmartPlayButton->Bind( wxEVT_TOGGLEBUTTON, &guPlayerPanel::OnSmartPlayButtonClick, this );
    m_RandomPlayButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnRandomPlayButtonClick, this );
    //m_RepeatPlayButton->Bind( wxEVT_TOGGLEBUTTON, &guPlayerPanel::OnRepeatPlayButtonClick, this );
    m_EqualizerButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnEqualizerButtonClicked, this );
    m_PlayModeButton->Bind( wxEVT_BUTTON, &guPlayerPanel::OnPlayModeButtonClicked, this );

    Bind( wxEVT_MENU, &guPlayerPanel::OnRandomPlayButtonClick, this, ID_PLAYER_PLAYLIST_RANDOMPLAY );

    m_TitleLabel->Bind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnTitleNameDClicked, this );
	m_AlbumLabel->Bind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnAlbumNameDClicked, this );
	m_ArtistLabel->Bind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnArtistNameDClicked, this );
	m_YearLabel->Bind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnYearDClicked, this );

	m_PositionLabel->Bind( wxEVT_LEFT_UP, &guPlayerPanel::OnTimeDClicked, this );
	m_Rating->Bind( guEVT_RATING_CHANGED, &guPlayerPanel::OnRatingChanged, this );

	m_PlayerCoverBitmap->Bind( wxEVT_LEFT_DOWN, &guPlayerPanel::OnLeftClickPlayerCoverBitmap, this );

	m_PlayerPositionSlider->Bind( wxEVT_SCROLL_THUMBTRACK, &guPlayerPanel::OnPlayerPositionSliderBeginSeek, this );
	m_PlayerPositionSlider->Bind( wxEVT_SCROLL_THUMBRELEASE, &guPlayerPanel::OnPlayerPositionSliderEndSeek, this );
	m_PlayerPositionSlider->Bind( wxEVT_SCROLL_CHANGED	, &guPlayerPanel::OnPlayerPositionSliderChanged, this );
    m_PlayerPositionSlider->Bind( wxEVT_MOUSEWHEEL, &guPlayerPanel::OnPlayerPositionSliderMouseWheel, this );

    m_PlayListCtrl->Bind( wxEVT_MENU, &guPlayerPanel::OnPlayListUpdated, this, ID_PLAYER_PLAYLIST_UPDATELIST );
    m_PlayListCtrl->Bind( wxEVT_LISTBOX_DCLICK, &guPlayerPanel::OnPlayListDClick, this );

    Bind( guEVT_MEDIA_LOADED, &guPlayerPanel::OnMediaLoaded, this );
    Bind( guEVT_MEDIA_FINISHED, &guPlayerPanel::OnMediaFinished, this );
    Bind( guEVT_MEDIA_FADEOUT_FINISHED, &guPlayerPanel::OnMediaFadeOutFinished, this );
    Bind( guEVT_MEDIA_FADEIN_STARTED, &guPlayerPanel::OnMediaFadeInStarted, this );
    Bind( guEVT_MEDIA_TAGINFO, &guPlayerPanel::OnMediaTags, this );
    Bind( guEVT_MEDIA_CHANGED_BITRATE, &guPlayerPanel::OnMediaBitrate, this );
    Bind( guEVT_MEDIA_BUFFERING, &guPlayerPanel::OnMediaBuffering, this );
    Bind( guEVT_MEDIA_LEVELINFO, &guPlayerPanel::OnMediaLevel, this );
    Bind( guEVT_MEDIA_ERROR, &guPlayerPanel::OnMediaError, this );
    Bind( guEVT_MEDIA_CHANGED_STATE, &guPlayerPanel::OnMediaState, this );
    Bind( guEVT_MEDIA_CHANGED_POSITION, &guPlayerPanel::OnMediaPosition, this );
    Bind( guEVT_MEDIA_CHANGED_LENGTH, &guPlayerPanel::OnMediaLength, this );

    Bind( wxEVT_MENU, &guPlayerPanel::OnSmartAddTracks, this, ID_SMARTMODE_ADD_TRACKS );
    Bind( wxEVT_MENU, &guPlayerPanel::OnSmartEndThread, this, ID_SMARTMODE_THREAD_END );

    Bind( guConfigUpdatedEvent, &guPlayerPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_MENU, &guPlayerPanel::OnCoverUpdated, this, ID_PLAYERPANEL_COVERUPDATED );

    Bind( wxEVT_MENU, &guPlayerPanel::OnAddTracks, this, ID_PLAYERPANEL_ADDTRACKS );
    Bind( wxEVT_MENU, &guPlayerPanel::OnRemoveTrack, this, ID_PLAYERPANEL_REMOVETRACK );
    Bind( wxEVT_MENU, &guPlayerPanel::OnRepeat, this, ID_PLAYERPANEL_SETREPEAT );
    Bind( wxEVT_MENU, &guPlayerPanel::OnLoop, this, ID_PLAYERPANEL_SETLOOP );
    Bind( wxEVT_MENU, &guPlayerPanel::OnRandom, this, ID_PLAYERPANEL_SETRANDOM );
    Bind( wxEVT_MENU, &guPlayerPanel::OnSetVolume, this, ID_PLAYERPANEL_SETVOLUME );

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

//    if( m_UpdateCoverThread )
//    {
//        m_UpdateCoverThread->Pause();
//        m_UpdateCoverThread->Delete();
//    }

    if( m_AudioScrobble )
        delete m_AudioScrobble;

    if( m_MediaRecordCtrl )
        delete m_MediaRecordCtrl;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->UnRegisterObject( this );

        //printf( "guPlayerPanel::guConfig Save\n" );
        //Config->WriteBool( wxT( "PlayerStopped" ), m_MediaCtrl->GetState() != guMEDIASTATE_PLAYING, CONFIG_PATH_GENERAL );
        Config->WriteNum( CONFIG_KEY_GENERAL_PLAYER_VOLUME, m_CurVolume, CONFIG_PATH_GENERAL );
        Config->WriteBool( CONFIG_KEY_GENERAL_PLAYER_VOLUME_VISIBLE, m_VolumeBar->IsShown(), CONFIG_PATH_GENERAL );
        //Config->WriteNum( CONFIG_KEY_GENERAL_PLAYER_LOOP, m_PlayLoop, CONFIG_PATH_GENERAL );
        //Config->WriteBool( CONFIG_KEY_GENERAL_PLAYER_SMART, m_PlaySmart, CONFIG_PATH_GENERAL );
        Config->WriteNum( CONFIG_KEY_GENERAL_PLAYER_PLAYMODE, m_PlayMode, CONFIG_PATH_GENERAL );
        // If the track length is at least the configured minimun track length save the pos offset
        if( Config->ReadBool( CONFIG_KEY_GENERAL_SAVE_CURRENT_TRACK_POSITION, false, CONFIG_PATH_GENERAL ) )
        {
            if( ( m_LastPlayState != guMEDIASTATE_STOPPED ) &&
                ( m_MediaSong.m_Length >= ( unsigned int ) ( Config->ReadNum( CONFIG_KEY_GENERAL_MIN_SAVE_PLAYL_POST_LENGTH, 10, CONFIG_PATH_GENERAL ) * 60000 ) ) )
            {
                Config->WriteNum( CONFIG_KEY_GENERAL_CURRENT_TRACK_POS, m_LastCurPos, CONFIG_PATH_GENERAL );
            }
            else
            {
                Config->WriteNum( CONFIG_KEY_GENERAL_CURRENT_TRACK_POS, 0, CONFIG_PATH_GENERAL );
            }
        }
        //printf( PlaySmart ? "Smart Enabled" : "Smart Disabled" );  printf( "\n" );

        int index;
        wxArrayInt Equalizer;
        for( index = 0; index < guEQUALIZER_BAND_COUNT; index++ )
        {
            Equalizer.Add( m_MediaCtrl->GetEqualizerBand( index ) );
        }
        Config->WriteANum( CONFIG_KEY_EQUALIZER_BAND, Equalizer, CONFIG_PATH_EQUALIZER );

        Config->WriteBool( CONFIG_KEY_GENERAL_SHOW_REV_TIME, m_ShowRevTime, CONFIG_PATH_GENERAL );
        Config->WriteBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, m_ForceGapless, CONFIG_PATH_CROSSFADER );
    }

    // Unbind Events
    m_PrevTrackButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnPrevTrackButtonClick, this );
    m_NextTrackButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnNextTrackButtonClick, this );
    m_PlayButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnPlayButtonClick, this );
    m_StopButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnStopButtonClick, this );
    m_RecordButton->Unbind( wxEVT_TOGGLEBUTTON, &guPlayerPanel::OnRecordButtonClick, this );
    m_ForceGaplessButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnForceGaplessClick, this );
    m_VolumeButton->Unbind( wxEVT_MOUSEWHEEL, &guPlayerPanel::OnVolumeMouseWheel, this );
    //m_SmartPlayButton->Unbind( wxEVT_TOGGLEBUTTON, &guPlayerPanel::OnSmartPlayButtonClick, this );
    m_RandomPlayButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnRandomPlayButtonClick, this );
    //m_RepeatPlayButton->Unbind( wxEVT_TOGGLEBUTTON, &guPlayerPanel::OnRepeatPlayButtonClick, this );
    m_EqualizerButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnEqualizerButtonClicked, this );
    m_PlayModeButton->Unbind( wxEVT_BUTTON, &guPlayerPanel::OnPlayModeButtonClicked, this );

    Unbind( wxEVT_MENU, &guPlayerPanel::OnRandomPlayButtonClick, this, ID_PLAYER_PLAYLIST_RANDOMPLAY );

    m_TitleLabel->Unbind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnTitleNameDClicked, this );
    m_AlbumLabel->Unbind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnAlbumNameDClicked, this );
    m_ArtistLabel->Unbind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnArtistNameDClicked, this );
    m_YearLabel->Unbind( wxEVT_LEFT_DCLICK, &guPlayerPanel::OnYearDClicked, this );

    m_PositionLabel->Unbind( wxEVT_LEFT_UP, &guPlayerPanel::OnTimeDClicked, this );
    m_Rating->Unbind( guEVT_RATING_CHANGED, &guPlayerPanel::OnRatingChanged, this );

    m_PlayerCoverBitmap->Unbind( wxEVT_LEFT_DOWN, &guPlayerPanel::OnLeftClickPlayerCoverBitmap, this );

    m_PlayerPositionSlider->Unbind( wxEVT_SCROLL_THUMBTRACK, &guPlayerPanel::OnPlayerPositionSliderBeginSeek, this );
    m_PlayerPositionSlider->Unbind( wxEVT_SCROLL_THUMBRELEASE, &guPlayerPanel::OnPlayerPositionSliderEndSeek, this );
    m_PlayerPositionSlider->Unbind( wxEVT_SCROLL_CHANGED	, &guPlayerPanel::OnPlayerPositionSliderChanged, this );
    m_PlayerPositionSlider->Unbind( wxEVT_MOUSEWHEEL, &guPlayerPanel::OnPlayerPositionSliderMouseWheel, this );

    //m_PlayListCtrl->Unbind( wxEVT_MENU, &guPlayerPanel::OnPlayListUpdated, this, ID_PLAYER_PLAYLIST_UPDATELIST );
    //m_PlayListCtrl->Unbind( wxEVT_LISTBOX_DCLICK, &guPlayerPanel::OnPlayListDClick, this );

    Unbind( guEVT_MEDIA_LOADED, &guPlayerPanel::OnMediaLoaded, this );
    Unbind( guEVT_MEDIA_FINISHED, &guPlayerPanel::OnMediaFinished, this );
    Unbind( guEVT_MEDIA_FADEOUT_FINISHED, &guPlayerPanel::OnMediaFadeOutFinished, this );
    Unbind( guEVT_MEDIA_FADEIN_STARTED, &guPlayerPanel::OnMediaFadeInStarted, this );
    Unbind( guEVT_MEDIA_TAGINFO, &guPlayerPanel::OnMediaTags, this );
    Unbind( guEVT_MEDIA_CHANGED_BITRATE, &guPlayerPanel::OnMediaBitrate, this );
    Unbind( guEVT_MEDIA_BUFFERING, &guPlayerPanel::OnMediaBuffering, this );
    Unbind( guEVT_MEDIA_LEVELINFO, &guPlayerPanel::OnMediaLevel, this );
    Unbind( guEVT_MEDIA_ERROR, &guPlayerPanel::OnMediaError, this );
    Unbind( guEVT_MEDIA_CHANGED_STATE, &guPlayerPanel::OnMediaState, this );
    Unbind( guEVT_MEDIA_CHANGED_POSITION, &guPlayerPanel::OnMediaPosition, this );
    Unbind( guEVT_MEDIA_CHANGED_LENGTH, &guPlayerPanel::OnMediaLength, this );

    Unbind( wxEVT_MENU, &guPlayerPanel::OnSmartAddTracks, this, ID_SMARTMODE_ADD_TRACKS );
    Unbind( wxEVT_MENU, &guPlayerPanel::OnSmartEndThread, this, ID_SMARTMODE_THREAD_END );

    Unbind( guConfigUpdatedEvent, &guPlayerPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Unbind( wxEVT_MENU, &guPlayerPanel::OnCoverUpdated, this, ID_PLAYERPANEL_COVERUPDATED );

    Unbind( wxEVT_MENU, &guPlayerPanel::OnAddTracks, this, ID_PLAYERPANEL_ADDTRACKS );
    Unbind( wxEVT_MENU, &guPlayerPanel::OnRemoveTrack, this, ID_PLAYERPANEL_REMOVETRACK );
    Unbind( wxEVT_MENU, &guPlayerPanel::OnRepeat, this, ID_PLAYERPANEL_SETREPEAT );
    Unbind( wxEVT_MENU, &guPlayerPanel::OnLoop, this, ID_PLAYERPANEL_SETLOOP );
    Unbind( wxEVT_MENU, &guPlayerPanel::OnRandom, this, ID_PLAYERPANEL_SETRANDOM );
    Unbind( wxEVT_MENU, &guPlayerPanel::OnSetVolume, this, ID_PLAYERPANEL_SETVOLUME );

    if( m_MediaCtrl )
        delete m_MediaCtrl;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    guConfig * Config = NULL;
    bool MediaCtrlNeedUpdated = false;

    if( Flags & guPREFERENCE_PAGE_FLAG_PLAYBACK )
    {
        Config = ( guConfig * ) guConfig::Get();

        //guLogDebug( wxT( "Reading PlayerPanel Config Updated" ) );
        m_PlayRandom = Config->ReadBool( CONFIG_KEY_GENERAL_RANDOM_PLAY_ON_EMPTY_PLAYLIST, false, CONFIG_PATH_GENERAL );
        m_PlayRandomMode = Config->ReadNum( CONFIG_KEY_GENERAL_RANDOM_MODE_ON_EMPTY_PLAYLIST, guRANDOM_MODE_TRACK, CONFIG_PATH_GENERAL );
        m_ShowNotifications = Config->ReadBool( CONFIG_KEY_GENERAL_SHOW_NOTIFICATIONS, true, CONFIG_PATH_GENERAL );
        m_ShowNotificationsTime = Config->ReadNum( CONFIG_KEY_GENERAL_NOTIFICATION_TIME, 0, CONFIG_PATH_GENERAL );

        m_SmartPlayAddTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_NUM_TRACKS_TO_ADD, 3, CONFIG_PATH_PLAYBACK );
        m_SmartPlayMinTracksToPlay = Config->ReadNum( CONFIG_KEY_PLAYBACK_MIN_TRACKS_PLAY, 4, CONFIG_PATH_PLAYBACK );
        m_DelTracksPlayed = Config->ReadBool( CONFIG_KEY_PLAYBACK_DEL_TRACKS_PLAYED, false, CONFIG_PATH_PLAYBACK );
        m_SmartMaxArtistsList = Config->ReadNum( CONFIG_KEY_PLAYBACK_SMART_FILTER_ARTISTS, 20, CONFIG_PATH_PLAYBACK );
        m_SmartMaxTracksList = Config->ReadNum( CONFIG_KEY_PLAYBACK_SMART_FILTER_TRACKS, 100, CONFIG_PATH_PLAYBACK );

        // We only insert the last CACHEITEMS as the rest should be forgiven
        while( ( int ) m_SmartAddedTracks.Count() > m_SmartMaxTracksList )
            m_SmartAddedTracks.RemoveAt( 0 );

        while( ( int ) m_SmartAddedArtists.Count() > m_SmartMaxArtistsList )
            m_SmartAddedArtists.RemoveAt( 0 );

        m_SilenceDetector = Config->ReadBool( CONFIG_KEY_PLAYBCK_SILENCE_DETECTOR, false, CONFIG_PATH_PLAYBACK );
        m_SilenceDetectorLevel = Config->ReadNum( CONFIG_KEY_PLAYBCK_SILENCE_LEVEL, -55, CONFIG_PATH_PLAYBACK );
        if( Config->ReadBool( CONFIG_KEY_PLAYBCK_SILENCE_AT_END, false, CONFIG_PATH_PLAYBACK ) )
        {
            m_SilenceDetectorTime = Config->ReadNum( CONFIG_KEY_PLAYBCK_SILENCE_END_TIME, 45, CONFIG_PATH_PLAYBACK ) * 1000;
        }

        MediaCtrlNeedUpdated = true;
    }

    if( Flags & guPREFERENCE_PAGE_FLAG_RECORD )
    {
        if( !Config )
            Config = ( guConfig * ) guConfig::Get();

        m_RecordButton->Show( Config->ReadBool( CONFIG_KEY_RECORD_ENABLED, false, CONFIG_PATH_RECORD ) );
        Layout();

        if( m_MediaRecordCtrl )
        {
            m_MediaRecordCtrl->UpdatedConfig();
        }
    }

    if( Flags & guPREFERENCE_PAGE_FLAG_CROSSFADER )
    {
        if( !Config )
            Config = ( guConfig * ) guConfig::Get();

        m_ForceGapless = Config->ReadBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, false, CONFIG_PATH_CROSSFADER );
        m_FadeOutTime = Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEOUT_TIME, 50, CONFIG_PATH_CROSSFADER ) * 100;

        MediaCtrlNeedUpdated = true;
    }

    if( Flags & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
    {
        if( !Config )
            Config = ( guConfig * ) guConfig::Get();

        m_AudioScrobbleEnabled = Config->ReadBool( CONFIG_KEY_LASTFM_ENABLED, false, CONFIG_PATH_LASTFM ) ||
                                 Config->ReadBool( CONFIG_KEY_LIBREFM_ENABLED, false, CONFIG_PATH_LIBREFM );

        if( m_AudioScrobbleEnabled )
        {
            if( !m_AudioScrobble )
                m_AudioScrobble = new guAudioScrobble( m_Db );
            else
                m_AudioScrobble->OnConfigUpdated();
        }
        else
        {
            wxCommandEvent event( wxEVT_MENU, ID_AUDIOSCROBBLE_UPDATED );
            event.SetInt( 1 );
            wxPostEvent( m_MainFrame, event );
        }
    }

    if( m_MediaCtrl && MediaCtrlNeedUpdated )
    {
        m_MediaCtrl->UpdatedConfig();
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
    unsigned int CurLen = m_LastLength;
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
void guPlayerPanel::SetFormatLabel( const wxString &format )
{
    //guLogDebug( wxT( "SetFormatLabel( %s )" ), format.c_str() );
    if (m_MediaSong.m_Format.Length() > 0 )
    {
        m_FormatLabel->Show();
        m_FormatLabel->SetLabel( wxString::Format( wxT( "[%s]" ), format.c_str() ) );
    }
    else
        m_FormatLabel->Hide();
    m_BitRateSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetBitRateLabel( int bitrate )
{
    //guLogDebug( wxT( "SetBitRateLabel( %i )" ), bitrate );
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
        //guLogDebug( wxT( "Bitrate: %u" ), bitrate );
        m_BitRateLabel->SetLabel( wxString::Format( wxT( "[%ukbps]" ), bitrate ) );
        if( ( m_MediaSong.m_Bitrate < bitrate ) && ( GetState() == guMEDIASTATE_PLAYING ) )
        {
            m_MediaSong.m_Bitrate = bitrate;

            if( m_MediaSong.m_MediaViewer )
            {
                guDbLibrary * Db = m_MediaSong.m_MediaViewer->GetDb();
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

    if( m_PlayMode == guPLAYER_PLAYMODE_SMART )
    {
        if( m_SmartAddTracksThread )
        {
            m_SmartAddTracksThread->Pause();
            m_SmartAddTracksThread->Delete();
            m_SmartAddTracksThread = NULL;
            m_SmartSearchEnabled = false;
        }

        // Reset the Smart played items
        m_SmartAddedTracks.Empty();
        m_SmartAddedArtists.Empty();

        //guLogDebug( wxT( "SetPlayList adding track to smart cache..." ) );
        int Index = 0;
        int Count = SongList.Count();

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
        m_PlayListCtrl->AddPlayListItem( files[ Index ], guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND );
    }
    m_PlayListCtrl->ReloadItems();
    if( m_PlayListCtrl->GetItemCount() )
    {
        m_PlayListCtrl->SetCurrent( 0 );
        SetNextTrack( m_PlayListCtrl->GetCurrent() );

        LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );

        TrackListChanged();

        // Add the added track to the smart cache
        if( m_PlayMode == guPLAYER_PLAYMODE_SMART )
        {
            if( m_SmartAddTracksThread )
            {
                m_SmartAddTracksThread->Pause();
                m_SmartAddTracksThread->Delete();
                m_SmartAddTracksThread = NULL;
                m_SmartSearchEnabled = false;
            }

            // Reset the Smart played items
            m_SmartAddedTracks.Empty();
            m_SmartAddedArtists.Empty();

            //guLogDebug( wxT( "SetPlayList adding track to smart cache..." ) );
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
void guPlayerPanel::AddToPlayList( const guTrackArray &tracks, const bool allowplay, const int aftercurrent )
{
    int PrevTrackCount = m_PlayListCtrl->GetCount();
    if( tracks.Count() )
    {
        guTrack * Track = &tracks[ 0 ];
        bool ClearPlayList = ( Track->m_TrackMode == guTRACK_MODE_RANDOM ||
                               Track->m_TrackMode == guTRACK_MODE_SMART ) && !m_DelTracksPlayed;

        m_PlayListCtrl->AddToPlayList( tracks, ClearPlayList, aftercurrent );

        TrackListChanged();

        if( m_PlayMode == guPLAYER_PLAYMODE_SMART )
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

        // Change vehaivour to start playing when its not playing
        if( allowplay && !PrevTrackCount && ( GetState() != guMEDIASTATE_PLAYING ) )
        {
            wxCommandEvent CmdEvent;
            OnNextTrackButtonClick( CmdEvent );
            OnPlayButtonClick( CmdEvent );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const wxString &FileName, const int aftercurrent  )
{
    int PrevTrackCount = m_PlayListCtrl->GetCount();

    m_PlayListCtrl->AddPlayListItem( FileName, aftercurrent, wxNOT_FOUND );
    m_PlayListCtrl->ReloadItems();
    TrackListChanged();
    // Add the added track to the smart cache
    if( m_PlayMode == guPLAYER_PLAYMODE_SMART )
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
void guPlayerPanel::AddToPlayList( const wxArrayString &files, const int aftercurrent  )
{
    int PrevTrackCount = m_PlayListCtrl->GetItemCount();

    int Index;
    int Count = files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_PlayListCtrl->AddPlayListItem( files[ Index ], aftercurrent, aftercurrent ? Index : wxNOT_FOUND );
    }

    m_PlayListCtrl->ReloadItems();
    TrackListChanged();

    // Add the added track to the smart cache
    if( m_PlayMode == guPLAYER_PLAYMODE_SMART )
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
void guPlayerPanel::AddToPlayList( const wxArrayString &files, const bool allowplay, const int aftercurrent  )
{
    guLogDebug( wxT( "AddToPlayList( ..., %i, %i )" ), allowplay, aftercurrent );
    int PrevTrackCount = m_PlayListCtrl->GetItemCount();

    int Index;
    int Count = files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_PlayListCtrl->AddPlayListItem( files[ Index ], aftercurrent, Index );
    }

    m_PlayListCtrl->ReloadItems();
    TrackListChanged();

    // Add the added track to the smart cache
    if( m_PlayMode == guPLAYER_PLAYMODE_SMART )
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

    if( allowplay && ( m_PlayListCtrl->GetCount() > PrevTrackCount ) && ( GetState() != guMEDIASTATE_PLAYING ) )
    {
        wxCommandEvent CmdEvent;
        CmdEvent.SetInt( PrevTrackCount );
        OnPlayListDClick( CmdEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::TrackListChanged( void )
{
//    m_PlayListLenStaticText->SetLabel( m_PlayListCtrl->GetLengthStr() );
//   	m_PlayListLabelsSizer->Layout();
//    m_PlayListCtrl->SetColumnLabel( 0, _( "Now Playing" ) +
//        wxString::Format( wxT( ":  %i / %li    ( %s )" ),
//            m_PlayListCtrl->GetCurItem() + 1,
//            m_PlayListCtrl->GetCount(),
//            m_PlayListCtrl->GetLengthStr().c_str() ) );
    wxCommandEvent TitleEvent( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATETITLE );
    wxPostEvent( m_MainFrame, TitleEvent );

    wxCommandEvent TracksChangedEvent( wxEVT_MENU, ID_PLAYERPANEL_TRACKLISTCHANGED );
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
    if( ( event.GetExtraLong() || event.GetInt() ) &&
        ( m_PlayMode == guPLAYER_PLAYMODE_SMART ) )
    {
        if( m_SmartAddTracksThread )
        {
            m_SmartAddTracksThread->Pause();
            m_SmartAddTracksThread->Delete();
            m_SmartAddTracksThread = NULL;
            m_SmartSearchEnabled = false;
        }

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

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnSmartEndThread( wxCommandEvent &event )
{
    m_SmartAddTracksThread = NULL;
    m_SmartSearchEnabled = false;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnSmartAddTracks( wxCommandEvent &event )
{
    guTrackArray * Tracks = ( guTrackArray * ) event.GetClientData();
    if( Tracks )
    {
        //guLogMessage( wxT( "Tracks %i" ), Tracks->Count() );
        if( Tracks->Count() )
        {
            AddToPlayList( * Tracks );
        }
        delete Tracks;
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

    m_SmartAddTracksThread = new guSmartModeThread( CurSong.m_MediaViewer ? CurSong.m_MediaViewer->GetDb() : m_Db, this,
        CurSong.m_ArtistName, CurSong.m_SongName,
        &m_SmartAddedTracks, &m_SmartAddedArtists,
        m_SmartMaxTracksList, m_SmartMaxArtistsList,
        m_SmartPlayAddTracks, guSMARTMODE_TRACK_LIMIT_TRACKS,
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
    guLogDebug( wxT( "SetNextTrack: %i" ), m_PlayListCtrl->GetCurItem() );

    if( !Song )
        return;

    m_NextSong = * Song;
    //m_NextTrackId = true;

}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListDClick( wxCommandEvent &event )
{
    int item = event.GetInt();
    m_PlayListCtrl->SetCurrent( item, m_DelTracksPlayed && !GetPlayLoop() );

    SetNextTrack( m_PlayListCtrl->GetCurrent() );
    //wxLogMessage( wxT( "Selected %i : %s - %s" ), m_MediaSong.SongId, m_MediaSong.ArtistName.c_str(), m_MediaSong.SongName.c_str() );

    LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );
}

// -------------------------------------------------------------------------------- //
wxString inline FileNameEncode( const wxString filename )
{
    static const wxChar NumChars[] = wxT( "0123456789" );
    wxString RetVal = filename;

    if( filename.StartsWith( wxT( "file://" ) ) )
    {
        RetVal.Replace( wxT( "%" ), wxT( "%25" ) );
    }
    else
    {
        int Pos;
        while( ( Pos = RetVal.Find( wxT( "%" ) ) ) != wxNOT_FOUND &&
                !( wxStrchr( NumChars, RetVal[ Pos + 1 ] ) &&
                   wxStrchr( NumChars, RetVal[ Pos + 2 ] ) ) )
        {
            RetVal = RetVal.Mid( 0, Pos ) + wxT( "%25" ) + RetVal.Mid( Pos + 1 );
        }
    }
    RetVal.Replace( wxT( "#" ), wxT( "%23" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::LoadMedia( guFADERPLAYBIN_PLAYTYPE playtype, const bool forceskip )
{
    guLogDebug( wxT( "LoadMedia  %i  %i  (%i)" ), m_CurTrackId, m_NextTrackId, m_SavedPlayedTrack );
    if( !forceskip && ( m_MediaSong.m_Type & guTRACK_TYPE_STOP_HERE ) )
    {
        m_MediaSong.m_Type = guTrackType( int( m_MediaSong.m_Type ) ^ guTRACK_TYPE_STOP_HERE );
        m_PlayListCtrl->ClearStopAtEnd();
        return;
    }

    guLogDebug( wxT( "LoadMedia Cur: %i  %i starting at %i" ), m_PlayListCtrl->GetCurItem(), playtype, m_NextSong.m_Offset );
    //m_MediaCtrl->Load( NextItem->FileName );
    wxURI UriPath( m_NextSong.m_FileName );
    wxString Uri;
    try {
        if( !UriPath.HasScheme() )
            Uri = wxT( "file://" ) + m_NextSong.m_FileName;
        else
            Uri = m_NextSong.m_FileName;

        if( ( playtype == guFADERPLAYBIN_PLAYTYPE_AFTER_EOS ) && ( m_MediaSong.m_Offset || m_NextSong.m_Offset ) )
        {
            playtype = guFADERPLAYBIN_PLAYTYPE_REPLACE;
        }

        //guLogDebug( wxT( "'%s'\n'%s'" ), FileName.c_str(), FileNameEncode( Uri ).c_str() );
        m_NextTrackId = m_MediaCtrl->Load( FileNameEncode( Uri ), playtype, m_NextSong.m_Offset + m_TrackStartPos );
        if( m_TrackStartPos )
        {
            m_TrackStartPos = 0;
        }
        if( !m_NextTrackId )
        {
            guLogError( wxT( "ee: Failed load of file '%s'" ), Uri.c_str() );

            int CurItem = m_PlayListCtrl->GetCurItem();

            wxCommandEvent event;
            event.SetId( ID_PLAYERPANEL_NEXTTRACK );
            event.SetInt( playtype );
            wxPostEvent( this, event );
            //m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
            RemoveItem( CurItem );
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
    wxCommandEvent GaugeEvent( wxEVT_MENU, ID_STATUSBAR_GAUGE_SETMAX );
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
          ( m_MediaSong.m_Length > ( unsigned int ) ( m_SilenceDetectorTime + 10000 ) ) ) )
    {
        //guLogDebug( wxT( "Decay Level: %0.2f /  %02i  %s  %li > %li" ), LevelInfo->m_Decay_L, m_SilenceDetectorLevel, LenToString( ( unsigned int ) ( m_LastCurPos / 1000 ) ).c_str(), m_MediaSong.m_Length * 1000, ( unsigned int ) ( m_SilenceDetectorTime + 10000 ) );
        if( LevelInfo->m_Decay_L < double( m_SilenceDetectorLevel ) )
        {
            unsigned long EventTime = m_LastCurPos; //LevelInfo->m_EndTime;
            if( EventTime > 10000 )
            {
                unsigned long TrackLength = m_MediaSong.m_Length;
                //guLogDebug( wxT( "The level is now lower than triger level" ) );
                //guLogDebug( wxT( "(%f) %02i : %li , %i, %i" ), LevelInfo->m_Decay_L, m_SilenceDetectorLevel, EventTime, TrackLength - EventTime, m_SilenceDetectorTime );
                //guLogDebug( wxT( "(%li) %f %02i : %li , %i, %i" ), event.GetExtraLong(), LevelInfo->m_Decay_L, m_SilenceDetectorLevel, EventTime, TrackLength - EventTime, m_SilenceDetectorTime );


                // We only skip to next track if the level is lower than the triger one and also if
                // we are at the end time period (if configured this way) and the time left is more than 500msecs
                if( !m_TrackChanged && !m_NextTrackId && ( m_CurTrackId == event.GetExtraLong() ) &&
                    ( !m_SilenceDetectorTime || ( ( ( unsigned int ) m_SilenceDetectorTime > ( TrackLength - EventTime ) ) &&
                      ( ( EventTime + 500 ) < TrackLength ) ) ) )
                {
                    m_SilenceDetected = true;
                    //guLogDebug( wxT( "Silence detected. Changed to next track %i" ), m_PlayListCtrl->GetCurItem() );
                    wxCommandEvent evt;
                    evt.SetId( ID_PLAYERPANEL_NEXTTRACK );
                    OnNextTrackButtonClick( evt );
                }
            }
        }
    }

    if( m_PlayerVumeters )
    {
        //guLogDebug( wxT( "%lli %lli   (%lli)" ), LevelInfo->m_EndTime, LevelInfo->m_OutTime, LevelInfo->m_OutTime - LevelInfo->m_EndTime );
        m_PlayerVumeters->SetLevels( * LevelInfo );

//        guLogDebug( wxT( "L:%02.02f  R:%02.02f" ),
//            event.m_LevelInfo.m_Peak_L,
//            event.m_LevelInfo.m_Peak_R );
    }

    delete LevelInfo;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaError( guMediaEvent &event )
{
    guLogDebug( wxT( "OnMediaError: %i" ), m_PlayListCtrl->GetCurItem() );
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

    m_NextTrackId = 0;

    wxCommandEvent CmdEvent;
    CmdEvent.SetInt( guFADERPLAYBIN_PLAYTYPE_REPLACE );
    OnNextTrackButtonClick( CmdEvent );

    RemoveItem( CurItem );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaState( guMediaEvent &event )
{
    guLogDebug( wxT( "OnMediaState: %i %li %li" ), event.GetInt(), m_CurTrackId, m_NextTrackId );
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
        wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_STATUSCHANGED );
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
    //guLogDebug( wxT( "OnMediaPosition... %i / %i - %li" ), event.GetInt(), m_MediaSong.m_Offset, event.GetExtraLong() );

    if( ( event.GetInt() < 0 ) || ( event.GetInt() < ( int ) m_MediaSong.m_Offset ) )
        return;

    long EventId = ( long ) event.GetClientData();

    if( EventId != m_CurTrackId )
        return;

    wxFileOffset CurPos = event.GetInt() - m_MediaSong.m_Offset;
    wxFileOffset CurLen = event.GetExtraLong();

    if( !m_MediaSong.m_Offset )
    {
        if( CurLen && ( CurLen != m_LastLength ) )
        {
            m_LastLength = CurLen;

            if( !m_LastLength )
                m_PlayerPositionSlider->SetValue( 0 );

            // Some track lengths are not correctly read by taglib so
            // we try to find the length from gstreamer and update the database
            // We need to not do this for radiostations or online streams
            if( CurLen > 0 && m_MediaSong.m_Length == 0 &&
                m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
            {
                m_MediaSong.m_Length = CurLen;

                if( m_MediaSong.m_SongId )
                {
                    if( m_MediaSong.m_MediaViewer )
                    {
                        guDbLibrary * Db = m_MediaSong.m_MediaViewer->GetDb();
                        Db->UpdateTrackLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
                    }
                    else if( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST )
                    {
                        guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                        DbPodcasts->UpdatePodcastItemLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
                    }
                }

                // Update the track in database, playlist, etc
                m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
            }
        }
    }
    else
    {
        if( m_LastLength != m_MediaSong.m_Length )
            m_LastLength = m_MediaSong.m_Length;
    }

    if( ( ( CurPos / 1000 ) != ( m_LastCurPos / 1000 ) ) && !m_SliderIsDragged )
    {
        guLogDebug( wxT( "OnMediaPosition... %i - %li => %li  %li %li" ), CurPos, CurLen, m_LastLength - CurPos, m_CurTrackId, m_NextTrackId );
        m_LastCurPos = CurPos;

        if( m_TrackChanged )
            m_TrackChanged = false;

        UpdatePositionLabel( CurPos );

        if( m_LastLength )
        {
            m_PlayerPositionSlider->SetValue( CurPos / ( m_LastLength / 1000 ) );
        }

        m_MediaSong.m_PlayTime = CurPos;

        if( !m_AboutToEndDetected &&
            !m_NextTrackId &&
            ( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION ) &&
            ( CurPos > 0 ) && ( m_LastLength > 0 ) &&
            ( ( CurPos + 4000 + ( !m_ForceGapless ? m_FadeOutTime : 0 ) ) >= m_LastLength )
          )
        {
            //if( !m_ForceGapless && m_FadeOutTime && !m_MediaSong.m_Offset && ( GetState() == guMEDIASTATE_PLAYING ) )
            if( ( !( m_ForceGapless || !m_FadeOutTime ) || !m_MediaSong.m_Offset ) && ( GetState() == guMEDIASTATE_PLAYING ) )
            {
                m_AboutToEndDetected = true;

                guLogDebug( wxT( "Detected about to finish track... Trying to load the next track..." ) );
                wxCommandEvent evt;
                OnNextTrackButtonClick( evt );
            }
        }
    }

    //
    // If we are in gapless mode with cue tracks only skip once we reach the end of the track.
    if( m_MediaSong.m_Offset && ( m_ForceGapless || !m_FadeOutTime ) && ( ( CurPos + 250 ) >= m_LastLength ) )
    {
        guLogDebug( wxT( "Track should finish now..." ) );
        wxCommandEvent evt;
        OnNextTrackButtonClick( evt );
    }
}

// -------------------------------------------------------------------------------- //
void  guPlayerPanel::OnMediaLength( guMediaEvent &event )
{
    guLogDebug( wxT( "OnMediaLength... %i" ), event.GetInt() );

    if( m_MediaSong.m_Offset )
        return;

    wxFileOffset CurLen = event.GetInt();

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


//// -------------------------------------------------------------------------------- //
//void guPlayerPanel::SendRecordSplitEvent( void )
//{
//    guLogMessage( wxT( "guPlayerPanel::SendRecordSplitEvent" ) );
//    // If its buffering
//    if( m_BufferGaugeId != wxNOT_FOUND )
//    {
//        m_PendingNewRecordName = true;
//        //guLogDebug( wxT( "Player is buffering. Will rename recording once its finished" ) );
//        return;
//    }

//    m_MediaRecordCtrl->SplitTrack();
//}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaTags( guMediaEvent &event )
{
    //guLogDebug( wxT( "OnMediaTags..." ) );
    guRadioTagInfo * RadioTag = ( guRadioTagInfo * ) event.GetClientData();
    if( RadioTag )
    {
        if( ( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION ) ||
            ( m_NextTrackId && ( m_NextSong.m_Type == guTRACK_TYPE_RADIOSTATION ) ) )
        {
            //guLogDebug( wxT( "Radio Name: %s" ), wxString( RadioTag->m_Organization, wxConvUTF8 ).c_str() );
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
                //guLogDebug( wxT( "Radio Genre: %s" ), wxString( RadioTag->m_Genre, wxConvUTF8 ).c_str() );
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

            //guLogDebug( wxT( "MediaTag:'%s'" ), TagStr->c_str() );
            if( RadioTag->m_Title )
            {
                wxString Title( RadioTag->m_Title, wxConvUTF8 );
                //guLogDebug( wxT( "Radio Title: %s" ), Title.c_str() );
                if( m_NextTrackId )
                {
                    ExtractMetaData( Title,
                            m_NextSong.m_ArtistName,
                            m_NextSong.m_SongName );
                }
                else
                {
                    if( m_AudioScrobble )
                        m_AudioScrobble->SendPlayedTrack( m_MediaSong );

                    ExtractMetaData( Title,
                            m_MediaSong.m_ArtistName,
                            m_MediaSong.m_SongName );
                    SetTitleLabel( m_MediaSong.m_SongName );
                    SetArtistLabel( m_MediaSong.m_ArtistName );
                }

                //guLogDebug( wxT( "AlbumName: '%s'" ), m_MediaSong.m_AlbumName.c_str() );

                if( m_MediaRecordCtrl && m_MediaRecordCtrl->IsRecording() )
                {
                    if( m_NextTrackId )
                        m_MediaRecordCtrl->SetTrackName( m_NextSong.m_ArtistName, m_NextSong.m_SongName );
                    else
                        m_MediaRecordCtrl->SetTrackName( m_MediaSong.m_ArtistName, m_MediaSong.m_SongName );

                    //SendRecordSplitEvent();
                }

                if( m_AudioScrobbleEnabled && m_AudioScrobble && m_AudioScrobble->IsOk() )
                {
                    if( !m_NextTrackId )
                    {
                        if( m_AudioScrobble )
                            m_AudioScrobble->SendNowPlayingTrack( m_MediaSong );
                    }
                }

                //guLogDebug( wxT( "Sending LastFMPanel::UpdateTrack event" ) );
                wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_TRACKCHANGED );
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
    //guLogDebug( wxT( "OnMediaBitrate... (%li) %i" ), event.GetExtraLong(), event.GetInt() );
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
    guLogDebug( wxT( "OnMediaLoaded Cur: %i %i   %li" ), m_PlayListCtrl->GetCurItem(), event.GetInt(), m_NextTrackId );

    try {

        if( event.GetInt() )
        {
            //
            m_MediaCtrl->Play();
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
    guLogDebug( wxT( "OnMediaPlayStarted  %i %i" ), m_CurTrackId, m_NextTrackId );

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
    m_LastLength = m_MediaSong.m_Length;
    if( !m_LastLength )
        m_PlayerPositionSlider->SetValue( 0 );

    // Update the Current Playing Song Info
    UpdateLabels();
    //UpdatePositionLabel( 0 );

//    m_PlayListCtrl->SetColumnLabel( 0, _( "Now Playing" ) +
//        wxString::Format( wxT( ":  %i / %li    ( %s )" ),
//            m_PlayListCtrl->GetCurItem() + 1,
//            m_PlayListCtrl->GetCount(),
//            m_PlayListCtrl->GetLengthStr().c_str() ) );
    wxCommandEvent TitleEvent( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATETITLE );
    wxPostEvent( m_MainFrame, TitleEvent );


    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_TRACKCHANGED );
    event.SetClientData( new guTrack( m_MediaSong ) );
    wxPostEvent( m_MainFrame, event );

    // Update the cover
    UpdateCover();

    // Check if Smart is enabled
    if( ( m_PlayMode == guPLAYER_PLAYMODE_SMART ) &&
        ( ( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION ) || ( m_MediaSong.m_Type != guTRACK_TYPE_PODCAST ) ) &&
        ( !m_SmartPlayMinTracksToPlay || ( ( m_PlayListCtrl->GetCurItem() + m_SmartPlayMinTracksToPlay ) >= m_PlayListCtrl->GetCount() ) ) )
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
    //wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_CAPSCHANGED );
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

    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_COVERUPDATED );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdateCover( const bool shownotify, const bool deleted )
{
//    if( m_UpdateCoverThread )
//    {
//        m_UpdateCoverThread->Pause();
//        m_UpdateCoverThread->Delete();
//    }

    //guLogMessage( wxT( "UpdateCover: '%s'  /// '%s'  %i" ), m_MediaSong.m_ArtistName.c_str(), m_MediaSong.m_AlbumName.c_str(), deleted );

    guUpdatePlayerCoverThread * UpdateCoverThread = new guUpdatePlayerCoverThread( m_Db, m_MainFrame, this, &m_MediaSong, shownotify, deleted );
    if( !UpdateCoverThread )
    {
        guLogError( wxT( "Could not create the UpdateCover thread." ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnCoverUpdated( wxCommandEvent &event )
{
//    m_UpdateCoverThread = NULL;

    UpdateCoverImage( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SavePlayedTrack( const bool forcesave )
{
    if( m_SavedPlayedTrack )
        return;

    m_SavedPlayedTrack = true;

    // If have played 'enough' of the song. At least half or >= 4 min.
    bool HeardEnought = m_MediaSong.m_PlayTime >= wxMin( guAS_MIN_PLAYTIME * 1000, m_MediaSong.m_Length / 2 );

    // Check if the Current Song have played more than the half or >= 4 min and if so add it to
    // The CachedPlayedSong database to be submitted to LastFM AudioScrobbling
    if( m_AudioScrobbleEnabled && ( m_MediaSong.m_Type != guTRACK_TYPE_PODCAST ) ) // If its not a podcast
    {
        guLogDebug( wxT( "PlayTime: %u Length: %u" ), m_MediaSong.m_PlayTime, m_MediaSong.m_Length );
        if( HeardEnought && 						   // >= 4min || >= half-of-length
            ( m_MediaSong.m_PlayTime > ( guAS_MIN_TRACKLEN * 1000 ) ) )    // If the Length is more than 30 secs
        {
            if( !m_MediaSong.m_SongName.IsEmpty() &&    // Check if we have no missing data
                !m_MediaSong.m_ArtistName.IsEmpty() )
            {
                //
                m_AudioScrobble->SendPlayedTrack( m_MediaSong );
            }
        }
    }

    // Update the play count if it has played enough of the track
    if( m_MediaSong.m_Loaded )
    {
        if( !SupportedPlayCountTypes.Count() )
        {
            SupportedPlayCountTypes.Add( guTRACK_TYPE_DB );
            SupportedPlayCountTypes.Add( guTRACK_TYPE_IPOD );
            SupportedPlayCountTypes.Add( guTRACK_TYPE_JAMENDO );
            SupportedPlayCountTypes.Add( guTRACK_TYPE_MAGNATUNE );
            SupportedPlayCountTypes.Add( guTRACK_TYPE_PODCAST );
        }

        if( SupportedPlayCountTypes.Index( m_MediaSong.m_Type ) != wxNOT_FOUND )
        {
            if( HeardEnought )
            {
                m_MediaSong.m_PlayCount++;
                m_MediaSong.m_LastPlay = wxDateTime::GetTimeNow();
                guLogDebug( wxT( "Increased PlayCount to %i" ), m_MediaSong.m_PlayCount );

                if( !m_MediaSong.m_Offset && m_MediaSong.m_MediaViewer && m_MediaSong.m_MediaViewer->GetEmbeddMetadata() )
                {
                    guTrackArray Tracks;
                    Tracks.Add( m_MediaSong );
                    guImagePtrArray Images;
                    wxArrayString Lyrics;
                    wxArrayInt ChangedFlags;
                    ChangedFlags.Add( guTRACK_CHANGED_DATA_RATING );
                    guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
                }

                if( m_MediaSong.m_Type != guTRACK_TYPE_PODCAST )
                {
                    if( m_MediaSong.m_MediaViewer )
                    {
                        guDbLibrary * Db = m_MediaSong.m_MediaViewer->GetDb();
                        Db->SetTrackPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount );
                    }
                }
                else
                {
                    guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                    DbPodcasts->SetPodcastItemPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount );
                }

                // Update the track in database, playlist, etc
                m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
            }
        }
    }

    m_MainFrame->CheckPendingUpdates( &m_MediaSong, forcesave );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFinished( guMediaEvent &event )
{
    guLogDebug( wxT( "OnMediaFinished (%li) Cur: %i  %li" ), event.GetExtraLong(), m_PlayListCtrl->GetCurItem(), m_NextTrackId );

    if( m_SilenceDetected || m_AboutToEndDetected || m_NextTrackId || ( m_CurTrackId != event.GetExtraLong() ) )
    {
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
        guLogDebug( wxT( "Media Finished Cancelled... %li %li" ), m_CurTrackId, m_NextTrackId );
        return;
    }

    guTrack * NextItem = m_PlayListCtrl->GetNext( m_PlayMode );
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
            if( GetPlayLoop() )
                SetPlayMode( guPLAYER_PLAYMODE_NONE );

            //guLogDebug( wxT( "Getting Random Tracks..." ) );
            guTrackArray Tracks;
            if( m_Db->GetRandomTracks( &Tracks, m_SmartPlayAddTracks, m_PlayRandomMode,
                    m_PlayerFilters->GetAllowFilterId(),
                    m_PlayerFilters->GetDenyFilterId() ) )
            {
                if( Tracks.Count() )
                {
                    AddToPlayList( Tracks, false );

                    OnMediaFinished( event );

                    return;
                }
            }
        }

        ResetVumeterLevel();
        SavePlayedTrack();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFadeOutFinished( guMediaEvent &event )
{
    guLogDebug( wxT( "OnMediaFadeOutFinished (%li) Cur: %i  %li" ), event.GetExtraLong(), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFadeInStarted( guMediaEvent &event )
{
    guLogDebug( wxT( "OnMediaFadeInStarted Cur: %i  %li" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
}

// -------------------------------------------------------------------------------- //
const guMediaState guPlayerPanel::GetState( void )
{
    return m_MediaCtrl->GetState();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::CheckFiltersEnable( void )
{
    bool IsEnabled = ( m_PlayMode == guPLAYER_PLAYMODE_SMART ) ||
            ( ( m_PlayMode == guPLAYER_PLAYMODE_NONE ) && m_PlayRandom );
    m_PlayerFilters->EnableFilters( IsEnabled );
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::GetPlaySmart()
{
    return ( m_PlayMode == guPLAYER_PLAYMODE_SMART );
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetPlayMode()
{
    return m_PlayMode;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetPlayMode( int playmode )
{
    if( m_PlayMode != playmode )
    {
        m_PlayMode = playmode;

        PlayModeChanged();
    }
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::GetPlayLoop()
{
    return ( m_PlayMode >= guPLAYER_PLAYMODE_REPEAT_PLAYLIST );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPrevTrackButtonClick( wxCommandEvent& event )
{
    bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_PREVTRACK ) ||
                      ( event.GetEventObject() == m_PrevTrackButton );

    if( wxGetKeyState( WXK_SHIFT ) && m_MainFrame->IsActive() && ForceSkip )
    {
        OnPrevAlbumButtonClick( event );
        return;
    }
    guMediaState State;
//    wxFileOffset CurPos;
    guTrack * PrevItem;

    // If we are already in the first Item start again the song from the begining
    State = m_MediaCtrl->GetState();
    //CurPos = m_MediaCtrl->Tell();
    int CurItem = m_PlayListCtrl->GetCurItem();
    if( ( ( CurItem == 0 ) && ( State == guMEDIASTATE_PLAYING ) ) ||
        ( ( State != guMEDIASTATE_STOPPED ) && ( m_LastCurPos  > GUPLAYER_MIN_PREVTRACK_POS ) ) )
    {
        SetPosition( m_MediaSong.m_Offset );
        return;
    }

    PrevItem = m_PlayListCtrl->GetPrev( m_PlayMode, ForceSkip );
    if( PrevItem )
    {
        SetNextTrack( PrevItem );

        if( State == guMEDIASTATE_PLAYING )
        {
//            if( State == guMEDIASTATE_PLAYING )
//            {
            LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE :
                ( ForceSkip ? guFADERPLAYBIN_PLAYTYPE_REPLACE : guFADERPLAYBIN_PLAYTYPE_AFTER_EOS ), ForceSkip );
//            }
        }
        else
        {
            if( State == guMEDIASTATE_PAUSED )
            {
                m_MediaCtrl->Stop();
            }
            if( m_MediaSong.m_Loaded )
            {
                SavePlayedTrack();

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
            //guLogDebug( wxT( "Prev Track when not playing.." ) );
//            m_MediaCtrl->SetCurrentState( GST_STATE_READY );
        }
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
//    //event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnNextTrackButtonClick( wxCommandEvent& event )
{
    bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_NEXTTRACK ) ||
                      ( event.GetEventObject() == m_NextTrackButton );

    if( wxGetKeyState( WXK_SHIFT ) && m_MainFrame->IsActive() && ForceSkip )
    {
        OnNextAlbumButtonClick( event );
        return;
    }
    guLogDebug( wxT( "OnNextTrackButtonClick Cur: %i    %li   %i" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId, event.GetInt() );
    guMediaState State;
    guTrack * NextItem;

    NextItem = m_PlayListCtrl->GetNext( m_PlayMode, ForceSkip );
    if( NextItem )
    {
        State = m_MediaCtrl->GetState();
        guLogDebug( wxT( "OnNextTrackButtonClick : State = %i" ), State );

        SetNextTrack( NextItem );

        if( State == guMEDIASTATE_PLAYING || State == guMEDIASTATE_ERROR )
        {
            if( ( State != guMEDIASTATE_ERROR ) &&
                !ForceSkip &&
                !( !m_ForceGapless && m_FadeOutTime ) &&
                m_MediaSong.m_Offset && m_NextSong.m_Offset &&
                ( m_MediaSong.m_FileName == m_NextSong.m_FileName ) &&
                ( m_MediaSong.m_Number + 1 == m_NextSong.m_Number ) )
            {
                guLogDebug( wxT( "Simulate track change for track %i => %i" ), m_MediaSong.m_Number, m_NextSong.m_Number );
                m_NextTrackId = m_CurTrackId;
                OnMediaPlayStarted();
            }
            else
            {
                LoadMedia( ( event.GetInt() ? ( guFADERPLAYBIN_PLAYTYPE ) event.GetInt() :
                    ( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE :
                        ( ForceSkip ? guFADERPLAYBIN_PLAYTYPE_REPLACE : guFADERPLAYBIN_PLAYTYPE_AFTER_EOS ) ) ),
                        ForceSkip );
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
                SavePlayedTrack();

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
            //guLogDebug( wxT( "Next Track when not playing.." ) );
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
            if( GetPlayLoop() )
                SetPlayMode( guPLAYER_PLAYMODE_NONE );

            //guLogDebug( wxT( "Getting Random Tracks..." ) );
            guTrackArray Tracks;
            if( m_Db->GetRandomTracks( &Tracks, m_SmartPlayAddTracks, m_PlayRandomMode,
                    m_PlayerFilters->GetAllowFilterId(),
                    m_PlayerFilters->GetDenyFilterId() ) )
            {
                AddToPlayList( Tracks, false );

                OnNextTrackButtonClick( event );
            }
        }
        else
        {
            if( m_MediaSong.m_Offset )
            {
                OnStopButtonClick( event );
            }
            else
            {
                SavePlayedTrack();
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnNextAlbumButtonClick( wxCommandEvent& event )
{
    guLogDebug( wxT( "OnNextAlbumButtonClick Cur: %i    %li" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
    guMediaState State;

    guTrack * NextAlbumTrack = m_PlayListCtrl->GetNextAlbum( m_PlayMode, true );
    if( NextAlbumTrack )
    {
        State = m_MediaCtrl->GetState();

        SetNextTrack( NextAlbumTrack );

        if( State == guMEDIASTATE_PLAYING )
        {
            bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_NEXTTRACK ) ||
                              ( event.GetEventObject() == m_NextTrackButton );
            LoadMedia( ( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE ),
                       ForceSkip );
        }
        //else
        //{
            //guLogDebug( wxT( "Next ALbum Track when not playing.." ) );
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
            if( GetPlayLoop() )
                SetPlayMode( guPLAYER_PLAYMODE_NONE );

            //guLogDebug( wxT( "Getting Random Tracks..." ) );
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
    //guLogDebug( wxT( "OnPrevAlbumButtonClick Cur: %i    %li" ), m_PlayListCtrl->GetCurItem(), m_NextTrackId );
    guMediaState State;

    guTrack * NextAlbumTrack = m_PlayListCtrl->GetPrevAlbum( m_PlayMode, true );
    if( NextAlbumTrack )
    {
        State = m_MediaCtrl->GetState();

        SetNextTrack( NextAlbumTrack );

        if( State == guMEDIASTATE_PLAYING )
        {
            bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_PREVTRACK ) ||
                              ( event.GetEventObject() == m_PrevTrackButton );
            LoadMedia( ( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE ),
                        ForceSkip );
        }
        //else
        //{
            //guLogDebug( wxT( "Prev Album Track when not playing.." ) );
//            m_MediaCtrl->SetCurrentState( GST_STATE_READY );
        //}

        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayButtonClick( wxCommandEvent& event )
{
    guLogDebug( wxT( "OnPlayButtonClick Cur: %i %i %i" ), m_PlayListCtrl->GetCurItem(), m_MediaSong.m_Loaded, m_PlayListCtrl->GetItemCount() );
    guMediaState State;

    //if( m_PendingNewRecordName )
    //    m_PendingNewRecordName = false;

    // Get The Current Song From m_PlayListCtrl
    //guTrack * CurItem = m_PlayListCtrl->GetCurrent();
    //if( !m_MediaSong.m_SongId && m_PlayListCtrl->GetItemCount() )
    if( !m_MediaSong.m_Loaded && m_PlayListCtrl->GetCount() )
    {
        if( m_PlayListCtrl->GetCurItem() == wxNOT_FOUND )
            m_PlayListCtrl->SetCurrent( 0, m_DelTracksPlayed && !GetPlayLoop() );
        //m_MediaSong = * m_PlayListCtrl->GetCurrent();
        SetNextTrack( m_PlayListCtrl->GetCurrent() );

        LoadMedia( ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_REPLACE );
        return;
    }

    if( m_MediaSong.m_Loaded )
    {
        //State = m_MediaCtrl->GetState();
        State = GetState();
        guLogDebug( wxT( ">>>> PlayButtonClick State: %i" ), State );
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
            //guLogDebug( wxT( "Loading '%s'" ), m_NextSong.m_FileName.c_str() );
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
            if( GetPlayLoop() )
                SetPlayMode( guPLAYER_PLAYMODE_NONE );

            //guLogDebug( wxT( "Getting Random Tracks..." ) );
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
    //guLogDebug( wxT( "OnStopButtonClick Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    if( wxGetKeyState( WXK_SHIFT ) )
    {
        OnStopAtEnd( event );
        return;
    }

    //guMediaState State;
    //State = m_MediaCtrl->GetState();
    //guLogDebug( wxT( "State: %i" ), State );
    //if( State != guMEDIASTATE_STOPPED )
    //{
    m_MediaCtrl->Stop();
        //UpdatePositionLabel( 0 );
//        if( m_MediaSong.m_Length )
//    m_PlayerPositionSlider->SetValue( 0 );
//    ResetVumeterLevel();
    //}
    SavePlayedTrack( true );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnStopAtEnd( wxCommandEvent &event )
{
    //m_MediaSong.m_Type = guTrackType( int( m_MediaSong.m_Type ) ^ guTRACK_TYPE_STOP_HERE );
    m_PlayListCtrl->StopAtEnd();
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
void guPlayerPanel::OnRandomPlayButtonClick( wxCommandEvent &event )
{
    m_PlayListCtrl->Randomize( ( GetState() == guMEDIASTATE_PLAYING ) );
    wxCommandEvent evt( wxEVT_MENU, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, evt );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayModeButtonClicked( wxCommandEvent &event )
{
    m_PlayMode++;
    if( m_PlayMode > guPLAYER_PLAYMODE_REPEAT_TRACK )
        m_PlayMode = guPLAYER_PLAYMODE_NONE;

    PlayModeChanged();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::PlayModeChanged()
{
    CheckFiltersEnable();
    UpdatePlayModeButton();

    // Send Notification for the mpris interface
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_PLAYERPANEL_STATUSCHANGED );
    wxPostEvent( m_MainFrame, CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdatePlayModeButton()
{
    guIMAGE_INDEX NormalImage;
    guIMAGE_INDEX HighlightImage;
    wxString Tooltip;
    switch( m_PlayMode )
    {
        case guPLAYER_PLAYMODE_SMART :
            NormalImage = guIMAGE_INDEX_player_normal_smart;
            HighlightImage = guIMAGE_INDEX_player_highlight_smart;
            Tooltip = _( "Smart mode enabled" );
            break;

        case guPLAYER_PLAYMODE_REPEAT_PLAYLIST :
            NormalImage = guIMAGE_INDEX_player_normal_repeat;
            HighlightImage = guIMAGE_INDEX_player_highlight_repeat;
            Tooltip = _( "Repeat all" );
            break;
        case guPLAYER_PLAYMODE_REPEAT_TRACK :
            NormalImage = guIMAGE_INDEX_player_normal_repeat_single;
            HighlightImage = guIMAGE_INDEX_player_highlight_repeat_single;
            Tooltip = _( "Repeat one" );
            break;

        //case guPLAYER_PLAYMODE_NONE :
        default :
            NormalImage = guIMAGE_INDEX_player_light_smart;
            HighlightImage = guIMAGE_INDEX_player_light_smart;
            Tooltip = _( "Smart mode disabled" );
            break;
    }

    m_PlayModeButton->SetBitmapLabel( guImage( NormalImage ) );
    m_PlayModeButton->SetBitmapHover( guImage( HighlightImage ) );
    m_PlayModeButton->SetToolTip( Tooltip );
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
void guPlayerPanel::OnLeftClickPlayerCoverBitmap( wxMouseEvent &event )
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
        BigCover->SetBitmap( m_MediaSong.m_CoverType, m_MediaSong.m_CoverPath );
        BigCover->Show();
        BigCover->SetFocus();
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
        UpdatePositionLabel( CurPos );
        //printf( "Slider Tracking %d\n", CurPos );

        //m_MediaCtrl->Seek( CurPos * m_MediaSong.Length );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderEndSeek( wxScrollEvent &event )
{
//    wxFileOffset NewPos;
//    if( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
//    {
//        NewPos = event.GetPosition();
//        guLogDebug( wxT( "Slider EndSeek Set Pos to %i Of %i" ), ( int ) NewPos, 1000 );
//        //printf( "SetPos: %llu\n", ( long long int ) NewPos * m_MediaSong.Length );
//        //m_MediaCtrl->Seek( NewPos * m_MediaSong.m_Length );
//        SetPosition( NewPos * m_MediaSong.m_Length );
//    }
    m_SliderIsDragged = false;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderChanged( wxScrollEvent &event )
{
    wxFileOffset NewPos;
    if( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
    {
        NewPos = event.GetPosition();
        //guLogDebug( wxT( "Slider Changed Set Pos to %i Of %i" ), ( int ) NewPos, 1000 );
        SetPosition( m_MediaSong.m_Offset + ( NewPos * ( m_LastLength / 1000 ) ) );
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
        //guLogDebug( wxT( "Pos : %i -> %i" ), GetPosition(), GetPosition() + ( Rotation * 7000 ) );
        SetPosition( m_MediaSong.m_Offset + wxMax( 0, wxMin( ( int ) m_LastLength, GetPosition() + ( Rotation * 7000 ) ) ) );
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
    m_VolumeBar->SetValue( m_CurVolume );

    wxCommandEvent evt( wxEVT_MENU, ID_PLAYERPANEL_VOLUMECHANGED );
    wxPostEvent( m_MainFrame, evt );
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::SetPosition( int pos )
{
    bool Result = m_MediaCtrl->Seek( pos );
    if( Result )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_PLAYERPANEL_SEEKED );
        evt.SetInt( pos );
        wxPostEvent( m_MainFrame, evt );
    }
    return Result;
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetPosition()
{
    return m_LastCurPos; //m_MediaCtrl->Tell();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnTitleNameDClicked( wxMouseEvent &event )
{
    if( ( m_MediaSong.m_MediaViewer && m_MediaSong.m_SongId ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_TRACK );
        evt.SetInt( m_MediaSong.m_SongId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        evt.SetClientData( ( void * ) m_MediaSong.m_MediaViewer );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnAlbumNameDClicked( wxMouseEvent &event )
{
    if( ( m_MediaSong.m_MediaViewer && m_MediaSong.m_SongId ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ALBUM );
        evt.SetInt( m_MediaSong.m_AlbumId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        evt.SetClientData( ( void * ) m_MediaSong.m_MediaViewer );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnArtistNameDClicked( wxMouseEvent &event )
{
    if( ( m_MediaSong.m_MediaViewer && m_MediaSong.m_SongId ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( m_MediaSong.m_ArtistId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        evt.SetClientData( ( void * ) m_MediaSong.m_MediaViewer );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnYearDClicked( wxMouseEvent &event )
{
    if( m_MediaSong.m_MediaViewer && m_MediaSong.m_Year )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_YEAR );
        evt.SetInt( m_MediaSong.m_Year );
        evt.SetClientData( ( void * ) m_MediaSong.m_MediaViewer );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRatingChanged( guRatingEvent &event )
{
    m_MediaSong.m_Rating = event.GetInt();

    //if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
    if( m_MediaSong.m_MediaViewer )
    {
        if( !m_MediaSong.m_Offset && m_MediaSong.m_MediaViewer->GetEmbeddMetadata() )
        {
            guTrackArray Tracks;
            Tracks.Add( m_MediaSong );
            guImagePtrArray Images;
            wxArrayString Lyrics;
            wxArrayInt ChangedFlags;
            ChangedFlags.Add( guTRACK_CHANGED_DATA_RATING );
            guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
        }

        guDbLibrary * Db = m_MediaSong.m_MediaViewer->GetDb();
        Db->SetTrackRating( m_MediaSong.m_SongId, m_MediaSong.m_Rating );
    }
    else if( ( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION ) ||
             ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        m_MediaSong.m_Rating = -1;
        m_Rating->SetRating( -1 );
    }

    // Update the track in database, playlist, etc
    m_MainFrame->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdatedTracks( const guTrackArray * tracks )
{
    int index;
    int count = tracks->Count();
    for( index = 0; index < count; index++ )
    {
        if( ( tracks->Item( index ).m_FileName == m_MediaSong.m_FileName ) &&
            ( tracks->Item( index ).m_Offset == m_MediaSong.m_Offset ) )
        {
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
    guLogDebug( wxT( "UpdateLabels..." ) );
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

    SetFormatLabel( m_MediaSong.m_Format );
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
            //Body.Append( wxT( "★" ), m_MediaSong.m_Rating );
            // There is a wxWidgets bug that dont append the count characters but only one allways
            for( int Index = 0; Index < m_MediaSong.m_Rating; Index++ )
            {
                Body += wxT( "★" );
            }
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
void guPlayerPanel::SetForceGapless( const bool forcegapless )
{
    if( m_ForceGapless != forcegapless )
    {
        wxCommandEvent event;
        OnForceGaplessClick( event );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnVolumeMouseWheel( wxMouseEvent &event )
{
    int Rotation = ( event.GetWheelRotation() / event.GetWheelDelta() ) * ( event.ShiftDown() ? 1 : 4 );
    //guLogDebug( wxT( "CurVol: %u  Rotations:%i" ), m_CurVolume, Rotation );
    SetVolume( m_CurVolume + Rotation );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnVolumeChanged( wxScrollEvent &event )
{
    SetVolume( event.GetPosition() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnVolumeClicked( wxCommandEvent &event )
{
    if( m_VolumeBar->IsShown() )
        m_VolumeBar->Hide();
    else
        m_VolumeBar->Show();
    Layout();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnAddTracks( wxCommandEvent &event )
{
    //bool PlayTracks = event.GetInt();
    wxArrayString * TrackList = ( wxArrayString * ) event.GetClientData();

    if( TrackList )
    {
        AddToPlayList( * TrackList, true, guINSERT_AFTER_CURRENT_NONE );
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
    OnPlayModeButtonClicked( event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnLoop( wxCommandEvent &event )
{
    SetPlayMode( event.GetInt() );
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
void guPlayerPanel::OnForceGaplessClick( wxCommandEvent &event )
{
    guLogMessage( wxT( "ForceGapless clicked...." ) );
    m_ForceGapless = !m_ForceGapless;
    m_MediaCtrl->ForceGapless( m_ForceGapless );

    m_ForceGaplessButton->SetBitmapLabel( guImage( m_ForceGapless ? guIMAGE_INDEX_player_normal_gapless : guIMAGE_INDEX_player_normal_crossfading ) );
    m_ForceGaplessButton->SetBitmapHover( guImage( m_ForceGapless ? guIMAGE_INDEX_player_highlight_gapless : guIMAGE_INDEX_player_highlight_crossfading ) );
    m_ForceGaplessButton->SetToolTip( m_ForceGapless ? _( "Enable crossfading" ) : _( "Disable crossfading" ) );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::MediaViewerClosed( guMediaViewer * mediaviewer )
{
    if( m_MediaSong.m_MediaViewer == mediaviewer )
    {
        m_MediaSong.m_MediaViewer = NULL;
    }

    if( m_NextSong.m_MediaViewer == mediaviewer )
    {
        m_NextSong.m_MediaViewer = NULL;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::CheckStartPlaying( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    guLogDebug( wxT( "CheckStartPlaying: %i" ), m_PlayListCtrl->StartPlaying() );
    // There was a track passed as argument that we will play
    if( m_PlayListCtrl->StartPlaying() )
    {
        wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_PLAY );
        OnPlayButtonClick( event );
    }
    else
    {
        if( Config->ReadBool( CONFIG_KEY_GENERAL_SAVE_CURRENT_TRACK_POSITION, false, CONFIG_PATH_GENERAL ) )
        {
            m_TrackStartPos = wxMax( 0, Config->ReadNum( CONFIG_KEY_GENERAL_CURRENT_TRACK_POS, 0, CONFIG_PATH_GENERAL ) - 500 );
            if( m_TrackStartPos > 0 )
            {
                wxCommandEvent event;
                event.SetInt( Config->ReadNum( CONFIG_KEY_PLAYLIST_CURITEM, 0, CONFIG_PATH_PLAYLIST_NOWPLAYING ) );
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
}


// -------------------------------------------------------------------------------- //
// guUpdatePlayerCoverThread
// -------------------------------------------------------------------------------- //
guUpdatePlayerCoverThread::guUpdatePlayerCoverThread( guDbLibrary * db,
    guMainFrame * mainframe, guPlayerPanel * playerpanel, guCurrentTrack * currenttrack,
    const bool shownotify, const bool deleted ) : wxThread()
{
    m_Db = db;
    m_MainFrame = mainframe;
    m_PlayerPanel = playerpanel;
    m_CurrentTrack = currenttrack;
    m_ShowNotify = shownotify;
    m_Deleted = deleted;

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

    if( m_Deleted )
    {
        m_CurrentTrack->m_CoverPath = wxEmptyString;
        // As it was deleted we dont try to get a new one...
    }
    else if( m_CurrentTrack->m_Type == guTRACK_TYPE_RADIOSTATION )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_net_radio ) );
        m_CurrentTrack->m_CoverType = GU_SONGCOVER_RADIO;
    }
    else if( m_CurrentTrack->m_Type == guTRACK_TYPE_PODCAST )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_podcast ) );
        m_CurrentTrack->m_CoverType = GU_SONGCOVER_PODCAST;
    }
    else if( m_CurrentTrack->m_MediaViewer )
    {
        guMediaViewer * MediaViewer = m_CurrentTrack->m_MediaViewer;

        CoverImage = MediaViewer->GetAlbumCover( m_CurrentTrack->m_AlbumId, m_CurrentTrack->m_CoverId,
                            m_CurrentTrack->m_CoverPath, m_CurrentTrack->m_ArtistName, m_CurrentTrack->m_AlbumName );
        m_CurrentTrack->m_CoverType = GU_SONGCOVER_FILE;
    }

    if( !CoverImage )
    {
        if( ( CoverImage = guTagGetPicture( m_CurrentTrack->m_FileName ) ) )
        {
            m_CurrentTrack->m_CoverType = GU_SONGCOVER_ID3TAG;
            m_CurrentTrack->m_CoverPath = m_CurrentTrack->m_FileName;
        }
        else
        {
            //guLogWarning( wxT( "Trying to find covers in %s" ), wxPathOnly( m_CurrentTrack->m_FileName ).c_str() );
            m_CurrentTrack->m_CoverPath = m_PlayerPanel->PlayListCtrl()->FindCoverFile( wxPathOnly( m_CurrentTrack->m_FileName ) );
        }
    }

    if( TestDestroy() )
    {
        if( CoverImage )
            delete CoverImage;
        return 0;
    }


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
    {
        if( CoverImage )
            delete CoverImage;
        return 0;
    }

    CoverImage->Rescale( 100, 100, wxIMAGE_QUALITY_HIGH );

    //guLogMessage( wxT( "   File : %s" ), m_CurrentTrack->m_FileName.c_str() );
    //guLogMessage( wxT( " Loaded : %i" ), m_CurrentTrack->m_Loaded );
    //guLogMessage( wxT( "   Type : %i" ), m_CurrentTrack->m_Type );
    //guLogMessage( wxT( " SongId : %i" ), m_CurrentTrack->m_SongId );
    //guLogMessage( wxT( "CoverId : %i" ), m_CurrentTrack->m_CoverId );
    //guLogMessage( wxT( "Co.Type : %i" ), m_CurrentTrack->m_CoverType );
    //guLogMessage( wxT( "  Cover : '%s'" ), m_CurrentTrack->m_CoverPath.c_str() );
    //guLogMessage( wxT( "  Width : %u" ), CoverImage->GetWidth() );
    //guLogMessage( wxT( " Height : %u" ), CoverImage->GetHeight() );
    //guLogMessage( wxT( "===========================================" ) );

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

            //guLogDebug( wxT( "Saving temp cover file to '%s'" ), LastTmpCoverFile.c_str() );
            if( CoverImage->SaveFile( LastTmpCoverFile, wxBITMAP_TYPE_PNG ) )
            {
                if( m_CurrentTrack->m_CoverPath.IsEmpty() )
                    m_CurrentTrack->m_CoverPath = LastTmpCoverFile;

                m_PlayerPanel->SetLastTmpCoverFile( LastTmpCoverFile );
            }
        }
    }

    if( TestDestroy() )
    {
        if( CoverImage )
            delete CoverImage;
        return 0;
    }

    if( m_CurrentTrack->m_CoverImage )
        delete m_CurrentTrack->m_CoverImage;
    m_CurrentTrack->m_CoverImage = CoverImage;

    wxCommandEvent Event( wxEVT_MENU, ID_PLAYERPANEL_COVERUPDATED );
    Event.SetInt( m_ShowNotify );
    wxPostEvent( m_PlayerPanel, Event );

    return 0;
}

}

// -------------------------------------------------------------------------------- //
