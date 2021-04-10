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
#ifndef __MAINFRAME_H__
#define __MAINFRAME_H__

#include "AlbumBrowser.h"
#include "AudioCdPanel.h"
#include "AuiNotebook.h"
#include "Config.h"
#include "CoverPanel.h"
#include "Collections.h"
#include "Http.h"
#include "dbus/gudbus.h"
#include "dbus/mpris.h"
#include "dbus/mpris2.h"
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
#include "MediaViewer.h"
#include "PlayerFilters.h"
#include "PlayerPanel.h"
#include "PlayListPanel.h"
#include "PodcastsPanel.h"
#include "PortableMedia.h"
#include "RadioPanel.h"
#include "StatusBar.h"
#include "SplashWin.h"
#include "TrackEdit.h"
#include "TreePanel.h"
#include "Vumeters.h"

#include <wx/aui/aui.h>
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/uri.h>
#include <wx/xml/xml.h>
#include <wx/zstream.h>

namespace Guayadeque {

#ifdef WITH_LIBINDICATE_SUPPORT

#define GUAYADEQUE_INDICATOR_NAME               "music.guayadeque"
#define GUAYADEQUE_DESKTOP_PATH                 DATAROOTDIR "/applications/guayadeque.desktop"

#include "libindicate/server.h"
#include "libindicate/indicator.h"

#endif

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
#define     guPANEL_MAIN_LOCATIONS          ( 1 << 13 )
#define     guPANEL_MAIN_SHOWCOVER          ( 1 << 14 )
#define     guPANEL_MAIN_TREEVIEW           ( 1 << 15 )
#define     guPANEL_MAIN_AUDIOCD            ( 1 << 16 )


#define     guPANEL_MAIN_SELECTOR           ( guPANEL_MAIN_RADIOS | guPANEL_MAIN_LASTFM | guPANEL_MAIN_LYRICS | guPANEL_MAIN_PODCASTS )
#define     guPANEL_MAIN_VISIBLE_DEFAULT    ( guPANEL_MAIN_PLAYERPLAYLIST | guPANEL_MAIN_PLAYERFILTERS | \
                                              guPANEL_MAIN_SELECTOR )

#define     guPORTABLEDEVICE_COMMANDS_COUNT 20  // 0 .. 9 Main Window Commands
                                                // 10 .. 19 -> Library Pane Windows

#define     guWINDOW_STATE_NORMAL           0
#define     guWINDOW_STATE_FULLSCREEN       ( 1 << 0 )
#define     guWINDOW_STATE_MAXIMIZED        ( 1 << 1 )
#define     guWINDOW_STATE_NOSTATUSBAR      ( 1 << 3 )

enum guUPDATED_TRACKS {
    guUPDATED_TRACKS_NONE = 0,
    guUPDATED_TRACKS_PLAYER,
    guUPDATED_TRACKS_PLAYER_PLAYLIST,
//    guUPDATED_TRACKS_PLAYLISTS,
//    guUPDATED_TRACKS_TREEVIEW,
//    guUPDATED_TRACKS_LIBRARY,
    guUPDATED_TRACKS_MEDIAVIEWER
};

class guTaskBarIcon;
class guLibUpdateThread;
class guLibCleanThread;
class guUpdatePodcastsTimer;
class guCopyToThread;
class guLocationPanel;

// -------------------------------------------------------------------------------- //
class guMainFrame : public wxFrame
{
  private :
    static guMainFrame *            m_MainFrame;

  private:
    wxAuiManager                    m_AuiManager;
    unsigned int                    m_VisiblePanels;

    guAuiNotebook *                 m_MainNotebook;
    bool                            m_DestroyLastWindow;
    wxString                        m_NBPerspective;
    wxSplitterWindow *              m_PlayerSplitter;

    guPlayerFilters *               m_PlayerFilters;
    guPlayerPlayList *              m_PlayerPlayList;
    guPlayerVumeters *              m_PlayerVumeters;
    guPlayerPanel *                 m_PlayerPanel;

