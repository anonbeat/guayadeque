// -------------------------------------------------------------------------------- //
//  Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "MusicBrainz.h"

//#include "Http.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/sstream.h>

namespace Guayadeque {

WX_DEFINE_OBJARRAY( guMBRecordingArray );
WX_DEFINE_OBJARRAY( guMBReleaseArray );


// -------------------------------------------------------------------------------- //
#define guMB_URL_BASE                       "http://musicbrainz.org/ws/2/"
#define guMB_URL_RELEASES_BY_TEXT           guMB_URL_BASE "release/?query=artist:%s,title:%s"
#define guMB_URL_RELEASES_BY_RECORDING_ID   guMB_URL_BASE "recording/%s?inc=releases"
#define guMB_URL_RECORDINGS_BY_RELEASE_ID   guMB_URL_BASE "release/%s?inc=recordings+artists"
#define guMB_URL_RECORDING_BY_ID            guMB_URL_BASE "recording/%s?inc=artists"


// -------------------------------------------------------------------------------- //
guMusicBrainz::guMusicBrainz()
{
}

// -------------------------------------------------------------------------------- //
guMusicBrainz::~guMusicBrainz()
{
}

// -------------------------------------------------------------------------------- //
static void ReadXmlReleaseArtist( wxXmlNode * XmlNode, guMBRelease * mbrelease )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "name-credit" ) )
        {
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "artist" ) )
        {
            XmlNode->GetAttribute( wxT( "id" ), &mbrelease->m_ArtistId );
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "name" ) )
        {
            mbrelease->m_ArtistName = XmlNode->GetNodeContent();
            return;
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
static void ReadXmlRelease( wxXmlNode * XmlNode, guMBRelease * mbrelease )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            mbrelease->m_Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "artist-credit" ) )
        {
            ReadXmlReleaseArtist( XmlNode->GetChildren(), mbrelease );
        }
        else if( XmlNode->GetName() == wxT( "date" ) )
        {
            long Year = 0;
            if( XmlNode->GetNodeContent().Left( 4 ).ToLong( &Year ) )
            {
                mbrelease->m_Year = Year;
            }
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
static void ReadXmlReleases( wxXmlNode * XmlNode, guMBReleaseArray * mbreleases )
{
    while( XmlNode && XmlNode->GetName() == wxT( "release" ) )
    {
        wxString ReleaseId;
        XmlNode->GetAttribute( wxT( "id" ), &ReleaseId );
        if( FindMBReleaseId( mbreleases, ReleaseId ) == wxNOT_FOUND )
        {
            guMBRelease * MBRelease = new guMBRelease();
            MBRelease->m_Id = ReleaseId;

            ReadXmlRelease( XmlNode->GetChildren(), MBRelease );

            mbreleases->Add( MBRelease );
        }

        XmlNode = XmlNode->GetNext();
    }
}


// -------------------------------------------------------------------------------- //
int guMusicBrainz::GetReleases( const wxString &artist, const wxString &title, guMBReleaseArray * mbreleases )
{
    /*
    <metadata xmlns="http://musicbrainz.org/ns/mmd-2.0#" ...
      <release-list count="28721" offset="0">
        <release id="3eae64c9-dd5f-4bbd-ae76-e07406deeee8" ext:score="100">
          <title>Money for Nothing</title>
          <status>Official</status>
          <packaging>Jewel Case</packaging>
          <text-representation>...</text-representation>
          <artist-credit>...</artist-credit>
          <release-group id="8af3b92c-34b7-3182-b132-1b952637f7a6" type="Compilation">
            <primary-type>Album</primary-type>
            <secondary-type-list>...</secondary-type-list>
          </release-group>
          <date>1988</date>
          <country>CA</country>
          <release-event-list>
            <release-event>
              <date>1988</date>
              <area id="71bbafaa-e825-3e15-8ca9-017dcad1748b">
              <name>Canada</name>
              <sort-name>Canada</sort-name>
              <iso-3166-1-code-list>...</iso-3166-1-code-list>
              </area>
            </release-event>
          </release-event-list>
          <barcode>04228364192197</barcode>
          <label-info-list>...</label-info-list>
          <medium-list count="1">...</medium-list>
        </release>
      <release id="f45f7422-5f8f-4a9b-b333-430d6c01ef60" ext:score="100">...</release>
    */
    wxString QueryUrl = wxString::Format( wxT( guMB_URL_RELEASES_BY_TEXT ),
                                          guURLEncode( artist ).c_str(),
                                          guURLEncode( title ).c_str() );

    //guLogMessage( wxT( "GetRelease: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
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

    return mbreleases->Count();
}

// -------------------------------------------------------------------------------- //
int guMusicBrainz::GetReleases( const wxString &recordid, guMBReleaseArray * mbreleases )
{
    if( recordid.IsEmpty() )
    {
        guLogError( wxT( "The MusicBrainz recordid is empty" ) );
        return wxNOT_FOUND;
    }

    /*
    <metadata xmlns="http://musicbrainz.org/ns/mmd-2.0#">
      <recording id="682c31da-3749-4a7c-a8cb-e001f9e4a4f5">
        <title>Have You Heard</title>
        <length>385200</length>
        <release-list count="6">
          <release id="0340d76f-d9b4-38b4-b6e7-719d17ea5abf">
            <title>Letter From Home</title>
            <status id="4e304316-386d-3409-af2e-78857eec5cfe">Official</status>
            <quality>normal</quality>
            <text-representation>...</text-representation>
            <date>2006-02-07</date>
            <country>US</country>
            <release-event-list count="1">...</release-event-list>
            <barcode>075992424523</barcode>
          </release>
        <release id="19721ca4-81da-4a29-bae6-97dc46334d4b">...</release>
        <release id="3a48d210-22ed-4728-af46-1935f9adf718">...</release>
        <release id="758457d4-aec8-448e-baa2-511b10e076bd">...</release>
        <release id="d1e787cc-3411-4319-a127-5178e83f1468">...</release>
        <release id="f8aa4601-bd7c-490a-a297-08464a4a3db5">...</release>
        </release-list>
      </recording>
    </metadata>
    */

    wxString QueryUrl = wxString::Format( wxT( guMB_URL_RELEASES_BY_RECORDING_ID ),
                                          recordid.c_str() );
    //guLogMessage( wxT( "MusicBrainz: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    if( Content.Length() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "error" ) )
            {
                break;
            }
            else if( XmlNode->GetName() == wxT( "metadata" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "recording" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "release-list" ) )
            {
                ReadXmlReleases( XmlNode->GetChildren(), mbreleases );
            }

            XmlNode = XmlNode->GetNext();
        }
    }

    return mbreleases->Count();
}

