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
#include "MusicBrainz.h"

#include "Utils.h"
#include "curl/http.h"

#include <wx/arrimpl.cpp>
#include <wx/sstream.h>

#define guMUSICBRAINZ_URL_BASE                  "http://musicbrainz.org/ws/1/"
#define guMUSICBRAINZ_URL_ARTIST                guMUSICBRAINZ_URL_BASE "artist/?type=xml"
#define guMUSICBRAINZ_URL_ARTIST_ID             guMUSICBRAINZ_URL_BASE "artist/%s?type=xml"
#define guMUSICBRAINZ_URL_RELEASE_GROUP         guMUSICBRAINZ_URL_BASE "release-group/?type=xml"
#define guMUSICBRAINZ_URL_RELEASE_GROUP_ID      guMUSICBRAINZ_URL_BASE "release-group/%s?type=xml"
#define guMUSICBRAINZ_URL_RELEASE               guMUSICBRAINZ_URL_BASE "release/?type=xml"
#define guMUSICBRAINZ_URL_RELEASE_ID            guMUSICBRAINZ_URL_BASE "release/%s?type=xml"
#define guMUSICBRAINZ_URL_TRACK                 guMUSICBRAINZ_URL_BASE "track/?type=xml"
#define guMUSICBRAINZ_URL_TRACK_ID              guMUSICBRAINZ_URL_BASE "track/%s?type=xml"
#define guMUSICBRAINZ_URL_LABEL                 guMUSICBRAINZ_URL_BASE "label/?type=xml"
#define guMUSICBRAINZ_URL_LABEL_ID              guMUSICBRAINZ_URL_BASE "label/%s?type=xml"
#define guMUSICBRAINZ_URL_TAG                   guMUSICBRAINZ_URL_BASE "tag/?type=xml"
#define guMUSICBRAINZ_URL_RATING                guMUSICBRAINZ_URL_BASE "rating/?type=xml"

#define guMUSICBRAINZ_URL_RELEASE_BY_TEXT       guMUSICBRAINZ_URL_BASE "release/?type=xml&artist=%s&title=%s"

WX_DEFINE_OBJARRAY( guMBTrackArray );
WX_DEFINE_OBJARRAY( guMBReleaseArray );

// -------------------------------------------------------------------------------- //
guMusicBrainz::guMusicBrainz()
{
}

// -------------------------------------------------------------------------------- //
guMusicBrainz::~guMusicBrainz()
{
}

// -------------------------------------------------------------------------------- //
wxString ReadXmlArtistName( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "name" ) )
        {
            return XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxString ReadXmlReleaseName( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            return XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void ReadXmlTrackRelease( wxXmlNode * XmlNode, guMBTrack * track )
{
    if( XmlNode && XmlNode->GetName() == wxT( "release" ) )
    {
        XmlNode->GetPropVal( wxT( "id" ), &track->m_ReleaseId );
        track->m_ReleaseName = ReadXmlReleaseName( XmlNode->GetChildren() );
    }
}

// -------------------------------------------------------------------------------- //
int GetXmlTrackLength( wxXmlNode * XmlNode )
{
    wxASSERT( XmlNode );
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "duration" ) )
        {
            long RetVal;
            XmlNode->GetNodeContent().ToLong( &RetVal );
            return ( int ) RetVal;
        }
        XmlNode = XmlNode->GetNext();
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void ReadXmlTrack( wxXmlNode * XmlNode, guMBTrack * track )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            track->m_Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "duration" ) )
        {
            long LongValue;
            XmlNode->GetNodeContent().ToLong( &LongValue );
            track->m_Length = LongValue;
        }
        else if( XmlNode->GetName() == wxT( "artist" ) )
        {
            XmlNode->GetPropVal( wxT( "id" ), &track->m_ArtistId );
            track->m_ArtistName = ReadXmlArtistName( XmlNode->GetChildren() );
        }
        else if( XmlNode->GetName() == wxT( "release-list" ) )
        {
            ReadXmlTrackRelease( XmlNode->GetChildren(), track );
        }
        XmlNode = XmlNode->GetNext();
    }
}


