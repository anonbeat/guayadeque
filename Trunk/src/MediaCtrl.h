// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
//	anonbeat@gmail.com
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
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef MEDIACTRL_H
#define MEDIACTRL_H

#include "DbLibrary.h"
#include "TimeLine.h"

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/uri.h>
//#include <wx/mediactrl.h>
#include <wx/dynarray.h>

#undef ATTRIBUTE_PRINTF // there are warnings about redefined ATTRIBUTE_PRINTF in Fedora
#include <gst/gst.h>


#define guEQUALIZER_BAND_COUNT  10

//#define guFADERPLAYBIN_MESSAGE_PLAYING          "guayadeque-playing"
#define guFADERPLAYBIN_MESSAGE_FADEOUT_DONE     "guayadeque-fade-out-done"
#define guFADERPLAYBIN_MESSAGE_FADEIN_START     "guayadeque-fade-in-start"
//#define guFADERPLAYBIN_MESSAGE_FADEIN_DONE      "guayadeque-fade-in-done"
//#define guFADERPLAYBIN_MESSAGE_EOS              "guayadeque-eos"


enum guRecordFormat {
    guRECORD_FORMAT_MP3,
    guRECORD_FORMAT_OGG,
    guRECORD_FORMAT_FLAC
};

enum guRecordQuality {
    guRECORD_QUALITY_VERY_HIGH,
    guRECORD_QUALITY_HIGH,
    guRECORD_QUALITY_NORMAL,
    guRECORD_QUALITY_LOW,
    guRECORD_QUALITY_VERY_LOW
};

enum guOutputDeviceSink {
    guOUTPUT_DEVICE_AUTOMATIC,
    guOUTPUT_DEVICE_GCONF,
    guOUTPUT_DEVICE_ALSA,
    guOUTPUT_DEVICE_PULSEAUDIO,
    guOUTPUT_DEVICE_OSS,
    guOUTPUT_DEVICE_OTHER
};

// -------------------------------------------------------------------------------- //
class guLevelInfo
{
  public :
    GstClockTime    m_EndTime;
    wxFileOffset    m_OutTime;
    gint            m_Channels;
    double          m_RMS_L;
    double          m_RMS_R;
    double          m_Peak_L;
    double          m_Peak_R;
    double          m_Decay_L;
    double          m_Decay_R;
};
WX_DEFINE_ARRAY_PTR( guLevelInfo *, guLevelInfoArray );

// -------------------------------------------------------------------------------- //
class guRadioTagInfo
{
  public :
    gchar * m_Organization;
    gchar * m_Location;
    gchar * m_Title;
    gchar * m_Genre;

    guRadioTagInfo() { m_Organization = NULL; m_Location = NULL; m_Title = NULL; m_Genre = NULL; }
    ~guRadioTagInfo()
    {
        if( m_Organization )
            g_free( m_Organization );
        if( m_Location )
            g_free( m_Location );
        if( m_Title )
            g_free( m_Title );
        if( m_Genre )
            g_free( m_Genre );
    }
};

// -------------------------------------------------------------------------------- //
enum guMediaState
{
    guMEDIASTATE_STOPPED,
    guMEDIASTATE_PAUSED,
    guMEDIASTATE_PLAYING,
    guMEDIASTATE_ERROR
};

// -------------------------------------------------------------------------------- //
//
// guMediaEvent
//
// -------------------------------------------------------------------------------- //
class guMediaEvent : public wxNotifyEvent
{
  public:
    guMediaEvent( wxEventType commandType = wxEVT_NULL, int winid = 0 ) : wxNotifyEvent( commandType, winid ) { }
    guMediaEvent( const guMediaEvent &clone ) : wxNotifyEvent( clone ) { }

    virtual wxEvent * Clone() const
    {
        return new guMediaEvent( * this );
    }
};


//Function type(s) our events need
typedef void (wxEvtHandler::*wxMediaEventFunction)(guMediaEvent&);

#define guMediaEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxMediaEventFunction, &func)

