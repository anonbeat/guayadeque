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
#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "Config.h"
#include "DbLibrary.h"
#include "LastFM.h"
#include "LastFMPanel.h"
#include "LibPanel.h"
//#include "LibUpdate.h"
#include "LyricsPanel.h"
#include "PlayerPanel.h"
#include "PlayListPanel.h"
#include "RadioPanel.h"
#include "StatusBar.h"
#include "SplashWin.h"

#include "dbus/mpris.h"

#include <wx/wx.h>
#include <wx/splitter.h>


class guTaskBarIcon;
class guLibUpdateThread;

// -------------------------------------------------------------------------------- //
class guMainFrame : public wxFrame
{
  private:
    wxNotebook *        m_CatNotebook;
    wxSplitterWindow *  m_PlayerSplitter;
    guPlayerPanel *     m_PlayerPanel;
    guLibPanel *        m_LibPanel;
    guRadioPanel *      m_RadioPanel;
    guLastFMPanel *     m_LastFMPanel;
    guLyricsPanel *     m_LyricsPanel;
    guPlayListPanel *   m_PlayListPanel;
    guTaskBarIcon *     m_TaskBarIcon;
    guStatusBar *       m_MainStatusBar;

    DbLibrary *         m_Db;
    guLibUpdateThread * m_LibUpdateThread;

    guMPRIS *           m_MPRIS;

    void                OnUpdateLibrary( wxCommandEvent &event );
    void                OnUpdateCovers( wxCommandEvent &event );
    void                OnUpdateTrack( wxCommandEvent &event );
    void                OnPlayerStatusChanged( wxCommandEvent &event );
    void                OnPlayerTrackListChanged( wxCommandEvent &event );
    void                OnPlayerCapsChanged( wxCommandEvent &event );
    void                OnAudioScrobbleUpdate( wxCommandEvent &event );
    void                PlayerSplitterOnIdle( wxIdleEvent &event );
    void                CreateMenu();
    void                DoCreateStatusBar( int kind );
    void                OnCloseWindow( wxCloseEvent &event );
    void                OnPreferences( wxCommandEvent &event );

    void                OnPlay( wxCommandEvent &event );
    void                OnStop( wxCommandEvent &event );
    void                OnNextTrack( wxCommandEvent &event );
    void                OnPrevTrack( wxCommandEvent &event );
    void                OnSmartPlay( wxCommandEvent &event );
    void                OnRandomize( wxCommandEvent &event );
    void                OnRepeat( wxCommandEvent &event );
    void                OnAbout( wxCommandEvent &event );
    void                OnCopyTracksTo( wxCommandEvent &event );
    void                OnUpdateLabels( wxCommandEvent &event );

    void                OnViewLibrary( wxCommandEvent &event );
    void                OnViewRadio( wxCommandEvent &event );
    void                OnViewLastFM( wxCommandEvent &event );
    void                OnViewLyrics( wxCommandEvent &event );
    void                OnViewPlayLists( wxCommandEvent &event );

    void                OnGaugePulse( wxCommandEvent &event );
    void                OnGaugeSetMax( wxCommandEvent &event );
    void                OnGaugeUpdate( wxCommandEvent &event );
    void                OnGaugeRemove( wxCommandEvent &event );

    void                OnSelectAlbumName( wxCommandEvent &event );
    void                OnSelectArtistName( wxCommandEvent &event );
    void                OnGenreSetSelection( wxCommandEvent &event );
    void                OnArtistSetSelection( wxCommandEvent &event );
    void                OnAlbumSetSelection( wxCommandEvent &event );

    void                OnPlayListUpdated( wxCommandEvent &event );

    void                CreateTaskBarIcon( void );

  public:
                        guMainFrame( wxWindow * parent );
                        ~guMainFrame();
    void                LibraryUpdated( wxCommandEvent &event );
    void                OnQuit( wxCommandEvent &WXUNUSED(event) );

};

// -------------------------------------------------------------------------------- //
class guUpdateCoversThread : public wxThread
{
  private:
    DbLibrary * m_Db;
    int         m_GaugeId;

  public:
    guUpdateCoversThread( DbLibrary * db, int gaugeid );
    ~guUpdateCoversThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guCopyToDirThread : public wxThread
{
  private:
    wxString        m_DestDir;
    guTrackArray *  m_Tracks;
    int             m_GaugeId;

  public:
    guCopyToDirThread( const wxChar * destdir, guTrackArray * tracks, int gaugeid );
    ~guCopyToDirThread();

    virtual ExitCode Entry();
};

#endif
// -------------------------------------------------------------------------------- //
