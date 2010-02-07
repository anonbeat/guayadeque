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

#include "AuiNotebook.h"
#include "Config.h"
#include "DbLibrary.h"
#include "DbCache.h"
#include "LastFM.h"
#include "LastFMPanel.h"
#include "LibPanel.h"
//#include "LibUpdate.h"
#include "LyricsPanel.h"
#include "PlayerFilters.h"
#include "PlayerPanel.h"
#include "PlayListPanel.h"
#include "PodcastsPanel.h"
#include "RadioPanel.h"
#include "StatusBar.h"
#include "SplashWin.h"
#include "curl/http.h"

#include "dbus/gudbus.h"
#include "dbus/mpris.h"
#include "dbus/mmkeys.h"
#include "dbus/gsession.h"

#include <wx/aui/aui.h>
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/uri.h>
#include <wx/xml/xml.h>
#include <wx/zstream.h>

#define     guPANEL_MAIN_PLAYERPLAYLIST     ( 1 << 0 )
#define     guPANEL_MAIN_PLAYERFILTERS      ( 1 << 1 )
#define     guPANEL_MAIN_SELECTOR           ( 1 << 2 )
#define     guPANEL_MAIN_LIBRARY            ( 1 << 3 )
#define     guPANEL_MAIN_RADIOS             ( 1 << 4 )
#define     guPANEL_MAIN_LASTFM             ( 1 << 5 )
#define     guPANEL_MAIN_LYRICS             ( 1 << 6 )
#define     guPANEL_MAIN_PLAYLISTS          ( 1 << 7 )
#define     guPANEL_MAIN_PODCASTS           ( 1 << 8 )


class guTaskBarIcon;
class guLibUpdateThread;
class guUpdatePodcastsTimer;

// -------------------------------------------------------------------------------- //
class guMainFrame : public wxFrame
{
  private:
    wxAuiManager                m_AuiManager;
    unsigned int                m_VisiblePanels;

    guAuiNotebook *             m_CatNotebook;
    wxString                    m_NBPerspective;
    wxSplitterWindow *          m_PlayerSplitter;
    guPlayerFilters *           m_PlayerFilters;
    guPlayerPlayList *          m_PlayerPlayList;
    guPlayerPanel *             m_PlayerPanel;
    guLibPanel *                m_LibPanel;
    guRadioPanel *              m_RadioPanel;
    guLastFMPanel *             m_LastFMPanel;
    guLyricsPanel *             m_LyricsPanel;
    guPlayListPanel *           m_PlayListPanel;
    guPodcastPanel *            m_PodcastsPanel;
    guTaskBarIcon *             m_TaskBarIcon;
    guStatusBar *               m_MainStatusBar;

    wxMenuItem *                m_PlaySmartMenuItem;
    wxMenuItem *                m_LoopPlayListMenuItem;
    wxMenuItem *                m_LoopTrackMenuItem;

    wxMenu *                    m_MainMenu;
    wxMenu *                    m_LayoutLoadMenu;
    wxMenu *                    m_LayoutDelMenu;

    wxMenuItem *                m_ViewPlayerPlayList;
    wxMenuItem *                m_ViewPlayerFilters;
    wxMenuItem *                m_ViewPlayerSelector;
    wxMenuItem *                m_ViewLibrary;
    wxMenuItem *                m_ViewLibTextSearch;
    wxMenuItem *                m_ViewLibLabels;
    wxMenuItem *                m_ViewLibGenres;
    wxMenuItem *                m_ViewLibArtists;
    wxMenuItem *                m_ViewLibAlbums;
//    wxMenuItem *                m_ViewLibTracks;
//    wxMenuItem *                m_ViewLibYears;
//    wxMenuItem *                m_ViewLibRattings;
    wxMenuItem *                m_ViewRadios;
    wxMenuItem *                m_ViewRadTextSearch;
    wxMenuItem *                m_ViewRadLabels;
    wxMenuItem *                m_ViewRadGenres;

    wxMenuItem *                m_ViewLastFM;
    wxMenuItem *                m_ViewLyrics;
    wxMenuItem *                m_ViewPlayLists;

    wxMenuItem *                m_ViewPodcasts;
    wxMenuItem *                m_ViewPodChannels;
    wxMenuItem *                m_ViewPodDetails;


    guDbLibrary *               m_Db;
    guDbCache *                 m_DbCache;
    guLibUpdateThread *         m_LibUpdateThread;

    wxIcon                      m_AppIcon;

    guUpdatePodcastsTimer *     m_UpdatePodcastsTimer;

    guPodcastDownloadQueueThread    * m_DownloadThread;
    wxMutex                     m_DownloadThreadMutex;

