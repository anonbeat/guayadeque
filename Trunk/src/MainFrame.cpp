// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
#include "MainFrame.h"

#include "Accelerators.h"
#include "AuiDockArt.h"
#include "Commands.h"
#include "CopyTo.h"
#include "ConfirmExit.h"
#include "EditWithOptions.h"
#include "FileRenamer.h"    // NormalizeField
#include "Images.h"
#include "LibUpdate.h"
#include "MediaViewerLibrary.h"
#include "iPodMedia.h"
#include "LocationPanel.h"
#include "Preferences.h"
#include "Settings.h"
#include "TagInfo.h"
#include "TaskBar.h"
#include "TrackChangeInfo.h"
#include "Transcode.h"
#include "Utils.h"
#include "Version.h"

#include <wx/event.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/datetime.h>
#include <wx/tokenzr.h>
#include <wx/txtstrm.h>

// The default update podcasts timeout is 15 minutes
#define guPODCASTS_UPDATE_TIMEOUT   ( 15 * 60 * 1000 )

guMainFrame * guMainFrame::m_MainFrame = NULL;


// -------------------------------------------------------------------------------- //
guMainFrame::guMainFrame( wxWindow * parent, guDbCache * dbcache )
{
    SetMainFrame();

    //
    // Init the Config Object
    //
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( !Config )
    {
        guLogError( wxT( "Could not open the guayadeque configuration object" ) );
        return;
    }
    Config->RegisterObject( this );

    // Init the Accelerators
    guAccelInit();

    m_Db = NULL;
    m_DbCache = dbcache;
    m_DbPodcasts = NULL;
//    m_JamendoDb = NULL;
//    m_MagnatuneDb = NULL;
    m_CopyToThread = NULL;
    m_LoadLayoutPending = wxNOT_FOUND;
    m_MenuLayoutLoad = NULL;

    bool NeedSaveCollections = false;
    m_CollectionsMutex.Lock();
    if( !Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_NORMAL ) )
    {
        guMediaCollection * MediaCollection = new guMediaCollection( guMEDIA_COLLECTION_TYPE_NORMAL );
        MediaCollection->m_Name = _( "My Music" );
        MediaCollection->m_Paths.Add( wxGetHomeDir() + wxT( "/Music" ) );
        MediaCollection->m_CoverWords.Add( wxT( "cover" ) );
        MediaCollection->m_CoverWords.Add( wxT( "front" ) );
        m_Collections.Add( MediaCollection );
        NeedSaveCollections = true;
    }

    if( !Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_JAMENDO ) )
    {
        guMediaCollection * MediaCollection = new guMediaCollection( guMEDIA_COLLECTION_TYPE_JAMENDO );
        MediaCollection->m_Name = wxT( "Jamendo" );
        MediaCollection->m_UniqueId = wxT( "Jamendo" );
        m_Collections.Add( MediaCollection );
        NeedSaveCollections = true;
    }

    if( !Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_MAGNATUNE ) )
    {
        guMediaCollection * MediaCollection = new guMediaCollection( guMEDIA_COLLECTION_TYPE_MAGNATUNE );
        MediaCollection->m_Name = wxT( "Magnatune" );
        MediaCollection->m_UniqueId = wxT( "Magnatune" );
        m_Collections.Add( MediaCollection );
        NeedSaveCollections = true;
    }

    Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE );
    Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_IPOD );

    m_CollectionsMutex.Unlock();

    if( NeedSaveCollections )
    {
        Config->SaveCollections( &m_Collections );
    }
    //
//    m_Db->SetLibPath( Config->ReadAStr( wxT( "LibPath" ),
//                                      wxGetHomeDir() + wxT( "/Music" ),
//                                      wxT( "libpaths" ) ) );

//    m_LibUpdateThread = NULL;
//    m_LibCleanThread = NULL;
    m_UpdatePodcastsTimer = NULL;
    m_DownloadThread = NULL;
    m_NotifySrv = NULL;

    m_SelCount = 0;
    m_SelLength = 0;
    m_SelSize = 0;

    //
    m_PlayerPanel = NULL;
    m_PlayerPlayList = NULL;
    m_RadioPanel = NULL;
    m_LastFMPanel = NULL;
    m_LyricsPanel = NULL;
    m_PodcastsPanel = NULL;
    m_PlayerVumeters = NULL;
    m_FileBrowserPanel = NULL;
    m_VolumeMonitor = NULL;
    m_LocationPanel = NULL;
    m_CoverPanel = NULL;

    m_MenuPlayerPlayList = NULL;
    m_MenuPlayerFilters = NULL;
    m_MenuPlayerVumeters = NULL;
    m_MenuMainLocations = NULL;
    m_MenuMainShowCover = NULL;

    m_LyricSearchEngine = NULL;
    m_LyricSearchContext = NULL;

    //
    wxImage TaskBarIcon( guImage( guIMAGE_INDEX_guayadeque_taskbar ) );
    TaskBarIcon.ConvertAlphaToMask();
    m_AppIcon.CopyFromBitmap( TaskBarIcon );

    //
    m_VolumeMonitor = new guGIO_VolumeMonitor( this );

    m_LyricSearchEngine = new guLyricSearchEngine();

    //
    // guMainFrame GUI components
    //
    wxPoint MainWindowPos;
    MainWindowPos.x = Config->ReadNum( wxT( "PosX" ), 1, wxT( "mainwindow/positions" ) );
    MainWindowPos.y = Config->ReadNum( wxT( "PosY" ), 1, wxT( "mainwindow/positions" ) );
    wxSize MainWindowSize;
    MainWindowSize.x = Config->ReadNum( wxT( "Width" ), 800, wxT( "mainwindow/positions" ) );
    MainWindowSize.y = Config->ReadNum( wxT( "Height" ), 600, wxT( "mainwindow/positions" ) );
    Create( parent, wxID_ANY, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION "-" ID_GUAYADEQUE_REVISION ),
                MainWindowPos, MainWindowSize, wxDEFAULT_FRAME_STYLE );

    m_AuiManager.SetManagedWindow( this );
    m_AuiManager.SetArtProvider( new guAuiDockArt() );
    m_AuiManager.SetFlags( wxAUI_MGR_ALLOW_FLOATING |
                           wxAUI_MGR_TRANSPARENT_DRAG |
                           wxAUI_MGR_TRANSPARENT_HINT );

    wxVisualAttributes VisualAttributes = wxStaticText::GetClassDefaultAttributes();

    wxAuiDockArt * AuiDockArt = m_AuiManager.GetArtProvider();

    wxColour BaseColor = VisualAttributes.colBg; //wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, BaseColor );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR,
            wxAuiStepColour( BaseColor, 93 ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR,
                          BaseColor );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR,
            wxAuiStepColour( BaseColor, 93 ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_SASH_COLOUR, wxAuiStepColour( BaseColor, 98 ) );

    AuiDockArt->SetMetric( wxAUI_DOCKART_CAPTION_SIZE, 22 );
    AuiDockArt->SetMetric( wxAUI_DOCKART_PANE_BORDER_SIZE, 0 );
    AuiDockArt->SetMetric( wxAUI_DOCKART_SASH_SIZE, 5 );

    AuiDockArt->SetMetric( wxAUI_DOCKART_GRADIENT_TYPE,
            wxAUI_GRADIENT_VERTICAL );

    if( Config->ReadBool( wxT( "LoadDefaultLayouts" ), false, wxT( "general" ) ) )
    {
        Config->WriteBool( wxT( "LoadDefaultLayouts" ), false, wxT( "general" ) );
        Config->SetIgnoreLayouts( true );
    }

	//m_MainNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_DestroyLastWindow = false;
	m_MainNotebook = new guAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON );

    guMediaViewerLibrary * MediaViewer = new guMediaViewerLibrary( m_MainNotebook, m_Collections[ 0 ], ID_COLLECTIONS_BASE, this, guMEDIAVIEWER_MODE_NONE, NULL );
    MediaViewer->SetDefault( true );
    m_MediaViewers.Add( MediaViewer );

    m_Db = MediaViewer->GetDb();


    m_VisiblePanels = Config->ReadNum( wxT( "VisiblePanels" ), guPANEL_MAIN_VISIBLE_DEFAULT, wxT( "mainwindow" ) );
    if( Config->GetIgnoreLayouts() )
        m_VisiblePanels = guPANEL_MAIN_VISIBLE_DEFAULT;
    //guLogMessage( wxT( "%08X" ), m_VisiblePanels );

	m_MainStatusBar = new guStatusBar( this );
	SetStatusBar(  m_MainStatusBar );
	//MainFrameSizer = new wxBoxSizer( wxVERTICAL );
	SetStatusText( _( "Welcome to Guayadeque " ) );
	SetStatusBarPane( 0 );

    //
    if( m_VisiblePanels & guPANEL_MAIN_PLAYERVUMETERS )
    {
        ShowMainPanel( guPANEL_MAIN_PLAYERVUMETERS, true );
    }

    if( m_VisiblePanels & guPANEL_MAIN_LOCATIONS )
    {
        ShowMainPanel( guPANEL_MAIN_LOCATIONS, true );
    }

    m_PlayerFilters = new guPlayerFilters( this, m_Db );
    m_AuiManager.AddPane( m_PlayerFilters, wxAuiPaneInfo().Name( wxT( "PlayerFilters" ) ).Caption( _( "Filters" ) ).
        DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 50, 50 ).
        CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "general" ) ) ).
        Bottom().Layer( 0 ).Row( 1 ).Position( 0 ) );

    m_PlayerPlayList = new guPlayerPlayList( this, m_Db, &m_AuiManager );
	m_AuiManager.AddPane( m_PlayerPlayList, wxAuiPaneInfo().Name( wxT( "PlayerPlayList" ) ).
        DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 100, 100 ).
        CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "general" ) ) ).
        Bottom().Layer( 0 ).Row( 2 ).Position( 0 ) );

	m_PlayerPanel = new guPlayerPanel( this, m_Db, m_PlayerPlayList->GetPlayListCtrl(), m_PlayerFilters );

	m_PlayerPlayList->SetPlayerPanel( m_PlayerPanel );
	m_PlayerPanel->SetPlayerVumeters( m_PlayerVumeters );
	MediaViewer->SetPlayerPanel( m_PlayerPanel );

	m_AuiManager.AddPane( m_PlayerPanel, wxAuiPaneInfo().Name( wxT( "PlayerPanel" ) ).
        CloseButton( false ).DestroyOnClose( false ).
        Resizable( true ).MinSize( 100, 100 ).
        CaptionVisible( true ).
        Bottom().Layer( 0 ).Row( 0 ).Position( 0 ) );


    if( m_VisiblePanels & guPANEL_MAIN_SHOWCOVER )
    {
        ShowMainPanel( guPANEL_MAIN_SHOWCOVER, true );
    }

    CreateMenu();

    if( m_LocationPanel )
        m_LocationPanel->Lock();

    m_NBPerspective = Config->ReadStr( wxT( "NotebookLayout" ), wxEmptyString, wxT( "mainwindow" ) );
    if( !Config->GetIgnoreLayouts() && !m_NBPerspective.IsEmpty() )
    {
        LoadTabsPerspective( m_NBPerspective );
    }
    else
    {
        wxCommandEvent ShowEvent;
        ShowEvent.SetInt( 1 );

        wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_COLLECTIONS_BASE );
        Event.SetInt( 1 );
        OnCollectionCommand( Event );

        // Radio Page
        if( m_VisiblePanels & guPANEL_MAIN_RADIOS )
        {
            OnViewRadio( ShowEvent );
        }

        // LastFM Info Panel
        if( m_VisiblePanels & guPANEL_MAIN_LASTFM )
        {
            OnViewLastFM( ShowEvent );
        }

        // Lyrics Panel
        if( m_VisiblePanels & guPANEL_MAIN_LYRICS )
        {
            OnViewLyrics( ShowEvent );
        }

        // Podcasts Page
        if( m_VisiblePanels & guPANEL_MAIN_PODCASTS )
        {
            OnViewPodcasts( ShowEvent );
        }

        // FileSystem Page
        if( m_VisiblePanels & guPANEL_MAIN_FILEBROWSER )
        {
            OnViewFileBrowser( ShowEvent );
        }
    }

    m_AuiManager.AddPane( m_MainNotebook, wxAuiPaneInfo().Name( wxT( "PlayerSelector" ) ).
        CaptionVisible( false ).CloseButton( false ).DestroyOnClose( false ).
        CenterPane() );

    //m_AuiManager.Update();
    wxString Perspective = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "mainwindow" ) );
    if( !Config->GetIgnoreLayouts() && !Perspective.IsEmpty() )
    {
        //m_AuiManager.LoadPerspective( Perspective, true );
        LoadPerspective( Perspective );
    }
    else
    {
        Perspective  = wxT( "layout2|name=PlayerVumeters;caption=" ) + wxString( _( "VU Meters" ) );
        Perspective += wxT( ";state=2098172;dir=4;layer=0;row=1;pos=1;prop=37931;bestw=20;besth=20;minw=20;minh=20;maxw=-1;maxh=-1;floatx=95;floaty=866;floatw=28;floath=44|" );
        Perspective += wxT( "name=PlayerFilters;caption=" ) + wxString( _( "Filters" ) );
        Perspective += wxT( ";state=2098172;dir=4;layer=0;row=1;pos=3;prop=44139;bestw=138;besth=63;minw=50;minh=50;maxw=-1;maxh=-1;floatx=81;floaty=863;floatw=146;floath=87|" );
        Perspective += wxT( "name=PlayerPlayList;caption=" ) + wxString( _( "Now Playing" ) );
        Perspective += wxT( ";state=2099196;dir=4;layer=0;row=1;pos=2;prop=195862;bestw=100;besth=100;minw=100;minh=100;maxw=-1;maxh=-1;floatx=81;floaty=534;floatw=300;floath=265|" );
        Perspective += wxT( "name=PlayerPanel;caption=;state=1020;dir=4;layer=0;row=1;pos=0;prop=122068;bestw=356;besth=177;minw=100;minh=100;maxw=-1;maxh=-1;floatx=83;floaty=746;floatw=364;floath=201|" );
        Perspective += wxT( "name=PlayerSelector;caption=;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=20;besth=20;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        Perspective += wxT( "dock_size(5,0,0)=22|dock_size(4,0,1)=302|" );
        m_AuiManager.LoadPerspective( Perspective, true );
          //m_AuiManager.Update();
    }

    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_MainNotebook );
    if( !PaneInfo.IsShown() )
    {
        m_VisiblePanels = m_VisiblePanels & ( guPANEL_MAIN_PLAYERPLAYLIST |
                                              guPANEL_MAIN_PLAYERFILTERS |
                                              guPANEL_MAIN_PLAYERVUMETERS |
                                              guPANEL_MAIN_LOCATIONS |
                                              guPANEL_MAIN_SHOWCOVER );

        // Reset the Menu entry for all elements
//        ResetViewMenuState();
    }

    m_CurrentPage = m_MainNotebook->GetPage( m_MainNotebook->GetSelection() );

    if( m_LocationPanel )
        m_LocationPanel->Unlock();

    //
    m_TaskBarIcon = NULL;
#ifdef WITH_LIBINDICATE_SUPPORT
    m_IndicateServer = NULL;
#endif

    m_DBusServer = new guDBusServer( NULL );
    guDBusServer::Set( m_DBusServer );
    if( !m_DBusServer )
    {
        guLogError( wxT( "Could not create the dbus server object" ) );
    }

    // Init the MPRIS object
    //m_MPRIS = NULL;
    m_MPRIS = new guMPRIS( m_DBusServer, m_PlayerPanel );
    if( !m_MPRIS )
    {
        guLogError( wxT( "Could not create the mpris dbus object" ) );
    }

    m_MPRIS2 = new guMPRIS2( m_DBusServer, m_PlayerPanel, m_Db );
    if( !m_MPRIS2 )
    {
        guLogError( wxT( "Could not create the mpris2 dbus object" ) );
    }
    guMPRIS2::Set( m_MPRIS2 );

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

    // Fill the Format extensions array
    guIsValidAudioFile( wxEmptyString );

    if( Config->ReadBool( wxT( "ShowFullScreen" ), IsFullScreen(), wxT( "mainwindow" ) ) != IsFullScreen() )
    {
        ShowFullScreen( !IsFullScreen(), wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION );
    }

//    // Load the layouts menu
//    CreateLayoutMenus();

    //
	Connect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );
	Connect( wxEVT_SIZE, wxSizeEventHandler( guMainFrame::OnSize ), NULL, this );

    Connect( ID_MENU_PLAY_STREAM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayStream ), NULL, this );
    Connect( ID_MENU_UPDATE_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdatePodcasts ), NULL, this );
    Connect( ID_MENU_VIEW_CLOSEWINDOW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCloseTab ), NULL, this );
    Connect( ID_MENU_HIDE_CAPTIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnShowCaptions ), NULL, this );
    Connect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ), NULL, this );

    Connect( ID_MENU_VOLUME_DOWN, ID_MENU_VOLUME_UP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnChangeVolume ), NULL, this );

//    Connect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ), NULL, this );
//    Connect( ID_JAMENDO_UPDATE_FINISHED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnJamendoUpdated ), NULL, this );
//    Connect( ID_MAGNATUNE_UPDATE_FINISHED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnMagnatuneUpdated ), NULL, this );
//    Connect( ID_LIBRARY_DOCLEANDB, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::DoLibraryClean ), NULL, this );
//    Connect( ID_LIBRARY_CLEANFINISHED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryCleanFinished ), NULL, this );
//    Connect( ID_LIBRARY_RELOADCONTROLS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryReloadControls ), NULL, this );

    Connect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ), NULL, this );
    Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    //Connect( wxEVT_ICONIZE, wxIconizeEventHandler( guMainFrame::OnIconizeWindow ), NULL, this );
    Connect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );
    Connect( ID_MENU_PREFERENCES_COMMANDS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );
    Connect( ID_MENU_PREFERENCES_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );
    Connect( ID_MENU_PREFERENCES_LINKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );
    Connect( ID_MENU_COLLECTION_NEW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );

    Connect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ), NULL, this );
    Connect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_VOLUMECHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerVolumeChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_SEEKED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerSeeked ), NULL, this );


	Connect( ID_MAINFRAME_SELECT_TRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetSelection ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetSelection ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_ALBUMARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetSelection ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_COMPOSER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetSelection ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetSelection ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_YEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetSelection ), NULL, this );
	Connect( ID_MAINFRAME_SELECT_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetSelection ), NULL, this );

	Connect( ID_MAINFRAME_SELECT_LOCATION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectLocation ), NULL, this );

    Connect( ID_MAINFRAME_SET_ALLOW_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetAllowDenyFilter ), NULL, this );
    Connect( ID_MAINFRAME_SET_DENY_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetAllowDenyFilter ), NULL, this );

    Connect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlay ), NULL, this );
    Connect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStop ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_STOP_ATEND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStopAtEnd ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnClearPlaylist ), NULL, this );
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
    Connect( ID_MENU_HELP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnHelp ), NULL, this );
    Connect( ID_MENU_COMMUNITY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCommunity ), NULL, this );

    Connect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );
    Connect( ID_MAINFRAME_COPYTODEVICE_TRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksToDevice ), NULL, this );
    Connect( ID_MAINFRAME_COPYTODEVICE_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyPlayListToDevice ), NULL, this );

