// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 3, or ( at your option )
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
#include "Curl.h"

#include "Config.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
extern "C"
{

size_t string_write_callback( void * buffer, size_t size, size_t nitems, wxCharBuffer * outstring )
{
    size_t Size = size * nitems;
    if( outstring )
    {
        wxString str = wxString( * outstring, wxConvLibc ) + wxString( ( const char * ) buffer, wxConvLibc );
        * outstring = str.ToAscii();
    }

    return Size;
}

size_t string_read_callback( void * buffer, size_t size, size_t nitems, wxCharBuffer * instring )
{
    size_t Retval = 0;
    if( instring )
    {
        size_t BufLen = size * nitems;
        size_t InLen = instring->length();
        Retval = InLen > BufLen ? BufLen : InLen;
        memcpy( ( char * ) buffer, instring->data(), Retval );

        wxString remaining = wxString( instring->data(), wxConvLibc ).Right( InLen - Retval );
        * instring = remaining.ToAscii();
    }

    return Retval;
}

size_t stream_write_callback( void * buffer, size_t size, size_t nitems, wxOutputStream * outstream )
{
    if( outstream )
    {
        size_t Size = size * nitems;

        outstream->Write( buffer, Size );

        return outstream->LastWrite();
    }
    return 0;
}

size_t stream_read_callback( void * buffer, size_t size, size_t nitems, wxInputStream * instream )
{
    if( instream )
    {
        size_t Size = size * nitems;

        instream->Read( buffer, Size );

        return instream->LastRead();
    }

    return 0;
}

}

// -------------------------------------------------------------------------------- //
guCurl::guCurl( const wxString &url )
{
    m_CurlHandle = NULL;
    m_Url = url.ToAscii();
    m_User = "";
    m_Password = "";
    m_Port = 0;
    m_ResCode = -1;
    m_CurlHeaders = NULL;

    guConfig * Config = guConfig::Get();

    m_UseProxy = Config->ReadBool( CONFIG_KEY_PROXY_ENABLED, false, CONFIG_PATH_PROXY );
    m_ProxyHost = Config->ReadStr( CONFIG_KEY_PROXY_HOSTNAME, wxEmptyString, CONFIG_PATH_PROXY ).ToAscii();
    m_ProxyPort = Config->ReadNum( CONFIG_KEY_PROXY_PORT, 8080, CONFIG_PATH_PROXY );
    m_ProxyUser = Config->ReadStr( CONFIG_KEY_PROXY_USERNAME, wxEmptyString, CONFIG_PATH_PROXY ).ToAscii();
    m_ProxyPassword = Config->ReadStr( CONFIG_KEY_PROXY_PASSWORD, wxEmptyString, CONFIG_PATH_PROXY ).ToAscii();

    m_ErrorBuffer[ 0 ] = 0;
}

// -------------------------------------------------------------------------------- //
guCurl::~guCurl()
{
    CleanHeaders();
    CleanupHandle();
}

// -------------------------------------------------------------------------------- //
bool guCurl::InitHandle()
{
    if( m_CurlHandle )
        return false;

    m_CurlHandle = curl_easy_init();

    return ( m_CurlHandle != NULL );
}

// -------------------------------------------------------------------------------- //
bool guCurl::CleanupHandle()
{
    if( m_CurlHandle )
    {
        curl_easy_cleanup( m_CurlHandle );
        m_CurlHandle = NULL;
    }

    return true;
}

// -------------------------------------------------------------------------------- //
bool guCurl::ReInitHandle()
{
    CleanupHandle();
    return InitHandle();
}

// -------------------------------------------------------------------------------- //
void guCurl::ResetHandle()
{
    curl_easy_reset( m_CurlHandle );
}

// -------------------------------------------------------------------------------- //
bool guCurl::Perform()
{
    CURLcode res = curl_easy_perform( m_CurlHandle );

    GetInfo( CURLINFO_RESPONSE_CODE, &m_ResCode );

    return ( res == CURLE_OK );
}

