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
#ifndef MEDIACTRL_H
#define MEDIACTRL_H

#include "DbLibrary.h"

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/uri.h>
//#include <wx/mediactrl.h>
#include <wx/dynarray.h>

#undef ATTRIBUTE_PRINTF // there are warnings about redefined ATTRIBUTE_PRINTF in Fedora
#include <gst/gst.h>
#include <gst/controller/gstcontroller.h>
#include <gst/base/gstbasetransform.h>


#define guEQUALIZER_BAND_COUNT  10

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

// -------------------------------------------------------------------------------- //
class guLevelInfo
{
  public :
    GstClockTime    m_EndTime;
    gint            m_Channels;
    double          m_RMS_L;
    double          m_RMS_R;
    double          m_Peak_L;
    double          m_Peak_R;
    double          m_Decay_L;
    double          m_Decay_R;
};

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
    guMEDIASTATE_PLAYING
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
//DECLARE_EVENT_TYPE( guEVT_MEDIA_SET_NEXT_MEDIA,     wxID_ANY )
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

enum guFaderPlayBinState {
    // Stable
    guFADERPLAYBIN_STATE_WAITING =              ( 1 << 0 ),
    guFADERPLAYBIN_STATE_PLAYING =              ( 1 << 1 ),
    guFADERPLAYBIN_STATE_PAUSED  =              ( 1 << 2 ),
    // Transitions
    guFADERPLAYBIN_STATE_REUSING =              ( 1 << 3 ),
    guFADERPLAYBIN_STATE_PREROLLING =           ( 1 << 4 ),
    guFADERPLAYBIN_STATE_PREROLL_PLAY =         ( 1 << 5 ),
    guFADERPLAYBIN_STATE_FADING_IN =            ( 1 << 6 ),
    guFADERPLAYBIN_STATE_SEEKING =              ( 1 << 7 ),
    guFADERPLAYBIN_STATE_SEEKING_PAUSED =       ( 1 << 8 ),
    guFADERPLAYBIN_STATE_SEEKING_STOPPED =      ( 1 << 9 ),
    guFADERPLAYBIN_STATE_SEEKING_EOS =          ( 1 << 10 ),
    guFADERPLAYBIN_STATE_WAITING_EOS =          ( 1 << 11 ),
    guFADERPLAYBIN_STATE_FADING_OUT =           ( 1 << 12 ),
    guFADERPLAYBIN_STATE_FADING_OUT_PAUSED =    ( 1 << 13 ),
    guFADERPLAYBIN_STATE_FADING_OUT_STOPPED =   ( 1 << 14 ),
    guFADERPLAYBIN_STATE_PENDING_REMOVE =       ( 1 << 15 )
};

enum guPlayerPlayType {
    guFADERPLAYBIN_PLAYTYPE_CROSSFADE,
    guFADERPLAYBIN_PLAYTYPE_AFTER_EOS,
    guFADERPLAYBIN_PLAYTYPE_REPLACE
};

#define guFADERPLAYBIN_MESSAGE_PLAYING          "guayadeque-playing"
#define guFADERPLAYBIN_MESSAGE_FADEOUT_DONE     "guayadeque-fade-out-done"
#define guFADERPLAYBIN_MESSAGE_FADEIN_START     "guayadeque-fade-in-start"
#define guFADERPLAYBIN_MESSAGE_FADEIN_DONE      "guayadeque-fade-in-done"
#define guFADERPLAYBIN_MESSAGE_EOS              "guayadeque-eos"

class guMediaCtrl;

// -------------------------------------------------------------------------------- //
class guFaderPlayBin
{
  protected :

  public :
    guMediaCtrl *       m_Player;
    GstElement *        m_Parent;

    wxMutex             m_Lock;

    wxString            m_Uri;
    wxString            m_NewUri;


    GstElement *        m_PlayBin;
    GstElement *        m_Decoder;
    GstElement *        m_Volume;
    GstElement *        m_AudioConvert;
    GstElement *        m_AudioResample;
    GstElement *        m_CapsFilter;
    GstElement *        m_PreRoll;
    GstElement *        m_Identity;

    bool                m_DecoderLinked;
    bool                m_EmittedPlaying;
    bool                m_EmittedFakePlaying;
    bool                m_EmittedStartFadeIn;

    GstPad *            m_DecoderPad;
    GstPad *            m_SourcePad;
    GstPad *            m_GhostPad;
    GstPad *            m_AdderPad;

    bool                m_SoureBlocked;
    bool                m_NeedsUnlink;

    GstClockTime        m_BaseTime;

    wxFileOffset        m_SeekTarget;

	GstController *     m_Fader;
    guFaderPlayBinState m_State;
    guPlayerPlayType    m_PlayType;
    gint64              m_FadeOutTime;
    bool                m_Fading;