//    Connect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

    Connect( ID_MENU_LAYOUT_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCreateNewLayout ), NULL, this );
    Connect( ID_MENU_LAYOUT_LOAD, ID_MENU_LAYOUT_LOAD + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadLayout ), NULL, this );
    Connect( ID_MENU_LAYOUT_DELETE, ID_MENU_LAYOUT_DELETE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnDeleteLayout ), NULL, this );

    Connect( ID_MENU_VIEW_PLAYER_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_PLAYER_FILTERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_PLAYER_VUMETERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_PLAYER_NOTEBOOK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_MAIN_LOCATIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_MAIN_SHOWCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );

    Connect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ), NULL, this );
    Connect( ID_MENU_VIEW_RAD_TEXTSEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_RAD_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_RAD_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_RAD_PROPERTIES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioProperties ), NULL, this );

    Connect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ), NULL, this );
    Connect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ), NULL, this );

    Connect( ID_MENU_VIEW_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPodcasts ), NULL, this );
    Connect( ID_MENU_VIEW_POD_CHANNELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_POD_DETAILS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );
    Connect( ID_MENU_VIEW_POD_PROPERTIES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsProperties ), NULL, this );

    Connect( ID_MENU_VIEW_FILEBROWSER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFileBrowser ), NULL, this );

    Connect( ID_ALBUM_COVER_CHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryCoverChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_COVERUPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerPanelCoverChanged ), NULL, this );

    Connect( ID_MENU_VIEW_FULLSCREEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFullScreen ), NULL, this );
    Connect( ID_MENU_VIEW_STATUSBAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewStatusBar ), NULL, this );


    Connect( ID_VOLUMEMANAGER_MOUNT_CHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnVolumeMonitorUpdated ), NULL, this );

    Connect( ID_STATUSBAR_GAUGE_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeCreate ), NULL, this );
    Connect( ID_STATUSBAR_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ), NULL, this );
    Connect( ID_STATUSBAR_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ), NULL, this );
    Connect( ID_STATUSBAR_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ), NULL, this );
    Connect( ID_STATUSBAR_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ), NULL, this );

    Connect( ID_PLAYLIST_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListUpdated ), NULL, this );

    Connect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guMainFrame::OnPodcastItemUpdated ), NULL, this );
    Connect( ID_MAINFRAME_REMOVEPODCASTTHREAD, wxCommandEventHandler( guMainFrame::OnRemovePodcastThread ), NULL, this );

    Connect( ID_MAINFRAME_SETFORCEGAPLESS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetForceGapless ), NULL, this  );
    Connect( ID_MAINFRAME_SETAUDIOSCROBBLE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetAudioScrobble ), NULL, this  );

    m_AuiManager.Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guMainFrame::OnMainPaneClose ), NULL, this );

    m_MainNotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );
    m_MainNotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler( guMainFrame::OnPageClosed ), NULL, this );

    Connect( ID_MAINFRAME_UPDATE_SELINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateSelInfo ), NULL, this );
    Connect( ID_MAINFRAME_REQUEST_CURRENTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRequestCurrentTrack ), NULL, this );

    Connect( ID_LYRICS_LYRICFOUND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricFound ), NULL, this );
    Connect( ID_MAINFRAME_LYRICSSEARCHFIRST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricSearchFirst ), NULL, this );
    Connect( ID_MAINFRAME_LYRICSSEARCHNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricSearchNext ), NULL, this );
    Connect( ID_MAINFRAME_LYRICSSAVECHANGES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricSaveChanges ), NULL, this );
    Connect( ID_MAINFRAME_LYRICSEXECCOMMAND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricExecCommand ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guMainFrame::OnConfigUpdated ), NULL, this );

    Connect( ID_PLAYERPANEL_SETRATING_0, ID_PLAYERPANEL_SETRATING_5, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSongSetRating ), NULL, this );

    Connect( ID_MAINFRAME_WINDOW_RAISE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRaiseWindow ), NULL, this );
    Connect( ID_MAINFRAME_LOAD_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadPlayList ), NULL, this );

    Connect( ID_MENU_VIEW_PORTABLE_DEVICE, ID_MENU_VIEW_PORTABLE_DEVICE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPortableDevice ), NULL, this );
    Connect( ID_COLLECTIONS_BASE, ID_COLLECTIONS_BASE + ( guCOLLECTION_ACTION_COUNT * guCOLLECTIONS_MAXCOUNT ), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCollectionCommand ), NULL, this );
    Connect( ID_RADIO_DOUPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateRadio ), NULL, this );

    Connect( ID_MAINFRAME_MEDIAVIEWER_CLOSED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnMediaViewerClosed ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guMainFrame::~guMainFrame()
{
//    Disconnect( ID_MENU_UPDATE_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLibrary ), NULL, this );
//    Disconnect( ID_MENU_UPDATE_LIBRARYFORCED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnForceUpdateLibrary ), NULL, this );
//    Disconnect( ID_MENU_LIBRARY_ADD_PATH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAddLibraryPath ), NULL, this );
    Disconnect( ID_MENU_UPDATE_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdatePodcasts ), NULL, this );
//    Disconnect( ID_MENU_UPDATE_COVERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateCovers ), NULL, this );
    Disconnect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ), NULL, this );

//    Disconnect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ), NULL, this );
//    Disconnect( ID_JAMENDO_UPDATE_FINISHED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnJamendoUpdated ), NULL, this );
//    Disconnect( ID_LIBRARY_DOCLEANDB, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::DoLibraryClean ), NULL, this );

    Disconnect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ), NULL, this );
    Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    //Disconnect( wxEVT_ICONIZE, wxIconizeEventHandler( guMainFrame::OnIconizeWindow ), NULL, this );
    Disconnect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );

    Disconnect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ), NULL, this );
    Disconnect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_VOLUMECHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerVolumeChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_SEEKED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerSeeked ), NULL, this );

	Disconnect( ID_MAINFRAME_SELECT_LOCATION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectLocation ), NULL, this );
    Disconnect( ID_MENU_VIEW_MAIN_SHOWCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );

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
    Disconnect( ID_MENU_HELP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnHelp ), NULL, this );
    Disconnect( ID_MENU_COMMUNITY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCommunity ), NULL, this );

    Disconnect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );
    Disconnect( ID_MAINFRAME_COPYTODEVICE_TRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksToDevice ), NULL, this );
    Disconnect( ID_MAINFRAME_COPYTODEVICE_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyPlayListToDevice ), NULL, this );

//    Disconnect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

    Disconnect( ID_MENU_LAYOUT_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCreateNewLayout ), NULL, this );
    Disconnect( ID_MENU_LAYOUT_LOAD, ID_MENU_LAYOUT_LOAD + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadLayout ), NULL, this );
    Disconnect( ID_MENU_LAYOUT_DELETE, ID_MENU_LAYOUT_DELETE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnDeleteLayout ), NULL, this );

    Disconnect( ID_MENU_VIEW_PLAYER_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_PLAYER_FILTERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_PLAYER_VUMETERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_PLAYER_NOTEBOOK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_MAIN_LOCATIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerShowPanel ), NULL, this );

    Disconnect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ), NULL, this );
    Disconnect( ID_MENU_VIEW_RAD_TEXTSEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_RAD_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_RAD_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRadioShowPanel ), NULL, this );

    Disconnect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ), NULL, this );
    Disconnect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ), NULL, this );

    Disconnect( ID_MENU_VIEW_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPodcasts ), NULL, this );
    Disconnect( ID_MENU_VIEW_POD_CHANNELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );
    Disconnect( ID_MENU_VIEW_POD_DETAILS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPodcastsShowPanel ), NULL, this );

    Disconnect( ID_MENU_VIEW_FILEBROWSER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFileBrowser ), NULL, this );

    Disconnect( ID_ALBUM_COVER_CHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLibraryCoverChanged ), NULL, this );

    Disconnect( ID_MENU_VIEW_FULLSCREEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewFullScreen ), NULL, this );
    Disconnect( ID_MENU_VIEW_STATUSBAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewStatusBar ), NULL, this );

    Disconnect( ID_STATUSBAR_GAUGE_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeCreate ), NULL, this );
    Disconnect( ID_STATUSBAR_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ), NULL, this );
    Disconnect( ID_STATUSBAR_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ), NULL, this );
    Disconnect( ID_STATUSBAR_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ), NULL, this );
    Disconnect( ID_STATUSBAR_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ), NULL, this );

    Disconnect( ID_PLAYLIST_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListUpdated ), NULL, this );

    Disconnect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guMainFrame::OnPodcastItemUpdated ), NULL, this );
    Disconnect( ID_MAINFRAME_REMOVEPODCASTTHREAD, wxCommandEventHandler( guMainFrame::OnRemovePodcastThread ), NULL, this );

    Disconnect( ID_MAINFRAME_SETFORCEGAPLESS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetForceGapless ), NULL, this  );
    Disconnect( ID_MAINFRAME_SETAUDIOSCROBBLE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSetAudioScrobble ), NULL, this  );

    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guMainFrame::OnMainPaneClose ), NULL, this );

    m_MainNotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );
    m_MainNotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler( guMainFrame::OnPageClosed ), NULL, this );

    Disconnect( ID_MAINFRAME_UPDATE_SELINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateSelInfo ), NULL, this );
    Disconnect( ID_MAINFRAME_REQUEST_CURRENTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRequestCurrentTrack ), NULL, this );

    Disconnect( ID_LYRICS_LYRICFOUND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricFound ), NULL, this );
    Disconnect( ID_MAINFRAME_LYRICSSEARCHFIRST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricSearchFirst ), NULL, this );
    Disconnect( ID_MAINFRAME_LYRICSSEARCHNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricSearchNext ), NULL, this );
    Disconnect( ID_MAINFRAME_LYRICSSAVECHANGES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLyricSaveChanges ), NULL, this );
    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guMainFrame::OnConfigUpdated ), NULL, this );

    Disconnect( ID_MAINFRAME_WINDOW_RAISE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRaiseWindow ), NULL, this );
    Disconnect( ID_MAINFRAME_LOAD_PLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadPlayList ), NULL, this );

    Disconnect( ID_MENU_VIEW_PORTABLE_DEVICE, ID_MENU_VIEW_PORTABLE_DEVICE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPortableDevice ), NULL, this );
    Disconnect( ID_COLLECTIONS_BASE, ID_COLLECTIONS_BASE + ( guCOLLECTION_ACTION_COUNT * 24 ), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCollectionCommand ), NULL, this );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        wxPoint MainWindowPos = GetPosition();
        Config->WriteNum( wxT( "PosX" ), MainWindowPos.x, wxT( "mainwindow/positions" ) );
        Config->WriteNum( wxT( "PosY" ), MainWindowPos.y, wxT( "mainwindow/positions" ) );
        wxSize MainWindowSize = GetSize();
        Config->WriteNum( wxT( "Width" ), MainWindowSize.x, wxT( "mainwindow/positions" ) );
        Config->WriteNum( wxT( "Height" ), MainWindowSize.y, wxT( "mainwindow/positions" ) );

        Config->WriteNum( wxT( "VisiblePanels" ), m_VisiblePanels, wxT( "mainwindow" ) );

        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
        PaneInfo.Caption( _( "Now Playing" ) );
        Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "mainwindow" ) );
        Config->WriteStr( wxT( "NotebookLayout" ), m_MainNotebook->SavePerspective(), wxT( "mainwindow" ) );

        Config->WriteBool( wxT( "ShowFullScreen" ), IsFullScreen() , wxT( "mainwindow" ) );
        Config->WriteBool( wxT( "ShowStatusBar" ), m_MainStatusBar->IsShown() , wxT( "mainwindow" ) );
    }

    if( m_TaskBarIcon )
        delete m_TaskBarIcon;

#ifdef WITH_LIBINDICATE_SUPPORT
    if( m_IndicateServer )
    {
        indicate_server_hide( m_IndicateServer );
        g_object_unref( m_IndicateServer );
    }