    guRadioPanel *                  m_RadioPanel;
    guLastFMPanel *                 m_LastFMPanel;
    guLyricsPanel *                 m_LyricsPanel;
    guPodcastPanel *                m_PodcastsPanel;
    guFileBrowser *                 m_FileBrowserPanel;
    guLocationPanel *               m_LocationPanel;
    guCoverPanel *                  m_CoverPanel;

    guAudioCdPanel *                m_AudioCdPanel;

    guTaskBarIcon *                 m_TaskBarIcon;
#ifdef WITH_LIBINDICATE_SUPPORT
    IndicateServer *                m_IndicateServer;
#endif
    guStatusBar *                   m_MainStatusBar;

    wxMenuItem *                    m_MenuPlaySmart;
    wxMenuItem *                    m_MenuLoopPlayList;
    wxMenuItem *                    m_MenuLoopTrack;

    wxMenu *                        m_MenuLayoutLoad;
    wxMenu *                        m_MenuLayoutDelete;

    wxMenuItem *                    m_MenuPlayerPlayList;
    wxMenuItem *                    m_MenuPlayerFilters;
    wxMenuItem *                    m_MenuPlayerVumeters;
    wxMenuItem *                    m_MenuMainLocations;
    wxMenuItem *                    m_MenuMainShowCover;

    wxMenuItem *                    m_MenuRadios;
    wxMenuItem *                    m_MenuRadTextSearch;
//    wxMenuItem *                    m_MenuRadLabels;
    wxMenuItem *                    m_MenuRadGenres;
    wxMenuItem *                    m_MenuRadUpdate;

    wxMenuItem *                    m_MenuPodcasts;
    wxMenuItem *                    m_MenuPodChannels;
    wxMenuItem *                    m_MenuPodDetails;
    wxMenuItem *                    m_MenuPodUpdate;

    wxMenuItem *                    m_MenuLastFM;
    wxMenuItem *                    m_MenuLyrics;

    wxMenuItem *                    m_MenuAudioCD;

    wxMenuItem *                    m_MenuFileBrowser;

    wxMenuItem *                    m_MenuFullScreen;
    wxMenuItem *                    m_MenuStatusBar;

    wxMenuItem *                    m_MenuForceGapless;
    wxMenuItem *                    m_MenuAudioScrobble;

    wxMenu *                        m_MenuCollections;

    guDbLibrary *                   m_Db;
    guDbCache *                     m_DbCache;
    guDbPodcasts *                  m_DbPodcasts;

    wxIcon                          m_AppIcon;

    guUpdatePodcastsTimer *         m_UpdatePodcastsTimer;

    guPodcastDownloadQueueThread *  m_DownloadThread;
    wxMutex                         m_DownloadThreadMutex;

    guDBusServer *                  m_DBusServer;
    guMPRIS *                       m_MPRIS;
    guMPRIS2 *                      m_MPRIS2;
    guMMKeys *                      m_MMKeys;
    guGSession *                    m_GSession;
    guDBusNotify *                  m_NotifySrv;

    guGIO_VolumeMonitor *           m_VolumeMonitor;

    wxWindow *                      m_CurrentPage;

    wxLongLong                      m_SelCount;
    wxLongLong                      m_SelLength;
    wxLongLong                      m_SelSize;

    // Layouts
    wxArrayString                   m_LayoutNames;

    guCopyToThread *                m_CopyToThread;
    wxMutex                         m_CopyToThreadMutex;

    guLyricSearchEngine *           m_LyricSearchEngine;
    guLyricSearchContext *          m_LyricSearchContext;

    guMediaCollectionArray          m_Collections;
    wxMutex                         m_CollectionsMutex;
    guMediaViewerArray              m_MediaViewers;

    int                             m_LoadLayoutPending;

    // Store the Pending Update Tracks
    guTrackArray                    m_PendingUpdateTracks;
    wxArrayString                   m_PendingUpdateFiles;
    guImagePtrArray                 m_PendingUpdateImages;
    wxArrayString                   m_PendingUpdateLyrics;
    wxArrayInt                      m_PendingUpdateFlags;
    wxMutex                         m_PendingUpdateMutex;

