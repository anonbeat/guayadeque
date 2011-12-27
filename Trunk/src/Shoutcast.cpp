// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
//	anonbeat@gmail.com
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

#include <wx/xml/xml.h>
#include <wx/fileconf.h>


//#define SHOUTCAST_GET_GENRE_URL         wxT( "http://yp.shoutcast.com/sbin/newxml.phtml" )
//s o 1 N 1 5 v h C B 7 8 Z 6 k 4 __ s h 1 i q r P H n h j F m X i T __ f a 1 _ B m i c Y Q P D b b K m
//a m 1 S G I 9 p 6 W l 8 Y b b W __ f a 1 6 6 9 M u i R P o r U B w __ s h 1 7 t O U i _ e z L 6 1 _ A
#define SHOUTCAST_DEV_KEY0   "sh1iqrPH"
#define SHOUTCAST_DEV_KEY1   "nhjFmXiT"
#define SHOUTCAST_DEV_KEY    SHOUTCAST_DEV_KEY0 SHOUTCAST_DEV_KEY1
#define SHOUTCAST_GET_GENRE_URL         wxT( "http://api.shoutcast.com/legacy/genrelist?k=" SHOUTCAST_DEV_KEY )
//#define SHOUTCAST_GET_STATIONS_URL      wxT( "http://yp.shoutcast.com/sbin/newxml.phtml?genre=%s" )
#define SHOUTCAST_GET_STATIONS_URL      wxT( "http://api.shoutcast.com/legacy/genresearch?k=" SHOUTCAST_DEV_KEY "&genre=%s" )
//#define SHOUTCAST_SEARCH_STATIONS_URL   wxT( "http://yp.shoutcast.com/sbin/newxml.phtml?search=%s" )
#define SHOUTCAST_SEARCH_STATIONS_URL   wxT( "http://api.shoutcast.com/legacy/stationsearch?k=" SHOUTCAST_DEV_KEY "&search=%s" )
//#define SHOUTCAST_GET_STATION_PLAYLIST  wxT( "http://yp.shoutcast.com/sbin/tunein-station.pls?id=%u" )
#define SHOUTCAST_GET_STATION_PLAYLIST  wxT( "http://yp.shoutcast.com/sbin/tunein-station.pls?id=%u" )


// -------------------------------------------------------------------------------- //
wxArrayString guShoutCast::GetGenres( void ) const
{
    wxArrayString   RetVal;
    wxString        Content;
    wxString        GenreName;
    Content = GetUrlContent( SHOUTCAST_GET_GENRE_URL );
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
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guShoutCast::GetStations( const int source, const int flags, const wxString &GenreName, const int GenreId, guRadioStations * Stations, long MinBitRate ) const
{
    wxString    Content;
    wxString    Value;
    long        BitRate;
    wxString    StationName;
    wxString    StationType;
    wxString    StationGenre;
    wxString    StationCurrent;
    long        StationId;
    long        Listeners;

    //guLogMessage( wxT( "About to get stations for genre '%s'" ), GenreName.c_str() );
    //
    //guLogMessage( wxT( "GetStations:\n%s" ), wxString::Format( source == guRADIO_SOURCE_GENRE ?
    //            SHOUTCAST_GET_STATIONS_URL : SHOUTCAST_SEARCH_STATIONS_URL, guURLEncode( GenreName ).c_str() ).c_str() );

    Content = GetUrlContent( wxString::Format( source == guRADIO_SOURCE_GENRE ?
                SHOUTCAST_GET_STATIONS_URL : SHOUTCAST_SEARCH_STATIONS_URL, guURLEncode( GenreName ).c_str() ),
                            wxEmptyString, true );
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
                        XmlNode->GetPropVal( wxT( "name" ), &StationName );
                        StationName.Replace( wxT( " - [SHOUTcast.com]" ), wxT( "" ) );
                        StationName.Replace( wxT( " - a SHOUTcast.com member station" ), wxT( "" ) );
                        XmlNode->GetPropVal( wxT( "mt" ), &StationType );
                        XmlNode->GetPropVal( wxT( "genre" ), &StationGenre );
                        XmlNode->GetPropVal( wxT( "ct" ), &StationCurrent );
                        XmlNode->GetPropVal( wxT( "id" ), &Value );
                        Value.ToLong( &StationId );
                        XmlNode->GetPropVal( wxT( "lc" ), &Value );
                        Value.ToLong( &Listeners );

                        if( StationType.StartsWith( wxT( "audio" ) ) )
                        {

                            guRadioStation * RadioStation = new guRadioStation();
                            if( RadioStation )
                            {
                                RadioStation->m_Id = wxNOT_FOUND;
                                RadioStation->m_Name = StationName;
                                RadioStation->m_SCId = StationId;
                                RadioStation->m_Type = StationType;
                                RadioStation->m_GenreId = GenreId;
                                RadioStation->m_GenreName = StationGenre;
                                RadioStation->m_Source = source;
                                RadioStation->m_Listeners = Listeners;
                                RadioStation->m_BitRate = BitRate;
                                RadioStation->m_NowPlaying = StationCurrent;

                                Stations->Add( RadioStation );
                                //guLogMessage( wxT( "Station: '%s'" ), RadioStation->m_Name.c_str() );
                            }
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

// -------------------------------------------------------------------------------- //
wxString guShoutCast::GetStationUrl( const int id ) const
{
    return wxString::Format( SHOUTCAST_GET_STATION_PLAYLIST, id );
}

// -------------------------------------------------------------------------------- //
guPlaylistItemArray guShoutCast::GetStationPlayList( const int StationId ) const
{
    return GetStationPlayList( wxString::Format( SHOUTCAST_GET_STATION_PLAYLIST, StationId ) );
}

// -------------------------------------------------------------------------------- //
guPlaylistItemArray guShoutCast::GetStationPlayList( const wxString &stationurl ) const
{
    wxString                Content;
    guPlaylistItemArray       RetVal;
    guPlaylistItem * NewStation;
    wxFileConfig *          PlayList;
    Content = GetUrlContent( stationurl );
    if( Content.Length() )
    {
        //guLogMessage( Content );
        wxStringInputStream ConfigData( Content );
        PlayList = new wxFileConfig( ConfigData );
        if( PlayList )
        {
            if( PlayList->HasGroup( wxT( "playlist" ) ) )
            {
                PlayList->SetPath( wxT( "playlist" ) );
                int count;
                if( PlayList->Read( wxT( "numberofentries" ), &count ) )
                {
                    if( !count )
                    {
                        guLogMessage( wxT( "This station playlist is empty" ) );
                    }
                    else
                    {
                        for( int index = 1; index <= count; index++ )
                        {
                            NewStation = new guPlaylistItem();

                            PlayList->Read( wxString::Format( wxT( "File%u" ), index ), &NewStation->m_Location );
                            PlayList->Read( wxString::Format( wxT( "Title%u" ), index ), &NewStation->m_Name );
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
            delete PlayList;
        }
        else
          guLogError( wxT( "ee: Station Playlist empty" ) );
    }
    else
    {
        guLogError( wxT( "Retrieving station playlist empty response" ) );
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
    //
    if( m_LastServerUrl != ServerUrl )
    {
        Content = ServerUrl;
        if( !Content.EndsWith( wxT( "/" ) ) )
            Content.Append( wxT( "/" ) );

        Content = GetUrlContent( Content );
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
    return m_LastServerData;
}

// -------------------------------------------------------------------------------- //
