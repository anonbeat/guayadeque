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
#include "PlayerPanel.h"

#include "Commands.h"
#include "CoverFrame.h"
#include "Config.h"
#include "DbLibrary.h"
#include "Equalizer.h"
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


guLevelInfo LastLevelInfo;

// -------------------------------------------------------------------------------- //
guPlayerPanel::guPlayerPanel( wxWindow * parent, guDbLibrary * db,
    guPlayList * playlist, guPlayerFilters * filters )
       : wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 310, 170 ), wxTAB_TRAVERSAL )
{
    double SavedVol = 50.0;

    m_Db = db;
    m_PlayListCtrl = playlist;
    m_NotifySrv = NULL;
    m_PlayerFilters = filters;
    m_BufferGaugeId = wxNOT_FOUND;
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

    m_LastCurPos = -1;
    m_LastPlayState = wxMEDIASTATE_STOPPED;
    m_LastTotalLen = -1;

    m_AboutToFinishPending = false;

    wxArrayInt Equalizer;

    m_SilenceDetector = false;
    m_SilenceDetectorLevel = wxNOT_FOUND;
    m_SilenceDetectorTime = 0;

    m_ShowRevTime = false;
    m_PlayRandom = false;
    m_PlayRandomMode = 0;
    m_DelTracksPlayed = false;
    m_PendingScrob = false;
    m_IsSkipping = false;
    m_ShowNotifications = true;
    m_ShowNotificationsTime = 0;

    // Load configuration
    Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->RegisterObject( this );

        //guLogMessage( wxT( "Reading PlayerPanel Config" ) );
        SavedVol = Config->ReadNum( wxT( "PlayerCurVol" ), 50, wxT( "General" ) );
        //guLogMessage( wxT( "Current Volume Var : %d" ), ( int ) m_CurVolume );
        m_PlayLoop = Config->ReadNum( wxT( "PlayerLoop" ), 0, wxT( "General" )  );
        m_PlaySmart = Config->ReadBool( wxT( "PlayerSmart" ), m_PlayLoop ? false : true, wxT( "General" )  );
        m_PlayRandom = Config->ReadBool( wxT( "RndPlayOnEmptyPlayList" ), false, wxT( "General" ) );
        m_PlayRandomMode = Config->ReadNum( wxT( "RndModeOnEmptyPlayList" ), 0, wxT( "General" ) );
        m_ShowNotifications = Config->ReadBool( wxT( "ShowNotifications" ), true, wxT( "General" ) );
        m_ShowNotificationsTime = Config->ReadNum( wxT( "NotificationsTime" ), 0, wxT( "General" ) );

        m_SmartPlayAddTracks = Config->ReadNum( wxT( "NumTracksToAdd" ), 3, wxT( "Playback" ) );
        m_SmartPlayMinTracksToPlay = Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) );
        m_DelTracksPlayed = Config->ReadBool( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) );
        m_AudioScrobbleEnabled = Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) );
        Equalizer = Config->ReadANum( wxT( "Band" ), 0, wxT( "Equalizer" ) );

        m_SilenceDetector = Config->ReadBool( wxT( "SilenceDetector" ), false, wxT( "Playback" ) );
        m_SilenceDetectorLevel = Config->ReadNum( wxT( "SilenceLevel" ), -55, wxT( "Playback" ) );
        if( Config->ReadBool( wxT( "SilenceAtEnd" ), false, wxT( "Playback" ) ) )
        {
            m_SilenceDetectorTime = Config->ReadNum( wxT( "SilenceEndTime" ), 45, wxT( "Playback" ) ) * 1000;
        }

        m_ShowRevTime = Config->ReadBool( wxT( "ShowRevTime" ), false, wxT( "General" ) );
//        m_ShowFiltersChoices = Config->ReadBool( wxT( "ShowFiltersChoices" ), true, wxT( "Positions" ) );
    }

    m_SliderIsDragged = false;
    m_SmartSearchEnabled = false;
    m_SmartAddTracksThread = NULL;

    // ---------------------------------------------------------------------------- //
    // The player controls
    // ---------------------------------------------------------------------------- //