DECLARE_EVENT_TYPE( guEVT_MEDIA_LOADED,             wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_FINISHED,           wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_CHANGED_STATE,      wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_BUFFERING,          wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_LEVELINFO,          wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_TAGINFO,            wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_CHANGED_BITRATE,    wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_CHANGED_POSITION,   wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_CHANGED_LENGTH,     wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_FADEOUT_FINISHED,   wxID_ANY )
DECLARE_EVENT_TYPE( guEVT_MEDIA_FADEIN_STARTED,     wxID_ANY )

DECLARE_EVENT_TYPE( guEVT_MEDIA_ERROR,              wxID_ANY )

class guPlayerPanel;
class guMediaCtrl;

enum guFADERPLAYBIN_PLAYTYPE {
    guFADERPLAYBIN_PLAYTYPE_CROSSFADE,
    guFADERPLAYBIN_PLAYTYPE_AFTER_EOS,
    guFADERPLAYBIN_PLAYTYPE_REPLACE
};

enum guFADERPLAYBIN_STATE {
    guFADERPLAYBIN_STATE_WAITING,
    guFADERPLAYBIN_STATE_WAITING_EOS,
    guFADERPLAYBIN_STATE_PLAYING,
    guFADERPLAYBIN_STATE_PAUSED,
    guFADERPLAYBIN_STATE_STOPPED,
    guFADERPLAYBIN_STATE_FADEIN,
    guFADERPLAYBIN_STATE_FADEOUT,
    guFADERPLAYBIN_STATE_FADEOUT_STOP,
    guFADERPLAYBIN_STATE_FADEOUT_PAUSE,
    guFADERPLAYBIN_STATE_PENDING_REMOVE,
    guFADERPLAYBIN_STATE_ERROR
};


class guMediaCtrl;
class guFaderPlayBin;

// -------------------------------------------------------------------------------- //
class guFaderTimeLine : public guTimeLine
{
  protected :
    guFaderPlayBin * m_FaderPlayBin;
    double           m_VolStep;
    double           m_VolStart;
    double           m_VolEnd;

  public :
    guFaderTimeLine( const int timeout = 3000, wxEvtHandler * parent = NULL, guFaderPlayBin * playbin = NULL,
        double volstart = 0.0, double volend = 1.0 );
    ~guFaderTimeLine();

    virtual void    ValueChanged( float value );
    virtual void    Finished( void );
    virtual int     TimerCreate( void );
};

// -------------------------------------------------------------------------------- //
class guFaderPlayBin
{
  protected :
    guMediaCtrl *       m_Player;
    wxMutex             m_Lock;
    guTimeLine *        m_FaderTimeLine;
    wxString            m_Uri;
    wxString            m_NextUri;
    int                 m_PlayType;
    bool                m_IsFading;
    bool                m_IsBuffering;
    bool                m_EmittedStartFadeIn;
    bool                m_AboutToFinishPending;
    int                 m_AboutToFinishPendingId;
    long                m_Id;
    long                m_NextId;
    double              m_LastFadeVolume;


    int                 m_ErrorCode;
    int                 m_State;
    gint64              m_PausePosition;

    //
    GstElement *        m_OutputSink;
    GstElement *        m_Playbin;
    GstElement *        m_Playbackbin;
    GstElement *        m_FaderVolume;
    GstElement *        m_ReplayGain;
    GstElement *        m_Volume;
    GstElement *        m_Equalizer;
    GstElement *        m_Tee;

    GstElement *        m_RecordBin;
    GstElement *        m_FileSink;
    GstPad *            m_RecordGhostPad;
    GstPad *            m_RecordPad;

    int                 m_StartOffset;
    int                 m_SeekTimerId;

    bool                BuildPlaybackBin( void );
    bool                BuildOutputBin( void );
    bool                BuildRecordBin( const wxString &path, GstElement * encoder, GstElement * muxer );

  public :
    guFaderPlayBin( guMediaCtrl * mediactrl, const wxString &uri, const int playtype, const int startpos = 0 );
    ~guFaderPlayBin();

    void                SendEvent( guMediaEvent &event );