#endif

    // destroy the mpris object
    if( m_MPRIS )
    {
        delete m_MPRIS;
    }

    if( m_MPRIS2 )
    {
        delete m_MPRIS2;
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

    if( m_VolumeMonitor )
    {
        delete m_VolumeMonitor;
    }

    m_AuiManager.UnInit();

    if( m_CopyToThread )
    {
        m_CopyToThreadMutex.Lock();
        m_CopyToThread->Pause();
        m_CopyToThread->Delete();
        m_CopyToThread = NULL;
        m_CopyToThreadMutex.Unlock();
    }

    if( m_LyricSearchContext )
    {
        delete m_LyricSearchContext;
    }

    if( m_LyricSearchEngine )
    {
        delete m_LyricSearchEngine;
    }
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
void guMainFrame::OnVolumeMonitorUpdated( wxCommandEvent &event )
{
    guLogMessage( wxT( "guMainFrame::OnVolumeMonitorUpdated" ) );

    // a mount point have been removed
    if( !event.GetInt() )
    {
        guLogMessage( wxT( "It was unmounted..." ) );
        GMount * Mount = ( GMount * ) event.GetClientData();

        int Index;
        int Count = m_MediaViewers.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guMediaViewer * MediaViewer = m_MediaViewers[ Index ];
            int Type = MediaViewer->GetType();
            if( ( Type == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
                ( Type == guMEDIA_COLLECTION_TYPE_IPOD ) )
            {
                if( ( ( guMediaViewerPortableDeviceBase * ) MediaViewer )->IsMount( Mount ) )
                {
                    event.SetId( MediaViewer->GetBaseCommand() );
                    event.SetInt( 0 );
                    AddPendingEvent( event );

                    break;
                }
            }
        }
    }

    CollectionsUpdated();
}

// -------------------------------------------------------------------------------- //
guMediaViewer * guMainFrame::FindCollectionMediaViewer( const wxString &uniqueid )
{
    //guLogMessage( wxT( "FindCollectionMediaViewer( '%s' )" ), uniqueid.c_str() );
    int Index;
    int Count = m_MediaViewers.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        //guLogMessage( wxT( "Collection[ %i/%i ] -> '%s'" ), Index, Count, m_MediaViewers[ Index ]->GetMediaCollection()->m_UniqueId.c_str() );
        if( m_MediaViewers[ Index ]->GetMediaCollection()->m_UniqueId == uniqueid )
            return m_MediaViewers[ Index ];
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
int guMainFrame::GetMediaViewerIndex( guMediaViewer * mediaviewer )
{
    //guLogMessage( wxT( "FindCollectionMediaViewer( '%s' )" ), uniqueid.c_str() );
    int Index;
    int Count = m_MediaViewers.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_MediaViewers[ Index ] == mediaviewer )
            return Index;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
guMediaCollection * guMainFrame::FindCollection( const wxString &uniqueid )
{
    int Index;
    int Count = m_Collections.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guMediaCollection &Collection = m_Collections[ Index ];

        if( Collection.m_UniqueId == uniqueid )
            return &Collection;
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guMediaViewer * guMainFrame::FindCollectionMediaViewer( void * windowptr )
{
    int Index;
    int Count = m_MediaViewers.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_MediaViewers[ Index ] == windowptr )
            return ( guMediaViewer * ) windowptr;
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
int FindCollectionUniqueId( guMediaCollectionArray * collections, const wxString &uniqueid )
{
    int Index;
    int Count = collections->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( collections->Item( Index ).m_UniqueId == uniqueid )
            return Index;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
wxString guMainFrame::GetCollectionIconString( const wxString &uniqueid )
{
    guMediaViewerPortableDeviceBase * MediaViewer = ( guMediaViewerPortableDeviceBase *  ) FindCollectionMediaViewer( uniqueid );
    if( MediaViewer )
    {
        return MediaViewer->GetPortableMediaDevice()->IconString();
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CollectionsUpdated( void )
{
    CreateCollectionsMenu( m_MenuCollections );

    if( m_LocationPanel )
    {
        m_LocationPanel->CollectionsUpdated();
    }

    if( m_FileBrowserPanel )
    {
        m_FileBrowserPanel->CollectionsUpdated();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateCollectionsMenu( wxMenu * menu )
{
    wxMutexLocker Locker( m_CollectionsMutex );
    wxMenuItem * MenuItem;

    int CollectionBaseCommand = ID_COLLECTIONS_BASE;

    while( menu->GetMenuItemCount() )
    {
        menu->Destroy( menu->FindItemByPosition( 0 ) );
    }

//    MenuItem = new wxMenuItem( menu, -1, _( "Local Music" ), wxEmptyString, wxITEM_NORMAL );
//    menu->Append( MenuItem );
//    MenuItem->Enable( false );

    MenuItem = new wxMenuItem( menu, ID_MENU_COLLECTION_NEW, _( "New Collection" ), _( "Create a new collection" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    menu->AppendSeparator();

    //CollectionBaseCommand = ID_COLLECTIONS_BASE;
    int Index;
    int Count = m_Collections.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_Collections[ Index ].m_Type == guMEDIA_COLLECTION_TYPE_NORMAL )
        {
            CreateCollectionMenu( menu, m_Collections[ Index ], CollectionBaseCommand );
        }

        CollectionBaseCommand += guCOLLECTION_ACTION_COUNT;
    }

    //
    // File Browser
    m_MenuFileBrowser = new wxMenuItem( menu, ID_MENU_VIEW_FILEBROWSER,
                                        wxString( _( "File Browser" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_FILEBROWSER ),
                                        _( "Show/Hide the file browser panel" ), wxITEM_CHECK );
    menu->Append( m_MenuFileBrowser );
    m_MenuFileBrowser->Check( m_FileBrowserPanel );

    menu->AppendSeparator();

//    MenuItem = new wxMenuItem( menu, -1, _( "Online Music" ), wxEmptyString, wxITEM_NORMAL );
//    menu->Append( MenuItem );
//    MenuItem->Enable( false );


    //
    // Radio
    wxMenu * SubMenu = new wxMenu();

    m_MenuRadios = new wxMenuItem( SubMenu, ID_MENU_VIEW_RADIO,
                    wxString( _( "Show" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_RADIO ),
                    _( "Show/Hide the radio panel" ), wxITEM_CHECK );
    SubMenu->Append( m_MenuRadios );
    m_MenuRadios->Check( m_VisiblePanels & guPANEL_MAIN_RADIOS );

    SubMenu->AppendSeparator();

    m_MenuRadTextSearch = new wxMenuItem( SubMenu, ID_MENU_VIEW_RAD_TEXTSEARCH, _( "Text Search" ), _( "Show/Hide the radio text search" ), wxITEM_CHECK );
    SubMenu->Append( m_MenuRadTextSearch );
    m_MenuRadTextSearch->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_TEXTSEARCH ) );
    m_MenuRadTextSearch->Enable( m_VisiblePanels & guPANEL_MAIN_RADIOS );

//    m_MenuRadLabels = new wxMenuItem( SubMenu, ID_MENU_VIEW_RAD_LABELS, _( "Labels" ), _( "Show/Hide the radio labels" ), wxITEM_CHECK );
//    SubMenu->Append( m_MenuRadLabels );
//    m_MenuRadLabels->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_LABELS ) );
//    m_MenuRadLabels->Enable( m_VisiblePanels & guPANEL_MAIN_RADIOS );

    m_MenuRadGenres = new wxMenuItem( SubMenu, ID_MENU_VIEW_RAD_GENRES, _( "Genres" ), _( "Show/Hide the radio genres" ), wxITEM_CHECK );
    SubMenu->Append( m_MenuRadGenres );
    m_MenuRadGenres->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_GENRES ) );
    m_MenuRadGenres->Enable( m_VisiblePanels & guPANEL_MAIN_RADIOS );

    SubMenu->AppendSeparator();

    MenuItem = new wxMenuItem( SubMenu, ID_RADIO_DOUPDATE, _( "Update" ), _( "Update the radio stations" ) );
	SubMenu->Append( MenuItem );
    MenuItem->Enable( m_VisiblePanels & guPANEL_MAIN_RADIOS );

    SubMenu->AppendSeparator();

	MenuItem = new wxMenuItem( SubMenu, ID_MENU_VIEW_RAD_PROPERTIES,
                                _( "Properties" ),
                                _( "Set the radio preferences" ), wxITEM_NORMAL );
	SubMenu->Append( MenuItem );

    menu->AppendSubMenu( SubMenu, _( "Radio" ), _( "Set the radio visible panels" ) );

    //
    // Podcasts
    SubMenu = new wxMenu();

    m_MenuPodcasts = new wxMenuItem( SubMenu, ID_MENU_VIEW_PODCASTS,
                                        wxString( _( "Show" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_PODCASTS ),
                                        _( "Show/Hide the podcasts panel" ), wxITEM_CHECK );
    SubMenu->Append( m_MenuPodcasts );
    m_MenuPodcasts->Check( m_VisiblePanels & guPANEL_MAIN_PODCASTS );

    SubMenu->AppendSeparator();

    m_MenuPodChannels = new wxMenuItem( SubMenu, ID_MENU_VIEW_POD_CHANNELS, _( "Channels" ), _( "Show/Hide the podcasts channels" ), wxITEM_CHECK );
    SubMenu->Append( m_MenuPodChannels );
    m_MenuPodChannels->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_CHANNELS ) );
    m_MenuPodChannels->Enable( m_VisiblePanels & guPANEL_MAIN_PODCASTS );

    m_MenuPodDetails = new wxMenuItem( SubMenu, ID_MENU_VIEW_POD_DETAILS, _( "Details" ), _( "Show/Hide the podcasts details" ), wxITEM_CHECK );
    SubMenu->Append( m_MenuPodDetails );
    m_MenuPodDetails->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_DETAILS ) );
    m_MenuPodDetails->Enable( m_VisiblePanels & guPANEL_MAIN_PODCASTS );

    SubMenu->AppendSeparator();

	m_MenuPodUpdate = new wxMenuItem( SubMenu, ID_MENU_UPDATE_PODCASTS,
                                wxString( _( "Update" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_UPDATE_PODCASTS ),
                                _( "Update the podcasts added" ), wxITEM_NORMAL );
	SubMenu->Append( m_MenuPodUpdate );
    m_MenuPodUpdate->Enable( m_VisiblePanels & guPANEL_MAIN_PODCASTS );

    SubMenu->AppendSeparator();

	MenuItem = new wxMenuItem( SubMenu, ID_MENU_VIEW_POD_PROPERTIES,
                                _( "Properties" ),
                                _( "Set the podcasts preferences" ), wxITEM_NORMAL );
	SubMenu->Append( MenuItem );

    menu->AppendSubMenu( SubMenu, _( "Podcasts" ), _( "Set the podcasts visible panels" ) );


    CollectionBaseCommand = ID_COLLECTIONS_BASE;
    for( Index = 0; Index < Count; Index++ )
    {
        if( ( m_Collections[ Index ].m_Type == guMEDIA_COLLECTION_TYPE_JAMENDO ) ||
            ( m_Collections[ Index ].m_Type == guMEDIA_COLLECTION_TYPE_MAGNATUNE ) )
        {
            CreateCollectionMenu( menu, m_Collections[ Index ], CollectionBaseCommand, m_Collections[ Index ].m_Type );
        }
        CollectionBaseCommand += guCOLLECTION_ACTION_COUNT;
    }

	MenuItem = new wxMenuItem( menu, ID_MENU_PLAY_STREAM,
                               wxString( _( "Play Stream" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_PLAY_STREAM ),
                               _( "Play a online music stream" ), wxITEM_NORMAL );
	menu->Append( MenuItem );

    menu->AppendSeparator();

//    MenuItem = new wxMenuItem( menu, -1, _( "Portable Devices" ), wxEmptyString, wxITEM_NORMAL );
//    menu->Append( MenuItem );
//    MenuItem->Enable( false );

    int PortableDeviceMenuCount = 0;

    CollectionBaseCommand = ID_COLLECTIONS_BASE;
    for( Index = 0; Index < Count; Index++ )
    {
        if( ( m_Collections[ Index ].m_Type == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
            ( m_Collections[ Index ].m_Type == guMEDIA_COLLECTION_TYPE_IPOD ) )
        {
            CreateCollectionMenu( menu, m_Collections[ Index ], CollectionBaseCommand, m_Collections[ Index ].m_Type );
            PortableDeviceMenuCount++;
        }

        CollectionBaseCommand += guCOLLECTION_ACTION_COUNT;
    }

    //
	if( m_VolumeMonitor )
	{
	    Count = m_VolumeMonitor->GetMountCount();
	    if( Count )
	    {
	        for( Index = 0; Index < Count; Index++ )
	        {
                guGIO_Mount * Mount = m_VolumeMonitor->GetMount( Index );
	            guLogMessage( wxT( "Mount: '%s' => '%s'" ), Mount->GetName().c_str(), Mount->GetMountPath().c_str() );

	            if( FindCollectionUniqueId( &m_Collections, Mount->GetId() ) == wxNOT_FOUND )
	            {
                    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_PORTABLE_DEVICE + Index, Mount->GetName(), _( "Show the selected portable devices" ), wxITEM_NORMAL );
                    menu->Append( MenuItem );
	            }
	        }
	    }
	    else if( !PortableDeviceMenuCount )
	    {
            MenuItem = new wxMenuItem( menu, -1, _( "No Device Found" ), _( "No portable device was detected" ), wxITEM_NORMAL );
            menu->Append( MenuItem );
            MenuItem->Enable( false );
	    }
	}

	menu->AppendSeparator();

	MenuItem = new wxMenuItem( menu, ID_MENU_QUIT,
                                wxString( _( "Quit" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_QUIT ),
                                _( "Exits the application" ), wxITEM_NORMAL );
	menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
bool guMainFrame::IsCollectionPresent( const wxString &uniqueid )
{
    return m_VolumeMonitor && m_VolumeMonitor->GetMountById( uniqueid );
}

// -------------------------------------------------------------------------------- //
bool guMainFrame::IsCollectionActive( const wxString &uniqueid )
{
    guMediaViewer * MediaViewer = FindCollectionMediaViewer( uniqueid );
    return MediaViewer && ( MediaViewer->GetViewMode() != guMEDIAVIEWER_MODE_NONE );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateCollectionMenu( wxMenu * menu, const guMediaCollection &collection, const int basecommand, int collectiontype )
{
    wxMenu * Menu = new wxMenu();

    //guLogMessage( wxT( "CreateCollectionMenu( '%s' : '%s' )" ), collection.m_UniqueId.c_str(), collection.m_Name.c_str() );

    guMediaViewer * MediaViewer = FindCollectionMediaViewer( collection.m_UniqueId );
    int ViewMode = MediaViewer ? MediaViewer->GetViewMode() : guMEDIAVIEWER_MODE_NONE;
    bool IsEnabled;
    bool IsPresent = true;
    if( ( collectiontype == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
        ( collectiontype == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        guGIO_Mount * Mount = NULL;
        if( m_VolumeMonitor )
            Mount = m_VolumeMonitor->GetMountById( collection.m_UniqueId );
        if( !Mount )
            IsPresent = false;
    }
    IsEnabled = IsPresent && ( ViewMode != guMEDIAVIEWER_MODE_NONE );
    int VisiblePanels = 0;
    if( MediaViewer && ( ViewMode == guMEDIAVIEWER_MODE_LIBRARY ) )
    {
        VisiblePanels = MediaViewer->GetLibPanel()->VisiblePanels();
    }

    wxMenuItem * MenuItem = new wxMenuItem( menu, basecommand, _( "Show" ), _( "Open the music collection" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( IsEnabled );
    MenuItem->Enable( IsPresent );

    Menu->AppendSeparator();

    //
    //
    if( IsEnabled )
    {
        wxMenu * SubMenu = new wxMenu();

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIBRARY, _( "Show" ), _( "View the collection library" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        //MenuItem->Enable( IsEnabled );
        MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );

        SubMenu->AppendSeparator();

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_LABELS, _( "Labels" ), _( "View the library labels" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_LABELS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_GENRES, _( "Genres" ), _( "View the library genres" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_GENRES );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_ARTISTS, _( "Artists" ), _( "View the library artists" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_ARTISTS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_COMPOSERS, _( "Composers" ), _( "View the library composers" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_COMPOSERS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_ALBUMARTISTS, _( "Album Artists" ), _( "View the library album artists" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_ALBUMARTISTS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_ALBUMS, _( "Albums" ), _( "View the library albums" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_ALBUMS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_YEARS, _( "Years" ), _( "View the library years" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_YEARS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_RATINGS, _( "Ratings" ), _( "View the library ratings" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_RATINGS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_PLAYCOUNT, _( "Plays" ), _( "View the library play counts" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_PLAYCOUNT );

        Menu->AppendSubMenu( SubMenu, _( "Library" ) );
    }
    else
    {
        MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_VIEW_LIBRARY, _( "Library" ), _( "View the collection library" ), wxITEM_CHECK );
        Menu->Append( MenuItem );
        MenuItem->Enable( false );
        MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
    }

    MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_VIEW_ALBUMBROWSER, _( "Album Browser" ), _( "View the collection album browser" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );
    MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER );

    MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_VIEW_TREEVIEW, _( "Tree" ), _( "View the collection tree view" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );
    MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_TREEVIEW );

    MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_VIEW_PLAYLISTS, _( "Playlists" ), _( "View the collection playlists" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );
    MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_PLAYLISTS );

    if( ( collection.m_Type == guMEDIA_COLLECTION_TYPE_NORMAL ) ||
        ( collection.m_Type == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
        ( collection.m_Type == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_ADD_PATH, _( "Add Path" ), _( "Add path to the collection" ), wxITEM_NORMAL );
        Menu->Append( MenuItem );
        MenuItem->Enable( IsEnabled );

        MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_IMPORT, _( "Import Files" ), _( "Import files into the collection" ), wxITEM_NORMAL );
        Menu->Append( MenuItem );
        MenuItem->Enable( IsEnabled );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_UPDATE_LIBRARY,
                              _( "Update" ),
                              _( "Update the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );

    MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_RESCAN_LIBRARY,
                              _( "Rescan" ),
                              _( "Rescan the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );

    MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_SEARCH_COVERS,
                              _( "Search Covers" ),
                              _( "Search the collection missing covers" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );

    if( ( collectiontype == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) || ( collectiontype == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_UNMOUNT, _( "Unmount" ), _( "Unmount the device" ) );
        Menu->Append( MenuItem );
        MenuItem->Enable( IsPresent );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, basecommand + guCOLLECTION_ACTION_VIEW_PROPERTIES, _( "Properties" ), _( "Show collection properties" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );
    if( ( collectiontype == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) || ( collectiontype == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        MenuItem->Enable( IsEnabled );
    }

    menu->AppendSubMenu( Menu, collection.m_Name );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateViewMenu( wxMenu * menu )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MENU_LAYOUT_CREATE,
                    wxString( _( "Save Layout" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_LAYOUT_CREATE ),
                    _( "Saves the current layout" ) );
    menu->Append( MenuItem );

    CreateLayoutMenus( menu );

    menu->AppendSeparator();

    //
    // Player PlayList
    m_MenuPlayerPlayList = new wxMenuItem( menu, ID_MENU_VIEW_PLAYER_PLAYLIST,
                    wxString( _( "Player Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_PLAYER_PLAYLIST ),
                    _( "Show/Hide the player playlist panel" ), wxITEM_CHECK );
    menu->Append( m_MenuPlayerPlayList );
    m_MenuPlayerPlayList->Check( m_PlayerPlayList );

    //
    // Locations
    m_MenuMainLocations = new wxMenuItem( menu, ID_MENU_VIEW_MAIN_LOCATIONS,
                    wxString( _( "Sources" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_MAIN_LOCATIONS ),
                    _( "Show/Hide the locatons" ), wxITEM_CHECK );
    menu->Append( m_MenuMainLocations );
    m_MenuMainLocations->Check( m_LocationPanel );

    //
    // Player Filters
    m_MenuPlayerFilters = new wxMenuItem( menu, ID_MENU_VIEW_PLAYER_FILTERS,
                    wxString( _( "Player Filters" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_PLAYER_FILTERS ),
                    _( "Show/Hide the player filters panel" ), wxITEM_CHECK );
    menu->Append( m_MenuPlayerFilters );
    m_MenuPlayerFilters->Check( m_PlayerFilters );

    //
    // Player Vumeters
    m_MenuPlayerVumeters = new wxMenuItem( menu, ID_MENU_VIEW_PLAYER_VUMETERS,
                    wxString( _( "VU Meters" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_PLAYER_VUMETERS ),
                    _( "Show/Hide the player vumeter" ), wxITEM_CHECK );
    menu->Append( m_MenuPlayerVumeters );
    m_MenuPlayerVumeters->Check( m_PlayerVumeters );

    //
    // Main Cover
    m_MenuMainShowCover = new wxMenuItem( menu, ID_MENU_VIEW_MAIN_SHOWCOVER,
                    wxString( _( "Cover" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_MAIN_SHOWCOVER ),
                    _( "Show/Hide the cover" ), wxITEM_CHECK );
    menu->Append( m_MenuMainShowCover );
    m_MenuMainShowCover->Check( m_CoverPanel );


    menu->AppendSeparator();

    //
    // Last.fm
    m_MenuLastFM = new wxMenuItem( menu, ID_MENU_VIEW_LASTFM,
                                        wxString( _( "Last.fm" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_LASTFM ),
                                        _( "Show/Hide the Last.fm panel" ), wxITEM_CHECK );
    menu->Append( m_MenuLastFM );
    m_MenuLastFM->Check( m_VisiblePanels & guPANEL_MAIN_LASTFM );

    //
    // Lyrics
    m_MenuLyrics = new wxMenuItem( menu, ID_MENU_VIEW_LYRICS,
                                        wxString( _( "Lyrics" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_LYRICS ),
                                        _( "Show/Hide the lyrics panel" ), wxITEM_CHECK );
    menu->Append( m_MenuLyrics );
    m_MenuLyrics->Check( m_VisiblePanels & guPANEL_MAIN_LYRICS );

    //
    // Other options
    menu->AppendSeparator();

    m_MenuFullScreen = new wxMenuItem( menu, ID_MENU_VIEW_FULLSCREEN,
                                    wxString( _( "Full Screen" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_FULLSCREEN ),
                                    _( "Show/Restore the main window in full screen" ), wxITEM_CHECK );
    menu->Append( m_MenuFullScreen );
    m_MenuFullScreen->Check( Config->ReadBool( wxT( "ShowFullScreen" ), false, wxT( "mainwindow" ) ) );

    m_MenuStatusBar = new wxMenuItem( menu, ID_MENU_VIEW_STATUSBAR,
                                    wxString( _( "Statusbar" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_STATUSBAR ),
                                    _( "Show/Hide the statusbar" ), wxITEM_CHECK );
    menu->Append( m_MenuStatusBar );
    m_MenuStatusBar->Check( Config->ReadBool( wxT( "ShowStatusBar" ), true, wxT( "mainwindow" ) ) );

    MenuItem = new wxMenuItem( menu, ID_MENU_HIDE_CAPTIONS,
                                    wxString( _( "Show Captions" ) ),
                                    _( "Show/Hide the windows caption" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( Config->ReadBool( wxT( "ShowCaptions" ), true, wxT( "mainwindow" ) ) );

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_CLOSEWINDOW,
                                    wxString( _( "Close Current Tab" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_CLOSEWINDOW ),
                                    _( "Close the current selected tab" ), wxITEM_NORMAL );

    menu->Append( MenuItem );


    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_MENU_PREFERENCES,
                                    wxString( _( "Preferences" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_PREFERENCES ),
                                    _( "View the preferences" ), wxITEM_NORMAL );

    menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateLayoutMenus( wxMenu * menu )
{
    m_MenuLayoutLoad = new wxMenu();
    m_MenuLayoutDelete = new wxMenu();

    menu->AppendSubMenu( m_MenuLayoutLoad, _( "Load Layout" ), _( "Set current view from a user defined layout" ) );
    menu->AppendSubMenu( m_MenuLayoutDelete, _( "Delete Layout" ), _( "Delete a user defined layout" ) );

    ReloadLayoutMenus();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateControlsMenu( wxMenu * menu )
{
    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_PLAYERPANEL_NEXTTRACK,
                                wxString( _( "Next Track" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_NEXTTRACK ),
                                _( "Play the next track in the playlist" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    MenuItem = new wxMenuItem( menu, ID_PLAYERPANEL_PREVTRACK,
                                wxString( _( "Prev. Track" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_PREVTRACK ),
                                _( "Play the previous track in the playlist" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_PLAYERPANEL_NEXTALBUM,
                                wxString( _( "Next Album" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_NEXTALBUM ),
                                _( "Play the next album in the playlist" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_PLAYERPANEL_PREVALBUM,
                                wxString( _( "Prev. Album" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_PREVALBUM ),
                                _( "Play the previous album in the playlist" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_PLAYERPANEL_PLAY,
                                wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_PLAY ),
                                _( "Play or Pause the current track in the playlist" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    MenuItem = new wxMenuItem( menu, ID_PLAYERPANEL_STOP,
                                wxString( _( "Stop" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_STOP ),
                                _( "Stop the current played track" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_PLAYER_PLAYLIST_STOP_ATEND,
                            wxString( _( "Stop at End" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_STOP_ATEND ),
                            _( "Stop after current playing or selected track" ) );
    menu->Append( MenuItem );

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_PLAYER_PLAYLIST_CLEAR,
                                wxString( _( "Clear Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_CLEAR ),
                                _( "Clear the now playing playlist" ), wxITEM_NORMAL );

    menu->Append( MenuItem );



    wxMenu * RatingMenu = new wxMenu();

    MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_0,
                            wxT( "☆☆☆☆☆" ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_SETRATING_0 ),
                            _( "Set the rating to 0" ), wxITEM_NORMAL );
    RatingMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_1,
                            wxT( "★☆☆☆☆" ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_SETRATING_1 ),
                            _( "Set the rating to 1" ), wxITEM_NORMAL );
    RatingMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_2,
                            wxT( "★★☆☆☆" ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_SETRATING_2 ),
                            _( "Set the rating to 2" ), wxITEM_NORMAL );
    RatingMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_3,
                            wxT( "★★★☆☆" ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_SETRATING_3 ),
                            _( "Set the rating to 3" ), wxITEM_NORMAL );
    RatingMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_4,
                            wxT( "★★★★☆" ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_SETRATING_4 ),
                            _( "Set the rating to 4" ), wxITEM_NORMAL );
    RatingMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_5,
                            wxT( "★★★★★" ) + guAccelGetCommandKeyCodeString( ID_PLAYERPANEL_SETRATING_5 ),
                            _( "Set the rating to 5" ), wxITEM_NORMAL );
    RatingMenu->Append( MenuItem );

    menu->AppendSubMenu( RatingMenu, _( "Set Rating" ), _( "Set the rating of the current track" ) );

    menu->AppendSeparator();

    m_MenuPlaySmart = new wxMenuItem( menu, ID_PLAYER_PLAYLIST_SMARTPLAY,
                                wxString( _( "Smart Play" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SMARTPLAY ),
                                _( "Update playlist based on Last.fm statics" ), wxITEM_CHECK );
    menu->Append( m_MenuPlaySmart );
    m_MenuPlaySmart->Check( m_PlayerPanel->GetPlaySmart() );

    MenuItem = new wxMenuItem( menu, ID_PLAYER_PLAYLIST_RANDOMPLAY,
                              wxString( _( "Randomize" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_RANDOMPLAY ),
                              _( "Randomize the playlist" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    m_MenuLoopPlayList = new wxMenuItem( menu, ID_PLAYER_PLAYLIST_REPEATPLAYLIST,
                                wxString( _( "Repeat Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_REPEATPLAYLIST ),
                                _( "Repeat the tracks in the playlist" ), wxITEM_CHECK );
    menu->Append( m_MenuLoopPlayList );
    m_MenuLoopPlayList->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );

    m_MenuLoopTrack = new wxMenuItem( menu, ID_PLAYER_PLAYLIST_REPEATTRACK,
                                wxString( _( "Repeat Track" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_REPEATTRACK ),
                                _( "Repeat the current track in the playlist" ), wxITEM_CHECK );
    menu->Append( m_MenuLoopTrack );
    m_MenuLoopTrack->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_MENU_VOLUME_UP,
                                wxString( _( "Volume Up" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VOLUME_UP ),
                                _( "Increment the volume level" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_MENU_VOLUME_DOWN,
                                wxString( _( "Volume Down" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VOLUME_DOWN ),
                                _( "Decrement the volume level" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    menu->AppendSeparator();

    m_MenuForceGapless = new wxMenuItem( menu, ID_MAINFRAME_SETFORCEGAPLESS,
                                wxString( _( "Force Gapless Mode" ) ) + guAccelGetCommandKeyCodeString( ID_MAINFRAME_SETFORCEGAPLESS ),
                                _( "Set playback in gapless mode" ), wxITEM_CHECK );
    menu->Append( m_MenuForceGapless );
    m_MenuForceGapless->Check( m_PlayerPanel->GetForceGapless() );

    m_MenuAudioScrobble = new wxMenuItem( menu, ID_MAINFRAME_SETAUDIOSCROBBLE,
                                wxString( _( "Audioscrobble" ) ) + guAccelGetCommandKeyCodeString( ID_MAINFRAME_SETAUDIOSCROBBLE ),
                                _( "Send played tracks information" ), wxITEM_CHECK );
    menu->Append( m_MenuAudioScrobble );

    m_MenuAudioScrobble->Check( m_PlayerPanel->GetAudioScrobbleEnabled() );
}

//// -------------------------------------------------------------------------------- //
//void guMainFrame::CreateActionsMenu( wxMenu * menu )
//{
//	//menu->AppendSeparator();
//}
//
// -------------------------------------------------------------------------------- //
void guMainFrame::CreateHelpMenu( wxMenu * menu )
{
    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MENU_HELP,
                              wxString( _( "Help" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_HELP ),
                              _( "Get help using guayadeque" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_MENU_COMMUNITY,
                              wxString( _( "Community" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_COMMUNITY ),
                              _( "Get guayadeque support from guayadeque.org" ), wxITEM_NORMAL );
    menu->Append( MenuItem );

    menu->AppendSeparator();
    MenuItem = new wxMenuItem( menu, ID_MENU_ABOUT,
                              _( "About" ),
                              _( "Show information about guayadeque music player" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateMenu()
{
	wxMenuBar *     MenuBar;
	wxMenu *        MenuEntry;

    // Create the main Menu Bar
	MenuBar = new wxMenuBar( 0 );

    //
    // Now create the Collections Menu Entry
    m_MenuCollections = new wxMenu();

    CreateCollectionsMenu( m_MenuCollections );

	MenuBar->Append( m_MenuCollections, _( "Sources" ) );

    //
    // Now create the View Menu Entry
    MenuEntry = new wxMenu();

    CreateViewMenu( MenuEntry );

	MenuBar->Append( MenuEntry, _( "View" ) );

    //
    // Now create the Controls Menu Entry
    MenuEntry = new wxMenu();

    CreateControlsMenu( MenuEntry );

	MenuBar->Append( MenuEntry, _( "Controls" ) );

    //
    // Now create the Help Menu Entry
    MenuEntry = new wxMenu();

    CreateHelpMenu( MenuEntry );

	MenuBar->Append( MenuEntry, _( "Help" ) );


	SetMenuBar( MenuBar );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPreferences( wxCommandEvent &event )
{
    int Page;
    switch( event.GetId() )
    {
        case ID_MENU_COLLECTION_NEW  :
            Page = guPREFERENCE_PAGE_LIBRARY;
            break;

        case ID_MENU_PREFERENCES_COMMANDS :
            Page = guPREFERENCE_PAGE_COMMANDS;
            break;

        case ID_MENU_PREFERENCES_COPYTO :
            Page = guPREFERENCE_PAGE_COPYTO;
            break;

        case ID_MENU_PREFERENCES_LINKS :
            Page = guPREFERENCE_PAGE_LINKS;
            break;

        default :
            Page = event.GetInt() ? event.GetInt() : wxNOT_FOUND;
    }

    guPrefDialog * PrefDialog = new guPrefDialog( this, m_Db, Page );
    if( PrefDialog )
    {
        if( PrefDialog->ShowModal() == wxID_OK )
        {
            PrefDialog->SaveSettings();

            CreateTaskBarIcon();

            guConfig * Config = ( guConfig * ) guConfig::Get();
            Config->SendConfigChangedEvent( PrefDialog->GetVisiblePanels() );

            //m_Db->ConfigChanged();
        }
        PrefDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCloseWindow( wxCloseEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

#ifdef WITH_LIBINDICATE_SUPPORT
    if( m_IndicateServer )
    {
        guMediaState State = m_PlayerPanel->GetState();
        if( State == guMEDIASTATE_PLAYING )
        {
            if( event.CanVeto() )
            {
                Show( false );
                return;
            }
        }
    }
    else
#else
    if( m_MPRIS2->Indicators_Sound_Available() )
    {
        if( Config->ReadBool( wxT( "SoundMenuIntegration" ), false, wxT( "general" ) ) )
        {
            guMediaState State = m_PlayerPanel->GetState();
            if( State == guMEDIASTATE_PLAYING )
            {
                if( event.CanVeto() )
                {
                    Show( false );
                    return;
                }
            }
        }
    }
    else
#endif
    {
        // If the icon
        if( m_TaskBarIcon &&
            Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "general" ) ) &&
            Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "general" ) ) )
        {
            if( event.CanVeto() )
            {
                Show( false );
                return;
            }
        }
        else if( Config->ReadBool( wxT( "ShowCloseConfirm" ), true, wxT( "general" ) ) )
        {
            guExitConfirmDlg * ExitConfirmDlg = new guExitConfirmDlg( this );
            if( ExitConfirmDlg )
            {
                int Result = ExitConfirmDlg->ShowModal();
                if( ExitConfirmDlg->GetConfirmChecked() )
                {
                    Config->WriteBool( wxT( "ShowCloseConfirm" ), false, wxT( "general" ) );
                }
                ExitConfirmDlg->Destroy();

                if( Result != wxID_OK )
                    return;
            }
        }
    }

    event.Skip();
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
        m_LyricsPanel->OnSetCurrentTrack( event );
    }

    if( m_LyricSearchEngine )
    {
        if( m_LyricSearchContext )
        {
            delete m_LyricSearchContext;
            m_LyricSearchContext = NULL;
        }
        if( m_LyricSearchEngine->TargetsEnabled() ||
            ( m_LyricsPanel && m_LyricsPanel->UpdateEnabled() ) )
        {
            m_LyricSearchContext = m_LyricSearchEngine->CreateContext( this, Track );
            m_LyricSearchEngine->SearchStart( m_LyricSearchContext );
        }
    }

    if( m_MPRIS )
    {
        m_MPRIS->OnPlayerTrackChange();
    }

    if( m_MPRIS2 )
    {
        m_MPRIS2->OnPlayerTrackChange();
    }

    CheckPendingUpdates( Track );

    if( Track )
    {
        delete Track;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerStatusChanged( wxCommandEvent &event )
{
    if( m_MPRIS )
    {
        m_MPRIS->OnPlayerStatusChange();
    }

    if( m_MPRIS2 )
    {
        m_MPRIS2->OnPlayerStatusChange();
    }

    if( m_PlayerPanel )
    {
        m_MenuPlaySmart->Check( m_PlayerPanel->GetPlaySmart() );
        m_MenuLoopPlayList->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );
        m_MenuLoopTrack->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerTrackListChanged( wxCommandEvent &event )
{
    if( m_MPRIS )
    {
        m_MPRIS->OnTrackListChange();
    }

    if( m_MPRIS2 )
    {
        m_MPRIS2->OnTrackListChange();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerCapsChanged( wxCommandEvent &event )
{
    if( m_MPRIS )
    {
        m_MPRIS->OnPlayerCapsChange();
    }

    if( m_MPRIS2 )
    {
        m_MPRIS2->OnPlayerCapsChange();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerVolumeChanged( wxCommandEvent &event )
{
    if( m_MPRIS2 )
    {
        m_MPRIS2->OnPlayerVolumeChange();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerSeeked( wxCommandEvent &event )
{
    if( m_MPRIS2 )
    {
        m_MPRIS2->OnPlayerSeeked( event.GetInt() );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnAudioScrobbleUpdate( wxCommandEvent &event )
{
    //guLogMessage( wxT( "######## OnAudioScrobbleUpdate ( %i ) ########" ), event.GetInt() );
    if( m_MainStatusBar )
    {
        m_MainStatusBar->UpdateAudioScrobbleIcon( !event.GetInt() );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnQuit( wxCommandEvent &event )
{
    Close( true );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCloseTab( wxCommandEvent &event )
{
    m_CurrentPage = m_MainNotebook->GetPage( m_MainNotebook->GetSelection() );
    if( m_CurrentPage )
    {
        DoPageClose( ( wxPanel * ) m_CurrentPage );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnShowCaptions( wxCommandEvent &event )
{
    bool Visible = event.GetInt();
    //guLogMessage( wxT( "guMainFrame::OnShowCaptions( %i )" ) );

    wxAuiPaneInfoArray &PaneInfoArray = m_AuiManager.GetAllPanes();

    int Index;
    int Count = PaneInfoArray.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        wxAuiPaneInfo &PaneInfo = PaneInfoArray[ Index ];
        if( PaneInfo.dock_direction != wxAUI_DOCK_CENTER )
        {
            if( PaneInfo.name == wxT( "PlayerPlayList" ) )
                continue;
            PaneInfo.CaptionVisible( Visible );
        }
    }
    m_AuiManager.Update();
    //
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( wxT( "ShowCaptions" ), Visible, wxT( "mainwindow" ) );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnChangeVolume( wxCommandEvent &event )
{
    if( m_PlayerPanel )
    {
        double CurVolume = m_PlayerPanel->GetVolume();
        //guLogMessage( wxT( "CurVolume: %.2f" ), CurVolume );
        m_PlayerPanel->SetVolume( ( event.GetId() == ID_MENU_VOLUME_UP ) ? CurVolume + 4 : CurVolume - 4 );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayStream( wxCommandEvent &event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Stream:" ),
                                            _( "Play Stream" ), wxEmptyString );
    if( EntryDialog->ShowModal() == wxID_OK && !EntryDialog->GetValue().IsEmpty() )
    {
        wxArrayString Streams;
        Streams.Add( EntryDialog->GetValue() );

        m_PlayerPanel->SetPlayList( Streams );
    }
    EntryDialog->Destroy();
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
void guMainFrame::OnStopAtEnd( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnStopAtEnd( event );
}

// ---------------------------------------------------------------------- //
void guMainFrame::OnClearPlaylist( wxCommandEvent &event )
{
    if( m_PlayerPlayList )
    {
        m_PlayerPlayList->GetPlayListCtrl()->ClearItems();
    }
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
        m_MenuPlaySmart->Check( m_PlayerPanel->GetPlaySmart() );
        m_MenuLoopPlayList->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );
        m_MenuLoopTrack->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );
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
        m_MenuPlaySmart->Check( m_PlayerPanel->GetPlaySmart() );
        m_MenuLoopPlayList->Check( RepeatMode == guPLAYER_PLAYLOOP_PLAYLIST );
        m_MenuLoopTrack->Check( RepeatMode == guPLAYER_PLAYLOOP_TRACK );
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
void guMainFrame::OnHelp( wxCommandEvent &event )
{
    guWebExecute( wxT( "http://guayadeque.org/forums/index.php?p=/page/manual#TableOfContent" ) );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCommunity( wxCommandEvent &event )
{
    guWebExecute( wxT( "http://guayadeque.org/forums/index.php?p=/discussions" ) );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerPlayListUpdateTitle( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnPlayerPlayListUPdateTitle..." ) );
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
            m_CopyToThreadMutex.Lock();

            if( !m_CopyToThread )
            {
                int GaugeId = m_MainStatusBar->AddGauge( _( "Copy To..." ), false );
                m_CopyToThread = new guCopyToThread( this, GaugeId );
            }

            guConfig * Config = ( guConfig * ) guConfig::Get();
            wxArrayString CopyToOptions = Config->ReadAStr( wxT( "Option"), wxEmptyString, wxT( "copyto/options") );
            wxArrayString SelCopyTo = wxStringTokenize( CopyToOptions[ event.GetInt() ], wxT( ":") );

            while( SelCopyTo.Count() != 6 )
                SelCopyTo.Add( wxEmptyString );

            wxString DestDir = SelCopyTo[ 5 ];

            if( DestDir.IsEmpty() )
            {
                wxDirDialog * DirDialog = new wxDirDialog( this,
                    _( "Select destination directory" ), wxGetHomeDir(), wxDD_DIR_MUST_EXIST );
                if( DirDialog )
                {
                    if( DirDialog->ShowModal() == wxID_OK )
                    {
                        DestDir = DirDialog->GetPath() + wxT( "/" );
                    }
                    DirDialog->Destroy();
                }
            }

            if( !DestDir.IsEmpty() )
            {
                guMediaViewer * MediaViewer = Tracks->Item( 0 ).m_MediaViewer;
                m_CopyToThread->AddAction( Tracks, MediaViewer, DestDir,
                            unescape_configlist_str( SelCopyTo[ 1 ] ),
                            wxAtoi( SelCopyTo[ 2 ] ),
                            wxAtoi( SelCopyTo[ 3 ] ),
                            wxAtoi( SelCopyTo[ 4 ] ) );
            }
            else
            {
                delete Tracks;
            }

            m_CopyToThreadMutex.Unlock();
        }
        else
        {
            delete Tracks;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCopyTracksToDevice( wxCommandEvent &event )
{
    guLogMessage( wxT( "guMainFrame::OnCopyTracksToDevice... %i" ), event.GetInt() );
    guTrackArray * Tracks = ( guTrackArray * ) event.GetClientData();
    if( Tracks )
    {
        if( Tracks->Count() )
        {
            int PortableIndex = event.GetInt();
            if( PortableIndex >= 0 && PortableIndex < ( int ) m_Collections.Count() )
            {
                guMediaViewer * MediaViewer = m_MediaViewers[ PortableIndex ];
                if( MediaViewer )
                {
                    guLogMessage( wxT( "MediaViewer with collection id '%s' for CopyTracks found" ), m_Collections[ PortableIndex ].m_UniqueId.c_str() );
                    m_CopyToThreadMutex.Lock();

                    if( !m_CopyToThread )
                    {
                        int GaugeId = m_MainStatusBar->AddGauge( _( "Copy To..." ), false );
                        m_CopyToThread = new guCopyToThread( this, GaugeId );
                    }

                    m_CopyToThread->AddAction( Tracks, MediaViewer );

                    m_CopyToThreadMutex.Unlock();

                    return;
                }
            }
            else
            {
                guLogMessage( wxT( "Wrong portable device index in copy tracks to device command" ) );
            }
        }

        delete Tracks;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::ImportFiles( guMediaViewer * mediaviewer, guTrackArray * tracks, const wxString &copytooption, const wxString &destdir )
{
    guCopyToPattern * CopyToPattern = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString CopyToOptions = Config->ReadAStr( wxT( "Option" ), wxEmptyString, wxT( "copyto/options" ) );

    int Index;
    int Count;
    if( ( Count = CopyToOptions.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( CopyToOptions[ Index ].BeforeFirst( wxT( ':' ) ) == copytooption )
            {
                CopyToPattern = new guCopyToPattern( CopyToOptions[ Index ] );
                break;
            }
        }
    }

    if( CopyToPattern )
    {
        m_CopyToThreadMutex.Lock();

        if( !m_CopyToThread )
        {
            int GaugeId = m_MainStatusBar->AddGauge( _( "Copy To..." ), false );
            m_CopyToThread = new guCopyToThread( this, GaugeId );
        }

        m_CopyToThread->AddAction( tracks, mediaviewer, destdir,
                    CopyToPattern->m_Pattern,
                    CopyToPattern->m_Format,
                    CopyToPattern->m_Quality,
                    CopyToPattern->m_MoveFiles );

        m_CopyToThreadMutex.Unlock();

        delete CopyToPattern;
    }
    else
    {
        guLogMessage( wxT( "Could not find the copy to option selected" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCopyPlayListToDevice( wxCommandEvent &event )
{
    guLogMessage( wxT( "guMainFrame::OnCopyPlayListToDevice" ) );
    wxString * PlayListPath = ( wxString * ) event.GetClientData();
    if( PlayListPath )
    {
        int PortableIndex = event.GetInt();
        if( PortableIndex >= 0 && PortableIndex < ( int ) m_Collections.Count() )
        {
            guMediaViewerPortableDevice * MediaViewer = ( guMediaViewerPortableDevice * ) m_MediaViewers[ PortableIndex ];
            if( MediaViewer )
            {
                m_CopyToThreadMutex.Lock();

                if( !m_CopyToThread )
                {
                    int GaugeId = m_MainStatusBar->AddGauge( _( "Copy To..." ), false );
                    m_CopyToThread = new guCopyToThread( this, GaugeId );
                }

                m_CopyToThread->AddAction( PlayListPath, MediaViewer );

                m_CopyToThreadMutex.Unlock();
            }
        }
        else
        {
            guLogMessage( wxT( "Wrong portable device index in copy playlist to device command" ) );
        }
    }
}

//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnUpdateLabels( wxCommandEvent &event )
//{
////    if( m_LibPanel )
////    {
////        m_LibPanel->UpdateLabels();
////    }
//}

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
    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_MainNotebook );
    if( !PaneInfo.IsShown() )
    {
        PaneInfo.Show();
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CheckHideNotebook( void )
{
    if( !m_MainNotebook->GetPageCount() )
    {
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_MainNotebook );
        PaneInfo.Hide();
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::RemoveTabPanel( wxPanel * panel )
{
    int PageIndex = m_MainNotebook->GetPageIndex( panel );
    if( PageIndex != wxNOT_FOUND )
    {
        if( m_MainNotebook->GetPageCount() > 1 )
        {
            m_MainNotebook->RemovePage( PageIndex );
        }
        else
        {
            guConfig * Config = ( guConfig * ) guConfig::Get();
            Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "mainwindow/notebook" ) );
            wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_MainNotebook );
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
void guMainFrame::InsertTabPanel( wxPanel * panel, const int index, const wxString &label, const wxString &panelid )
{
    int PageIndex = m_MainNotebook->GetPageIndex( panel );
    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_MainNotebook );
    if( !PaneInfo.IsShown() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();

        LoadPerspective( Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "mainwindow/notebook" ) ) );
        wxWindow * OldPage = m_MainNotebook->GetPage( 0 );
        // Was hidden
        if( PageIndex == wxNOT_FOUND )
        {
            m_MainNotebook->InsertPage( wxMin( index, ( int ) m_MainNotebook->GetPageCount() ), panel, label, true );
            m_MainNotebook->AddId( panel, panelid );
            int OldIndex = m_MainNotebook->GetPageIndex( OldPage );
            m_MainNotebook->RemovePage( OldIndex );

            if( m_DestroyLastWindow )
            {
                delete ( ( guMediaViewer * ) OldPage );
                m_DestroyLastWindow = false;
            }
        }
        PaneInfo.Show();
        m_AuiManager.Update();
    }
    else
    {
        m_MainNotebook->InsertPage( wxMin( index, ( int ) m_MainNotebook->GetPageCount() ), panel, label, true );
        m_MainNotebook->AddId( panel, panelid );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCollectionCommand( wxCommandEvent &event )
{
    int CollectionIndex = ( event.GetId() - ID_COLLECTIONS_BASE ) / guCOLLECTION_ACTION_COUNT;
    int CollectionCmdId = ( event.GetId() - ID_COLLECTIONS_BASE ) % guCOLLECTION_ACTION_COUNT;

    bool IsEnabled = event.IsChecked();

    guLogMessage( wxT( "OnCollectionCommand %i  %i %i  %i" ), event.GetId(), CollectionIndex, CollectionCmdId, IsEnabled );

    guMediaCollection &Collection = m_Collections[ CollectionIndex ];
    guMediaViewer * MediaViewer = FindCollectionMediaViewer( Collection.m_UniqueId );
    if( !MediaViewer )
    {
        guLogMessage( wxT( "MediaViewer Not Found for '%s'" ), Collection.m_UniqueId.c_str() );
    }

    switch( CollectionCmdId )
    {
        case guCOLLECTION_ACTION_VIEW_COLLECTION :
        {
            if( IsEnabled )
            {
                if( !MediaViewer )
                {
                    switch( Collection.m_Type )
                    {
                        case guMEDIA_COLLECTION_TYPE_NORMAL :
                            MediaViewer = ( guMediaViewer * ) new guMediaViewerLibrary( m_MainNotebook, Collection, event.GetId(), this, -1, m_PlayerPanel );
                            break;

                        case guMEDIA_COLLECTION_TYPE_JAMENDO :
                            MediaViewer = ( guMediaViewer * ) new guMediaViewerJamendo( m_MainNotebook, Collection, event.GetId(), this, -1, m_PlayerPanel );
                            break;

                        case guMEDIA_COLLECTION_TYPE_MAGNATUNE :
                            MediaViewer = ( guMediaViewer * ) new guMediaViewerMagnatune( m_MainNotebook, Collection, event.GetId(), this, -1, m_PlayerPanel );
                            break;

                        case guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE :
                        {
                            guGIO_Mount * Mount = m_VolumeMonitor->GetMountById( Collection.m_UniqueId );
                            MediaViewer = ( guMediaViewer * ) new guMediaViewerPortableDevice( m_MainNotebook, Collection, event.GetId(), this, -1, m_PlayerPanel, Mount );
                            break;
                        }

#ifdef WITH_LIBGPOD_SUPPORT
                        case guMEDIA_COLLECTION_TYPE_IPOD :
                        {
                            guGIO_Mount * Mount = m_VolumeMonitor->GetMountById( Collection.m_UniqueId );
                            MediaViewer = ( guMediaViewer * ) new guMediaVieweriPodDevice( m_MainNotebook, Collection, event.GetId(), this, -1, m_PlayerPanel, Mount );
                            break;
                        }
#endif
                    }

                    m_MediaViewers.Add( MediaViewer );
                }

                if( MediaViewer->IsDefault() && ( MediaViewer->GetViewMode() == guMEDIAVIEWER_MODE_NONE ) )
                {
                    MediaViewer->SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );
                }

                InsertTabPanel( MediaViewer, 5, Collection.m_Name, Collection.m_UniqueId );

            }
            else
            {
                if( MediaViewer )
                {
                    DoPageClose( MediaViewer );
                }
            }

            CollectionsUpdated();
            break;
        }

        case guCOLLECTION_ACTION_VIEW_LIBRARY :
        case guCOLLECTION_ACTION_VIEW_ALBUMBROWSER :
        case guCOLLECTION_ACTION_VIEW_TREEVIEW :
        case guCOLLECTION_ACTION_VIEW_PLAYLISTS :
        {
            if( MediaViewer )
            {
                MediaViewer->SetViewMode( ( CollectionCmdId == guCOLLECTION_ACTION_VIEW_LIBRARY ) ? guMEDIAVIEWER_MODE_LIBRARY :
                             ( ( CollectionCmdId - guCOLLECTION_ACTION_VIEW_ALBUMBROWSER ) + 1 ) );
            }
            else
            {
                guLogMessage( wxT( "Got a command %i %i  %i?" ), CollectionIndex, CollectionCmdId, IsEnabled );
            }
            break;
        }

        case guCOLLECTION_ACTION_VIEW_LIB_LABELS :
        case guCOLLECTION_ACTION_VIEW_LIB_GENRES :
        case guCOLLECTION_ACTION_VIEW_LIB_ARTISTS :
        case guCOLLECTION_ACTION_VIEW_LIB_COMPOSERS :
        case guCOLLECTION_ACTION_VIEW_LIB_ALBUMARTISTS :
        case guCOLLECTION_ACTION_VIEW_LIB_ALBUMS :
        case guCOLLECTION_ACTION_VIEW_LIB_YEARS :
        case guCOLLECTION_ACTION_VIEW_LIB_RATINGS :
        case guCOLLECTION_ACTION_VIEW_LIB_PLAYCOUNT :
        {
            if( MediaViewer )
            {
                MediaViewer->ShowPanel( CollectionCmdId, IsEnabled );
            }
            break;
        }


        case guCOLLECTION_ACTION_UPDATE_LIBRARY :
        case guCOLLECTION_ACTION_RESCAN_LIBRARY :
        case guCOLLECTION_ACTION_SEARCH_COVERS :
        case guCOLLECTION_ACTION_ADD_PATH :
        case guCOLLECTION_ACTION_IMPORT :
        {
            if( MediaViewer )
            {
                MediaViewer->HandleCommand( CollectionCmdId );
            }
            break;
        }

        case guCOLLECTION_ACTION_VIEW_PROPERTIES :
        {
            if( MediaViewer )
            {
                MediaViewer->EditProperties();
            }
            else if( Collection.m_Type == guMEDIA_COLLECTION_TYPE_NORMAL )
            {
                wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
                CmdEvent.SetInt( guPREFERENCE_PAGE_LIBRARY );
                AddPendingEvent( CmdEvent );
            }
            else if( Collection.m_Type == guMEDIA_COLLECTION_TYPE_JAMENDO )
            {
                wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
                CmdEvent.SetInt( guPREFERENCE_PAGE_JAMENDO );
                AddPendingEvent( CmdEvent );
            }
            else if( Collection.m_Type == guMEDIA_COLLECTION_TYPE_MAGNATUNE )
            {
                wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
                CmdEvent.SetInt( guPREFERENCE_PAGE_MAGNATUNE );
                AddPendingEvent( CmdEvent );
            }
            break;
        }

        case guCOLLECTION_ACTION_UNMOUNT :
        {
            if( MediaViewer )
            {
                MediaViewer->HandleCommand( CollectionCmdId );
            }
            else
            {
                guGIO_Mount * Mount = m_VolumeMonitor->GetMountById( Collection.m_UniqueId );
                if( Mount && Mount->CanUnmount() )
                {
                    Mount->Unmount();
                }
            }
            break;
        }
    }
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
            m_RadioPanel = new guRadioPanel( m_MainNotebook, m_Db, m_PlayerPanel );

        InsertTabPanel( m_RadioPanel, 0, _( "Radio" ), wxT( "Radio" ) );

        m_VisiblePanels |= guPANEL_MAIN_RADIOS;
    }
    else
    {
        RemoveTabPanel( m_RadioPanel );
        m_VisiblePanels ^= guPANEL_MAIN_RADIOS;
    }
    m_MainNotebook->Refresh();

    m_MenuRadios->Check( IsEnabled );

    m_MenuRadTextSearch->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_TEXTSEARCH ) );
    m_MenuRadTextSearch->Enable( IsEnabled );

//    m_MenuRadLabels->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_LABELS ) );
//    m_MenuRadLabels->Enable( IsEnabled );

    m_MenuRadGenres->Check( m_RadioPanel && m_RadioPanel->IsPanelShown( guPANEL_RADIO_GENRES ) );
    m_MenuRadGenres->Enable( IsEnabled );

    if( m_LocationPanel )
    {
        m_LocationPanel->OnPanelVisibleChanged();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnRadioShowPanel( wxCommandEvent &event )
{
    unsigned int PanelId = 0;

    switch( event.GetId() )
    {
        case ID_MENU_VIEW_RAD_TEXTSEARCH :
            PanelId = guPANEL_RADIO_TEXTSEARCH;
            m_MenuRadTextSearch->Check( event.IsChecked() );
            break;

//        case ID_MENU_VIEW_RAD_LABELS :
//            PanelId = guPANEL_RADIO_LABELS;
//            m_MenuRadLabels->Check( event.IsChecked() );
//            break;

        case ID_MENU_VIEW_RAD_GENRES :
            PanelId = guPANEL_RADIO_GENRES;
            m_MenuRadGenres->Check( event.IsChecked() );
            break;
    }

    if( PanelId && m_RadioPanel )
        m_RadioPanel->ShowPanel( PanelId, event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnRadioProperties( wxCommandEvent &event )
{
    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_ONLINE );
    AddPendingEvent( CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPodcastsShowPanel( wxCommandEvent &event )
{
    unsigned int PanelId = 0;

    switch( event.GetId() )
    {
        case ID_MENU_VIEW_POD_CHANNELS :
            PanelId = guPANEL_PODCASTS_CHANNELS;
            m_MenuPodChannels->Check( event.IsChecked() );
            break;

        case ID_MENU_VIEW_POD_DETAILS :
            PanelId = guPANEL_PODCASTS_DETAILS;
            m_MenuPodDetails->Check( event.IsChecked() );
            break;
    }

    if( PanelId && m_PodcastsPanel )
        m_PodcastsPanel->ShowPanel( PanelId, event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPodcastsProperties( wxCommandEvent &event )
{
    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_PODCASTS );
    AddPendingEvent( CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLastFM( wxCommandEvent &event )
{
    bool IsEnabled = event.IsChecked();
    if( IsEnabled )
    {
        if( !m_LastFMPanel )
            m_LastFMPanel = new guLastFMPanel( m_MainNotebook, m_Db, m_DbCache, m_PlayerPanel );

        InsertTabPanel( m_LastFMPanel, 1, _( "Last.fm" ), wxT( "Last.fm" ) );

        m_VisiblePanels |= guPANEL_MAIN_LASTFM;
    }
    else
    {

        RemoveTabPanel( m_LastFMPanel );

        m_VisiblePanels ^= guPANEL_MAIN_LASTFM;
    }
    m_MainNotebook->Refresh();

    m_MenuLastFM->Check( IsEnabled );

    if( m_LocationPanel )
    {
        m_LocationPanel->OnPanelVisibleChanged();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLyrics( wxCommandEvent &event )
{
    bool IsEnabled = event.IsChecked();
    if( IsEnabled )
    {
        if( !m_LyricsPanel )
        {
            m_LyricsPanel = new guLyricsPanel( m_MainNotebook, m_Db, m_LyricSearchEngine );
        }

        InsertTabPanel( m_LyricsPanel, 2, _( "Lyrics" ), wxT( "Lyrics" ) );

        m_VisiblePanels |= guPANEL_MAIN_LYRICS;
    }
    else
    {
        RemoveTabPanel( m_LyricsPanel );

        m_VisiblePanels ^= guPANEL_MAIN_LYRICS;
    }
    m_MainNotebook->Refresh();

    m_MenuLyrics->Check( IsEnabled );

    if( m_LocationPanel )
    {
        m_LocationPanel->OnPanelVisibleChanged();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPodcasts( wxCommandEvent &event )
{
    bool IsEnabled = event.IsChecked();
    if( event.IsChecked() )
    {
        if( !m_PodcastsPanel )
            m_PodcastsPanel = new guPodcastPanel( m_MainNotebook, GetPodcastsDb(), this, m_PlayerPanel );

        InsertTabPanel( m_PodcastsPanel, 3, _( "Podcasts" ), wxT( "Podcasts" ) );

        m_VisiblePanels |= guPANEL_MAIN_PODCASTS;
    }
    else
    {

        RemoveTabPanel( m_PodcastsPanel );

        m_VisiblePanels ^= guPANEL_MAIN_PODCASTS;
    }
    m_MainNotebook->Refresh();

    m_MenuPodcasts->Check( IsEnabled );

    m_MenuPodChannels->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_CHANNELS ) );
    m_MenuPodChannels->Enable( IsEnabled );

    m_MenuPodDetails->Check( m_PodcastsPanel && m_PodcastsPanel->IsPanelShown( guPANEL_PODCASTS_DETAILS ) );
    m_MenuPodDetails->Enable( IsEnabled );

    m_MenuPodUpdate->Enable( IsEnabled );

    if( m_LocationPanel )
    {
        m_LocationPanel->OnPanelVisibleChanged();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewFileBrowser( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        if( !m_FileBrowserPanel )
            m_FileBrowserPanel = new guFileBrowser( m_MainNotebook, this, m_Db, m_PlayerPanel );

        InsertTabPanel( m_FileBrowserPanel, 4, _( "File Browser" ), wxT( "FileBrowser" ) );

        m_VisiblePanels |= guPANEL_MAIN_FILEBROWSER;
    }
    else
    {
        RemoveTabPanel( m_FileBrowserPanel );
        m_VisiblePanels ^= guPANEL_MAIN_FILEBROWSER;
    }
    m_MainNotebook->Refresh();

    m_MenuFileBrowser->Check( m_VisiblePanels & guPANEL_MAIN_FILEBROWSER );

    if( m_LocationPanel )
    {
        m_LocationPanel->OnPanelVisibleChanged();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPortableDevice( wxCommandEvent &event )
{
    int DeviceIndex = event.GetId() - ID_MENU_VIEW_PORTABLE_DEVICE;
    guGIO_Mount * Mount = m_VolumeMonitor->GetMount( DeviceIndex );
    if( Mount )
    {
        m_CollectionsMutex.Lock();

        guMediaCollection * MediaCollection = new guMediaCollection( guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE );
        if( GetPortableMediaType( Mount->GetMountPath() ) == guPORTABLE_MEDIA_TYPE_IPOD )
            MediaCollection->m_Type = guMEDIA_COLLECTION_TYPE_IPOD;
        MediaCollection->m_Name = Mount->GetName();
        MediaCollection->m_UniqueId = Mount->GetId();
        MediaCollection->m_Paths.Add( Mount->GetMountPath() );

        m_Collections.Add( MediaCollection );

        m_CollectionsMutex.Unlock();

        guConfig * Config = ( guConfig * ) guConfig::Get();
        Config->SaveCollections( &m_Collections );

        event.SetId( ID_COLLECTIONS_BASE + ( ( m_Collections.Count() - 1 ) * guCOLLECTION_ACTION_COUNT ) );
        event.SetInt( 1 );
        AddPendingEvent( event );

        guLogMessage( wxT( "Sent Event to open collection" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewFullScreen( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    bool IsFull = event.IsChecked();
    guLogMessage( wxT( "OnViewFullScreen %i" ), IsFull );

    if( IsFull )
    {
        // Save the normal perspective
        Config->WriteNum( wxT( "VisiblePanels" ), m_VisiblePanels, wxT( "mainwindow" ) );
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
        PaneInfo.Caption( _( "Now Playing" ) );
        Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "mainwindow" ) );

        ShowFullScreen( IsFull, wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION );

        // Restore the previous full screen layout
        m_VisiblePanels = Config->ReadNum( wxT( "VisiblePanels" ), guPANEL_MAIN_VISIBLE_DEFAULT, wxT( "mainwindow/fullscreen" ) );

        wxString Perspective = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "mainwindow/fullscreen" ) );

        LoadPerspective( Perspective );
    }
    else
    {
        // Save the full screen layout
        Config->WriteNum( wxT( "VisiblePanels" ), m_VisiblePanels, wxT( "mainwindow/fullscreen" ) );
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
        PaneInfo.Caption( _( "Now Playing" ) );
        Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "mainwindow/fullscreen" ) );

        ShowFullScreen( IsFull, wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION );

        // Restore the normal layout
        m_VisiblePanels = Config->ReadNum( wxT( "VisiblePanels" ), guPANEL_MAIN_VISIBLE_DEFAULT, wxT( "mainwindow" ) );
        Config->WriteNum( wxT( "VisiblePanels" ), m_VisiblePanels, wxT( "mainwindow" ) );

        wxString Perspective = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "mainwindow" ) );

        LoadPerspective( Perspective );
    }

    if( m_MenuFullScreen )
    {
        m_MenuFullScreen->Check( IsFull );
    }

    if( m_MPRIS2 )
    {
        m_MPRIS2->OnFullscreenChanged();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewStatusBar( wxCommandEvent &event )
{
    m_MainStatusBar->Show( event.IsChecked() );
    m_AuiManager.Update();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSetSelection( wxCommandEvent &event )
{
    guMediaViewer * MediaViewer = ( guMediaViewer * ) event.GetClientData();
    if( MediaViewer )
    {
        int Type = wxNOT_FOUND;
        switch( event.GetId() )
        {
            case ID_MAINFRAME_SELECT_TRACK       : Type = guMEDIAVIEWER_SELECT_TRACK; break;
            case ID_MAINFRAME_SELECT_ALBUM       : Type = guMEDIAVIEWER_SELECT_ALBUM; break;
            case ID_MAINFRAME_SELECT_ALBUMARTIST : Type = guMEDIAVIEWER_SELECT_ALBUMARTIST; break;
            case ID_MAINFRAME_SELECT_COMPOSER    : Type = guMEDIAVIEWER_SELECT_COMPOSER; break;
            case ID_MAINFRAME_SELECT_ARTIST      : Type = guMEDIAVIEWER_SELECT_ARTIST; break;
            case ID_MAINFRAME_SELECT_YEAR        : Type = guMEDIAVIEWER_SELECT_YEAR; break;
            case ID_MAINFRAME_SELECT_GENRE       : Type = guMEDIAVIEWER_SELECT_GENRE; break;
        }

        if( Type != wxNOT_FOUND )
        {
            int PanelIndex = m_MainNotebook->GetPageIndex( MediaViewer );
            m_MainNotebook->SetSelection( PanelIndex );
            MediaViewer->SetSelection( Type, event.GetInt() );
        }
    }
    else if( event.GetExtraLong() == guTRACK_TYPE_PODCAST )
    {
        int Type = wxNOT_FOUND;
        switch( event.GetId() )
        {
            case ID_MAINFRAME_SELECT_TRACK      : Type = guMEDIAVIEWER_SELECT_TRACK; break;
            case ID_MAINFRAME_SELECT_ALBUM      : Type = guMEDIAVIEWER_SELECT_ALBUM; break;
        }

        if( Type != wxNOT_FOUND )
        {
            if( m_PodcastsPanel )
            {
                int PanelIndex = m_MainNotebook->GetPageIndex( m_PodcastsPanel );
                m_MainNotebook->SetSelection( PanelIndex );
                m_PodcastsPanel->SetSelection( Type, event.GetInt() );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectLocation( wxCommandEvent &event )
{
    int PanelIndex = wxNOT_FOUND;
    switch( event.GetInt() )
    {
        case ID_MENU_VIEW_RADIO :
            PanelIndex = m_MainNotebook->GetPageIndex( m_RadioPanel );
            break;

        case ID_MENU_VIEW_LASTFM :
            PanelIndex = m_MainNotebook->GetPageIndex( m_LastFMPanel );
            break;

        case ID_MENU_VIEW_LYRICS :
            PanelIndex = m_MainNotebook->GetPageIndex( m_LyricsPanel );
            break;

        case ID_MENU_VIEW_PODCASTS :
            PanelIndex = m_MainNotebook->GetPageIndex( m_PodcastsPanel );
            break;

        case ID_MENU_VIEW_FILEBROWSER :
            PanelIndex = m_MainNotebook->GetPageIndex( m_FileBrowserPanel );
            break;

        default : // Its a Collection
        {
            int CollectionIndex = ( event.GetInt() - ID_COLLECTIONS_BASE ) / guCOLLECTION_ACTION_COUNT;
            //int CollectionCmdId = ( event.GetId() - ID_COLLECTIONS_BASE ) % guCOLLECTION_ACTION_COUNT;
            guLogMessage( wxT( "Show Location for collection %i" ), CollectionIndex );
            guMediaViewer * MediaViewer = FindCollectionMediaViewer( m_Collections[ CollectionIndex ].m_UniqueId );
            if( MediaViewer )
            {
                PanelIndex = m_MainNotebook->GetPageIndex( MediaViewer );
            }
            break;
        }
    }

    if( PanelIndex != wxNOT_FOUND )
    {
        m_MainNotebook->SetSelection( PanelIndex );
    }
    if( m_LocationPanel )
        m_LocationPanel->SetFocus();
}

//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnGenreSetSelection( wxCommandEvent &event )
//{
//    wxArrayInt * genres = ( wxArrayInt * ) event.GetClientData();
//    if( genres )
//    {
////        m_MainNotebook->SetSelection( 0 );
////        m_LibPanel->SelectGenres( genres );
//        delete genres;
//    }
//}
//
//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnArtistSetSelection( wxCommandEvent &event )
//{
//    wxArrayInt * artists = ( wxArrayInt * ) event.GetClientData();
//    if( artists )
//    {
////        m_MainNotebook->SetSelection( 0 );
////        m_LibPanel->SelectArtists( artists );
//        delete artists;
//    }
//}
//
//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnAlbumArtistSetSelection( wxCommandEvent &event )
//{
//    wxArrayInt * Ids = ( wxArrayInt * ) event.GetClientData();
//    if( Ids )
//    {
////        m_MainNotebook->SetSelection( 0 );
////        m_LibPanel->SelectAlbumArtists( Ids );
//        delete Ids;
//    }
//}
//
//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnComposerSetSelection( wxCommandEvent &event )
//{
//    wxArrayInt * Ids = ( wxArrayInt * ) event.GetClientData();
//    if( Ids )
//    {
////        m_MainNotebook->SetSelection( 0 );
////        m_LibPanel->SelectComposers( Ids );
//        delete Ids;
//    }
//}
//
//// -------------------------------------------------------------------------------- //
//void guMainFrame::OnAlbumSetSelection( wxCommandEvent &event )
//{
//    wxArrayInt * albums = ( wxArrayInt * ) event.GetClientData();
//    if( albums )
//    {
////        m_MainNotebook->SetSelection( 0 );
////        m_LibPanel->SelectAlbums( albums );
//        delete albums;
//    }
//}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayListUpdated( wxCommandEvent &event )
{
    if( m_PlayerFilters )
        m_PlayerFilters->UpdateFilters();
}

// -------------------------------------------------------------------------------- //
int guMainFrame::AddGauge( const wxString &label, const bool showpercent )
{
    return m_MainStatusBar->AddGauge( label, showpercent );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::RemoveGauge( const int gaugeid )
{
    m_MainStatusBar->RemoveGauge( gaugeid );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugeCreate( wxCommandEvent &event )
{
    wxString * Label = ( wxString * ) event.GetClientData();
    if( Label )
    {
        int NewGauge = m_MainStatusBar->AddGauge( * Label, event.GetInt() );
        wxEvtHandler * SourceCtrl = ( wxEvtHandler * ) event.GetEventObject();
        event.SetId( ID_STATUSBAR_GAUGE_CREATED );
        event.SetInt( NewGauge );
        event.SetEventObject( this );
        wxPostEvent( SourceCtrl, event );

        delete Label;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPageChanged( wxAuiNotebookEvent& event )
{
    m_CurrentPage = m_MainNotebook->GetPage( m_MainNotebook->GetSelection() );

    OnUpdateSelInfo( event );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::DoPageClose( wxPanel * curpage )
{
    int PanelId = 0;

    if( m_LocationPanel )
        m_LocationPanel->Lock();

    RemoveTabPanel( curpage );

    if( curpage == m_RadioPanel )
    {
        m_MenuRadios->Check( false );
        m_MenuRadTextSearch->Enable( false );
        //m_MenuRadLabels->Enable( false );
        m_MenuRadGenres->Enable( false );

        PanelId = guPANEL_MAIN_RADIOS;
    }
    else if( curpage == m_LastFMPanel )
    {
        m_MenuLastFM->Check( false );
        PanelId = guPANEL_MAIN_LASTFM;
    }
    else if( curpage == m_LyricsPanel )
    {
        m_MenuLyrics->Check( false );
        PanelId = guPANEL_MAIN_LYRICS;
    }
    else if( curpage == m_PodcastsPanel )
    {
        m_MenuPodcasts->Check( false );
        m_MenuPodChannels->Enable( false  );
        m_MenuPodDetails->Enable( false );
        PanelId = guPANEL_MAIN_PODCASTS;
    }
    else if( curpage == m_FileBrowserPanel )
    {
        m_MenuFileBrowser->Check( false );
        PanelId = guPANEL_MAIN_FILEBROWSER;
    }
    else if( FindCollectionMediaViewer( curpage ) )
    {
        guMediaViewer * MediaViewer = ( guMediaViewer * ) curpage;
        MediaViewer->SetMenuState( false );

        m_MediaViewers.Remove( MediaViewer );

        if( MediaViewer->IsDefault() )
        {
            guLogMessage( wxT( "Was the default mediaviewer" ) );
            MediaViewer->SetViewMode( guMEDIAVIEWER_MODE_NONE );
        }
        else
        {
            if( m_MainNotebook->GetPageCount() > 1 )
            {
                delete MediaViewer;
            }
            else
            {
                m_DestroyLastWindow = true;
            }
        }

        CollectionsUpdated();
    }

    //CheckHideNotebook();
    m_VisiblePanels ^= PanelId;

    if( m_LocationPanel )
    {
        m_LocationPanel->OnPanelVisibleChanged();
        m_LocationPanel->Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPageClosed( wxAuiNotebookEvent& event )
{
    wxAuiNotebook * ctrl = ( wxAuiNotebook * ) event.GetEventObject();

    wxPanel * CurPage = ( wxPanel * ) ctrl->GetPage( event.GetSelection() );

    DoPageClose( CurPage );

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateSelInfo( wxCommandEvent &event )
{
    if( !m_CurrentPage )
    {
        m_MainStatusBar->SetSelInfo( wxEmptyString );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_RadioPanel )
    {
        //m_Db->GetRadioCounter( &m_SelCount );
        m_RadioPanel->GetRadioCounter( &m_SelCount );
        wxString SelInfo = wxString::Format( wxT( "%llu " ), m_SelCount.GetValue() );
        SelInfo += m_SelCount == 1 ? _( "station" ) : _( "stations" );
        m_MainStatusBar->SetSelInfo( SelInfo );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_PodcastsPanel )
    {
        m_PodcastsPanel->GetCounters( &m_SelCount, &m_SelLength, &m_SelSize );
        //m_Db->GetPodcastCounters( &m_SelCount, &m_SelLength, &m_SelSize );

        wxString SelInfo = wxString::Format( wxT( "%llu " ), m_SelCount.GetValue() );
        SelInfo += m_SelCount == 1 ? _( "podcast" ) : _( "podcasts" );
        SelInfo += wxString::Format( wxT( ",   %s,   %s" ),
            LenToString( m_SelLength.GetValue() ).c_str(),
            SizeToString( m_SelSize.GetValue() ).c_str() );
        m_MainStatusBar->SetSelInfo( SelInfo );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_LyricsPanel )
    {
        m_MainStatusBar->SetSelInfo( m_LyricsPanel->GetLyricSource() );
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
        guMediaViewer * MediaViewer = ( guMediaViewer * ) FindCollectionMediaViewer( m_CurrentPage );
        if( MediaViewer )
        {
            m_MainStatusBar->SetSelInfo( MediaViewer->GetSelInfo() );
        }
        else
        {
            //m_SelCount = wxNOT_FOUND;
            //m_SelLength = wxNOT_FOUND;
            //m_SelSize = wxNOT_FOUND;
            m_MainStatusBar->SetSelInfo( wxEmptyString );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnRequestCurrentTrack( wxCommandEvent &event )
{
    const guCurrentTrack * CurrentTrack = m_PlayerPanel->GetCurrentTrack();
    if( CurrentTrack->m_Loaded )
    {
        wxCommandEvent UpdateEvent( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_TRACKCHANGED );
        guTrack * Track = new guTrack( * CurrentTrack );
        UpdateEvent.SetClientData( Track );

        if( event.GetClientData() == m_LyricsPanel )
        {
            m_LyricsPanel->OnSetCurrentTrack( UpdateEvent );

            if( m_LyricSearchEngine )
            {
                if( m_LyricSearchContext )
                    delete m_LyricSearchContext;
                m_LyricSearchContext = m_LyricSearchEngine->CreateContext( this, Track );
                m_LyricSearchEngine->SearchStart( m_LyricSearchContext );
            }
        }
        else if( event.GetClientData() == m_LastFMPanel )
        {
            m_LastFMPanel->OnUpdatedTrack( UpdateEvent );
        }

        delete Track;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnIdle( wxIdleEvent& WXUNUSED( event ) )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Disconnect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );

	m_MainStatusBar->Show( Config->ReadBool( wxT( "ShowStatusBar" ), true, wxT( "mainwindow" ) ) );

    // If the Podcasts update is enable launch it...
    if( Config->ReadBool( wxT( "Update" ), true, wxT( "podcasts" ) ) )
    {
        guLogMessage( wxT( "Updating the podcasts..." ) );
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_UPDATE_PODCASTS );
        AddPendingEvent( event );
    }

    // Add the previously pending podcasts to download
    guDbPodcasts * DbPodcasts = GetPodcastsDb();
    guPodcastItemArray Podcasts;
    DbPodcasts->GetPendingPodcasts( &Podcasts );
    if( Podcasts.Count() )
        AddPodcastsDownloadItems( &Podcasts );

    // Now we can start the dbus server
    m_DBusServer->Run();

    CreateTaskBarIcon();

    if( m_PlayerPlayList )
    {
        m_PlayerPlayList->LoadPlaylistTracks();
    }

    if( m_PlayerPanel )
    {
        if( m_PlayerPanel->GetAudioScrobbleEnabled() )
        {
            wxCommandEvent Event;
            OnAudioScrobbleUpdate( Event );
        }

        m_PlayerPanel->CheckStartPlaying();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateTaskBarIcon( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    bool ShowIcon = Config->ReadBool( wxT( "ShowTaskBarIcon" ), true, wxT( "general" ) );
    bool SoundMenuEnabled = false;

    if( ShowIcon )
    {
#ifdef WITH_LIBINDICATE_SUPPORT
        SoundMenuEnabled = Config->ReadBool( wxT( "SoundMenuIntegration" ), false, wxT( "general" ) );
        if( SoundMenuEnabled )
        {
            if( !m_IndicateServer )
            {
                m_IndicateServer = indicate_server_ref_default();
                indicate_server_set_type( m_IndicateServer, GUAYADEQUE_INDICATOR_NAME );
                indicate_server_set_desktop_file( m_IndicateServer, GUAYADEQUE_DESKTOP_PATH );
                indicate_server_show( m_IndicateServer );
            }
        }
#else
        if( m_MPRIS2->Indicators_Sound_Available() )
        {
            SoundMenuEnabled = Config->ReadBool( wxT( "SoundMenuIntegration" ), false, wxT( "general" ) );
            int IsBlacklisted = m_MPRIS2->Indicators_Sound_IsBlackListed();
            if( IsBlacklisted != wxNOT_FOUND )
            {
                if( SoundMenuEnabled == bool( IsBlacklisted ) )
                {
                    m_MPRIS2->Indicators_Sound_BlacklistMediaPlayer( !SoundMenuEnabled );
                }
            }
        }
#endif

        if( SoundMenuEnabled )
        {
            if( m_TaskBarIcon )
            {
                m_TaskBarIcon->RemoveIcon();
                delete m_TaskBarIcon;
                m_TaskBarIcon = NULL;
            }
        }
        else
        {
#ifdef WITH_LIBINDICATE_SUPPORT
            if( m_IndicateServer )
            {
                indicate_server_hide( m_IndicateServer );
                g_object_unref( m_IndicateServer );
                m_IndicateServer = NULL;
            }
#else
            if( m_MPRIS2->Indicators_Sound_Available() )
            {
                int IsBlacklisted = m_MPRIS2->Indicators_Sound_IsBlackListed();
                if( IsBlacklisted != wxNOT_FOUND )
                {
                    if( !IsBlacklisted )
                    {
                        m_MPRIS2->Indicators_Sound_BlacklistMediaPlayer( true );
                    }
                }
            }
#endif
            if( !m_TaskBarIcon )
            {
                m_TaskBarIcon = new guTaskBarIcon( this, m_PlayerPanel );
                if( m_TaskBarIcon )
                {
                    m_TaskBarIcon->SetIcon( m_AppIcon, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION "-" ID_GUAYADEQUE_REVISION ) );
                }
            }
        }
    }
    else
    {
#ifdef WITH_LIBINDICATE_SUPPORT
        if( m_IndicateServer )
        {
            indicate_server_hide( m_IndicateServer );
            g_object_unref( m_IndicateServer );
            m_IndicateServer = NULL;
        }
#else
        if( m_MPRIS2->Indicators_Sound_Available() )
        {
            int IsBlacklisted = m_MPRIS2->Indicators_Sound_IsBlackListed();
            if( IsBlacklisted != wxNOT_FOUND )
            {
                if( !IsBlacklisted )
                {
                    m_MPRIS2->Indicators_Sound_BlacklistMediaPlayer( true );
                }
            }
        }
#endif
        if( m_TaskBarIcon )
        {
            m_TaskBarIcon->RemoveIcon();
            delete m_TaskBarIcon;
            m_TaskBarIcon = NULL;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::UpdatePodcasts( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config->ReadBool( wxT( "Update" ), true, wxT( "podcasts" ) ) )
    {
        if( !m_UpdatePodcastsTimer )
        {
            //guLogMessage( wxT( "Creating the UpdatePodcasts timer..." ) );
            m_UpdatePodcastsTimer = new guUpdatePodcastsTimer( this, m_Db );
            m_UpdatePodcastsTimer->Start( guPODCASTS_UPDATE_TIMEOUT );
        }

        wxDateTime LastUpdate;
        LastUpdate.ParseDateTime( Config->ReadStr( wxT( "LastPodcastUpdate" ), wxEmptyString, wxT( "podcasts" ) ) );

        wxDateTime UpdateTime = wxDateTime::Now();

        switch( Config->ReadNum( wxT( "UpdatePeriod" ), 0, wxT( "podcasts" ) ) )
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
            guUpdatePodcastsThread * UpdatePodcastThread = new guUpdatePodcastsThread( this, GaugeId );
            if( !UpdatePodcastThread )
            {
                guLogError( wxT( "Could not create the Update Podcasts thread" ) );
            }

            Config->WriteStr( wxT( "LastPodcastUpdate" ), wxDateTime::Now().Format(), wxT( "podcasts" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::AddPodcastsDownloadItems( guPodcastItemArray * items )
{
    int Index;
    int Count = items->Count();
    if( Count )
    {
        if( !m_DownloadThread )
        {
            m_DownloadThread = new guPodcastDownloadQueueThread( this );
        }

        guDbPodcasts * DbPodcasts = GetPodcastsDb();
        for( Index = 0; Index < Count; Index++ )
        {
            if( items->Item( Index ).m_Status != guPODCAST_STATUS_PENDING )
            {
                items->Item( Index ).m_Status = guPODCAST_STATUS_PENDING;
                DbPodcasts->SetPodcastItemStatus( items->Item( Index ).m_Id, guPODCAST_STATUS_PENDING );
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
wxString GetLayoutName( const wxString &filename )
{
    wxString LayoutName;
    //guLogMessage( wxT( "Layout file: '%s'" ), filename.c_str() );
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        wxXmlDocument XmlDoc( Ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && XmlNode->GetName() == wxT( "layout" ) )
        {
            XmlNode->GetPropVal( wxT( "name" ), &LayoutName );
        }
    }
    return LayoutName;
}

// -------------------------------------------------------------------------------- //
wxString GetLayoutFileName( const wxString &layoutname )
{
    wxRegEx ReplaceEx( wxT( "[ <>:\\\\|\\?\\*]" ) );
    wxString LayoutName = layoutname;
    ReplaceEx.Replace( &LayoutName, wxT( "_" ) );
    LayoutName = guPATH_LAYOUTS + LayoutName + wxT( ".xml" );
    return LayoutName;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::LoadLayoutNames( void )
{
    m_LayoutNames.Empty();

    wxDir         Dir;
    wxString      FileName;
    wxString      LayoutDir = guPATH_LAYOUTS;

    Dir.Open( LayoutDir );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES ) )
        {
            do {
                if( FileName[ 0 ] == '.' )
                    continue;

                if( FileName.EndsWith( wxT( ".xml" ) ) )
                {
                    wxString LayoutName = GetLayoutName( LayoutDir + FileName );
                    //guLogMessage( wxT( "LayoutName: '%s'" ), LayoutName.c_str() );
                    if( !LayoutName.IsEmpty() )
                    {
                        m_LayoutNames.Add( LayoutName );
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
    }
    else
    {
        guLogMessage( wxT( "Could not open the Layouts dir" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::ReloadLayoutMenus( void )
{
    LoadLayoutNames();

    while( m_MenuLayoutLoad->GetMenuItemCount() )
        m_MenuLayoutLoad->Destroy( m_MenuLayoutLoad->FindItemByPosition( 0 ) );

    while( m_MenuLayoutDelete->GetMenuItemCount() )
        m_MenuLayoutDelete->Destroy( m_MenuLayoutDelete->FindItemByPosition( 0 ) );

    int Index;
    int Count = m_LayoutNames.Count();
    if( Count )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            m_MenuLayoutLoad->Append( ID_MENU_LAYOUT_LOAD + Index, m_LayoutNames[ Index ], _( "Load this user defined layout" ) );
            m_MenuLayoutDelete->Append( ID_MENU_LAYOUT_DELETE + Index, m_LayoutNames[ Index ], _( "Delete this user defined layout" ) );
        }
    }
    else
    {
        wxMenuItem * MenuItem = new wxMenuItem( m_MenuLayoutLoad, ID_MENU_LAYOUT_DUMMY, _( "No Layouts Defined" ), _( "Load this user defined layout" ) );
        m_MenuLayoutLoad->Append( MenuItem );
        MenuItem->Enable( false );
        MenuItem = new wxMenuItem( m_MenuLayoutLoad, ID_MENU_LAYOUT_DUMMY, _( "No Layouts Defined" ), _( "Load this user defined layout" ) );
        m_MenuLayoutDelete->Append( MenuItem );
        MenuItem->Enable( false );
    }

}

// -------------------------------------------------------------------------------- //
bool guMainFrame::SaveCurrentLayout( const wxString &layoutname )
{
    wxXmlNode * RootNode;
    wxXmlNode * XmlNode;
    wxXmlDocument OutXml;

    // RootNode
    //
    RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "layout" ) );

    wxXmlProperty * Property = new wxXmlProperty( wxT( "name" ), layoutname,
                               new wxXmlProperty( wxT( "version" ), wxT( "1.0" ),
                               NULL ) );

    RootNode->SetProperties( Property );


    // MainWindow
    //
    XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "mainwindow" ) );

    // Size
    int Width;
    int Height;
    GetSize( &Width, &Height );
    // Pos
    int PosX;
    int PosY;
    GetPosition( &PosX, &PosY );
    // State
    int State = guWINDOW_STATE_NORMAL;

    if( IsFullScreen() )
        State |= guWINDOW_STATE_FULLSCREEN;

    if( IsMaximized() )
        State |= guWINDOW_STATE_MAXIMIZED;

    if( !m_MainStatusBar->IsShown() )
        State |= guWINDOW_STATE_NOSTATUSBAR;

    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( wxT( "PlayerPlayList" ) );
    wxString PLCaption = PaneInfo.caption;
    PaneInfo.Caption( wxT( "Now Playing" ) );

    Property = new wxXmlProperty( wxT( "posx" ), wxString::Format( wxT( "%d" ), PosX ),
               new wxXmlProperty( wxT( "posy" ), wxString::Format( wxT( "%d" ), PosY ),
               new wxXmlProperty( wxT( "width" ), wxString::Format( wxT( "%d" ), Width ),
               new wxXmlProperty( wxT( "height" ), wxString::Format( wxT( "%d" ), Height ),
               new wxXmlProperty( wxT( "state" ), wxString::Format( wxT( "%d" ), State ),
               new wxXmlProperty( wxT( "panels" ), wxString::Format( wxT( "%d" ), m_VisiblePanels ),
               new wxXmlProperty( wxT( "layout" ), m_AuiManager.SavePerspective(),
               new wxXmlProperty( wxT( "tabslayout" ), m_MainNotebook->SavePerspective(),
               NULL ) ) ) ) ) ) ) );

    XmlNode->SetProperties( Property );

    RootNode->AddChild( XmlNode );


    // Radio
    //
    if( m_RadioPanel )
    {
        m_RadioPanel->SaveLayout( RootNode, wxT( "radio" ) );
    }

    // Podcasts
    //
    if( m_PodcastsPanel )
    {
        m_PodcastsPanel->SaveLayout( RootNode, wxT( "podcasts" ) );
    }

    // FileBrowser
    //
    if( m_FileBrowserPanel )
    {
        m_FileBrowserPanel->SaveLayout( RootNode, wxT( "filebrowser" ) );
    }

    int Index;
    int Count = m_MediaViewers.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_MediaViewers[ Index ]->SaveLayout( RootNode );
    }

    OutXml.SetRoot( RootNode );
    return OutXml.Save( GetLayoutFileName( layoutname ) );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCreateNewLayout( wxCommandEvent &event )
{
//    wxTextEntryDialog EntryDialog( this, _( "Enter the layout name:"), _( "New Layout" ) );
    guEditWithOptions * SaveLayoutDialog = new guEditWithOptions( this, _( "Save Layout" ), _( "Layout:" ), wxString::Format( _( "Layout %u" ), m_LayoutNames.GetCount() + 1 ), m_LayoutNames );

    if( SaveLayoutDialog )
    {
        if( SaveLayoutDialog->ShowModal() == wxID_OK )
        {
            if( !SaveCurrentLayout( SaveLayoutDialog->GetData() ) )
            {
                guLogMessage( wxT( "Could not save the layout '%s'" ), SaveLayoutDialog->GetData().c_str() );
            }
            else
            {
                ReloadLayoutMenus();
            }
        }
        SaveLayoutDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnDeleteLayout( wxCommandEvent &event )
{
    int Layout = event.GetId() - ID_MENU_LAYOUT_DELETE;
    //guLogMessage( wxT( "Delete Layout %i" ), Layout );

    if( Layout >= 0 && Layout < ( int ) m_LayoutNames.Count() )
    {
        wxString LayoutFile = GetLayoutFileName( m_LayoutNames[ Layout ] );
        if( wxFileExists( LayoutFile ) )
        {
            wxRemoveFile( LayoutFile );
            ReloadLayoutMenus();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::LoadPerspective( const wxString &layout )
{
    m_AuiManager.LoadPerspective( layout, true );

    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_MainNotebook );
    if( !PaneInfo.IsShown() )
    {
        m_VisiblePanels = m_VisiblePanels & ( guPANEL_MAIN_PLAYERPLAYLIST |
                                              guPANEL_MAIN_PLAYERFILTERS |
                                              guPANEL_MAIN_PLAYERVUMETERS |
                                              guPANEL_MAIN_LOCATIONS |
                                              guPANEL_MAIN_SHOWCOVER );

        // Reset the Menu entry for all elements
//        ResetViewMenuState();
    }

    //m_AuiManager.Update();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSize( wxSizeEvent &event )
{
    //wxSize Size = event.GetSize();
    //guLogMessage( wxT( "MainFrame.Size( %i, %i ) %i" ), Size.GetWidth(), Size.GetHeight(), m_LoadLayoutPending );
    if( m_LoadLayoutPending != wxNOT_FOUND )
    {
        //guLogMessage( wxT( "LoadLayout command sent" ) );
        wxCommandEvent LoadLayoutEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_LAYOUT_LOAD + m_LoadLayoutPending );
        m_LoadLayoutPending = wxNOT_FOUND;
        AddPendingEvent( LoadLayoutEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLoadLayout( wxCommandEvent &event )
{
    int LayoutIndex = event.GetId() - ID_MENU_LAYOUT_LOAD;
    //guLogMessage( wxT( "Loading LayoutIndex %i" ), LayoutIndex );

    if( LayoutIndex >= 0 && LayoutIndex < ( int ) m_LayoutNames.Count() )
    {
        wxString LayoutFile = GetLayoutFileName( m_LayoutNames[ LayoutIndex ] );
        if( wxFileExists( LayoutFile ) )
        {
            wxFileInputStream Ins( LayoutFile );
            if( Ins.IsOk() )
            {
                wxXmlDocument XmlDoc( Ins );
                wxXmlNode * XmlNode = XmlDoc.GetRoot();

                if( XmlNode && XmlNode->GetName() == wxT( "layout" ) )
                {
                    wxArrayString OpenViewers;
                    int Index;
                    int Count = m_MediaViewers.Count();
                    for( Index = 0; Index < Count; Index++ )
                    {
                        guMediaViewer * CurMediaViewer = m_MediaViewers[ Index ];
                        if( !CurMediaViewer->IsDefault() ||
                            CurMediaViewer->GetViewMode() != guMEDIAVIEWER_MODE_NONE )
                        {
                            OpenViewers.Add( CurMediaViewer->GetMediaCollection()->m_UniqueId );
                        }
                    }

                    Hide();

                    XmlNode = XmlNode->GetChildren();
                    while( XmlNode )
                    {
                        wxString NodeName = XmlNode->GetName();
                        if( NodeName == wxT( "mainwindow" ) )
                        {
                            wxString Field;
                            long PosX;
                            long PosY;
                            long Width;
                            long Height;
                            long State;
                            long VisiblePanels;
                            wxString LayoutStr;
                            wxString TabsLayoutStr;

                            XmlNode->GetPropVal( wxT( "posx" ), &Field );
                            Field.ToLong( &PosX );
                            XmlNode->GetPropVal( wxT( "posy" ), &Field );
                            Field.ToLong( &PosY );
                            XmlNode->GetPropVal( wxT( "width" ), &Field );
                            Field.ToLong( &Width );
                            XmlNode->GetPropVal( wxT( "height" ), &Field );
                            Field.ToLong( &Height );
                            XmlNode->GetPropVal( wxT( "state" ), &Field );
                            Field.ToLong( &State );
                            XmlNode->GetPropVal( wxT( "panels" ), &Field );
                            Field.ToLong( &VisiblePanels );
                            XmlNode->GetPropVal( wxT( "layout" ), &LayoutStr );
                            XmlNode->GetPropVal( wxT( "tabslayout" ), &TabsLayoutStr );

                            if( IsFullScreen() != bool( State & guWINDOW_STATE_FULLSCREEN ) )
                            {
                                ShowFullScreen( bool( State & guWINDOW_STATE_FULLSCREEN ), wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION );
                                if( bool( State & guWINDOW_STATE_FULLSCREEN ) )
                                    Hide();
                                m_LoadLayoutPending = LayoutIndex;
                                Refresh();
                                Update();
                                m_MenuFullScreen->Check( bool( State & guWINDOW_STATE_FULLSCREEN ) );
                            }

                            if( IsMaximized() != bool( State & guWINDOW_STATE_MAXIMIZED ) )
                            {
                                Maximize( bool( State & guWINDOW_STATE_MAXIMIZED ) );
                                m_LoadLayoutPending = LayoutIndex;
                                Refresh();
                                Update();
                            }

                            if( !m_MainStatusBar->IsShown() != bool( State & guWINDOW_STATE_NOSTATUSBAR ) )
                            {
                                m_MainStatusBar->Show( !( State & guWINDOW_STATE_NOSTATUSBAR ) );
                                Refresh();
                                Update();
                                m_MenuStatusBar->Check( !( State & guWINDOW_STATE_NOSTATUSBAR ) );
                            }

                            if( !( State & ( guWINDOW_STATE_MAXIMIZED | guWINDOW_STATE_FULLSCREEN ) ) )
                            {
                                SetSize( PosX, PosY, Width, Height );
                                Refresh();
                                Update();
                            }

                            LoadTabsPerspective( TabsLayoutStr );

                            wxArrayInt PanelIds;
                            PanelIds.Add( guPANEL_MAIN_PLAYERPLAYLIST );
                            PanelIds.Add( guPANEL_MAIN_PLAYERFILTERS );
                            PanelIds.Add( guPANEL_MAIN_PLAYERVUMETERS );
                            PanelIds.Add( guPANEL_MAIN_LOCATIONS );
                            PanelIds.Add( guPANEL_MAIN_SHOWCOVER );
                            int Index;
                            int Count = PanelIds.Count();

                            for( Index = 0; Index < Count; Index++ )
                            {
                                int PanelId = PanelIds[ Index ];
                                if( ( VisiblePanels & PanelId ) != ( int ) ( m_VisiblePanels & PanelId ) )
                                {
                                    ShowMainPanel( PanelId, ( VisiblePanels & PanelId ) );
                                }
                            }

                            LoadPerspective( LayoutStr );

                            OnPlayerPlayListUpdateTitle( event );
                        }
                        else if( NodeName == wxT( "radio" ) )
                        {
                            if( m_RadioPanel )
                            {
                                m_RadioPanel->LoadLayout( XmlNode );
                            }
                        }
                        else if( NodeName == wxT( "podcasts" ) )
                        {
                            if( m_PodcastsPanel )
                            {
                                m_PodcastsPanel->LoadLayout( XmlNode );
                            }
                        }
                        else if( NodeName == wxT( "filebrowser" ) )
                        {
                            if( m_FileBrowserPanel )
                            {
                                m_FileBrowserPanel->LoadLayout( XmlNode );
                            }
                        }
                        else if( NodeName == wxT( "mediaviewer" ) )
                        {
                            wxString UniqueId;
                            XmlNode->GetPropVal( wxT( "id" ), &UniqueId );

                            int CollectionIndex = FindCollectionUniqueId( &m_Collections, UniqueId );
                            if( CollectionIndex != wxNOT_FOUND )
                            {
                                guMediaCollection &Collection = m_Collections[ CollectionIndex ];

                                if( OpenViewers.Index( UniqueId ) != wxNOT_FOUND )
                                {
                                    OpenViewers.Remove( UniqueId );
                                }

                                guMediaViewer * MediaViewer = FindCollectionMediaViewer( UniqueId );

                                if( !MediaViewer ||
                                    ( MediaViewer->IsDefault() && ( MediaViewer->GetViewMode() == guMEDIAVIEWER_MODE_NONE ) ) ||
                                    ( ( ( Collection.m_Type == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
                                      ( Collection.m_Type == guMEDIA_COLLECTION_TYPE_IPOD ) ) &&
                                      IsCollectionActive( Collection.m_UniqueId ) ) )
                                {
                                    event.SetId( ID_COLLECTIONS_BASE + ( CollectionIndex * guCOLLECTION_ACTION_COUNT ) );
                                    event.SetInt( 1 );
                                    OnCollectionCommand( event );
                                }

                                while( !MediaViewer )
                                {
                                    MediaViewer = FindCollectionMediaViewer( UniqueId );
                                    wxMilliSleep( 5 );
                                }

                                MediaViewer->LoadLayout( XmlNode );
                            }
                        }

                        XmlNode = XmlNode->GetNext();
                    }

                    if( OpenViewers.Count() )
                    {
                        Count = OpenViewers.Count();
                        for( Index = 0; Index < Count; Index++ )
                        {
                            int CollectionIndex = FindCollectionUniqueId( &m_Collections, OpenViewers[ Index ] );
                            event.SetId( ID_COLLECTIONS_BASE + ( CollectionIndex * guCOLLECTION_ACTION_COUNT ) );
                            event.SetInt( 0 );
                            OnCollectionCommand( event );
                        }
                    }

//                    RefreshViewMenuState();

                    Show();
                }
            }
            else
            {
                guLogError( wxT( "Could not read the file '%s'" ), LayoutFile.c_str() );
            }

        }
        else
        {
            guLogError( wxT( "Could not find the file '%s'" ), LayoutFile.c_str() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::LoadTabsPerspective( const wxString &layout )
{
    wxCommandEvent event;

    if( m_LocationPanel )
        m_LocationPanel->Lock();

    // Empty the tabs
    int Index = 0;
    int Count = m_MainNotebook->GetPageCount();
    for( Index = 0; Index < Count; Index++ )
    {
        RemoveTabPanel( ( wxPanel * ) m_MainNotebook->GetPage( 0 ) );
    }

    // Add the tabs in the proper ordering
    wxString TabsLayout = layout.AfterFirst( wxT( '=' ) );
    TabsLayout = TabsLayout.BeforeFirst( wxT( '@' ) );
    event.SetInt( 1 );

    m_VisiblePanels = m_VisiblePanels & ( guPANEL_MAIN_PLAYERPLAYLIST |
                                          guPANEL_MAIN_PLAYERFILTERS |
                                          guPANEL_MAIN_PLAYERVUMETERS |
                                          guPANEL_MAIN_LOCATIONS |
                                          guPANEL_MAIN_SHOWCOVER );

    // Reset the Menu entry for all elements
//    ResetViewMenuState();
    Index = 0;
    while( true )
    {
        int FindPos = TabsLayout.Find( wxString::Format( wxT( "%02i[" ), Index ) );
        if( FindPos == wxNOT_FOUND )
            break;
        wxString TabName = TabsLayout.Mid( FindPos ).AfterFirst( wxT( '[' ) ).BeforeFirst( wxT( ']' ) );

        //guLogMessage( wxT( "Creating tab %i '%s'" ), Index, TabName.c_str() );

        if( TabName == wxT( "Radio" ) )
        {
            OnViewRadio( event );
        }
        else if( TabName == wxT( "Last.fm" ) )
        {
            OnViewLastFM( event );
        }
        else if( TabName == wxT( "Lyrics" ) )
        {
            OnViewLyrics( event );
        }
        else if( TabName == wxT( "Podcasts" ) )
        {
            OnViewPodcasts( event );
        }
        else if( TabName == wxT( "FileBrowser" ) )
        {
            OnViewFileBrowser( event );
        }
        else    // It must be a collection
        {
            //guMediaViewer * MediaViewer = FindCollectionMediaViewer( TabName );
            int CollectionIndex = FindCollectionUniqueId( &m_Collections, TabName );
            if( CollectionIndex != wxNOT_FOUND )
            {
                if( m_Collections[ CollectionIndex ].m_Type < guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE )
                {
                    event.SetId( ID_COLLECTIONS_BASE + ( CollectionIndex * guCOLLECTION_ACTION_COUNT ) );
                    event.SetInt( 1 );
                    OnCollectionCommand( event );
                }
            }
        }

        Index++;
    }

    m_MainNotebook->LoadPerspective( layout );

    if( m_LocationPanel )
        m_LocationPanel->Unlock();
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
        CmdId = ID_MENU_VIEW_PLAYER_NOTEBOOK;
    }
    else if( PaneName == wxT( "MainSources" ) )
    {
        CmdId = ID_MENU_VIEW_MAIN_LOCATIONS;
    }
    else if( PaneName == wxT( "MainShowCover" ) )
    {
        CmdId = ID_MENU_VIEW_MAIN_SHOWCOVER;
    }

    //guLogMessage( wxT( "OnMainPaneClose: %s  %i" ), PaneName.c_str(), CmdId );
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

        case ID_MENU_VIEW_PLAYER_NOTEBOOK :
        {
            if( m_VisiblePanels & guPANEL_MAIN_RADIOS )
                OnViewRadio( event );

            if( m_VisiblePanels & guPANEL_MAIN_LASTFM )
                OnViewLastFM( event );

            if( m_VisiblePanels & guPANEL_MAIN_LYRICS )
                OnViewLyrics( event );

            if( m_VisiblePanels & guPANEL_MAIN_PODCASTS )
                OnViewPodcasts( event );

            if( m_VisiblePanels & guPANEL_MAIN_FILEBROWSER )
                OnViewFileBrowser( event );

            break;
        }

        case ID_MENU_VIEW_MAIN_LOCATIONS :
        {
            PanelId = guPANEL_MAIN_LOCATIONS;
            break;
        }

        case ID_MENU_VIEW_MAIN_SHOWCOVER :
        {
            PanelId = guPANEL_MAIN_SHOWCOVER;
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
    //guLogMessage( wxT( "ShowMainPanel( %08X, %i )" ), panelid, show );

    wxString PaneName;

    switch( panelid )
    {
        case guPANEL_MAIN_PLAYERPLAYLIST :
            PaneName = wxT( "PlayerPlayList" );
            m_MenuPlayerPlayList->Check( show );
            break;

        case guPANEL_MAIN_PLAYERFILTERS :
            PaneName = wxT( "PlayerFilters" );
            m_MenuPlayerFilters->Check( show );
            break;

        case guPANEL_MAIN_PLAYERVUMETERS :
            if( !m_PlayerVumeters )
            {
                guConfig * Config = ( guConfig * ) guConfig::Get();
                m_PlayerVumeters = new guPlayerVumeters( this );
                m_AuiManager.AddPane( m_PlayerVumeters, wxAuiPaneInfo().Name( wxT( "PlayerVumeters" ) ).Caption( _( "VU Meters" ) ).
                    DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 20, 20 ).
                    CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "general" ) ) ).
                    Left().Layer( 0 ).Row( 1 ).Position( 1 ).Hide() );
                if( m_PlayerPanel )
                    m_PlayerPanel->SetPlayerVumeters( m_PlayerVumeters );
            }
            PaneName = wxT( "PlayerVumeters" );
            if( m_MenuPlayerVumeters )
                m_MenuPlayerVumeters->Check( show );
            break;

        case guPANEL_MAIN_LOCATIONS :
            if( !m_LocationPanel )
            {
                guConfig * Config = ( guConfig * ) guConfig::Get();
                m_LocationPanel = new guLocationPanel( this );

                m_AuiManager.AddPane( m_LocationPanel, wxAuiPaneInfo().Name( wxT( "MainSources" ) ).Caption( _( "Sources" ) ).
                    DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 20, 20 ).
                    CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "general" ) ) ).
                    Left().Layer( 3 ).Row( 0 ).Position( 0 ).Hide() );
            }
            PaneName = wxT( "MainSources" );
            if( m_MenuMainLocations )
                m_MenuMainLocations->Check( show );
            break;

        case guPANEL_MAIN_SHOWCOVER :
            if( !m_CoverPanel )
            {
                guConfig * Config = ( guConfig * ) guConfig::Get();
                m_CoverPanel = new guCoverPanel( this, m_PlayerPanel );

                m_AuiManager.AddPane( m_CoverPanel, wxAuiPaneInfo().Name( wxT( "MainShowCover" ) ).Caption( _( "Cover" ) ).
                    DestroyOnClose( false ).Resizable( true ).Floatable( true ).MinSize( 50, 50 ).
                    CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "general" ) ) ).
                    Left().Layer( 3 ).Row( 0 ).Position( 0 ).Hide() );
            }
            PaneName = wxT( "MainShowCover" );
            if( m_MenuMainShowCover )
                m_MenuMainShowCover->Check( show );
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

    //guLogMessage( wxT( "Id: %i Pane: %s Show:%i  Flags:%08X" ), panelid, PaneName.c_str(), show, m_VisiblePanels );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::UpdatedTracks( int updatedby, const guTrackArray * tracks )
{
    if( !tracks->Count() )
        return;

    if( updatedby != guUPDATED_TRACKS_PLAYER )
    {
        m_PlayerPanel->UpdatedTracks( tracks );
    }

    if( updatedby != guUPDATED_TRACKS_PLAYER_PLAYLIST )
    {
        m_PlayerPlayList->UpdatedTracks( tracks );
    }

    if( updatedby != guUPDATED_TRACKS_MEDIAVIEWER )
    {
        int Index;
        int Count = m_MediaViewers.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            m_MediaViewers[ Index ]->UpdatedTracks( updatedby, tracks );
        }
    }

    if( m_LyricsPanel )
    {
        m_LyricsPanel->UpdatedTracks( tracks );
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

    if( track->m_MediaViewer )
    {
        track->m_MediaViewer->UpdatedTrack( updatedby, track );
    }

//    if( m_LibPanel && ( updatedby != guUPDATED_TRACKS_LIBRARY ) )
//    {
//        m_LibPanel->UpdatedTrack( track );
//    }

//    if( m_PlayListPanel && ( updatedby != guUPDATED_TRACKS_PLAYLISTS ) )
//    {
//        m_PlayListPanel->UpdatedTrack( track );
//    }
}


// -------------------------------------------------------------------------------- //
void guMainFrame::OnLibraryCoverChanged( wxCommandEvent &event )
{
    guMediaViewer * MediaViewer = ( guMediaViewer * ) event.GetClientData();
    if( m_PlayerPanel )
    {
        const guCurrentTrack * CurrentTrack = m_PlayerPanel->GetCurrentTrack();
        if( ( CurrentTrack->m_MediaViewer == MediaViewer )  && CurrentTrack->m_AlbumId == event.GetInt() )
        {
            m_PlayerPanel->UpdateCover( false, bool( event.GetExtraLong() ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPlayerPanelCoverChanged( wxCommandEvent &event )
{
    if( m_CoverPanel )
    {
        m_CoverPanel->OnUpdatedTrack( event );
    }

    if( m_MPRIS2 )
    {
        m_MPRIS2->OnPlayerTrackChange();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateCopyToMenu( wxMenu * menu )
{
    int Index;
    int Count;
    wxMenuItem * MenuItem;
    wxMenu * SubMenu = new wxMenu();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString CopyToOptions = Config->ReadAStr( wxT( "Option" ), wxEmptyString, wxT( "copyto/options" ) );

    if( ( Count = CopyToOptions.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            wxArrayString CurOption = wxStringTokenize( CopyToOptions[ Index ], wxT( ":") );
            MenuItem = new wxMenuItem( SubMenu, ID_COPYTO_BASE + Index, unescape_configlist_str( CurOption[ 0 ] ), _( "Copy the current selected songs to a directory or device" ) );
            SubMenu->Append( MenuItem );
        }
    }

    bool SeparatorAdded = false;
    Count = m_MediaViewers.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guMediaViewer * MediaViewer = m_MediaViewers[ Index ];
        int Type = MediaViewer->GetType();
        if( ( ( Type == guMEDIA_COLLECTION_TYPE_NORMAL ) && !MediaViewer->GetDefaultCopyAction().IsEmpty() ) ||
            ( Type == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
            ( Type == guMEDIA_COLLECTION_TYPE_IPOD ) )
        {
            if( !SeparatorAdded && SubMenu->GetMenuItemCount() )
            {
                SubMenu->AppendSeparator();
                SeparatorAdded = true;
            }

            MenuItem = new wxMenuItem( SubMenu, ID_COPYTO_BASE + guCOPYTO_DEVICE_BASE + Index, MediaViewer->GetName(), _( "Copy the current selected songs to a directory or device" ) );
            //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
            SubMenu->Append( MenuItem );
        }
    }

    if( SubMenu->GetMenuItemCount() )
    {
        SubMenu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( SubMenu, ID_MENU_PREFERENCES_COPYTO, _( "Preferences" ), _( "Add Copy To patterns in preferences" ) );
    SubMenu->Append( MenuItem );

    menu->AppendSubMenu( SubMenu, _( "Copy To..." ), _( "Copy the selected tracks to a folder or device" ) );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CopyToThreadFinished( void )
{
    if( m_CopyToThread )
    {
        m_CopyToThreadMutex.Lock();
        m_CopyToThread = NULL;
        m_CopyToThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSetForceGapless( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnSetForceGapless( %i )" ), event.GetInt() );
    const bool IsEnable = event.GetInt();
    m_MenuForceGapless->Check( IsEnable );

    m_PlayerPanel->SetForceGapless( IsEnable );

    if( m_MainStatusBar )
    {
        m_MainStatusBar->SetPlayMode( IsEnable );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSetAudioScrobble( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnSetAudioScrobble( %i )" ), event.GetInt() );
    const bool IsEnable = event.GetInt();
    if( m_MainStatusBar )
    {
        m_MainStatusBar->SetAudioScrobble( IsEnable );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLyricFound( wxCommandEvent &event )
{
    //guLogMessage( wxT( "guMainFrame::OnLyricFound" ) );
    wxString * LyricText = ( wxString * ) event.GetClientData();
    if( m_LyricsPanel )
    {
        m_LyricsPanel->SetLyricText( LyricText );
        m_LyricsPanel->SetLastSource( event.GetInt() );
    }

    if( LyricText )
    {
        delete LyricText;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLyricSearchFirst( wxCommandEvent &event )
{
    if( m_LyricSearchEngine && m_LyricSearchContext )
    {
        m_LyricSearchContext->ResetIndex();
        m_LyricSearchEngine->SearchStart( m_LyricSearchContext );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLyricSearchNext( wxCommandEvent &event )
{
    if( m_LyricSearchEngine && m_LyricSearchContext )
    {
        m_LyricSearchEngine->SearchStart( m_LyricSearchContext );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLyricSaveChanges( wxCommandEvent &event )
{
    if( m_LyricSearchEngine && m_LyricSearchContext )
    {
        wxString * LyricText = ( wxString * ) event.GetClientData();

        m_LyricSearchEngine->SetLyricText( m_LyricSearchContext, * LyricText );

        delete LyricText;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLyricExecCommand( wxCommandEvent &event )
{
    guLyricSearchThread * LyricSearchThread = ( guLyricSearchThread * ) event.GetClientObject();
    wxString * CommandText = ( wxString * ) event.GetClientData();
    guLogMessage( wxT( "OnLyricExecCommand: '%s'" ), CommandText->c_str() );

    if( CommandText && LyricSearchThread )
    {
        guLyricExecCommandTerminate * LyricExecCommandTerminate = new guLyricExecCommandTerminate( LyricSearchThread, event.GetInt() );
        if( LyricExecCommandTerminate )
        {
            if( !wxExecute( * CommandText, wxEXEC_ASYNC, LyricExecCommandTerminate ) )
            {
                guLogError( wxT( "Could not execute the command '%s'" ), CommandText->c_str() );
                delete LyricExecCommandTerminate;

                LyricSearchThread->FinishExecCommand( wxEmptyString );
            }
            LyricExecCommandTerminate->Redirect();
        }
    }
    else
    {
        guLogMessage( wxT( "Error on OnLyricExecCommand..." ) );
    }

    delete CommandText;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        guAccelOnConfigUpdated();

        wxMenuBar * OldMenu = GetMenuBar();
        CreateMenu();
        delete OldMenu;
    }

    if( Flags & guPREFERENCE_PAGE_FLAG_LIBRARY )
    {
        m_CollectionsMutex.Lock();

        guConfig * Config = ( guConfig * ) guConfig::Get();
        guMediaCollectionArray  Collections;
        Config->LoadCollections( &Collections, guMEDIA_COLLECTION_TYPE_NORMAL );
        Config->LoadCollections( &Collections, guMEDIA_COLLECTION_TYPE_JAMENDO );
        Config->LoadCollections( &Collections, guMEDIA_COLLECTION_TYPE_MAGNATUNE );
        Config->LoadCollections( &Collections, guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE );
        Config->LoadCollections( &Collections, guMEDIA_COLLECTION_TYPE_IPOD );

        //wxArrayInt DeletedIndex;
        int Index;
        int Count = m_Collections.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            if( FindCollectionUniqueId( &Collections, m_Collections[ Index ].m_UniqueId ) == wxNOT_FOUND )
            {
                guMediaViewer * MediaViewer = FindCollectionMediaViewer( m_Collections[ Index ].m_UniqueId );
                if( MediaViewer )
                {
                    event.SetId( ID_COLLECTIONS_BASE + ( Index * guCOLLECTION_ACTION_COUNT ) );
                    event.SetInt( 0 );
                    OnCollectionCommand( event );
                }
                //DeletedIndex.Add( Index );
            }
        }

        m_Collections = Collections;
        Count = m_Collections.Count();
        int CollectionBaseCommand = ID_COLLECTIONS_BASE;
        for( Index = 0; Index < Count; Index++ )
        {
            guMediaCollection &Collection = m_Collections[ Index ];
            guMediaViewer * MediaViewer = FindCollectionMediaViewer( Collection.m_UniqueId );
            if( MediaViewer )
            {
                guLogMessage( wxT( "Setting new collection to '%s' => '%s'" ), MediaViewer->GetName().c_str(), Collection.m_Name.c_str() );
                MediaViewer->SetCollection( Collection, CollectionBaseCommand );
            }
            CollectionBaseCommand += guCOLLECTION_ACTION_COUNT;
        }

        m_CollectionsMutex.Unlock();

        CollectionsUpdated();
    }

    if( ( Flags & guPREFERENCE_PAGE_FLAG_LYRICS ) && m_LyricSearchEngine )
    {
        m_LyricSearchEngine->Load();
    }

    if( Flags & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        bool AudioScrobbleEnabled = Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "lastfm" ) ) ||
                                 Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "librefm" ) );
        m_MenuAudioScrobble->Check( AudioScrobbleEnabled );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSongSetRating( wxCommandEvent &event )
{
    int Rating = event.GetId() - ID_PLAYERPANEL_SETRATING_0;
    //guLogMessage( wxT( "Set rating to %i" ), Rating );
    if( m_PlayerPanel )
    {
        m_PlayerPanel->SetRating( Rating );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSetAllowDenyFilter( wxCommandEvent &event )
{
    if( ( event.GetId() == ID_MAINFRAME_SET_ALLOW_PLAYLIST ) )
    {
        m_PlayerFilters->SetAllowFilterId( event.GetInt() );
    }
    else
    {
        m_PlayerFilters->SetDenyFilterId( event.GetInt() );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnRaiseWindow( wxCommandEvent &event )
{
    //guLogMessage( wxT( "guMainFrame::OnRaiseWindow" ) );
    if( !IsShown() )
        Show( true );

    if( IsIconized() )
        Iconize( false );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLoadPlayList( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnLoadPlaylist %i " ), event.GetInt() );
    guTrackArray Tracks;
    if( m_Db->GetPlayListSongs( event.GetInt(), &Tracks ) )
    {
        m_PlayerPanel->SetPlayList( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::SaveCollections( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->SaveCollections( &m_Collections );
    Config->Flush();
}

// -------------------------------------------------------------------------------- //
void inline AddCollectionCoverNames( wxArrayString &covernames, const guMediaCollection &mediacollection )
{
    int Index;
    int Count = mediacollection.m_CoverWords.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        wxString CoverName = mediacollection.m_CoverWords[ Index ];
        if( covernames.Index( CoverName ) == wxNOT_FOUND )
            covernames.Add( CoverName );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::GetCollectionsCoverNames( wxArrayString &covernames )
{
    int Index;
    int Count = m_Collections.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        AddCollectionCoverNames( covernames, m_Collections[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::MediaViewerCreated( const wxString &uniqueid, guMediaViewer * mediaviewer )
{
    if( m_PlayerPlayList )
    {
        m_PlayerPlayList->MediaViewerCreated( uniqueid, mediaviewer );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::MediaViewerClosed( guMediaViewer * mediaviewer )
{
    if( m_PlayerPanel )
    {
        m_PlayerPanel->MediaViewerClosed( mediaviewer );
    }

    if( m_PlayerPlayList )
    {
        m_PlayerPlayList->MediaViewerClosed( mediaviewer );
    }

    if( m_LastFMPanel )
    {
        m_LastFMPanel->MediaViewerClosed( mediaviewer );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::AddPendingUpdateTrack( const guTrack &track, const wxImage * image, const wxString &lyric, const int changedflags )
{
    guLogMessage( wxT( "Adding pending update track '%s'" ), track.m_FileName.c_str() );
    wxMutexLocker Lock( m_PendingUpdateMutex );
    m_PendingUpdateTracks.Insert( new guTrack( track ), 0 );
    m_PendingUpdateFiles.Insert( wxEmptyString, 0 );
    m_PendingUpdateImages.Insert( image ? new wxImage( * image ) : NULL, 0 );
    m_PendingUpdateLyrics.Insert( lyric, 0 );
    m_PendingUpdateFlags.Insert( changedflags, 0 );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::AddPendingUpdateTrack( const wxString &filename, const wxImage * image, const wxString &lyric, const int changedflags )
{
    guLogMessage( wxT( "Adding pending update track '%s'" ), filename.c_str() );
    wxMutexLocker Lock( m_PendingUpdateMutex );
    m_PendingUpdateTracks.Insert( NULL, 0 );
    m_PendingUpdateFiles.Insert( filename, 0 );
    m_PendingUpdateImages.Insert( image ? new wxImage( * image ) : NULL, 0 );
    m_PendingUpdateLyrics.Insert( lyric, 0 );
    m_PendingUpdateFlags.Insert( changedflags, 0 );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CheckPendingUpdates( const guTrack * track )
{
    wxMutexLocker Lock( m_PendingUpdateMutex );
    int Index;
    int Count = m_PendingUpdateTracks.Count();
    if( Count )
    {
        for( Index = Count - 1; Index >= 0; Index-- )
        {
            bool RemoveTrack = false;
            wxString CurFile = m_PendingUpdateFiles[ Index ];
            if( CurFile.IsEmpty() )
            {
                CurFile = m_PendingUpdateTracks[ Index ].m_FileName;
                if( CurFile != track->m_FileName )
                {
                    guTrackArray Tracks;
                    Tracks.Add( m_PendingUpdateTracks[ Index ] );
                    guImagePtrArray Images;
                    Images.Add( m_PendingUpdateImages[ Index ] );
                    wxArrayString Lyrics;
                    Lyrics.Add( m_PendingUpdateLyrics[ Index ] );
                    wxArrayInt ChangedFlags;
                    ChangedFlags.Add( m_PendingUpdateFlags[ Index ] );
                    guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
                    RemoveTrack = true;
                }
            }
            else
            {
                if( CurFile != track->m_FileName )
                {
                    int ChangedFlags = m_PendingUpdateFlags[ Index ];
                    if( ChangedFlags == guTRACK_CHANGED_DATA_LYRICS )
                    {
                        guTagSetLyrics( CurFile, m_PendingUpdateLyrics[ Index ] );
                    }
                    else
                    {
                        guTagSetPicture( CurFile, m_PendingUpdateImages[ Index ] );
                    }
                    RemoveTrack = true;
                }
            }
            if( RemoveTrack )
            {
                m_PendingUpdateTracks.RemoveAt( Index );
                m_PendingUpdateFiles.RemoveAt( Index );
                m_PendingUpdateImages.RemoveAt( Index );
                m_PendingUpdateLyrics.RemoveAt( Index );
                m_PendingUpdateFlags.RemoveAt( Index );
            }
        }
    }
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
guUpdatePodcastsThread::guUpdatePodcastsThread( guMainFrame * mainframe,
    int gaugeid ) : wxThread()
{
    m_MainFrame = mainframe;
    m_GaugeId = gaugeid;
    m_Db = mainframe->GetPodcastsDb();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guUpdatePodcastsThread::~guUpdatePodcastsThread()
{
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
guUpdatePodcastsThread::ExitCode guUpdatePodcastsThread::Entry()
{
    guPodcastChannelArray PodcastChannels;
    if( m_Db->GetPodcastChannels( &PodcastChannels ) )
    {
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( PodcastChannels.Count() );
        wxPostEvent( m_MainFrame, event );

        unsigned int Index = 0;
        while( !TestDestroy() && Index < PodcastChannels.Count() )
        {
            event.SetId( ID_STATUSBAR_GAUGE_UPDATE );
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
