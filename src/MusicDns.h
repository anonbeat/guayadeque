// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#ifndef MUSICDNS_H
#define MUSICDNS_H

#include "DbLibrary.h"

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/xml/xml.h>

#undef ATTRIBUTE_PRINTF // there are warnings about redefined ATTRIBUTE_PRINTF in Fedora
#include <gst/gst.h>

class guMusicDns;

// -------------------------------------------------------------------------------- //
class guMusicDnsThread : public wxThread
{
  protected :
    wxString        m_FileName;
    bool            m_Running;
    guMusicDns *    m_MusicDns;
    GstElement *    m_Pipeline;

  public :
    guMusicDnsThread( guMusicDns * musicdns, const wxChar * filename );
    ~guMusicDnsThread();

    virtual ExitCode    Entry();
    void                SetFingerprint( const char * fingerprint );
    void                Stop( void );
};

class guMusicBrainz;

#define guMDNS_STATUS_OK                    0
#define guMDNS_STATUS_ERROR_THREAD          1
#define guMDNS_STATUS_ERROR_GSTREAMER       2
#define guMDNS_STATUS_ERROR_NO_FINGERPRINT  3
#define guMDNS_STATUS_ERROR_HTTP            4
#define guMDNS_STATUS_ERROR_NOXMLDATA       5
#define guMDNS_STATUS_ERROR_XMLERROR        6
#define guMDNS_STATUS_ERROR_XMLPARSE        7

// -------------------------------------------------------------------------------- //
class guMusicDns
{
  protected :
    guMusicBrainz *     m_MusicBrainz;
    const guTrack *     m_Track;
    wxString            m_Fingerprint;
    wxString            m_PUID;
    wxString            m_XmlDoc;
    guMusicDnsThread *  m_MusicDnsThread;
    int                 m_Status;

    bool DoGetFingerprint( void );
    bool DoGetMetadata( void );
    bool DoParseXmlDoc( void );
    bool ReadTrackInfo( wxXmlNode * XmlNode );
    void     SetStatus( const int status );

  public :
    guMusicDns( guMusicBrainz * musicbrainz );
    ~guMusicDns();

    void     SetTrack( const guTrack * track );
    wxString GetXmlDoc( void );
    void     SetXmlDoc( const wxString &xmldoc );
    wxString GetFingerprint( void );
    void     SetFingerprint( const wxString &fingerprint );
    void     SetFingerprint( const char * fingerprint );

    void     SetPUID( const wxString &puid );
    void     SetPUID( const char * puid );
    wxString GetPUID( void );

    void     ClearMusicDnsThread( void );

    bool     IsRunning( void );
    void     CancelSearch( void );

    int      GetStatus( void );
    bool     IsOk( void );

    friend class guMusicDnsThread;
};

#endif
// -------------------------------------------------------------------------------- //