// -------------------------------------------------------------------------------- //
static void ReadXmlRecording( wxXmlNode * XmlNode, guMBRecording * mbrecording  )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "position" ) )
        {
            long Pos = 0;
            if( XmlNode->GetNodeContent().ToLong( &Pos ) )
            {
                mbrecording->m_Number = Pos;
            }
        }
        else if( XmlNode->GetName() == wxT( "length" ) )
        {
            long Length = 0;
            if( XmlNode->GetNodeContent().ToLong( &Length ) )
            {
                mbrecording->m_Length = Length;
            }
        }
        else if( XmlNode->GetName() == wxT( "recording" ) )
        {
            XmlNode->GetAttribute( wxT( "id" ), &mbrecording->m_Id );
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "title" ) )
        {
            mbrecording->m_Title = XmlNode->GetNodeContent();
            break;
        }

        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
static void ReadXmlRecordings( wxXmlNode * XmlNode, guMBRecordingArray * mbrecordings  )
{
    while( XmlNode && XmlNode->GetName() == wxT( "track" ) )
    {
        guMBRecording * mbrecording = new guMBRecording();

        ReadXmlRecording( XmlNode->GetChildren(), mbrecording );

        mbrecordings->Add( mbrecording );

        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
int guMusicBrainz::GetRecordings( const wxString &releaseid, guMBRecordingArray * mbrecordings )
{
    wxString QueryUrl = wxString::Format( wxT( guMB_URL_RECORDINGS_BY_RELEASE_ID ),
                                          releaseid.c_str() );
    //guLogMessage( wxT( "MusicBrainz: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    if( Content.Length() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "metadata" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "release" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "medium-list" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "track-list" ) )
            {
                ReadXmlRecordings( XmlNode->GetChildren(), mbrecordings );
            }

            XmlNode = XmlNode->GetNext();
        }
    }

    return mbrecordings->Count();
}