    void                            OnUpdatePodcasts( wxCommandEvent &event );
    void                            OnUpdateTrack( wxCommandEvent &event );
    void                            OnPlayerStatusChanged( wxCommandEvent &event );
    void                            OnPlayerTrackListChanged( wxCommandEvent &event );
    void                            OnPlayerCapsChanged( wxCommandEvent &event );
    void                            OnPlayerVolumeChanged( wxCommandEvent &event );
    void                            OnAudioScrobbleUpdate( wxCommandEvent &event );
    void                            OnPlayerSeeked( wxCommandEvent &event );

    guMediaViewer *                 FindCollectionMediaViewer( void * windowptr );

    void                            CreateCollectionsMenu( wxMenu * menu );
    void                            CreateCollectionMenu( wxMenu * menu, const guMediaCollection &collection,
                                                          const int basecommand, const int collectiontype = guMEDIA_COLLECTION_TYPE_NORMAL );
    void                            CollectionsUpdated( void );

    void                            CreateViewMenu( wxMenu * menu );
    void                            CreateLayoutMenus( wxMenu * menu );
    void                            CreateControlsMenu( wxMenu * menu );
    void                            CreateHelpMenu( wxMenu * menu );

    void                            CreateMenu();

    void                            DoCreateStatusBar( int kind );
    void                            OnCloseWindow( wxCloseEvent &event );
    //void                            OnIconizeWindow( wxIconizeEvent &event );
    void                            OnPreferences( wxCommandEvent &event );

    void                            OnPlay( wxCommandEvent &event );
    void                            OnStop( wxCommandEvent &event );
    void                            OnStopAtEnd( wxCommandEvent &event );
    void                            OnClearPlaylist( wxCommandEvent &event );
    void                            OnNextTrack( wxCommandEvent &event );
    void                            OnPrevTrack( wxCommandEvent &event );
    void                            OnNextAlbum( wxCommandEvent &event );
    void                            OnPrevAlbum( wxCommandEvent &event );
    void                            OnRandomize( wxCommandEvent &event );
    void                            OnAbout( wxCommandEvent &event );
    void                            OnHelp( wxCommandEvent &event );
    void                            OnCommunity( wxCommandEvent &event );
    void                            OnPlayMode( wxCommandEvent &event );

    void                            OnCopyTracksTo( wxCommandEvent &event );
    void                            OnCopyTracksToDevice( wxCommandEvent &event );
    void                            OnCopyPlayListToDevice( wxCommandEvent &event );

    void                            OnPlayerPlayListUpdateTitle( wxCommandEvent &event );

    void                            CheckShowNotebook( void );
    void                            CheckHideNotebook( void );

    void                            OnGaugeCreate( wxCommandEvent &event );

    void                            OnSetSelection( wxCommandEvent &event );
    void                            OnSelectLocation( wxCommandEvent &event );

    void                            OnPlayListUpdated( wxCommandEvent &event );

    void                            CreateTaskBarIcon( void );

    void                            OnPodcastItemUpdated( wxCommandEvent &event );
    void                            OnRemovePodcastThread( wxCommandEvent &event );

    void                            OnIdle( wxIdleEvent &event );
    void                            OnSize( wxSizeEvent &event );
    void                            OnIconize( wxIconizeEvent &event );
    void                            OnPageChanged( wxAuiNotebookEvent& event );
    void                            OnPageClosed( wxAuiNotebookEvent& event );
    void                            DoPageClose( wxPanel * panel );

    void                            OnUpdateSelInfo( wxCommandEvent &event );
    void                            OnRequestCurrentTrack( wxCommandEvent &event );

    void                            OnUpdateRadio( wxCommandEvent &event ) { if( m_RadioPanel ) wxPostEvent( m_RadioPanel, event ); }

    //void                            OnSysColorChanged( wxSysColourChangedEvent &event );