    GstElement *        OutputSink( void ) { return m_OutputSink; }
    GstElement *        Playbin( void ) { return m_Playbin; }
    GstElement *        Volume( void ) { return m_Volume; }
    guMediaCtrl *       GetPlayer( void ) { return m_Player; }

    GstElement *        RecordBin( void ) { return m_RecordBin; }
    void                SetRecordBin( GstElement * recordbin ) { m_RecordBin = recordbin; }

    wxString            Uri( void ) { return m_Uri; }
    void                Lock( void ) { m_Lock.Lock(); }
    void                Unlock( void ) { m_Lock.Unlock(); };
    long                GetId( void ) { return m_Id; }
    void                SetId( const long id ) { m_Id = id; }

    int                 GetState( void ) { return m_State; }
    void                SetState( int state ) { m_State = state; }

    bool                IsBuffering( void ) { return m_IsBuffering; }
    void                SetBuffering( const bool isbuffering ) { m_IsBuffering = isbuffering; }

    bool                SetVolume( double volume );
    double              GetFaderVolume( void );
    bool                SetFaderVolume( double volume );

    bool                SetEqualizer( const wxArrayInt &eqset );
    void                SetEqualizerBand( const int band, const int value );

    bool                Load( const wxString &uri, const bool restart = true, const int startpos = 0 );
    bool                Play( void );
    bool                Pause( void );
    bool                Stop( void );

    bool                StartPlay( void );
    bool                StartFade( double volstart, double volend, int timeout );
    void                EndFade( void ) { delete m_FaderTimeLine; m_FaderTimeLine = NULL; }

    bool                Seek( wxFileOffset where );
    wxFileOffset        Position( void );
    wxFileOffset        Length( void );

    bool                IsOk( void ) { return !m_ErrorCode; }
    int                 ErrorCode( void ) { return m_ErrorCode; }
    void                SetErrorCode( const int error ) { m_ErrorCode = error; }

    void                SetNextUri( const wxString &uri ) { m_NextUri = uri; }
    wxString            NextUri( void ) { return m_NextUri; }

    void                SetNextId( const long id ) { m_NextId = id; }
    long                NextId( void ) { return m_NextId; }


    void                AboutToFinish( void );
    void                AudioChanged( void );
    bool                AboutToFinishPending( void ) { return m_AboutToFinishPending; }
    void                ResetAboutToFinishPending( void ) { m_AboutToFinishPendingId = 0; m_AboutToFinishPending = false; }

    void                FadeInStart( void );
    void                FadeOutDone( void );
    bool                EmittedStartFadeIn( void ) { return m_EmittedStartFadeIn; }

    bool                EnableRecord( const wxString &path, const int format, const int quality );
    void                DisableRecord( void );
    bool                SetRecordFileName( const wxString &filename );

    void                AddRecordElement( GstPad * pad, bool isblocked );
    void                RemoveRecordElement( GstPad * pad, bool isblocked );

    bool                DoStartSeek( void );

    friend class guMediaCtrl;
    friend class guFaderTimeLine;
};
WX_DEFINE_ARRAY_PTR( guFaderPlayBin *, guFaderPlayBinArray );

// -------------------------------------------------------------------------------- //
class guMediaCtrl : public wxEvtHandler
{
  protected :
    guPlayerPanel *         m_PlayerPanel;

    wxMutex                 m_FaderPlayBinsMutex;
    guFaderPlayBinArray     m_FaderPlayBins;
    guFaderPlayBin *        m_CurrentPlayBin;

    int                     m_OutputDevice;
    wxString                m_OutputDeviceName;
    bool                    m_ForceGapless;
    int                     m_FadeOutTime;
    int                     m_FadeInTime;
    double                  m_FadeInVolStart;
    double                  m_FadeInVolTriger;
    int                     m_CleanUpId;

    bool                    m_IsRecording;

    int                     m_TickTimeoutId;
    gint64                  m_LastPosition;

    double                  m_Volume;
    wxArrayInt              m_EqBands;

    int                     m_LastError;

    int                     m_BufferSize;