//    wxPanel * PlayerPanel;
//	PlayerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxBoxSizer * PlayerMainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * PlayerBtnSizer = new wxBoxSizer( wxHORIZONTAL );

	//m_PrevTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_prev ), wxDefaultPosition, wxDefaultSize, 0 ); //wxBU_AUTODRAW );
	m_PrevTrackButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_prev ), guImage( guIMAGE_INDEX_player_highlight_prev ), 0 );
	m_PrevTrackButton->SetToolTip( _( "Go to Previous Track in the Playlist" ) );
	PlayerBtnSizer->Add( m_PrevTrackButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_PlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_play ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_play ), guImage( guIMAGE_INDEX_player_highlight_play ), 0 );
	m_PlayButton->SetToolTip( _( "Start playing or pauses current track in the Playlist" ) );
	PlayerBtnSizer->Add( m_PlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_NextTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_next ), wxDefaultPosition, wxDefaultSize, 0 );
	m_NextTrackButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_next ), guImage( guIMAGE_INDEX_player_highlight_next ), 0 );
	m_NextTrackButton->SetToolTip( _( "Go to Next Track in the Playlist" ) );
	PlayerBtnSizer->Add( m_NextTrackButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_StopButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_stop ), wxDefaultPosition, wxDefaultSize, 0 );
	m_StopButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_stop ), guImage( guIMAGE_INDEX_player_highlight_stop ), 0 );
	m_StopButton->SetToolTip( _( "Stops player reproduction" ) );
	PlayerBtnSizer->Add( m_StopButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM|wxRIGHT, guPLAYER_ICONS_SEPARATOR );

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
	m_SmartPlayButton->SetToolTip( _( "Add tracks to the playlist based on LastFM" ) );
	// Get PlayerPanel value from config file
	m_SmartPlayButton->SetValue( m_PlaySmart );
	PlayerBtnSizer->Add( m_SmartPlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	//m_RandomPlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_random ), wxDefaultPosition, wxDefaultSize, 0 );
	m_RandomPlayButton = new guRoundButton( this, guImage( guIMAGE_INDEX_player_normal_random ), guImage( guIMAGE_INDEX_player_highlight_random ), 0 );
	m_RandomPlayButton->SetToolTip( _( "Randomize the tracks in the playlist" ) );
	PlayerBtnSizer->Add( m_RandomPlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	m_RepeatPlayButton = new guToggleRoundButton( this, guImage( guIMAGE_INDEX_player_light_repeat ), guImage( guIMAGE_INDEX_player_normal_repeat ), guImage( guIMAGE_INDEX_player_highlight_repeat ) );
	m_RepeatPlayButton->SetToolTip( _( "Select the repeat mode" ) );
	m_RepeatPlayButton->SetValue( m_PlayLoop );
	PlayerBtnSizer->Add( m_RepeatPlayButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, guPLAYER_ICONS_SEPARATOR );

	PlayerMainSizer->Add( PlayerBtnSizer, 0, wxEXPAND, 2 );


	wxBoxSizer* PlayerDetailsSizer;
	PlayerDetailsSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayerCoverBitmap = new guStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_no_cover ), wxDefaultPosition, wxSize( 100,100 ), 0 );
	m_PlayerCoverBitmap->SetToolTip( _( "Shows the current track album cover if available" ) );
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
	m_BitRateSizer->Add( m_Rating, 0, wxRIGHT, 2 );

	m_BitRateSizer->Add( 0, 0, 1, wxALL, 5 );

	m_BitRateLabel = new wxStaticText( this, wxID_ANY, wxT( "[kbps]" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_BitRateLabel->SetToolTip( _( "Show the bitrate of the current track" ) );
	CurrentFont.SetPointSize( 8 );
	m_BitRateLabel->SetFont( CurrentFont );

    m_BitRateSizer->Add( m_BitRateLabel, 0, wxEXPAND|wxRIGHT|wxLEFT, 2 );

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
    //m_MediaCtrl->Create( this, wxID_ANY );

    //
    m_PlayListCtrl->ReloadItems();
    TrackListChanged();
    // The SetVolume call dont get set if the volume is the last one
    // so we do it two calls

    m_CurVolume = ( m_MediaCtrl->GetVolume() * 100.0 );
    SetVolume( SavedVol );
    //guLogMessage( wxT( "CurVol: %i SavedVol: %i" ), int( m_MediaCtrl->GetVolume() * 100.0 ), ( int ) m_CurVolume );

    if( Equalizer.Count() == guEQUALIZER_BAND_COUNT )
    {
        m_MediaCtrl->SetEqualizer( Equalizer );
    }

    // There was a track passed as argument that we will play
    if( m_PlayListCtrl->StartPlaying() )
    {
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
        //wxTheApp->GetTopWindow()->AddPendingEvent( event );
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
                SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
            }
        }
        else if( m_PlayListCtrl->GetCurItem() != wxNOT_FOUND )
        {
            SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
        }
    }

    CheckFiltersEnable();

	// Connect Events
	m_PrevTrackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPrevTrackButtonClick ), NULL, this );
	m_NextTrackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnNextTrackButtonClick ), NULL, this );
	m_PlayButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayButtonClick ), NULL, this );
 	m_StopButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnStopButtonClick ), NULL, this );
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
	m_ArtistLabel->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnArtistNameDClicked ), NULL, this );

	m_PositionLabel->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( guPlayerPanel::OnTimeDClicked ), NULL, this );
	m_Rating->Connect( guEVT_RATING_CHANGED, guRatingEventHandler( guPlayerPanel::OnRatingChanged ), NULL, this );

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

    m_MediaCtrl->Connect( wxEVT_MEDIA_LOADED, wxMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_ABOUT_TO_FINISH, wxMediaEventHandler( guPlayerPanel::OnMediaAboutToFinish ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_FINISHED, wxMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_TAG, wxMediaEventHandler( guPlayerPanel::OnMediaTag ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_BITRATE, wxMediaEventHandler( guPlayerPanel::OnMediaBitrate ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_BUFFERING, wxMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_LEVEL, wxMediaEventHandler( guPlayerPanel::OnMediaLevel ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_ERROR, wxMediaEventHandler( guPlayerPanel::OnMediaError ), NULL, this );

    Connect( ID_PLAYER_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracks ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPlayerPanel::OnConfigUpdated ), NULL, this );

    m_PlayerTimer = new guPlayerPanelTimer( this );
    m_PlayerTimer->Start( 400 );

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

    if( m_AudioScrobble )
        delete m_AudioScrobble;

    if( m_PlayerTimer )
        delete m_PlayerTimer;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->UnRegisterObject( this );

        //printf( "guPlayerPanel::guConfig Save\n" );
        Config->WriteBool( wxT( "PlayerStopped" ), m_MediaCtrl->GetState() != wxMEDIASTATE_PLAYING, wxT( "General" ) );
        Config->WriteNum( wxT( "PlayerCurVol" ), m_CurVolume, wxT( "General" ) );
        Config->WriteNum( wxT( "PlayerLoop" ), m_PlayLoop, wxT( "General" ) );
        Config->WriteBool( wxT( "PlayerSmart" ), m_PlaySmart, wxT( "General" ) );
        // If the track length is at least the configured minimun track length save the pos offset
        if( Config->ReadBool( wxT( "SaveCurrentTrackPos" ), false, wxT( "General" ) ) )
        {
            if( ( m_LastPlayState != wxMEDIASTATE_STOPPED ) &&
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

	// Connect Events
	m_PrevTrackButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPrevTrackButtonClick ), NULL, this );
	m_NextTrackButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnNextTrackButtonClick ), NULL, this );
	m_PlayButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayButtonClick ), NULL, this );
	m_StopButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnStopButtonClick ), NULL, this );
	m_VolumeButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnVolumenButtonClick ), NULL, this );
	m_SmartPlayButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnSmartPlayButtonClick ), NULL, this );
	m_RandomPlayButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ), NULL, this );
	m_RepeatPlayButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRepeatPlayButtonClick ), NULL, this );
	m_EqualizerButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnEqualizerButtonClicked ), NULL, this );

    Disconnect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ) );

	m_TitleLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnTitleNameDClicked ), NULL, this );
	m_AlbumLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnAlbumNameDClicked ), NULL, this );
	m_ArtistLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnArtistNameDClicked ), NULL, this );
	m_PositionLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnTimeDClicked ), NULL, this );
	m_Rating->Disconnect( guEVT_RATING_CHANGED, guRatingEventHandler( guPlayerPanel::OnRatingChanged ), NULL, this );

    //
	//m_PlayerCoverBitmap->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnLeftDClickPlayerCoverBitmap ), NULL, this );
    Disconnect( guEVT_STATICBITMAP_MOUSE_OVER, guStaticBitmapMouseOverEvent, wxCommandEventHandler( guPlayerPanel::OnPlayerCoverBitmapMouseOver ), NULL, this );

	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderBeginSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
