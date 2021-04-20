// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#ifndef __FADERPLAYBIN_H__
#define __FADERPLAYBIN_H__

#include "MediaEvent.h"
#include "FaderTimeLine.h"
#include "GstTypes.h"
#include "GstUtils.h"
#include "Utils.h"

#include <gst/gst.h>

#include <wx/dynarray.h>
#include <wx/filename.h>
#include <wx/uri.h>

#include <memory>

namespace Guayadeque {

#define guEQUALIZER_BAND_COUNT  10

#define guFADERPLAYBIN_MESSAGE_FADEOUT_DONE     "guayadeque-fade-out-done"
#define guFADERPLAYBIN_MESSAGE_FADEIN_START     "guayadeque-fade-in-start"


enum guOutputDeviceSink {
    guOUTPUT_DEVICE_AUTOMATIC,
    guOUTPUT_DEVICE_GCONF,
    guOUTPUT_DEVICE_ALSA,
    guOUTPUT_DEVICE_PULSEAUDIO,
    guOUTPUT_DEVICE_OSS,
    guOUTPUT_DEVICE_OTHER
};

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

class guMediaCtrl;

// -------------------------------------------------------------------------------- //
class guFaderPlaybin
{
  // smart pointer features {
  public:
    typedef std::weak_ptr<guFaderPlaybin*> WeakPtr;

    WeakPtr             * GetWeakPtr() { return new WeakPtr( m_SharedPointer ); }

  protected :
    typedef std::shared_ptr<guFaderPlaybin*> Ptr;

    Ptr                 m_SharedPointer;
  // } smart pointer features

    
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

    wxFileOffset        m_PositionDelta;

    int                 m_ErrorCode;
    int                 m_State;
    gint64              m_PausePosition;

    //
    GstElement *        m_OutputSink;
    GstElement *        m_Playbin;
    GstElement *        m_Playbackbin;
    GstElement *        m_FaderVolume;
    GstElement *        m_ReplayGain;
    GstElement *        m_ReplayGainLimiter;
    GstElement *        m_Volume;
    GstElement *        m_Equalizer;
    GstElement *        m_Tee;
    GstElement *        m_Valve;

    GstElement *        m_RecordBin;
    GstElement *        m_Muxer                     = NULL;
    GstElement *        m_Encoder                   = NULL;

    GstElement *        m_FileSink;
    GstPad *            m_RecordSinkPad;
    GstPad *            m_TeeSrcPad;
    bool                m_SettingRecordFileName;
    wxString            m_LastRecordFileName;

    wxString            m_PendingNewRecordName;

    int                 m_StartOffset;
    int                 m_SeekTimerId;

    guGstElementsChain  m_PlayChain;


    bool                BuildPlaybackBin( void );
    bool                BuildOutputBin( void );
    bool                BuildRecordBin( const wxString &path );

  public :
    guFaderPlaybin( guMediaCtrl * mediactrl, const wxString &uri, const int playtype, const int startpos = 0 );
    virtual ~guFaderPlaybin();

    void                SendEvent( guMediaEvent &event );

    GstElement *        OutputSink( void ) { return m_OutputSink; }
    GstElement *        Playbin( void ) { return m_Playbin; }
    GstElement *        Volume( void ) { return m_Volume; }
    guMediaCtrl *       GetPlayer( void ) { return m_Player; }

    GstElement *        RecordBin( void ) { return m_RecordBin; }
    void                SetRecordBin( GstElement * recordbin ) { m_RecordBin = recordbin; }

    wxString            Uri( void ) { return m_Uri; }
    void                Lock( void ) { m_Lock.Lock(); }
    void                Unlock( void ) { m_Lock.Unlock(); }
    long                GetId( void ) { return m_Id; }
    void                SetId( const long id ) { m_Id = id; }

    int                 GetState( void ) { guLogDebug("guFaderPlaybin::GetState: %i", m_State); return m_State; }
    void                SetState( int state ) { guLogDebug("guFaderPlaybin::SetState: %i -> %i", m_State, state); m_State = state; }

    bool                IsBuffering( void ) { return m_IsBuffering; }
    void                SetBuffering( const bool isbuffering );

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

    bool                Seek( wxFileOffset where, const bool accurate = false );
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

    bool                DisableRecord( void );
    bool                DisableRecordAndStop( void );
    void                DisableRecordAndStop_finish( void );


    void                DisableRecordSync( int timeout_msec = 1000 );
    bool                SetRecordFileName( const wxString &filename );

    bool                AddRecordElement( GstPad * pad );

    bool                DoStartSeek( void );
    void                ToggleEqualizer( void );
    void                RefreshPlaybackItems( void );
    bool                IsEqualizerEnabled( void ) { return guIsGstElementLinked( m_Equalizer ); }
    bool                IsVolCtlsEnabled( void ) { return guIsGstElementLinked( m_Volume ); }
    bool                IsRecording( void ) { return guIsGstElementLinked( m_RecordBin ); }
    void                ToggleVolCtl( void );

    void                ReconfigureRG( void );
    void                SetRGProperties( void );

    void                SetValveDrop( bool drop );

    friend class guMediaCtrl;
    friend class guFaderTimeLine;
};
WX_DEFINE_ARRAY_PTR( guFaderPlaybin *, guFaderPlayBinArray );


}

#endif
// -------------------------------------------------------------------------------- //
