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
#include "Discogs.h"

#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>

#define guDISCOGS_BASEURL           "http://www.discogs.com"
#define guDISCOGS_QUERY_APIKEY      "b955caf59c"
#define guDISCOGS_QUERY_ARTIST      guDISCOGS_BASEURL "/artist/%s?f=xml&api_key=" guDISCOGS_QUERY_APIKEY
#define guDISCOGS_QUERY_RELEASE     guDISCOGS_BASEURL "/release/%u?f=xml&api_key=" guDISCOGS_QUERY_APIKEY
#define guDISCOGS_QUERY_SEARCH      guDISCOGS_BASEURL "/search?type=all&q=%s&f=xml&api_key=" guDISCOGS_QUERY_APIKEY

#define guDISCOGS_REQUEST_ITEMS     10

// -------------------------------------------------------------------------------- //
WX_DEFINE_OBJARRAY( guDiscogsImageArray );
WX_DEFINE_OBJARRAY( guDiscogsTrackArray );
WX_DEFINE_OBJARRAY( guDiscogsReleaseArray );

// -------------------------------------------------------------------------------- //
guDiscogs::guDiscogs()
{
}

// -------------------------------------------------------------------------------- //
guDiscogs::~guDiscogs()
{
}

// -------------------------------------------------------------------------------- //
void GetImages( wxXmlNode * XmlNode, guDiscogsImageArray * images )
{
    wxASSERT( XmlNode );
    wxASSERT( images );

    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "image" ) )
        {
            guDiscogsImage * Image = new guDiscogsImage();
            XmlNode->GetPropVal( wxT( "uri" ), &Image->m_Url );
            wxString Value;
            XmlNode->GetPropVal( wxT( "width" ), &Value );
            Value.ToLong( &Image->m_Width );
            XmlNode->GetPropVal( wxT( "height" ), &Value );
            Value.ToLong( &Image->m_Height );
            images->Add( Image );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
wxArrayString GetUrls( wxXmlNode * XmlNode )
{
    wxASSERT( XmlNode );

    wxArrayString RetVal;
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "url" ) )
        {
            RetVal.Add( XmlNode->GetNodeContent() );
        }
        XmlNode = XmlNode->GetNext();
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void GetRelease( wxXmlNode * XmlNode, guDiscogsRelease * release )
{
    wxASSERT( XmlNode );
    wxASSERT( release );

    //guLogMessage( wxT( "GetRelease..." ) );
    while( XmlNode )
    {
        //guLogMessage( wxT( "GetReleases >> %s" ), XmlNode->GetName().c_str() );
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            release->m_Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "format" ) )
        {
            release->m_Format = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "year" ) )
        {
            long LongValue;
            XmlNode->GetNodeContent().ToLong( &LongValue );
            release->m_Year = LongValue;
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void GetReleases( wxXmlNode * XmlNode, guDiscogsReleaseArray * releases )
{
    wxASSERT( XmlNode );
    wxASSERT( releases );

    //guLogMessage( wxT( "GetReleases..." ) );
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "release" ) )
        {
            //guLogMessage( wxT( "reading release..." ) );
            guDiscogsRelease * Release = new guDiscogsRelease();
            wxString Value;
            long LongValue;

            XmlNode->GetPropVal( wxT( "id" ), &Value );
            Value.ToLong( &LongValue );
            Release->m_Id = LongValue;

            XmlNode->GetPropVal( wxT( "type" ), &Release->m_Type );
            GetRelease( XmlNode->GetChildren(), Release );
            releases->Add( Release );
        }
        else
        {
            guLogError( wxT( "GetReleases: the node is not a release" ) );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void GetTrack( wxXmlNode * XmlNode, guDiscogsTrack * track )
{
    wxASSERT( XmlNode );
    wxASSERT( track );

    while( XmlNode )
    {
        wxString Value;
        if( XmlNode->GetName() == wxT( "position" ) )
        {
            long LongValue;
            XmlNode->GetNodeContent().ToLong( &LongValue );
            track->m_Position = LongValue;
        }
        else if( XmlNode->GetName() == wxT( "title" ) )
        {
            track->m_Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "duration" ) )
        {
            Value = XmlNode->GetNodeContent();
            long minutes;
            long seconds;
            Value.BeforeFirst( wxT( ':' ) ).ToLong( &minutes );
            Value.AfterFirst( wxT( ':' ) ).ToLong( &seconds );
            track->m_Length = ( minutes * 60 ) + seconds;
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void GetTracks( wxXmlNode * XmlNode, guDiscogsTrackArray * tracks )
{
    wxASSERT( XmlNode );
    wxASSERT( tracks );

    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "track" ) )
        {
            guDiscogsTrack * Track = new guDiscogsTrack();
            GetTrack( XmlNode->GetChildren(), Track );
            tracks->Add( Track );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guDiscogs::GetArtist( const wxString &name, guDiscogsArtist * artist )
{
    wxASSERT( !name.IsEmpty() );
    wxASSERT( artist );

    wxString SearchUrl;
    SearchUrl = wxString::Format( wxT( guDISCOGS_QUERY_ARTIST ), guURLEncode( name ).c_str() );

    wxString Content = GetUrlContent( SearchUrl, wxEmptyString, true );

    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );

    if( !Content.IsEmpty() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && ( XmlNode->GetName() == wxT( "resp" ) ) )
        {
            wxString Status;
            XmlNode->GetPropVal( wxT( "stat" ), &Status );
            if( Status == wxT( "ok" ) )
            {
                XmlNode = XmlNode->GetChildren();
                if( XmlNode->GetName() == wxT( "artist" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    while( XmlNode )
                    {
                        //guLogMessage( wxT( "GetArtist >> %s" ), XmlNode->GetName().c_str() );
                        if( XmlNode->GetName() == wxT( "images" ) )
                        {
                            GetImages( XmlNode->GetChildren(), &artist->m_Images );
                        }
                        else if( XmlNode->GetName() == wxT( "name" ) )
                        {
                            artist->m_Name = XmlNode->GetNodeContent();
                        }
                        else if( XmlNode->GetName() == wxT( "realname" ) )
                        {
                            artist->m_RealName = XmlNode->GetNodeContent();
                        }
                        else if( XmlNode->GetName() == wxT( "urls" ) )
                        {
                            artist->m_Urls = GetUrls( XmlNode->GetChildren() );
                        }
                        else if( XmlNode->GetName() == wxT( "releases" ) )
                        {
                            GetReleases( XmlNode->GetChildren(), &artist->m_Releases );
                        }
                        XmlNode = XmlNode->GetNext();
                    }
                    return true;
                }
                else
                {
                    guLogError( wxT( "Could not get the artist xml data" ) );
                }
            }
            else
            {
                XmlNode = XmlNode->GetChildren();
                if( XmlNode->GetName() == wxT( "error" ) )
                {
                    wxString ErrorMsg;
                    XmlNode->GetPropVal( wxT( "msg" ), &ErrorMsg );
                    guLogError( wxT( "guDiscogs: %s" ), ErrorMsg.c_str() );
                }
                else
                {
                    guLogError( wxT( "guDiscogs error getting artist" ) );
                }
            }
        }
        else
        {
            guLogError( wxT( "Got no valid response from the Discogs server" ) );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guDiscogs::GetReleaseImages( const int id, guDiscogsRelease * release )
{
    return false;
}

// -------------------------------------------------------------------------------- //
bool guDiscogs::GetRelease( const int id, guDiscogsRelease * release )
{
    wxString SearchUrl;
    SearchUrl = wxString::Format( wxT( guDISCOGS_QUERY_RELEASE ), id );

    wxString Content = GetUrlContent( SearchUrl, wxEmptyString, true );

    //guLogMessage( wxT( "GetRelease:\n%s" ), Content.c_str() );

    if( !Content.IsEmpty() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && ( XmlNode->GetName() == wxT( "resp" ) ) )
        {
            wxString Status;
            XmlNode->GetPropVal( wxT( "stat" ), &Status );
            if( Status == wxT( "ok" ) )
            {
                XmlNode = XmlNode->GetChildren();
                if( XmlNode->GetName() == wxT( "release" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    while( XmlNode )
                    {
                        if( XmlNode->GetName() == wxT( "images" ) )
                        {
                            GetImages( XmlNode->GetChildren(), &release->m_Images );
                        }
                        else if( XmlNode->GetName() == wxT( "tracklist" ) )
                        {
                            GetTracks( XmlNode->GetChildren(), &release->m_Tracks );
                        }
                        XmlNode = XmlNode->GetNext();
                    }
                    return true;
                }
            }
            else
            {
                XmlNode = XmlNode->GetChildren();
                if( XmlNode->GetName() == wxT( "error" ) )
                {
                    wxString ErrorMsg;
                    XmlNode->GetPropVal( wxT( "msg" ), &ErrorMsg );
                    guLogError( wxT( "guDiscogs: %s" ), ErrorMsg.c_str() );
                }
                else
                {
                    guLogError( wxT( "guDiscogs error getting artist" ) );
                }
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guDiscogsCoverFetcher::guDiscogsCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                    const wxChar * artist, const wxChar * album ) :
    guCoverFetcher( mainthread, coverlinks, artist, album )
{
}

// -------------------------------------------------------------------------------- //
guDiscogsCoverFetcher::~guDiscogsCoverFetcher()
{
}

// -------------------------------------------------------------------------------- //
bool GetNameMatch( const wxString &title, const wxString &search )
{
    wxArrayString SearchWords = wxStringTokenize( search );
    int index;
    int count = SearchWords.Count();
    for( index = 0; index < count; index++ )
    {
        if( title.Find( SearchWords[ index ].Lower() ) == wxNOT_FOUND )
            return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
int guDiscogsCoverFetcher::AddCoverLinks( int pagenum )
{
    guDiscogs Discogs;
    guDiscogsArtist Artist;
    Discogs.GetArtist( m_Artist, &Artist );
    int Index;
    int Count = wxMin( (int) Artist.m_Releases.Count(), ( pagenum + 1 ) * guDISCOGS_REQUEST_ITEMS );
    int CheckCount = 0;

    //guLogMessage( wxT( "Reading %s %i Releases..." ), m_Artist.c_str(), Count );

    for( Index = ( pagenum * guDISCOGS_REQUEST_ITEMS ); Index < Count; Index++ )
    {
//        guLogMessage( wxT( "Release '%s' type '%s' format '%s'" ),
//            Artist.m_Releases[ Index ].m_Title.c_str(),
//            Artist.m_Releases[ Index ].m_Type.c_str(),
//            Artist.m_Releases[ Index ].m_Format.c_str() );

        if( Artist.m_Releases[ Index ].m_Type == wxT( "Main" ) &&
            GetNameMatch( Artist.m_Releases[ Index ].m_Title.Lower(), m_Album ) )
        {
            Discogs.GetRelease( Artist.m_Releases[ Index ].m_Id, &Artist.m_Releases[ Index ] );

            int ImgIndex;
            int ImgCount = Artist.m_Releases[ Index ].m_Images.Count();
            for( ImgIndex = 0; ImgIndex < ImgCount; ImgIndex++ )
            {
                wxArrayString ImgArray;

                ImgArray.Add( Artist.m_Releases[ Index ].m_Images[ ImgIndex ].m_Url );
                ImgArray.Add( wxString::Format( wxT( "%ix%i" ),
                    Artist.m_Releases[ Index ].m_Images[ ImgIndex ].m_Width,
                    Artist.m_Releases[ Index ].m_Images[ ImgIndex ].m_Height ) );

                m_CoverLinks->Add( ImgArray );

//                guLogMessage( wxT( "Added Image %s" ),
//                    Artist.m_Releases[ Index ].m_Images[ ImgIndex ].m_Url.c_str() );
                break;
            }
        }
        CheckCount++;
    }
    return CheckCount;
}

// -------------------------------------------------------------------------------- //
