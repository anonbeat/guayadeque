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
#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include "AudioScrobble.h"
#include "AutoScrollText.h"
#include "MediaCtrl.h"
#include "PlayList.h"
#include "RatingCtrl.h"


#include <wx/wx.h>
#include <wx/dnd.h>
//#include <wx/mediactrl.h>
#include <wx/tglbtn.h>

// -------------------------------------------------------------------------------- //
enum guSongCoverType {
    GU_SONGCOVER_NONE,
    GU_SONGCOVER_FILE,
    GU_SONGCOVER_ID3TAG,
    GU_SONGCOVER_RADIO,
    GU_SONGCOVER_PODCAST
};

// -------------------------------------------------------------------------------- //
class guCurrentTrack : public guTrack
{
  public:
    // Only for the Current Played song
    bool            m_Loaded;
    int             m_PlayTime;           // how many secs the song have been played
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
};

class guPlayerPanelTimer;
class guSmartAddTracksThread;

// -------------------------------------------------------------------------------- //
class guPlayerPanel : public wxPanel
{
  private:
	wxBitmapButton *        m_PrevTrackButton;
	wxBitmapButton *        m_NextTrackButton;
	wxBitmapButton *        m_PlayButton;
	wxBitmapButton *        m_StopButton;
	wxBitmapButton *        m_VolumeButton;
	//
	wxToggleBitmapButton *  m_SmartPlayButton;
	wxBitmapButton *        m_RandomPlayButton;
	wxToggleBitmapButton *  m_RepeatPlayButton;
	//
	wxStaticBitmap *        m_PlayerCoverBitmap;
//	guAutoScrollText *      m_TitleLabel;
//	guAutoScrollText *      m_AlbumLabel;
//	guAutoScrollText *      m_ArtistLabel;
	wxStaticText *          m_TitleLabel;
	wxStaticText *          m_AlbumLabel;
	wxStaticText *          m_ArtistLabel;
	wxStaticText *          m_YearLabel;
    guRating *              m_Rating;
	wxStaticText *          m_BitRateLabel;
	wxBoxSizer *            m_BitRateSizer;
	wxStaticText *          m_PositionLabel;
	wxBoxSizer *            m_PosLabelSizer;
	wxSlider *              m_PlayerPositionSlider;
//	wxStaticText *          m_PlayListStaticText;
//	wxStaticText *          m_PlayListLenStaticText;
//    wxBoxSizer *            m_PlayListLabelsSizer;

    DbLibrary *             m_Db;
	guPlayList *            m_PlayListCtrl;
	guMediaCtrl *           m_MediaCtrl;
	guPlayerPanelTimer *    m_PlayerTimer;
    guCurrentTrack          m_MediaSong;
	wxMediaState            m_LastPlayState;
	float                   m_LastVolume;
	wxFileOffset            m_LastCurPos;

	float                   m_CurVolume;
	bool                    m_PlayLoop;
	bool                    m_PlaySmart;
	bool                    m_SliderIsDragged;
	long                    m_LastTotalLen;
	wxArrayString           m_SmartAddedSongs;
	bool                    m_SmartSearchEnabled;
    int                     m_SmartPlayAddTracks;
    int                     m_SmartPlayMinTracksToPlay;
    unsigned int            m_TrackStartPos;

    // AudioScrobble
    guAudioScrobble *       m_AudioScrobble;
    bool                    m_AudioScrobbleEnabled;
    guSmartAddTracksThread * m_SmartAddTracksThread;

    int                     m_BufferGaugeId;

    bool                    m_AboutToFinishPending;

	void                OnVolumenButtonClick( wxCommandEvent &event );
	void                OnVolumenMouseWheel( wxMouseEvent &event );
    void                OnLeftDClickPlayerCoverBitmap( wxMouseEvent &event );
    void                OnPlayerPositionSliderBeginSeek( wxScrollEvent &event );
    void                OnPlayerPositionSliderEndSeek( wxScrollEvent &event );

    //
    void                OnPlayListUpdated( wxCommandEvent &event );
    void                OnPlayListDClick( wxCommandEvent &event );
    void                LoadMedia( const wxString &FileName, bool restart = true );
    void                OnMediaLoaded( wxMediaEvent &event );
    void                OnMediaAboutToFinish( wxMediaEvent &event );
    void                OnMediaFinished( wxMediaEvent &event );
    void                OnMediaTag( wxMediaEvent &event );
    void                OnMediaBitrate( wxMediaEvent &event );
    void                OnMediaBuffering( wxMediaEvent &event );
    void                SetCurrentTrack( const guTrack * Song );

    // SmartPlay Events
    void                SmartAddTracks( const guTrack &CurSong );
    void                OnSmartAddTracksClicked( wxCommandEvent &event );
    void                OnUpdatedRadioTrack( wxCommandEvent &event );

    void                OnAlbumNameDClicked( wxMouseEvent &event );
    void                OnArtistNameDClicked( wxMouseEvent &event );
    void                OnRatingChanged( guRatingEvent &event );

  public:
                        guPlayerPanel( wxWindow* parent, DbLibrary * NewDbLibrary ); //wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 368,191 ), long style = wxTAB_TRAVERSAL );
                        ~guPlayerPanel();
    void                UpdateStatus();
    void                SetPlayList( const guTrackArray &SongList );
    void                AddToPlayList( const guTrackArray &SongList );
    void                AddToPlayList( const wxString &FileName );
    float               GetVolume();
    void                SetVolume( float volume );
    bool                SetPosition( int pos );
    int                 GetPosition();
    void                TrackListChanged( void );
    const guCurrentTrack * GetCurrentTrack();
    int                 GetCurrentItem();
    int                 GetItemCount();
    const guTrack *     GetTrack( int index );
    void                RemoveItem( int itemnum );

    bool                GetPlayLoop();
    void                SetPlayLoop( bool playloop );
    bool                GetPlaySmart();
    void                SetPlaySmart( bool playsmart );

    int                 GetCaps();

    const wxMediaState  GetState( void );
    void                OnPrevTrackButtonClick( wxCommandEvent &event );
    void                OnNextTrackButtonClick( wxCommandEvent &event );
    void                OnPlayButtonClick( wxCommandEvent &event );
    void                OnStopButtonClick( wxCommandEvent &event );
	void                OnSmartPlayButtonClick( wxCommandEvent &event );
	void                OnRandomPlayButtonClick( wxCommandEvent &event );
	void                OnRepeatPlayButtonClick( wxCommandEvent &event );

    void                OnAboutToFinish( void );


    void                SetArtistLabel( const wxString &artistname );
    void                SetAlbumLabel( const wxString &albumname );
    void                SetTitleLabel( const wxString &trackname );
    void                SetRatingLabel( const int Rating );
    void                SetLengthLabel( const int length );
    void                SetBitRate( int bitrate );

    void                UpdatedTracks( const guTrackArray * tracks );


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
  private:
    DbLibrary *     m_Db;
    guPlayerPanel * m_PlayerPanel;
    const guTrack * m_CurSong;

  public:
    guSmartAddTracksThread( DbLibrary * NewDb, guPlayerPanel * NewPlayer, const guTrack * NewSong );
    ~guSmartAddTracksThread();

    virtual ExitCode Entry();
};

#endif
// -------------------------------------------------------------------------------- //
