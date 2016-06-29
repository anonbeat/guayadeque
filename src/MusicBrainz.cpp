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
#define guMB_URL_RECORDINGS_BY_RELEASE_ID   guMB_URL_BASE "release/%s?inc=recordings%%20artists%%20artist-credits"
#define guMB_URL_RECORDING_BY_ID            guMB_URL_BASE "recording/%s?inc=artists%%20artist-credits"

#define guMB_URL_RELEASES_BY_DISCID         guMB_URL_BASE "discid/%s"

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
static void ReadXmlReleaseArtist( wxXmlNode * XmlNode, guMBRecording * mbrecording )
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
            XmlNode->GetAttribute( wxT( "id" ), &mbrecording->m_ArtistId );
            XmlNode = XmlNode->GetChildren();
            continue;
        }
        else if( XmlNode->GetName() == wxT( "name" ) )
        {
            mbrecording->m_ArtistName = XmlNode->GetNodeContent();
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
void ReadXmlReleaseRecording( wxXmlNode * xmlnode, guMBRecording * mbrecording )
{
    while( xmlnode )
    {
        if( xmlnode->GetName() == wxT( "title" ) )
        {
            mbrecording->m_Title = xmlnode->GetNodeContent();
        }
        else if( xmlnode->GetName() == wxT( "length" ) )
        {
            long Len = 0;
            if( xmlnode->GetNodeContent().ToLong( &Len ) )
            {
                mbrecording->m_Length = Len;
            }
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlReleaseTrackList( wxXmlNode * xmlnode, guMBRelease * mbrelease )
{
    while( xmlnode )
    {
        if( xmlnode->GetName() == "track" )
        {
            guMBRecording * MBRecording = new guMBRecording();

            ReadXmlReleaseRecording( xmlnode->GetChildren(), MBRecording );

            mbrelease->m_Recordings.Add( MBRecording );
        }

        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlCdStub( wxXmlNode * xmlnode, guMBRelease * mbrelease )
{
    while( xmlnode )
    {
        if( xmlnode->GetName() == wxT( "title" ) )
        {
            mbrelease->m_Title = xmlnode->GetNodeContent();
        }
        else if( xmlnode->GetName() == wxT( "artist" ) )
        {
            mbrelease->m_ArtistName = xmlnode->GetNodeContent();
        }
        else if( xmlnode->GetName() == wxT( "track-list" ) )
        {
            ReadXmlReleaseTrackList( xmlnode->GetChildren(), mbrelease );
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
int guMusicBrainz::GetRecordReleases( const wxString &artist, const wxString &title, guMBReleaseArray * mbreleases )
{
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
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "error" ) )
            {
                m_ErrorMsg = XmlNode->GetNodeContent();
                return -1;
            }
            else if( XmlNode->GetName() == wxT( "metadata" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode && XmlNode->GetName() == wxT( "release-list" ) )
            {
                ReadXmlReleases( XmlNode->GetChildren(), mbreleases );
            }

            XmlNode = XmlNode->GetNext();
        }
    }

    return mbreleases->Count();
}

// -------------------------------------------------------------------------------- //
int guMusicBrainz::GetRecordReleases( const wxString &recordid, guMBReleaseArray * mbreleases )
{
    if( recordid.IsEmpty() )
    {
        guLogError( wxT( "The MusicBrainz recordid is empty" ) );
        return wxNOT_FOUND;
    }

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
                m_ErrorMsg = XmlNode->GetNodeContent();
                return -1;
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
int guMusicBrainz::GetDiscIdReleases( const wxString &discid, guMBReleaseArray * mbreleases )
{
    if( discid.IsEmpty() )
    {
        guLogError( wxT( "The MusicBrainz discid is empty" ) );
        return wxNOT_FOUND;
    }

    wxString QueryUrl = wxString::Format( wxT( guMB_URL_RELEASES_BY_DISCID ),
                                          discid.c_str() );
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
                m_ErrorMsg = XmlNode->GetNodeContent();
                return -1;
            }
            else if( XmlNode->GetName() == wxT( "metadata" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "disc" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "cdstub" ) )
            {
                guMBRelease * MBRelease = new guMBRelease();
                XmlNode->GetAttribute( wxT( "id" ), &MBRelease->m_Id );
                ReadXmlCdStub( XmlNode->GetChildren(), MBRelease );
                mbreleases->Add( MBRelease );
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
        else if( XmlNode->GetName() == wxT( "title" ) && mbrecording->m_Title.IsEmpty() )
        {
            mbrecording->m_Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "length" ) && !mbrecording->m_Length )
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
        else if( XmlNode->GetName() == wxT( "artist-credit" ) )
        {
            ReadXmlReleaseArtist( XmlNode->GetChildren(), mbrecording );
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
            else if( XmlNode->GetName() == wxT( "medium" ) )
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
