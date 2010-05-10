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
#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include "AudioScrobble.h"
#include "AutoScrollText.h"
#include "dbus/notify.h"
#include "MediaCtrl.h"
#include "PlayerFilters.h"
#include "PlayList.h"
#include "RatingCtrl.h"
#include "RoundButton.h"
#include "StaticBitmap.h"
#include "ToggleRoundButton.h"
#include "Vumeters.h"


#include <wx/aui/aui.h>
#include <wx/wx.h>
#include <wx/dnd.h>
//#include <wx/mediactrl.h>
#include <wx/tglbtn.h>

// -------------------------------------------------------------------------------- //
enum guSongCoverType {
    GU_SONGCOVER_NONE = 0,
    GU_SONGCOVER_FILE,
    GU_SONGCOVER_ID3TAG,
    GU_SONGCOVER_RADIO,
    GU_SONGCOVER_PODCAST
};

// -------------------------------------------------------------------------------- //
enum guPlayLoopMode {
    guPLAYER_PLAYLOOP_NONE = 0,
    guPLAYER_PLAYLOOP_PLAYLIST,
    guPLAYER_PLAYLOOP_TRACK
};

// -------------------------------------------------------------------------------- //
class guCurrentTrack : public guTrack
{
  public:
    // Only for the Current Played song
    bool            m_Loaded;
    unsigned int    m_PlayTime;           // how many secs the song have been played
    guSongCoverType m_CoverType;
    wxString        m_CoverPath;

    guCurrentTrack()
    {
        m_Loaded = false;
    }

    guCurrentTrack& operator=(const guTrack &Src)
    {
        m_Loaded = true;
        m_Type = Src.m_Type;
        m_SongId = Src.m_SongId;
        m_SongName = Src.m_SongName;
        m_AlbumId = Src.m_AlbumId;
        m_AlbumName = Src.m_AlbumName;
        m_ArtistId = Src.m_ArtistId;
        m_ArtistName = Src.m_ArtistName;
        m_GenreId = Src.m_GenreId;
        m_GenreName = Src.m_GenreName;
        m_PathId = Src.m_PathId;
        m_FileName = Src.m_FileName;
        m_Number = Src.m_Number;
        m_Year = Src.m_Year;
        m_Length = Src.m_Length;
        m_Bitrate = Src.m_Bitrate;
        m_PlayCount = Src.m_PlayCount;
        m_Rating = Src.m_Rating;
        m_LastPlay = Src.m_LastPlay;
        m_AddedTime = Src.m_AddedTime;
        m_CoverId = Src.m_CoverId;
        m_Disk = Src.m_Disk;
        m_Comments = Src.m_Comments;
        m_Composer = Src.m_Composer;
        m_PlayTime = 0;

        //CoverType = GU_SONGCOVER_NONE;
        if( m_Type == guTRACK_TYPE_RADIOSTATION )
        {
            m_CoverType = GU_SONGCOVER_RADIO;
        }
        else if( m_Type == guTRACK_TYPE_PODCAST )
        {
            m_CoverType = GU_SONGCOVER_PODCAST;
        }
        else if( m_CoverId )
        {
            m_CoverType = GU_SONGCOVER_FILE;
        }
        else
        {
            m_CoverType = GU_SONGCOVER_NONE;
        }
        m_CoverPath = wxEmptyString;
        return *this;
    }

    void Update( const guTrack &track )
    {
        m_Loaded = true;
        m_Type = track.m_Type;
        m_SongId = track.m_SongId;
        m_SongName = track.m_SongName;
        m_AlbumId = track.m_AlbumId;
        m_AlbumName = track.m_AlbumName;
        m_ArtistId = track.m_ArtistId;
        m_ArtistName = track.m_ArtistName;
        m_GenreId = track.m_GenreId;
        m_GenreName = track.m_GenreName;
        m_PathId = track.m_PathId;
        m_FileName = track.m_FileName;
        m_Number = track.m_Number;
        m_Year = track.m_Year;
        m_Length = track.m_Length;
        m_Bitrate = track.m_Bitrate;
        m_PlayCount = track.m_PlayCount;
        m_Rating = track.m_Rating;
        m_LastPlay = track.m_LastPlay;
        m_AddedTime = track.m_AddedTime;
        m_CoverId = track.m_CoverId;
    }
};

class guPlayerPanelTimer;
class guSmartAddTracksThread;

