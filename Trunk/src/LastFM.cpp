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
#include "LastFM.h"

#include "Config.h"
#include "DbCache.h"
#include "Utils.h"

#include "curl/http.h"

#include <wx/arrimpl.cpp>
#include <wx/sstream.h>
#include <wx/xml/xml.h>

WX_DEFINE_OBJARRAY(guAlbumInfoArray);
WX_DEFINE_OBJARRAY(guSimilarArtistInfoArray);
WX_DEFINE_OBJARRAY(guSimilarTrackInfoArray);
WX_DEFINE_OBJARRAY(guEventInfoArray);

// -------------------------------------------------------------------------------- //
// guLastFMRequest
// -------------------------------------------------------------------------------- //
guLastFMRequest::guLastFMRequest()
{
}

// -------------------------------------------------------------------------------- //
guLastFMRequest::~guLastFMRequest()
{
}

// -------------------------------------------------------------------------------- //
void guLastFMRequest::AddArgument( const wxString &ArgName, const wxString &ArgValue, bool Filter )
{
    if( Filter )
    {
        AddArgument( guURLEncode( ArgName ), guURLEncode( ArgValue ) );
    }
    else
    {
        AddArgument( ArgName, ArgValue );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMRequest::AddArgument( const wxString &ArgName, const wxString &ArgValue )
{
    m_Args.Add( ArgName + wxT( '=' ) + ArgValue );
}

// -------------------------------------------------------------------------------- //
void guLastFMRequest::SetMethod( const wxString &MethodName )
{
    //Method = MethodName;
    m_Args.Add( wxT( "method=" ) + MethodName );
}

// -------------------------------------------------------------------------------- //
wxString guLastFMRequest::GetSign()
{
    wxSortedArrayString Params;
    wxString SignStr = wxEmptyString;
    guMD5 md5;
    int Index;
    int Count = m_Args.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SignStr += m_Args[ Index ].BeforeFirst( '=' );
        SignStr += m_Args[ Index ].AfterFirst( '=' );
    }
    SignStr += LASTFM_SHARED_SECRET;
    return md5.MD5( SignStr );
}

// -------------------------------------------------------------------------------- //
wxString guLastFMRequest::DoRequest( const bool AddSign, const bool IsGetAction )
{
    int         Index;
    int         Count;
    wxString    RetVal = wxEmptyString;
    wxCurlHTTP  http;
    char *      Buffer = NULL;
    wxString    UrlStr = wxEmptyString; // = LASTFM_API_ROOT;

    // TODO : Add a check to check if there was a recent request to last fm.
    //        LastFM can ban apps that do too much requests per second.
    //
    // TODO : Add a LastFM query cache where stores the queries and respective responses
    //
    Count = m_Args.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        UrlStr += ( Index ? wxT( "&" ) : wxT( "?" ) ) + m_Args[ Index ];
    }

    if( AddSign )
        UrlStr += wxT( "&api_sig=" ) + GetSign();

    if( IsGetAction )
    {
        UrlStr = LASTFM_API_ROOT + UrlStr;

        guDbCache * DbCache = guDbCache::GetDbCache();

        if( DbCache )
        {
            RetVal = DbCache->GetContent( UrlStr );
            //guLogMessage( wxT( "DbCache::GetContent:\n%s" ), RetVal.Mid( 0, 100 ).c_str() );
        }

        if( RetVal.IsEmpty() )
        {
            // Only with a UserAgent is accepted the Charset requested
            http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
            http.AddHeader( wxT( "Accept: text/html" ) );
            http.AddHeader( wxT( "Accept-Charset: utf-8" ) );

            guLogMessage( wxT( "LastFM.DoRequest %s\n" ), UrlStr.c_str() );

            http.Get( Buffer, UrlStr );

            if( Buffer )
            {
                RetVal = wxString( Buffer, wxConvUTF8 );
                if( RetVal.IsEmpty() )
                {
                    RetVal = wxString( Buffer, wxConvISO8859_1 );
                    if( RetVal.IsEmpty() )
                        RetVal = wxString( Buffer, wxConvLibc );
                }

                if( !RetVal.IsEmpty() )
                {
                    DbCache->SetContent( UrlStr, RetVal );
                }

                free( Buffer );
            }
        }
    }
    else
    {
        guLogMessage( wxT( "LastFM.DoRequest POST %s\n" ), UrlStr.c_str() );
        http.Post( UrlStr.Mid( 1 ).char_str(), UrlStr.Length() - 1, LASTFM_API_ROOT );
    }

//    else
//    {
//        guLogError( wxT( "DoRequest() : %s\nError: (%d) %s\nHeaders: %s\nBody: %s" ),
//                UrlStr.c_str(),
//                http.GetResponseCode(),
//                http.GetDetailedErrorString().c_str(),
//                http.GetResponseHeader().c_str(),
//                http.GetResponseBody().c_str() );
//    }

    //guLogMessage( wxT( "%s" ), RetVal.c_str() );

    return RetVal;
}

