// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
#ifndef SMARTMODE_H
#define SMARTMODE_H

#include "DbLibrary.h"
#include "LastFM.h"

#include <wx/wx.h>
#include <wx/thread.h>

enum guSmartModeTrackLimitType {
    guSMARTMODE_TRACK_LIMIT_TRACKS,
    guSMARTMODE_TRACK_LIMIT_TIME_MINUTES,
    guSMARTMODE_TRACK_LIMIT_TIME_HOURS,
    guSMARTMODE_TRACK_LIMIT_SIZE_MB,
    guSMARTMODE_TRACK_LIMIT_SIZE_GB
};

#define guSMARTMODE_FILTER_ALLOW_ALL        0
#define guSMARTMODE_FILTER_DENY_NONE        0

// -------------------------------------------------------------------------------- //
class guSmartModeThread : public wxThread
{
  protected :
    guDbLibrary *   m_Db;

    wxEvtHandler *  m_Owner;

    guLastFM *      m_LastFM;

    wxString        m_ArtistName;
    wxString        m_TrackName;

    int             m_FilterAllowPlayList;
    int             m_FilterDenyPlayList;

    wxArrayInt *    m_SmartAddedTracks;
    wxArrayString * m_SmartAddedArtists;
    int             m_MaxSmartTracksList;
    int             m_MaxSmartArtistsList;

    wxLongLong      m_TrackLimit;
    int             m_LimitType;
    wxLongLong      m_LimitCounter;
    bool            m_LimitReached;

    int             m_GaugeId;

    bool            CheckAddTrack( const wxString &artist, const wxString &track, guTrackArray * tracks );
    int             AddSimilarTracks( const wxString &artist, const wxString &track, guTrackArray * tracks );

    bool            CheckLimit( const guTrack * track = NULL );
    void            SendTracks( guTrackArray * tracks );

  public:
    guSmartModeThread( guDbLibrary * db, wxEvtHandler * owner,
            const wxString &artistname, const wxString &trackname,
             wxArrayInt * smartaddedtracks, wxArrayString * smartaddedartists,
             const int maxtracks, const int maxartists,
             const double tracklimit, const int limittype = guSMARTMODE_TRACK_LIMIT_TRACKS,
             const int filterallow = guSMARTMODE_FILTER_ALLOW_ALL, const int filterdeny = guSMARTMODE_FILTER_DENY_NONE,
             const int gaugeid = wxNOT_FOUND );

    ~guSmartModeThread();

    virtual ExitCode Entry();
};

class guMediaViewer;
// -------------------------------------------------------------------------------- //
class guGenSmartPlaylist : public wxDialog
{
  protected:
    wxComboBox *    m_SaveToComboBox;
    wxChoice *      m_FilterAlowChoice;
    wxChoice *      m_FilterDenyChoice;
    wxTextCtrl *    m_LimitTextCtrl;
    wxChoice *      m_LimitChoice;

    guMediaViewer * m_MediaViewer;
    guDbLibrary *   m_Db;

    guListItems     m_Playlists;

  public:
    guGenSmartPlaylist( wxWindow * parent, guMediaViewer * mediaviewer, const wxString &playlistname );
    ~guGenSmartPlaylist();

    int             GetPlayListId( void );
    wxString        GetPlaylistName( void ) { return m_SaveToComboBox->GetValue(); }
    int             GetAllowFilter( void ) { return m_Playlists[ m_FilterAlowChoice->GetSelection() ].m_Id; }
    int             GetDenyFilter( void ) { return m_Playlists[ m_FilterDenyChoice->GetSelection() ].m_Id; }
    double          GetLimitValue( void ) { double RetVal = 0; m_LimitTextCtrl->GetValue().ToDouble( &RetVal ); return RetVal; }
    int             GetLimitType( void ) { return m_LimitChoice->GetSelection(); }
};


#endif
// -------------------------------------------------------------------------------- //
