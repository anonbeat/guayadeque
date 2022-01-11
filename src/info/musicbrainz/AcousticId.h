// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
#ifndef __ACOUSTICID_H__
#define __ACOUSTICID_H__

#include "DbLibrary.h"

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include <gst/gst.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// Any class that want to use the guAcousticId must be of this class
class guAcousticIdClient
{
  public :
    virtual ~guAcousticIdClient() {}

    virtual void OnAcousticIdFingerprintFound( const wxString &fingerprint ) {}
    virtual void OnAcousticIdMBIdFound( const wxString &mbid ) {}

    virtual void OnAcousticIdError( const int errorcode ) {}

};

class guAcousticIdThread;

// -------------------------------------------------------------------------------- //
class guAcousticId
{
  public :
    enum guAcousticIdStatus {
        guAID_STATUS_OK,
        guAID_STATUS_ERROR_THREAD,
        guAID_STATUS_ERROR_GSTREAMER,
        guAID_STATUS_ERROR_NO_FINGERPRINT,
        guAID_STATUS_ERROR_HTTP,
        guAID_STATUS_ERROR_BAD_STATUS,
        guAID_STATUS_ERROR_XMLERROR,
        guAID_STATUS_ERROR_XMLPARSE
    };

  private :
    guAcousticIdThread *    m_AcousticIdThread;

  protected :
    guAcousticIdClient *    m_AcousticIdClient;
    const guTrack *         m_Track;
    wxString                m_Fingerprint;
    wxString                m_MBId;
    wxString                m_XmlDoc;
    int                     m_Status;

    bool                    DoGetFingerprint( void );
    bool                    DoGetMetadata( void );
    bool                    DoParseXmlDoc( void );
    void                    SetStatus( const int status );

    wxString                GetXmlDoc( void );
    void                    SetXmlDoc( const wxString &xmldoc );

    void                    SetFingerprint( const wxString &fingerprint );
    void                    SetFingerprint( const gchar * fingerprint );

    void                    SetMBId( const wxString &puid );
    void                    ClearAcousticIdThread( void );

  public :
    guAcousticId( guAcousticIdClient * acidclient );
    ~guAcousticId();

    void                    SearchTrack( const guTrack * track );
    void                    CancelSearch( void );

    wxString                GetMBId( void );
    int                     GetStatus( void );

    bool                    IsRunning( void );
    bool                    IsOk( void );


    friend class guAcousticIdThread;
};

}

#endif
// -------------------------------------------------------------------------------- //
