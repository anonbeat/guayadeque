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
//
//    Here is the complete API for newxml.phtml, for non-commerical purposes.
//
//    Top Level Genre Listing
//    http://yp.shoutcast.com/sbin/newxml.phtml
//
//    returns list of genres
//    <genrelist>
//    <genre name="Rock"></genre>
//    </genrelist>
//
//    Station List on Genre
//    http://yp.shoutcast.com/sbin/newxml.phtml?genre=Trance
//
//    <stationlist>
//    <tunein base="/sbin/tunein-station.pls"></tunein>
//    <station name=" -=Sputnik Radio Muzika=- Hi Def Sound..WARNING: Subliminal Suggestions in audio.... Server FULL? Do" mt="audio/aacp" id="639026" br="128" genre="Trance Dance Pop" ct="Gabriel & Dresden - Sydney (Organised nature)" lc="45"></station>
//    </stationlist>
//
//    name = station name
//    mt = MIME type, determines the codec used. audio/mpeg for MP3 and audio/aacp for aacPlus
//    id = station ID. use this when connecting to http://yp.shoutcast.com/tunein-station.pls?id=12345 to retrieve the playlist containing the station URL
//    br = station bitrate
//    genre = station genre. This is a "freeform" text. These are used to populate the genre list. Calling the API with ?genre=xyz essential does a like genre like %xyz% database query.
//    ct = currently playing track. not gauranteed to be up-to-date because of database caching
//    lc = listener count
//
//
//    Station List on Search
//    http://yp.shoutcast.com/sbin/newxml.phtml?search=[UrlEncodedSearchString]
//
//    same results as the genre listings, but search searches station name, current playing track and genre
//
//    To limit results
//    add &limit=%d to limit xml entry results where %d is the number of items to return
//
//    Top 500:
//    Request genre=Top500
//
//
//
//    20 Random Stations
//
//    Request genre=random
//
//
//    RSS 2.0 version
//
//    append &rss=1 to the url
//
//    *** ALL STATION LISTS EXCEPT RANDOM, ARE SORTED BY LISTENER COUNT ***
//
//
//    To tune into a station, find the id="" attribute in the xml and call
//    http://yp.shoutcast.com/sbin/tunein-station.pls?id=1025
//
#include "Shoutcast.h"

#include "Utils.h"
#include "PlayListFile.h"

#include <wx/curl/http.h>
#include <wx/xml/xml.h>
#include <wx/fileconf.h>


#define SHOUTCAST_GET_GENRE_URL         wxT( "http://yp.shoutcast.com/sbin/newxml.phtml" )
#define SHOUTCAST_GET_STATIONS_URL      wxT( "http://yp.shoutcast.com/sbin/newxml.phtml?genre=%s" )
#define SHOUTCAST_GET_STATION_PLAYLIST  wxT( "http://yp.shoutcast.com/sbin/tunein-station.pls?id=%u" )