// -------------------------------------------------------------------------------- //
void guCurl::SetUrl( const wxString &url )
{
    m_Url = url.ToAscii();
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetUrl() const
{
    return wxString( m_Url, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
void guCurl::SetPort( const long &port )
{
    m_Port = port;
}

// -------------------------------------------------------------------------------- //
long guCurl::GetPort() const
{
    return m_Port;
}

// -------------------------------------------------------------------------------- //
void guCurl::SetUsername( const wxString &username )
{
    m_User = username.ToAscii();
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetUsername() const
{
    return wxString( m_User, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
void guCurl::SetPassword( const wxString &password )
{
    m_Password = password.ToAscii();
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetPassword() const
{
    return wxString( m_Password, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetResHeader() const
{
    return wxString( m_ResHeader, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetResData() const
{
    return wxString( m_ResData, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
long guCurl::GetResCode() const
{
    return m_ResCode;
}

// -------------------------------------------------------------------------------- //
void guCurl::SetUseProxy( const bool useproxy )
{
    m_UseProxy = useproxy;
}

// -------------------------------------------------------------------------------- //
bool guCurl::GetUseProxy() const
{
    return m_UseProxy;
}

// -------------------------------------------------------------------------------- //
void guCurl::SetProxyHost( const wxString &proxyhost )
{
    m_ProxyHost = proxyhost.ToAscii();
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetProxyHost() const
{
    return wxString( m_ProxyHost, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
void guCurl::SetProxyUser( const wxString &proxyusername )
{
    m_ProxyUser = proxyusername.ToAscii();
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetProxyUser() const
{
    return wxString( m_ProxyUser, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
void guCurl::SetProxyPassword( const wxString &proxypassword )
{
    m_ProxyPassword = proxypassword.ToAscii();
}

// -------------------------------------------------------------------------------- //
wxString guCurl::GetProxyPassword() const
{
    return wxString( m_ProxyPassword, wxConvLibc );
}

// -------------------------------------------------------------------------------- //
void guCurl::SetProxyPort( const long &proxyport )
{
    m_ProxyPort = proxyport;
}

// -------------------------------------------------------------------------------- //
long guCurl::GetProxyPort() const
{
    return m_ProxyPort;
}

// -------------------------------------------------------------------------------- //
bool guCurl::SetOption( CURLoption option, long value )
{
    return curl_easy_setopt( m_CurlHandle, option, value ) == CURLE_OK;
}

// -------------------------------------------------------------------------------- //
bool guCurl::SetOption( CURLoption option, wxCharBuffer &value )
{
    return curl_easy_setopt( m_CurlHandle, option, ( const char * ) value ) == CURLE_OK;
}

// -------------------------------------------------------------------------------- //
bool guCurl::SetOption( CURLoption option, void * value )
{
    return curl_easy_setopt( m_CurlHandle, option, value ) == CURLE_OK;
}

// -------------------------------------------------------------------------------- //
bool guCurl::SetStringReadOption( const wxCharBuffer &str )
{
    return SetOption( CURLOPT_READFUNCTION, ( void * ) string_read_callback ) &&
           SetOption( CURLOPT_READDATA, ( void * ) &str );
}

// -------------------------------------------------------------------------------- //
bool guCurl::SetStringWriteOption( const wxCharBuffer &str )
{
    return SetOption( CURLOPT_WRITEFUNCTION, ( void * ) string_write_callback ) &&
           SetOption( CURLOPT_WRITEDATA, ( void * ) &str );
}

// -------------------------------------------------------------------------------- //
bool guCurl::SetStreamReadOption( const wxInputStream &stream )
{
    return SetOption( CURLOPT_READFUNCTION, ( void * ) stream_read_callback ) &&
           SetOption( CURLOPT_READDATA, ( void * ) &stream );
}

// -------------------------------------------------------------------------------- //
bool guCurl::SetStreamWriteOption( const wxOutputStream &stream )
{
    return SetOption( CURLOPT_WRITEFUNCTION, ( void * ) stream_write_callback ) &&
           SetOption( CURLOPT_WRITEDATA, ( void * ) &stream );
}

// -------------------------------------------------------------------------------- //
bool guCurl::GetInfo( CURLINFO info, long * value )
{
    return curl_easy_getinfo( m_CurlHandle, info, value ) == CURLE_OK;
}

// -------------------------------------------------------------------------------- //
void guCurl::SetDefaultOptions( const wxString &url )
{
    if( !url.empty() )
        SetUrl( url );

    if( m_CurlHandle )
    {
        ResetHandle();
    }
    else
    {
        InitHandle();
    }
    ResetResVars();

    SetOption( CURLOPT_URL, m_Url );

    SetOption( CURLOPT_HEADERFUNCTION, ( void * ) string_write_callback );
    SetOption( CURLOPT_HEADERDATA, ( void * ) &m_ResHeader );

    SetOption( CURLOPT_ERRORBUFFER, ( void * ) m_ErrorBuffer );

    if( !wxCHARBUFFER_ISEMPTY( m_User ) &&
        !wxCHARBUFFER_ISEMPTY( m_Password ) )
    {
        wxString str = wxString( m_User, wxConvLibc ) +
                       wxT( ":" ) +
                       wxString( m_Password, wxConvLibc );

        m_CurlUserPass = str.ToAscii();
        SetOption( CURLOPT_USERPWD, m_CurlUserPass );

        SetOption( CURLOPT_HTTPAUTH, CURLAUTH_ANY );
    }

    if( m_Port )
    {
        SetOption( CURLOPT_PORT, m_Port );
    }

    if( m_UseProxy )
    {
        if( !wxCHARBUFFER_ISEMPTY( m_ProxyHost ) )
        {
            SetOption( CURLOPT_PROXY, m_ProxyHost );
        }

        if( m_ProxyPort )
        {
            SetOption( CURLOPT_PROXYPORT, m_ProxyPort );
        }

        if( !wxCHARBUFFER_ISEMPTY( m_ProxyUser ) &&
            !wxCHARBUFFER_ISEMPTY( m_ProxyPassword ) )
        {
            wxString str = wxString( m_ProxyUser, wxConvLibc ) +
                           wxT( ":" ) +
                           wxString( m_ProxyPassword, wxConvLibc );

            m_CurlProxyUserPass = str.ToAscii();
            SetOption( CURLOPT_PROXYUSERPWD, m_CurlProxyUserPass );
        }
    }

    SetOption( CURLOPT_VERBOSE, CURL_VERBOSE_MODE );
    SetOption( CURLOPT_NOSIGNAL, CURL_NO_SIGNALS );
    SetOption( CURLOPT_FOLLOWLOCATION, CURL_FOLLOW_LOCATIONS );
    SetOption( CURLOPT_MAXREDIRS, CURL_MAX_REDIRS );
}

// -------------------------------------------------------------------------------- //
void guCurl::SetHeaders()
{
    if( !m_ReqHeaders.IsEmpty() )
    {
        if( m_CurlHeaders )
        {
            curl_slist_free_all( m_CurlHeaders );

            m_CurlHeaders = NULL;

            SetOption( CURLOPT_HTTPHEADER, ( void * ) m_CurlHeaders );
        }

        int Count = m_ReqHeaders.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            m_CurlHeaders = curl_slist_append( m_CurlHeaders, m_ReqHeaders[ Index ].ToAscii() );
        }

        SetOption( CURLOPT_HTTPHEADER, ( void * ) m_CurlHeaders );
    }
}

// -------------------------------------------------------------------------------- //
void guCurl::CleanHeaders()
{
    m_ReqHeaders.Clear();

    if( m_CurlHeaders )
    {
        curl_slist_free_all( m_CurlHeaders );
        m_CurlHeaders = NULL;

        SetOption( CURLOPT_HTTPHEADER, ( void * ) m_CurlHeaders );
    }
}

// -------------------------------------------------------------------------------- //
void guCurl::ResetResVars()
{
    m_ResHeader = "";
    m_ResData = "";
    m_ResCode = -1;
}

// -------------------------------------------------------------------------------- //
void guCurl::CurlInit()
{
    curl_global_init( CURL_GLOBAL_ALL );
}

// -------------------------------------------------------------------------------- //
void guCurl::CurlDone()
{
    curl_global_cleanup();
}


}

// -------------------------------------------------------------------------------- //
