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
#include "Images.h"
#include "LastFM.h"
#include "MainFrame.h"
#include "TagInfo.h"
#include "TrackChangeInfo.h"
#include "Utils.h"
#include "VolumeFrame.h"

#include <wx/gdicmn.h>
#include <wx/regex.h>
#include <wx/utils.h>

#define GUPLAYER_MIN_PREVTRACK_POS      5000
#define GUPLAYER_SMART_ADDTRACKS        3
#define GUPLAYER_SMART_ADDTRACKSPOS     4
#define GUPLAYER_SMART_CACHEITEMS       100

// -------------------------------------------------------------------------------- //
guPlayerPanel::guPlayerPanel( wxWindow* parent, DbLibrary * NewDb ) //wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
       : wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 368,191 ), wxTAB_TRAVERSAL )
{
	wxBoxSizer* PlayerMainSizer;
	wxBoxSizer* PlayerBtnSizer;
	wxBoxSizer* PlayerDetailsSizer;
	wxBoxSizer* PlayerLabelsSizer;
	wxBoxSizer* PlayListSizer;
    wxPanel * PlayListPanel;
	wxBoxSizer* PlayListPanelSizer;

	wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

    m_Db = NewDb;
    m_BufferGaugeId = wxNOT_FOUND;
    m_MediaSong.m_SongId = 0;
    m_MediaSong.m_Length = 0;

    // For the Load configuration
    wxArrayString Songs;
    guConfig * Config;

    m_LastVolume = wxNOT_FOUND;

    m_LastCurPos = -1;
    m_LastPlayState = wxMEDIASTATE_STOPPED;
    m_LastTotalLen = -1;

    m_AboutToFinishPending = false;

    // Load configuration
    Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        //guLogMessage( wxT( "Reading PlayerPanel Config" ) );
        m_CurVolume = Config->ReadNum( wxT( "PlayerCurVol" ), 50, wxT( "General" ) );
        m_PlayLoop = Config->ReadBool( wxT( "PlayerLoop" ), false, wxT( "General" )  );
        m_PlaySmart = Config->ReadBool( wxT( "PlayerSmart" ), m_PlayLoop ? false : true, wxT( "General" )  );
        m_SmartPlayAddTracks = Config->ReadNum( wxT( "SmartPlayAddTracks" ), 3, wxT( "SmartPlayList" ) );
        m_SmartPlayMinTracksToPlay = Config->ReadNum( wxT( "SmartPlayMinTracksToPlay" ), 4, wxT( "SmartPlayList" ) );
        m_AudioScrobbleEnabled = Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) );
    }
    m_SliderIsDragged = false;
    m_SmartSearchEnabled = false;
    m_SmartAddTracksThread = NULL;

	PlayerMainSizer = new wxBoxSizer( wxVERTICAL );

	PlayerBtnSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PrevTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_skip_backward ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_PrevTrackButton->SetToolTip( _( "Go to Previous Track in the Playlist" ) );
	PlayerBtnSizer->Add( m_PrevTrackButton, 0, wxALL, 2 );

	m_NextTrackButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_skip_forward ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_NextTrackButton->SetToolTip( _( "Go to Next Track in the Playlist" ) );
	PlayerBtnSizer->Add( m_NextTrackButton, 0, wxALL, 2 );

	m_PlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_playback_start ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_PlayButton->SetToolTip( _( "Start playing or pauses current track in the Playlist" ) );
	PlayerBtnSizer->Add( m_PlayButton, 0, wxALL, 2 );

	m_StopButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_playback_stop ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_StopButton->SetToolTip( _( "Stops player reproduction" ) );
	PlayerBtnSizer->Add( m_StopButton, 0, wxALL, 2 );

	m_VolumeButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_volume_medium ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_VolumeButton->SetToolTip( _( "Volume 0%" ) );
	PlayerBtnSizer->Add( m_VolumeButton, 0, wxALL, 2 );

	m_SmartPlayButton = new wxToggleBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_playlist_smart ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_SmartPlayButton->SetToolTip( _( "Add tracks to the playlist bassed on LastFM" ) );
	// Get this value from config file
	m_SmartPlayButton->SetValue( m_PlaySmart );
	PlayerBtnSizer->Add( m_SmartPlayButton, 0, wxALL, 2 );

	m_RandomPlayButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_playlist_shuffle ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_RandomPlayButton->SetToolTip( _( "Randomize the tracks in the playlist" ) );
	PlayerBtnSizer->Add( m_RandomPlayButton, 0, wxALL, 2 );

	m_RepeatPlayButton = new wxToggleBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_playlist_repeat ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_RepeatPlayButton->SetToolTip( _( "Repeats the current playlist" ) );
	m_RepeatPlayButton->SetValue( m_PlayLoop );
	PlayerBtnSizer->Add( m_RepeatPlayButton, 0, wxALL, 2 );


	PlayerMainSizer->Add( PlayerBtnSizer, 0, wxEXPAND, 5 );

	PlayerDetailsSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayerCoverBitmap = new wxStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_no_cover ), wxDefaultPosition, wxSize( 100,100 ), 0 );
	m_PlayerCoverBitmap->SetToolTip( _( "Shows the current track album cover if available" ) );
	PlayerDetailsSizer->Add( m_PlayerCoverBitmap, 0, wxALL, 2 );

	PlayerLabelsSizer = new wxBoxSizer( wxVERTICAL );

	m_TitleLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	//m_TitleLabel = new guAutoScrollText( this, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TitleLabel->SetToolTip( _( "Show the name of the current track" ) );
	m_TitleLabel->Wrap( -1 );
	CurrentFont.SetPointSize( 16 );
	CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
	m_TitleLabel->SetFont( CurrentFont );

	PlayerLabelsSizer->Add( m_TitleLabel, 0, wxLEFT|wxRIGHT|wxBOTTOM, 2 );

	m_AlbumLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumLabel->SetToolTip( _( "Show the album name of the current track" ) );
	m_AlbumLabel->Wrap( -1 );
	CurrentFont.SetPointSize( 12 );
	CurrentFont.SetWeight( wxFONTWEIGHT_NORMAL );
	CurrentFont.SetStyle( wxFONTSTYLE_ITALIC );
	m_AlbumLabel->SetFont( CurrentFont );

	PlayerLabelsSizer->Add( m_AlbumLabel, 0, wxALL, 2 );

	m_ArtistLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistLabel->SetToolTip( _( "Show the artist name of the current track" ) );
	m_ArtistLabel->Wrap( -1 );
	CurrentFont.SetStyle( wxFONTSTYLE_NORMAL );
	m_ArtistLabel->SetFont( CurrentFont );

	PlayerLabelsSizer->Add( m_ArtistLabel, 0, wxALL, 2 );

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

    m_BitRateSizer->Add( m_BitRateLabel, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );

	PlayerLabelsSizer->Add( m_BitRateSizer, 0, wxEXPAND, 2 );

	PlayerDetailsSizer->Add( PlayerLabelsSizer, 1, wxEXPAND, 5 );

	PlayerMainSizer->Add( PlayerDetailsSizer, 0, wxEXPAND, 5 );

	m_PlayerPositionSlider = new wxSlider( this, wxID_ANY, 0, 0, 1000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	PlayerMainSizer->Add( m_PlayerPositionSlider, 0, wxALL|wxEXPAND, 0 );

	PlayListSizer = new wxBoxSizer( wxVERTICAL );

	PlayListPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	PlayListPanelSizer = new wxBoxSizer( wxVERTICAL );

	m_PlayListCtrl = new guPlayList( PlayListPanel, m_Db );
	PlayListPanelSizer->Add( m_PlayListCtrl );

	PlayListPanel->SetSizer( PlayListPanelSizer );
	PlayListPanel->Layout();

	PlayListPanelSizer->Fit( PlayListPanel );
	PlayListSizer->Add( PlayListPanel, 1, wxEXPAND | wxALL, 2 );

	PlayerMainSizer->Add( PlayListSizer, 1, wxEXPAND, 5 );

	this->SetSizer( PlayerMainSizer );
	this->Layout();

    m_MediaCtrl = new guMediaCtrl( this );
    //m_MediaCtrl->Create( this, wxID_ANY );

    //
    m_PlayListCtrl->ReloadItems();
    TrackListChanged();
    // The SetVolume call dont get set if the volume is the last one
    // so we do it two calls
    if( ( m_MediaCtrl->GetVolume() * 100.0 ) != m_CurVolume )
    {
        float SavedVol = m_CurVolume;
        SetVolume( 0.0 );
        SetVolume( SavedVol );
        //guLogMessage( wxT( "Set Volume %i %e" ), m_CurVolume, m_MediaCtrl->GetVolume() );
    }

    // There was a track passed as argument that we will play
    if( m_PlayListCtrl->StartPlaying() )
    {
        wxCommandEvent event;
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
        }
    }

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

    Connect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ) );

	m_AlbumLabel->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnAlbumNameDClicked ), NULL, this );
	m_ArtistLabel->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnArtistNameDClicked ), NULL, this );
	m_Rating->Connect( guEVT_RATING_CHANGED, guRatingEventHandler( guPlayerPanel::OnRatingChanged ), NULL, this );

    //
	m_PlayerCoverBitmap->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnLeftDClickPlayerCoverBitmap ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderBeginSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );

    m_PlayListCtrl->Connect( ID_PLAYER_PLAYLIST_UPDATELIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnPlayListUpdated ), NULL, this );
    m_PlayListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayListDClick ), NULL, this );

    m_MediaCtrl->Connect( wxEVT_MEDIA_LOADED, wxMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_ABOUT_TO_FINISH, wxMediaEventHandler( guPlayerPanel::OnMediaAboutToFinish ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_FINISHED, wxMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_TAG, wxMediaEventHandler( guPlayerPanel::OnMediaTag ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_BITRATE, wxMediaEventHandler( guPlayerPanel::OnMediaBitrate ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_BUFFERING, wxMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );

    Connect( ID_PLAYER_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracksClicked ) );

//    Connect( ID_PLAYERPANEL_UPDATERADIOTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnUpdatedRadioTrack ) );

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
        //printf( "guPlayerPanel::guConfig Save\n" );
        Config->WriteBool( wxT( "PlayerStopped" ), m_MediaCtrl->GetState() != wxMEDIASTATE_PLAYING, wxT( "General" ) );
        Config->WriteNum( wxT( "PlayerCurVol" ), m_CurVolume, wxT( "General" ) );
        Config->WriteBool( wxT( "PlayerLoop" ), m_PlayLoop, wxT( "General" ) );
        Config->WriteBool( wxT( "PlayerSmart" ), m_PlaySmart, wxT( "General" ) );
        // If the track length is at least the configured minimun track length save the pos offset
        if( Config->ReadBool( wxT( "SaveCurrentTrackPos" ), false, wxT( "General" ) ) )
        {
            if( ( m_LastPlayState != wxMEDIASTATE_STOPPED ) &&
                ( m_MediaSong.m_Length >= ( Config->ReadNum( wxT( "MinSavePlayPosLength" ), 10, wxT( "General" ) ) * 60 ) ) )
            {
                Config->WriteNum( wxT( "CurrentTrackPos" ), m_LastCurPos, wxT( "General" ) );
            }
            else
            {
                Config->WriteNum( wxT( "CurrentTrackPos" ), 0, wxT( "General" ) );
            }
        }
        //printf( PlaySmart ? "Smart Enabled" : "Smart Disabled" );  printf( "\n" );
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

    Disconnect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ) );

	m_AlbumLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnAlbumNameDClicked ), NULL, this );
	m_ArtistLabel->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnArtistNameDClicked ), NULL, this );
	m_Rating->Disconnect( guEVT_RATING_CHANGED, guRatingEventHandler( guPlayerPanel::OnRatingChanged ), NULL, this );

    //
	m_PlayerCoverBitmap->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnLeftDClickPlayerCoverBitmap ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderBeginSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );

    m_PlayListCtrl->Disconnect( ID_PLAYER_PLAYLIST_UPDATELIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnPlayListUpdated ), NULL, this );
    m_PlayListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayListDClick ), NULL, this );

    m_MediaCtrl->Disconnect( wxEVT_MEDIA_LOADED, wxMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_FINISHED, wxMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_TAG, wxMediaEventHandler( guPlayerPanel::OnMediaTag ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_BITRATE, wxMediaEventHandler( guPlayerPanel::OnMediaBitrate ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_BUFFERING, wxMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );

    Disconnect( ID_PLAYER_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracksClicked ) );

//    Disconnect( ID_PLAYERPANEL_UPDATERADIOTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnUpdatedRadioTrack ) );


    if( m_MediaCtrl )
        delete m_MediaCtrl;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetArtistLabel( const wxString &artistname )
{
    wxString Label = artistname;
    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_ArtistLabel->SetLabel( Label );
    //m_ArtistLabel->SetToolTip( Label );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetAlbumLabel( const wxString &albumname )
{
    wxString Label = albumname;
    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_AlbumLabel->SetLabel( Label );
    //m_AlbumLabel->SetToolTip( Label );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetTitleLabel( const wxString &trackname )
{
    wxString Label = trackname;
    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_TitleLabel->SetLabel( Label );
    //m_TitleLabel->SetToolTip( Label );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetRatingLabel( const int rating )
{
    m_Rating->SetRating( rating );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetLengthLabel( const int length )
{
    wxFileOffset CurPos = GetPosition();
    m_PositionLabel->SetLabel( LenToString( CurPos / 1000 ) + _( " of " ) + LenToString( length ) );
    m_PosLabelSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetBitRate( int bitrate )
{
    if( bitrate )
    {
        //guLogMessage( wxT( "Bitrate: %u" ), bitrate );
        m_BitRateLabel->SetLabel( wxString::Format( wxT( "[%ukbps]" ), bitrate / 1000 ) );
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
        // Reset the Smart added songs cache
        m_SmartAddedSongs.Empty();
        int count;
        int index = 0;
        count = SongList.Count();
        // We only insert the last CACHEITEMS as the rest should be forgiven
        if( count > GUPLAYER_SMART_CACHEITEMS )
            index = count - GUPLAYER_SMART_CACHEITEMS;
        for( ; index < count; index++ )
        {
            m_SmartAddedSongs.Add( SongList[ index ].m_ArtistName.Upper() + SongList[ index ].m_SongName.Upper() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const wxString &FileName )
{
    m_PlayListCtrl->AddPlayListItem( FileName );
    m_PlayListCtrl->ReloadItems();
    TrackListChanged();
    // TODO Need to add the track to the smart cache
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const guTrackArray &SongList )
{
    if( SongList.Count() )
    {
        guTrack * Track = &SongList[ 0 ];
        bool ClearPlayList = Track->m_TrackMode == guTRACK_MODE_RANDOM ||
                             Track->m_TrackMode == guTRACK_MODE_SMART;

        m_PlayListCtrl->AddToPlayList( SongList, ClearPlayList );

        TrackListChanged();

        if( m_PlaySmart )
        {
            // Add the Songs to the cache
            int count = SongList.Count();
            int index = 0;
            for( index = 0; index < count; index++ )
            {
                if( m_SmartAddedSongs.Index( SongList[ index ].m_ArtistName.Upper() +
                                 SongList[ index ].m_SongName.Upper() ) == wxNOT_FOUND )
                {
                    m_SmartAddedSongs.Add( SongList[ index ].m_ArtistName.Upper() + SongList[ index ].m_SongName.Upper() );
                    if( m_SmartAddedSongs.Count() > GUPLAYER_SMART_CACHEITEMS )
                    {
                        m_SmartAddedSongs.RemoveAt( 0 );
                    }
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::TrackListChanged( void )
{
//    m_PlayListLenStaticText->SetLabel( m_PlayListCtrl->GetLengthStr() );
//   	m_PlayListLabelsSizer->Layout();
    m_PlayListCtrl->SetColumnLabel( 0, _( "Now Playing" ) +
        wxString::Format( wxT( ":  %i / %i    ( %s )" ),
            m_PlayListCtrl->GetCurItem() + 1,
            m_PlayListCtrl->GetCount(),
            m_PlayListCtrl->GetLengthStr().c_str() ) );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListUpdated( wxCommandEvent &event )
{
    m_PlayListCtrl->ReloadItems();
    SetCurrentTrack( m_PlayListCtrl->GetCurrent() );

    // If a Player reset is needed
    if( event.GetExtraLong() )
    {
        OnStopButtonClick( event );
        OnPlayButtonClick( event );

        if( m_PlaySmart )
        {
            // Reset the Smart added songs cache
            m_SmartAddedSongs.Empty();
            int count;
            int index = 0;
            count = m_PlayListCtrl->GetCount();
            // We only insert the last CACHEITEMS as the rest should be forgiven
            if( count > GUPLAYER_SMART_CACHEITEMS )
                index = count - GUPLAYER_SMART_CACHEITEMS;
            for( ; index < count; index++ )
            {
                guTrack * Track = m_PlayListCtrl->GetItem( index );
                m_SmartAddedSongs.Add( Track->m_ArtistName.Upper() + Track->m_SongName.Upper() );
            }
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
            m_PlayButton->SetBitmapLabel( guImage( guIMAGE_INDEX_playback_pause ) );
        }
        else
        {
            m_PlayButton->SetBitmapLabel( guImage( guIMAGE_INDEX_playback_start ) );
        }
        m_PlayButton->Refresh();
        m_LastPlayState = State;
        //
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }

    if( State == wxMEDIASTATE_PLAYING )
    {
        //CurPos = m_MediaCtrl->Tell();
        CurPos = GetPosition();
        if( ( CurPos != m_LastCurPos ) && !m_SliderIsDragged )
        {
            m_PositionLabel->SetLabel( LenToString( CurPos / 1000 ) + _( " of " ) + LenToString( m_MediaSong.m_Length ) );
            m_PosLabelSizer->Layout();

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
            m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_volume_high ) );
        else if( m_CurVolume > 50 )
            m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_volume_medium ) );
        else if( m_CurVolume == 0 )
            m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_volume_muted ) );
        else
            m_VolumeButton->SetBitmapLabel( guImage( guIMAGE_INDEX_volume_low ) );
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
void guPlayerPanel::OnSmartAddTracksClicked( wxCommandEvent &event )
{
    guTrackArray NewSongs;
    guTrackArray InsertSongs;
    guTrack Song;
    guTrackArray * Songs = ( guTrackArray * ) event.GetClientData();
    int count;
    int index;
    if( Songs )
    {
        count = Songs->Count();
        for( index = 0; index < count; index++ )
        {
            Song = ( * Songs )[ index ];
            if( m_SmartAddedSongs.Index( Song.m_ArtistName.Upper() +
                             Song.m_SongName.Upper() ) == wxNOT_FOUND )
            {
                NewSongs.Add( Song );
            }
        }

        // Free the Songs object
        delete Songs;

        wxArrayInt AddedItems;
        count = NewSongs.Count();
        int SelIndex;
        // we try to find GUPLAYER_SMART_ADDTRACKS or all NewSongs if not enought
        for( index = 0; index < m_SmartPlayAddTracks; index++ )
        {
            // Only use random when dont need to add all tracks
            if( count > m_SmartPlayAddTracks )
            {
                do {
                    SelIndex = guRandom( count );
                } while( AddedItems.Index( SelIndex ) != wxNOT_FOUND );
                AddedItems.Add( SelIndex );
            }
            else
            {
                SelIndex = index;
                if( SelIndex >= count )
                  break;
            }

            // Add the track to the cache list
            m_SmartAddedSongs.Add( NewSongs[ SelIndex ].m_ArtistName.Upper() + NewSongs[ SelIndex ].m_SongName.Upper() );
            if( m_SmartAddedSongs.Count() > GUPLAYER_SMART_CACHEITEMS )
            {
                m_SmartAddedSongs.RemoveAt( 0 );
            }
            // Save the track to be Inserted later
            InsertSongs.Add( NewSongs[ SelIndex ] );
        }
        // If there was tracks to insert
        if( InsertSongs.Count() )
            AddToPlayList( InsertSongs );
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
    m_SmartAddTracksThread = new guSmartAddTracksThread( m_Db, this, &CurSong );
    if( m_SmartAddTracksThread )
    {
        m_SmartAddTracksThread->Create();
        m_SmartAddTracksThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        m_SmartAddTracksThread->Run();
    }
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
                if( !m_Db->AddCachedPlayedSong( m_MediaSong ) )
                    guLogError( wxT( "Could not add Song to CachedSongs Database" ) );
            }
        }
    }

    // Update the play count if it has player at least the half of the track
    if( ( m_MediaSong.m_Type == guTRACK_TYPE_DB ) ||
        ( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST ) )  // If its a song from the library
    {
        if( ( m_MediaSong.m_PlayTime > guAS_MIN_PLAYTIME ) || // If have played more than the min amount of time
            ( m_MediaSong.m_PlayTime >= ( m_MediaSong.m_Length / 2 ) ) )  // If have played at least the half
        {
            if( m_MediaSong.m_Type == guTRACK_TYPE_DB )
                m_Db->SetTrackPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount + 1 );
            else
                m_Db->SetPodcastItemPlayCount( m_MediaSong.m_SongId, m_MediaSong.m_PlayCount + 1 );
        }
    }


    // Set the Current Song
    m_MediaSong = * Song;

    // Update the Current Playing Song Info
    SetTitleLabel( m_MediaSong.m_SongName );
    SetAlbumLabel( m_MediaSong.m_AlbumName );
    SetArtistLabel( m_MediaSong.m_ArtistName );
    SetLengthLabel( m_MediaSong.m_Length );
    SetRatingLabel( m_MediaSong.m_Rating );

    if( m_MediaSong.m_Year > 0 )
    {
        m_YearLabel->SetLabel( wxString::Format( wxT( "%u" ), m_MediaSong.m_Year ) );
    }
    else
        m_YearLabel->SetLabel( wxEmptyString );

    SetBitRate( 0 );


    m_PlayListCtrl->SetColumnLabel( 0, _( "Now Playing" ) +
        wxString::Format( wxT( ":  %i / %i    ( %s )" ),
            m_PlayListCtrl->GetCurItem() + 1,
            m_PlayListCtrl->GetCount(),
            m_PlayListCtrl->GetLengthStr().c_str() ) );

    //guLogWarning( wxT( "SetCurrentTrack : CoverId = %u - %u" ), LastCoverId, m_MediaSong.CoverId );
    CoverImage = NULL;
    if( m_MediaSong.m_Type == guTRACK_TYPE_RADIOSTATION )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_net_radio ) );
        m_MediaSong.m_CoverType = GU_SONGCOVER_RADIO;
    }
    else if( m_MediaSong.m_Type == guTRACK_TYPE_PODCAST )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_podcast_icon ) );
        m_MediaSong.m_CoverType = GU_SONGCOVER_PODCAST;
    }
    else if( ( CoverImage = ID3TagGetPicture( m_MediaSong.m_FileName ) ) )
    {
        m_MediaSong.m_CoverType = GU_SONGCOVER_ID3TAG;
    }
    else if( m_MediaSong.m_CoverId )
    {
        //guLogMessage( wxT( "CoverId %i" ), m_MediaSong.CoverId );
        m_MediaSong.m_CoverPath = m_Db->GetCoverPath( m_MediaSong.m_CoverId );
        m_MediaSong.m_CoverType = GU_SONGCOVER_FILE;
    }
    else
    {
        //guLogWarning( wxT( "Trying to find covers in %s" ), wxPathOnly( m_MediaSong.FileName ).c_str() );
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
        delete CoverImage;
    }

    // Check if Smart is enabled
    if( ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) && m_PlaySmart &&
        ( ( m_PlayListCtrl->GetCurItem() + m_SmartPlayMinTracksToPlay ) > m_PlayListCtrl->GetCount() ) )
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

}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListDClick( wxCommandEvent &event )
{
    int item = event.GetInt();
    m_PlayListCtrl->SetCurrent( item );
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
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::LoadMedia( const wxString &FileName, bool restart )
{
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
                //SetAlbumLabel( m_MediaSong.AlbumName );
                SetArtistLabel( m_MediaSong.m_ArtistName );

                //guLogMessage( wxT( "Sending LastFMPanel::UpdateTrack event" ) );
                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
                event.SetClientData( new guTrackChangeInfo( m_MediaSong.m_ArtistName, m_MediaSong.m_SongName ) );
                wxPostEvent( wxTheApp->GetTopWindow(), event );
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
    try {
        //guLogMessage( wxT("OnMediaLoaded") );

        // If Enabled LastFM->Submit and no error then send Now Playing Information
        if( m_AudioScrobbleEnabled && m_AudioScrobble && m_AudioScrobble->IsOk() &&
            ( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION ) )
        {
            guAS_SubmitInfo SubmitInfo;
            //
            SubmitInfo.m_ArtistName = m_MediaSong.m_ArtistName;
            SubmitInfo.m_AlbumName  = m_MediaSong.m_AlbumName;
            SubmitInfo.m_TrackName  = m_MediaSong.m_SongName;
            SubmitInfo.m_TrackLen   = m_MediaSong.m_Length;
            SubmitInfo.m_TrackNum   = m_MediaSong.m_Number;
            //
            //AudioScrobble->SubmitNowPlaying( SubmitInfo );
            m_AudioScrobble->SetNowPlayingSong( SubmitInfo );
        }

        if( m_MediaSong.m_Type < guTRACK_TYPE_RADIOSTATION )
        {
            // Send an event so the LastFMPanel update its content.
            //guLogMessage( wxT( "Sending LastFMPanel::UpdateTrack event" ) );
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
            event.SetClientData( new guTrackChangeInfo( m_MediaSong.m_ArtistName, m_MediaSong.m_SongName ) );
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
    if( m_AboutToFinishPending )
    {
        //guLogMessage( wxT( "Ending About-To-Finish" ) );
        SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
        m_AboutToFinishPending = false;
        return;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFinished( wxMediaEvent &event )
{
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
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "RndTrackOnEmptyPlayList" ), false, wxT( "General" ) ) )
            {
                guTrackArray Tracks;
                if( m_Db->GetRandomTracks( &Tracks ) )
                {
                    AddToPlayList( Tracks );

                    OnMediaFinished( event );
                }
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
void guPlayerPanel::SetPlaySmart( bool playsmart )
{
    m_PlaySmart = playsmart;
    m_SmartPlayButton->SetValue( m_PlaySmart );
    if( m_PlaySmart && GetPlayLoop() )
    {
        SetPlayLoop( false );
    }
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::GetPlaySmart()
{
    return m_PlaySmart;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetPlayLoop( bool playloop )
{
    m_PlayLoop = playloop;
    m_RepeatPlayButton->SetValue( m_PlayLoop );
    if( m_PlayLoop && GetPlaySmart() )
    {
        SetPlaySmart( false );
    }
    //
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STATUSCHANGED );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::GetPlayLoop()
{
    return m_PlayLoop;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPrevTrackButtonClick( wxCommandEvent& event )
{
    wxMediaState State;
    wxFileOffset CurPos;
    guTrack * PrevItem;

    // If we are already in the first Item start again the song from the begining
    State = m_MediaCtrl->GetState();
    int CurItem = m_PlayListCtrl->GetCurItem();
    if( ( CurItem == 0 ) && ( State == wxMEDIASTATE_PLAYING ) )
    {
        m_MediaCtrl->Stop();
        m_MediaCtrl->Play();
        return;
    }

    PrevItem = m_PlayListCtrl->GetPrev( m_PlayLoop );
    if( PrevItem )
    {
        //State = m_MediaCtrl->GetState();
        if( State != wxMEDIASTATE_STOPPED )
        {
            CurPos = m_MediaCtrl->Tell();
            if( CurPos > GUPLAYER_MIN_PREVTRACK_POS ) // 5000
            {
                m_PlayListCtrl->GetNext( m_PlayLoop ); // <- Restore current track
                m_MediaCtrl->Stop();
                if( State == wxMEDIASTATE_PLAYING )
                {
                    m_MediaCtrl->Play();
                }
            }
            else
            {
                m_MediaCtrl->Stop();
                //m_MediaSong = * PrevItem;
                SetCurrentTrack( PrevItem );
                if( State == wxMEDIASTATE_PLAYING )
                {
                    LoadMedia( m_MediaSong.m_FileName );
                }
            }
        }
        else
        {
            SetCurrentTrack( PrevItem );
        }
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
    //event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnNextTrackButtonClick( wxCommandEvent& event )
{
    wxMediaState State;
    guTrack * NextItem;

//    wxMessageBox( wxT("OnPrevTrackButtonClick"), wxT("Event") );
    NextItem = m_PlayListCtrl->GetNext( m_PlayLoop );
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
            LoadMedia( m_MediaSong.m_FileName );
        }
        m_PlayListCtrl->RefreshAll( m_PlayListCtrl->GetCurItem() );
    }
    else
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "RndTrackOnEmptyPlayList" ), false, wxT( "General" ) ) )
            {
                SetPlayLoop( false );
                guTrackArray Tracks;
                if( m_Db->GetRandomTracks( &Tracks ) )
                {
                    AddToPlayList( Tracks );

                    OnNextTrackButtonClick( event );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayButtonClick( wxCommandEvent& event )
{
    wxMediaState State;

    // Get The Current Song From m_PlayListCtrl
    //guTrack * CurItem = m_PlayListCtrl->GetCurrent();
    //if( !m_MediaSong.m_SongId && m_PlayListCtrl->GetItemCount() )
    if( !m_MediaSong.m_Loaded && m_PlayListCtrl->GetItemCount() )
    {
        m_PlayListCtrl->SetCurrent( 0 );
        //m_MediaSong = * m_PlayListCtrl->GetCurrent();
        SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
    }
    if( m_MediaSong.m_Loaded )
    {
        State = m_MediaCtrl->GetState();
        if( State == wxMEDIASTATE_PLAYING )
        {
            m_MediaCtrl->Pause();
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
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "RndTrackOnEmptyPlayList" ), false, wxT( "General" ) ) )
            {
                SetPlayLoop( false );
                guTrackArray Tracks;
                if( m_Db->GetRandomTracks( &Tracks ) )
                {
                    AddToPlayList( Tracks );

                    OnPlayButtonClick( event );
                }
            }
        }
    }
    //wxLogMessage( wxT( "OnPlayButtonClick Id : %i" ), m_MediaSong.SongId );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnStopButtonClick( wxCommandEvent& event )
{
    wxMediaState State;
    State = m_MediaCtrl->GetState();
    if( State != wxMEDIASTATE_STOPPED )
    {
        m_MediaCtrl->Stop();
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
    m_PlayListCtrl->Randomize();
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, evt );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRepeatPlayButtonClick( wxCommandEvent &event )
{
    SetPlayLoop( !GetPlayLoop() );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnLeftDClickPlayerCoverBitmap( wxMouseEvent& event )
{
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

        m_PositionLabel->SetLabel( LenToString( CurPos / 1000 ) + _( " of " ) + LenToString( m_MediaSong.m_Length ) );
        m_PosLabelSizer->Layout();
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
        //guLogMessage( wxT( "Slider Set Pos to %i Of %i" ), ( int ) NewPos, 1000 );
        //printf( "SetPos: %llu\n", ( long long int ) NewPos * m_MediaSong.Length );
        //m_MediaCtrl->Seek( NewPos * m_MediaSong.m_Length );
        SetPosition( NewPos * m_MediaSong.m_Length );
    }
    m_SliderIsDragged = false;
}

// -------------------------------------------------------------------------------- //
float guPlayerPanel::GetVolume()
{
    return m_CurVolume;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetVolume( float volume )
{
    if( volume == m_CurVolume )
        return;

    if( volume < 0 )
        volume = 0;
    else if( volume > 100 )
        volume = 100;

    m_CurVolume = volume;

    m_MediaCtrl->SetVolume(  volume / 100.0 );
    m_VolumeButton->SetToolTip( wxString::Format( _( "Volume %u%%" ), ( int ) volume ) );
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
void guPlayerPanel::OnAlbumNameDClicked( wxMouseEvent &event )
{
    wxString * AlbumName = new wxString( m_MediaSong.m_AlbumName );
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUM_SELECTNAME );
    evt.SetClientData( ( void * ) AlbumName );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnArtistNameDClicked( wxMouseEvent &event )
{
    wxString * ArtistName = new wxString( m_MediaSong.m_ArtistName );
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ARTIST_SELECTNAME );
    evt.SetClientData( ( void * ) ArtistName );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnRatingChanged( guRatingEvent &event )
{
    m_MediaSong.m_Rating = event.GetInt();
    if( m_MediaSong.m_SongId > 0 )
    {
        m_Db->SetTrackRating( m_MediaSong.m_SongId, m_MediaSong.m_Rating );
        m_PlayListCtrl->UpdatedTrack( ( guTrack * ) &m_MediaSong );
    }
    else
    {
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
        if( ( * tracks )[ index ].m_SongId == m_MediaSong.m_SongId )
        {
            m_MediaSong = ( * tracks )[ index ];

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
            {
                m_YearLabel->SetLabel( wxEmptyString );
            }

            break;
        }
    }

    if( m_PlayListCtrl )
    {
        m_PlayListCtrl->UpdatedTracks( tracks );
    }
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
void guPlayerPanel::OnVolumenMouseWheel( wxMouseEvent &event )
{
    int Rotation = event.GetWheelRotation() / event.GetWheelDelta();
    //guLogMessage( wxT( "CurVol: %u  Rotations:%i" ), m_CurVolume, Rotation );
    SetVolume( m_CurVolume + ( Rotation * 4 ) );
}

// -------------------------------------------------------------------------------- //
// guSmartAddTracksThread
// -------------------------------------------------------------------------------- //
guSmartAddTracksThread::guSmartAddTracksThread( DbLibrary * NewDb,
                       guPlayerPanel * NewPlayer, const guTrack * NewSong ) : wxThread()
{
    m_Db = NewDb;
    m_PlayerPanel = NewPlayer;
    m_CurSong = NewSong;
}

// -------------------------------------------------------------------------------- //
guSmartAddTracksThread::~guSmartAddTracksThread()
{
//    printf( "guSmartAddTracksThread Object destroyed\n" );
  if( !TestDestroy() )
    m_PlayerPanel->m_SmartAddTracksThread = NULL;
};

// -------------------------------------------------------------------------------- //
guSmartAddTracksThread::ExitCode guSmartAddTracksThread::Entry()
{
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        //guLogMessage( wxT( "==== Getting Similar Tracks ====" ) );
        guSimilarTrackInfoArray SimilarTracks = LastFM->TrackGetSimilar( m_CurSong->m_ArtistName,
                                                                         m_CurSong->m_SongName );
        guTrackArray * Songs = NULL;
        guTrack * Song;
        if( SimilarTracks.Count() && !TestDestroy() )
        {
            Songs = new guTrackArray();
            if( Songs )
            {
                int count = SimilarTracks.Count();
                for( int index = 0; index < count; index++ )
                {
                  if( TestDestroy() )
                    break;
                  //guLogMessage( wxT( "Similar: '%s' - '%s'" ), SimilarTracks[ index ].ArtistName.c_str(), SimilarTracks[ index ].TrackName.c_str() );
                  Song = m_Db->FindSong( SimilarTracks[ index ].m_ArtistName, SimilarTracks[ index ].m_TrackName );
                  if( Song )
                  {
                      Song->m_TrackMode = guTRACK_MODE_SMART;
                      //guLogMessage( wxT( "Found this song in the Songs Library" ) );
                      Songs->Add( Song );
                  }
                }
            }
        }
        else if( !TestDestroy() )// No similar tracks so try to find similar artists
        {
            guSimilarArtistInfoArray SimilarArtists = LastFM->ArtistGetSimilar( m_CurSong->m_ArtistName );
            if( SimilarArtists.Count() && !TestDestroy() )
            {
                Songs = new guTrackArray();
                if( Songs )
                {
                    int ArtistId;
                    int count = SimilarArtists.Count();
                    for( int index = 0; index < count; index++ )
                    {
                        if( TestDestroy() )
                            break;
                        if( m_Db->GetArtistId( &ArtistId, SimilarArtists[ index ].m_Name, false ) )
                        {
                            wxArrayInt Artists;
                            Artists.Add( ArtistId );
                            m_Db->GetArtistsSongs( Artists, Songs, guTRACK_MODE_SMART );
                        }
                    }
                }
            }
        }
        if( !TestDestroy() )
        {
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYER_PLAYLIST_SMART_ADDTRACK );
            //event.SetEventObject( ( wxObject * ) this );
            event.SetClientData( ( void * ) Songs );
            wxPostEvent( m_PlayerPanel, event );
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
