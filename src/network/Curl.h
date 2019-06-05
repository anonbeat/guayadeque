// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#ifndef __CURL_H__
#define __CURL_H__

#include <wx/string.h>
#include <wx/mstream.h>

#include <curl/curl.h>

namespace Guayadeque {

#define wxCHARBUFFER_ISEMPTY( x )      ( strlen( x ) == 0 )

#define CURL_VERBOSE_MODE               0L
#define CURL_FOLLOW_LOCATIONS           1L
#define CURL_MAX_REDIRS                 3L
#define CURL_NO_SIGNALS                 1L


// -------------------------------------------------------------------------------- //
class guCurl
{
  protected:
    CURL *                  m_CurlHandle;

    wxCharBuffer            m_Url;

    int                     m_Port;
    wxCharBuffer            m_User;
    wxCharBuffer            m_Password;
    wxCharBuffer            m_CurlUserPass;


    long                    m_ResCode;
    wxCharBuffer            m_ResHeader;
    wxCharBuffer            m_ResData;


    struct curl_slist *     m_CurlHeaders;
    wxArrayString           m_ReqHeaders;

    bool                    m_UseProxy;
    wxCharBuffer            m_ProxyHost;
    int                     m_ProxyPort;
    wxCharBuffer            m_ProxyUser;
    wxCharBuffer            m_ProxyPassword;
    wxCharBuffer            m_CurlProxyUserPass;

    char                    m_ErrorBuffer[ CURL_ERROR_SIZE ];

  protected :

    virtual void            SetDefaultOptions( const wxString &url );

    virtual void            SetHeaders();
    virtual void            CleanHeaders();

    virtual void            ResetResVars();

    bool                    SetStringReadOption( const wxCharBuffer &str );
    bool                    SetStringWriteOption( const wxCharBuffer &str );
    bool                    SetStreamReadOption( const wxInputStream &stream );
    bool                    SetStreamWriteOption( const wxOutputStream &stream );

  public :
    guCurl( const wxString &url = wxEmptyString );
    virtual ~guCurl();

    static void             CurlInit();         // Must be called once before any curl use
    static void             CurlDone();         // Must be called once on application close

    bool                    InitHandle();
    bool                    CleanupHandle();

    bool                    ReInitHandle();
    void                    ResetHandle();

    bool                    Perform();

    void                    SetUrl( const wxString &url );
    wxString                GetUrl() const;

    void                    SetPort( const long &port );
    long                    GetPort() const;
    void                    SetUsername( const wxString &username  );
    wxString                GetUsername() const;
    void                    SetPassword( const wxString &password );
    wxString                GetPassword() const;

    wxString                GetResHeader() const;
    wxString                GetResData() const;
    long                    GetResCode() const;

    void                    SetUseProxy( const bool useproxy );
    bool                    GetUseProxy() const;
    void                    SetProxyHost( const wxString &proxyhost );
    wxString                GetProxyHost() const;
    void                    SetProxyPort( const long &proxyport );
    long                    GetProxyPort() const;
    void                    SetProxyUser( const wxString &proxyuser );
    wxString                GetProxyUser() const;
    void                    SetProxyPassword( const wxString &proxypass );
    wxString                GetProxyPassword() const;

    bool                    SetOption( CURLoption option, long value );
    bool                    SetOption( CURLoption option, wxCharBuffer &value );
    bool                    SetOption( CURLoption option, void * value );

    bool                    GetInfo( CURLINFO info, long * value );

};

}

#endif
// -------------------------------------------------------------------------------- //