// -------------------------------------------------------------------------------- //
static void ReadXmlNodeArtist( wxXmlNode * XmlNode, guMBRelease &mbrelase )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "name-credit" ) )
        {
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "artist" ) )
        {
            XmlNode->GetAttribute( wxT( "Id" ), mbrelase.m_ArtistId );
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "name" ) )
        {
            mbrelase.m_ArtistName = XmlNode->GetNodeContent();
            break;
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
int guMusicBrainz::GetRecordings( guMBRelease &mbrelease )
{
    /*
    <metadata xmlns="http://musicbrainz.org/ns/mmd-2.0#">
      <release id="0340d76f-d9b4-38b4-b6e7-719d17ea5abf">
        <title>Letter From Home</title>
        <status id="4e304316-386d-3409-af2e-78857eec5cfe">Official</status>
        <quality>normal</quality>
        <text-representation>...</text-representation>
        <artist-credit>
          <name-credit>
          <artist id="66a7f1f8-0ad6-4e3a-9346-a643e2739a8c">
            <name>Pat Metheny Group</name>
            <sort-name>Metheny, Pat, Group</sort-name>
          </artist>
          </name-credit>
        </artist-credit>
        <date>2006-02-07</date>
        <country>US</country>
        <release-event-list count="1">...</release-event-list>
        <barcode>075992424523</barcode>
        <asin>B000CZ0Q6G</asin>
        <cover-art-archive>...</cover-art-archive>
        <medium-list count="1">
          <medium>
            <position>1</position>
            <format id="9712d52a-4509-3d4b-a1a2-67c88c643e31">CD</format>
            <track-list count="12" offset="0">
              <track id="7d29547a-b20b-3e62-b006-004caab2728c">
                <position>1</position>
                <number>1</number>
                <length>385200</length>
                <recording id="682c31da-3749-4a7c-a8cb-e001f9e4a4f5">
                  <title>Have You Heard</title>
                  <length>385200</length>
                </recording>
              </track>
              <track id="72e7a8bf-1967-37ef-bf33-d036c205e254">...</track>
              ...
            </track-list>
          </medium>
        </medium-list>
      </release>
    </metadata>
    */

    wxString QueryUrl = wxString::Format( wxT( guMB_URL_RECORDINGS_BY_RELEASE_ID ),
                                          mbrelease.m_Id.c_str() );
    //guLogMessage( wxT( "MusicBrainz: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
    if( Content.Length() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "metadata" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "release" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "title" ) )
            {
                mbrelease.m_Title = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "artist-credit" ) )
            {
                ReadXmlNodeArtist( XmlNode->GetChildren(), mbrelease );
            }
            else if( XmlNode->GetName() == wxT( "date" ) )
            {
                long Year = 0;
                if( XmlNode->GetNodeContent().Left( 4 ).ToLong( &Year ) )
                {
                    mbrelease.m_Year = Year;
                }
            }
            else if( XmlNode->GetName() == wxT( "medium-list" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "medium" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "track-list" ) )
            {
                ReadXmlRecordings( XmlNode->GetChildren(), &mbrelease.m_Recordings );
            }

            XmlNode = XmlNode->GetNext();
        }
    }

    return mbrelease.m_Recordings.Count();
}


// -------------------------------------------------------------------------------- //
static void ReadXmlNodeArtist( wxXmlNode * XmlNode, guMBRecording * mbrecording )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "name-credit" ) )
        {
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "artist" ) )
        {
            XmlNode->GetAttribute( wxT( "Id" ), mbrecording->m_ArtistId );
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "name" ) )
        {
            mbrecording->m_ArtistName = XmlNode->GetNodeContent();
            break;
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guMusicBrainz::GetRecordingInfo( const wxString &recordingid, guMBRecording * mbrecording )
{
    /*
    <metadata xmlns="http://musicbrainz.org/ns/mmd-2.0#">
        <recording id="565cd4c1-c55d-4d7c-a511-b6afeabf0e64">
            <title>Our Love Is Fading</title>
            <length>381160</length>
            <artist-credit>
                <name-credit>
                    <artist id="80ccfede-c258-4575-a7ad-c982e9932e0f">
                        <name>Sheryl Crow</name>
                        <sort-name>Crow, Sheryl</sort-name>
                    </artist>
                </name-credit>
            </artist-credit>
        </recording>
    </metadata>
    */

    wxString QueryUrl = wxString::Format( wxT( guMB_URL_RECORDING_BY_ID ),
                                          recordingid.c_str() );
    //guLogMessage( wxT( "MusicBrainz: %s" ), QueryUrl.c_str() );
    wxString Content = GetUrlContent( QueryUrl );
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    if( Content.Length() )
    {
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "metadata" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "recording" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "title" ) )
            {
                mbrecording->m_Title = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "length" ) )
            {
                long Length = 0;
                if( XmlNode->GetNodeContent().ToLong( &Length ) )
                {
                    mbrecording->m_Length = Length;
                }
            }
            else if( XmlNode->GetName() == wxT( "artist-credit" ) )
            {
                ReadXmlNodeArtist( XmlNode->GetChildren(), mbrecording );
            }

            XmlNode = XmlNode->GetNext();
        }
    }
}



// -------------------------------------------------------------------------------- //
// Others
// -------------------------------------------------------------------------------- //
int FindMBReleaseId( guMBReleaseArray * mbreleases, const wxString &releaseid )
{
    int Count = mbreleases->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( mbreleases->Item( Index ).m_Id == releaseid )
            return Index;
    }
    return wxNOT_FOUND;
}

}

// -------------------------------------------------------------------------------- //
