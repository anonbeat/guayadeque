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
#include "MainFrame.h"

#include "AuiDockArt.h"
#include "Commands.h"
#include "ConfirmExit.h"
#include "FileRenamer.h"    // NormalizeField
#include "Images.h"
#include "LibUpdate.h"
#include "Preferences.h"
//#include "SplashWin.h"
#include "TrackChangeInfo.h"
#include "TaskBar.h"
#include "Utils.h"
#include "Version.h"

#include <wx/event.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/datetime.h>

// The default update podcasts timeout is 15 minutes
#define guPODCASTS_UPDATE_TIMEOUT   ( 15 * 60 * 1000 )


// -------------------------------------------------------------------------------- //
guMainFrame::guMainFrame( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache )
{
//	wxBoxSizer *    MainFrameSizer;
//	wxPanel *       MultiPanel;
//	wxPanel*        FileSysPanel;
//	wxBoxSizer*     MultiSizer;
	guConfig *      Config;

    //
    // Init the Config Object
    //
    Config = ( guConfig * ) guConfig::Get();
    if( !Config )
    {
        guLogError( wxT( "Could not open the guayadeque configuration object" ) );
        return;
    }

//    //
//    // Init the Database Object
//    //
//    m_Db = new guDbLibrary( wxGetHomeDir() + wxT( "/.guayadeque/guayadeque.db" ) );
//    if( !m_Db )
//    {
//        guLogError( wxT( "Could not open the guayadeque database" ) );
//        return;
//    }
//
//    m_DbCache = new guDbCache( wxGetHomeDir() + wxT( "/.guayadeque/cache.db" ) );
//    if( !m_DbCache )
//    {
//        guLogError( wxT( "Could not open the guayadeque cache database" ) );
//        return;
//    }
//
//    m_DbCache->SetDbCache();
    m_Db = db;
    m_DbCache = dbcache;

    //
    m_Db->SetLibPath( Config->ReadAStr( wxT( "LibPath" ),
                                      wxGetHomeDir() + wxT( "/Music" ),
                                      wxT( "LibPaths" ) ) );

    m_LibUpdateThread = NULL;
    m_LibCleanThread = NULL;
    m_UpdatePodcastsTimer = NULL;
    m_DownloadThread = NULL;
    m_NotifySrv = NULL;

    m_SelCount = 0;
    m_SelLength = 0;
    m_SelSize = 0;

    //
    m_LibPanel = NULL;
    m_PlayerPlayList = NULL;
    m_RadioPanel = NULL;
    m_LastFMPanel = NULL;
    m_LyricsPanel = NULL;
    m_PlayListPanel = NULL;
    m_PodcastsPanel = NULL;
    m_PlayerVumeters = NULL;
    m_AlbumBrowserPanel = NULL;
    m_FileBrowserPanel = NULL;

    //
    wxImage TaskBarIcon( guImage( guIMAGE_INDEX_guayadeque_taskbar ) );
    TaskBarIcon.ConvertAlphaToMask();
    m_AppIcon.CopyFromBitmap( TaskBarIcon );

    // Load the preconfigured layouts from config file
    LoadLayouts();

    //
    // guMainFrame GUI components
    //
    wxPoint MainWindowPos;
    MainWindowPos.x = Config->ReadNum( wxT( "MainWindowPosX" ), 1, wxT( "Positions" ) );
    MainWindowPos.y = Config->ReadNum( wxT( "MainWindowPosY" ), 1, wxT( "Positions" ) );
    wxSize MainWindowSize;
    MainWindowSize.x = Config->ReadNum( wxT( "MainWindowSizeWidth" ), 800, wxT( "Positions" ) );
    MainWindowSize.y = Config->ReadNum( wxT( "MainWindowSizeHeight" ), 600, wxT( "Positions" ) );
    Create( parent, wxID_ANY, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION "-" ID_GUAYADEQUE_REVISION ),
                MainWindowPos, MainWindowSize, wxDEFAULT_FRAME_STYLE );

    m_AuiManager.SetManagedWindow( this );
    m_AuiManager.SetArtProvider( new guAuiDockArt() );
    m_AuiManager.SetFlags( wxAUI_MGR_ALLOW_FLOATING |
                           wxAUI_MGR_TRANSPARENT_DRAG |
                           wxAUI_MGR_TRANSPARENT_HINT );
    wxAuiDockArt * AuiDockArt = m_AuiManager.GetArtProvider();
    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );
    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DSHADOW ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVEBORDER ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DSHADOW ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_GRADIENT_TYPE,
            wxAUI_GRADIENT_VERTICAL );

    if( Config->ReadBool( wxT( "LoadDefaultLayouts" ), false, wxT( "General" ) ) )
    {
        Config->WriteBool( wxT( "LoadDefaultLayouts" ), false, wxT( "General" ) );
        Config->SetIgnoreLayouts( true );
    }

    m_VisiblePanels = Config->ReadNum( wxT( "MainVisiblePanels" ), guPANEL_MAIN_VISIBLE_DEFAULT, wxT( "Positions" ) );
    if( Config->GetIgnoreLayouts() )
        m_VisiblePanels = guPANEL_MAIN_VISIBLE_DEFAULT;
    guLogMessage( wxT( "%08X" ), m_VisiblePanels );

	m_MainStatusBar = new guStatusBar( this );
	SetStatusBar(  m_MainStatusBar );
	//MainFrameSizer = new wxBoxSizer( wxVERTICAL );
	SetStatusText( _( "Welcome to guayadeque " ) );
	SetStatusBarPane( 0 );

    //
    m_PlayerVumeters = new guPlayerVumeters( this );

    m_PlayerFilters = new guPlayerFilters( this, m_Db );
	m_AuiManager.AddPane( m_PlayerVumeters, wxAuiPaneInfo().Name( wxT( "PlayerVumeters" ) ).Caption( _( "VU Meters" ) ).
        DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 20, 20 ).
        CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
        Bottom().Layer( 0 ).Row( 3 ).Position( 0 ).Hide() );

	m_AuiManager.AddPane( m_PlayerFilters, wxAuiPaneInfo().Name( wxT( "PlayerFilters" ) ).Caption( _( "Filters" ) ).
        DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 50, 50 ).
        CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
        Bottom().Layer( 0 ).Row( 1 ).Position( 0 ) );

    m_PlayerPlayList = new guPlayerPlayList( this, m_Db );

	m_AuiManager.AddPane( m_PlayerPlayList, wxAuiPaneInfo().Name( wxT( "PlayerPlayList" ) ).
        DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 100, 100 ).
        CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
        Bottom().Layer( 0 ).Row( 2 ).Position( 0 ) );



	m_PlayerPanel = new guPlayerPanel( this, m_Db, m_PlayerPlayList->GetPlayListCtrl(), m_PlayerFilters );

	m_PlayerPlayList->SetPlayerPanel( m_PlayerPanel );
	m_PlayerPanel->SetPlayerVumeters( m_PlayerVumeters );

	m_AuiManager.AddPane( m_PlayerPanel, wxAuiPaneInfo().Name( wxT( "PlayerPanel" ) ).
        CloseButton( false ).DestroyOnClose( false ).
        CaptionVisible( false ).
        CenterPane().Layer( 0 ).Row( 0 ).Position( 0 ) );


    CreateMenu();

	//m_CatNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_CatNotebook = new guAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON );

    m_NBPerspective = Config->ReadStr( wxT( "NotebookLayout" ), wxEmptyString, wxT( "Positions" ) );
    if( !Config->GetIgnoreLayouts() && !m_NBPerspective.IsEmpty() )
    {
        LoadTabsPerspective( m_NBPerspective );
    }
    else
    {
        wxCommandEvent ShowEvent;
        ShowEvent.SetInt( 1 );

        // Library Page
        if( m_VisiblePanels & guPANEL_MAIN_LIBRARY )
        {
            //m_LibPanel = new guLibPanel( m_CatNotebook, m_Db, m_PlayerPanel );
            //m_CatNotebook->AddPage( m_LibPanel, _( "Library" ), true );
            OnViewLibrary( ShowEvent );
        }
        else
            m_LibPanel = NULL;

        // Radio Page
        if( m_VisiblePanels & guPANEL_MAIN_RADIOS )
        {
            //m_RadioPanel = new guRadioPanel( m_CatNotebook, m_Db, m_PlayerPanel );
            //m_CatNotebook->AddPage( m_RadioPanel, _( "Radio" ), false );
            OnViewRadio( ShowEvent );
        }
        else
            m_RadioPanel = NULL;

        // LastFM Info Panel
        if( m_VisiblePanels & guPANEL_MAIN_LASTFM )
        {
            //m_LastFMPanel = new guLastFMPanel( m_CatNotebook, m_Db, m_DbCache, m_PlayerPanel );
            //m_CatNotebook->AddPage( m_LastFMPanel, _( "Last.fm" ), false );
            OnViewLastFM( ShowEvent );
        }
        else
            m_LastFMPanel = NULL;

        // Lyrics Panel
        if( m_VisiblePanels & guPANEL_MAIN_LYRICS )
        {
            //m_LyricsPanel = new guLyricsPanel( m_CatNotebook, m_Db );
            //m_CatNotebook->AddPage( m_LyricsPanel, _( "Lyrics" ), false );
            OnViewLyrics( ShowEvent );
        }
        else
            m_LyricsPanel = NULL;

        // PlayList Page
        if( m_VisiblePanels & guPANEL_MAIN_PLAYLISTS )
        {
            //m_PlayListPanel = new guPlayListPanel( m_CatNotebook, m_Db, m_PlayerPanel );
            //m_CatNotebook->AddPage( m_PlayListPanel, _( "PlayLists" ), false );
            OnViewPlayLists( ShowEvent );
        }
        else
            m_PlayListPanel = NULL;

        // Podcasts Page
        if( m_VisiblePanels & guPANEL_MAIN_PODCASTS )
        {
            //m_PodcastsPanel = new guPodcastPanel( m_CatNotebook, m_Db, this, m_PlayerPanel );
            //m_CatNotebook->AddPage( m_PodcastsPanel, _( "Podcasts" ), false );
            OnViewPodcasts( ShowEvent );
        }
        else
            m_PodcastsPanel = NULL;

        // Album Browser Page
        if( m_VisiblePanels & guPANEL_MAIN_ALBUMBROWSER )
        {
            //m_AlbumBrowserPanel = new guAlbumBrowser( m_CatNotebook, m_Db, m_PlayerPanel );
            //m_CatNotebook->AddPage( m_AlbumBrowserPanel, _( "Browser" ), false );
            OnViewAlbumBrowser( ShowEvent );
        }
        else
            m_AlbumBrowserPanel = NULL;

        // FileSystem Page
        if( m_VisiblePanels & guPANEL_MAIN_FILEBROWSER )
        {
            OnViewFileBrowser( ShowEvent );
        }
        else
            m_FileBrowserPanel = NULL;

    }

    m_AuiManager.AddPane( m_CatNotebook, wxAuiPaneInfo().Name( wxT("PlayerSelector") ).
        MinSize( 100, 100 ).
        CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
        Right().Layer( 1 ).Row( 0 ).Position( 0 ) );

