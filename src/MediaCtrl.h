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

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/uri.h>
//#include <wx/mediactrl.h>

#undef ATTRIBUTE_PRINTF // there are warnings about redefined ATTRIBUTE_PRINTF in Fedora
#include <gst/gst.h>

#define guEQUALIZER_BAND_COUNT  10

// ----------------------------------------------------------------------------
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

// Start_of_Ripped_Code_From_mediactrl_h
// This code is from mediactrl.h to avoid the need of the wxWidgets media library because its
// not included in many distributions by default.

// ----------------------------------------------------------------------------
enum wxMediaState
{
    wxMEDIASTATE_STOPPED,
    wxMEDIASTATE_PAUSED,
    wxMEDIASTATE_PLAYING
};

// ----------------------------------------------------------------------------
//
// wxMediaEvent
//
// ----------------------------------------------------------------------------
class wxMediaEvent : public wxNotifyEvent
{
  public:
    guLevelInfo m_LevelInfo;

    // ------------------------------------------------------------------------
    // wxMediaEvent Constructor
    //
    // Normal constructor, much the same as wxNotifyEvent
    // ------------------------------------------------------------------------
    wxMediaEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxNotifyEvent(commandType, winid)
    {                                       }

    // ------------------------------------------------------------------------
    // wxMediaEvent Copy Constructor
    //
    // Normal copy constructor, much the same as wxNotifyEvent
    // ------------------------------------------------------------------------
    wxMediaEvent(const wxMediaEvent &clone)
            : wxNotifyEvent(clone)
    {                                       }

    // ------------------------------------------------------------------------
    // wxMediaEvent::Clone
    //
    // Allocates a copy of this object.
    // Required for wxEvtHandler::AddPendingEvent
    // ------------------------------------------------------------------------
    virtual wxEvent *Clone() const
    {
        wxMediaEvent * pEvent = new wxMediaEvent(*this);
        if( pEvent )
            pEvent->m_LevelInfo = m_LevelInfo;
        return pEvent;
    }


//    // Put this class on wxWidget's RTTI table
//    DECLARE_DYNAMIC_CLASS(wxMediaEvent)
};

#define wxMEDIA_LOADED_ID      13002
DECLARE_EVENT_TYPE( wxEVT_MEDIA_LOADED,     wxMEDIA_LOADED_ID )
#define EVT_MEDIA_LOADED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_LOADED, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),

//Event ID to give to our events
#define wxMEDIA_ABOUT_TO_FINISH_ID 13006
#define wxMEDIA_FINISHED_ID    13000
#define wxMEDIA_STOP_ID    13001
//Define our event types - we need to call DEFINE_EVENT_TYPE(EVT) later
DECLARE_EVENT_TYPE( wxEVT_MEDIA_ABOUT_TO_FINISH, wxMEDIA_ABOUT_TO_FINISH_ID )
DECLARE_EVENT_TYPE( wxEVT_MEDIA_FINISHED, wxMEDIA_FINISHED_ID )
DECLARE_EVENT_TYPE( wxEVT_MEDIA_STOP,     wxMEDIA_STOP_ID )

//Function type(s) our events need
typedef void (wxEvtHandler::*wxMediaEventFunction)(wxMediaEvent&);

#define wxMediaEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxMediaEventFunction, &func)

//Macro for usage with message maps
#define EVT_MEDIA_ABOUT_TO_FINISH(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_ABOUT_TO_FINISH, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_FINISHED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_FINISHED, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_STOP(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_STOP, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),

#define wxMEDIA_STATECHANGED_ID      13003
#define wxMEDIA_PLAY_ID      13004
#define wxMEDIA_PAUSE_ID      13005
DECLARE_EVENT_TYPE( wxEVT_MEDIA_STATECHANGED, wxMEDIA_STATECHANGED_ID)
DECLARE_EVENT_TYPE( wxEVT_MEDIA_PLAY, wxMEDIA_PLAY_ID)
DECLARE_EVENT_TYPE( wxEVT_MEDIA_PAUSE, wxMEDIA_PAUSE_ID)
#define EVT_MEDIA_STATECHANGED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_STATECHANGED, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_PLAY(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_PLAY, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_PAUSE(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_PAUSE, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),

// End_of_Ripped_Code_From_mediactrl_h

DECLARE_EVENT_TYPE( wxEVT_MEDIA_TAG, wxID_ANY )
DECLARE_EVENT_TYPE( wxEVT_MEDIA_BUFFERING, wxID_ANY )
DECLARE_EVENT_TYPE( wxEVT_MEDIA_BITRATE, wxID_ANY )
DECLARE_EVENT_TYPE( wxEVT_MEDIA_LEVEL, wxID_ANY )
DECLARE_EVENT_TYPE( wxEVT_MEDIA_ERROR, wxID_ANY )
#define EVT_MEDIA_TAG(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_TAG, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_BUFFERING(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_BUFFERING, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_BITRATE(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_BITRATE, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_LEVEL(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_LEVEL, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_ERROR(winid, fn) DECLARE_EVENT_TABLE_ERROR( wxEVT_MEDIA_ERROR, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),

class guPlayerPanel;

// -------------------------------------------------------------------------------- //
// guMediaCtrl : Interface class for gstreamer
// -------------------------------------------------------------------------------- //
class guMediaCtrl : public wxEvtHandler
{
  private :
    guPlayerPanel * m_PlayerPanel;
    wxLongLong      m_llPausedPos;
    int             m_LastError;

    bool            SetProperty( GstElement * element, const char * name, gint64 value );

    GstElement *    BuildOutputBin( void );
    GstElement *    BuildPlaybackBin( GstElement * outputsink );
    GstElement *    BuildRecordBin( void );

  public :
    GstElement * m_Playbin;
    GstElement * m_Tee;
    GstElement * m_Volume;
    GstElement * m_Equalizer;
    bool         m_Buffering;
    bool         m_WasPlaying;

    guMediaCtrl( guPlayerPanel * playerpanel );
    ~guMediaCtrl();

    static bool Init();

    //bool Load( const wxURI &uri );
    bool Load( const wxString &uri, bool restart = true );
    bool Stop();
    bool Play();
    bool Pause();
    void ClearError();

    bool Seek( wxLongLong where );
    wxFileOffset Tell();
    wxFileOffset GetLength();

    wxMediaState GetState();

    double GetVolume();
    bool SetVolume( double volume );

    int GetEqualizerBand( const int band );
    bool SetEqualizer( const wxArrayInt &eqset );
    void ResetEqualizer( void );
    void SetEqualizerBand( const int band, const int value );

    void inline AboutToFinish( void );
    int  inline GetLastError( void ) { return m_LastError; };
    void inline SetLastError( const int error ) { m_LastError = error; };

};

#endif
// -------------------------------------------------------------------------------- //
