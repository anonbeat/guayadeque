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
#ifndef __PLAYERPANEL_H__
#define __PLAYERPANEL_H__

#include "AudioScrobble.h"
#include "AutoScrollText.h"
#include "dbus/notify.h"
#include "LastFM.h"
#include "MediaCtrl.h"
#include "MediaEvent.h"
#include "MediaRecordCtrl.h"
#include "PlayerFilters.h"
#include "PlayList.h"
#include "RatingCtrl.h"
#include "RoundButton.h"
#include "SmartMode.h"
#include "StaticBitmap.h"
#include "ToggleRoundButton.h"
#include "Vumeters.h"


#include <wx/aui/aui.h>
#include <wx/wx.h>
#include <wx/dnd.h>
//#include <wx/mediactrl.h>
#include <wx/tglbtn.h>

namespace Guayadeque {

#define guTEMPORARY_COVER_FILENAME      wxT( "guayadeque-tmp-cover" )

enum guInsertAfterCurrent {
    guINSERT_AFTER_CURRENT_NONE = 0,
    guINSERT_AFTER_CURRENT_TRACK,
    guINSERT_AFTER_CURRENT_ALBUM,
    guINSERT_AFTER_CURRENT_ARTIST,
};

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
    wxImage *       m_CoverImage;
    int             m_ASRating;

    guCurrentTrack()
    {
        m_Loaded = false;
        m_CoverImage = NULL;
        m_ASRating = guAS_RATING_NONE;
        m_Number = 0;
        m_Rating = 0;
        m_Year = 0;
        m_Offset = 0;
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
        m_AlbumArtistId = Src.m_AlbumArtistId;
        m_AlbumArtist = Src.m_AlbumArtist;
        m_GenreId = Src.m_GenreId;
        m_GenreName = Src.m_GenreName;
        m_PathId = Src.m_PathId;
        m_FileName = Src.m_FileName;
        m_Number = Src.m_Number;
        m_Year = Src.m_Year;
        m_Offset = Src.m_Offset;
        m_Length = Src.m_Length;
        m_Bitrate = Src.m_Bitrate;
        m_PlayCount = Src.m_PlayCount;
        m_Rating = Src.m_Rating;
        m_LastPlay = Src.m_LastPlay;
        m_AddedTime = Src.m_AddedTime;
        m_CoverId = Src.m_CoverId;
        m_Disk = Src.m_Disk;
        m_Comments = Src.m_Comments;
        m_ComposerId = Src.m_ComposerId;
        m_Composer = Src.m_Composer;
        m_PlayTime = 0;
        m_ASRating = guAS_RATING_NONE;
        m_MediaViewer = Src.m_MediaViewer;
        guLogMessage( wxT( "Track starts at %i with length %i" ), m_Offset, m_Length );

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
        if( m_CoverImage )
            delete m_CoverImage;
        m_CoverImage = NULL;
        return *this;
    }

    ~guCurrentTrack()
    {
        if( m_CoverImage )
        {
            delete m_CoverImage;
        }
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
        m_AlbumArtistId = track.m_AlbumArtistId;
        m_AlbumArtist = track.m_AlbumArtist;
        m_GenreId = track.m_GenreId;
        m_GenreName = track.m_GenreName;
        m_PathId = track.m_PathId;
        m_FileName = track.m_FileName;
        m_Number = track.m_Number;
        m_Year = track.m_Year;
        m_Offset = track.m_Offset;
        m_Length = track.m_Length;
        m_Bitrate = track.m_Bitrate;
        m_PlayCount = track.m_PlayCount;
        m_Rating = track.m_Rating;
        m_LastPlay = track.m_LastPlay;
        m_AddedTime = track.m_AddedTime;
        m_CoverId = track.m_CoverId;
        m_Disk = track.m_Disk;
        m_Comments = track.m_Comments;
        m_ComposerId = track.m_ComposerId;
        m_Composer = track.m_Composer;
        m_MediaViewer = track.m_MediaViewer;
        if( m_CoverImage )
        {
            delete m_CoverImage;
            m_CoverImage = NULL;
        }
    }

    void SetCoverImage( wxImage * image )
    {
        if( m_CoverImage )
        {
            delete m_CoverImage;
        }
        m_CoverImage = image;
    }
};