// -------------------------------------------------------------------------------- //
class guPlayerPanel : public wxPanel
{
  private:
	guRoundButton *         m_PrevTrackButton;
	guRoundButton *         m_NextTrackButton;
	guRoundButton *         m_PlayButton;
	guRoundButton *         m_StopButton;
	guToggleRoundButton *   m_RecordButton;
	guRoundButton *         m_VolumeButton;
	//
	guToggleRoundButton *   m_SmartPlayButton;
	guRoundButton *         m_RandomPlayButton;
	guToggleRoundButton *   m_RepeatPlayButton;
	guRoundButton *         m_EqualizerButton;
	//
	guStaticBitmap *        m_PlayerCoverBitmap;
	//wxStaticText *          m_TitleLabel;
	guAutoScrollText *      m_TitleLabel;
	//wxStaticText *          m_AlbumLabel;
	guAutoScrollText *      m_AlbumLabel;
	//wxStaticText *          m_ArtistLabel;
	guAutoScrollText *      m_ArtistLabel;
	wxStaticText *          m_YearLabel;
    guRating *              m_Rating;
    wxBoxSizer *            m_BitRateSizer;
	wxStaticText *          m_BitRateLabel;
	wxBoxSizer *            m_PosLabelSizer;
	wxStaticText *          m_PositionLabel;
	wxSlider *              m_PlayerPositionSlider;

    guDbLibrary *           m_Db;
	guPlayList *            m_PlayListCtrl;
	guDBusNotify *          m_NotifySrv;
	guPlayerFilters *       m_PlayerFilters;
	guPlayerVumeters *      m_PlayerVumeters;
	guMediaCtrl *           m_MediaCtrl;
	guMediaRecordCtrl *     m_MediaRecordCtrl;
	guPlayerPanelTimer *    m_PlayerTimer;
    guCurrentTrack          m_MediaSong;
	int                     m_LastPlayState;
	double                  m_LastVolume;
	wxFileOffset            m_LastCurPos;
	wxFileOffset            m_LastLength;
	bool                    m_IsSkipping;
	bool                    m_ShowNotifications;
	int                     m_ShowNotificationsTime;

	double                  m_CurVolume;
	int                     m_PlayLoop;
	bool                    m_PlaySmart;
	bool                    m_PlayRandom;
	int                     m_PlayRandomMode;
	bool                    m_SliderIsDragged;
	long                    m_LastTotalLen;

	wxArrayInt              m_SmartAddedTracks;
	wxArrayString           m_SmartAddedArtists;
	bool                    m_SmartSearchEnabled;
    int                     m_SmartPlayAddTracks;
    int                     m_SmartPlayMinTracksToPlay;

    bool                    m_DelTracksPlayed;

    unsigned int            m_TrackStartPos;

    bool                    m_SilenceDetector;
    int                     m_SilenceDetectorLevel;
    int                     m_SilenceDetectorTime;

    int                     m_FadeOutTime;

    bool                    m_PendingScrob;

    // AudioScrobble
    guAudioScrobble *       m_AudioScrobble;
    bool                    m_AudioScrobbleEnabled;
    guSmartAddTracksThread * m_SmartAddTracksThread;

    int                     m_BufferGaugeId;

    bool                    m_AboutToFinishPending;

    bool                    m_ShowRevTime;

    wxArrayString           m_ErrorStrings;

    bool                    m_PendingNewRecordName;

    bool                    m_TrackChanged;

	void                OnVolumenButtonClick( wxCommandEvent &event );
	void                OnVolumenMouseWheel( wxMouseEvent &event );
    void                OnPlayerCoverBitmapMouseOver( wxCommandEvent &event );
    void                OnPlayerPositionSliderBeginSeek( wxScrollEvent &event );
    void                OnPlayerPositionSliderEndSeek( wxScrollEvent &event );
    void                OnPlayerPositionSliderChanged( wxScrollEvent &event );
    void                OnPlayerPositionSliderMouseWheel( wxMouseEvent &event );
    //
    void                OnPlayListUpdated( wxCommandEvent &event );
    void                OnPlayListDClick( wxCommandEvent &event );

    void                LoadMedia( const wxString &FileName, guPlayerPlayType playtype );
    void                OnMediaLoaded( guMediaEvent &event );
    void                OnMediaPlayStarted( void );
    void                OnMediaAboutToFinish( guMediaEvent &event );
    void                OnMediaFinished( guMediaEvent &event );
    void                OnMediaFadeOutFinished( guMediaEvent &event );
    void                OnMediaFadeInStarted( guMediaEvent &event );
    void                OnMediaTags( guMediaEvent &event );
    void                OnMediaBitrate( guMediaEvent &event );
    void                OnMediaBuffering( guMediaEvent &event );
    void                OnMediaLevel( guMediaEvent &event );
    void                OnMediaError( guMediaEvent &event );
    void                OnMediaState( guMediaEvent &event );
    void                OnMediaPosition( guMediaEvent &event );
    void                OnMediaLength( guMediaEvent &event );


    void                SetCurrentTrack( const guTrack * Song );

    // SmartPlay Events
    void                SmartAddTracks( const guTrack &CurSong );
    void                OnSmartAddTracks( wxCommandEvent &event );
    void                OnUpdatedRadioTrack( wxCommandEvent &event );