//	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
    m_PlayerPositionSlider->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guPlayerPanel::OnPlayerPositionSliderMouseWheel ), NULL, this );

    //m_PlayListCtrl->Disconnect( ID_PLAYER_PLAYLIST_UPDATELIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnPlayListUpdated ), NULL, this );
    //m_PlayListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayListDClick ), NULL, this );
    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPlayerPanel::OnConfigUpdated ), NULL, this );

    m_MediaCtrl->Disconnect( wxEVT_MEDIA_LOADED, wxMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_FINISHED, wxMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_TAG, wxMediaEventHandler( guPlayerPanel::OnMediaTag ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_BITRATE, wxMediaEventHandler( guPlayerPanel::OnMediaBitrate ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_BUFFERING, wxMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_LEVEL, wxMediaEventHandler( guPlayerPanel::OnMediaLevel ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_ERROR, wxMediaEventHandler( guPlayerPanel::OnMediaError ), NULL, this );

    Disconnect( ID_PLAYER_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracks ) );

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
        m_PlayRandomMode = Config->ReadNum( wxT( "RndModeOnEmptyPlayList" ), 0, wxT( "General" ) );
        m_ShowNotifications = Config->ReadBool( wxT( "ShowNotifications" ), true, wxT( "General" ) );
        m_ShowNotificationsTime = Config->ReadNum( wxT( "NotificationsTime" ), 0, wxT( "General" ) );

        m_SmartPlayAddTracks = Config->ReadNum( wxT( "NumTracksToAdd" ), 3, wxT( "Playback" ) );
        m_SmartPlayMinTracksToPlay = Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "Playback" ) );
        m_DelTracksPlayed = Config->ReadBool( wxT( "DelTracksPlayed" ), false, wxT( "Playback" ) );
        m_AudioScrobbleEnabled = Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) );

        m_SilenceDetector = Config->ReadBool( wxT( "SilenceDetector" ), false, wxT( "Playback" ) );
        m_SilenceDetectorLevel = Config->ReadNum( wxT( "SilenceLevel" ), -55, wxT( "Playback" ) );
        if( Config->ReadBool( wxT( "SilenceAtEnd" ), false, wxT( "Playback" ) ) )
        {
            m_SilenceDetectorTime = Config->ReadNum( wxT( "SilenceEndTime" ), 45, wxT( "Playback" ) ) * 1000;
        }

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
            wxTheApp->GetTopWindow()->AddPendingEvent( event );
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
    m_MediaSong.m_Rating = rating;
    m_Rating->SetRating( rating );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdatePositionLabel( const unsigned int curpos )
{
    wxString Label;
    if( !m_ShowRevTime || !m_MediaSong.m_Length )
    {
        Label = LenToString( curpos );
    }
    else if( m_MediaSong.m_Length )
    {
        Label = wxT( "-" ) + LenToString( m_MediaSong.m_Length - curpos );
    }

    if( m_MediaSong.m_Length )
    {
        Label += wxT( " / " ) + LenToString( m_MediaSong.m_Length );
    }

    m_PositionLabel->SetLabel( Label );

    m_PosLabelSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetBitRateLabel( int bitrate )
{
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
        if( !m_MediaSong.m_Bitrate )
        {
            m_MediaSong.m_Bitrate = bitrate;

            if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
                m_Db->UpdateTrackBitRate( m_MediaSong.m_SongId, bitrate );

            // Update the track in database, playlist, etc
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
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
    //m_MediaSong = * m_PlayListCtrl->GetCurrent();
    SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
    //m_MediaSong.SongId = 0;

    OnStopButtonClick( event );
    OnPlayButtonClick( event );
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
        if( Count > guPLAYER_SMART_CACHEITEMS )
            Index = Count - guPLAYER_SMART_CACHEITEMS;
        for( ; Index < Count; Index++ )
        {
            m_SmartAddedTracks.Add( SongList[ Index ].m_SongId );
        }

        Index = 0;
        if( Count > guPLAYER_SMART_CACHEARTISTS )
            Index = Count - guPLAYER_SMART_CACHEARTISTS;
        for( ; Index < Count; Index++ )
        {
            m_SmartAddedArtists.Add( SongList[ Index ].m_ArtistName.Upper() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const wxString &FileName )
{
    int PrevTrackCount = m_PlayListCtrl->GetCount();

    m_PlayListCtrl->AddPlayListItem( FileName );
    m_PlayListCtrl->ReloadItems();
    TrackListChanged();
    // Add the added track to the smart cache
    if( m_PlaySmart )
    {
        int Count = m_PlayListCtrl->GetCount();

        // TODO : Check if the track was really added or not
        if( Count )
        {
            guTrack * Track = m_PlayListCtrl->GetItem( Count - 1 );

            m_SmartAddedTracks.Add( Track->m_SongId );
            m_SmartAddedArtists.Add( Track->m_ArtistName.Upper() );

            if( m_SmartAddedTracks.Count() > guPLAYER_SMART_CACHEITEMS )
                m_SmartAddedTracks.RemoveAt( 0 );

            if( m_SmartAddedArtists.Count() > guPLAYER_SMART_CACHEARTISTS )
                m_SmartAddedArtists.RemoveAt( 0 );
        }
    }

    if( !PrevTrackCount && ( GetState() != wxMEDIASTATE_PLAYING ) )
    {
        wxCommandEvent CmdEvent;
        OnNextTrackButtonClick( CmdEvent );
        OnPlayButtonClick( CmdEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const guTrackArray &tracks, const bool allowplay )
{
    int PrevTrackCount = m_PlayListCtrl->GetCount();
    if( tracks.Count() )
    {
        guTrack * Track = &tracks[ 0 ];
        bool ClearPlayList = ( Track->m_TrackMode == guTRACK_MODE_RANDOM ||
                               Track->m_TrackMode == guTRACK_MODE_SMART ) && !m_DelTracksPlayed;

        m_PlayListCtrl->AddToPlayList( tracks, ClearPlayList );

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

            if( ( Count = m_SmartAddedTracks.Count() ) > guPLAYER_SMART_CACHEITEMS )
                m_SmartAddedTracks.RemoveAt( 0, Count - guPLAYER_SMART_CACHEITEMS );

            if( ( Count = m_SmartAddedArtists.Count() ) > guPLAYER_SMART_CACHEARTISTS )
                m_SmartAddedArtists.RemoveAt( 0, Count - guPLAYER_SMART_CACHEARTISTS );
        }

        if( allowplay && !PrevTrackCount && ( GetState() != wxMEDIASTATE_PLAYING ) )
        {
            wxCommandEvent CmdEvent;
            OnNextTrackButtonClick( CmdEvent );
            OnPlayButtonClick( CmdEvent );
        }
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
    wxPostEvent( wxTheApp->GetTopWindow(), TitleEvent );

    wxCommandEvent TracksChangedEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, TracksChangedEvent );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListUpdated( wxCommandEvent &event )
{
    m_PlayListCtrl->ReloadItems();
    //SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
    m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );

    // If a Player reset is needed
    if( event.GetExtraLong() )
    {
        OnStopButtonClick( event );
        OnPlayButtonClick( event );
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
        if( Count > guPLAYER_SMART_CACHEITEMS )
            Index = Count - guPLAYER_SMART_CACHEITEMS;
        for( ; Index < Count; Index++ )
        {
            guTrack * Track = m_PlayListCtrl->GetItem( Index );
            m_SmartAddedTracks.Add( Track->m_SongId );
        }

        Index = 0;
        Count = m_PlayListCtrl->GetCount();
        if( Count > guPLAYER_SMART_CACHEARTISTS )
            Index = Count - guPLAYER_SMART_CACHEARTISTS;
        for( ; Index < Count; Index++ )
        {
            guTrack * Track = m_PlayListCtrl->GetItem( Index );
            m_SmartAddedArtists.Add( Track->m_ArtistName.Upper() );
        }
    }
    TrackListChanged();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::UpdateStatus()
{
    wxMediaState State;
    wxFileOffset CurPos;
    static bool IsUpdatingStatus = false;

    if( IsUpdatingStatus )
        return;

    IsUpdatingStatus = true;

    State = m_MediaCtrl->GetState();
    if( State != m_LastPlayState )
    {
        if( State == wxMEDIASTATE_PLAYING )
        {
            m_PlayButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_pause ) );
            m_PlayButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_pause ) );
        }
        else
        {
            m_PlayButton->SetBitmapLabel( guImage( guIMAGE_INDEX_player_normal_play ) );
            m_PlayButton->SetBitmapHover( guImage( guIMAGE_INDEX_player_highlight_play ) );
        }
        m_PlayButton->Refresh();
        m_LastPlayState = State;
        //
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }

    if( State == wxMEDIASTATE_PLAYING )
    {
        // Some track lengths are not correctly read by taglib so
        // we try to find the length from gstreamer and update the database
        // We need to not do this for radiostations or online streams
        if( m_MediaSong.m_Length == 0 &&
            m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
        {
            m_MediaSong.m_Length = m_MediaCtrl->GetLength();

            if( m_MediaSong.m_SongId )
            {
                if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
                    m_Db->UpdateTrackLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
                else if( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST )
                    m_Db->UpdatePodcastItemLength( m_MediaSong.m_SongId, m_MediaSong.m_Length );
            }

            // Update the track in database, playlist, etc
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
        }

        //CurPos = m_MediaCtrl->Tell();
        CurPos = GetPosition();
        if( ( CurPos != m_LastCurPos ) && !m_SliderIsDragged )
        {
//            if( !m_ShowRevTime )
//            {
//                m_PositionLabel->SetLabel( LenToString( CurPos / 1000 ) + _( " of " ) + LenToString( m_MediaSong.m_Length ) );
//            }
//            else
//                m_PositionLabel->SetLabel( wxT( "-" ) + LenToString( m_MediaSong.m_Length - ( CurPos / 1000 ) ) + _( " of " ) + LenToString( m_MediaSong.m_Length ) );
//
//            m_PosLabelSizer->Layout();

            UpdatePositionLabel( CurPos / 1000 );

            if( m_MediaSong.m_Length )
                m_PlayerPositionSlider->SetValue( CurPos / m_MediaSong.m_Length );

            //printf( "Slider Updated to %lli of %lli\n", CurPos, ( long long int ) m_MediaSong.Length * 1000 );
            m_MediaSong.m_PlayTime = ( CurPos / 1000 );
            m_LastCurPos = CurPos;
        }
    }

    if( m_CurVolume != m_LastVolume )
    {
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
    }

    // Total Length
    if( m_LastTotalLen != m_PlayListCtrl->GetLength() )
    {
        m_LastTotalLen = m_PlayListCtrl->GetLength();
        TrackListChanged();
    }

    IsUpdatingStatus = false;
}

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
const guCurrentTrack * guPlayerPanel::GetCurrentTrack()
{
    //return m_PlayListCtrl->GetCurrent();
    return &m_MediaSong;
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
void guPlayerPanel::SetCurrentTrack( const guTrack * Song )
{
    wxImage * CoverImage;

    if( !Song )
        return;

    // Check if the Current Song have played more than the half and if so add it to
    // The CachedPlayedSong database to be submitted to LastFM AudioScrobbling
    if( m_AudioScrobbleEnabled && ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) ) // If its not a radiostation
    {
        //guLogMessage( wxT( "PlayTime: %u Length: %u" ), m_MediaSong.PlayTime, m_MediaSong.Length );
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
                m_Db->SetTrackPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount );
            else
                m_Db->SetPodcastItemPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount );

            // Update the track in database, playlist, etc
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
        }
    }


    // Set the Current Song
    m_MediaSong = * Song;

    // Update the Current Playing Song Info
    UpdateLabels();
    UpdatePositionLabel( 0 );

//    m_PlayListCtrl->SetColumnLabel( 0, _( "Now Playing" ) +
//        wxString::Format( wxT( ":  %i / %i    ( %s )" ),
//            m_PlayListCtrl->GetCurItem() + 1,
//            m_PlayListCtrl->GetCount(),
//            m_PlayListCtrl->GetLengthStr().c_str() ) );
    wxCommandEvent TitleEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_UPDATETITLE );
    wxPostEvent( wxTheApp->GetTopWindow(), TitleEvent );

    //guLogWarning( wxT( "SetCurrentTrack : CoverId = %u - %u" ), LastCoverId, m_MediaSong.CoverId );
    CoverImage = NULL;
    if( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_net_radio ) );
        m_MediaSong.m_CoverType = GU_SONGCOVER_RADIO;
    }
    else if( ( CoverImage = guTagGetPicture( m_MediaSong.m_FileName ) ) )
    {
        m_MediaSong.m_CoverType = GU_SONGCOVER_ID3TAG;
    }
    else if( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_podcast ) );
        m_MediaSong.m_CoverType = GU_SONGCOVER_PODCAST;
    }
    else if( m_MediaSong.m_CoverId )
    {
        //guLogMessage( wxT( "CoverId %i" ), m_MediaSong.m_CoverId );
        m_MediaSong.m_CoverPath = m_Db->GetCoverPath( m_MediaSong.m_CoverId );
        m_MediaSong.m_CoverType = GU_SONGCOVER_FILE;
    }
    else
    {
        //guLogWarning( wxT( "Trying to find covers in %s" ), wxPathOnly( m_MediaSong.m_FileName ).c_str() );
        m_MediaSong.m_CoverPath = m_PlayListCtrl->FindCoverFile( wxPathOnly( m_MediaSong.m_FileName ) );
    }