//    if( !m_CatNotebook->GetPageCount() )
//    {
//        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
//        PaneInfo.Hide();
//    }

    //m_AuiManager.Update();
    wxString Perspective = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "Positions" ) );
    if( !Config->GetIgnoreLayouts() && !Perspective.IsEmpty() )
    {
        m_AuiManager.LoadPerspective( Perspective, true );
    }
    else
    {
        Perspective = wxT( "layout2|name=PlayerVumeters;caption=" );
        Perspective += _( "VU Meters" );
        Perspective += wxT( ";state=2099198;dir=3;layer=0;row=3;pos=0;prop=100000;bestw=20;besth=20;minw=20;minh=20;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=331;floath=249|" );
        Perspective += wxT( "name=PlayerFilters;caption=" );
        Perspective += _( "Filters" );
        Perspective += wxT( ";state=2099196;dir=3;layer=0;row=1;pos=0;prop=100000;bestw=135;besth=68;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        Perspective += wxT( "name=PlayerPlayList;caption=" );
        Perspective += _( "Now Playing" );
        Perspective += wxT( ";state=2099196;dir=3;layer=0;row=2;pos=0;prop=100000;bestw=100;besth=100;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        Perspective += wxT( "name=PlayerPanel;caption=;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=308;besth=167;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        Perspective += wxT( "name=PlayerSelector;caption=;state=2099196;dir=2;layer=1;row=0;pos=0;prop=100000;bestw=100;besth=100;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        Perspective += wxT( "dock_size(3,0,1)=86|" );
        Perspective += wxString::Format( wxT( "dock_size(3,0,2)=%i|" ), MainWindowSize.y - 86 - 230 );
        Perspective += wxT( "dock_size(5,0,0)=310|" );
        Perspective += wxString::Format( wxT( "dock_size(2,1,0)=%i|" ), MainWindowSize.x - 315 );
        m_AuiManager.LoadPerspective( Perspective, true );
        //m_AuiManager.Update();
    }

    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
    if( !PaneInfo.IsShown() )
    {
        m_VisiblePanels = m_VisiblePanels & ( guPANEL_MAIN_PLAYERPLAYLIST |
                                              guPANEL_MAIN_PLAYERFILTERS |
                                              guPANEL_MAIN_PLAYERVUMETERS );

        // Reset the Menu entry for all elements
        m_ViewLibrary->Check( false );
        m_ViewLibTextSearch->Enable( false );
        m_ViewLibLabels->Enable( false );
        m_ViewLibGenres->Enable( false );
        m_ViewLibArtists->Enable( false );
        m_ViewLibAlbums->Enable( false );
        m_ViewLibYears->Enable( false );
        m_ViewLibRatings->Enable( false );
        m_ViewLibPlayCounts->Enable( false );
        m_ViewLibComposers->Enable( false );
        m_ViewLibAlbumArtists->Enable( false );

        m_ViewRadios->Check( false );
        m_ViewRadTextSearch->Enable( false );
        m_ViewRadLabels->Enable( false );
        m_ViewRadGenres->Enable( false );

        m_ViewLastFM->Check( false );

        m_ViewLyrics->Check( false );

        m_ViewPlayLists->Check( false );
        m_ViewPLTextSearch->Check( false );

        m_ViewPodcasts->Check( false );
        m_ViewPodChannels->Enable( m_ViewPodcasts->IsChecked() );
        m_ViewPodDetails->Enable( m_ViewPodcasts->IsChecked() );

        m_ViewAlbumBrowser->Check( false );

        m_ViewFileBrowser->Check( false );
    }

    m_CurrentPage = m_CatNotebook->GetPage( m_CatNotebook->GetSelection() );

    //
    m_TaskBarIcon = NULL;
    if( Config->ReadBool( wxT( "ShowTaskBarIcon" ), true, wxT( "General" ) ) )
    {
        CreateTaskBarIcon();
    }


    m_DBusServer = new guDBusServer( NULL );
    guDBusServer::Set( m_DBusServer );
    if( !m_DBusServer )
    {
        guLogError( wxT( "Could not create the dbus server object" ) );
    }

    // Init the MPRIS object
    m_MPRIS = new guMPRIS( m_DBusServer, m_PlayerPanel );
    if( !m_MPRIS )
    {
        guLogError( wxT( "Could not create the mpris dbus object" ) );
    }

    // Init the MMKeys object
    m_MMKeys = new guMMKeys( m_DBusServer, m_PlayerPanel );
    if( !m_MMKeys )
    {
        guLogError( wxT( "Could not create the mmkeys dbus object" ) );
    }

    m_GSession = new guGSession( m_DBusServer );
    if( !m_GSession )
    {
        guLogError( wxT( "Could not create the gnome session dbus object" ) );
    }

    m_NotifySrv = new guDBusNotify( m_DBusServer );
    if( !m_NotifySrv )
    {
        guLogMessage( wxT( "Could not create the notify dbus object" ) );
    }

    m_PlayerPanel->SetNotifySrv( m_NotifySrv );
    //m_DBusServer->Run();

    //
	Connect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );

    Connect( ID_MENU_UPDATE_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLibrary ), NULL, this );
    Connect( ID_MENU_UPDATE_LIBRARYFORCED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnForceUpdateLibrary ), NULL, this );
    Connect( ID_MENU_LIBRARY_ADD_PATH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAddLibraryPath ), NULL, this );
    Connect( ID_MENU_UPDATE_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdatePodcasts ), NULL, this );
    Connect( ID_MENU_UPDATE_COVERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateCovers ), NULL, this );
    Connect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ), NULL, this );

    Connect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ), NULL, this );
    Connect( ID_LIBRARY_DOCLEANDB, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::DoLibraryClean ), NULL, this );
    Connect( ID_LIBRARY_CLEANFINISHED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryCleanFinished ), NULL, this );

    Connect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ), NULL, this );
    Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    //Connect( wxEVT_ICONIZE, wxIconizeEventHandler( guMainFrame::OnIconizeWindow ), NULL, this );
    Connect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );

    Connect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ), NULL, this );
    Connect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ), NULL, this );


	Connect( ID_MAINFRAME_SELECT_TRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectTrack ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectAlbum ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectArtist ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_YEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectYear ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectGenre ), NULL, this );

	Connect( ID_GENRE_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGenreSetSelection ), NULL, this );
	Connect( ID_ARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnArtistSetSelection ), NULL, this );
	Connect( ID_ALBUMARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumArtistSetSelection ), NULL, this );
	Connect( ID_ALBUM_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumSetSelection ), NULL, this );

    Connect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlay ), NULL, this );
    Connect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStop ), NULL, this );
    Connect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextTrack ), NULL, this );
    Connect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevTrack ), NULL, this );
    Connect( ID_PLAYERPANEL_NEXTALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextAlbum ), NULL, this );
    Connect( ID_PLAYERPANEL_PREVALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevAlbum ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSmartPlay ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRandomize ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_REPEATPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_REPEATTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_UPDATETITLE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerPlayListUpdateTitle ), NULL, this );
    Connect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ), NULL, this );

    Connect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );

    Connect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

    Connect( ID_MENU_LAYOUT_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCreateNewLayout ), NULL, this );
    Connect( ID_MENU_LAYOUT_LOAD, ID_MENU_LAYOUT_LOAD + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadLayout ), NULL, this );
    Connect( ID_MENU_LAYOUT_DELETE, ID_MENU_LAYOUT_DELETE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnDeleteLayout ), NULL, this );

    Connect( ID_MENU_VIEW_PLAYER_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_PLAYER_FILTERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_PLAYER_VUMETERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_PLAYER_SELECTOR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );

    Connect( ID_MENU_VIEW_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLibrary ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_TEXTSEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_ARTISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_COMPOSERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_ALBUMARTISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_ALBUMS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_YEARS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_RATINGS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_LIB_PLAYCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );

    Connect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ), NULL, this );
    Connect( ID_MENU_VIEW_RAD_TEXTSEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_RAD_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_RAD_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );

    Connect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ), NULL, this );
    Connect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ), NULL, this );

    Connect( ID_MENU_VIEW_PLAYLISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPlayLists ), NULL, this );
    Connect( ID_MENU_VIEW_PL_TEXTSEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListShowPanel ), NULL, this );

    Connect( ID_MENU_VIEW_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPodcasts ), NULL, this );
    Connect( ID_MENU_VIEW_POD_CHANNELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_POD_DETAILS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );

    Connect( ID_MENU_VIEW_ALBUMBROWSER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewAlbumBrowser ), NULL, this );

    Connect( ID_MENU_VIEW_FILEBROWSER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFileBrowser ), NULL, this );

    Connect( ID_MENU_VIEW_FULLSCREEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFullScreen ), NULL, this );
    Connect( ID_MENU_VIEW_STATUSBAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewStatusBar ), NULL, this );


    Connect( ID_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ), NULL, this );
    Connect( ID_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ), NULL, this );
    Connect( ID_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ), NULL, this );
    Connect( ID_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ), NULL, this );

    Connect( ID_PLAYLIST_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListUpdated ), NULL, this );

    Connect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guMainFrame::OnPodcastItemUpdated ), NULL, this );
    Connect( ID_MAINFRAME_REMOVEPODCASTTHREAD, wxCommandEventHandler( guMainFrame::OnRemovePodcastThread ), NULL, this );

    m_AuiManager.Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guMainFrame::OnMainPaneClose ), NULL, this );

    m_CatNotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );
    m_CatNotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler( guMainFrame::OnPageClosed ), NULL, this );

    Connect( ID_MAINFRAME_UPDATE_SELINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateSelInfo ), NULL, this );
    Connect( ID_MAINFRAME_REQUEST_CURRENTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRequestCurrentTrack ), NULL, this );
//    Connect( ID_MAINFRAME_SET_RADIOSTATIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetRadioStations ), NULL, this );
//    Connect( ID_MAINFRAME_SET_PLAYLISTTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPlayListTracks ), NULL, this );
//    Connect( ID_MAINFRAME_SET_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPodcasts ), NULL, this );

    //Connect( wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEventHandler( guMainFrame::OnSysColorChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guMainFrame::~guMainFrame()
{
    Disconnect( ID_MENU_UPDATE_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLibrary ), NULL, this );
    Disconnect( ID_MENU_UPDATE_LIBRARYFORCED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnForceUpdateLibrary ), NULL, this );
    Disconnect( ID_MENU_LIBRARY_ADD_PATH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAddLibraryPath ), NULL, this );
    Disconnect( ID_MENU_UPDATE_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdatePodcasts ), NULL, this );
    Disconnect( ID_MENU_UPDATE_COVERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateCovers ), NULL, this );
    Disconnect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ), NULL, this );

    Disconnect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ), NULL, this );
    Disconnect( ID_LIBRARY_DOCLEANDB, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::DoLibraryClean ), NULL, this );
    Disconnect( ID_LIBRARY_CLEANFINISHED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryCleanFinished ), NULL, this );

    Disconnect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ), NULL, this );
    Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    //Disconnect( wxEVT_ICONIZE, wxIconizeEventHandler( guMainFrame::OnIconizeWindow ), NULL, this );
    Disconnect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );

    Disconnect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ), NULL, this );
    Disconnect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ), NULL, this );

	Disconnect( ID_MAINFRAME_SELECT_TRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectTrack ), NULL, this );
	Disconnect( ID_MAINFRAME_SELECT_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectAlbum ), NULL, this );
	Disconnect( ID_MAINFRAME_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectArtist ), NULL, this );
	Disconnect( ID_MAINFRAME_SELECT_YEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectYear ), NULL, this );
	Disconnect( ID_MAINFRAME_SELECT_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectGenre ), NULL, this );

	Disconnect( ID_GENRE_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGenreSetSelection ), NULL, this );
	Disconnect( ID_ARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnArtistSetSelection ), NULL, this );
	Disconnect( ID_ALBUMARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumArtistSetSelection ), NULL, this );
	Disconnect( ID_ALBUM_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumSetSelection ), NULL, this );

    Disconnect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlay ), NULL, this );
    Disconnect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStop ), NULL, this );
    Disconnect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextTrack ), NULL, this );
    Disconnect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevTrack ), NULL, this );
    Disconnect( ID_PLAYERPANEL_NEXTALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextAlbum ), NULL, this );
    Disconnect( ID_PLAYERPANEL_PREVALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevAlbum ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSmartPlay ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRandomize ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_REPEATPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_REPEATTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_UPDATETITLE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerPlayListUpdateTitle ), NULL, this );
    Disconnect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ), NULL, this );

    Disconnect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );

    Disconnect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

    Disconnect( ID_MENU_LAYOUT_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCreateNewLayout ), NULL, this );
    Disconnect( ID_MENU_LAYOUT_LOAD, ID_MENU_LAYOUT_LOAD + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadLayout ), NULL, this );
    Disconnect( ID_MENU_LAYOUT_DELETE, ID_MENU_LAYOUT_DELETE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnDeleteLayout ), NULL, this );

    Disconnect( ID_MENU_VIEW_PLAYER_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_PLAYER_FILTERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_PLAYER_SELECTOR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );

    Disconnect( ID_MENU_VIEW_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLibrary ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_TEXTSEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_ARTISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_COMPOSERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_ALBUMARTISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_ALBUMS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_YEARS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_RATINGS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_LIB_PLAYCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryShowPanel ), NULL, this );



    Disconnect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ), NULL, this );
    Disconnect( ID_MENU_VIEW_RAD_TEXTSEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_RAD_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_RAD_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );

    Disconnect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ), NULL, this );
    Disconnect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ), NULL, this );
    Disconnect( ID_MENU_VIEW_PLAYLISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPlayLists ), NULL, this );

    Disconnect( ID_MENU_VIEW_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPodcasts ), NULL, this );
    Disconnect( ID_MENU_VIEW_POD_CHANNELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_POD_DETAILS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );

    Disconnect( ID_MENU_VIEW_ALBUMBROWSER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewAlbumBrowser ), NULL, this );

    Disconnect( ID_MENU_VIEW_FILEBROWSER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFileBrowser ), NULL, this );

    Disconnect( ID_MENU_VIEW_FULLSCREEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFullScreen ), NULL, this );
    Disconnect( ID_MENU_VIEW_STATUSBAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewStatusBar ), NULL, this );

    Disconnect( ID_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ), NULL, this );
    Disconnect( ID_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ), NULL, this );
    Disconnect( ID_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ), NULL, this );
    Disconnect( ID_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ), NULL, this );

    Disconnect( ID_PLAYLIST_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListUpdated ), NULL, this );

    Disconnect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guMainFrame::OnPodcastItemUpdated ), NULL, this );
    Disconnect( ID_MAINFRAME_REMOVEPODCASTTHREAD, wxCommandEventHandler( guMainFrame::OnRemovePodcastThread ), NULL, this );

    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guMainFrame::OnMainPaneClose ), NULL, this );

    m_CatNotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );
    m_CatNotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler( guMainFrame::OnPageClosed ), NULL, this );

    Disconnect( ID_MAINFRAME_UPDATE_SELINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateSelInfo ), NULL, this );
    Disconnect( ID_MAINFRAME_REQUEST_CURRENTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRequestCurrentTrack ), NULL, this );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        //Config->WriteNum( wxT( "PlayerSashPos" ), m_PlayerSplitter->GetSashPosition(), wxT( "Positions" ) );
        wxPoint MainWindowPos = GetPosition();
        Config->WriteNum( wxT( "MainWindowPosX" ), MainWindowPos.x, wxT( "Positions" ) );
        Config->WriteNum( wxT( "MainWindowPosY" ), MainWindowPos.y, wxT( "Positions" ) );
        wxSize MainWindowSize = GetSize();
        Config->WriteNum( wxT( "MainWindowSizeWidth" ), MainWindowSize.x, wxT( "Positions" ) );
        Config->WriteNum( wxT( "MainWindowSizeHeight" ), MainWindowSize.y, wxT( "Positions" ) );

        Config->WriteNum( wxT( "MainVisiblePanels" ), m_VisiblePanels, wxT( "Positions" ) );
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
        PaneInfo.Caption( _( "Now Playing" ) );
        Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
        Config->WriteStr( wxT( "NotebookLayout" ), m_CatNotebook->SavePerspective(), wxT( "Positions" ) );

        Config->WriteBool( wxT( "ShowFullScreen" ), IsFullScreen() , wxT( "General" ) );
        Config->WriteBool( wxT( "ShowStatusBar" ), m_MainStatusBar->IsShown() , wxT( "General" ) );
    }

    if( m_LibUpdateThread )
    {
        m_LibUpdateThread->Pause();
        m_LibUpdateThread->Delete();
    }

    if( m_LibCleanThread )
    {
        m_LibCleanThread->Pause();
        m_LibCleanThread->Delete();
    }

    if( m_TaskBarIcon )
        delete m_TaskBarIcon;

    // destroy the mpris object
    if( m_MPRIS )
    {
        delete m_MPRIS;
    }

    if( m_GSession )
    {
        delete m_GSession;
    }

    if( m_MMKeys )
    {
        delete m_MMKeys;
    }

    if( m_NotifySrv )
    {
        m_PlayerPanel->SetNotifySrv( NULL );
        delete m_NotifySrv;
    }

    if( m_DBusServer )
    {
        delete m_DBusServer;
    }

    m_AuiManager.UnInit();

//    if( m_Db )
//    {
//        m_Db->Close();
//        delete m_Db;
//    }
//
//    if( m_DbCache )
//    {
//        m_DbCache->Close();
//        delete m_DbCache;
//    }
}

extern void wxClearGtkSystemObjects();

