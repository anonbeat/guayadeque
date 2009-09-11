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
#ifndef MUSICDNS_H
#define MUSICDNS_H

#include "DbLibrary.h"

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/xml/xml.h>

#undef ATTRIBUTE_PRINTF // there are warnings about redefined ATTRIBUTE_PRINTF in Fedora
#include <gst/gst.h>

DECLARE_EVENT_TYPE( guMUSICDNS_EVENT_PUID, wxID_ANY )
DECLARE_EVENT_TYPE( guMUSICDNS_EVENT_SEARCHDONE, wxID_ANY )

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

// -------------------------------------------------------------------------------- //
class guMusicDns : public wxEvtHandler
{
  protected :
    guTrack *           m_Track;
    wxString            m_Fingerprint;
    wxString            m_PUID;
    wxString            m_XmlDoc;
    guMusicDnsThread *  m_MusicDnsThread;

    bool DoGetFingerprint( void );
    bool DoGetMetadata( void );
    bool DoParseXmlDoc( void );
    bool ReadTrackInfo( wxXmlNode * XmlNode );
    void OnFingerprint( wxCommandEvent &event );

  public :
    guMusicDns( void );
    ~guMusicDns();

    void     SetTrack( guTrack * track );
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

    friend class guMusicDnsThread;
};

#endif
// -------------------------------------------------------------------------------- //