//    guLogMessage( wxT( "   File : %s" ), m_MediaSong.m_FileName.c_str() );
//    guLogMessage( wxT( " Loaded : %i" ), m_MediaSong.m_Loaded );
//    guLogMessage( wxT( "   Type : %i" ), m_MediaSong.m_Type );
//    guLogMessage( wxT( " SongId : %i" ), m_MediaSong.m_SongId );
//    guLogMessage( wxT( "CoverId : %i" ), m_MediaSong.m_CoverId );
//    guLogMessage( wxT( "Co.Type : %i" ), m_MediaSong.m_CoverType );
//    guLogMessage( wxT( "  Cover : '%s'" ), m_MediaSong.m_CoverPath.c_str() );
//    guLogMessage( wxT( "===========================================" ) );

    if( !CoverImage )
    {
        if( m_MediaSong.m_CoverPath.IsEmpty() || !wxFileExists( m_MediaSong.m_CoverPath ) )
        {
            //printf( "No coverpath set\n" );
            CoverImage = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
            m_MediaSong.m_CoverType = GU_SONGCOVER_NONE;
            m_MediaSong.m_CoverPath = wxEmptyString;
        }
        else
        {
            CoverImage = new wxImage( m_MediaSong.m_CoverPath );
            m_MediaSong.m_CoverType = GU_SONGCOVER_FILE;
            //m_MediaSong.CoverPath = CoverPath;
        }
    }

    // Cover
    if( CoverImage )
    {
        if( CoverImage->IsOk() )
        {
            CoverImage->Rescale( 100, 100, wxIMAGE_QUALITY_HIGH );
            m_PlayerCoverBitmap->SetBitmap( wxBitmap( *CoverImage ) );
            m_PlayerCoverBitmap->Refresh();
        }
    }

    // Check if Smart is enabled
    if( ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) && m_PlaySmart &&
        ( ( m_PlayListCtrl->GetCurItem() + m_SmartPlayMinTracksToPlay ) >= m_PlayListCtrl->GetCount() ) )
    {
        SmartAddTracks( m_MediaSong );
    }

    // If its a Radio disable PositionSlider
    m_PlayerPositionSlider->SetValue( 0 );
    if( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION )
    {
        m_PlayerPositionSlider->Disable();
    }
    else if( !m_PlayerPositionSlider->IsEnabled() )
    {
        m_PlayerPositionSlider->Enable();
    }

    // Send the CapsChanged Event
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_CAPSCHANGED );
    wxPostEvent( wxTheApp->GetTopWindow(), event );

    if( m_MediaCtrl->GetState() == wxMEDIASTATE_PLAYING )
    {
        // If Enabled LastFM->Submit and no error then send Now Playing Information
        if( m_AudioScrobbleEnabled && m_AudioScrobble && m_AudioScrobble->IsOk() &&
            ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) )
        {
            m_AudioScrobble->SendNowPlayingTrack( m_MediaSong );
        }
    }
    else
    {
        m_PendingScrob = true;
    }

    SendNotifyInfo( CoverImage );

    if( CoverImage )
        delete CoverImage;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListDClick( wxCommandEvent &event )
{
    int item = event.GetInt();
    m_PlayListCtrl->SetCurrent( item, m_DelTracksPlayed && !m_PlayLoop );
    //m_MediaSong = * m_PlayListCtrl->GetCurrent();
    SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
    //wxLogMessage( wxT( "Selected %i : %s - %s" ), m_MediaSong.SongId, m_MediaSong.ArtistName.c_str(), m_MediaSong.SongName.c_str() );
    OnStopButtonClick( event );
    OnPlayButtonClick( event );
}