class guSmartAddTracksThread;
class guUpdatePlayerCoverThread;

// -------------------------------------------------------------------------------- //
class guPlayerPanel : public wxPanel
{
  private:
	guRoundButton *             m_PrevTrackButton;
	guRoundButton *             m_NextTrackButton;
	guRoundButton *             m_PlayButton;
	guToggleRoundButton *       m_RecordButton;
	guRoundButton *             m_VolumeButton;
	//
	guToggleRoundButton *       m_SmartPlayButton;
	guRoundButton *             m_RandomPlayButton;
	guToggleRoundButton *       m_RepeatPlayButton;
	guRoundButton *             m_EqualizerButton;
	//
	wxStaticBitmap *            m_PlayerCoverBitmap;
	guAutoScrollText *          m_TitleLabel;
	guAutoScrollText *          m_AlbumLabel;
	guAutoScrollText *          m_ArtistLabel;
	wxStaticText *              m_YearLabel;
    guRating *                  m_Rating;
    guToggleRoundButton *       m_LoveBanButton;
    wxBoxSizer *                m_BitRateSizer;
	wxStaticText *              m_BitRateLabel;
	wxBoxSizer *                m_PosLabelSizer;
	wxStaticText *              m_PositionLabel;
	wxSlider *                  m_PlayerPositionSlider;

    guDbLibrary *               m_Db;
    guMainFrame *               m_MainFrame;
	guPlayList *                m_PlayListCtrl;
	guDBusNotify *              m_NotifySrv;
	guPlayerFilters *           m_PlayerFilters;
	guPlayerVumeters *          m_PlayerVumeters;
	guMediaCtrl *               m_MediaCtrl;
	guMediaRecordCtrl *         m_MediaRecordCtrl;
    guCurrentTrack              m_MediaSong;
    guTrack                     m_NextSong;
	int                         m_LastPlayState;
	double                      m_LastVolume;
	wxFileOffset                m_LastCurPos;
	wxFileOffset                m_LastLength;
	bool                        m_IsSkipping;
	bool                        m_ShowNotifications;
	int                         m_ShowNotificationsTime;

	double                      m_CurVolume;
	int                         m_PlayLoop;
	bool                        m_PlaySmart;
	bool                        m_PlayRandom;
	int                         m_PlayRandomMode;
	bool                        m_SliderIsDragged;
	long                        m_LastTotalLen;

	bool                        m_SilenceDetected;
	bool                        m_AboutToEndDetected;
	bool                        m_FadeInStarted;

	wxArrayInt                  m_SmartAddedTracks;
	wxArrayString               m_SmartAddedArtists;
	int                         m_SmartMaxTracksList;
	int                         m_SmartMaxArtistsList;
	bool                        m_SmartSearchEnabled;
    int                         m_SmartPlayAddTracks;
    int                         m_SmartPlayMinTracksToPlay;

    bool                        m_DelTracksPlayed;

    unsigned int                m_TrackStartPos;

    bool                        m_SilenceDetector;
    int                         m_SilenceDetectorLevel;
    int                         m_SilenceDetectorTime;

    bool                        m_ForceGapless;
    int                         m_FadeOutTime;

    bool                        m_PendingScrob;

    // AudioScrobble
    guAudioScrobble *           m_AudioScrobble;
    bool                        m_AudioScrobbleEnabled;
    //guSmartAddTracksThread *    m_SmartAddTracksThread;
    guSmartModeThread *         m_SmartAddTracksThread;

    int                         m_BufferGaugeId;

    long                        m_NextTrackId;
    long                        m_CurTrackId;
    bool                        m_TrackChanged;

    bool                        m_ShowRevTime;

    //bool                        m_PendingNewRecordName;

    bool                        m_ErrorFound;

    int                         m_SavedPlayedTrack;

    wxString                    m_LastTmpCoverFile;
//    guUpdatePlayerCoverThread * m_UpdateCoverThread;

	void                        OnVolumenButtonClick( wxCommandEvent &event );
	void                        OnVolumenMouseWheel( wxMouseEvent &event );
    //void                        OnPlayerCoverBitmapMouseOver( wxCommandEvent &event );
    void                        OnLeftClickPlayerCoverBitmap( wxMouseEvent &event );
    void                        OnPlayerPositionSliderBeginSeek( wxScrollEvent &event );
    void                        OnPlayerPositionSliderEndSeek( wxScrollEvent &event );
    void                        OnPlayerPositionSliderChanged( wxScrollEvent &event );
    void                        OnPlayerPositionSliderMouseWheel( wxMouseEvent &event );
    //
    void                        OnPlayListUpdated( wxCommandEvent &event );
    void                        OnPlayListDClick( wxCommandEvent &event );