//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnSysColorChanged( wxSysColourChangedEvent &event )
//{
//    guLogMessage( wxT( "Clear syssettings cache" ) );
//    wxClearGtkSystemObjects();
//    event.Skip();
//}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateMenu()
{
	wxMenuBar * MenuBar;
	wxMenuItem * MenuItem;
	wxMenu *     SubMenu;

    guConfig * Config = ( guConfig * ) guConfig::Get();

	m_MainMenu = new wxMenu();

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LIBRARY_ADD_PATH, _("&Add Directory" ), _( "Add a directory to the Library paths" ), wxITEM_NORMAL );
	m_MainMenu->Append( MenuItem );

	m_MainMenu->AppendSeparator();

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_UPDATE_LIBRARY, _("&Update Library" ), _( "Update all new songs from the directories configured" ), wxITEM_NORMAL );
	m_MainMenu->Append( MenuItem );

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_UPDATE_LIBRARYFORCED, _("&Rescan Library" ), _( "Update all songs from the directories configured" ), wxITEM_NORMAL );
	m_MainMenu->Append( MenuItem );

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_UPDATE_PODCASTS, _("Update P&odcasts" ), _( "Update the podcasts added" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
	m_MainMenu->Append( MenuItem );

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_UPDATE_COVERS, _("Update Covers"), _( "Try to download all missing covers" ), wxITEM_NORMAL );
//    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_ ) );
	m_MainMenu->Append( MenuItem );

	m_MainMenu->AppendSeparator();

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_PREFERENCES, _( "&Preferences" ), _( "Change the options of the application" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_pref_general ) );
	m_MainMenu->Append( MenuItem );

	m_MainMenu->AppendSeparator();

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_QUIT, _( "&Quit" ), _( "Exits the application" ), wxITEM_NORMAL );
	m_MainMenu->Append( MenuItem );

	MenuBar = new wxMenuBar( 0 );
	MenuBar->Append( m_MainMenu, _( "&Library" ) );

    m_MainMenu = new wxMenu();

    MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_CREATE, _( "Save Layout" ), _( "Saves the current layout" ) );
    m_MainMenu->Append( MenuItem );

    m_LayoutLoadMenu = new wxMenu();
    m_LayoutDelMenu = new wxMenu();

    int Count = m_LayoutName.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_LOAD + Index, m_LayoutName[ Index ], _( "Load this user defined layout" ) );
            m_LayoutLoadMenu->Append( MenuItem );
            MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DELETE + Index, m_LayoutName[ Index ], _( "Delete this user defined layout" ) );
            m_LayoutDelMenu->Append( MenuItem );
        }
    }

    m_MainMenu->AppendSubMenu( m_LayoutLoadMenu, _( "Load Layout" ), _( "Set current view from a user defined layout" ) );
    m_MainMenu->AppendSubMenu( m_LayoutDelMenu, _( "Delete Layout" ), _( "Delete a user defined layout" ) );
    if( !Count )
    {
        MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DUMMY, _( "No layouts defined" ), _( "Load this user defined layout" ) );
        m_LayoutLoadMenu->Append( MenuItem );
        MenuItem->Enable( false );
        MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DUMMY, _( "No layouts defined" ), _( "Load this user defined layout" ) );
        m_LayoutDelMenu->Append( MenuItem );
        MenuItem->Enable( false );
    }

    m_MainMenu->AppendSeparator();

    m_ViewPlayerPlayList = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_PLAYER_PLAYLIST, _( "Player PlayList" ), _( "Show/Hide the player playlist panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewPlayerPlayList );
    m_ViewPlayerPlayList->Check( m_VisiblePanels & guPANEL_MAIN_PLAYERPLAYLIST );

    m_ViewPlayerFilters = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_PLAYER_FILTERS, _( "Player Filters" ), _( "Show/Hide the player filters panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewPlayerFilters );
    m_ViewPlayerFilters->Check( m_VisiblePanels & guPANEL_MAIN_PLAYERFILTERS );

    m_ViewPlayerVumeters = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_PLAYER_VUMETERS, _( "VU Meters" ), _( "Show/Hide the player vumeter" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewPlayerVumeters );
    m_ViewPlayerVumeters->Check( m_VisiblePanels & guPANEL_MAIN_PLAYERVUMETERS );

    SubMenu = new wxMenu();

    m_ViewLibrary = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIBRARY, _( "&Library" ), _( "Show/Hide the library panel" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibrary );
    m_ViewLibrary->Check( m_VisiblePanels & guPANEL_MAIN_LIBRARY );

    SubMenu->AppendSeparator();

    m_ViewLibTextSearch = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_TEXTSEARCH, _( "Text Search" ), _( "Show/Hide the library text search" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibTextSearch );
    m_ViewLibTextSearch->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_TEXTSEARCH ) );
    m_ViewLibTextSearch->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibLabels = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_LABELS, _( "Labels" ), _( "Show/Hide the library labels" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibLabels );
    m_ViewLibLabels->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_LABELS ) );
    m_ViewLibLabels->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibGenres = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_GENRES, _( "Genres" ), _( "Show/Hide the library genres" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibGenres );
    m_ViewLibGenres->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_GENRES ) );
    m_ViewLibGenres->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibArtists = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_ARTISTS, _( "Artists" ), _( "Show/Hide the library artists" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibArtists );
    m_ViewLibArtists->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_ARTISTS ) );
    m_ViewLibArtists->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibComposers = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_COMPOSERS, _( "Composers" ), _( "Show/Hide the library composers" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibComposers );
    m_ViewLibComposers->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_COMPOSERS ) );
    m_ViewLibComposers->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibAlbumArtists = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_ALBUMARTISTS, _( "Album Artist" ), _( "Show/Hide the library album artist" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibAlbumArtists );
    m_ViewLibAlbumArtists->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_ALBUMARTISTS ) );
    m_ViewLibAlbumArtists->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibAlbums = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_ALBUMS, _( "Albums" ), _( "Show/Hide the library albums" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibAlbums );
    m_ViewLibAlbums->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_ALBUMS ) );
    m_ViewLibAlbums->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibYears = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_YEARS, _( "Years" ), _( "Show/Hide the library years" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibYears );
    m_ViewLibYears->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_YEARS ) );
    m_ViewLibYears->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibRatings = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_RATINGS, _( "Ratings" ), _( "Show/Hide the library ratings" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibRatings );
    m_ViewLibRatings->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_RATINGS ) );
    m_ViewLibRatings->Enable( m_ViewLibrary->IsChecked() );

    m_ViewLibPlayCounts = new wxMenuItem( SubMenu, ID_MENU_VIEW_LIB_PLAYCOUNT, _( "Play Counts" ), _( "Show/Hide the library play counts" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewLibPlayCounts );
    m_ViewLibPlayCounts->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_PLAYCOUNT ) );
    m_ViewLibPlayCounts->Enable( m_ViewLibrary->IsChecked() );

    m_MainMenu->AppendSubMenu( SubMenu, _( "Library" ), _( "Set the library visible panels" ) );


    SubMenu = new wxMenu();

    m_ViewRadios = new wxMenuItem( SubMenu, ID_MENU_VIEW_RADIO, _( "&Radio" ), _( "Show/Hide the radio panel" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewRadios );
    m_ViewRadios->Check( m_VisiblePanels & guPANEL_MAIN_RADIOS );

    SubMenu->AppendSeparator();

    m_ViewRadTextSearch = new wxMenuItem( SubMenu, ID_MENU_VIEW_RAD_TEXTSEARCH, _( "Text Search" ), _( "Show/Hide the radio text search" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewRadTextSearch );
    m_ViewRadTextSearch->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_TEXTSEARCH ) );
    m_ViewRadTextSearch->Enable( m_ViewRadios->IsChecked() );

    m_ViewRadLabels = new wxMenuItem( SubMenu, ID_MENU_VIEW_RAD_LABELS, _( "Labels" ), _( "Show/Hide the radio labels" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewRadLabels );
    m_ViewRadLabels->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_LABELS ) );
    m_ViewRadLabels->Enable( m_ViewRadios->IsChecked() );

    m_ViewRadGenres = new wxMenuItem( SubMenu, ID_MENU_VIEW_RAD_GENRES, _( "Genres" ), _( "Show/Hide the radio genres" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewRadGenres );
    m_ViewRadGenres->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_GENRES ) );
    m_ViewRadGenres->Enable( m_ViewRadios->IsChecked() );

    m_MainMenu->AppendSubMenu( SubMenu, _( "&Radio" ), _( "Set the radio visible panels" ) );


    m_ViewLastFM = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_LASTFM, _( "Last.&fm" ), _( "Show/Hide the Last.fm panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewLastFM );
    m_ViewLastFM->Check( m_VisiblePanels & guPANEL_MAIN_LASTFM );

    m_ViewLyrics = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_LYRICS, _( "L&yrics" ), _( "Show/Hide the lyrics panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewLyrics );
    m_ViewLyrics->Check( m_VisiblePanels & guPANEL_MAIN_LYRICS );


    SubMenu = new wxMenu();

    m_ViewPlayLists = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_PLAYLISTS, _( "&PlayLists" ), _( "Show/Hide the playlists panel" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewPlayLists );
    m_ViewPlayLists->Check( m_VisiblePanels & guPANEL_MAIN_PLAYLISTS );

    m_ViewPLTextSearch = new wxMenuItem( SubMenu, ID_MENU_VIEW_PL_TEXTSEARCH, _( "Text Search" ), _( "Show/Hide the playlists text search" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewPLTextSearch );
    m_ViewPLTextSearch->Check( m_PlayListPanel && m_PlayListPanel->IsPanelShown( guPANEL_PLAYLIST_TEXTSEARCH ) );
    m_ViewPLTextSearch->Enable( m_ViewPlayLists->IsChecked() );

    m_MainMenu->AppendSubMenu( SubMenu, _( "&PlayLists" ), _( "Set the playlists visible panels" ) );

    SubMenu = new wxMenu();

    m_ViewPodcasts = new wxMenuItem( SubMenu, ID_MENU_VIEW_PODCASTS, _( "P&odcasts" ), _( "Show/Hide the podcasts panel" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewPodcasts );
    m_ViewPodcasts->Check( m_VisiblePanels & guPANEL_MAIN_PODCASTS );

    SubMenu->AppendSeparator();

    m_ViewPodChannels = new wxMenuItem( SubMenu, ID_MENU_VIEW_POD_CHANNELS, _( "Channels" ), _( "Show/Hide the podcasts channels" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewPodChannels );
    m_ViewPodChannels->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_CHANNELS ) );
    m_ViewPodChannels->Enable( m_ViewPodcasts->IsChecked() );

    m_ViewPodDetails = new wxMenuItem( SubMenu, ID_MENU_VIEW_POD_DETAILS, _( "Details" ), _( "Show/Hide the podcasts details" ), wxITEM_CHECK );
    SubMenu->Append( m_ViewPodDetails );
    m_ViewPodDetails->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_DETAILS ) );
    m_ViewPodDetails->Enable( m_ViewPodcasts->IsChecked() );

    m_MainMenu->AppendSubMenu( SubMenu, _( "&Podcasts" ), _( "Set the podcasts visible panels" ) );

    m_ViewAlbumBrowser = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_ALBUMBROWSER, _( "Browser" ), _( "Show/Hide the album browser panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewAlbumBrowser );
    m_ViewAlbumBrowser->Check( m_VisiblePanels & guPANEL_MAIN_ALBUMBROWSER );

    m_ViewFileBrowser = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_FILEBROWSER, _( "Files" ), _( "Show/Hide the file browser panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewFileBrowser );
    m_ViewFileBrowser->Check( m_VisiblePanels & guPANEL_MAIN_FILEBROWSER );

    m_MainMenu->AppendSeparator();

    m_ViewFullScreen = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_FULLSCREEN, _( "Full Screen" ), _( "Show/Restore the main window in full screen" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewFullScreen );
    m_ViewFullScreen->Check( Config->ReadBool( wxT( "ShowFullScreen" ), false, wxT( "General" ) ) );

    m_ViewStatusBar = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_STATUSBAR, _( "StatusBar" ), _( "Show/Hide the statusbar" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewStatusBar );
    m_ViewStatusBar->Check( Config->ReadBool( wxT( "ShowStatusBar" ), true, wxT( "General" ) ) );

    MenuBar->Append( m_MainMenu, _( "&View" ) );

    m_MainMenu = new wxMenu();
    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_NEXTTRACK, _( "&Next Track" ), _( "Play the next track in the playlist" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_forward ) );
    m_MainMenu->Append( MenuItem );
    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_PREVTRACK, _( "&Prev. Track" ), _( "Play the previous track in the playlist" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_backward ) );
    m_MainMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_NEXTALBUM, _( "Next Album" ), _( "Play the next album in the playlist" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_forward ) );
    m_MainMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_PREVALBUM, _( "Prev. Album" ), _( "Play the previous album in the playlist" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_forward ) );
    m_MainMenu->Append( MenuItem );

    m_MainMenu->AppendSeparator();

    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_PLAY, _( "&Play" ), _( "Play or Pause the current track in the playlist" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playback_start ) );
    m_MainMenu->Append( MenuItem );
    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_STOP, _( "&Stop" ), _( "Stop the current played track" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playback_ ) );
    m_MainMenu->Append( MenuItem );
    m_MainMenu->AppendSeparator();
    m_PlaySmartMenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_SMARTPLAY, _( "&Smart Mode" ), _( "Update playlist based on Last.fm statics" ), wxITEM_CHECK );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search_engine ) );
    m_MainMenu->Append( m_PlaySmartMenuItem );
    m_PlaySmartMenuItem->Check( m_PlayerPanel->GetPlaySmart() );

    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_RANDOMPLAY, _( "R&andomize" ), _( "Randomize the playlist" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_shuffle ) );
    m_MainMenu->Append( MenuItem );
    m_LoopPlayListMenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_REPEATPLAYLIST, _( "&Repeat Playlist" ), _( "Repeat the tracks in the playlist" ), wxITEM_CHECK );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_repeat ) );
    m_MainMenu->Append( m_LoopPlayListMenuItem );
    m_LoopPlayListMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );

    m_LoopTrackMenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_REPEATTRACK, _( "&Repeat Track" ), _( "Repeat the current track in the playlist" ), wxITEM_CHECK );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_repeat ) );
    m_MainMenu->Append( m_LoopTrackMenuItem );
    m_LoopTrackMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );

    MenuBar->Append( m_MainMenu, _( "&Control" ) );

    m_MainMenu = new wxMenu();
    MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_ABOUT, _( "&About" ), _( "Show information about guayadeque music player" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_volume_high ) );
    m_MainMenu->Append( MenuItem );
    MenuBar->Append( m_MainMenu, _( "&Help" ) );

	SetMenuBar( MenuBar );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPreferences( wxCommandEvent &event )
{
    guPrefDialog * PrefDialog = new guPrefDialog( this, m_Db, event.GetInt() ? event.GetInt() : wxNOT_FOUND );
    if( PrefDialog )
    {
        if( PrefDialog->ShowModal() == wxID_OK )
        {
            PrefDialog->SaveSettings();

            guConfig * Config = ( guConfig * ) guConfig::Get();
            if( !m_TaskBarIcon && Config->ReadBool( wxT( "ShowTaskBarIcon" ), true, wxT( "General" ) ) )
            {
                CreateTaskBarIcon();
            }
            else if( m_TaskBarIcon && !Config->ReadBool( wxT( "ShowTaskBarIcon" ), true, wxT( "General" ) ) )
            {
                m_TaskBarIcon->RemoveIcon();
                delete m_TaskBarIcon;
                m_TaskBarIcon = NULL;
            }

            Config->SendConfigChangedEvent();
            m_Db->ConfigChanged();
        }
        PrefDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCloseWindow( wxCloseEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        // If the icon
        if( m_TaskBarIcon &&
            Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "General" ) ) &&
            Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) )
        {
            if( event.CanVeto() )
            {
                Show( false );
                return;
            }
        }
        else if( Config->ReadBool( wxT( "ShowCloseConfirm" ), true, wxT( "General" ) ) )
        {
            guExitConfirmDlg * ExitConfirmDlg = new guExitConfirmDlg( this );
            if( ExitConfirmDlg )
            {
                int Result = ExitConfirmDlg->ShowModal();
                if( ExitConfirmDlg->GetConfirmChecked() )
                {
                    Config->WriteBool( wxT( "ShowCloseConfirm" ), false, wxT( "General" ) );
                }
                ExitConfirmDlg->Destroy();

                if( Result != wxID_OK )
                    return;
            }
        }
    }
    event.Skip();
}

