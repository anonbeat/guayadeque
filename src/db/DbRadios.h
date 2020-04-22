// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#ifndef __DBRADIOS_H__
#define __DBRADIOS_H__

#include "DbLibrary.h"

#include <wx/dynarray.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guRadioStation
{
  public :
    int         m_Id;
    int         m_SCId;
    wxString    m_Name;
    wxString    m_Link;
    int         m_GenreId;
    wxString    m_GenreName;
    int         m_Source;
    wxString    m_Type;
    int         m_BitRate;
    int         m_Listeners;
    wxString    m_NowPlaying;
};
WX_DECLARE_OBJARRAY(guRadioStation,guRadioStations);

enum guRadioSource {
    guRADIO_SOURCE_SHOUTCAST_GENRE = 0,
    guRADIO_SOURCE_USER,
    guRADIO_SOURCE_SHOUTCAST_SEARCH,
    guRADIO_SOURCE_TUNEIN
};

enum guRADIOSTATION_ORDER {
    guRADIOSTATIONS_ORDER_NAME = 0,
    guRADIOSTATIONS_ORDER_BITRATE,
    guRADIOSTATIONS_ORDER_LISTENERS,
    guRADIOSTATIONS_ORDER_TYPE,
    guRADIOSTATIONS_ORDER_NOWPLAYING
};

// -------------------------------------------------------------------------------- //
class guDbRadios : public guDb
{
  protected :
//    guDb *              m_Db;

    int                 m_StationsOrder; // 0 -> Name, 1 -> BitRate, 2 -> Listeners
    bool                m_StationsOrderDesc;

    // Radio Filters Options
    wxArrayInt          m_RaGeFilters;
    int                 m_RadioSource;
    wxArrayInt          m_RaLaFilters;
    wxArrayString       m_RaTeFilters;

    int                 GetRadioFiltersCount( void ) const;
    wxString            RadioFiltersSQL( void );

  public :
    guDbRadios( const wxString &dbname );
    ~guDbRadios();

    int                 GetRadioLabelsSongs( const wxArrayInt &Labels, guTrackArray * Songs );
    int                 AddRadioLabel( wxString LabelName );
    int                 SetRadioLabelName( const int LabelId, wxString NewName );
    int                 DelRadioLabel( const int LabelId );
    wxArrayInt          GetRadioLabels( void );
    void                GetRadioLabels( guListItems * Labels, const bool FullList = false );

    //
    // Radio support functions
    //
    void                    SetRaTeFilters( const wxArrayString &filters );
    void                    SetRadioLabelsFilters( const wxArrayInt &filters );
    void                    SetRadioGenresFilters( const wxArrayInt &filters );
    void                    SetRadioSourceFilter( int source );
    void                    GetRadioGenresList( const int source, const wxArrayInt &ids, guListItems * listitems, wxArrayInt * radioflags = NULL );
    void                    GetRadioGenres( const int source, guListItems * radiogenres, bool allowfilter = true, wxArrayInt * radioflags = NULL );
    void                    SetRadioGenres( const wxArrayString &Genres );
    int                     GetRadioStations( guRadioStations * stations );
//    void                    GetRadioCounter( wxLongLong * count );
    int                     GetUserRadioStations( guRadioStations * stations );
    void                    SetRadioStation( const guRadioStation * RadioStation );
    bool                    GetRadioStation( const int id, guRadioStation * radiostation );
    bool                    RadioStationExists( const int shoutcastid );
    void                    SetRadioStations( const guRadioStations * RadioStations );
    wxArrayInt              GetStationsSCIds( const wxArrayInt &stations );
    guArrayListItems        GetStationsLabels( const wxArrayInt &Stations );
    void                    SetRadioStationsLabels( const guArrayListItems &labelsets );
    int                     DelRadioStations( const int source, const wxArrayInt &radioids );
    int                     DelRadioStation( const int radioid );
    void                    SetRadioStationsOrder( int OrderValue );

    int                     AddRadioGenre( const wxString &name, const int source, const int flags );
    int                     SetRadioGenre( const int id, const wxString &name, const int source = guRADIO_SOURCE_SHOUTCAST_GENRE,
                                            const int flags = 0 );
    int                     DelRadioGenre( const int GenreId );

};

}

#endif
// -------------------------------------------------------------------------------- //