    void                        LoadMedia( guFADERPLAYBIN_PLAYTYPE playtype, const bool forceskip = false );
    void                        OnMediaLoaded( guMediaEvent &event );
    void                        OnMediaPlayStarted( void );
    void                        SavePlayedTrack( void );
    void                        OnMediaFinished( guMediaEvent &event );
    void                        OnMediaFadeOutFinished( guMediaEvent &event );
    void                        OnMediaFadeInStarted( guMediaEvent &event );
    void                        OnMediaTags( guMediaEvent &event );
    void                        OnMediaBitrate( guMediaEvent &event );
    void                        OnMediaBuffering( guMediaEvent &event );
    void                        OnMediaLevel( guMediaEvent &event );
    void                        OnMediaError( guMediaEvent &event );
    void                        OnMediaState( guMediaEvent &event );
    void                        OnMediaPosition( guMediaEvent &event );
    void                        OnMediaLength( guMediaEvent &event );

    void                        SetNextTrack( const guTrack * Song );

    // SmartPlay Events
    void                        SmartAddTracks( const guTrack &CurSong );
    void                        OnSmartEndThread( wxCommandEvent &event );
    void                        OnSmartAddTracks( wxCommandEvent &event );
    void                        OnUpdatedRadioTrack( wxCommandEvent &event );

    void                        OnTitleNameDClicked( wxMouseEvent &event );
    void                        OnAlbumNameDClicked( wxMouseEvent &event );
    void                        OnArtistNameDClicked( wxMouseEvent &event );
    void                        OnYearDClicked( wxMouseEvent &event );
    void                        OnTimeDClicked( wxMouseEvent &event ) { m_ShowRevTime = !m_ShowRevTime;
                                                                UpdatePositionLabel( GetPosition() ); }
    void                        OnRatingChanged( guRatingEvent &event );
    void                        CheckFiltersEnable( void );

    void                        OnConfigUpdated( wxCommandEvent &event );

    void                        SendRecordSplitEvent( void );

    void                        OnCoverUpdated( wxCommandEvent &event ); // Once the cover have been found shows it

    void                        OnAddTracks( wxCommandEvent &event );
    void                        OnRemoveTrack( wxCommandEvent &event );
    void                        OnRepeat( wxCommandEvent &event );
    void                        OnLoop( wxCommandEvent &event );
    void                        OnRandom( wxCommandEvent &event );
    void                        OnSetVolume( wxCommandEvent &event );

  public:
    guPlayerPanel( wxWindow * parent, guDbLibrary * db,
                   guPlayList * playlist, guPlayerFilters * filters );
    ~guPlayerPanel();

    guMainFrame *               MainFrame( void ) { return m_MainFrame; }

    void                        SetPlayList( const guTrackArray &SongList );
    void                        AddToPlayList( const guTrackArray &SongList, const bool allowplay = true, const int aftercurrent = guINSERT_AFTER_CURRENT_NONE );
    void                        AddToPlayList( const wxString &FileName, const int aftercurrent = guINSERT_AFTER_CURRENT_NONE );
    void                        AddToPlayList( const wxArrayString &files, const int aftercurrent = guINSERT_AFTER_CURRENT_NONE );
    void                        AddToPlayList( const wxArrayString &files, const bool play, const int aftercurrent );
    void                        ClearPlayList( void ) { m_PlayListCtrl->ClearItems(); }
    void                        SetPlayList( const wxArrayString &files );
    guPlayList *                PlayListCtrl( void ) { return m_PlayListCtrl; }

    double                      GetVolume() { return m_CurVolume; }
    void                        SetVolume( double volume );
    bool                        SetPosition( int pos );
    int                         GetPosition();
    void                        TrackListChanged( void );
    const guCurrentTrack *      GetCurrentTrack() { return &m_MediaSong; }
    int                         GetCurrentItem();
    int                         GetItemCount();
    const guTrack *             GetTrack( int index );
    void                        RemoveItem( int itemnum );