    gulong              m_AdjustProbeId;

    double              m_FadeEnd;

    bool                m_EmittedError;
    bool                m_ErrorIdleId;
    GError *            m_Error;

    wxArrayPtrVoid      m_Tags;

    guFaderPlayBin( guMediaCtrl * mediactrl, const wxString &uri );
    ~guFaderPlayBin();

    void                Lock( void ) { m_Lock.Lock(); }
    void                Unlock( void ) { m_Lock.Unlock(); };

    //void                PrepareSource( GstElement * source );
    void                UnlinkAndBlock( void );
    bool                LinkAndUnblock( GError ** error );
    bool                ActuallyStart( GError ** error );
    void                StartFade( double start, double end, gint64 time );
    void                AdjustBaseTime( void );
    void                PostPlayMessage( bool fake );
    void                EmitError( GError * error );
    bool                Preroll( void );
    void                Reuse( void );

    friend class guMediaCtrl;
};
WX_DEFINE_ARRAY_PTR( guFaderPlayBin *, guFaderPlayBinArray );

// -------------------------------------------------------------------------------- //
class guMediaCtrl : public wxEvtHandler
{
  protected :
    guPlayerPanel *         m_PlayerPanel;

    int                     m_LastError;
    GstState                m_CurrentState;

    GstElement *            BuildOutputBin( void );
    GstElement *            BuildPlaybackBin( GstElement * outputsink );
    GstElement *            BuildRecordBin( const wxString &path, GstElement * encoder, GstElement * muxer );
    void                    AddBusWatch( void );

  public :
    guFaderPlayBinArray     m_FaderPlayBins;
    wxMutex                 m_FaderPlayBinsMutex;
    int                     m_LinkedStreams;

    wxMutex                 m_SinkLock;
	enum {
		SINK_NULL,
		SINK_STOPPED,
		SINK_PLAYING
	}                       m_SinkState;

    GstElement *            m_Pipeline;
    GstElement *            m_Adder;
    GstElement *            m_RecordBin;
    GstElement *            m_OutputBin;
    GstElement *            m_PlaybackBin;
    GstElement *            m_SilenceBin;
    GstElement *            m_CapsFilter;

    GstElement *            m_Tee;
    GstPad *                m_RecordPad;
    GstElement *            m_Volume;
    GstElement *            m_Equalizer;
    GstElement *            m_FileSink;

    bool                    m_Buffering;
    bool                    m_WasPlaying;

	int                     m_TickTimeoutId;

	int                     m_PlayBinReapId;
	int                     m_StopSinkId;
	int                     m_BusWatchId;

    gint64                  m_FadeOutTime;
    gint64                  m_FadeInTime;
    double                  m_FadeInVolStart;
    double                  m_FadeInVolTriger;

    guMediaCtrl( guPlayerPanel * playerpanel );
    ~guMediaCtrl();

    static bool     Init();

    //bool Load( const wxURI &uri );
    bool            Load( const wxString &uri, guPlayerPlayType playtype );
    bool            Stop( void );
    bool            Play( void );
    bool            Pause( void );
    void            ClearError( void );

    bool            Seek( wxFileOffset where );
    wxFileOffset    Tell( void );
    wxFileOffset    GetLength( void );

    double          GetVolume( void );
    bool            SetVolume( double volume );

    int             GetEqualizerBand( const int band );
    bool            SetEqualizer( const wxArrayInt &eqset );
    void            ResetEqualizer( void );
    void            SetEqualizerBand( const int band, const int value );

    //void            AboutToFinish( void );
    int  inline     GetLastError( void ) { return m_LastError; };
    void inline     SetLastError( const int error ) { m_LastError = error; };

    bool            EnableRecord( const wxString &path, const int format, const int quality );
    void            DisableRecord( void );
    bool            SetRecordFileName( const wxString &filename );


    void            AddPendingEvent( guMediaEvent &event );

    GstState        GetCurrentState( void ) { return m_CurrentState; }
    guMediaState    GetState( void );
    void            SetCurrentState( GstState state );

    bool            StartSink( GError ** error );
    bool            StartSinkLocked( wxArrayPtrVoid &messages, GError ** error );
    bool            StopSink( void );
    void            MaybeStopSink( void );
    void            ScheduleReap( void );

    void            Lock( void ) { m_FaderPlayBinsMutex.Lock(); }
    void            Unlock( void ) { m_FaderPlayBinsMutex.Unlock(); }

    bool            CanReuse( const wxString &uri, guFaderPlayBin * faderplaybin );

    void            UpdatedConfig( void );

    bool            IsBuffering( void ) { return m_Buffering; }

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
