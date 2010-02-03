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
#include "MainFrame.h"

#include "Commands.h"
#include "ConfirmExit.h"
#include "Images.h"
#include "LibUpdate.h"
#include "Preferences.h"
#include "SplashWin.h"
#include "TrackChangeInfo.h"
#include "TaskBar.h"
#include "Utils.h"

#include <wx/event.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/datetime.h>

// The default update podcasts timeout is 15 minutes
#define guPODCASTS_UPDATE_TIMEOUT   ( 15 * 60 * 1000 )


// -------------------------------------------------------------------------------- //
guMainFrame::guMainFrame( wxWindow * parent )
{
	wxBoxSizer *    MainFrameSizer;
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

    //
    // Init the Database Object
    //
    m_Db = new guDbLibrary( wxGetHomeDir() + wxT( "/.guayadeque/guayadeque.db" ) );
    if( !m_Db )
    {
        guLogError( wxT( "Could not open the guayadeque database" ) );
        return;
    }

    m_DbCache = new guDbCache( wxGetHomeDir() + wxT( "/.guayadeque/cache.db" ) );
    if( !m_DbCache )
    {
        guLogError( wxT( "Could not open the guayadeque cache database" ) );
        return;
    }

    m_DbCache->SetDbCache();

    //
    m_Db->SetLibPath( Config->ReadAStr( wxT( "LibPath" ),
                                      wxGetHomeDir() + wxT( "/Music" ),
                                      wxT( "LibPaths" ) ) );

    m_LibUpdateThread = NULL;
    m_UpdatePodcastsTimer = NULL;
    m_DownloadThread = NULL;

    m_SelCount = 0;
    m_SelLength = 0;
    m_SelSize = 0;

    //
    m_AppIcon.CopyFromBitmap( guImage( guIMAGE_INDEX_guayadeque ) );


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
    Create( parent, wxID_ANY, wxT("Guayadeque Player"), MainWindowPos, MainWindowSize, wxDEFAULT_FRAME_STYLE );

    m_AuiManager.SetManagedWindow( this );

	m_MainStatusBar = new guStatusBar( this );
	SetStatusBar(  m_MainStatusBar );
	MainFrameSizer = new wxBoxSizer( wxVERTICAL );
	SetStatusText( _( "Welcome to guayadeque!" ) );

//	m_PlayerSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
//    m_PlayerSplitter->SetMinimumPaneSize( 100 );

	m_PlayerPanel = new guPlayerPanel( this, m_Db ); //wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_AuiManager.AddPane( m_PlayerPanel, wxAuiPaneInfo().Name( _("Player")).CenterPane() );


//	MultiPanel = new wxPanel( m_PlayerSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//
//	MultiSizer = new wxBoxSizer( wxVERTICAL );

	//m_CatNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_CatNotebook = new guAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON );

    // Library Page
    if( Config->ReadBool( wxT( "ShowLibrary" ), true, wxT( "ViewPanels" ) ) )
    {
        m_LibPanel = new guLibPanel( m_CatNotebook, m_Db, m_PlayerPanel );
        m_CatNotebook->AddPage( m_LibPanel, _( "Library" ), true );
    }
    else
        m_LibPanel = NULL;

    // Radio Page
    if( Config->ReadBool( wxT( "ShowRadio" ), true, wxT( "ViewPanels" ) ) )
    {
        m_RadioPanel = new guRadioPanel( m_CatNotebook, m_Db, m_PlayerPanel );
        m_CatNotebook->AddPage( m_RadioPanel, _( "Radio" ), false );
    }
    else
        m_RadioPanel = NULL;

    // LastFM Info Panel
    if( Config->ReadBool( wxT( "ShowLastfm" ), true, wxT( "ViewPanels" ) ) )
    {
        m_LastFMPanel = new guLastFMPanel( m_CatNotebook, m_Db, m_DbCache, m_PlayerPanel );
        m_CatNotebook->AddPage( m_LastFMPanel, _( "Last.fm" ), false );
    }
    else
        m_LastFMPanel = NULL;

    // Lyrics Panel
    if( Config->ReadBool( wxT( "ShowLyrics" ), true, wxT( "ViewPanels" ) ) )
    {
        m_LyricsPanel = new guLyricsPanel( m_CatNotebook, m_Db );
        m_CatNotebook->AddPage( m_LyricsPanel, _( "Lyrics" ), false );
    }
    else
        m_LyricsPanel = NULL;

    // PlayList Page
    if( Config->ReadBool( wxT( "ShowPlayLists" ), true, wxT( "ViewPanels" ) ) )
    {
        m_PlayListPanel = new guPlayListPanel( m_CatNotebook, m_Db, m_PlayerPanel );
        m_CatNotebook->AddPage( m_PlayListPanel, _( "PlayLists" ), false );
    }
    else
        m_PlayListPanel = NULL;

    // Podcast Page
    if( Config->ReadBool( wxT( "ShowPodcasts" ), true, wxT( "ViewPanels" ) ) )
    {
        m_PodcastsPanel = new guPodcastPanel( m_CatNotebook, m_Db, this, m_PlayerPanel );
        m_CatNotebook->AddPage( m_PodcastsPanel, _( "Podcasts" ), false );
    }
    else
        m_PodcastsPanel = NULL;

    m_AuiManager.AddPane( m_CatNotebook, wxAuiPaneInfo().Name( wxT("Selector") ).CaptionVisible( false ).CloseButton( false ).DestroyOnClose( false ).Resizable( true ).Floatable( false ).Top() );
    if( m_CatNotebook->GetPageCount() )
    {
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
        PaneInfo.Hide();
    }

    // FileSystem Page
//    if( Config->ReadBool( wxT( "ShowFileSys" ), true, wxT( "ViewPanels" ) ) )
//    {
//	      FileSysPanel = new wxPanel( CatNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//	      CatNotebook->AddPage( FileSysPanel, wxT("FileSystem"), false );
//    }

//	MultiSizer->Add( m_CatNotebook, 1, wxEXPAND | wxALL, 2 );
//
//	MultiPanel->SetSizer( MultiSizer );
//	MultiPanel->Layout();
//	MultiSizer->Fit( MultiPanel );
//
//	m_PlayerSplitter->SplitVertically( m_PlayerPanel, MultiPanel, Config->ReadNum( wxT( "PlayerSashPos" ), 280, wxT( "Positions" ) ) );
//	MainFrameSizer->Add( m_PlayerSplitter, 1, wxEXPAND, 5 );
//
//	this->SetSizer( MainFrameSizer );
//	this->Layout();


    //m_AuiManager.Update();
    wxString Perspective = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "Positions" ) );
    if( !Perspective.IsEmpty() )
        m_AuiManager.LoadPerspective( Perspective, true );
    else
        m_AuiManager.Update();

    m_NBPerspective = Config->ReadStr( wxT( "NotebookLayout" ), wxEmptyString, wxT( "Positions" ) );
    if( !m_NBPerspective.IsEmpty() )
        m_CatNotebook->LoadPerspective( m_NBPerspective );


    m_CurrentPage = m_LibPanel;

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


    CreateMenu();

    //m_DBusServer->Run();

    //
	Connect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );

    Connect( ID_MENU_UPDATE_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLibrary ), NULL, this );
    Connect( ID_MENU_UPDATE_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdatePodcasts ), NULL, this );
    Connect( ID_MENU_UPDATE_COVERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateCovers ), NULL, this );
    Connect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ), NULL, this );
    Connect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ), NULL, this );
    Connect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ), NULL, this );
    Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    Connect( wxEVT_ICONIZE, wxIconizeEventHandler( guMainFrame::OnIconizeWindow ), NULL, this );
    Connect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );

    Connect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ), NULL, this );
    Connect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ), NULL, this );
    Connect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ), NULL, this );

	Connect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectAlbumName ), NULL, this );
	Connect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectArtistName ), NULL, this );

	Connect( ID_GENRE_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGenreSetSelection ), NULL, this );
	Connect( ID_ARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnArtistSetSelection ), NULL, this );
	Connect( ID_ALBUM_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumSetSelection ), NULL, this );

    Connect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlay ), NULL, this );
    Connect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStop ), NULL, this );
    Connect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextTrack ), NULL, this );
    Connect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevTrack ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSmartPlay ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRandomize ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_REPEATPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Connect( ID_PLAYER_PLAYLIST_REPEATTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Connect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ), NULL, this );

    Connect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );

    Connect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

    Connect( ID_MENU_LAYOUT_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCreateNewLayout ), NULL, this );
    Connect( ID_MENU_LAYOUT_LOAD, ID_MENU_LAYOUT_LOAD + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadLayout ), NULL, this );
    Connect( ID_MENU_LAYOUT_DELETE, ID_MENU_LAYOUT_DELETE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnDeleteLayout ), NULL, this );

    Connect( ID_MENU_VIEW_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLibrary ), NULL, this );
    Connect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ), NULL, this );
    Connect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ), NULL, this );
    Connect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ), NULL, this );
    Connect( ID_MENU_VIEW_PLAYLISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPlayLists ), NULL, this );
    Connect( ID_MENU_VIEW_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPodcasts ), NULL, this );

    Connect( ID_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ), NULL, this );
    Connect( ID_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ), NULL, this );
    Connect( ID_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ), NULL, this );
    Connect( ID_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ), NULL, this );

    Connect( ID_PLAYLIST_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListUpdated ), NULL, this );

    Connect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guMainFrame::OnPodcastItemUpdated ), NULL, this );
    Connect( ID_MAINFRAME_REMOVEPODCASTTHREAD, wxCommandEventHandler( guMainFrame::OnRemovePodcastThread ), NULL, this );

	if( m_CatNotebook )
	{
        m_CatNotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );
        m_CatNotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler( guMainFrame::OnPageClosed ), NULL, this );
	}

    Connect( ID_MAINFRAME_UPDATE_SELINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateSelInfo ), NULL, this );