    int                         GetPlayLoop();
    void                        SetPlayLoop( int playloop );
    bool                        GetPlaySmart();
    void                        SetPlaySmart( bool playsmart );
    void                        UpdatePlayListFilters( void );

    void                        SetCurrentCoverImage( wxImage * coverimage, const guSongCoverType CoverType, const wxString &CoverPath = wxEmptyString );
    void                        UpdateCoverImage( const bool shownotify = true );

    int                         GetCaps();

    const guMediaState          GetState( void );

    void                        OnPrevTrackButtonClick( wxCommandEvent &event );
    void                        OnNextTrackButtonClick( wxCommandEvent &event );
    void                        OnPrevAlbumButtonClick( wxCommandEvent &event );
    void                        OnNextAlbumButtonClick( wxCommandEvent &event );
    void                        OnPlayButtonClick( wxCommandEvent &event );
    void                        OnStopButtonClick( wxCommandEvent &event );
    void                        OnStopAtEnd( wxCommandEvent &event );
    void                        OnRecordButtonClick( wxCommandEvent &event );
	void                        OnSmartPlayButtonClick( wxCommandEvent &event );
	void                        OnRandomPlayButtonClick( wxCommandEvent &event );
	void                        OnRepeatPlayButtonClick( wxCommandEvent &event );
	void                        OnLoveBanButtonClick( wxCommandEvent &event );
	void                        OnEqualizerButtonClicked( wxCommandEvent &event );

    void                        SetArtistLabel( const wxString &artistname );
    void                        SetAlbumLabel( const wxString &albumname );
    void                        SetTitleLabel( const wxString &trackname );
    void                        SetRatingLabel( const int Rating );
    int                         GetRating( void ) { return m_MediaSong.m_Rating; }
    void                        SetRating( const int rating );
    void                        UpdatePositionLabel( const unsigned int curpos );
    void                        SetBitRateLabel( const int bitrate );
    void                        SetBitRate( int bitrate );

    void                        UpdatedTracks( const guTrackArray * tracks );
    void                        UpdatedTrack( const guTrack * track );

    void                        UpdateLabels( void );

    void                        SetPlayerVumeters( guPlayerVumeters * vumeters ) { m_PlayerVumeters = vumeters; }
    void                        ResetVumeterLevel( void );

    void                        SetNotifySrv( guDBusNotify * notify ) { m_NotifySrv = notify; }
    void                        SendNotifyInfo( wxImage * image );

    void                        SetForceGapless( const bool forcegapless ) { m_ForceGapless = forcegapless; m_MediaCtrl->ForceGapless( forcegapless ); }
    bool                        GetForceGapless( void ) { return m_ForceGapless; }

    bool                        GetAudioScrobbleEnabled( void ) { return m_AudioScrobbleEnabled; }

    void                        UpdateCover( const bool shownotify = true, const bool deleted = false );    // Start the thread that search for the cover

    wxString                    LastTmpCoverFile( void ) { return m_LastTmpCoverFile; }
    void                        SetLastTmpCoverFile( const wxString &lastcoverfile ) { m_LastTmpCoverFile = lastcoverfile; }

    void                        StopAtEnd( void ) { m_MediaSong.m_Type = guTrackType( ( int ) m_MediaSong.m_Type ^ guTRACK_TYPE_STOP_HERE ); }

    void                        MediaViewerClosed( guMediaViewer * mediaviewer );

    void                        CheckStartPlaying( void );

    //friend class guSmartAddTracksThread;
};

// -------------------------------------------------------------------------------- //
class guUpdatePlayerCoverThread : public wxThread
{
  protected :
    guDbLibrary *       m_Db;
    guPlayerPanel *     m_PlayerPanel;
    guCurrentTrack *    m_CurrentTrack;
    guMainFrame *       m_MainFrame;
    bool                m_ShowNotify;
    bool                m_Deleted;

  public:
    guUpdatePlayerCoverThread( guDbLibrary * db, guMainFrame * mainframe, guPlayerPanel * playerpanel,
                              guCurrentTrack * currenttrack, const bool shownotify, const bool deleted = false );
    ~guUpdatePlayerCoverThread();

    virtual ExitCode Entry();
};

}

#endif
// -------------------------------------------------------------------------------- //
