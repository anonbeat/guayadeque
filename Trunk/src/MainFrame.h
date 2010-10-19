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
#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "AlbumBrowser.h"
#include "AuiNotebook.h"
#include "Config.h"
#include "curl/http.h"
#include "dbus/gudbus.h"
#include "dbus/mpris.h"
#include "dbus/mmkeys.h"
#include "dbus/gsession.h"
#include "dbus/notify.h"
#include "DbLibrary.h"
#include "DbCache.h"
#include "FileBrowser.h"
#include "Jamendo.h"
#include "LastFM.h"
#include "LastFMPanel.h"
#include "LibPanel.h"
#include "GIO_Volume.h"
#include "LyricsPanel.h"
#include "Magnatune.h"
#include "PlayerFilters.h"
#include "PlayerPanel.h"
#include "PlayListPanel.h"
#include "PodcastsPanel.h"
#include "PortableMedia.h"
#include "RadioPanel.h"
#include "StatusBar.h"
#include "SplashWin.h"
#include "Vumeters.h"

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
#define     guPANEL_MAIN_PLAYERVUMETERS     ( 1 << 2 )
#define     guPANEL_MAIN_LIBRARY            ( 1 << 3 )
#define     guPANEL_MAIN_RADIOS             ( 1 << 4 )
#define     guPANEL_MAIN_LASTFM             ( 1 << 5 )
#define     guPANEL_MAIN_LYRICS             ( 1 << 6 )
#define     guPANEL_MAIN_PLAYLISTS          ( 1 << 7 )
#define     guPANEL_MAIN_PODCASTS           ( 1 << 8 )
#define     guPANEL_MAIN_ALBUMBROWSER       ( 1 << 9 )
#define     guPANEL_MAIN_FILEBROWSER        ( 1 << 10 )
#define     guPANEL_MAIN_JAMENDO            ( 1 << 11 )
#define     guPANEL_MAIN_MAGNATUNE          ( 1 << 12 )
#define     guPANEL_MAIN_IPOD               ( 1 << 13 )

#define     guPANEL_MAIN_SELECTOR           ( guPANEL_MAIN_LIBRARY | guPANEL_MAIN_RADIOS | \
                                              guPANEL_MAIN_LASTFM | guPANEL_MAIN_LYRICS  | \
                                              guPANEL_MAIN_PLAYLISTS | guPANEL_MAIN_PODCASTS | \
                                              guPANEL_MAIN_ALBUMBROWSER | guPANEL_MAIN_FILEBROWSER )
#define     guPANEL_MAIN_VISIBLE_DEFAULT    ( guPANEL_MAIN_PLAYERPLAYLIST | guPANEL_MAIN_PLAYERFILTERS | \
                                              guPANEL_MAIN_SELECTOR )

enum guUPDATED_TRACKS {
    guUPDATED_TRACKS_NONE = 0,
    guUPDATED_TRACKS_PLAYER,
    guUPDATED_TRACKS_PLAYER_PLAYLIST,
    guUPDATED_TRACKS_LIBRARY,
    guUPDATED_TRACKS_PLAYLISTS
};