//    Connect( ID_MAINFRAME_SET_RADIOSTATIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetRadioStations ), NULL, this );
//    Connect( ID_MAINFRAME_SET_PLAYLISTTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPlayListTracks ), NULL, this );
//    Connect( ID_MAINFRAME_SET_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPodcasts ), NULL, this );

    //Connect( wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEventHandler( guMainFrame::OnSysColorChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guMainFrame::~guMainFrame()
{
	//Disconnect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );

    Disconnect( ID_MENU_UPDATE_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLibrary ), NULL, this );
    Disconnect( ID_MENU_UPDATE_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdatePodcasts ), NULL, this );
    Disconnect( ID_MENU_UPDATE_COVERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateCovers ), NULL, this );
    Disconnect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ), NULL, this );
    Disconnect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ), NULL, this );
    Disconnect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ), NULL, this );
    Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    Disconnect( wxEVT_ICONIZE, wxIconizeEventHandler( guMainFrame::OnIconizeWindow ), NULL, this );
    Disconnect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ), NULL, this );

    Disconnect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ), NULL, this );
    Disconnect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ), NULL, this );
    Disconnect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ), NULL, this );

	Disconnect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectAlbumName ), NULL, this );
	Disconnect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectArtistName ), NULL, this );

	Disconnect( ID_GENRE_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGenreSetSelection ), NULL, this );
	Disconnect( ID_ARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnArtistSetSelection ), NULL, this );
	Disconnect( ID_ALBUM_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumSetSelection ), NULL, this );

    Disconnect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlay ), NULL, this );
    Disconnect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStop ), NULL, this );
    Disconnect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextTrack ), NULL, this );
    Disconnect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevTrack ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSmartPlay ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRandomize ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_REPEATPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Disconnect( ID_PLAYER_PLAYLIST_REPEATTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Disconnect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ), NULL, this );

    Disconnect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );

    Disconnect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

    Disconnect( ID_MENU_LAYOUT_CREATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCreateNewLayout ), NULL, this );
    Disconnect( ID_MENU_LAYOUT_LOAD, ID_MENU_LAYOUT_LOAD + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnLoadLayout ), NULL, this );
    Disconnect( ID_MENU_LAYOUT_DELETE, ID_MENU_LAYOUT_DELETE + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnDeleteLayout ), NULL, this );

    Disconnect( ID_MENU_VIEW_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLibrary ), NULL, this );
    Disconnect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ), NULL, this );
    Disconnect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ), NULL, this );
    Disconnect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ), NULL, this );
    Disconnect( ID_MENU_VIEW_PLAYLISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPlayLists ), NULL, this );
    Disconnect( ID_MENU_VIEW_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPodcasts ), NULL, this );

    Disconnect( ID_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ), NULL, this );
    Disconnect( ID_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ), NULL, this );
    Disconnect( ID_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ), NULL, this );
    Disconnect( ID_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ), NULL, this );

    Disconnect( ID_PLAYLIST_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListUpdated ), NULL, this );

    Disconnect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guMainFrame::OnPodcastItemUpdated ), NULL, this );
    Disconnect( ID_MAINFRAME_REMOVEPODCASTTHREAD, wxCommandEventHandler( guMainFrame::OnRemovePodcastThread ), NULL, this );

	if( m_CatNotebook )
	{
        m_CatNotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );
        m_CatNotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler( guMainFrame::OnPageClosed ), NULL, this );
	}

    Disconnect( ID_MAINFRAME_UPDATE_SELINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateSelInfo ), NULL, this );