    void                            OnCreateNewLayout( wxCommandEvent &event );
    void                            OnLoadLayout( wxCommandEvent &event );
    void                            OnDeleteLayout( wxCommandEvent &event );
    bool                            SaveCurrentLayout( const wxString &layoutname );
    void                            LoadLayoutNames( void );
    void                            ReloadLayoutMenus( void );

    void                            OnPlayerShowPanel( wxCommandEvent &event );
    void                            ShowMainPanel( const int panelid, const bool enable );

    void                            OnCollectionCommand( wxCommandEvent &event );

    void                            OnViewRadio( wxCommandEvent &event );
    void                            OnRadioShowPanel( wxCommandEvent &event );
    void                            OnRadioProperties( wxCommandEvent &event );

    void                            OnViewLastFM( wxCommandEvent &event );
    void                            OnViewLyrics( wxCommandEvent &event );

    void                            OnViewPlayLists( wxCommandEvent &event );
    void                            OnPlayListShowPanel( wxCommandEvent &event );

    void                            OnViewPodcasts( wxCommandEvent &event );
    void                            OnPodcastsShowPanel( wxCommandEvent &event );
    void                            OnPodcastsProperties( wxCommandEvent &event );

    void                            OnViewAudioCD( wxCommandEvent &event );

    void                            OnViewFileBrowser( wxCommandEvent &event );

    void                            OnViewPortableDevice( wxCommandEvent &event );

    void                            OnMainPaneClose( wxAuiManagerEvent &event );

    void                            LoadPerspective( const wxString &layout );
    void                            LoadTabsPerspective( const wxString &layout );

    void                            OnPlayStream( wxCommandEvent &event );

    void                            OnViewFullScreen( wxCommandEvent &event );
    void                            OnViewStatusBar( wxCommandEvent &event );

    // There is a bug that dont removes correctly the last window from the AuiNotebook
    // When a new one is inserted if it was already in the Notebook then
    // Its shown the last removed tab instead of the new we created.
    // As a workaround we control if we are going to remove the last window and if so
    // Instead of remove it we hide the Notebook
    // When inserting we need to control if it was hiden and if so add the new one and remove the first
    // We create two functions RemoveTabPanel and InsertTabPanel that controls this
    void                            RemoveTabPanel( wxPanel * panel );
    void                            InsertTabPanel( wxPanel * panel, const int index, const wxString &label, const wxString &panelid );

    void                            OnLibraryCoverChanged( wxCommandEvent &event );
    void                            OnPlayerPanelCoverChanged( wxCommandEvent &event );

    void                            OnMountMonitorUpdated( wxCommandEvent &event );
    void                            OnAudioCdVolumeUpdated( wxCommandEvent &event );

    void                            OnSetAudioScrobble( wxCommandEvent &event );

    void                            OnLyricFound( wxCommandEvent &event );
    void                            OnLyricSearchFirst( wxCommandEvent &event );
    void                            OnLyricSearchNext( wxCommandEvent &event );
    void                            OnLyricSaveChanges( wxCommandEvent &event );
    void                            OnLyricExecCommand( wxCommandEvent &event );

    void                            OnConfigUpdated( wxCommandEvent &event );

    void                            OnChangeVolume( wxCommandEvent &event );

    void                            OnSongSetRating( wxCommandEvent &event );

    void                            OnSetAllowDenyFilter( wxCommandEvent &event );

    void                            OnRaiseWindow( wxCommandEvent &event );
    void                            OnLoadPlayList( wxCommandEvent &event );

    void                            OnMediaViewerClosed( wxCommandEvent &event ) { MediaViewerClosed( ( guMediaViewer * ) event.GetClientData() ); }

  public:
                                    guMainFrame( wxWindow * parent, guDbCache * dbcache );
                                    ~guMainFrame();

    void                            OnSetForceGapless( wxCommandEvent &event );

    static guMainFrame *            GetMainFrame( void ) { return m_MainFrame; }
    void                            SetMainFrame( void ) { m_MainFrame = this; }

    guRadioPanel *                  GetRadioPanel( void ) { return m_RadioPanel; }
    guPodcastPanel *                GetPodcastsPanel( void ) { return m_PodcastsPanel; }


