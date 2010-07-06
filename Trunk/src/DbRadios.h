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
#ifndef DBRADIOS_H
#define DBRADIOS_H

#include "DbLibrary.h"

#include <wx/dynarray.h>

// -------------------------------------------------------------------------------- //
class guRadioStation
{
  public :
    int         m_Id;
    long        m_SCId;
    wxString    m_Name;
    wxString    m_Link;
    long        m_GenreId;
    wxString    m_GenreName;
    int         m_Source;
    wxString    m_Type;
    long        m_BitRate;
    long        m_Listeners;
    wxString    m_NowPlaying;
};
WX_DECLARE_OBJARRAY(guRadioStation,guRadioStations);

enum guRADIOSTATION_ORDER {
    guRADIOSTATIONS_ORDER_NAME = 0,
    guRADIOSTATIONS_ORDER_BITRATE,
    guRADIOSTATIONS_ORDER_LISTENERS,
    guRADIOSTATIONS_ORDER_TYPE,
    guRADIOSTATIONS_ORDER_NOWPLAYING
};

// -------------------------------------------------------------------------------- //
class guDbRadios
{
  protected :
    guDb *              m_Db;

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
    guDbRadios( guDb * db );
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
    guArrayListItems        GetStationsLabels( const wxArrayInt &Stations );
    void                    SetRadioStationsLabels( const guArrayListItems &labelsets );
    int                     DelRadioStations( const int source, const wxArrayInt &radioids );
    int                     DelRadioStation( const int radioid );
    void                    SetRadioStationsOrder( int OrderValue );

    int                     AddRadioGenre( const wxString &name, const int source, const int flags );
    int                     SetRadioGenre( const int id, const wxString &name, const int source = guRADIO_SOURCE_GENRE,
                                            const int flags = 0 );
    int                     DelRadioGenre( const int GenreId );

};

#endif
// -------------------------------------------------------------------------------- //