    guDBusServer *              m_DBusServer;
    guMPRIS *                   m_MPRIS;
    guMMKeys *                  m_MMKeys;
    guGSession *                m_GSession;

    wxWindow *                  m_CurrentPage;

    wxLongLong                  m_SelCount;
    wxLongLong                  m_SelLength;
    wxLongLong                  m_SelSize;

    // Layouts
    wxArrayString               m_LayoutName;
    wxArrayString               m_LayoutData;
    wxArrayString               m_LayoutTabs;


    void                OnUpdateLibrary( wxCommandEvent &event );
    void                OnUpdatePodcasts( wxCommandEvent &event );
    void                OnUpdateCovers( wxCommandEvent &event );
    void                OnUpdateTrack( wxCommandEvent &event );
    void                OnPlayerStatusChanged( wxCommandEvent &event );
    void                OnPlayerTrackListChanged( wxCommandEvent &event );
    void                OnPlayerCapsChanged( wxCommandEvent &event );
    void                OnAudioScrobbleUpdate( wxCommandEvent &event );
    void                CreateMenu();
    void                DoCreateStatusBar( int kind );
    void                OnCloseWindow( wxCloseEvent &event );
    void                OnIconizeWindow( wxIconizeEvent &event );
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
    void                OnPlayerPlayListUpdateTitle( wxCommandEvent &event );

    void                CheckShowNotebook( void );
    void                CheckHideNotebook( void );

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

    void                OnPodcastItemUpdated( wxCommandEvent &event );
    void                OnRemovePodcastThread( wxCommandEvent &event );

    void                OnIdle( wxIdleEvent &event );
    void                OnPageChanged( wxAuiNotebookEvent& event );
    void                OnPageClosed( wxAuiNotebookEvent& event );

//    void                SetLibTracks( wxCommandEvent &event );
//    void                SetRadioStations( wxCommandEvent &event );
//    void                SetPlayListTracks( wxCommandEvent &event );
//    void                SetPodcasts( wxCommandEvent &event );
    void                OnUpdateSelInfo( wxCommandEvent &event );

    //void                OnSysColorChanged( wxSysColourChangedEvent &event );

    void                OnCreateNewLayout( wxCommandEvent &event );
    void                LoadLayouts( void );
    void                SaveLayouts( void );
    void                OnLoadLayout( wxCommandEvent &event );
    void                OnDeleteLayout( wxCommandEvent &event );

    void                OnPlayerShowPanel( wxCommandEvent &event );
    void                ShowMainPanel( const int panelid, const bool enable );

    void                OnViewLibrary( wxCommandEvent &event );
    void                OnLibraryShowPanel( wxCommandEvent &event );

    void                OnViewRadio( wxCommandEvent &event );
    void                OnRadioShowPanel( wxCommandEvent &event );

    void                OnViewLastFM( wxCommandEvent &event );
    void                OnViewLyrics( wxCommandEvent &event );
    void                OnViewPlayLists( wxCommandEvent &event );

    void                OnViewPodcasts( wxCommandEvent &event );
    void                OnPodcastsShowPanel( wxCommandEvent &event );

    void                OnMainPaneClose( wxAuiManagerEvent &event );

  public:
                        guMainFrame( wxWindow * parent );
                        ~guMainFrame();
    void                LibraryUpdated( wxCommandEvent &event );
    void                OnQuit( wxCommandEvent &WXUNUSED(event) );
    void                UpdatePodcasts( void );

    void                RemovePodcastsDownloadThread( void );
    void                AddPodcastsDownloadItems( guPodcastItemArray * items );
    void                RemovePodcastDownloadItems( guPodcastItemArray * items );

};

// -------------------------------------------------------------------------------- //
class guUpdateCoversThread : public wxThread
{
  private:
    guDbLibrary * m_Db;
    int         m_GaugeId;

  public:
    guUpdateCoversThread( guDbLibrary * db, int gaugeid );
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

// -------------------------------------------------------------------------------- //
class guUpdatePodcastsTimer : public wxTimer
{
  protected :
    guDbLibrary * m_Db;
    guMainFrame * m_MainFrame;

  public:
    guUpdatePodcastsTimer( guMainFrame * mainframe, guDbLibrary * db );

    //Called each time the timer's timeout expires
    void Notify();
};

// -------------------------------------------------------------------------------- //
class guUpdatePodcastsThread : public wxThread
{
  protected :
    int             m_GaugeId;
    guDbLibrary *     m_Db;
    guMainFrame *   m_MainFrame;

  public :
    guUpdatePodcastsThread( guDbLibrary * db, guMainFrame * mainframe, int gaugeid );
    ~guUpdatePodcastsThread();

    virtual ExitCode Entry();

};

#endif
// -------------------------------------------------------------------------------- //