    guMediaViewer *                 GetDefaultMediaViewer( void );
    const guMediaCollectionArray &  GetMediaCollections( void ) { wxMutexLocker Locker( m_CollectionsMutex ); return m_Collections; }
    bool                            IsCollectionActive( const wxString &uniqueid );
    bool                            IsCollectionPresent( const wxString &uniqueid );
    wxString                        GetCollectionIconString( const wxString &uniqueid );

    void                            OnQuit( wxCommandEvent &event );

    void                            OnCloseTab( wxCommandEvent &event );

    void                            DoShowCaptions( const bool visible );
    void                            OnShowCaptions( wxCommandEvent &event );

    void                            UpdatePodcasts( void );

    void                            RemovePodcastsDownloadThread( void );
    void                            AddPodcastsDownloadItems( guPodcastItemArray * items );
    void                            RemovePodcastDownloadItems( guPodcastItemArray * items );

    void                            UpdatedTracks( int updatedby, const guTrackArray * tracks );
    void                            UpdatedTrack( int updatedby, const guTrack * track );

    guDBusNotify *                  GetNotifyObject( void ) { return m_NotifySrv; }

    guDbPodcasts *                  GetPodcastsDb( void )
    {
        if( !m_DbPodcasts )
            m_DbPodcasts = new guDbPodcasts( guPATH_PODCASTS_DBNAME );
         return m_DbPodcasts;
    }

    void                            CreateCopyToMenu( wxMenu * menu );

    void                            CopyToThreadFinished( void );
    int                             VisiblePanels( void ) { return m_VisiblePanels; }

    wxArrayString                   PortableDeviceVolumeNames( void ) { return m_VolumeMonitor->GetMountNames(); }

    guLyricSearchEngine *           LyricSearchEngine( void ) { return m_LyricSearchEngine; }

    int                             AddGauge( const wxString &label, const bool showpercent = true );
    void                            RemoveGauge( const int gaugeid );
    void                            OnGaugePulse( wxCommandEvent &event ) { m_MainStatusBar->Pulse( event.GetInt() ); }
    void                            OnGaugeSetMax( wxCommandEvent &event ) { m_MainStatusBar->SetTotal( event.GetInt(), event.GetExtraLong() ); }
    void                            OnGaugeUpdate( wxCommandEvent &event ) { m_MainStatusBar->SetValue( event.GetInt(), event.GetExtraLong() ); }
    void                            OnGaugeRemove( wxCommandEvent &event ) { m_MainStatusBar->RemoveGauge( event.GetInt() ); }

    void                            SaveCollections( void );
    void                            GetCollectionsCoverNames( wxArrayString &covernames );

    guMediaViewer *                 FindCollectionMediaViewer( const wxString &uniqueid );
    int                             GetMediaViewerIndex( guMediaViewer * mediaviewer );
    guMediaCollection *             FindCollection( const wxString &uniqueid );

    void                            MediaViewerCreated( const wxString &uniqueid, guMediaViewer * mediaviewer );
    void                            MediaViewerClosed( guMediaViewer * mediaviewer );

    void                            ImportFiles( guMediaViewer * mediaviewer, guTrackArray * tracks, const wxString &copytooption, const wxString &destdir );

    const guCurrentTrack *          GetCurrentTrack( void ) const { return m_PlayerPanel->GetCurrentTrack(); }

    void                            AddPendingUpdateTrack( const guTrack &track, const wxImage * image, const wxString &lyric, const int changedflags );
    void                            AddPendingUpdateTrack( const wxString &filename, const wxImage * image, const wxString &lyric, const int changedflags );
    void                            CheckPendingUpdates( const guTrack * track, const bool forcesave = false );

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
    guDbPodcasts *  m_Db;
    guMainFrame *   m_MainFrame;

  public :
    guUpdatePodcastsThread( guMainFrame * mainframe, int gaugeid );
    ~guUpdatePodcastsThread();

    virtual ExitCode Entry();

};

}

#endif
// -------------------------------------------------------------------------------- //