class guTaskBarIcon;
class guLibUpdateThread;
class guLibCleanThread;
class guUpdatePodcastsTimer;
class guCopyToThread;

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
    guPlayerVumeters *          m_PlayerVumeters;
    guPlayerPanel *             m_PlayerPanel;
    guLibPanel *                m_LibPanel;
    guRadioPanel *              m_RadioPanel;
    guLastFMPanel *             m_LastFMPanel;
    guLyricsPanel *             m_LyricsPanel;
    guPlayListPanel *           m_PlayListPanel;
    guPodcastPanel *            m_PodcastsPanel;
    guAlbumBrowser *            m_AlbumBrowserPanel;
    guFileBrowser *             m_FileBrowserPanel;
    guJamendoPanel *            m_JamendoPanel;
    guMagnatunePanel *          m_MagnatunePanel;

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
    wxMenuItem *                m_ViewPlayerVumeters;
    wxMenuItem *                m_ViewLibrary;
    wxMenuItem *                m_ViewLibTextSearch;
    wxMenuItem *                m_ViewLibLabels;
    wxMenuItem *                m_ViewLibGenres;
    wxMenuItem *                m_ViewLibArtists;
    wxMenuItem *                m_ViewLibAlbums;
    wxMenuItem *                m_ViewLibYears;
    wxMenuItem *                m_ViewLibRatings;
    wxMenuItem *                m_ViewLibPlayCounts;
    wxMenuItem *                m_ViewLibComposers;
    wxMenuItem *                m_ViewLibAlbumArtists;

    wxMenuItem *                m_ViewRadios;
    wxMenuItem *                m_ViewRadTextSearch;
    wxMenuItem *                m_ViewRadLabels;
    wxMenuItem *                m_ViewRadGenres;

    wxMenuItem *                m_ViewLastFM;
    wxMenuItem *                m_ViewLyrics;

    wxMenuItem *                m_ViewPlayLists;
    wxMenuItem *                m_ViewPLTextSearch;

    wxMenuItem *                m_ViewAlbumBrowser;

    wxMenuItem *                m_ViewFileBrowser;

    wxMenuItem *                m_ViewJamendo;
    wxMenuItem *                m_ViewJamTextSearch;
    wxMenuItem *                m_ViewJamLabels;
    wxMenuItem *                m_ViewJamGenres;
    wxMenuItem *                m_ViewJamArtists;
    wxMenuItem *                m_ViewJamAlbums;
    wxMenuItem *                m_ViewJamYears;
    wxMenuItem *                m_ViewJamRatings;
    wxMenuItem *                m_ViewJamPlayCounts;
    wxMenuItem *                m_ViewJamComposers;
    wxMenuItem *                m_ViewJamAlbumArtists;

    wxMenuItem *                m_ViewMagnatune;
    wxMenuItem *                m_ViewMagTextSearch;
    wxMenuItem *                m_ViewMagLabels;
    wxMenuItem *                m_ViewMagGenres;
    wxMenuItem *                m_ViewMagArtists;
    wxMenuItem *                m_ViewMagAlbums;
    wxMenuItem *                m_ViewMagYears;
    wxMenuItem *                m_ViewMagRatings;
    wxMenuItem *                m_ViewMagPlayCounts;
    wxMenuItem *                m_ViewMagComposers;
    wxMenuItem *                m_ViewMagAlbumArtists;

    wxMenuItem *                m_ViewPodcasts;
    wxMenuItem *                m_ViewPodChannels;
    wxMenuItem *                m_ViewPodDetails;

    wxMenuItem *                m_ViewFullScreen;
    wxMenuItem *                m_ViewStatusBar;

    wxMenu *                    m_PortableDevicesMenu;


    guDbLibrary *               m_Db;
    guDbCache *                 m_DbCache;
    guJamendoLibrary *          m_JamendoDb;
    guMagnatuneLibrary *        m_MagnatuneDb;
    guLibUpdateThread *         m_LibUpdateThread;
    guLibCleanThread *          m_LibCleanThread;

    wxIcon                      m_AppIcon;

    guUpdatePodcastsTimer *     m_UpdatePodcastsTimer;

    guPodcastDownloadQueueThread    * m_DownloadThread;
    wxMutex                     m_DownloadThreadMutex;

    guDBusServer *              m_DBusServer;
    guMPRIS *                   m_MPRIS;
    guMMKeys *                  m_MMKeys;
    guGSession *                m_GSession;
    guDBusNotify *              m_NotifySrv;

    guGIO_VolumeMonitor *       m_VolumeMonitor;
    guPortableMediaLibraryArray m_PortableMediaDbs;
    guPortableMediaPanelArray   m_PortableMediaPanels;

    wxWindow *                  m_CurrentPage;

    wxLongLong                  m_SelCount;
    wxLongLong                  m_SelLength;
    wxLongLong                  m_SelSize;

    // Layouts
    wxArrayString               m_LayoutName;
    wxArrayString               m_LayoutData;
    wxArrayString               m_LayoutTabs;

    guCopyToThread *            m_CopyToThread;
    wxMutex                     m_CopyToThreadMutex;


    void                OnUpdateLibrary( wxCommandEvent &event );
    void                OnUpdatePodcasts( wxCommandEvent &event );
    void                OnUpdateCovers( wxCommandEvent &event );
    void                OnUpdateTrack( wxCommandEvent &event );
    void                OnPlayerStatusChanged( wxCommandEvent &event );
    void                OnPlayerTrackListChanged( wxCommandEvent &event );
    void                OnPlayerCapsChanged( wxCommandEvent &event );
    void                OnAudioScrobbleUpdate( wxCommandEvent &event );
    void                CreatePortablePlayersMenu( wxMenu * menu );
    void                CreateMenu();
    void                DoCreateStatusBar( int kind );
    void                OnCloseWindow( wxCloseEvent &event );
    //void                OnIconizeWindow( wxIconizeEvent &event );
    void                OnPreferences( wxCommandEvent &event );

    void                OnPlay( wxCommandEvent &event );
    void                OnStop( wxCommandEvent &event );
    void                OnNextTrack( wxCommandEvent &event );
    void                OnPrevTrack( wxCommandEvent &event );
    void                OnNextAlbum( wxCommandEvent &event );
    void                OnPrevAlbum( wxCommandEvent &event );
    void                OnSmartPlay( wxCommandEvent &event );
    void                OnRandomize( wxCommandEvent &event );
    void                OnRepeat( wxCommandEvent &event );
    void                OnAbout( wxCommandEvent &event );
    void                OnHelp( wxCommandEvent &event );
    void                OnCommunity( wxCommandEvent &event );
    void                OnCopyTracksTo( wxCommandEvent &event );
    void                OnCopyTracksToDevice( wxCommandEvent &event );
    void                OnUpdateLabels( wxCommandEvent &event );
    void                OnPlayerPlayListUpdateTitle( wxCommandEvent &event );

    void                CheckShowNotebook( void );
    void                CheckHideNotebook( void );

    void                OnGaugePulse( wxCommandEvent &event );
    void                OnGaugeSetMax( wxCommandEvent &event );
    void                OnGaugeUpdate( wxCommandEvent &event );
    void                OnGaugeRemove( wxCommandEvent &event );

    void                OnSelectTrack( wxCommandEvent &event );
    void                OnSelectAlbum( wxCommandEvent &event );
    void                OnSelectArtist( wxCommandEvent &event );
    void                OnSelectYear( wxCommandEvent &event );
    void                OnSelectGenre( wxCommandEvent &event );
    void                OnGenreSetSelection( wxCommandEvent &event );
    void                OnAlbumArtistSetSelection( wxCommandEvent &event );
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
    void                OnRequestCurrentTrack( wxCommandEvent &event );

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
    void                OnPlayListShowPanel( wxCommandEvent &event );

    void                OnViewPodcasts( wxCommandEvent &event );
    void                OnPodcastsShowPanel( wxCommandEvent &event );

    void                OnViewAlbumBrowser( wxCommandEvent &event );

    void                OnViewFileBrowser( wxCommandEvent &event );

    void                OnViewJamendo( wxCommandEvent &event );
    void                OnJamendoShowPanel( wxCommandEvent &event );

    void                OnViewMagnatune( wxCommandEvent &event );
    void                OnMagnatuneShowPanel( wxCommandEvent &event );

    void                OnViewPortableDevice( wxCommandEvent &event );
    void                OnViewPortableDevicePanel( wxCommandEvent &event );

    void                OnMainPaneClose( wxAuiManagerEvent &event );

    void                LoadTabsPerspective( const wxString &layout );

    void                OnForceUpdateLibrary( wxCommandEvent &event );
    void                OnAddLibraryPath( wxCommandEvent &event );

    void                OnViewFullScreen( wxCommandEvent &event );
    void                OnViewStatusBar( wxCommandEvent &event );

    // There is a bug that dont removes correctly the last window from the AuiNotebook
    // When a new one is inserted if it was already in the Notebook then
    // Its shown the last removed tab instead of the new we created.
    // As a workaround we control if we are going to remove the last window and if so
    // Instead of remove it we hide the Notebook
    // When inserting we need to control if it was hiden and if so add the new one and remove the first
    // We create two functions RemoveTabPanel and InsertTabPanel that controls this
    void                RemoveTabPanel( wxPanel * panel );
    void                InsertTabPanel( wxPanel * panel, const int index, const wxString &label );

    void                OnJamendoCoverDownloaded( wxCommandEvent &event );
    void                OnMagnatuneCoverDownloaded( wxCommandEvent &event );

    void                OnVolumeMonitorUpdated( wxCommandEvent &event );
    void                CreatePortableMediaDeviceMenu( wxMenu * menu, const wxString &devicename, const int basecmd );
    guPortableMediaPanel *  GetPortableMediaPanel( const int basecmd );

  public:
                        guMainFrame( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache );
                        ~guMainFrame();
    void                DoLibraryClean( wxCommandEvent &event );
    void                LibraryUpdated( wxCommandEvent &event );
    void                OnJamendoUpdated( wxCommandEvent &event );
    void                OnMagnatuneUpdated( wxCommandEvent &event );
    void                LibraryCleanFinished( wxCommandEvent &event );
    void                OnQuit( wxCommandEvent &WXUNUSED(event) );
    void                UpdatePodcasts( void );

    void                RemovePodcastsDownloadThread( void );
    void                AddPodcastsDownloadItems( guPodcastItemArray * items );
    void                RemovePodcastDownloadItems( guPodcastItemArray * items );

    void                UpdatedTracks( int updatedby, const guTrackArray * tracks );
    void                UpdatedTrack( int updatedby, const guTrack * track );

    guDBusNotify *      GetNotifyObject( void ) { return m_NotifySrv; };

    guJamendoPanel *    GetJamendoPanel( void ) { return m_JamendoPanel; }
    guJamendoLibrary *  GetJamendoDb( void )
    {
        if( !m_JamendoDb )
            m_JamendoDb = new guJamendoLibrary( wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/Jamendo.db" ) );
         return m_JamendoDb;
    }
    guMagnatunePanel *    GetMagnatunePanel( void ) { return m_MagnatunePanel; }
    guMagnatuneLibrary *  GetMagnatuneDb( void )
    {
        if( !m_MagnatuneDb )
            m_MagnatuneDb = new guMagnatuneLibrary( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/Magnatune.db" ) );
         return m_MagnatuneDb;
    }

    void                CreateCopyToMenu( wxMenu * menu, const int basecmd );

    void                CopyToThreadFinished( void );

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