//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnIconizeWindow( wxIconizeEvent &event )
//{
////    guConfig * Config = ( guConfig * ) guConfig::Get();
////    if( Config )
////    {
////        // If the icon
////        if( m_TaskBarIcon &&
////            Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "General" ) ) &&
////            Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) )
////        {
////            if( event.IsIconized() )
////            {
////                if( IsShown() )
////                    Show( false );
////            }
////            else
////            {
////                if( !IsShown() )
////                    Show( true );
////            }
////        }
////    }
//    event.Skip();
//}

// -------------------------------------------------------------------------------- //
void guMainFrame::LibraryCleanFinished( wxCommandEvent &event )
{
    m_LibCleanThread = NULL;
    //m_Db->LoadCache();

    if( m_LibPanel )
        m_LibPanel->ReloadControls( event );

    if( m_AlbumBrowserPanel )
        m_AlbumBrowserPanel->LibraryUpdated();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::DoLibraryClean( wxCommandEvent &event )
{
    if( m_LibCleanThread )
    {
        m_LibCleanThread->Pause();
        m_LibCleanThread->Delete();
    }

    m_LibCleanThread = new guLibCleanThread( m_Db );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::LibraryUpdated( wxCommandEvent &event )
{
//    guLogMessage( wxT( "Library Updated Event fired" ) );
    //m_Db->DoCleanUp();

//    if( m_LibPanel )
//        m_LibPanel->ReloadControls( event );
//
//    if( m_AlbumBrowserPanel )
//        m_AlbumBrowserPanel->LibraryUpdated();

    m_LibUpdateThread = NULL;

    DoLibraryClean( event );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateTrack( wxCommandEvent &event )
{
    guTrack * Track = ( guTrack * ) event.GetClientData();
    if( Track )
    {
        SetTitle( Track->m_SongName + wxT( " / " ) + Track->m_ArtistName +
            wxT( " - Guayadeque Music Player " ID_GUAYADEQUE_VERSION "-" ID_GUAYADEQUE_REVISION ) );

        if( m_TaskBarIcon )
        {
            m_TaskBarIcon->SetIcon( m_AppIcon, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION
                                                    "-" ID_GUAYADEQUE_REVISION "\r" ) +
                                               Track->m_ArtistName + wxT( "\n" ) +
                                               Track->m_SongName );
        }
    }
    else
    {
        SetTitle( wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION "-" ID_GUAYADEQUE_REVISION ) );

        if( m_TaskBarIcon )
        {
            m_TaskBarIcon->SetIcon( m_AppIcon, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION "-" ID_GUAYADEQUE_REVISION ) );
        }
    }


    if( m_LastFMPanel )
    {
        m_LastFMPanel->OnUpdatedTrack( event );
    }
    if( m_LyricsPanel )
    {
        m_LyricsPanel->OnUpdatedTrack( event );
    }
    if( m_MPRIS )
    {
        m_MPRIS->OnPlayerTrackChange();
    }

    if( event.GetClientData() )
    {
        delete ( guTrack * ) event.GetClientData();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerStatusChanged( wxCommandEvent &event )
{
    //guLogError( wxT( "Player Status Change Fired" ) );
    if( m_MPRIS )
    {
        m_MPRIS->OnPlayerStatusChange();
    }

    if( m_PlayerPanel )
    {
        m_PlaySmartMenuItem->Check( m_PlayerPanel->GetPlaySmart() );
        m_LoopPlayListMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );
        m_LoopTrackMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerTrackListChanged( wxCommandEvent &event )
{
    //guLogError( wxT( "Player TrackList Change Fired" ) );
    if( m_MPRIS )
    {
        m_MPRIS->OnTrackListChange();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerCapsChanged( wxCommandEvent &event )
{
    //guLogError( wxT( "Player TrackList Change Fired" ) );
    if( m_MPRIS )
    {
        m_MPRIS->OnPlayerCapsChange();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnAudioScrobbleUpdate( wxCommandEvent &event )
{
    if( m_MainStatusBar )
    {
        m_MainStatusBar->SetAudioScrobbleService( event.GetInt() == 0 );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnQuit( wxCommandEvent& WXUNUSED(event) )
{
    Close( true );
}


// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateCovers( wxCommandEvent &WXUNUSED( event ) )
{
    int GaugeId = m_MainStatusBar->AddGauge( _( "Covers" ), false );
    //guLogMessage( wxT( "Created gauge id %u" ), GaugeId );
    guUpdateCoversThread * UpdateCoversThread = new guUpdateCoversThread( m_Db, GaugeId );
    if( UpdateCoversThread )
    {
        UpdateCoversThread->Create();
        UpdateCoversThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        UpdateCoversThread->Run();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateLibrary( wxCommandEvent& WXUNUSED(event) )
{
    if( m_LibUpdateThread )
        return;
    int gaugeid = m_MainStatusBar->AddGauge( _( "Library" ), false );
    m_LibUpdateThread = new guLibUpdateThread( m_Db, gaugeid );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnForceUpdateLibrary( wxCommandEvent &event )
{
    if( m_LibUpdateThread )
        return;
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( wxT( "LastUpdate" ), 0, wxT( "General" ) );
    int gaugeid = m_MainStatusBar->AddGauge( _( "Library" ), false );
    m_LibUpdateThread = new guLibUpdateThread( m_Db, gaugeid );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnAddLibraryPath( wxCommandEvent &event )
{
    wxDirDialog * DirDialog = new wxDirDialog( this, _( "Select library path" ), wxGetHomeDir() );
    if( DirDialog )
    {
        if( DirDialog->ShowModal() == wxID_OK )
        {
            wxString PathValue = DirDialog->GetPath();
            if( !PathValue.IsEmpty() )
            {
                guConfig * Config = ( guConfig * ) guConfig::Get();
                wxArrayString LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );

                if( !PathValue.EndsWith( wxT( "/" ) ) )
                    PathValue += '/';

                //guLogMessage( wxT( "LibPaths: '%s'" ), LibPaths[ 0 ].c_str() );
                //guLogMessage( wxT( "Add Path: '%s'" ), PathValue.c_str() );
                //guLogMessage( wxT( "Exists  : %i" ), m_Db->PathExists( PathValue ) );
                //guLogMessage( wxT( "Index   : %i" ), LibPaths.Index( PathValue ) );

                if( ( m_Db->PathExists( PathValue ) == wxNOT_FOUND ) &&
                    !CheckFileLibPath( LibPaths, PathValue ) )
                {

                    LibPaths.Add( PathValue );
                    Config->WriteAStr( wxT( "LibPath" ), LibPaths, wxT( "LibPaths" ) );

                    //event.SetId( ID_MENU_UPDATE_LIBRARY );
                    //AddPendingEvent( event );
                    if( !m_LibUpdateThread )
                    {
                        int gaugeid = m_MainStatusBar->AddGauge( _( "Library" ), false );
                        m_LibUpdateThread = new guLibUpdateThread( m_Db, gaugeid, PathValue );
                    }
                    else
                    {
                        wxMessageBox( _( "The library is already updating" ), _( "Add Directory" ), wxOK | wxICON_EXCLAMATION  );
                    }
                }
                else
                {
                    wxMessageBox( _( "This Path is already in the library" ),
                        _( "Adding path error" ),
                        wxICON_EXCLAMATION | wxOK  );
                }
            }
        }
        DirDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdatePodcasts( wxCommandEvent& WXUNUSED(event) )
{
    UpdatePodcasts();
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnPlay( wxCommandEvent &event )
{
    if( m_PlayerPanel )
      m_PlayerPanel->OnPlayButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnStop( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnStopButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnNextTrack( wxCommandEvent &event )
{
    event.SetInt( 0 );
    if( m_PlayerPanel )
        m_PlayerPanel->OnNextTrackButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnPrevTrack( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnPrevTrackButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnNextAlbum( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnNextAlbumButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnPrevAlbum( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnPrevAlbumButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnSmartPlay( wxCommandEvent &event )
{
    if( m_PlayerPanel )
    {
        m_PlayerPanel->OnSmartPlayButtonClick( event );
        m_PlaySmartMenuItem->Check( m_PlayerPanel->GetPlaySmart() );
        m_LoopPlayListMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );
        m_LoopTrackMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );
    }
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnRandomize( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnRandomPlayButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnRepeat( wxCommandEvent &event )
{
    if( m_PlayerPanel )
    {
        int RepeatMode = m_PlayerPanel->GetPlayLoop();
        if( event.GetId() == ID_PLAYER_PLAYLIST_REPEATPLAYLIST )
        {
            if( RepeatMode != guPLAYER_PLAYLOOP_PLAYLIST )
                RepeatMode = guPLAYER_PLAYLOOP_PLAYLIST;
            else
                RepeatMode = guPLAYER_PLAYLOOP_NONE;
        }
        else if( event.GetId() == ID_PLAYER_PLAYLIST_REPEATTRACK )
        {
            if( RepeatMode != guPLAYER_PLAYLOOP_TRACK )
                RepeatMode = guPLAYER_PLAYLOOP_TRACK;
            else
                RepeatMode = guPLAYER_PLAYLOOP_NONE;
        }

        m_PlayerPanel->SetPlayLoop( RepeatMode );

        //m_PlayerPanel->OnRepeatPlayButtonClick( event );
        m_PlaySmartMenuItem->Check( m_PlayerPanel->GetPlaySmart() );
        m_LoopPlayListMenuItem->Check( RepeatMode == guPLAYER_PLAYLOOP_PLAYLIST );
        m_LoopTrackMenuItem->Check( RepeatMode == guPLAYER_PLAYLOOP_TRACK );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnAbout( wxCommandEvent &event )
{
    guSplashFrame * SplashFrame = new guSplashFrame( this, 10000 );
    if( !SplashFrame )
    {
        guLogError( wxT( "Could not create the splash screen window" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerPlayListUpdateTitle( wxCommandEvent &event )
{
    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
    if( PaneInfo.IsOk() )
    {
        //guLogMessage( wxT( "Updating PlayListTitle..." ) );
        guPlayList * PlayList = m_PlayerPlayList->GetPlayListCtrl();
        PaneInfo.Caption( _( "Now Playing" ) +
            wxString::Format( wxT( ":  %i / %i    ( %s )" ),
                PlayList->GetCurItem() + 1,
                PlayList->GetCount(),
                PlayList->GetLengthStr().c_str() ) );
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCopyTracksTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = ( guTrackArray * ) event.GetClientData();
    if( Tracks )
    {
        if( Tracks->Count() )
        {
            wxDirDialog * DirDialog = new wxDirDialog( this,
                _( "Select destination directory" ), wxGetHomeDir(), wxDD_DIR_MUST_EXIST );
            if( DirDialog )
            {
                if( DirDialog->ShowModal() == wxID_OK )
                {
                    int GaugeId = m_MainStatusBar->AddGauge( _( "Copy To..." ) );
                    guCopyToDirThread * CopyToDirThread = new guCopyToDirThread( DirDialog->GetPath().c_str(),
                        Tracks, GaugeId );
                    if( !CopyToDirThread )
                    {
                        guLogError( wxT( "Could not create the CopyTo thread object" ) );
                        delete Tracks;
                    }
                }
                else
                {
                    delete Tracks;
                }
                DirDialog->Destroy();
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateLabels( wxCommandEvent &event )
{
    if( m_LibPanel )
    {
        m_LibPanel->UpdateLabels();
    }
}

//// -------------------------------------------------------------------------------- //
//int GetPageIndex( wxAuiNotebook * Notebook, wxPanel * Page )
//{
//    int index;
//    int count = Notebook->GetPageCount();
//    for( index = 0; index < count; index++ )
//    {
//        if( Notebook->GetPage( index ) == Page )
//            return index;
//    }
//    return -1;
//}

// -------------------------------------------------------------------------------- //
void guMainFrame::CheckShowNotebook( void )
{
    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
    if( !PaneInfo.IsShown() )
    {
        PaneInfo.Show();
////        if( !m_NBPerspective.IsEmpty() )
////            m_CatNotebook->LoadPerspective( m_NBPerspective );
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CheckHideNotebook( void )
{
    if( !m_CatNotebook->GetPageCount() )
    {
//        m_NBPerspective = m_CatNotebook->SavePerspective();
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
        PaneInfo.Hide();
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::RemoveTabPanel( wxPanel * panel )
{
    int PageIndex = m_CatNotebook->GetPageIndex( panel );
    if( PageIndex != wxNOT_FOUND )
    {
        if( m_CatNotebook->GetPageCount() > 1 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }
        else
        {
            guConfig * Config = ( guConfig * ) guConfig::Get();
            Config->WriteStr( wxT( "LastTabsSize" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
            wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
            PaneInfo.Hide();
            m_AuiManager.Update();
        }
    }
    else
    {
        guLogError( wxT( "Asked to remove a panel that is not in the tab control" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::InsertTabPanel( wxPanel * panel, const int index, const wxString &label )
{
    int PageIndex = m_CatNotebook->GetPageIndex( panel );
    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
    if( !PaneInfo.IsShown() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_AuiManager.LoadPerspective( Config->ReadStr( wxT( "LastTabsSize" ), wxEmptyString, wxT( "Positions" ) ) );
        wxWindow * OldPage = m_CatNotebook->GetPage( 0 );
        // Was hidden
        if( PageIndex == wxNOT_FOUND )
        {
            m_CatNotebook->InsertPage( wxMin( index, ( int ) m_CatNotebook->GetPageCount() ), panel, label, true );
            int OldIndex = m_CatNotebook->GetPageIndex( OldPage );
            m_CatNotebook->RemovePage( OldIndex );
        }
        PaneInfo.Show();
//        guConfig * Config = ( guConfig * ) Config::Get();
//        Config->WriteStr( wxT( "LastTabsSize" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
        m_AuiManager.Update();
    }
    else
    {
        m_CatNotebook->InsertPage( wxMin( index, ( int ) m_CatNotebook->GetPageCount() ), panel, label, true );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLibrary( wxCommandEvent &event )
{
    bool IsEnabled = event.IsChecked();

    if( IsEnabled )
    {
        if( !m_LibPanel )
            m_LibPanel = new guLibPanel( m_CatNotebook, m_Db, m_PlayerPanel );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( 0, m_LibPanel, _( "Library" ), true );
        InsertTabPanel( m_LibPanel, 0, _( "Library" ) );

        m_VisiblePanels |= guPANEL_MAIN_LIBRARY;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_LibPanel );
//        if( PageIndex != wxNOT_FOUND )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();
        RemoveTabPanel( m_LibPanel );

        m_VisiblePanels ^= guPANEL_MAIN_LIBRARY;
    }
    m_CatNotebook->Refresh();

    m_ViewLibrary->Check( m_VisiblePanels & guPANEL_MAIN_LIBRARY );

    m_ViewLibTextSearch->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_TEXTSEARCH ) );
    m_ViewLibTextSearch->Enable( IsEnabled );

    m_ViewLibLabels->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_LABELS ) );
    m_ViewLibLabels->Enable( IsEnabled );

    m_ViewLibGenres->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_GENRES ) );
    m_ViewLibGenres->Enable( IsEnabled );

    m_ViewLibArtists->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_ARTISTS ) );
    m_ViewLibArtists->Enable( IsEnabled );

    m_ViewLibComposers->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_COMPOSERS ) );
    m_ViewLibComposers->Enable( IsEnabled );

    m_ViewLibAlbumArtists->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_ALBUMARTISTS ) );
    m_ViewLibAlbumArtists->Enable( IsEnabled );

    m_ViewLibAlbums->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_ALBUMS ) );
    m_ViewLibAlbums->Enable( IsEnabled );

    m_ViewLibYears->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_YEARS ) );
    m_ViewLibYears->Enable( IsEnabled );

    m_ViewLibRatings->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_RATINGS ) );
    m_ViewLibRatings->Enable( IsEnabled );

    m_ViewLibPlayCounts->Check( m_LibPanel && m_LibPanel->IsPanelShown( guPANEL_LIBRARY_PLAYCOUNT ) );
    m_ViewLibPlayCounts->Enable( IsEnabled );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLibraryShowPanel( wxCommandEvent &event )
{
    unsigned int PanelId = 0;

    switch( event.GetId() )
    {
        case ID_MENU_VIEW_LIB_TEXTSEARCH :
            PanelId = guPANEL_LIBRARY_TEXTSEARCH;
            m_ViewLibTextSearch->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_LABELS :
            PanelId = guPANEL_LIBRARY_LABELS;
            m_ViewLibLabels->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_GENRES :
            PanelId = guPANEL_LIBRARY_GENRES;
            m_ViewLibGenres->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_ARTISTS :
            PanelId = guPANEL_LIBRARY_ARTISTS;
            m_ViewLibArtists->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_ALBUMS :
            PanelId = guPANEL_LIBRARY_ALBUMS;
            m_ViewLibAlbums->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_YEARS :
            PanelId = guPANEL_LIBRARY_YEARS;
            m_ViewLibYears->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_RATINGS :
            PanelId = guPANEL_LIBRARY_RATINGS;
            m_ViewLibRatings->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_PLAYCOUNT :
            PanelId = guPANEL_LIBRARY_PLAYCOUNT;
            m_ViewLibPlayCounts->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_COMPOSERS :
            PanelId = guPANEL_LIBRARY_COMPOSERS;
            m_ViewLibComposers->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_LIB_ALBUMARTISTS :
            PanelId = guPANEL_LIBRARY_ALBUMARTISTS;
            m_ViewLibAlbumArtists->Check( event.IsChecked() );
            break;
    }

    if( PanelId && m_LibPanel )
        m_LibPanel->ShowPanel( PanelId, event.IsChecked() );

}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewRadio( wxCommandEvent &event )
{
//	guConfig *      Config = ( guConfig * ) guConfig::Get();
//	Config->WriteBool( wxT( "ShowRadio" ), event.IsChecked(), wxT( "ViewPanels" ) );
    bool IsEnabled = event.IsChecked();

    if( IsEnabled )
    {
        if( !m_RadioPanel )
            m_RadioPanel = new guRadioPanel( m_CatNotebook, m_Db, m_PlayerPanel );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( wxMin( 1, m_CatNotebook->GetPageCount() ), m_RadioPanel, _( "Radio" ), true );
        InsertTabPanel( m_RadioPanel, 1, _( "Radio" ) );

        m_VisiblePanels |= guPANEL_MAIN_RADIOS;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_RadioPanel );
//        if( PageIndex >= 0 )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();
        RemoveTabPanel( m_RadioPanel );
        m_VisiblePanels ^= guPANEL_MAIN_RADIOS;
    }
    m_CatNotebook->Refresh();

    m_ViewRadios->Check( m_VisiblePanels & guPANEL_MAIN_RADIOS );

    m_ViewRadTextSearch->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_TEXTSEARCH ) );
    m_ViewRadTextSearch->Enable( IsEnabled );

    m_ViewRadLabels->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_LABELS ) );
    m_ViewRadLabels->Enable( IsEnabled );

    m_ViewRadGenres->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_GENRES ) );
    m_ViewRadGenres->Enable( IsEnabled );

}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnRadioShowPanel( wxCommandEvent &event )
{
    unsigned int PanelId = 0;

    switch( event.GetId() )
    {
        case ID_MENU_VIEW_RAD_TEXTSEARCH :
            PanelId = guPANEL_RADIO_TEXTSEARCH;
            m_ViewRadTextSearch->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_RAD_LABELS :
            PanelId = guPANEL_RADIO_LABELS;
            m_ViewRadLabels->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_RAD_GENRES :
            PanelId = guPANEL_RADIO_GENRES;
            m_ViewRadGenres->Check( event.IsChecked() );
            break;

//        case ID_MENU_VIEW_RAD_STATIONS :
//            PanelId = guPANEL_RADIO_STATIONS;
//            m_ViewLibTracks->Check( event.IsChecked() );
//            break;

    }

    if( PanelId && m_RadioPanel )
        m_RadioPanel->ShowPanel( PanelId, event.IsChecked() );

}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayListShowPanel( wxCommandEvent &event )
{
    unsigned int PanelId = 0;

    switch( event.GetId() )
    {
        case ID_MENU_VIEW_PL_TEXTSEARCH :
            PanelId = guPANEL_PLAYLIST_TEXTSEARCH ;
            m_ViewPLTextSearch->Check( event.IsChecked() );
            break;

    }

    if( PanelId && m_PlayListPanel )
        m_PlayListPanel->ShowPanel( PanelId, event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPodcastsShowPanel( wxCommandEvent &event )
{
    unsigned int PanelId = 0;

    switch( event.GetId() )
    {
        case ID_MENU_VIEW_POD_CHANNELS :
            PanelId = guPANEL_PODCASTS_CHANNELS;
            m_ViewPodChannels->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_POD_DETAILS :
            PanelId = guPANEL_PODCASTS_DETAILS;
            m_ViewPodDetails->Check( event.IsChecked() );
            break;

    }

    if( PanelId && m_PodcastsPanel )
        m_PodcastsPanel->ShowPanel( PanelId, event.IsChecked() );

}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLastFM( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        if( !m_LastFMPanel )
            m_LastFMPanel = new guLastFMPanel( m_CatNotebook, m_Db, m_DbCache, m_PlayerPanel );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( wxMin( 2, m_CatNotebook->GetPageCount() ), m_LastFMPanel, _( "Last.fm" ), true );
        InsertTabPanel( m_LastFMPanel, 2, _( "Last.fm" ) );

        m_VisiblePanels |= guPANEL_MAIN_LASTFM;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_LastFMPanel );
//        if( PageIndex >= 0 )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();

        RemoveTabPanel( m_LastFMPanel );

        m_VisiblePanels ^= guPANEL_MAIN_LASTFM;
    }
    m_CatNotebook->Refresh();

    m_ViewLastFM->Check( m_VisiblePanels & guPANEL_MAIN_LASTFM );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLyrics( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        if( !m_LyricsPanel )
            m_LyricsPanel = new guLyricsPanel( m_CatNotebook, m_Db );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( wxMin( 3, m_CatNotebook->GetPageCount() ), m_LyricsPanel, _( "Lyrics" ), true );
        InsertTabPanel( m_LyricsPanel, 3, _( "Lyrics" ) );

        m_VisiblePanels |= guPANEL_MAIN_LYRICS;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_LyricsPanel );
//        if( PageIndex >= 0 )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();
        RemoveTabPanel( m_LyricsPanel );

        m_VisiblePanels ^= guPANEL_MAIN_LYRICS;
    }
    m_CatNotebook->Refresh();

    m_ViewLyrics->Check( m_VisiblePanels & guPANEL_MAIN_LYRICS );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPodcasts( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        if( !m_PodcastsPanel )
            m_PodcastsPanel = new guPodcastPanel( m_CatNotebook, m_Db, this, m_PlayerPanel );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( wxMin( 5, m_CatNotebook->GetPageCount() ), m_PodcastsPanel, _( "Podcasts" ), true );
        InsertTabPanel( m_PodcastsPanel, 5, _( "Podcasts" ) );

        m_VisiblePanels |= guPANEL_MAIN_PODCASTS;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_PodcastsPanel );
//        if( PageIndex >= 0 )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();

        RemoveTabPanel( m_PodcastsPanel );

        m_VisiblePanels ^= guPANEL_MAIN_PODCASTS;
    }
    m_CatNotebook->Refresh();

    m_ViewPodcasts->Check( m_VisiblePanels & guPANEL_MAIN_PODCASTS );

    m_ViewPodChannels->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_CHANNELS ) );
    m_ViewPodChannels->Enable( m_ViewPodcasts->IsChecked() );

    m_ViewPodDetails->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_DETAILS ) );
    m_ViewPodDetails->Enable( m_ViewPodcasts->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewAlbumBrowser( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        if( !m_AlbumBrowserPanel )
            m_AlbumBrowserPanel = new guAlbumBrowser( m_CatNotebook, m_Db, m_PlayerPanel );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( wxMin( 6, m_CatNotebook->GetPageCount() ), m_AlbumBrowserPanel, _( "Browser" ), true );
        InsertTabPanel( m_AlbumBrowserPanel, 6, _( "Browser" ) );

        m_VisiblePanels |= guPANEL_MAIN_ALBUMBROWSER;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_AlbumBrowserPanel );
//        if( PageIndex >= 0 )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();
        RemoveTabPanel( m_AlbumBrowserPanel );

        m_VisiblePanels ^= guPANEL_MAIN_ALBUMBROWSER;
    }
    m_CatNotebook->Refresh();

    m_ViewAlbumBrowser->Check( m_VisiblePanels & guPANEL_MAIN_ALBUMBROWSER );

}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewFileBrowser( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        if( !m_FileBrowserPanel )
            m_FileBrowserPanel = new guFileBrowser( m_CatNotebook, m_Db, m_PlayerPanel );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( wxMin( 7, m_CatNotebook->GetPageCount() ), m_FileBrowserPanel, _( "Files" ), true );
        InsertTabPanel( m_FileBrowserPanel, 7, _( "Files" ) );

        m_VisiblePanels |= guPANEL_MAIN_FILEBROWSER;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_FileBrowserPanel );
//        if( PageIndex >= 0 )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();
        RemoveTabPanel( m_FileBrowserPanel );
        m_VisiblePanels ^= guPANEL_MAIN_FILEBROWSER;
    }
    m_CatNotebook->Refresh();

    m_ViewFileBrowser->Check( m_VisiblePanels & guPANEL_MAIN_FILEBROWSER );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewFullScreen( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    bool IsFull = event.IsChecked();

    if( IsFull )
    {
        // Save the normal perspective
        Config->WriteNum( wxT( "MainVisiblePanels" ), m_VisiblePanels, wxT( "Positions" ) );
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
        PaneInfo.Caption( _( "Now Playing" ) );
        Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
//        Config->WriteStr( wxT( "NotebookLayout" ), m_CatNotebook->SavePerspective(), wxT( "Positions" ) );

        ShowFullScreen( IsFull, wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION );

        // Restore the previous full screen layout
        m_VisiblePanels = Config->ReadNum( wxT( "MainVisiblePanelsFull" ), guPANEL_MAIN_VISIBLE_DEFAULT, wxT( "Positions" ) );
//        wxString NBLayout = Config->ReadStr( wxT( "NotebookLayoutFull" ), wxEmptyString, wxT( "Positions" ) );
//        if( !NBLayout.IsEmpty() )
//            LoadTabsPerspective( NBLayout );

        wxString Perspective = Config->ReadStr( wxT( "LastLayoutFull" ), wxEmptyString, wxT( "Positions" ) );
        if( !Perspective.IsEmpty() )
            m_AuiManager.LoadPerspective( Perspective, true );
    }
    else
    {
        // Save the full screen layout
        Config->WriteNum( wxT( "MainVisiblePanelsFull" ), m_VisiblePanels, wxT( "Positions" ) );
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
        PaneInfo.Caption( _( "Now Playing" ) );
        Config->WriteStr( wxT( "LastLayoutFull" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
//        Config->WriteStr( wxT( "NotebookLayoutFull" ), m_CatNotebook->SavePerspective(), wxT( "Positions" ) );

        ShowFullScreen( IsFull, wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION );

        // Restore the normal layout
        m_VisiblePanels = Config->ReadNum( wxT( "MainVisiblePanels" ), guPANEL_MAIN_VISIBLE_DEFAULT, wxT( "Positions" ) );
        Config->WriteNum( wxT( "MainVisiblePanels" ), m_VisiblePanels, wxT( "Positions" ) );
//        wxString NBLayout = Config->ReadStr( wxT( "NotebookLayout" ), wxEmptyString, wxT( "Positions" ) );
//        if( !NBLayout.IsEmpty() )
//            LoadTabsPerspective( NBLayout );

        wxString Perspective = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "Positions" ) );
        if( !Perspective.IsEmpty() )
            m_AuiManager.LoadPerspective( Perspective, true );
    }

//    m_AuiManager.Update();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewStatusBar( wxCommandEvent &event )
{
    m_MainStatusBar->Show( event.IsChecked() );
    m_AuiManager.Update();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPlayLists( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        if( !m_PlayListPanel )
            m_PlayListPanel = new guPlayListPanel( m_CatNotebook, m_Db, m_PlayerPanel );

//        CheckShowNotebook();
//
//        m_CatNotebook->InsertPage( wxMin( 4, m_CatNotebook->GetPageCount() ), m_PlayListPanel, _( "PlayLists" ), true );
        InsertTabPanel( m_PlayListPanel, 4, _( "PlayLists" ) );

        m_VisiblePanels |= guPANEL_MAIN_PLAYLISTS;
    }
    else
    {
//        int PageIndex = m_CatNotebook->GetPageIndex( m_PlayListPanel );
//        if( PageIndex >= 0 )
//        {
//            m_CatNotebook->RemovePage( PageIndex );
//        }
//
//        CheckHideNotebook();
        RemoveTabPanel( m_PlayListPanel );

        m_VisiblePanels ^= guPANEL_MAIN_PLAYLISTS;
    }
    m_CatNotebook->Refresh();

    m_ViewPlayLists->Check( m_VisiblePanels & guPANEL_MAIN_PLAYLISTS );

    m_ViewPLTextSearch->Check( m_PlayListPanel && m_PlayListPanel->IsPanelShown( guPANEL_PLAYLIST_TEXTSEARCH ) );
    m_ViewPLTextSearch->Enable( m_ViewPlayLists->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectTrack( wxCommandEvent &event )
{
    if( event.GetExtraLong() == guTRACK_TYPE_PODCAST )
    {
        if( m_PodcastsPanel )
        {
            int PaneIndex = m_CatNotebook->GetPageIndex( m_PodcastsPanel );
            if( PaneIndex != wxNOT_FOUND )
            {
                m_CatNotebook->SetSelection( PaneIndex );
            }
            m_PodcastsPanel->SelectPodcast( event.GetInt() );
        }
    }
    else
    {
        if( m_LibPanel )
        {
            int PaneIndex = m_CatNotebook->GetPageIndex( m_LibPanel );
            if( PaneIndex != wxNOT_FOUND )
            {
                m_CatNotebook->SetSelection( PaneIndex );
            }
            m_LibPanel->SelectTrack( event.GetInt() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectAlbum( wxCommandEvent &event )
{
    if( event.GetExtraLong() == guTRACK_TYPE_PODCAST )
    {
        if( m_PodcastsPanel )
        {
            int PaneIndex = m_CatNotebook->GetPageIndex( m_PodcastsPanel );
            if( PaneIndex != wxNOT_FOUND )
            {
                m_CatNotebook->SetSelection( PaneIndex );
            }
            m_PodcastsPanel->SelectChannel( event.GetInt() );
        }
    }
    else
    {
        if( m_LibPanel )
        {
            int PaneIndex = m_CatNotebook->GetPageIndex( m_LibPanel );
            if( PaneIndex != wxNOT_FOUND )
            {
                m_CatNotebook->SetSelection( PaneIndex );
            }
            m_LibPanel->SelectAlbum( event.GetInt() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectArtist( wxCommandEvent &event )
{
    if( event.GetExtraLong() == guTRACK_TYPE_PODCAST )
    {
        return;
    }
    else
    {
        if( m_LibPanel )
        {
            int PaneIndex = m_CatNotebook->GetPageIndex( m_LibPanel );
            if( PaneIndex != wxNOT_FOUND )
            {
                m_CatNotebook->SetSelection( PaneIndex );
            }
            m_LibPanel->SelectArtist( event.GetInt() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectYear( wxCommandEvent &event )
{
    if( m_LibPanel )
    {
        int PaneIndex = m_CatNotebook->GetPageIndex( m_LibPanel );
        if( PaneIndex != wxNOT_FOUND )
        {
            m_CatNotebook->SetSelection( PaneIndex );
        }
        m_LibPanel->SelectYear( event.GetInt() );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectGenre( wxCommandEvent &event )
{
    if( m_LibPanel )
    {
        int PaneIndex = m_CatNotebook->GetPageIndex( m_LibPanel );
        if( PaneIndex != wxNOT_FOUND )
        {
            m_CatNotebook->SetSelection( PaneIndex );
        }
        wxArrayInt Genres;
        Genres.Add( event.GetInt() );
        m_LibPanel->SelectGenres( &Genres );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGenreSetSelection( wxCommandEvent &event )
{
    wxArrayInt * genres = ( wxArrayInt * ) event.GetClientData();
    if( genres )
    {
        m_CatNotebook->SetSelection( 0 );
        m_LibPanel->SelectGenres( genres );
        delete genres;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnArtistSetSelection( wxCommandEvent &event )
{
    wxArrayInt * artists = ( wxArrayInt * ) event.GetClientData();
    if( artists )
    {
        m_CatNotebook->SetSelection( 0 );
        m_LibPanel->SelectArtists( artists );
        delete artists;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnAlbumArtistSetSelection( wxCommandEvent &event )
{
    wxArrayInt * Ids = ( wxArrayInt * ) event.GetClientData();
    if( Ids )
    {
        m_CatNotebook->SetSelection( 0 );
        m_LibPanel->SelectAlbumArtists( Ids );
        delete Ids;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnAlbumSetSelection( wxCommandEvent &event )
{
    wxArrayInt * albums = ( wxArrayInt * ) event.GetClientData();
    if( albums )
    {
        m_CatNotebook->SetSelection( 0 );
        m_LibPanel->SelectAlbums( albums );
        delete albums;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayListUpdated( wxCommandEvent &event )
{
    if( m_PlayListPanel )
        m_PlayListPanel->PlayListUpdated();

    if( m_PlayerFilters )
        m_PlayerFilters->UpdateFilters();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugePulse( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Pulse message for gauge %u" ), event.GetInt() );
    m_MainStatusBar->Pulse( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugeSetMax( wxCommandEvent &event )
{
    //guLogMessage( wxT( "SetMax message for gauge %u to %u" ), event.GetInt(), event.GetExtraLong() );
    m_MainStatusBar->SetTotal( event.GetInt(), event.GetExtraLong() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugeUpdate( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Update message for gauge %u" ), event.GetInt() );
    m_MainStatusBar->SetValue( event.GetInt(), event.GetExtraLong() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugeRemove( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Remove message for gauge %u" ), event.GetInt() );
    m_MainStatusBar->RemoveGauge( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPageChanged( wxAuiNotebookEvent& event )
{
    m_CurrentPage = m_CatNotebook->GetPage( m_CatNotebook->GetSelection() );

    OnUpdateSelInfo( event );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPageClosed( wxAuiNotebookEvent& event )
{
    wxAuiNotebook * ctrl = ( wxAuiNotebook * ) event.GetEventObject();
    int PanelId = 0;

    wxPanel * CurPage = ( wxPanel * ) ctrl->GetPage( event.GetSelection() );
    //m_CatNotebook->RemovePage( event.GetSelection() );
    RemoveTabPanel( CurPage );

    if( CurPage == m_LibPanel )
    {
        m_ViewLibrary->Check( false );
        m_ViewLibTextSearch->Enable( false );
        m_ViewLibLabels->Enable( false );
        m_ViewLibGenres->Enable( false );
        m_ViewLibArtists->Enable( false );
        m_ViewLibComposers->Enable( false );
        m_ViewLibAlbumArtists->Enable( false );
        m_ViewLibAlbums->Enable( false );
        m_ViewLibYears->Enable( false );
        m_ViewLibRatings->Enable( false );
        m_ViewLibPlayCounts->Enable( false );

        PanelId = guPANEL_MAIN_LIBRARY;
    }
    else if( CurPage == m_RadioPanel )
    {
        m_ViewRadios->Check( false );
        m_ViewRadTextSearch->Enable( false );
        m_ViewRadLabels->Enable( false );
        m_ViewRadGenres->Enable( false );

        PanelId = guPANEL_MAIN_RADIOS;
    }
    else if( CurPage == m_LastFMPanel )
    {
        m_ViewLastFM->Check( false );
        PanelId = guPANEL_MAIN_LASTFM;
    }
    else if( CurPage == m_LyricsPanel )
    {
        m_ViewLyrics->Check( false );
        PanelId = guPANEL_MAIN_LYRICS;
    }
    else if( CurPage == m_PlayListPanel )
    {
        m_ViewPlayLists->Check( false );
        PanelId = guPANEL_MAIN_PLAYLISTS;
    }
    else if( CurPage == m_PodcastsPanel )
    {
        m_ViewPodcasts->Check( false );
        m_ViewPodChannels->Enable( false  );
        m_ViewPodDetails->Enable( false );
        PanelId = guPANEL_MAIN_PODCASTS;
    }
    else if( CurPage == m_AlbumBrowserPanel )
    {
        m_ViewAlbumBrowser->Check( false );
        PanelId = guPANEL_MAIN_ALBUMBROWSER;
    }
    else if( CurPage == m_FileBrowserPanel )
    {
        m_ViewFileBrowser->Check( false );
        PanelId = guPANEL_MAIN_FILEBROWSER;
    }

    //CheckHideNotebook();
    m_VisiblePanels ^= PanelId;

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateSelInfo( wxCommandEvent &event )
{
    if( m_CurrentPage == ( wxWindow * ) m_LibPanel )
    {
        m_Db->GetTracksCounters( &m_SelCount, &m_SelLength, &m_SelSize );

        wxString SelInfo = wxString::Format( wxT( "%llu " ), m_SelCount.GetValue() );
        SelInfo += m_SelCount == 1 ? _( "track" ) : _( "tracks" );
        SelInfo += wxString::Format( wxT( ",   %s,   %s" ),
            LenToString( m_SelLength.GetLo() ).c_str(),
            SizeToString( m_SelSize.GetValue() ).c_str() );
        m_MainStatusBar->SetSelInfo( SelInfo );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_RadioPanel )
    {
        //m_Db->GetRadioCounter( &m_SelCount );
        m_RadioPanel->GetRadioCounter( &m_SelCount );
        wxString SelInfo = wxString::Format( wxT( "%llu " ), m_SelCount.GetValue() );
        SelInfo += m_SelCount == 1 ? _( "station" ) : _( "stations" );
        m_MainStatusBar->SetSelInfo( SelInfo );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_PlayListPanel )
    {
        if( m_PlayListPanel->GetPlayListCounters( &m_SelCount, &m_SelLength, &m_SelSize ) )
        {
            wxString SelInfo = wxString::Format( wxT( "%llu " ), m_SelCount.GetValue() );
            SelInfo += m_SelCount == 1 ? _( "track" ) : _( "tracks" );
            SelInfo += wxString::Format( wxT( ",   %s,   %s" ),
                LenToString( m_SelLength.GetLo() ).c_str(),
                SizeToString( m_SelSize.GetValue() ).c_str() );
            m_MainStatusBar->SetSelInfo( SelInfo );
        }
        else
        {
            m_MainStatusBar->SetSelInfo( wxEmptyString );
        }
    }
    else if( m_CurrentPage == ( wxWindow * ) m_PodcastsPanel )
    {
        m_PodcastsPanel->GetCounters( &m_SelCount, &m_SelLength, &m_SelSize );
        //m_Db->GetPodcastCounters( &m_SelCount, &m_SelLength, &m_SelSize );

        wxString SelInfo = wxString::Format( wxT( "%llu " ), m_SelCount.GetValue() );
        SelInfo += m_SelCount == 1 ? _( "podcast" ) : _( "podcasts" );
        SelInfo += wxString::Format( wxT( ",   %s,   %s" ),
            LenToString( m_SelLength.GetLo() ).c_str(),
            SizeToString( m_SelSize.GetValue() ).c_str() );
        m_MainStatusBar->SetSelInfo( SelInfo );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_FileBrowserPanel )
    {
        if( m_FileBrowserPanel->GetCounters( &m_SelCount, &m_SelLength, &m_SelSize ) )
        {
            wxString SelInfo = wxString::Format( wxT( "%llu " ), m_SelLength.GetValue() );
            SelInfo += m_SelLength == 1 ? _( "dir" ) : _( "dirs" );
            SelInfo += wxString::Format( wxT( ", %llu " ), m_SelCount.GetValue() );
            SelInfo += m_SelLength == 1 ? _( "file" ) : _( "files" );

            SelInfo += wxString::Format( wxT( ",   %s" ),
                SizeToString( m_SelSize.GetValue() ).c_str() );

            m_MainStatusBar->SetSelInfo( SelInfo );
        }
        else
        {
            m_MainStatusBar->SetSelInfo( wxEmptyString );
        }
    }
    else
    {
        //m_SelCount = wxNOT_FOUND;
        //m_SelLength = wxNOT_FOUND;
        //m_SelSize = wxNOT_FOUND;
        m_MainStatusBar->SetSelInfo( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnRequestCurrentTrack( wxCommandEvent &event )
{
    wxCommandEvent UpdateEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
    guTrack * CurTrack = new guTrack( * m_PlayerPanel->GetCurrentTrack() );
    UpdateEvent.SetClientData( CurTrack );

    if( event.GetClientData() == m_LyricsPanel )
    {
        m_LyricsPanel->OnUpdatedTrack( UpdateEvent );
    }
    else if( event.GetClientData() == m_LastFMPanel )
    {
        m_LastFMPanel->OnUpdatedTrack( UpdateEvent );
    }

    delete CurTrack;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnIdle( wxIdleEvent& WXUNUSED( event ) )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Disconnect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );

	m_MainStatusBar->Show( Config->ReadBool( wxT( "ShowStatusBar" ), true, wxT( "General" ) ) );

    // If the database need to be updated
    if( m_Db->NeedUpdate() || Config->ReadBool( wxT( "UpdateLibOnStart" ), false, wxT( "General" ) ) )
    {
        guLogMessage( wxT( "Database updating started." ) );
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED,
            m_Db->NeedUpdate() ? ID_MENU_UPDATE_LIBRARYFORCED : ID_MENU_UPDATE_LIBRARY );
        AddPendingEvent( event );
    }

    // If the Podcasts update is enable launch it...
    if( Config->ReadBool( wxT( "Update" ), true, wxT( "Podcasts" ) ) )
    {
        guLogMessage( wxT( "Updating the podcasts..." ) );
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_UPDATE_PODCASTS );
        AddPendingEvent( event );
    }

    // Add the previously pending podcasts to download
    guPodcastItemArray Podcasts;
    m_Db->GetPendingPodcasts( &Podcasts );
    if( Podcasts.Count() )
        AddPodcastsDownloadItems( &Podcasts );

    // Now we can start the dbus server
    m_DBusServer->Run();

//    // If enabled Show the Splash Screen on Startup
//    guSplashFrame * SplashFrame = NULL;
//    if( Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "General" ) ) )
//    {
//        SplashFrame = new guSplashFrame( 0 );
//        if( !SplashFrame )
//            guLogError( wxT( "Could not create splash object" ) );
//        SplashFrame->Show( true );
//        //wxYield();
//    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateTaskBarIcon( void )
{
    m_TaskBarIcon = new guTaskBarIcon( this, m_PlayerPanel );
    if( m_TaskBarIcon )
    {
        m_TaskBarIcon->SetIcon( m_AppIcon, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION "-" ID_GUAYADEQUE_REVISION ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::UpdatePodcasts( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config->ReadBool( wxT( "Update" ), true, wxT( "Podcasts" ) ) )
    {
        if( !m_UpdatePodcastsTimer )
        {
            //guLogMessage( wxT( "Creating the UpdatePodcasts timer..." ) );
            m_UpdatePodcastsTimer = new guUpdatePodcastsTimer( this, m_Db );
            m_UpdatePodcastsTimer->Start( guPODCASTS_UPDATE_TIMEOUT );
        }

        wxDateTime LastUpdate;
        LastUpdate.ParseDateTime( Config->ReadStr( wxT( "LastPodcastUpdate" ), wxEmptyString, wxT( "Podcasts" ) ) );

        wxDateTime UpdateTime = wxDateTime::Now();

        switch( Config->ReadNum( wxT( "UpdatePeriod" ), 0, wxT( "Podcasts" ) ) )
        {
            case guPODCAST_UPDATE_HOUR :    // Hour
                UpdateTime.Subtract( wxTimeSpan::Hour() );
                break;
            case guPODCAST_UPDATE_DAY :    // Day
                UpdateTime.Subtract( wxDateSpan::Day() );
                break;

            case guPODCAST_UPDATE_WEEK :    // Week
                UpdateTime.Subtract( wxDateSpan::Week() );
                break;

            case guPODCAST_UPDATE_MONTH :    // Month
                UpdateTime.Subtract( wxDateSpan::Month() );
                break;

            default :
                guLogError( wxT( "Unrecognized UpdatePeriod in podcasts" ) );
                return;
        }

        //guLogMessage( wxT( "%s -- %s" ), LastUpdate.Format().c_str(), UpdateTime.Format().c_str() );

        if( UpdateTime.IsLaterThan( LastUpdate ) )
        {
            //guLogMessage( wxT( "Starting UpdatePodcastsThread Process..." ) );
            int GaugeId = m_MainStatusBar->AddGauge( _( "Podcasts" ) );
            guUpdatePodcastsThread * UpdatePodcastThread = new guUpdatePodcastsThread( m_Db, this, GaugeId );
            if( !UpdatePodcastThread )
            {
                guLogError( wxT( "Could not create the Update Podcasts thread" ) );
            }

            Config->WriteStr( wxT( "LastPodcastUpdate" ), wxDateTime::Now().Format(), wxT( "Podcasts" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::AddPodcastsDownloadItems( guPodcastItemArray * items )
{
    wxASSERT( items );

    int Index;
    int Count = items->Count();
    if( Count )
    {
        if( !m_DownloadThread )
        {
            m_DownloadThread = new guPodcastDownloadQueueThread( this );
        }

        for( Index = 0; Index < Count; Index++ )
        {
            if( items->Item( Index ).m_Status != guPODCAST_STATUS_PENDING )
            {
                items->Item( Index ).m_Status = guPODCAST_STATUS_PENDING;
                m_Db->SetPodcastItemStatus( items->Item( Index ).m_Id, guPODCAST_STATUS_PENDING );
            }
        }

        m_DownloadThread->AddPodcastItems( items );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::RemovePodcastDownloadItems( guPodcastItemArray * items )
{
    if( m_DownloadThread )
    {
        m_DownloadThread->RemovePodcastItems( items );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnRemovePodcastThread( wxCommandEvent &event )
{
    RemovePodcastsDownloadThread();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::RemovePodcastsDownloadThread( void )
{
    wxMutexLocker Lock( m_DownloadThreadMutex );

    if( m_DownloadThread && !m_DownloadThread->GetCount() )
    {
        m_DownloadThread->Pause();
        m_DownloadThread->Delete();
        //guLogMessage( wxT( "Podcast Download Thread destroyed..." ) );
        m_DownloadThread = NULL;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPodcastItemUpdated( wxCommandEvent &event )
{
    if( m_PodcastsPanel )
    {
        wxPostEvent( m_PodcastsPanel, event );
    }
    else
    {
        guPodcastItem * PodcastItem = ( guPodcastItem * ) event.GetClientData();
        delete PodcastItem;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::LoadLayouts( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_LayoutName    = Config->ReadAStr( wxT( "Name"   ), wxEmptyString, wxT( "Layouts" ) );
    m_LayoutData    = Config->ReadAStr( wxT( "Data"   ), wxEmptyString, wxT( "Layouts" ) );
    m_LayoutTabs    = Config->ReadAStr( wxT( "Tabs"   ), wxEmptyString, wxT( "Layouts" ) );

    size_t Count = wxMin( wxMin( m_LayoutName.Count(),
                                        m_LayoutData.Count() ),
                                 m_LayoutTabs.Count() );

    while( m_LayoutName.Count() > Count )
        m_LayoutName.RemoveAt( m_LayoutName.Count() - 1 );

    while( m_LayoutData.Count() > Count )
        m_LayoutData.RemoveAt( m_LayoutData.Count() - 1 );

    while( m_LayoutTabs.Count() > Count )
        m_LayoutTabs.RemoveAt( m_LayoutTabs.Count() - 1 );

}

// -------------------------------------------------------------------------------- //
void guMainFrame::SaveLayouts( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteAStr( wxT( "Name"   ), m_LayoutName,   wxT( "Layouts" ) );
    Config->WriteAStr( wxT( "Data"   ), m_LayoutData,   wxT( "Layouts" ), false );
    Config->WriteAStr( wxT( "Tabs"   ), m_LayoutTabs,   wxT( "Layouts" ), false );
    Config->Flush();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCreateNewLayout( wxCommandEvent &event )
{
    wxTextEntryDialog EntryDialog( this, _( "Enter the layout name:"), _( "New Layout" ) );

    EntryDialog.SetValue( wxString::Format( _( "Layout %u" ), unsigned( m_LayoutName.GetCount() + 1 ) ) );

    if( EntryDialog.ShowModal() == wxID_OK )
    {
        // If was the first layout delete the dummy ones
        if( !m_LayoutName.Count() )
        {
            while( m_LayoutLoadMenu->GetMenuItemCount() )
                m_LayoutLoadMenu->Delete( m_LayoutLoadMenu->FindItemByPosition( 0 ) );

            while( m_LayoutDelMenu->GetMenuItemCount() )
                m_LayoutDelMenu->Delete( m_LayoutDelMenu->FindItemByPosition( 0 ) );
        }

        int LayoutIndex = m_LayoutName.Index( EntryDialog.GetValue(), false );
        if( LayoutIndex == wxNOT_FOUND )
        {

            m_LayoutName.Add( EntryDialog.GetValue() );

            wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
            wxString PLCaption = PaneInfo.caption;
            PaneInfo.Caption( _( "Now Playing" ) );
            m_LayoutData.Add( m_AuiManager.SavePerspective() );
            PaneInfo.Caption( PLCaption );
            m_LayoutTabs.Add( m_CatNotebook->SavePerspective() );

            m_LayoutLoadMenu->Append( ID_MENU_LAYOUT_LOAD + m_LayoutName.Count() - 1,
                    EntryDialog.GetValue(), _( "Load this user defined layout" ) );
            m_LayoutDelMenu->Append( ID_MENU_LAYOUT_DELETE + m_LayoutName.Count() - 1,
                    EntryDialog.GetValue(), _( "Load this user defined layout" ) );
        }
        else
        {
            wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
            wxString PLCaption = PaneInfo.caption;
            PaneInfo.Caption( _( "Now Playing" ) );
            m_LayoutData[ LayoutIndex ] = m_AuiManager.SavePerspective();
            PaneInfo.Caption( PLCaption );
            m_LayoutTabs[ LayoutIndex ] = m_CatNotebook->SavePerspective();
        }
        SaveLayouts();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnDeleteLayout( wxCommandEvent &event )
{
    int Layout = event.GetId() - ID_MENU_LAYOUT_DELETE;
    //guLogMessage( wxT( "Delete Layout %i" ), Layout );
    int Index;
    int Count;

    while( m_LayoutLoadMenu->GetMenuItemCount() )
        m_LayoutLoadMenu->Delete( m_LayoutLoadMenu->FindItemByPosition( 0 ) );

    while( m_LayoutDelMenu->GetMenuItemCount() )
        m_LayoutDelMenu->Delete( m_LayoutDelMenu->FindItemByPosition( 0 ) );

    m_LayoutName.RemoveAt( Layout );
    m_LayoutData.RemoveAt( Layout );
    m_LayoutTabs.RemoveAt( Layout );

    wxMenuItem * MenuItem;
    Count = m_LayoutName.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_LOAD + Index, m_LayoutName[ Index ], _( "Load this user defined layout" ) );
        m_LayoutLoadMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DELETE + Index, m_LayoutName[ Index ], _( "Delete this user defined layout" ) );
        m_LayoutDelMenu->Append( MenuItem );
    }
    if( !Count )
    {
        MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DUMMY, _( "No layouts defined" ), _( "Load this user defined layout" ) );
        m_LayoutLoadMenu->Append( MenuItem );
        MenuItem->Enable( false );
        MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DUMMY, _( "No layouts defined" ), _( "Load this user defined layout" ) );
        m_LayoutDelMenu->Append( MenuItem );
        MenuItem->Enable( false );
    }

    SaveLayouts();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLoadLayout( wxCommandEvent &event )
{
    int Layout = event.GetId() - ID_MENU_LAYOUT_LOAD;
    bool IsShown;

    Hide();
//    bool NBIsShown = PaneInfo.IsShown();
    //guLogMessage( wxT( "Loading Layout %i" ), Layout );
    LoadTabsPerspective( m_LayoutTabs[ Layout ] );

    m_AuiManager.LoadPerspective( m_LayoutData[ Layout ] );

    wxAuiPaneInfo &PlayListPaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
    //guLogMessage( wxT( "PlayerPlayList: %i   %i" ), PlayListPaneInfo.IsShown(), bool( m_VisiblePanels & guPANEL_MAIN_PLAYERPLAYLIST ) );
    if( ( IsShown = PlayListPaneInfo.IsShown() ) != bool( m_VisiblePanels & guPANEL_MAIN_PLAYERPLAYLIST ) )
    {
        if( IsShown )
            m_VisiblePanels |= guPANEL_MAIN_PLAYERPLAYLIST;
        else
            m_VisiblePanels ^= guPANEL_MAIN_PLAYERPLAYLIST;

        m_ViewPlayerPlayList->Check( IsShown );
    }

    wxAuiPaneInfo &FiltersPaneInfo = m_AuiManager.GetPane( wxT( "PlayerFilters" ) );
    //guLogMessage( wxT( "PlayerFilters: %i   %i" ), FiltersPaneInfo.IsShown(), bool( m_VisiblePanels & guPANEL_MAIN_PLAYERFILTERS ) );
    if( ( IsShown = FiltersPaneInfo.IsShown() ) != bool( m_VisiblePanels & guPANEL_MAIN_PLAYERFILTERS ) )
    {
        if( IsShown )
            m_VisiblePanels |= guPANEL_MAIN_PLAYERFILTERS;
        else
            m_VisiblePanels ^= guPANEL_MAIN_PLAYERFILTERS;

        m_ViewPlayerFilters->Check( IsShown );
    }

    wxAuiPaneInfo &VumetersPaneInfo = m_AuiManager.GetPane( wxT( "PlayerVumeters" ) );
    //guLogMessage( wxT( "PlayerFilters: %i   %i" ), FiltersPaneInfo.IsShown(), bool( m_VisiblePanels & guPANEL_MAIN_PLAYERFILTERS ) );
    if( ( IsShown = VumetersPaneInfo.IsShown() ) != bool( m_VisiblePanels & guPANEL_MAIN_PLAYERVUMETERS ) )
    {
        if( IsShown )
            m_VisiblePanels |= guPANEL_MAIN_PLAYERVUMETERS;
        else
            m_VisiblePanels ^= guPANEL_MAIN_PLAYERVUMETERS;

        m_ViewPlayerVumeters->Check( IsShown );
    }

    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
    if( !PaneInfo.IsShown() )
    {
        m_VisiblePanels = m_VisiblePanels & ( guPANEL_MAIN_PLAYERPLAYLIST |
                                              guPANEL_MAIN_PLAYERFILTERS |
                                              guPANEL_MAIN_PLAYERVUMETERS );

        // Reset the Menu entry for all elements
        m_ViewLibrary->Check( false );
        m_ViewLibTextSearch->Enable( false );
        m_ViewLibLabels->Enable( false );
        m_ViewLibGenres->Enable( false );
        m_ViewLibArtists->Enable( false );
        m_ViewLibAlbums->Enable( false );
        m_ViewLibYears->Enable( false );
        m_ViewLibRatings->Enable( false );
        m_ViewLibPlayCounts->Enable( false );
        m_ViewLibComposers->Enable( false );
        m_ViewLibAlbumArtists->Enable( false );

        m_ViewRadios->Check( false );
        m_ViewRadTextSearch->Enable( false );
        m_ViewRadLabels->Enable( false );
        m_ViewRadGenres->Enable( false );

        m_ViewLastFM->Check( false );

        m_ViewLyrics->Check( false );

        m_ViewPlayLists->Check( false );

        m_ViewPodcasts->Check( false );
        m_ViewPodChannels->Enable( m_ViewPodcasts->IsChecked() );
        m_ViewPodDetails->Enable( m_ViewPodcasts->IsChecked() );

        m_ViewAlbumBrowser->Check( false );

        m_ViewFileBrowser->Check( false );
    }

    m_AuiManager.Update();

    OnPlayerPlayListUpdateTitle( event );

    Show();

}

// -------------------------------------------------------------------------------- //
void guMainFrame::LoadTabsPerspective( const wxString &layout )
{
    wxCommandEvent event;

    // Empty the tabs
    int Index = 0;
    int Count = m_CatNotebook->GetPageCount();
    for( Index = 0; Index < Count; Index++ )
    {
        RemoveTabPanel( ( wxPanel * ) m_CatNotebook->GetPage( 0 ) );
    }

    // Add the tabs in the proper ordering
    wxString TabsLayout = layout.AfterFirst( wxT( '=' ) );
    TabsLayout = TabsLayout.BeforeFirst( wxT( '@' ) );
    event.SetInt( 1 );

    //02dbb6304b6f3a2b000e09c000000002=
    //+00[Librería],01[Radio],03[Letras],04[Listas],05[Podcasts]|02fc2e004b6f433c007e30b000000003=*02[Last.fm]
    //@
    m_VisiblePanels = m_VisiblePanels & ( guPANEL_MAIN_PLAYERPLAYLIST |
                                          guPANEL_MAIN_PLAYERFILTERS |
                                          guPANEL_MAIN_PLAYERVUMETERS );

    // Reset the Menu entry for all elements
    m_ViewLibrary->Check( false );
    m_ViewLibTextSearch->Enable( false );
    m_ViewLibLabels->Enable( false );
    m_ViewLibGenres->Enable( false );
    m_ViewLibArtists->Enable( false );
    m_ViewLibAlbums->Enable( false );
    m_ViewLibYears->Enable( false );
    m_ViewLibRatings->Enable( false );
    m_ViewLibPlayCounts->Enable( false );
    m_ViewLibComposers->Enable( false );
    m_ViewLibAlbumArtists->Enable( false );

    m_ViewRadios->Check( false );
    m_ViewRadTextSearch->Enable( false );
    m_ViewRadLabels->Enable( false );
    m_ViewRadGenres->Enable( false );

    m_ViewLastFM->Check( false );

    m_ViewLyrics->Check( false );

    m_ViewPlayLists->Check( false );

    m_ViewPodcasts->Check( false );
    m_ViewPodChannels->Enable( m_ViewPodcasts->IsChecked() );
    m_ViewPodDetails->Enable( m_ViewPodcasts->IsChecked() );

    m_ViewAlbumBrowser->Check( false );

    m_ViewFileBrowser->Check( false );


    Index = 0;
    while( true )
    {
        int FindPos = TabsLayout.Find( wxString::Format( wxT( "%02i[" ), Index ) );
        if( FindPos == wxNOT_FOUND )
            break;
        wxString TabName = TabsLayout.Mid( FindPos ).AfterFirst( wxT( '[' ) ).BeforeFirst( wxT( ']' ) );

        //guLogMessage( wxT( "Creating tab %i '%s'" ), Index, TabName.c_str() );

        if( TabName == _( "Library" ) )
        {
            OnViewLibrary( event );
        }
        else if( TabName == _( "Radio" ) )
        {
            OnViewRadio( event );
        }
        else if( TabName == _( "Last.fm" ) )
        {
            OnViewLastFM( event );
        }
        else if( TabName == _( "Lyrics" ) )
        {
            OnViewLyrics( event );
        }
        else if( TabName == _( "PlayLists" ) )
        {
            OnViewPlayLists( event );
        }
        else if( TabName == _( "Podcasts" ) )
        {
            OnViewPodcasts( event );
        }
        else if( TabName == _( "Browser" ) )
        {
            OnViewAlbumBrowser( event );
        }
        else if( TabName == _( "Files" ) )
        {
            OnViewFileBrowser( event );
        }

        Index++;
    }

    m_CatNotebook->LoadPerspective( layout );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnMainPaneClose( wxAuiManagerEvent &event )
{
    wxAuiPaneInfo * PaneInfo = event.GetPane();
    wxString PaneName = PaneInfo->name;
    int CmdId = 0;

    if( PaneName == wxT( "PlayerPlayList" ) )
    {
        CmdId = ID_MENU_VIEW_PLAYER_PLAYLIST;
    }
    else if( PaneName == wxT( "PlayerFilters" ) )
    {
        CmdId = ID_MENU_VIEW_PLAYER_FILTERS;
    }
    else if( PaneName == wxT( "PlayerVumeters" ) )
    {
        CmdId = ID_MENU_VIEW_PLAYER_VUMETERS;
    }
    else if( PaneName == wxT( "PlayerSelector" ) )
    {
        CmdId = ID_MENU_VIEW_PLAYER_SELECTOR;
    }

    guLogMessage( wxT( "OnMainPaneClose: %s  %i" ), PaneName.c_str(), CmdId );
    if( CmdId )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, CmdId );
        evt.SetInt( 0 );
        AddPendingEvent( evt );
    }

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerShowPanel( wxCommandEvent &event )
{
    unsigned int PanelId = 0;
    switch( event.GetId() )
    {
        case ID_MENU_VIEW_PLAYER_PLAYLIST :
        {
            PanelId = guPANEL_MAIN_PLAYERPLAYLIST;
            break;
        }

        case ID_MENU_VIEW_PLAYER_FILTERS :
        {
            PanelId = guPANEL_MAIN_PLAYERFILTERS;
            break;
        }

        case ID_MENU_VIEW_PLAYER_VUMETERS :
        {
            PanelId = guPANEL_MAIN_PLAYERVUMETERS;
            break;
        }

        case ID_MENU_VIEW_PLAYER_SELECTOR :
        {
            if( m_VisiblePanels & guPANEL_MAIN_LIBRARY )
                OnViewLibrary( event );

            if( m_VisiblePanels & guPANEL_MAIN_RADIOS )
                OnViewRadio( event );

            if( m_VisiblePanels & guPANEL_MAIN_LASTFM )
                OnViewLastFM( event );

            if( m_VisiblePanels & guPANEL_MAIN_LYRICS )
                OnViewLyrics( event );

            if( m_VisiblePanels & guPANEL_MAIN_PLAYLISTS )
                OnViewPlayLists( event );

            if( m_VisiblePanels & guPANEL_MAIN_PODCASTS )
                OnViewPodcasts( event );

            if( m_VisiblePanels & guPANEL_MAIN_ALBUMBROWSER )
                OnViewAlbumBrowser( event );

            if( m_VisiblePanels & guPANEL_MAIN_FILEBROWSER )
                OnViewFileBrowser( event );

            break;
        }

        default :
            return;
    }

    if( PanelId )
        ShowMainPanel( PanelId, event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::ShowMainPanel( const int panelid, const bool show )
{
    wxString PaneName;

    switch( panelid )
    {
        case guPANEL_MAIN_PLAYERPLAYLIST :
            PaneName = wxT( "PlayerPlayList" );
            m_ViewPlayerPlayList->Check( show );
            break;

        case guPANEL_MAIN_PLAYERFILTERS :
            PaneName = wxT( "PlayerFilters" );
            m_ViewPlayerFilters->Check( show );
            break;

        case guPANEL_MAIN_PLAYERVUMETERS :
            PaneName = wxT( "PlayerVumeters" );
            m_ViewPlayerVumeters->Check( show );
            break;

        default :
            return;

    }

    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( PaneName );
    if( PaneInfo.IsOk() )
    {
        if( show )
            PaneInfo.Show();
        else
            PaneInfo.Hide();

        m_AuiManager.Update();
    }

    if( show )
        m_VisiblePanels |= panelid;
    else
        m_VisiblePanels ^= panelid;

    guLogMessage( wxT( "Id: %i Pane: %s Show:%i  Flags:%08X" ), panelid, PaneName.c_str(), show, m_VisiblePanels );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::UpdatedTracks( int updatedby, const guTrackArray * tracks )
{
    if( updatedby != guUPDATED_TRACKS_PLAYER )
    {
        m_PlayerPanel->UpdatedTracks( tracks );
    }

    if( updatedby != guUPDATED_TRACKS_PLAYER_PLAYLIST )
    {
        m_PlayerPlayList->UpdatedTracks( tracks );
    }

    if( ( updatedby != guUPDATED_TRACKS_LIBRARY ) && m_LibPanel )
    {
        m_LibPanel->UpdatedTracks( tracks );
    }

    if( ( updatedby != guUPDATED_TRACKS_PLAYLISTS ) && m_PlayListPanel )
    {
        m_PlayListPanel->UpdatedTracks( tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::UpdatedTrack( int updatedby, const guTrack * track )
{
    if( m_PlayerPanel && ( updatedby != guUPDATED_TRACKS_PLAYER ) )
    {
        m_PlayerPanel->UpdatedTrack( track );
    }

    if( m_PlayerPlayList && ( updatedby != guUPDATED_TRACKS_PLAYER_PLAYLIST ) )
    {
        m_PlayerPlayList->UpdatedTrack( track );
    }

    if( m_LibPanel && ( updatedby != guUPDATED_TRACKS_LIBRARY ) )
    {
        m_LibPanel->UpdatedTrack( track );
    }

    if( m_PlayListPanel && ( updatedby != guUPDATED_TRACKS_PLAYLISTS ) )
    {
        m_PlayListPanel->UpdatedTrack( track );
    }
}





// -------------------------------------------------------------------------------- //
// guUpdateCoversThread
// -------------------------------------------------------------------------------- //
guUpdateCoversThread::guUpdateCoversThread( guDbLibrary * db, int gaugeid ) : wxThread()
{
    m_Db = db;
    m_GaugeId = gaugeid;
}

// -------------------------------------------------------------------------------- //
guUpdateCoversThread::~guUpdateCoversThread()
{
//    printf( "guUpdateCoversThread Object destroyed\n" );
};


// -------------------------------------------------------------------------------- //
bool FindCoverLink( guDbLibrary * Db, int AlbumId, const wxString &Album, const wxString &Artist, const wxString &Path )
{
    bool RetVal = false;
    guLastFM * LastFM;
    wxString AlbumName;
    guAlbumInfo AlbumInfo;

    LastFM = new guLastFM();
    if( LastFM )
    {
        // Remove from album name expressions like (cd1),(cd2) etc
        AlbumName = RemoveSearchFilters( Album );

        AlbumInfo = LastFM->AlbumGetInfo( Artist, AlbumName );

        // Try to download the cover
        if( LastFM->IsOk() )
        {
            if( !AlbumInfo.m_ImageLink.IsEmpty() )
            {
                //guLogMessage( wxT( "ImageLink : %s" ), AlbumInfo.ImageLink.c_str() );
                // Save the cover into the target directory.
                // Changed to DownloadImage to convert all images to jpg
                guConfig * Config = ( guConfig * ) guConfig::Get();
                wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
                wxString CoverName = Path + ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );
                if( !DownloadImage( AlbumInfo.m_ImageLink, CoverName ) )
                {
                    guLogWarning( wxT( "Could not download cover file" ) );
                }
                else
                {
//                    DownloadedCovers++;
                    Db->SetAlbumCover( AlbumId, CoverName );
                    //guLogMessage( wxT( "Cover file downloaded for %s - %s" ), Artist.c_str(), AlbumName.c_str() );
                    RetVal = true;
                }
            }
        }
        else
        {
            // There was en error...
            guLogError( wxT( "Error getting the cover for %s - %s (%u)" ),
                     Artist.c_str(),
                     AlbumName.c_str(),
                     LastFM->GetLastError() );
        }
        delete LastFM;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guUpdateCoversThread::ExitCode guUpdateCoversThread::Entry()
{
    guCoverInfos CoverInfos = m_Db->GetEmptyCovers();

    //
    int Count = CoverInfos.Count();

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    event.SetInt( m_GaugeId );
    event.SetExtraLong( Count );
    wxPostEvent( wxTheApp->GetTopWindow(), event );

    for( int Index = 0; Index < Count; Index++ )
    {
        guCoverInfo * CoverInfo = &CoverInfos[ Index ];
        Sleep( 1000 ); // Dont hammer LastFM and wait 1 second before each LastFM query
        //guLogMessage( wxT( "Downloading cover for %s - %s" ), CoverInfo->m_ArtistName.c_str(), CoverInfo->m_AlbumName.c_str() );
        FindCoverLink( m_Db, CoverInfo->m_AlbumId, CoverInfo->m_AlbumName, CoverInfo->m_ArtistName, CoverInfo->m_PathName );

        //wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_UPDATE );
        event.SetId( ID_GAUGE_UPDATE );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( Index );
        wxPostEvent( wxTheApp->GetTopWindow(), event );

    }
    //guLogMessage( wxT( "Finalized Cover Update Thread" ) );

    //event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
    event.SetId( ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( wxTheApp->GetTopWindow(), event );

    return 0;
}

// -------------------------------------------------------------------------------- //
// guCopyToDirThread
// -------------------------------------------------------------------------------- //
guCopyToDirThread::guCopyToDirThread( const wxChar * destdir, guTrackArray * tracks, int gaugeid ) :
    wxThread()
{
    m_DestDir   = wxString( destdir );
    m_Tracks    = tracks;
    m_GaugeId   = gaugeid;
    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guCopyToDirThread::~guCopyToDirThread()
{
    if( m_Tracks )
        delete m_Tracks;
};

// -------------------------------------------------------------------------------- //
guCopyToDirThread::ExitCode guCopyToDirThread::Entry()
{
    int         count = m_Tracks->Count();
    int         index;
    wxString    FileName;
    wxString    FilePattern;
	guConfig *  Config;
	bool        FileOverwrite = true;

    Config = ( guConfig * ) guConfig::Get();
    FilePattern = Config->ReadStr( wxT( "CopyToPattern" ), wxT( "{g}/{a}/{b}/{n} - {a} - {t}" ), wxT( "General" ) );

    if( !m_DestDir.EndsWith( wxT( "/" ) ) )
        m_DestDir.Append( wxT( "/" ) );

    //
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    event.SetInt( m_GaugeId );
    event.SetExtraLong( count );
    wxPostEvent( wxTheApp->GetTopWindow(), event );

    for( index = 0; index < count; index++ )
    {
        FileName = wxEmptyString;

        if( ( * m_Tracks )[ index ].m_Type == guTRACK_TYPE_RADIOSTATION )
            continue;

        if( ( * m_Tracks )[ index ].m_Type == guTRACK_TYPE_PODCAST )
        {
            FileName = ( * m_Tracks )[ index ].m_AlbumName + wxT( "/" ) + wxFileNameFromPath( ( * m_Tracks )[ index ].m_FileName );
        }
        else
        {
            FileName = FilePattern;
            FileName.Replace( wxT( "{a}" ), NormalizeField( ( * m_Tracks )[ index ].m_ArtistName ) );
            FileName.Replace( wxT( "{aa}" ), NormalizeField( ( * m_Tracks )[ index ].m_AlbumArtist ) );
            FileName.Replace( wxT( "{b}" ), NormalizeField( ( * m_Tracks )[ index ].m_AlbumName ) );
            FileName.Replace( wxT( "{f}" ), wxFileNameFromPath( ( * m_Tracks )[ index ].m_FileName ) );
            FileName.Replace( wxT( "{g}" ), NormalizeField( ( * m_Tracks )[ index ].m_GenreName ) );
            FileName.Replace( wxT( "{n}" ), wxString::Format( wxT( "%02u" ), ( * m_Tracks )[ index ].m_Number ) );
            FileName.Replace( wxT( "{t}" ), NormalizeField( ( * m_Tracks )[ index ].m_SongName ) );
            FileName.Replace( wxT( "{y}" ), wxString::Format( wxT( "%u" ), ( * m_Tracks )[ index ].m_Year ) );
            FileName.Replace( wxT( "{d}" ), NormalizeField( ( * m_Tracks )[ index ].m_Disk ) );
            //guLogMessage( wxT( "File: '%s' " ), FileName.c_str() );

        }

        FileName += wxT( '.' ) + ( * m_Tracks )[ index ].m_FileName.Lower().AfterLast( wxT( '.' ) );

        FileName = m_DestDir + FileName;

        // Replace all the special chars < > : " / \ | ? *
        FileName.Replace( wxT( "<" ), wxT( "_" ) );
        FileName.Replace( wxT( ">" ), wxT( "_" ) );
        FileName.Replace( wxT( ":" ), wxT( "_" ) );
        FileName.Replace( wxT( "\"" ), wxT( "_" ) );
        FileName.Replace( wxT( "|" ), wxT( "_" ) );
        FileName.Replace( wxT( "?" ), wxT( "_" ) );
        FileName.Replace( wxT( "*" ), wxT( "_" ) );

        guLogMessage( wxT( "Copy %s =>> %s" ), ( * m_Tracks )[ index ].m_FileName.c_str(), FileName.c_str() );
        if( wxFileName::Mkdir( wxPathOnly( FileName ), 0777, wxPATH_MKDIR_FULL ) )
        {
            if( !wxCopyFile( ( * m_Tracks )[ index ].m_FileName, FileName, FileOverwrite ) )
            {
                guLogError( wxT( "Could not copy the file '%s'" ), FileName.c_str() );
            }
        }
        else
        {
            guLogError( wxT( "Could not create path for copy the file '%s'" ), FileName.c_str() );
        }
        //
        event.SetId( ID_GAUGE_UPDATE );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( index + 1 );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }

    //
    event.SetId( ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
    //wxMessageBox( "Copy to dir finished" );
    return 0;
}

// -------------------------------------------------------------------------------- //
// guUpdatePodcastsTimer
// -------------------------------------------------------------------------------- //
guUpdatePodcastsTimer::guUpdatePodcastsTimer( guMainFrame * mainframe, guDbLibrary * db ) : wxTimer()
{
    m_MainFrame = mainframe;
    m_Db = db;
}

// -------------------------------------------------------------------------------- //
void guUpdatePodcastsTimer::Notify()
{
    m_MainFrame->UpdatePodcasts();
}

// -------------------------------------------------------------------------------- //
// guUpdatePodcastThread
// -------------------------------------------------------------------------------- //
guUpdatePodcastsThread::guUpdatePodcastsThread( guDbLibrary * db, guMainFrame * mainframe,
    int gaugeid ) : wxThread()
{
    m_Db = db;
    m_MainFrame = mainframe;
    m_GaugeId = gaugeid;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guUpdatePodcastsThread::~guUpdatePodcastsThread()
{
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
guUpdatePodcastsThread::ExitCode guUpdatePodcastsThread::Entry()
{
    guPodcastChannelArray PodcastChannels;
    if( m_Db->GetPodcastChannels( &PodcastChannels ) )
    {
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( PodcastChannels.Count() );
        wxPostEvent( m_MainFrame, event );

        unsigned int Index = 0;
        while( !TestDestroy() && Index < PodcastChannels.Count() )
        {
            event.SetId( ID_GAUGE_UPDATE );
            event.SetInt( m_GaugeId );
            event.SetExtraLong( Index + 1 );
            wxPostEvent( m_MainFrame, event );

            PodcastChannels[ Index ].Update( m_Db, m_MainFrame );
            Index++;

            Sleep( 20 );
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
