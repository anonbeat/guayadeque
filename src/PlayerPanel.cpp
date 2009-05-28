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
//#include "StatusBar.h"
#include "Utils.h"
#include "VolumeFrame.h"

#include <wx/regex.h>

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

    m_Db = NewDb;
    m_BufferGaugeId = wxNOT_FOUND;

    // For the Load configuration
    wxArrayString Songs;
    guConfig * Config;

    m_LastVolume = wxNOT_FOUND;

    m_LastCurPos = -1;
    m_LastPlayState = wxMEDIASTATE_STOPPED;
    m_LastTotalLen = -1;
    //

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

	m_PrevTrackButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_media_skip_backward ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_PrevTrackButton->SetToolTip( _( "Go to Previous Track in the Playlist" ) );
	PlayerBtnSizer->Add( m_PrevTrackButton, 0, wxALL, 2 );

	m_NextTrackButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_media_skip_forward ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_NextTrackButton->SetToolTip( _( "Go to Next Track in the Playlist" ) );
	PlayerBtnSizer->Add( m_NextTrackButton, 0, wxALL, 2 );

	m_PlayButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_media_playback_start ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_PlayButton->SetToolTip( _( "Start playing or pauses current track in the Playlist" ) );
	PlayerBtnSizer->Add( m_PlayButton, 0, wxALL, 2 );

	m_StopButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_media_playback_stop ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_StopButton->SetToolTip( _( "Stops player reproduction" ) );
	PlayerBtnSizer->Add( m_StopButton, 0, wxALL, 2 );

	m_VolumenButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_audio_volume_medium ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_VolumenButton->SetToolTip( _( "Set player volumen" ) );
	PlayerBtnSizer->Add( m_VolumenButton, 0, wxALL, 2 );

	m_SmartPlayButton = new wxToggleBitmapButton( this, wxID_ANY, wxBitmap( guImage_smart_play ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_SmartPlayButton->SetToolTip( _( "Add tracks to the playlist bassed on LastFM" ) );
	// Get this value from config file
	m_SmartPlayButton->SetValue( m_PlaySmart );
	PlayerBtnSizer->Add( m_SmartPlayButton, 0, wxALL, 2 );

	m_RandomPlayButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_media_playlist_shuffle ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_RandomPlayButton->SetToolTip( _( "Randomize the tracks in the playlist" ) );
	PlayerBtnSizer->Add( m_RandomPlayButton, 0, wxALL, 2 );

	m_RepeatPlayButton = new wxToggleBitmapButton( this, wxID_ANY, wxBitmap( guImage_media_playlist_repeat ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_RepeatPlayButton->SetToolTip( _( "Repeats the current playlist" ) );
	m_RepeatPlayButton->SetValue( m_PlayLoop );
	PlayerBtnSizer->Add( m_RepeatPlayButton, 0, wxALL, 2 );


	PlayerMainSizer->Add( PlayerBtnSizer, 0, wxEXPAND, 5 );

	PlayerDetailsSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayerCoverBitmap = new wxStaticBitmap( this, wxID_ANY, wxBitmap( guImage_no_cover ), wxDefaultPosition, wxSize( 100,100 ), 0 );
	m_PlayerCoverBitmap->SetToolTip( _( "Shows the current track album cover if available" ) );
	PlayerDetailsSizer->Add( m_PlayerCoverBitmap, 0, wxALL, 2 );

	PlayerLabelsSizer = new wxBoxSizer( wxVERTICAL );

	m_TitleLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TitleLabel->SetToolTip( _( "Show the name of the current track" ) );
	m_TitleLabel->Wrap( -1 );
	m_TitleLabel->SetFont( wxFont( 16, 77, 90, 92, false, wxT("Arial") ) );

	PlayerLabelsSizer->Add( m_TitleLabel, 0, wxALL, 2 );

	m_AlbumLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumLabel->SetToolTip( _( "Show the album name of the current track" ) );
	m_AlbumLabel->Wrap( -1 );
	m_AlbumLabel->SetFont( wxFont( 12, 77, 93, 90, false, wxT( "Arial" ) ) );

	PlayerLabelsSizer->Add( m_AlbumLabel, 0, wxALL, 2 );

	m_ArtistLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistLabel->SetToolTip( _( "Show the artist name of the current track" ) );
	m_ArtistLabel->Wrap( -1 );
	m_ArtistLabel->SetFont( wxFont( 12, 74, 90, 90, false, wxT("Arial") ) );

	PlayerLabelsSizer->Add( m_ArtistLabel, 0, wxALL, 2 );

	m_PosLabelSizer = new wxBoxSizer( wxHORIZONTAL );
	m_PosLabelSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_PositionLabel = new wxStaticText( this, wxID_ANY, _("00:00 of 00:00"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PositionLabel->SetToolTip( _( "Show the current position and song length of the current track" ) );
	m_PositionLabel->Wrap( -1 );
	m_PositionLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_PosLabelSizer->Add( m_PositionLabel, 0, wxEXPAND|wxRIGHT, 4 );

	PlayerLabelsSizer->Add( m_PosLabelSizer, 1, wxEXPAND, 5 );

	PlayerDetailsSizer->Add( PlayerLabelsSizer, 1, wxEXPAND, 5 );

	PlayerMainSizer->Add( PlayerDetailsSizer, 0, wxEXPAND, 5 );

	m_PlayerPositionSlider = new wxSlider( this, wxID_ANY, 0, 0, 1000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	PlayerMainSizer->Add( m_PlayerPositionSlider, 0, wxALL|wxEXPAND, 0 );

	PlayListSizer = new wxBoxSizer( wxVERTICAL );

	PlayListPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	PlayListPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );

	PlayListPanelSizer = new wxBoxSizer( wxVERTICAL );

	m_PlayListLabelsSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayListStaticText = new wxStaticText( PlayListPanel, wxID_ANY, _(" Now Playing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayListStaticText->Wrap( -1 );
	//PlayListStaticText->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	m_PlayListLabelsSizer->Add( m_PlayListStaticText, 0, wxALL, 2 );
	m_PlayListLabelsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_PlayListLenStaticText = new wxStaticText( PlayListPanel, wxID_ANY, wxT("00:00"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayListLenStaticText->SetToolTip( _( "Shows the total length of the current playlist" ) );
	m_PlayListLenStaticText->Wrap( -1 );
	m_PlayListLabelsSizer->Add( m_PlayListLenStaticText, 0, wxALL, 2 );

	PlayListPanelSizer->Add( m_PlayListLabelsSizer, 0, wxEXPAND, 5 );

	m_PlayListCtrl = new guPlayList( PlayListPanel, wxID_ANY ); //, wxDefaultPosition, wxDefaultSize, wxLC_NO_HEADER|wxLC_REPORT );
	PlayListPanelSizer->Add( m_PlayListCtrl, 1, wxALL|wxEXPAND, 1 );

	PlayListPanel->SetSizer( PlayListPanelSizer );
	PlayListPanel->Layout();
	PlayListPanelSizer->Fit( PlayListPanel );
	PlayListSizer->Add( PlayListPanel, 1, wxEXPAND | wxALL, 2 );


	PlayerMainSizer->Add( PlayListSizer, 1, wxEXPAND, 5 );
	this->SetSizer( PlayerMainSizer );
	this->Layout();
	PlayerMainSizer->Fit( this );


    m_MediaCtrl = new guMediaCtrl();
    //m_MediaCtrl->Create( this, wxID_ANY );

    //
    m_PlayListCtrl->RefreshItems();
    TrackListChanged();
    SetVolume( m_CurVolume );

	// Connect Events
	m_PrevTrackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPrevTrackButtonClick ), NULL, this );
	m_NextTrackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnNextTrackButtonClick ), NULL, this );
	m_PlayButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayButtonClick ), NULL, this );
	m_StopButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnStopButtonClick ), NULL, this );
	m_VolumenButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnVolumenButtonClick ), NULL, this );
	m_SmartPlayButton->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnSmartPlayButtonClick ), NULL, this );
	m_RandomPlayButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ), NULL, this );
	m_RepeatPlayButton->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRepeatPlayButtonClick ), NULL, this );

    Connect( ID_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ) );

    //
	m_PlayerCoverBitmap->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnLeftDClickPlayerCoverBitmap ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderBeginSeek ), NULL, this );
	m_PlayerPositionSlider->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );

    m_PlayListCtrl->Connect( ID_PLAYLIST_UPDATELIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnPlayListUpdated ), NULL, this );
    m_PlayListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayListDClick ), NULL, this );

    m_MediaCtrl->Connect( wxEVT_MEDIA_LOADED, wxMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_FINISHED, wxMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_TAG, wxMediaEventHandler( guPlayerPanel::OnMediaTag ), NULL, this );
    m_MediaCtrl->Connect( wxEVT_MEDIA_BUFFERING, wxMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );

    Connect( ID_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracksClicked ) );

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
        //printf( PlaySmart ? "Smart Enabled" : "Smart Disabled" );  printf( "\n" );
    }

	// Connect Events
	m_PrevTrackButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPrevTrackButtonClick ), NULL, this );
	m_NextTrackButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnNextTrackButtonClick ), NULL, this );
	m_PlayButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayButtonClick ), NULL, this );
	m_StopButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnStopButtonClick ), NULL, this );
	m_VolumenButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnVolumenButtonClick ), NULL, this );
	m_SmartPlayButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnSmartPlayButtonClick ), NULL, this );
	m_RandomPlayButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ), NULL, this );
	m_RepeatPlayButton->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guPlayerPanel::OnRepeatPlayButtonClick ), NULL, this );

    Disconnect( ID_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnRandomPlayButtonClick ) );

    //
	m_PlayerCoverBitmap->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guPlayerPanel::OnLeftDClickPlayerCoverBitmap ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderBeginSeek ), NULL, this );
	m_PlayerPositionSlider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guPlayerPanel::OnPlayerPositionSliderEndSeek ), NULL, this );

    m_PlayListCtrl->Disconnect( ID_PLAYLIST_UPDATELIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnPlayListUpdated ), NULL, this );
    m_PlayListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPlayerPanel::OnPlayListDClick ), NULL, this );

    m_MediaCtrl->Disconnect( wxEVT_MEDIA_LOADED, wxMediaEventHandler( guPlayerPanel::OnMediaLoaded ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_FINISHED, wxMediaEventHandler( guPlayerPanel::OnMediaFinished ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_TAG, wxMediaEventHandler( guPlayerPanel::OnMediaTag ), NULL, this );
    m_MediaCtrl->Disconnect( wxEVT_MEDIA_BUFFERING, wxMediaEventHandler( guPlayerPanel::OnMediaBuffering ), NULL, this );

    Disconnect( ID_PLAYLIST_SMART_ADDTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayerPanel::OnSmartAddTracksClicked ) );

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
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetAlbumLabel( const wxString &albumname )
{
    wxString Label = albumname;
    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_AlbumLabel->SetLabel( Label );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetTitleLabel( const wxString &trackname )
{
    wxString Label = trackname;
    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_TitleLabel->SetLabel( Label );
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
    m_PlayListCtrl->RefreshItems();
    TrackListChanged();
    // TODO Need to add the track to the smart cache
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::AddToPlayList( const guTrackArray &SongList )
{
    m_PlayListCtrl->AddToPlayList( SongList, m_PlaySmart );
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

// -------------------------------------------------------------------------------- //
void guPlayerPanel::TrackListChanged( void )
{
    m_PlayListLenStaticText->SetLabel( m_PlayListCtrl->GetLengthStr() );
   	m_PlayListLabelsSizer->Layout();

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayListUpdated( wxCommandEvent &event )
{
    m_PlayListCtrl->RefreshItems();
    TrackListChanged();
    SetCurrentTrack( m_PlayListCtrl->GetCurrent() );
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
            m_PlayButton->SetBitmapLabel( wxBitmap( guImage_media_playback_pause ) );
        }
        else
        {
            m_PlayButton->SetBitmapLabel( wxBitmap( guImage_media_playback_start ) );
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
        //ReadVol = m_MediaCtrl->GetVolume() * 100;
        if( m_CurVolume > 75 )
            m_VolumenButton->SetBitmapLabel( wxBitmap( guImage_audio_volume_high ) );
        else if( m_CurVolume > 50 )
            m_VolumenButton->SetBitmapLabel( wxBitmap( guImage_audio_volume_medium ) );
        else if( m_CurVolume > 25 )
            m_VolumenButton->SetBitmapLabel( wxBitmap( guImage_audio_volume_low ) );
        else if( m_CurVolume == 0 )
            m_VolumenButton->SetBitmapLabel( wxBitmap( guImage_audio_volume_muted ) );
        m_VolumenButton->Refresh();
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
    m_PlayListCtrl->RefreshItems();
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
    if( m_AudioScrobbleEnabled && ( m_MediaSong.m_SongId != guPLAYLIST_RADIOSTATION ) ) // If its not a radiostation
    {
        // TODO : Control when the track have been added automatically or manually
        //        to submit it the correct source to LastFM
        //guLogMessage( wxT( "PlayTime: %u Length: %u" ), m_MediaSong.PlayTime, m_MediaSong.Length );
        if( ( ( m_MediaSong.m_PlayTime > guAS_MIN_PLAYTIME ) || // If have played more than the min amount of time
            ( m_MediaSong.m_PlayTime >= ( m_MediaSong.m_Length / 2 ) ) ) && // If have played at least the half
            ( m_MediaSong.m_PlayTime > guAS_MIN_TRACKLEN ) )    // If the Length is more than 30 secs
        {
            if( !m_MediaSong.m_SongName.IsEmpty() &&    // Check if we have no missing data
                !m_MediaSong.m_AlbumName.IsEmpty() &&
                !m_MediaSong.m_ArtistName.IsEmpty() )
            {
                if( !m_Db->AddCachedPlayedSong( m_MediaSong ) )
                    guLogError( wxT( "Could not add Song to CachedSongs Database" ) );
            }
        }
    }

    // Set the Current Song
    m_MediaSong = * Song;

    // Update the Current Playing Song Info
    SetTitleLabel( m_MediaSong.m_SongName );
    SetAlbumLabel( m_MediaSong.m_AlbumName );
    SetArtistLabel( m_MediaSong.m_ArtistName );

    //guLogWarning( wxT( "SetCurrentTrack : CoverId = %u - %u" ), LastCoverId, m_MediaSong.CoverId );
    CoverImage = NULL;
    //guLogMessage( "CoverId : %i", m_MediaSong.CoverId );
    if( m_MediaSong.m_SongId == guPLAYLIST_RADIOSTATION )
    {
        CoverImage = new wxImage( guImage_net_radio );
        m_MediaSong.m_CoverType = GU_SONGCOVER_RADIO;
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

    if( !CoverImage )
    {
        if( m_MediaSong.m_CoverPath.IsEmpty() || !wxFileExists( m_MediaSong.m_CoverPath ) )
        {
            //printf( "No coverpath set\n" );
            CoverImage = new wxImage( guImage_no_cover );
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
    if( ( m_MediaSong.m_SongId !=  guPLAYLIST_RADIOSTATION ) && m_PlaySmart &&
        ( ( m_PlayListCtrl->GetCurItem() + m_SmartPlayMinTracksToPlay ) > m_PlayListCtrl->GetCount() ) )
    {
        SmartAddTracks( m_MediaSong );
    }

    // If its a Radio disable PositionSlider
    m_PlayerPositionSlider->SetValue( 0 );
    if( m_MediaSong.m_SongId == guPLAYLIST_RADIOSTATION )
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
void guPlayerPanel::LoadMedia( const wxString &FileName )
{
    //m_MediaCtrl->Load( NextItem->FileName );
    wxURI UriPath( FileName );
    wxString Uri;
    try {
        if( !UriPath.HasScheme() )
            Uri = wxT( "file:////" ) + FileName;
        else
            Uri = FileName;
        if( !m_MediaCtrl->Load( Uri ) )
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
              m_BufferGaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge();
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
        if( m_MediaSong.m_SongId == guPLAYLIST_RADIOSTATION )
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
                wxArrayString * Params = new wxArrayString();
                Params->Add( m_MediaSong.m_ArtistName );
                Params->Add( m_MediaSong.m_SongName );
                event.SetClientData( Params );
                wxPostEvent( wxTheApp->GetTopWindow(), event );
            }
        }
        delete TagStr;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaLoaded( wxMediaEvent &event )
{
    try {
        //guLogMessage( wxT("OnMediaLoaded") );

        // If Enabled LastFM->Submit and no error then send Now Playing Information
        if( m_AudioScrobbleEnabled && m_AudioScrobble && m_AudioScrobble->IsOk() &&
            ( m_MediaSong.m_SongId != guPLAYLIST_RADIOSTATION ) )
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

        if( m_MediaSong.m_SongId != guPLAYLIST_RADIOSTATION )
        {
            // Send an event so the LastFMPanel update its content.
            //guLogMessage( wxT( "Sending LastFMPanel::UpdateTrack event" ) );
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
            wxArrayString * Params = new wxArrayString();
            Params->Add( m_MediaSong.m_ArtistName );
            Params->Add( m_MediaSong.m_SongName );
            event.SetClientData( Params );
            wxPostEvent( wxTheApp->GetTopWindow(), event );
        }

        //
        m_MediaCtrl->Play();
        m_PlayListCtrl->UpdateView();
//        SetVolume( CurVolume );
    }
    catch(...)
    {
        OnNextTrackButtonClick( event );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnMediaFinished( wxMediaEvent &event )
{
    guTrack * NextItem = m_PlayListCtrl->GetNext( m_PlayLoop );
    if( NextItem )
    {
        //m_MediaSong = * NextItem;
        SetCurrentTrack( NextItem );
        LoadMedia( NextItem->m_FileName );
        m_PlayListCtrl->UpdateView();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPrevTrackButtonClick( wxCommandEvent& event )
{
    wxMediaState State;
    wxFileOffset CurPos;
    guTrack * PrevItem;

//    wxMessageBox( wxT("OnPrevTrackButtonClick"), wxT("Event") );
    PrevItem = m_PlayListCtrl->GetPrev( m_PlayLoop );
    if( PrevItem )
    {
        State = m_MediaCtrl->GetState();
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
        m_PlayListCtrl->UpdateView();
    }
    //event.Skip();
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
        m_PlayListCtrl->UpdateView();
    }
    //event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::OnPlayButtonClick( wxCommandEvent& event )
{
    wxMediaState State;
    // Get The Current Song From m_PlayListCtrl
    //guTrack * CurItem = m_PlayListCtrl->GetCurrent();
    if( !m_MediaSong.m_SongId && m_PlayListCtrl->GetItemCount() )
    {
        m_PlayListCtrl->SetCurrent( 0 );
        m_MediaSong = * m_PlayListCtrl->GetCurrent();
    }
    if( m_MediaSong.m_SongId )
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
        m_PlayListCtrl->UpdateView();
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
    Pos = ClientToScreen( m_VolumenButton->GetPosition() );
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
    if( m_MediaSong.m_SongId )
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
int guPlayerPanel::GetVolume()
{
    return m_CurVolume;
}

// -------------------------------------------------------------------------------- //
void guPlayerPanel::SetVolume( int Vol )
{
    m_CurVolume = Vol;
//    if( m_MediaCtrl->GetState() != wxMEDIASTATE_STOPPED )
//    {
        m_MediaCtrl->SetVolume( ( ( double ) Vol ) / 100 );
//    }
}

// -------------------------------------------------------------------------------- //
bool guPlayerPanel::SetPosition( int pos )
{
    return m_MediaCtrl->Seek( pos );
}

// -------------------------------------------------------------------------------- //
int guPlayerPanel::GetPosition()
{
    return m_MediaCtrl->Tell();
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
                            m_Db->GetArtistsSongs( Artists, Songs );
                        }
                    }
                }
            }
        }
        if( !TestDestroy() )
        {
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_SMART_ADDTRACK );
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