    bool                    m_ReplayGainMode;
    double                  m_ReplayGainPreAmp;
    //double                  m_ReplayGainFallback;

    bool                    RemovePlayBin( guFaderPlayBin * playbin );

    void                    FadeOutDone( guFaderPlayBin * faderplaybin );
    void                    FadeInStart( void );

  public :

    guMediaCtrl( guPlayerPanel * playerpanel );
    ~guMediaCtrl();

    static bool     Init();

    guFaderPlayBin * CurrentPlayBin( void ) { return m_CurrentPlayBin; }

    void            UpdatePosition( void );

    long            Load( const wxString &uri, guFADERPLAYBIN_PLAYTYPE playtype, const int startpos = 0 );
    bool            Stop( void );
    bool            Play( void );
    bool            Pause( void );

    bool            Seek( wxFileOffset where );
    wxFileOffset    Position( void );
    wxFileOffset    Length( void );

    bool            IsBuffering( void ) { return ( m_CurrentPlayBin && m_CurrentPlayBin->IsBuffering() ); }
    int             BufferSize( void ) { return m_BufferSize; }
    bool            IsRecording( void ) { return m_IsRecording; }

    bool            ForceGapless( void ) { return m_ForceGapless; }
    void            ForceGapless( const bool forcegapless ) { m_ForceGapless = forcegapless; }

    double          GetVolume( void ) { return m_Volume; }
    bool            SetVolume( double volume );

    void            DoCleanUp( void );

    int             GetEqualizerBand( const int band ) { return m_EqBands[ band ]; }
    bool            SetEqualizer( const wxArrayInt &eqset );
    wxArrayInt      GetEqualizer( void ) { return m_EqBands; }
    void            ResetEqualizer( void );
    void            SetEqualizerBand( const int band, const int value );

    void            SendEvent( guMediaEvent &event );

    int             GetLastError( void ) { return m_LastError; };
    void            SetLastError( const int error ) { m_LastError = error; };
    void            ClearError( void ) { m_LastError = 0; }

    guMediaState    GetState( void );

    void            Lock( void ) { m_FaderPlayBinsMutex.Lock(); }
    void            Unlock( void ) { m_FaderPlayBinsMutex.Unlock(); }

    void            UpdatedConfig( void );

    void            ScheduleCleanUp( void );

    bool            EnableRecord( const wxString &path, const int format, const int quality );
    void            DisableRecord( void );
    bool            SetRecordFileName( const wxString &filename );

    int             OutputDevice( void ) { return m_OutputDevice; }
    wxString        OutputDeviceName( void ) { return m_OutputDeviceName; }

    friend class guFaderPlayBin;
};

// -------------------------------------------------------------------------------- //
class guMediaRecordCtrl
{
  protected:
    guPlayerPanel * m_PlayerPanel;
    guMediaCtrl *   m_MediaCtrl;
    guTrack         m_TrackInfo;
    guTrack         m_PrevTrack;
    wxString        m_PrevFileName;

    wxString        m_MainPath;
    int             m_Format;
    int             m_Quality;
    bool            m_DeleteTracks;
    int             m_DeleteTime;
    wxString        m_Ext;
    wxString        m_FileName;


    bool            m_Recording;
    bool            m_SplitTracks;
    bool            m_FirstChange;

    wxString        GetRecordFileName( void );

  public :
    guMediaRecordCtrl( guPlayerPanel * playerpanel, guMediaCtrl * mediactrl );
    ~guMediaRecordCtrl();

    void            SetTrack( const guTrack &track );
    void            SetTrackName( const wxString &artistname, const wxString &trackname );

    void            SetStation( const wxString &station );

    void            SetGenre( const wxString &genre ) { m_TrackInfo.m_GenreName = genre; }

    bool            SaveTagInfo( const wxString &filename, const guTrack * track );

    bool            IsRecording( void ) { return m_Recording; }

    bool            Start( const guTrack * track );
    bool            Stop( void );

    void            SplitTrack( void );

    void            UpdatedConfig( void );

};

#endif
// -------------------------------------------------------------------------------- //