    void                OnTitleNameDClicked( wxMouseEvent &event );
    void                OnAlbumNameDClicked( wxMouseEvent &event );
    void                OnArtistNameDClicked( wxMouseEvent &event );
    void                OnYearDClicked( wxMouseEvent &event );
    void                OnTimeDClicked( wxMouseEvent &event ) { m_ShowRevTime = !m_ShowRevTime;
                                                                UpdatePositionLabel( GetPosition() / 1000 ); };
    void                OnRatingChanged( guRatingEvent &event );
    void                CheckFiltersEnable( void );

    void                OnConfigUpdated( wxCommandEvent &event );

    void                SendRecordSplitEvent( void );

  public:
                        guPlayerPanel( wxWindow* parent, guDbLibrary * db,
                                       guPlayList * playlist, guPlayerFilters * filters );
                        ~guPlayerPanel();
    void                UpdateStatus();
    void                SetPlayList( const guTrackArray &SongList );
    void                AddToPlayList( const guTrackArray &SongList, const bool allowplay = true );
    void                AddToPlayList( const wxString &FileName );
    void                AddToPlayList( const wxArrayString &files );
    void                ClearPlayList( void ) { m_PlayListCtrl->ClearItems(); };
    void                SetPlayList( const wxArrayString &files );

    double              GetVolume() { return m_CurVolume; }
    void                SetVolume( double volume );
    bool                SetPosition( int pos );
    int                 GetPosition();
    void                TrackListChanged( void );
    const guCurrentTrack * GetCurrentTrack();
    int                 GetCurrentItem();
    int                 GetItemCount();
    const guTrack *     GetTrack( int index );
    void                RemoveItem( int itemnum );

    int                 GetPlayLoop();
    void                SetPlayLoop( int playloop );
    bool                GetPlaySmart();
    void                SetPlaySmart( bool playsmart );
    void                UpdatePlayListFilters( void );

    int                 GetCaps();

    const guMediaState  GetState( void );
    void                OnPrevTrackButtonClick( wxCommandEvent &event );
    void                OnNextTrackButtonClick( wxCommandEvent &event );
    void                OnPlayButtonClick( wxCommandEvent &event );
    void                OnStopButtonClick( wxCommandEvent &event );
    void                OnRecordButtonClick( wxCommandEvent &event );
	void                OnSmartPlayButtonClick( wxCommandEvent &event );
	void                OnRandomPlayButtonClick( wxCommandEvent &event );
	void                OnRepeatPlayButtonClick( wxCommandEvent &event );
	void                OnEqualizerButtonClicked( wxCommandEvent &event );

    void                OnAboutToFinish( void );

    void                SetArtistLabel( const wxString &artistname );
    void                SetAlbumLabel( const wxString &albumname );
    void                SetTitleLabel( const wxString &trackname );
    void                SetRatingLabel( const int Rating );
    void                SetRating( const int rating );
    void                UpdatePositionLabel( const unsigned int curpos );
    void                SetBitRateLabel( const int bitrate );
    void                SetBitRate( int bitrate );

    void                UpdatedTracks( const guTrackArray * tracks );
    void                UpdatedTrack( const guTrack * track );

    void                UpdateLabels( void );

    void                SetPlayerVumeters( guPlayerVumeters * vumeters ) { m_PlayerVumeters = vumeters; };
    void                ResetVumeterLevel( void );

    void                SetNotifySrv( guDBusNotify * notify ) { m_NotifySrv = notify; };
    void                SendNotifyInfo( wxImage * image );


    friend class guSmartAddTracksThread;
};

// -------------------------------------------------------------------------------- //
class guPlayerPanelTimer : public wxTimer
{
public:
    //Ctor
    guPlayerPanelTimer( guPlayerPanel * NewPlayer ) { Player = NewPlayer; }

    //Called each time the timer's timeout expires
    void Notify();

    guPlayerPanel * Player;       //
};

// -------------------------------------------------------------------------------- //
class guSmartAddTracksThread : public wxThread
{
  protected :
    guDbLibrary *   m_Db;
    guPlayerPanel * m_PlayerPanel;
    const guTrack * m_CurSong;
    int             m_TrackCount;
    int             m_FilterAllowPlayList;
    int             m_FilterDenyPlayList;
    wxArrayInt *    m_SmartAddedTracks;
    wxArrayString * m_SmartAddedArtists;

    void           AddSimilarTracks( const wxString &artist, const wxString &track, guTrackArray * songs );

  public:
    guSmartAddTracksThread( guDbLibrary * db, guPlayerPanel * playerpanel, const guTrack * track,
             wxArrayInt * smartaddedtracks, wxArrayString * smartaddedartists,
             const int trackcount, const int filterallow, const int filterdeny );
    ~guSmartAddTracksThread();

    virtual ExitCode Entry();
};

#endif
// -------------------------------------------------------------------------------- //