// -------------------------------------------------------------------------------- //
wxString inline FileNameEncode( const wxString filename )
{
    wxString RetVal = filename;
    RetVal.Replace( wxT( "%" ), wxT( "%25" ) );
    RetVal.Replace( wxT( "#" ), wxT( "%23" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::LoadMedia( const wxString &FileName, bool restart )
{
    //guLogMessage( wxT( "LoadMedia Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    //m_MediaCtrl->Load( NextItem->FileName );
    wxURI UriPath( FileName );
    wxString Uri;
    try {
        if( !UriPath.HasScheme() )
            Uri = wxT( "file://" ) + FileName;
        else
            Uri = FileName;

        if( !m_MediaCtrl->Load( FileNameEncode( Uri ), restart ) )
        {
            guLogError( wxT( "ee: Failed load of file '%s'" ), Uri.c_str() );
            //guLogError( wxT( "ee: The filename was '%s'" ), FileName.c_str() );
        }
    }
    catch(...)
    {
        guLogError( wxT( "Error loading '%s'" ), FileName.c_str() );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaBuffering( wxMediaEvent &event )
{
    wxCommandEvent GaugeEvent( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    int Percent = event.GetInt();
//    printf( "Buffering: %d%%\n", Percent );
    if( Percent == 100 )
    {
        if( m_BufferGaugeId != wxNOT_FOUND )
        {
            GaugeEvent.SetId( ID_GAUGE_REMOVE );
            GaugeEvent.SetInt( m_BufferGaugeId );
            wxPostEvent( wxTheApp->GetTopWindow(), GaugeEvent );
            m_BufferGaugeId = wxNOT_FOUND;
        }
    }
    else
    {
        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
        if( m_BufferGaugeId == wxNOT_FOUND )
        {
              m_BufferGaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge( _( "Buffering..." ) );
              GaugeEvent.SetId( ID_GAUGE_SETMAX );
              GaugeEvent.SetInt( m_BufferGaugeId );
              GaugeEvent.SetExtraLong( 100 );
              wxPostEvent( wxTheApp->GetTopWindow(), GaugeEvent );
        }

        if( m_BufferGaugeId != wxNOT_FOUND )
        {
            GaugeEvent.SetId( ID_GAUGE_UPDATE );
            GaugeEvent.SetInt( m_BufferGaugeId );
            GaugeEvent.SetExtraLong( Percent );
            wxPostEvent( MainFrame, GaugeEvent );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaLevel( wxMediaEvent &event )
{
    if( m_SilenceDetector && m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
    {
        //guLogMessage( wxT( "Decay Level: %f  %s" ), event.m_LevelInfo.m_Decay_L, LenToString( event.m_LevelInfo.m_EndTime / 1000000000 ).c_str() );
        if( int( event.m_LevelInfo.m_Decay_L ) < m_SilenceDetectorLevel )
        {
            unsigned long EventTime = event.m_LevelInfo.m_EndTime / 1000000;
            unsigned long TrackLength = m_MediaSong.m_Length * 1000;
            //guLogMessage( wxT( "The level is now lower than triger level" ) );
            //guLogMessage( wxT( "(%f) %u : %u , %i" ), event.m_LevelInfo.m_Decay_L, m_SilenceDetectorTime, EventTime, TrackLength - EventTime );

            // We only skip to next track if the level is lower than the triger one and also if
            // we are at the end time period (if configured this way) and the time left is more than 500msecs
            if( !m_SilenceDetectorTime ||
                ( ( ( unsigned int ) m_SilenceDetectorTime > ( TrackLength - EventTime ) ) &&
                  ( ( EventTime + 500 ) < TrackLength ) ) )
            {
                wxCommandEvent evt;
                OnNextTrackButtonClick( evt );
                //guLogMessage( wxT( "Silence detected. Changed to next track" ) );
            }
        }
    }

    if( m_PlayerVumeters )
    {
//        guLogMessage( wxT( "%lli %u" ), GST_TIME_AS_MSECONDS( event.m_LevelInfo.m_EndTime ), GetPosition() );

        m_PlayerVumeters->SetLevels( LastLevelInfo );
        LastLevelInfo = event.m_LevelInfo;
//        guLogMessage( wxT( "L:%02.02f  R:%02.02f" ),
//            event.m_LevelInfo.m_Peak_L,
//            event.m_LevelInfo.m_Peak_R );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaError( wxMediaEvent &event )
{
    //guLogMessage( wxT( "OnMediaError: %i" ), m_PlayListCtrl->GetCurItem() );
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

    if( m_AboutToFinishPending )
        m_AboutToFinishPending = false;

    if( m_IsSkipping )
        m_IsSkipping = false;

    m_MediaCtrl->ClearError();

    // Be sure it will not try to play the track again
    event.SetInt( 0 );
    // Simulate the track loaded correctly.
    OnMediaLoaded( event );

    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STOP );
    AddPendingEvent( CmdEvent );

    CmdEvent.SetId( ID_PLAYERPANEL_NEXTTRACK );
    AddPendingEvent( CmdEvent );

    CmdEvent.SetId( ID_PLAYERPANEL_STOP );
    AddPendingEvent( CmdEvent );

    CmdEvent.SetId( ID_PLAYERPANEL_PLAY );
    AddPendingEvent( CmdEvent );
}

// -------------------------------------------------------------------------------- //
// 0 -> Artist
// 1 -> Title
wxArrayString ExtractMetaData( const wxString &TitleStr )
{
    wxArrayString RetVal;
    wxString Artist = wxEmptyString;
    wxString Title = wxEmptyString;
    int FindPos;
    if( !TitleStr.IsEmpty() )
    {
        FindPos = TitleStr.Find( wxT( " - " ) );
        if( FindPos != wxNOT_FOUND )
        {
            Artist = TitleStr.Mid( 0, FindPos );
            Title = TitleStr.Mid( FindPos + 3 );
            FindPos = Title.Find( wxT( " - " ) );
            if( FindPos != wxNOT_FOUND )
            {
                Title = Title.Mid( 0, FindPos );
            }
        }
        else
        {
            Artist = TitleStr;
        }
        RetVal.Add( Artist );
        RetVal.Add( Title );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaTag( wxMediaEvent &event )
{
    wxString * TagStr = ( wxString * ) event.GetClientData();
    if( TagStr )
    {
        if( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION )
        {
            wxArrayString MetaData = ExtractMetaData( * TagStr );
            if( MetaData.Count() )
            {
                m_MediaSong.m_ArtistName = MetaData[ 0 ];
                m_MediaSong.m_SongName = MetaData[ 1 ];
                //m_MediaSong.AlbumName = MetaData[ 2 ];

                SetTitleLabel( m_MediaSong.m_SongName );
                //SetAlbumLabel( m_MediaSong.m_AlbumName );
                SetArtistLabel( m_MediaSong.m_ArtistName );

                GetSizer()->Layout();

                //guLogMessage( wxT( "Sending LastFMPanel::UpdateTrack event" ) );
                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
                event.SetClientData( new guTrack( m_MediaSong ) );
                wxPostEvent( wxTheApp->GetTopWindow(), event );

                wxImage Image( guImage( guIMAGE_INDEX_net_radio ) );
                SendNotifyInfo( &Image );
            }
        }
        delete TagStr;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaBitrate( wxMediaEvent &event )
{
    SetBitRate( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaLoaded( wxMediaEvent &event )
{
    //guLogMessage( wxT( "OnMediaLoaded Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    if( m_AboutToFinishPending )
    {
        //guLogMessage( wxT( "Push back the OnMediaLoaded event..." ) );
        wxPostEvent( m_MediaCtrl, event );
        return;
    }

    if( m_IsSkipping )
        m_IsSkipping = false;

    try {
        //guLogMessage( wxT("OnMediaLoaded") );

        if( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION )
        {
            // Send an event so the LastFMPanel update its content.
            //guLogMessage( wxT( "Sending LastFMPanel::UpdateTrack event" ) );
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
            event.SetClientData( new guTrack( m_MediaSong ) );
            wxPostEvent( wxTheApp->GetTopWindow(), event );
        }
        else if( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST )
        {
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
            event.SetClientData( NULL );
            wxPostEvent( wxTheApp->GetTopWindow(), event );
        }

        if( event.GetInt() )
        {
            //
            m_MediaCtrl->Play();
        }

        if( m_TrackStartPos )
        {
            SetPosition( m_TrackStartPos );
            m_TrackStartPos = 0;
        }
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
        //SetVolume( m_CurVolume );
        // If Enabled LastFM->Submit and no error then send Now Playing Information
        if( m_PendingScrob && m_AudioScrobbleEnabled && m_AudioScrobble && m_AudioScrobble->IsOk() &&
            ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) )
        {
            m_AudioScrobble->SendNowPlayingTrack( m_MediaSong );
            m_PendingScrob = false;
        }

    }
    catch(...)
    {
        OnNextTrackButtonClick( event );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnAboutToFinish( void )
{
    guTrack * NextItem = m_PlayListCtrl->GetNext( m_PlayLoop );
    if( NextItem )
    {
        //guLogMessage( wxT( "Starting of About-To-Finish" ) );
        m_AboutToFinishPending = true;
        LoadMedia( NextItem->m_FileName, false );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaAboutToFinish( wxMediaEvent &event )
{
    //guLogMessage( wxT( "Ending About-To-Finish %i" ), m_AboutToFinishPending );
    //guLogMessage( wxT( "Ending About to Finsih Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    if( m_AboutToFinishPending )
    {
        SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
        m_AboutToFinishPending = false;
        //guLogMessage( wxT( "End About-To-Finish %i" ), m_AboutToFinishPending );
        return;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFinished( wxMediaEvent &event )
{
    //guLogMessage( wxT( "OnMediaFinished Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    if( m_AboutToFinishPending )
    {
        m_AboutToFinishPending = false;
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
        //guLogMessage( wxT( "EOS cancelled..." ) );
        return;
    }

    guTrack * NextItem = m_PlayListCtrl->GetNext( m_PlayLoop );
    if( NextItem )
    {
        //m_MediaSong = * NextItem;
        SetCurrentTrack( NextItem );
        LoadMedia( NextItem->m_FileName );
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

                OnMediaFinished( event );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
const wxMediaState guPlayerPanel::GetState( void )
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

    // Send Notification for the mpris interface
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
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

    // Send Notification for the mpris interface
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetPlayLoop()
{
    return m_PlayLoop;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPrevTrackButtonClick( wxCommandEvent& event )
{
    wxMediaState State;
    wxFileOffset CurPos;
    guTrack * PrevItem;

    if( m_IsSkipping )
        return;

    // If we are already in the first Item start again the song from the begining
    State = m_MediaCtrl->GetState();
    CurPos = m_MediaCtrl->Tell();
    int CurItem = m_PlayListCtrl->GetCurItem();
    if( ( ( CurItem == 0 ) && ( State == wxMEDIASTATE_PLAYING ) ) ||
        ( ( State != wxMEDIASTATE_STOPPED ) && ( CurPos  > GUPLAYER_MIN_PREVTRACK_POS ) ) )
    {
        //m_MediaCtrl->Stop();
        SetPosition( 0 );
        //m_MediaCtrl->Play();
        return;
    }

    bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_PREVTRACK ) ||
                      ( event.GetEventObject() == m_PrevTrackButton );

    PrevItem = m_PlayListCtrl->GetPrev( m_PlayLoop, ForceSkip );
    if( PrevItem )
    {
        //State = m_MediaCtrl->GetState();
        if( State != wxMEDIASTATE_STOPPED )
        {
            m_MediaCtrl->Stop();
            //m_MediaSong = * PrevItem;
            SetCurrentTrack( PrevItem );
            if( State == wxMEDIASTATE_PLAYING )
            {
                m_IsSkipping = true;
                LoadMedia( m_MediaSong.m_FileName );
            }
        }
        else
        {
            SetCurrentTrack( PrevItem );
        }
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
//    //event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnNextTrackButtonClick( wxCommandEvent& event )
{
    //guLogMessage( wxT( "OnNextTrackButtonClick Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    wxMediaState State;
    guTrack * NextItem;

    if( m_IsSkipping )
        return;

    bool ForceSkip = ( event.GetId() == ID_PLAYERPANEL_NEXTTRACK ) ||
                      ( event.GetEventObject() == m_NextTrackButton );

    NextItem = m_PlayListCtrl->GetNext( m_PlayLoop, ForceSkip );
    if( NextItem )
    {
        State = m_MediaCtrl->GetState();
        if( State != wxMEDIASTATE_STOPPED )
        {
            m_MediaCtrl->Stop();
        }
        //m_MediaSong = * NextItem;
        SetCurrentTrack( NextItem );
        if( State == wxMEDIASTATE_PLAYING )
        {
            m_IsSkipping = true;
            LoadMedia( m_MediaSong.m_FileName );
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
void guPlayerPanel::OnPlayButtonClick( wxCommandEvent& event )
{
    //guLogMessage( wxT( "OnPlayButtonClick Cur: %i" ), m_PlayListCtrl->GetCurItem() );
    wxMediaState State;

    // Get The Current Song From m_PlayListCtrl
    //guTrack * CurItem = m_PlayListCtrl->GetCurrent();
    //if( !m_MediaSong.m_SongId && m_PlayListCtrl->GetItemCount() )
    if( !m_MediaSong.m_Loaded && m_PlayListCtrl->GetItemCount() )
    {
        m_PlayListCtrl->SetCurrent( 0, m_DelTracksPlayed && !m_PlayLoop );
        //m_MediaSong = * m_PlayListCtrl->GetCurrent();
        SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
    }

    if( m_MediaSong.m_Loaded )
    {
        State = m_MediaCtrl->GetState();
        if( State == wxMEDIASTATE_PLAYING )
        {
            m_MediaCtrl->Pause();
            ResetVumeterLevel();
        }
        else if( State == wxMEDIASTATE_PAUSED )
        {
            m_MediaCtrl->Play();
        }
        else if( State == wxMEDIASTATE_STOPPED )
        {
            //guLogMessage( wxT( "Loading '%s'" ), m_MediaSong.FileName.c_str() );
            LoadMedia( m_MediaSong.m_FileName );
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
    //wxMediaState State;
    //State = m_MediaCtrl->GetState();
    //guLogMessage( wxT( "State: %i" ), State );
    //if( State != wxMEDIASTATE_STOPPED )
    //{
        m_MediaCtrl->Stop();
        UpdatePositionLabel( 0 );
        if( m_MediaSong.m_Length )
            m_PlayerPositionSlider->SetValue( 0 );
        ResetVumeterLevel();
    //}
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
    m_PlayListCtrl->Randomize();
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
        if( m_MediaSong.m_CoverType == GU_SONGCOVER_ID3TAG )
            BigCover->SetBitmap( m_MediaSong.m_CoverType, m_MediaSong.m_FileName );
        else
            BigCover->SetBitmap( m_MediaSong.m_CoverType, m_MediaSong.m_CoverPath );
        BigCover->Show();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderBeginSeek( wxScrollEvent &event )
{
    m_SliderIsDragged = true;
    //
    if( m_MediaSong.m_Length )
    {
        int CurPos = event.GetPosition() * m_MediaSong.m_Length;

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
        SetPosition( NewPos * m_MediaSong.m_Length );
    }
    m_SliderIsDragged = false;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayerPositionSliderMouseWheel( wxMouseEvent &event )
{
    if( m_MediaSong.m_Type != guTRACK_TYPE_RADIOSTATION )
    {
        int Rotation = event.GetWheelRotation() / event.GetWheelDelta();
        m_PlayerPositionSlider->SetValue( wxMax( 0, wxMin( 1000, m_PlayerPositionSlider->GetValue() +
            ( Rotation * 50 ) ) ) );
        SetPosition( m_PlayerPositionSlider->GetValue() * m_MediaSong.m_Length );
    }
    m_SliderIsDragged = false;
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

    m_MediaCtrl->SetVolume(  volume / ( double ) 100.0 );
    m_VolumeButton->SetToolTip( _( "Volume" ) + wxString::Format( wxT( " %u%%" ), ( int ) volume ) );
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::SetPosition( int pos )
{
    return m_MediaCtrl->Seek( pos );
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetPosition()
{
    if( m_AboutToFinishPending )
        return 0;
    return m_MediaCtrl->Tell();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnTitleNameDClicked( wxMouseEvent &event )
{
    int TrackId = wxNOT_FOUND;
    if( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        TrackId = m_MediaSong.m_SongId;
    }
    else
    {
        TrackId = m_Db->FindTrack( m_MediaSong.m_ArtistName, m_MediaSong.m_SongName );
    }

    if( TrackId != wxNOT_FOUND )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_TRACK );
        evt.SetInt( TrackId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        wxPostEvent( wxTheApp->GetTopWindow(), evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnAlbumNameDClicked( wxMouseEvent &event )
{
    int AlbumId = wxNOT_FOUND;
    if( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        AlbumId = m_MediaSong.m_AlbumId;
    }
    else
    {
        AlbumId = m_Db->FindAlbum( m_MediaSong.m_ArtistName, m_MediaSong.m_AlbumName );
    }

    if( AlbumId != wxNOT_FOUND )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ALBUM );
        evt.SetInt( AlbumId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        wxPostEvent( wxTheApp->GetTopWindow(), evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnArtistNameDClicked( wxMouseEvent &event )
{
    int ArtistId = wxNOT_FOUND;
    if( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )
    {
        ArtistId = m_MediaSong.m_ArtistId;
    }
    else
    {
        ArtistId = m_Db->FindArtist( m_MediaSong.m_ArtistName );
    }

    if( ArtistId != wxNOT_FOUND )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( ArtistId );
        evt.SetExtraLong( m_MediaSong.m_Type );
        wxPostEvent( wxTheApp->GetTopWindow(), evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRatingChanged( guRatingEvent &event )
{
    m_MediaSong.m_Rating = event.GetInt();
    if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
    {
        m_Db->SetTrackRating( m_MediaSong.m_SongId, m_MediaSong.m_Rating );

        // Update the track in database, playlist, etc
        ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTrack( guUPDATED_TRACKS_PLAYER, &m_MediaSong );
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
    LastLevelInfo = LevelInfo;
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
    int Rotation = event.GetWheelRotation() / event.GetWheelDelta();
    //guLogMessage( wxT( "CurVol: %u  Rotations:%i" ), m_CurVolume, Rotation );
    SetVolume( m_CurVolume + ( Rotation * 4 ) );
}

// -------------------------------------------------------------------------------- //
// guPlayerPanelTimer
// -------------------------------------------------------------------------------- //
void guPlayerPanelTimer::Notify()
{
    if( Player )
    {
        Player->UpdateStatus();
    }
}

// -------------------------------------------------------------------------------- //
// guSmartAddTracksThread
// -------------------------------------------------------------------------------- //
guSmartAddTracksThread::guSmartAddTracksThread( guDbLibrary * db,
        guPlayerPanel * playerpanel, const guTrack * track,
        wxArrayInt * smartaddedtracks, wxArrayString * smartaddedartists,
        const int trackcount, const int filterallow, const int filterdeny ) : wxThread()
{
    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_CurSong = track;
    m_SmartAddedTracks = smartaddedtracks;
    m_SmartAddedArtists = smartaddedartists;
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
};

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

        if( m_SmartAddedTracks->Count() > guPLAYER_SMART_CACHEITEMS )
            m_SmartAddedTracks->RemoveAt( 0, m_SmartAddedTracks->Count() - guPLAYER_SMART_CACHEITEMS );

        if( m_SmartAddedArtists->Count() > guPLAYER_SMART_CACHEARTISTS )
            m_SmartAddedArtists->RemoveAt( 0, m_SmartAddedArtists->Count() - guPLAYER_SMART_CACHEARTISTS );

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