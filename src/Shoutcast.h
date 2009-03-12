// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
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
#ifndef SHOUTCAST_H
#define SHOUTCAST_H

#include "DbLibrary.h"

class guStationPlayList
{
  public:
    wxString m_Name;
    wxString m_Url;

};

WX_DECLARE_OBJARRAY(guStationPlayList, guStationPlayLists);

#define SHOUTCAST_STATION_STATUS_NAME       0
#define SHOUTCAST_STATION_STATUS_GENRE      1
#define SHOUTCAST_STATION_STATUS_URL        2
#define SHOUTCAST_STATION_STATUS_TYPE       3
#define SHOUTCAST_STATION_STATUS_BITRATE    4
#define SHOUTCAST_STATION_STATUS_CURSONG    5

#define SHOUTCAST_STATION_ALLBITRATES       -1

// -------------------------------------------------------------------------------- //
class guShoutCast
{
  protected :
   wxString         m_LastServerUrl;
   wxArrayString    m_LastServerData;

  public :
    guShoutCast() { m_LastServerUrl = wxEmptyString; };
    wxArrayString       GetGenres( void ) const;
    void                GetStations( const wxString &GenreName, const int GenreId, guRadioStations * Stations, long MinBitrate = SHOUTCAST_STATION_ALLBITRATES ) const;
    guStationPlayLists  GetStationPlayList( int StationId ) const;
    wxArrayString       GetStationStatus( const wxString ServerUrl );
};

#endif
// -------------------------------------------------------------------------------- //