// -------------------------------------------------------------------------------- //
wxArrayString guShoutCast::GetGenres( void ) const
{
    wxArrayString   RetVal;
    wxString        Content;
    wxString        GenreName;
    wxCurlHTTP      http;
    char *          Buffer = NULL;    //
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, SHOUTCAST_GET_GENRE_URL );
    if( Buffer )
    {
        Content = wxString( Buffer, wxConvUTF8 );
        free( Buffer );
        //
        if( Content.Length() )
        {
            wxStringInputStream InStr( Content );
            wxXmlDocument XmlDoc( InStr );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName() == wxT( "genrelist" ) )
            {
                XmlNode = XmlNode->GetChildren();
                while( XmlNode )
                {
                    if( XmlNode->GetName() == wxT( "genre" ) )
                    {
                        XmlNode->GetPropVal( wxT( "name" ), &GenreName );
                        RetVal.Add( GenreName );
                        //guLogMessage( wxT( "Genre : %s" ), GenreName.c_str() );
                    }
                    XmlNode = XmlNode->GetNext();
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guShoutCast::GetStations( const wxString &GenreName, const int GenreId, guRadioStations * Stations, long MinBitRate ) const
{
    wxString    Content;
    wxString    Value;
    long        BitRate;
    char *      Buffer = NULL;
    wxCurlHTTP  http;

    //guLogMessage( wxT( "About to get stations for genre '%s'" ), GenreName.c_str() );
    //
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, wxString::Format( SHOUTCAST_GET_STATIONS_URL,
                                                guURLEncode( GenreName ).c_str() ) );
    if( Buffer )
    {
        Content = wxString( Buffer, wxConvUTF8 );
        free( Buffer );
        //
        if( Content.Length() )
        {
            wxStringInputStream InStr( Content );
            wxXmlDocument XmlDoc( InStr );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName() == wxT( "stationlist" ) )
            {
                XmlNode = XmlNode->GetChildren();
                while( XmlNode )
                {
                    if( XmlNode->GetName() == wxT( "station" ) )
                    {
                        XmlNode->GetPropVal( wxT( "br" ), &Value );
                        Value.ToLong( &BitRate );
                        if( ( MinBitRate == SHOUTCAST_STATION_ALLBITRATES ) ||
                            ( MinBitRate <= BitRate ) )
                        {
                            guRadioStation * RadioStation = new guRadioStation();
                            if( RadioStation )
                            {
                                RadioStation->m_Id = wxNOT_FOUND;
                                XmlNode->GetPropVal( wxT( "name" ), &RadioStation->m_Name );
                                RadioStation->m_Name.Replace( wxT( " - [SHOUTcast.com]" ), wxT( "" ) );
                                XmlNode->GetPropVal( wxT( "mt" ), &RadioStation->m_Type );
                                XmlNode->GetPropVal( wxT( "id" ), &Value );
                                Value.ToLong( &RadioStation->m_SCId );
                                //XmlNode->GetPropVal( wxT( "br" ), &Value );
                                //Value.ToLong( &RadioStation->m_BitRate );
                                RadioStation->m_BitRate = BitRate;
                                RadioStation->m_GenreId = GenreId;
                                RadioStation->m_IsUser = false;
                                XmlNode->GetPropVal( wxT( "lc" ), &Value );
                                Value.ToLong( &RadioStation->m_Listeners );

                                Stations->Add( RadioStation );
                                //guLogMessage( wxT( "Station: '%s'" ), RadioStation->Name.c_str() );
                            }
                        }
                    }
                    XmlNode = XmlNode->GetNext();
                }
            }
        }
        else
        {
          guLogError( wxT( "ee: Could not get radio stations for genre '%s'" ), GenreName.c_str() );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guShoutCast::GetStationUrl( const int id ) const
{
    return wxString::Format( SHOUTCAST_GET_STATION_PLAYLIST, id );
}

// -------------------------------------------------------------------------------- //
guStationPlayList guShoutCast::GetStationPlayList( const int StationId ) const
{
    wxString                Content;
    guStationPlayList       RetVal;
    guStationPlayListItem * NewStation;
    wxFileConfig *          Config;
    char *                  Buffer = NULL;
    wxCurlHTTP              http;
    //
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, wxString::Format( SHOUTCAST_GET_STATION_PLAYLIST, StationId ) );
    if( Buffer )
    {
        Content = wxString( Buffer, wxConvUTF8 );
        free( Buffer );
        if( Content.Length() )
        {
            //guLogMessage( Content );
            wxStringInputStream ConfigData( Content );
            Config = new wxFileConfig( ConfigData );
            if( Config )
            {
                if( Config->HasGroup( wxT( "playlist" ) ) )
                {
                    Config->SetPath( wxT( "playlist" ) );
                    int count;
                    if( Config->Read( wxT( "numberofentries" ), &count ) )
                    {
                        if( !count )
                        {
                            guLogMessage( wxT( "This station playlist is empty" ) );
                        }
                        else
                        {
                            for( int index = 1; index <= count; index++ )
                            {
                                NewStation = new guStationPlayListItem();

                                wxASSERT( NewStation );

                                Config->Read( wxString::Format( wxT( "File%u" ), index ), &NewStation->m_Location );
                                Config->Read( wxString::Format( wxT( "Title%u" ), index ), &NewStation->m_Name );
                                if( NewStation->m_Name.IsEmpty() )
                                    NewStation->m_Name = NewStation->m_Location;
                                RetVal.Add( NewStation );
                            }
                        }
                    }
                }
                else
                {
                    guLogError( wxT( "ee: Station Playlist without 'playlist' group" ) );
                }
                delete Config;
            }
            else
              guLogError( wxT( "ee: Station Playlist empty" ) );
        }
        else
        {
            guLogError( wxT( "Retrieving station playlist empty response" ) );
        }
    }
    else
    {
        guLogError( wxT( "No response from server retrieving the station playlist." ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guStationPlayList guShoutCast::GetStationPlayList( const wxString &stationurl ) const
{
    wxString                Content;
    guStationPlayList       RetVal;
    guStationPlayListItem * NewStation;
    wxFileConfig *          Config;
    char *                  Buffer = NULL;
    wxCurlHTTP              http;
    //
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, stationurl );
    if( Buffer )
    {
        Content = wxString( Buffer, wxConvUTF8 );
        free( Buffer );
        if( Content.Length() )
        {
            //guLogMessage( Content );
            wxStringInputStream ConfigData( Content );
            Config = new wxFileConfig( ConfigData );
            if( Config )
            {
                if( Config->HasGroup( wxT( "playlist" ) ) )
                {
                    Config->SetPath( wxT( "playlist" ) );
                    int count;
                    if( Config->Read( wxT( "numberofentries" ), &count ) )
                    {
                        if( !count )
                        {
                            guLogMessage( wxT( "This station playlist is empty" ) );
                        }
                        else
                        {
                            for( int index = 1; index <= count; index++ )
                            {
                                NewStation = new guStationPlayListItem();

                                wxASSERT( NewStation );

                                Config->Read( wxString::Format( wxT( "File%u" ), index ), &NewStation->m_Location );
                                Config->Read( wxString::Format( wxT( "Title%u" ), index ), &NewStation->m_Name );
                                if( NewStation->m_Name.IsEmpty() )
                                    NewStation->m_Name = NewStation->m_Location;
                                RetVal.Add( NewStation );
                            }
                        }
                    }
                }
                else
                {
                    guLogError( wxT( "ee: Station Playlist without 'playlist' group" ) );
                }
                delete Config;
            }
            else
              guLogError( wxT( "ee: Station Playlist empty" ) );
        }
        else
        {
            guLogError( wxT( "Retrieving station playlist empty response" ) );
        }
    }
    else
    {
        guLogError( wxT( "No response from server retrieving the station playlist." ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString GetProperty( const wxString Content, const wxString Name )
{
    wxString RetVal;
    int StrPos = Content.Find( Name );
    if( StrPos != wxNOT_FOUND )
    {
        RetVal = Content.Mid( StrPos );
        RetVal = RetVal.Mid( RetVal.Find( wxT( "class=default><b>" ) ) );
        RetVal = RetVal.Mid( 17 );
        RetVal = RetVal.Mid( 0, RetVal.Find( wxT( "</b>" ) ) );
        //guLogMessage( Name + RetVal );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guShoutCast::GetStationStatus( const wxString ServerUrl )
{
    wxString        Content;
    wxArrayString   RetVal;
    char *          Buffer = NULL;
    wxCurlHTTP      http;
    //
    if( m_LastServerUrl != ServerUrl )
    {
        Content = ServerUrl;
        if( !Content.EndsWith( wxT( "/" ) ) )
            Content.Append( wxT( "/" ) );

        http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
        http.AddHeader( wxT( "Accept: text/html" ) );
        http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
        http.Get( Buffer, Content );
        if( Buffer )
        {
            Content = wxString( Buffer, wxConvUTF8 );
            free( Buffer );

//            if( Content.Len() && http.GetResHeader( wxT( "Content-Type" ) ).Find( wxT( "text/html" ) ) != wxNOT_FOUND )
            if( Content.Length() )
            {
                Content = Content.Mid( Content.Find( wxT( "Current Stream Information" ) ) );
                RetVal.Add( GetProperty( Content, wxT( "Stream Title: " ) ) );
                RetVal.Add( GetProperty( Content, wxT( "Content Type: " ) ) );
                RetVal.Add( GetProperty( Content, wxT( "Current Song: " ) ) );
            }
            m_LastServerData = RetVal;
            m_LastServerUrl = ServerUrl;
        }
    }
    return m_LastServerData;
}

// -------------------------------------------------------------------------------- //