// -------------------------------------------------------------------------------- //
void ReadXmlTracks( wxXmlNode * XmlNode, guMBTrackArray * tracks, int filterlength = wxNOT_FOUND );
void ReadXmlTracks( wxXmlNode * XmlNode, guMBTrackArray * tracks, int filterlength  )
{
    wxASSERT( XmlNode );
    wxASSERT( tracks );

    int TrackNum = 1;
    while( XmlNode && XmlNode->GetName() == wxT( "track" ) )
    {
        // We filter the tracks by the lengths so if the difference of the
        // tracks is bigger than 3 secs we discard them
//        if( filterlength == wxNOT_FOUND ||
//            GetTrackLengthDiff( filterlength, GetXmlTrackLength( XmlNode->GetChildren() ) ) > guMDNS_MAX_TIME_DIFF )
        {
            guMBTrack * Track = new guMBTrack();
            wxASSERT( Track );
            XmlNode->GetPropVal( wxT( "id" ), &Track->m_Id );
            Track->m_Number = TrackNum;
            ReadXmlTrack( XmlNode->GetChildren(), Track );
            tracks->Add( Track );
            TrackNum++;
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guMusicBrainz::GetTracks( guMBTrackArray * mbtracks, const guTrack * track, int tracklength )
{
    wxASSERT( mbtracks );
    wxASSERT( track );

    guMusicDns * MusicDns = new guMusicDns( this );
    if( MusicDns )
    {
        MusicDns->SetTrack( track );

        m_ReceivedPUId = false;

        //guLogMessage( wxT( "IsOk: %u   ReceivedPUID: %u" ), MusicDns->IsOk(), m_ReceivedPUId );

        while( MusicDns->IsOk() )
        {
            if( m_ReceivedPUId )
                break;
            wxMilliSleep( 100 );
        }

        if( m_ReceivedPUId )
        {
            //guLogMessage( wxT( "IsOk: %u   ReceivedPUID: %u" ), MusicDns->IsOk(), m_ReceivedPUId );
            GetTracks( mbtracks, MusicDns->GetPUID(), tracklength );
        }
        else
        {
            guLogError( wxT( "Error in MusicDns %u" ), MusicDns->GetStatus() );
        }
    }
    else
    {
        guLogError( wxT( "Could not create the musicdns object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMusicBrainz::GetTracks( guMBTrackArray * mbtracks, const wxString &trackpuid, int tracklength )
{
    if( trackpuid.IsEmpty() )
    {
        guLogError( wxT( "The MusicBrainz puid is empty" ) );
        return;
    }


    wxString QueryUrl = wxString::Format( wxT( guMUSICBRAINZ_URL_TRACK
                "&puid=%s&releasetype=official" ), trackpuid.c_str() );

    //guLogMessage( wxT( "MusicBrainz: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    if( Content.Length() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && ( XmlNode->GetName() == wxT( "metadata" ) ) )
        {
            XmlNode = XmlNode->GetChildren();
            if( XmlNode && XmlNode->GetName() == wxT( "track-list" ) )
            {
                ReadXmlTracks( XmlNode->GetChildren(), mbtracks, tracklength );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMusicBrainz::FoundPUID( const wxString &puid )
{
    m_ReceivedPUId = true;
}

// -------------------------------------------------------------------------------- //
void ReadXmlReleaseYear( wxXmlNode * XmlNode, guMBRelease * mbrelease )
{
    while( XmlNode && XmlNode->GetName() == wxT( "event" ) )
    {
        wxString Event;
        wxString Value;
        XmlNode->GetPropVal( wxT( "date" ), &Value );
        if( !Value.IsEmpty() )
        {
            Event += Value;
            XmlNode->GetPropVal( wxT( "country" ), &Value );
            Event += wxT( " : " ) + Value;
            XmlNode->GetPropVal( wxT( "format" ), &Value );
            Event += wxT( " : " ) + Value;
            mbrelease->m_Events.Add( Event );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlRelease( wxXmlNode * XmlNode, guMBRelease * mbrelease )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            mbrelease->m_Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "artist" ) )
        {
            XmlNode->GetPropVal( wxT( "id" ), &mbrelease->m_ArtistId );
            mbrelease->m_ArtistName = ReadXmlArtistName( XmlNode->GetChildren() );
        }
        else if( XmlNode->GetName() == wxT( "release-event-list" ) )
        {
            ReadXmlReleaseYear( XmlNode->GetChildren(), mbrelease );
        }
        else if( XmlNode->GetName() == wxT( "track-list" ) )
        {
            ReadXmlTracks( XmlNode->GetChildren(), &mbrelease->m_Tracks );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlReleases( wxXmlNode * XmlNode, guMBReleaseArray * mbreleases )
{
    while( XmlNode && XmlNode->GetName() == wxT( "release" ) )
    {
        guMBRelease * MBRelease = new guMBRelease();
        XmlNode->GetPropVal( wxT( "id" ), &MBRelease->m_Id );
        //guLogMessage( wxT( "Release found %s" ), MBRelease->m_Id.c_str() );
        ReadXmlRelease( XmlNode->GetChildren(), MBRelease );
        mbreleases->Add( MBRelease );
        //delete MBRelease;

        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guMusicBrainz::GetReleases( guMBReleaseArray * mbreleases, const wxString &artist, const wxString &title )
{
    wxString QueryUrl = wxString::Format( wxT( guMUSICBRAINZ_URL_RELEASE_BY_TEXT
                "&releasetypes=Official" ),
                guURLEncode( artist ).c_str(), guURLEncode( title ).c_str() );

    //guLogMessage( wxT( "GetRelease: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    if( Content.Length() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && ( XmlNode->GetName() == wxT( "metadata" ) ) )
        {
            XmlNode = XmlNode->GetChildren();
            if( XmlNode && XmlNode->GetName() == wxT( "release-list" ) )
            {
                ReadXmlReleases( XmlNode->GetChildren(), mbreleases );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMusicBrainz::GetRelease( guMBRelease * mbrelease, const wxString &releaseid )
{
    wxASSERT( mbrelease );
    wxString QueryUrl = wxString::Format( wxT( guMUSICBRAINZ_URL_RELEASE_ID
                "&inc=tracks+artist+release-events" ), releaseid.c_str() );

    //guLogMessage( wxT( "GetRelease: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    if( Content.Length() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && ( XmlNode->GetName() == wxT( "metadata" ) ) )
        {
            XmlNode = XmlNode->GetChildren();
            if( XmlNode && XmlNode->GetName() == wxT( "release" ) )
            {
                XmlNode->GetPropVal( wxT( "id" ), &mbrelease->m_Id );
                ReadXmlRelease( XmlNode->GetChildren(), mbrelease );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
// Others
// -------------------------------------------------------------------------------- //
int FindMBTracksReleaseId( guMBTrackArray * mbtracks, const wxString &releaseid )
{
    int Index;
    int Count = mbtracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( mbtracks->Item( Index ).m_ReleaseId == releaseid )
            return Index;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