// -------------------------------------------------------------------------------- //
// guLastFM
// -------------------------------------------------------------------------------- //
guLastFM::guLastFM()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_UserName    = Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "LastFM" ) );
        m_Password    = Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "LastFM" ) );
        m_AuthSession = Config->ReadStr( wxT( "SessionKey" ), wxEmptyString, wxT( "LastFM" ) );
        m_Language    = Config->ReadStr( wxT( "Language" ), wxEmptyString, wxT( "LastFM" ) );
    }
    m_ErrorCode = 0;
    //DoAuthentication();
}

// -------------------------------------------------------------------------------- //
guLastFM::~guLastFM()
{
}

// -------------------------------------------------------------------------------- //
wxString guLastFM::GetAuthURL()
{
    return LASTFM_AUTH_ROOT wxT( "?api_key=" ) LASTFM_API_KEY wxT( "&token=" ) + m_AuthToken;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::IsOk()
{
    return m_ErrorCode == wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
wxString guLastFM::GetAuthSession()
{
    if( m_AuthSession.IsEmpty() )
    {
        m_AuthToken = AuthGetToken();
        if( IsOk() )
        {
            // Get Authorization From User in Navigator
            // Get Session
            m_AuthSession = AuthGetSession();
        }
    }
    return m_AuthSession;
}

// -------------------------------------------------------------------------------- //
wxString guLastFM::AuthGetToken()
{
    guLastFMRequest * Req = new guLastFMRequest();
    wxString Res;
    wxString Status;
    wxString Token = wxEmptyString;

    Req->SetMethod( wxT( "auth.gettoken" ) );
    Req->AddArgument( wxT( "api_key" ), LASTFM_API_KEY );

    Res = Req->DoRequest();
//    printf( "getToken() : " ); printf( Res.char_str() ); printf( "\n" );
//<?xml version="1.0" encoding="utf-8"?>
//<lfm status="ok">
//<token>6fe1b405832e101e73a70d0d6c1832f6</token></lfm>
    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "token" ) )
                    {
                        m_ErrorCode = 0;
                        Token = XmlNode->GetNodeContent();
                        //printf( Token.char_str() ); printf( "\n" );
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    //
    delete Req;

    return Token;
}

// -------------------------------------------------------------------------------- //
wxString guLastFM::AuthGetSession()
{
    guLastFMRequest * Req = new guLastFMRequest();
    wxString Res;
    wxString Status;
    wxString Key = wxEmptyString;

    Req->SetMethod( wxT( "auth.getsession" ) );
    Req->AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req->AddArgument( wxT( "token" ), m_AuthToken );
    Res = Req->DoRequest();

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "session" ) )
                    {
                        XmlNode = XmlNode->GetChildren();
                        while( XmlNode )
                        {
                            if( XmlNode->GetName() == wxT( "key" ) )
                            {
                                m_ErrorCode = 0;
                                Key = XmlNode->GetNodeContent();
                                //printf( Token.char_str() ); printf( "\n" );
                                break;
                            }
                            XmlNode = XmlNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    //
    delete Req;

    return Key;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::DoAuthentication()
{
    m_AuthToken = AuthGetToken();
    if( m_ErrorCode == 0 )
    {
        //printf( GetAuthURL().char_str() ); printf( "\n" );
        m_AuthSession = AuthGetSession();
        //printf( "AuthSession: " ); printf( AuthSession.char_str() ); printf( "\n" );
        if( m_AuthSession.IsEmpty() )
            guLogError( wxT( "LastFM::DoAuthentication ErrorCode: %d " ), m_ErrorCode );
        else
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::AlbumAddTags( const wxString &Artist, const wxString &Album, const wxString &Tags )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;

    Req.SetMethod( wxT( "album.addtags" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "sk" ), GetAuthSession() );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "album" ), Album, true );
    Req.AddArgument( wxT( "tags" ), Tags, true );
    Res = Req.DoRequest( true, false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    return true;
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guAlbumInfo guLastFM::AlbumGetInfo( const wxString &Artist, const wxString &Album )
{
    guLastFMRequest         Req; // = guLastFMRequest();
    wxString                Res;
    wxString                Status;
    guAlbumInfo             RetVal;
    wxString                ItemName;
    wxString                ImageSize;
    wxString                Tags = wxEmptyString;

    Req.SetMethod( wxT( "album.getinfo" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "album" ), Album, true );
    if( !m_Language.IsEmpty() )
    {
        Req.AddArgument( wxT( "lang" ), m_Language );
    }
    Res = Req.DoRequest( false );

    //printf( "====\n" ); printf( Res.char_str() ); printf( "====\n" );
    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode->GetName() == wxT( "album" ) )
                    {
                        XmlNode = XmlNode->GetChildren();
                        while( XmlNode )
                        {
                            ItemName = XmlNode->GetName();
                            if( ItemName == wxT( "name" ) )
                            {
                                RetVal.m_Name = XmlNode->GetNodeContent();
                            }
                            else if( ItemName == wxT( "artist" ) )
                            {
                                RetVal.m_Artist = XmlNode->GetNodeContent();
                            }
                            else if( ItemName == wxT( "releasedate" ) )
                            {
                                RetVal.m_ReleaseDate = XmlNode->GetNodeContent();
                            }
                            else if( ItemName == wxT( "url" ) )
                            {
                                RetVal.m_Url = XmlNode->GetNodeContent();
                            }
                            else if( ItemName == wxT( "image" ) )
                            {
                                XmlNode->GetPropVal( wxT( "size" ), &ImageSize );
                                //printf( "IMAGESIZE: " ); printf( ImageSize.char_str() ); printf( "\n" );
                                if( ImageSize == wxT( "large" ) && RetVal.m_ImageLink.IsEmpty() )
                                {
                                    RetVal.m_ImageLink = XmlNode->GetNodeContent();
                                }
                                else if( ImageSize == wxT( "extralarge" ) && !XmlNode->GetNodeContent().IsEmpty() )
                                {
                                    RetVal.m_ImageLink = XmlNode->GetNodeContent();
                                }
                            }
                            else if( ItemName == wxT( "toptags" ) )
                            {
                                wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                                while( XmlSubNode )
                                {
                                    if( XmlSubNode->GetName() == wxT( "tag" ) )
                                    {
                                        Tags += XmlSubNode->GetChildren()->GetNodeContent() + wxT( "," );
                                    }
                                    XmlSubNode = XmlSubNode->GetNext();
                                }
                                RetVal.m_Tags = Tags.RemoveLast( 1 );
                            }
                            XmlNode = XmlNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guLastFM::AlbumGetTags( const wxString &Artist, const wxString &Album )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    wxArrayString RetVal;

    Req.SetMethod( wxT( "album.gettags" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "sk" ), GetAuthSession() );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "album" ), Album, true );
    Res = Req.DoRequest();

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "tags" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode )
                        {
                            if( XmlSubNode->GetName() == wxT( "tag" ) )
                            {
                                XmlNode = XmlSubNode->GetChildren();
                                while( XmlNode )
                                {
                                    if( XmlNode->GetName() == wxT( "name" ) )
                                    {
                                        RetVal.Add( XmlNode->GetContent() );
                                        break;
                                    }
                                    XmlNode = XmlNode->GetNext();
                                }
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::AlbumRemoveTag( const wxString &Artist, const wxString &Album, const wxString &Tag )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;

    Req.SetMethod( wxT( "album.removetag" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "sk" ), GetAuthSession() );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "album" ), Album, true );
    Req.AddArgument( wxT( "tag" ), Tag, true );
    Res = Req.DoRequest( true, false ); // Append Sign and Do as Post

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    return true;
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::ArtistAddTags( const wxString &Artist, const wxString &Tags )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;

    Req.SetMethod( wxT( "album.addtags" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "sk" ), GetAuthSession() );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "tags" ), Tags, true );
    Res = Req.DoRequest( true, false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    return true;
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guArtistInfo guLastFM::ArtistGetInfo( const wxString &Artist )
{
    guLastFMRequest      Req; // = guLastFMRequest();
    wxString                Res;
    wxString                Status;
    guArtistInfo            RetVal;
    wxString                ItemName;
    wxString                ImageSize;

    Req.SetMethod( wxT( "artist.getinfo" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    if( !m_Language.IsEmpty() )
    {
        Req.AddArgument( wxT( "lang" ), m_Language );
    }
    Res = Req.DoRequest( false );

    //guLogMessage( wxT( "ArtistGetInfo:\n%s" ), Res.c_str() );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode->GetName() == wxT( "artist" ) )
                    {
                        XmlNode = XmlNode->GetChildren();
                        while( XmlNode )
                        {
                            ItemName = XmlNode->GetName();
                            if( ItemName == wxT( "name" ) )
                            {
                                RetVal.m_Name = XmlNode->GetNodeContent();
                            }
                            else if( ItemName == wxT( "url" ) )
                            {
                                RetVal.m_Url = XmlNode->GetNodeContent();
                            }
                            else if( ItemName == wxT( "image" ) )
                            {
                                XmlNode->GetPropVal( wxT( "size" ), &ImageSize );
                                if( ImageSize == wxT( "large" ) && RetVal.m_ImageLink.IsEmpty() )
                                {
                                    RetVal.m_ImageLink = XmlNode->GetNodeContent();
                                }
                                else if( ImageSize == wxT( "extralarge" ) && !XmlNode->GetNodeContent().IsEmpty() )
                                {
                                    RetVal.m_ImageLink = XmlNode->GetNodeContent();
                                }
                            }
                            else if( ItemName == wxT( "tags" ) )
                            {
                                wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                                while( XmlSubNode )
                                {
                                    if( XmlSubNode->GetName() == wxT( "tag" ) )
                                    {
                                        RetVal.m_Tags.Add( XmlSubNode->GetChildren()->GetNodeContent() );
                                    }
                                    XmlSubNode = XmlSubNode->GetNext();
                                }
                            }
                            else if( ItemName == wxT( "bio" ) )
                            {
                                wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                                while( XmlSubNode )
                                {
                                    if( XmlSubNode->GetName() == wxT( "summary" ) )
                                    {
                                        RetVal.m_BioSummary = XmlSubNode->GetNodeContent();
                                    } else if( XmlSubNode->GetName() == wxT( "content" ) )
                                    {
                                        RetVal.m_BioContent = XmlSubNode->GetNodeContent();
                                    }
                                    XmlSubNode = XmlSubNode->GetNext();
                                }
                            }
                            XmlNode = XmlNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guSimilarArtistInfoArray guLastFM::ArtistGetSimilar( const wxString &Artist )
{
    guLastFMRequest             Req; // = guLastFMRequest();
    wxString                    Res;
    wxString                    Status;
    guSimilarArtistInfoArray    RetVal;
    wxString                    ItemName;
    wxString                    ImageSize;
    wxString                    Tags = wxEmptyString;

    Req.SetMethod( wxT( "artist.getsimilar" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Res = Req.DoRequest( false ); // Does not require signature
//    <lfm status="ok">
//        <similarartists artist="Cher">
//            <artist>
//                <name>Tina Turner</name>
//                <mbid>9072df14-b61e-42e2-b4f4-6bbb7fdb5586</mbid>
//                <match>100</match>
//                <url>www.last.fm/music/Tina+Turner</url>
//                <image size="small">http://userserve-ak.last.fm/serve/34/72791.jpg</image>
//                <image size="medium">http://userserve-ak.last.fm/serve/64/72791.jpg</image>
//                <image size="large">http://userserve-ak.last.fm/serve/126/72791.jpg</image>
//                <streamable>1</streamable>
//            </artist>
//    ...

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode->GetName() == wxT( "similarartists" ) )
                    {
                        XmlNode = XmlNode->GetChildren();
                        while( XmlNode && XmlNode->GetName() == wxT( "artist" ) )
                        {
                            guSimilarArtistInfo * CurItem = new guSimilarArtistInfo();
                            wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                            while( XmlSubNode )
                            {
                                ItemName = XmlSubNode->GetName();
                                //guLogMessage( wxT( "%s : %s" ), ItemName.c_str(), XmlSubNode->GetNodeContent().c_str() );
                                if( ItemName == wxT( "name" ) )
                                {
                                    CurItem->m_Name = XmlSubNode->GetNodeContent();
                                }
                                else if( ItemName == wxT( "match" ) )
                                {
                                    CurItem->m_Match = XmlSubNode->GetNodeContent();
                                }
                                else if( ItemName == wxT( "url" ) )
                                {
                                    CurItem->m_Url = XmlSubNode->GetNodeContent();
                                }
                                else if( ItemName == wxT( "image" ) )
                                {
                                    XmlSubNode->GetPropVal( wxT( "size" ), &ImageSize );
                                    if( ImageSize == wxT( "large" ) && CurItem->m_ImageLink.IsEmpty() )
                                    {
                                        CurItem->m_ImageLink = XmlSubNode->GetNodeContent();
                                    }
                                    else if( ImageSize == wxT( "extralarge" ) && !XmlNode->GetNodeContent().IsEmpty() )
                                    {
                                        CurItem->m_ImageLink = XmlSubNode->GetNodeContent();
                                    }
                                }
                                XmlSubNode = XmlSubNode->GetNext();
                            }
                            RetVal.Add( CurItem );
                            XmlNode = XmlNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guLastFM::ArtistGetTags( const wxString &Artist )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    wxArrayString RetVal;

    Req.SetMethod( wxT( "artist.gettags" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "sk" ), GetAuthSession() );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Res = Req.DoRequest();

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "tags" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode )
                        {
                            if( XmlSubNode->GetName() == wxT( "tag" ) )
                            {
                                RetVal.Add( XmlSubNode->GetChildren()->GetContent() );
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guAlbumInfoArray guLastFM::ArtistGetTopAlbums( const wxString &Artist )
{
    guLastFMRequest  Req; // = guLastFMRequest();
    wxString            Res;
    wxString            Status;
    wxString            ItemName;
    wxString            ImageSize;
    guAlbumInfoArray    RetVal;
    guAlbumInfo *       CurItem;

    Req.SetMethod( wxT( "artist.gettopalbums" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "topalbums" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode )
                        {
                            if( XmlSubNode->GetName() == wxT( "album" ) )
                            {
                                CurItem = new guAlbumInfo();
                                if( CurItem )
                                {
                                    CurItem->m_Artist = Artist;
                                    XmlSubNode->GetPropVal( wxT( "rank" ), &CurItem->m_Rank );
                                    XmlNode = XmlSubNode->GetChildren();
                                    while( XmlNode )
                                    {
                                        ItemName = XmlNode->GetName();
                                        if( ItemName == wxT( "name" ) )
                                        {
                                            CurItem->m_Name = XmlNode->GetNodeContent();
                                        }
                                        else if( ItemName == wxT( "url" ) )
                                        {
                                            CurItem->m_Url = XmlNode->GetNodeContent();
                                        }
                                        else if( ItemName == wxT( "image" ) )
                                        {
                                            XmlNode->GetPropVal( wxT( "size" ), &ImageSize );
                                            //printf( "IMAGESIZE: " ); printf( ImageSize.char_str() ); printf( "\n" );
                                            if( ImageSize == wxT( "large" ) && CurItem->m_ImageLink.IsEmpty() )
                                            {
                                                CurItem->m_ImageLink = XmlNode->GetNodeContent();
                                            }
                                            else if( ImageSize == wxT( "extralarge" ) && !XmlNode->GetNodeContent().IsEmpty() )
                                            {
                                                CurItem->m_ImageLink = XmlNode->GetNodeContent();
                                            }
                                        }
                                        XmlNode = XmlNode->GetNext();
                                    }
                                    RetVal.Add( CurItem );
                                }
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guLastFM::ArtistGetTopTags( const wxString &Artist )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    wxArrayString RetVal;

    Req.SetMethod( wxT( "artist.gettoptags" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "toptags" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode )
                        {
                            XmlNode = XmlSubNode->GetChildren();
                            while( XmlNode )
                            {
                                if( XmlNode->GetName() == wxT( "name" ) )
                                {
                                    RetVal.Add( XmlNode->GetNodeContent() );
                                    break;
                                }
                                XmlNode = XmlNode->GetNext();
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guLastFM::ArtistGetTopTracks( const wxString &Artist )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    wxArrayString RetVal;

    Req.SetMethod( wxT( "artist.gettoptracks" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "toptracks" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode )
                        {
                            XmlNode = XmlSubNode->GetChildren();
                            while( XmlNode )
                            {
                                if( XmlNode->GetName() == wxT( "name" ) )
                                {
                                    RetVal.Add( XmlNode->GetNodeContent() );
                                    break;
                                }
                                XmlNode = XmlNode->GetNext();
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void ReadXmlArtistEventArtists( wxXmlNode * XmlNode, wxArrayString * Artists )
{
    while( XmlNode && XmlNode->GetName() == wxT( "artist" ) )
    {
        Artists->Add( XmlNode->GetNodeContent() );
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlArtistEventVenueLocationGeo( wxXmlNode * XmlNode, guEventInfo * Event )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "geo:lat" ) )
        {
            Event->m_LocationGeoLat = XmlNode->GetNodeContent();
        }
        if( XmlNode->GetName() == wxT( "geo:long" ) )
        {
            Event->m_LocationGeoLong = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlArtistEventVenueLocation( wxXmlNode * XmlNode, guEventInfo * Event )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "city" ) )
        {
            Event->m_LocationCity = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "country" ) )
        {
            Event->m_LocationCountry = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "street" ) )
        {
            Event->m_LocationStreet = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "postalcode" ) )
        {
            Event->m_LocationZipCode = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "geo:point" ) )
        {
            ReadXmlArtistEventVenueLocationGeo( XmlNode->GetChildren(), Event );
        }
        else if( XmlNode->GetName() == wxT( "timezone" ) )
        {
            Event->m_LocationTimeZone = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlArtistEventVenue( wxXmlNode * XmlNode, guEventInfo * Event )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "name" ) )
        {
            Event->m_LocationName = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "location" ) )
        {
            ReadXmlArtistEventVenueLocation( XmlNode->GetChildren(), Event );
        }
        else if( XmlNode->GetName() == wxT( "url" ) )
        {
            Event->m_LocationLink = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlArtistEvent( wxXmlNode * XmlNode, guEventInfo * Event )
{
    while( XmlNode )
    {
        //guLogMessage( wxT( "%s = %s" ), XmlNode->GetName().c_str(), XmlNode->GetNodeContent().c_str() );
        if( XmlNode->GetName() == wxT( "id" ) )
        {
            long Id;
            XmlNode->GetNodeContent().ToLong( &Id );
            Event->m_Id = Id;
        }
        else if( XmlNode->GetName() == wxT( "title" ) )
        {
            Event->m_Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "artists" ) )
        {
            ReadXmlArtistEventArtists( XmlNode->GetChildren(), &Event->m_Artists );
        }
        else if( XmlNode->GetName() == wxT( "venue" ) )
        {
            ReadXmlArtistEventVenue( XmlNode->GetChildren(), Event );
        }
        else if( XmlNode->GetName() == wxT( "startDate" ) )
        {
            Event->m_Date = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "startTime" ) )
        {
            Event->m_Time = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "description" ) )
        {
            Event->m_Description = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "description" ) )
        {
            Event->m_Description = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "image" ) )
        {
            wxString LastUrl = Event->m_ImageLink;
            Event->m_ImageLink = XmlNode->GetNodeContent();
            if( Event->m_ImageLink.IsEmpty() )
                Event->m_ImageLink = LastUrl;
        }
        else if( XmlNode->GetName() == wxT( "url" ) )
        {
            Event->m_Url = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlArtistEvents( wxXmlNode * XmlNode, guEventInfoArray * Events )
{
    while( XmlNode && XmlNode->GetName() == wxT( "event" ) )
    {
        guEventInfo * Event = new guEventInfo;
        ReadXmlArtistEvent( XmlNode->GetChildren(), Event );
        Events->Add( Event );

        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
guEventInfoArray guLastFM::ArtistGetEvents( const wxString &Artist )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    guEventInfoArray RetVal;

    Req.SetMethod( wxT( "artist.getEvents" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "events" ) )
                    {
                        ReadXmlArtistEvents( XmlNode->GetChildren(), &RetVal );
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guTrackInfo guLastFM::TrackGetInfo( const wxString &Artist, const wxString &Track )
{
    guLastFMRequest  Req; // = guLastFMRequest();
    wxString            Res;
    wxString            Status;
    guTrackInfo         RetVal;
    wxString            ItemName;

    Req.SetMethod( wxT( "track.getinfo" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "track" ), Track, true );
    if( !m_Language.IsEmpty() )
    {
        Req.AddArgument( wxT( "lang" ), m_Language );
    }
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "track" ) )
                    {
                        XmlNode = XmlNode->GetChildren();
                        while( XmlNode )
                        {
                            ItemName = XmlNode->GetName();
                            if( ItemName == wxT( "name" ) )
                            {
                                RetVal.m_TrackName = XmlNode->GetContent();
                            }
                            else if( ItemName == wxT( "url" ) )
                            {
                                RetVal.m_Url = XmlNode->GetContent();
                            }
                            else if( ItemName == wxT( "toptags" ) )
                            {
                                wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                                while( XmlSubNode && XmlSubNode->GetName() == wxT( "tag" ) )
                                {
                                    // Get Tag Property
                                    wxXmlNode * TagNode = XmlSubNode->GetChildren();
                                    while( TagNode )
                                    {
                                        if( TagNode->GetName() == wxT( "name" ) )
                                        {
                                            RetVal.m_TopTags.Add( TagNode->GetContent() );
                                            break;
                                        }
                                        TagNode = TagNode->GetNext();
                                    }
                                    XmlSubNode = XmlSubNode->GetNext();
                                }
                            }
                            else if( ItemName == wxT( "wiky" ) )
                            {
                                wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                                while( XmlSubNode )
                                {
                                    if( XmlSubNode->GetName() == wxT( "content" ) )
                                    {
                                        RetVal.m_Content = XmlSubNode->GetContent();
                                    }
                                    else if( XmlSubNode->GetName() == wxT( "summary" ) )
                                    {
                                        RetVal.m_Summary = XmlSubNode->GetContent();
                                    }
                                    XmlSubNode = XmlSubNode->GetNext();
                                }
                            }
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guSimilarTrackInfoArray guLastFM::TrackGetSimilar( const wxString &Artist, const wxString &Track )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    wxString ImageSize;
    guSimilarTrackInfoArray RetVal;

    Req.SetMethod( wxT( "track.getsimilar" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "track" ), Track, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "similartracks" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode && XmlSubNode->GetName() == wxT( "track" ) )
                        {
                            guSimilarTrackInfo * CurItem = new guSimilarTrackInfo();
                            if( CurItem )
                            {
                                XmlNode = XmlSubNode->GetChildren();
                                while( XmlNode )
                                {
                                    wxString ItemName = XmlNode->GetName();
                                    //wxLogWarning( wxT( "XmlNode: %s" ), ItemName.c_str() );
                                    if( ItemName == wxT( "name" ) )
                                    {
                                        CurItem->m_TrackName = XmlNode->GetNodeContent();
                                    }
                                    else if( ItemName == wxT( "match" ) )
                                    {
                                        CurItem->m_Match = XmlNode->GetNodeContent();
                                    }
                                    else if( ItemName == wxT( "url" ) )
                                    {
                                        CurItem->m_Url = XmlNode->GetNodeContent();
                                    }
                                    else if( ItemName == wxT( "artist" ) )
                                    {
                                        wxXmlNode * ArtistNode = XmlNode->GetChildren();
                                        while( ArtistNode )
                                        {
                                            //wxLogWarning( wxT( "ArtistXmlNode: %s" ), ArtistNode->GetName().c_str() );
                                            if( ArtistNode->GetName() == wxT( "name" ) )
                                            {
                                                CurItem->m_ArtistName = ArtistNode->GetNodeContent();
                                                break;
                                            }
                                            ArtistNode = ArtistNode->GetNext();
                                        }
                                    }
                                    else if( ItemName == wxT( "image" ) )
                                    {
                                        XmlNode->GetPropVal( wxT( "size" ), &ImageSize );
                                        if( ImageSize == wxT( "large" ) && CurItem->m_ImageLink.IsEmpty() )
                                        {
                                            CurItem->m_ImageLink = XmlNode->GetNodeContent();
                                        }
                                        else if( ImageSize == wxT( "extralarge" ) && !XmlNode->GetNodeContent().IsEmpty() )
                                        {
                                            CurItem->m_ImageLink = XmlNode->GetNodeContent();
                                        }
                                    }
                                    XmlNode = XmlNode->GetNext();
                                }
                                RetVal.Add( CurItem );
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guLastFM::TrackGetTags( const wxString &Artist, const wxString &Album )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    wxArrayString RetVal;

    Req.SetMethod( wxT( "track.gettags" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "sk" ), GetAuthSession() );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "album" ), Album, true );
    Res = Req.DoRequest();

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "tags" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode && XmlSubNode->GetName() == wxT( "tag" ) )
                        {
                            XmlNode = XmlSubNode->GetChildren();
                            while( XmlNode )
                            {
                                if( XmlNode->GetName() == wxT( "name" ) )
                                {
                                    RetVal.Add( XmlNode->GetContent() );
                                    break;
                                }
                                XmlNode = XmlNode->GetNext();
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guLastFM::TrackGetTopTags( const wxString &Artist, const wxString &Track )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;
    wxArrayString RetVal;

    Req.SetMethod( wxT( "track.gettoptags" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "track" ), Track, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "toptags" ) )
                    {
                        wxXmlNode * XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode && XmlSubNode->GetName() == wxT( "tag" ) )
                        {
                            XmlNode = XmlSubNode->GetChildren();
                            while( XmlNode )
                            {
                                if( XmlNode->GetName() == wxT( "name" ) )
                                {
                                    RetVal.Add( XmlNode->GetNodeContent() );
                                    break;
                                }
                                XmlNode = XmlNode->GetNext();
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::TrackRemoveTag( const wxString &Artist, const wxString &Track, const wxString &Tag )
{
    guLastFMRequest Req; // = guLastFMRequest();
    wxString Res;
    wxString Status;

    Req.SetMethod( wxT( "track.removetag" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "sk" ), GetAuthSession() );
    Req.AddArgument( wxT( "artist" ), Artist, true );
    Req.AddArgument( wxT( "track" ), Track, true );
    Req.AddArgument( wxT( "tag" ), Tag, true );
    Res = Req.DoRequest( true, false ); // Append Sign and Do as Post

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                //printf( XmlNode->GetName().char_str() ); printf( "\n" );
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                //printf( Status.char_str() ); printf( "\n" );
                if( Status == wxT( "ok" ) )
                {
                    return true;
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::TrackLove( const wxString &artist, const wxString &title )
{
    guLastFMRequest     Req; // = guLastFMRequest();
    wxString            Res;
    wxString            Status;
    wxString            ItemName;

    Req.SetMethod( wxT( "track.love" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), artist, true );
    Req.AddArgument( wxT( "track" ), title, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                if( Status == wxT( "ok" ) )
                {
                    return true;
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guLastFM::TrackBan( const wxString &artist, const wxString &title )
{
    guLastFMRequest     Req;
    wxString            Res;
    wxString            Status;
    wxString            ItemName;

    Req.SetMethod( wxT( "track.ban" ) );
    Req.AddArgument( wxT( "api_key" ), LASTFM_API_KEY );
    Req.AddArgument( wxT( "artist" ), artist, true );
    Req.AddArgument( wxT( "track" ), title, true );
    Res = Req.DoRequest( false );

    m_ErrorCode = wxNOT_FOUND;
    if( Res.Length() )
    {
        wxStringInputStream ins( Res );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "lfm" ) )
            {
                XmlNode->GetPropVal( wxT( "status" ), &Status );
                if( Status == wxT( "ok" ) )
                {
                    return true;
                }
                else if( Status == wxT( "failed" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    if( XmlNode && XmlNode->GetName() == wxT( "error" ) )
                    {
                        XmlNode->GetPropVal( wxT( "code" ), &Status );
                        Status.ToLong( ( long * ) &m_ErrorCode );
                    }
                }
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