//    Disconnect( ID_MAINFRAME_SET_RADIOSTATIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetRadioStations ), NULL, this );
//    Disconnect( ID_MAINFRAME_SET_PLAYLISTTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPlayListTracks ), NULL, this );
//    Disconnect( ID_MAINFRAME_SET_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPodcasts ), NULL, this );

    //Disconnect( wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEventHandler( guMainFrame::OnSysColorChanged ), NULL, this );

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


        Config->WriteBool( wxT( "ShowLibrary" ), m_ViewLibrary->IsChecked(), wxT( "ViewPanels" ) );
        Config->WriteBool( wxT( "ShowRadio" ), m_ViewRadios->IsChecked(), wxT( "ViewPanels" ) );
        Config->WriteBool( wxT( "ShowLastfm" ), m_ViewLastFM->IsChecked(), wxT( "ViewPanels" ) );
        Config->WriteBool( wxT( "ShowLyrics" ), m_ViewLyrics->IsChecked(), wxT( "ViewPanels" ) );
        Config->WriteBool( wxT( "ShowPodcasts" ), m_ViewPodcasts->IsChecked(), wxT( "ViewPanels" ) );
        Config->WriteBool( wxT( "ShowPlayLists" ), m_ViewPlayLists->IsChecked(), wxT( "ViewPanels" ) );

        Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
        Config->WriteStr( wxT( "NotebookLayout" ), m_CatNotebook->SavePerspective(), wxT( "Positions" ) );

        SaveLayouts();
    }

    if( m_LibUpdateThread )
    {
        m_LibUpdateThread->Pause();
        m_LibUpdateThread->Delete();
    }

    if( m_TaskBarIcon )
        delete m_TaskBarIcon;

    if( m_Db )
    {
        m_Db->Close();
        delete m_Db;
    }

    if( m_DbCache )
    {
        m_DbCache->Close();
        delete m_DbCache;
    }

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


    if( m_DBusServer )
    {
        delete m_DBusServer;
    }

    m_AuiManager.UnInit();
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
	guConfig *      Config = ( guConfig * ) guConfig::Get();

	m_MainMenu = new wxMenu();
	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_UPDATE_LIBRARY, _("&Update Library" ), _( "Update all songs from the directories configured" ), wxITEM_NORMAL );
	m_MainMenu->Append( MenuItem );

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_UPDATE_PODCASTS, _("Update &Podcasts" ), _( "Update the podcasts added" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
	m_MainMenu->Append( MenuItem );

	MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_UPDATE_COVERS, _("Update Covers"), _( "Try to download all missing covers" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_ ) );
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

    MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_CREATE, _( "New Layout" ), _( "Create a new layout" ) );
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
            if( Index == 3 )
                m_LayoutLoadMenu->AppendSeparator();
            if( Index > 3 )
            {
                MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DELETE + Index, m_LayoutName[ Index ], _( "Delete this user defined layout" ) );
                m_LayoutDelMenu->Append( MenuItem );
            }
        }
    }

    m_MainMenu->AppendSubMenu( m_LayoutLoadMenu, _( "Load Layout" ), _( "Set current view from a user defined layout" ) );
    m_MainMenu->AppendSubMenu( m_LayoutDelMenu, _( "Delete Layout" ), _( "Delete a user defined layout" ) );

    m_MainMenu->AppendSeparator();

    m_ViewLibrary = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_LIBRARY, wxString( _( "&Library" ) ), _( "Show/Hide the library panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewLibrary );
    m_ViewLibrary->Check( Config->ReadBool( wxT( "ShowLibrary" ), true, wxT( "ViewPanels" ) ) );

    m_ViewRadios = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_RADIO, wxString( _( "&Radio" ) ), _( "Show/Hide the radio panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewRadios );
    m_ViewRadios->Check( Config->ReadBool( wxT( "ShowRadio" ), true, wxT( "ViewPanels" ) ) );

    m_ViewLastFM = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_LASTFM, wxString( _( "Last.&fm" ) ), _( "Show/Hide the Last.fm panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewLastFM );
    m_ViewLastFM->Check( Config->ReadBool( wxT( "ShowLastfm" ), true, wxT( "ViewPanels" ) ) );

    m_ViewLyrics = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_LYRICS, wxString( _( "L&yrics" ) ), _( "Show/Hide the lyrics panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewLyrics );
    m_ViewLyrics->Check( Config->ReadBool( wxT( "ShowLyrics" ), true, wxT( "ViewPanels" ) ) );

    m_ViewPlayLists = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_PLAYLISTS, wxString( _( "&PlayLists" ) ), _( "Show/Hide the playlists panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewPlayLists );
    m_ViewPlayLists->Check( Config->ReadBool( wxT( "ShowPlayLists" ), true, wxT( "ViewPanels" ) ) );

    m_ViewPodcasts = new wxMenuItem( m_MainMenu, ID_MENU_VIEW_PODCASTS, wxString( _( "P&odcasts" ) ), _( "Show/Hide the podcasts panel" ), wxITEM_CHECK );
    m_MainMenu->Append( m_ViewPodcasts );
    m_ViewPodcasts->Check( Config->ReadBool( wxT( "ShowPodcasts" ), true, wxT( "ViewPanels" ) ) );

    MenuBar->Append( m_MainMenu, _( "&View" ) );

    m_MainMenu = new wxMenu();
    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_NEXTTRACK, wxString( _( "&Next Track" ) ), _( "Play the next track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_forward ) );
    m_MainMenu->Append( MenuItem );
    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_PREVTRACK, wxString( _( "&Prev. Track" ) ), _( "Play the previous track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_backward ) );
    m_MainMenu->Append( MenuItem );
    m_MainMenu->AppendSeparator();
    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_PLAY, wxString( _( "&Play" ) ), _( "Play or Pause the current track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playback_start ) );
    m_MainMenu->Append( MenuItem );
    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYERPANEL_STOP, wxString( _( "&Stop" ) ), _( "Stop the current played track" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playback_ ) );
    m_MainMenu->Append( MenuItem );
    m_MainMenu->AppendSeparator();
    m_PlaySmartMenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_SMARTPLAY, wxString( _( "&Smart Mode" ) ), _( "Update playlist based on Last.fm statics" ), wxITEM_CHECK );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search_engine ) );
    m_MainMenu->Append( m_PlaySmartMenuItem );
    m_PlaySmartMenuItem->Check( m_PlayerPanel->GetPlaySmart() );

    MenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_RANDOMPLAY, wxString( _( "R&andomize" ) ), _( "Randomize the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_shuffle ) );
    m_MainMenu->Append( MenuItem );
    m_LoopPlayListMenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_REPEATPLAYLIST, wxString( _( "&Repeat Playlist" ) ), _( "Repeat the tracks in the playlist" ), wxITEM_CHECK );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_repeat ) );
    m_MainMenu->Append( m_LoopPlayListMenuItem );
    m_LoopPlayListMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );

    m_LoopTrackMenuItem = new wxMenuItem( m_MainMenu, ID_PLAYER_PLAYLIST_REPEATTRACK, wxString( _( "&Repeat Track" ) ), _( "Repeat the current track in the playlist" ), wxITEM_CHECK );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_repeat ) );
    m_MainMenu->Append( m_LoopTrackMenuItem );
    m_LoopTrackMenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );

    MenuBar->Append( m_MainMenu, _( "&Control" ) );

    m_MainMenu = new wxMenu();
    MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_ABOUT, wxString( _( "&About" ) ), _( "Show information about guayadeque music player" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_volume_high ) );
    m_MainMenu->Append( MenuItem );
    MenuBar->Append( m_MainMenu, _( "&Help" ) );

	SetMenuBar( MenuBar );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnPreferences( wxCommandEvent &event )
{
    guPrefDialog * PrefDialog = new guPrefDialog( this, m_Db );
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

// -------------------------------------------------------------------------------- //
void guMainFrame::OnIconizeWindow( wxIconizeEvent &event )
{
//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    if( Config )
//    {
//        // If the icon
//        if( m_TaskBarIcon &&
//            Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "General" ) ) &&
//            Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) )
//        {
//            if( event.IsIconized() )
//            {
//                if( IsShown() )
//                    Show( false );
//            }
//            else
//            {
//                if( !IsShown() )
//                    Show( true );
//            }
//        }
//    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::LibraryUpdated( wxCommandEvent &event )
{
    m_Db->DoCleanUp();
//    guLogMessage( wxT( "Library Updated Event fired" ) );
    if( m_LibPanel )
        m_LibPanel->ReloadControls( event );
    m_LibUpdateThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateTrack( wxCommandEvent &event )
{
    if( m_TaskBarIcon )
    {
        guTrack * Track = ( guTrack * ) event.GetClientData();
        if( Track )
        {
            m_TaskBarIcon->SetIcon( m_AppIcon, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION "\r" ) +
                                               Track->m_ArtistName + wxT( "\n" ) +
                                               Track->m_SongName );
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
        if( !m_NBPerspective.IsEmpty() )
            m_CatNotebook->LoadPerspective( m_NBPerspective );
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CheckHideNotebook( void )
{
    if( !m_CatNotebook->GetPageCount() )
    {
        m_NBPerspective = m_CatNotebook->SavePerspective();
        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( m_CatNotebook );
        PaneInfo.Hide();
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLibrary( wxCommandEvent &event )
{
    guLogMessage( wxT( "View/Hide Library Panel called..." ) );
//	guConfig *      Config = ( guConfig * ) guConfig::Get();
//	Config->WriteBool( wxT( "ShowLibrary" ), event.IsChecked(), wxT( "ViewPanels" ) );
    if( event.IsChecked() )
    {
        CheckShowNotebook();

        if( !m_LibPanel )
            m_LibPanel = new guLibPanel( m_CatNotebook, m_Db, m_PlayerPanel );

        m_CatNotebook->InsertPage( 0, m_LibPanel, _( "Library" ), true );
    }
    else
    {
        int PageIndex = m_CatNotebook->GetPageIndex( m_LibPanel );
        if( PageIndex != wxNOT_FOUND )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }

        CheckHideNotebook();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewRadio( wxCommandEvent &event )
{
//	guConfig *      Config = ( guConfig * ) guConfig::Get();
//	Config->WriteBool( wxT( "ShowRadio" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        CheckShowNotebook();

        if( !m_RadioPanel )
            m_RadioPanel = new guRadioPanel( m_CatNotebook, m_Db, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 1, m_CatNotebook->GetPageCount() ), m_RadioPanel, _( "Radio" ), true );
    }
    else
    {
        int PageIndex = m_CatNotebook->GetPageIndex( m_RadioPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }

        CheckHideNotebook();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLastFM( wxCommandEvent &event )
{
//	guConfig *      Config = ( guConfig * ) guConfig::Get();
//	Config->WriteBool( wxT( "ShowLastfm" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        CheckShowNotebook();

        if( !m_LastFMPanel )
            m_LastFMPanel = new guLastFMPanel( m_CatNotebook, m_Db, m_DbCache, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 2, m_CatNotebook->GetPageCount() ), m_LastFMPanel, _( "Last.fm" ), true );
    }
    else
    {
        int PageIndex = m_CatNotebook->GetPageIndex( m_LastFMPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }

        CheckHideNotebook();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLyrics( wxCommandEvent &event )
{
//	guConfig *      Config = ( guConfig * ) guConfig::Get();
//	Config->WriteBool( wxT( "ShowLyrics" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        CheckShowNotebook();

        if( !m_LyricsPanel )
            m_LyricsPanel = new guLyricsPanel( m_CatNotebook, m_Db );

        m_CatNotebook->InsertPage( wxMin( 3, m_CatNotebook->GetPageCount() ), m_LyricsPanel, _( "Lyrics" ), true );
    }
    else
    {
        int PageIndex = m_CatNotebook->GetPageIndex( m_LyricsPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }

        CheckHideNotebook();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPodcasts( wxCommandEvent &event )
{
//	guConfig *      Config = ( guConfig * ) guConfig::Get();
//	Config->WriteBool( wxT( "ShowPodcasts" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        CheckShowNotebook();

        if( !m_PodcastsPanel )
            m_PodcastsPanel = new guPodcastPanel( m_CatNotebook, m_Db, this, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 5, m_CatNotebook->GetPageCount() ), m_PodcastsPanel, _( "Podcasts" ), true );
    }
    else
    {
        int PageIndex = m_CatNotebook->GetPageIndex( m_PodcastsPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }

        CheckHideNotebook();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPlayLists( wxCommandEvent &event )
{
//	guConfig *      Config = ( guConfig * ) guConfig::Get();
//	Config->WriteBool( wxT( "ShowPlayLists" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        CheckShowNotebook();

        if( !m_PlayListPanel )
            m_PlayListPanel = new guPlayListPanel( m_CatNotebook, m_Db, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 4, m_CatNotebook->GetPageCount() ), m_PlayListPanel, _( "PlayLists" ), true );
    }
    else
    {
        int PageIndex = m_CatNotebook->GetPageIndex( m_PlayListPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }

        CheckHideNotebook();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectAlbumName( wxCommandEvent &event )
{
    wxString * album = ( wxString * ) event.GetClientData();
    if( album )
    {
        m_CatNotebook->SetSelection( 0 );
        m_LibPanel->SelectAlbumName( * album );
        delete album;
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnSelectArtistName( wxCommandEvent &event )
{
    wxString * artist = ( wxString * ) event.GetClientData();
    if( artist )
    {
        m_CatNotebook->SetSelection( 0 );
        m_LibPanel->SelectArtistName( * artist );
        delete artist;
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

    if( m_PlayerPanel )
        m_PlayerPanel->UpdatePlayListFilters();
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

    wxPanel * CurPage = ( wxPanel * ) ctrl->GetPage( event.GetSelection() );
    m_CatNotebook->RemovePage( event.GetSelection() );

    if( CurPage == m_LibPanel )
    {
        m_ViewLibrary->Check( false );
    }
    else if( CurPage == m_RadioPanel )
    {
        m_ViewRadios->Check( false );
    }
    else if( CurPage == m_LastFMPanel )
    {
        m_ViewLastFM->Check( false );
    }
    else if( CurPage == m_LyricsPanel )
    {
        m_ViewLyrics->Check( false );
    }
    else if( CurPage == m_PlayListPanel )
    {
        m_ViewPlayLists->Check( false );
    }
    else if( CurPage == m_PodcastsPanel )
    {
        m_ViewPodcasts->Check( false );
    }

    CheckHideNotebook();
    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnUpdateSelInfo( wxCommandEvent &event )
{
    if( m_CurrentPage == ( wxWindow * ) m_LibPanel )
    {
        m_Db->GetTracksCounters( &m_SelCount, &m_SelLength, &m_SelSize );

        m_MainStatusBar->SetSelInfo( wxString::Format( _( "%llu tracks,   %s,   %s" ),
            m_SelCount.GetValue(),
            LenToString( m_SelLength.GetLo() ).c_str(),
            SizeToString( m_SelSize.GetValue() ).c_str() ) );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_RadioPanel )
    {
        m_Db->GetRadioCounter( &m_SelCount );
        m_MainStatusBar->SetSelInfo( wxString::Format( _( "%llu stations" ), m_SelCount.GetValue() ) );
    }
    else if( m_CurrentPage == ( wxWindow * ) m_PlayListPanel )
    {
        if( m_PlayListPanel->GetPlayListCounters( &m_SelCount, &m_SelLength, &m_SelSize ) )
        {
            m_MainStatusBar->SetSelInfo( wxString::Format( _( "%llu tracks,   %s,   %s" ),
                m_SelCount.GetValue(),
                LenToString( m_SelLength.GetLo() ).c_str(),
                SizeToString( m_SelSize.GetValue() ).c_str() ) );
        }
        else
        {
            m_MainStatusBar->SetSelInfo( wxEmptyString );
        }
    }
    else if( m_CurrentPage == ( wxWindow * ) m_PodcastsPanel )
    {
        m_Db->GetPodcastCounters( &m_SelCount, &m_SelLength, &m_SelSize );

        m_MainStatusBar->SetSelInfo( wxString::Format( _( "%llu podcasts,   %s,   %s" ),
            m_SelCount.GetValue(),
            LenToString( m_SelLength.GetLo() ).c_str(),
            SizeToString( m_SelSize.GetValue() ).c_str() ) );
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
void guMainFrame::OnIdle( wxIdleEvent& WXUNUSED( event ) )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
//    m_PlayerSplitter->SetSashPosition( Config->ReadNum( wxT( "PlayerSashPos" ), 280, wxT( "Positions" ) ) );
    Disconnect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );


    // If the database need to be updated
    if( m_Db->NeedUpdate() || Config->ReadBool( wxT( "UpdateLibOnStart" ), false, wxT( "General" ) ) )
    {
        guLogMessage( wxT( "Database updating started." ) );
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_UPDATE_LIBRARY );
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
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateTaskBarIcon( void )
{
    m_TaskBarIcon = new guTaskBarIcon( this, m_PlayerPanel );
    if( m_TaskBarIcon )
    {
        m_TaskBarIcon->SetIcon( m_AppIcon, wxT( "Guayadeque Music Player " ID_GUAYADEQUE_VERSION ) );
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

    m_LayoutName = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "Layouts" ) );
    m_LayoutData = Config->ReadAStr( wxT( "Data" ), wxEmptyString, wxT( "Layouts" ) );
    m_LayoutTabs = Config->ReadAStr( wxT( "Tabs" ), wxEmptyString, wxT( "Layouts" ) );

    size_t Count = wxMin( wxMin( m_LayoutName.Count(), m_LayoutData.Count() ), m_LayoutTabs.Count() );

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
    Config->WriteAStr( wxT( "Name" ), m_LayoutName, wxT( "Layouts" ) );
    Config->WriteAStr( wxT( "Data" ), m_LayoutData, wxT( "Layouts" ), false );
    Config->WriteAStr( wxT( "Tabs" ), m_LayoutTabs, wxT( "Layouts" ), false );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCreateNewLayout( wxCommandEvent &event )
{
    wxTextEntryDialog EntryDialog( this, _( "Enter the layout name:"), _( "New Layout" ) );

    EntryDialog.SetValue( wxString::Format( _( "Layout %u" ), unsigned( m_LayoutName.GetCount() + 1 ) ) );

    if( EntryDialog.ShowModal() == wxID_OK )
    {
        m_LayoutName.Add( EntryDialog.GetValue() );
        m_LayoutData.Add( m_AuiManager.SavePerspective() );
        m_LayoutTabs.Add( m_CatNotebook->SavePerspective() );

        m_LayoutLoadMenu->Append( ID_MENU_LAYOUT_LOAD + m_LayoutName.Count() - 1,
                EntryDialog.GetValue(), _( "Load this user defined layout" ) );
        m_LayoutDelMenu->Append( ID_MENU_LAYOUT_LOAD + m_LayoutName.Count() - 1,
                EntryDialog.GetValue(), _( "Load this user defined layout" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnLoadLayout( wxCommandEvent &event )
{
    int Layout = event.GetId() - ID_MENU_LAYOUT_LOAD;
    wxAuiPaneInfo &NBPaneInfo = m_AuiManager.GetPane( m_CatNotebook );
    bool NBIsShown = NBPaneInfo.IsShown();

    guLogMessage( wxT( "Load Layout %i" ), Layout );
    m_AuiManager.LoadPerspective( m_LayoutData[ Layout ] );
    m_CatNotebook->LoadPerspective( m_LayoutTabs[ Layout ] );
    if( !NBIsShown )
    {
        NBPaneInfo.Hide();
        m_AuiManager.Update();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnDeleteLayout( wxCommandEvent &event )
{
    int Layout = event.GetId() - ID_MENU_LAYOUT_DELETE;
    guLogMessage( wxT( "Delete Layout %i" ), Layout );
    int Index;
    int Count = m_LayoutName.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_LayoutLoadMenu->Destroy( ID_MENU_LAYOUT_LOAD + Index );
        m_LayoutDelMenu->Destroy( ID_MENU_LAYOUT_DELETE + Index );
    }

    m_LayoutName.RemoveAt( Layout );
    m_LayoutData.RemoveAt( Layout );
    m_LayoutTabs.RemoveAt( Layout );

    wxMenuItem * MenuItem;
    Count = m_LayoutName.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_LOAD + Index, m_LayoutName[ Index ], _( "Load this user defined layout" ) );
        m_LayoutLoadMenu->Append( MenuItem );
        if( Index == 3 )
            m_LayoutLoadMenu->AppendSeparator();
        if( Index > 3 )
        {
            MenuItem = new wxMenuItem( m_MainMenu, ID_MENU_LAYOUT_DELETE + Index, m_LayoutName[ Index ], _( "Delete this user defined layout" ) );
            m_LayoutDelMenu->Append( MenuItem );
        }
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
                //if( !DownloadFile( AlbumInfo.ImageLink, Path + wxT( "/cover.jpg" ) ) )
                if( !DownloadImage( AlbumInfo.m_ImageLink, Path + wxT( "/cover.jpg" ) ) )
                {
                    guLogWarning( wxT( "Could not download cover file" ) );
                }
                else
                {
//                    DownloadedCovers++;
                    Db->SetAlbumCover( AlbumId, Path + wxT( "/cover.jpg" ) );
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
            FileName.Replace( wxT( "{a}" ), ( * m_Tracks )[ index ].m_ArtistName );
            FileName.Replace( wxT( "{b}" ), ( * m_Tracks )[ index ].m_AlbumName );
            FileName.Replace( wxT( "{f}" ), wxFileNameFromPath( ( * m_Tracks )[ index ].m_FileName ) );
            FileName.Replace( wxT( "{g}" ), ( * m_Tracks )[ index ].m_GenreName );
            FileName.Replace( wxT( "{n}" ), wxString::Format( wxT( "%02u" ), ( * m_Tracks )[ index ].m_Number ) );
            FileName.Replace( wxT( "{t}" ), ( * m_Tracks )[ index ].m_SongName );
            FileName.Replace( wxT( "{y}" ), wxString::Format( wxT( "%u" ), ( * m_Tracks )[ index ].m_Year ) );
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
