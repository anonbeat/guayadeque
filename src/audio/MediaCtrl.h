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
#ifndef __MEDIACTRL_H__
#define __MEDIACTRL_H__

#include "DbLibrary.h"
#include "FaderPlaybin.h"

#undef ATTRIBUTE_PRINTF // there are warnings about redefined ATTRIBUTE_PRINTF in Fedora
#include <gst/gst.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
enum guMediaState
{
    guMEDIASTATE_STOPPED,
    guMEDIASTATE_PAUSED,
    guMEDIASTATE_PLAYING,
    guMEDIASTATE_ERROR
};

class guPlayerPanel;

// -------------------------------------------------------------------------------- //
class guMediaCtrl : public wxEvtHandler
{
  protected :
    guPlayerPanel *         m_PlayerPanel;

    wxMutex                 m_FaderPlayBinsMutex;
    guFaderPlayBinArray     m_FaderPlayBins;
    guFaderPlaybin *        m_CurrentPlayBin;

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

    bool                    m_EnableEq;
    bool                    m_EnableVolCtls;

    long                    m_ReplayGainMode;
    double                  m_ReplayGainPreAmp;
    //double                  m_ReplayGainFallback;

    bool                    m_ProxyEnabled;
    wxString                m_ProxyHost;
    int                     m_ProxyPort;
    wxString                m_ProxyUser;
    wxString                m_ProxyPass;
    wxString                m_ProxyServer;


    void                    CleanUp( void );
    bool                    RemovePlayBin( guFaderPlaybin * playbin );

    void                    FadeOutDone( guFaderPlaybin * faderplaybin );
    void                    FadeInStart( void );

  public :

    guMediaCtrl( guPlayerPanel * playerpanel );
    virtual ~guMediaCtrl();

    static bool     Init();

    guFaderPlaybin * CurrentPlayBin( void ) { return m_CurrentPlayBin; }

    void            UpdatePosition( void );

    long            Load( const wxString &uri, guFADERPLAYBIN_PLAYTYPE playtype, const int startpos = 0 );
    bool            Stop( void );
    bool            Play( void );
    bool            Pause( void );

    bool            Seek( wxFileOffset where, const bool accurate = false );
    wxFileOffset    Position( void );
    wxFileOffset    Length( void );

    bool            IsBuffering( void ) { return ( m_CurrentPlayBin && m_CurrentPlayBin->IsBuffering() ); }
    int             BufferSize( void ) { return m_BufferSize; }
    bool            IsRecording( void );

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

    int             GetLastError( void ) { return m_LastError; }
    void            SetLastError( const int error ) { m_LastError = error; }
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

    bool            ProxyEnabled() const;
    wxString        ProxyHost() const;
    int             ProxyPort() const;
    wxString        ProxyUser() const;
    wxString        ProxyPass() const;
    wxString        ProxyServer() const;

    void            ToggleEqualizer();
    void            ToggleVolCtl();

    void            ReconfigureRG();

    bool            IsEqualizerEnabled() { return m_CurrentPlayBin!= NULL ? m_CurrentPlayBin->IsEqualizerEnabled() : m_EnableEq; }
    bool            IsVolCtlsEnabled() { return m_CurrentPlayBin!= NULL ? m_CurrentPlayBin->IsVolCtlsEnabled() : m_EnableEq; }

    friend class guFaderPlaybin;
};

}

#endif
// -------------------------------------------------------------------------------- //
