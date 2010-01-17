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
	wxPanel *       MultiPanel;
//	wxPanel*        FileSysPanel;
	wxBoxSizer*     MultiSizer;
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

    m_Initiated = false;
    m_LibCount = 0;
    m_RadCount = 0;
    m_PLCount = 0;
    m_PodCount = 0;

    //
    m_AppIcon.CopyFromBitmap( guImage( guIMAGE_INDEX_guayadeque ) );

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

    CreateMenu();

	m_MainStatusBar = new guStatusBar( this );
	SetStatusBar(  m_MainStatusBar );
	MainFrameSizer = new wxBoxSizer( wxVERTICAL );
	SetStatusText( _( "Welcome to guayadeque!" ) );

	m_PlayerSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
    m_PlayerSplitter->SetMinimumPaneSize( 200 );

	m_PlayerPanel = new guPlayerPanel( m_PlayerSplitter, m_Db ); //wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	MultiPanel = new wxPanel( m_PlayerSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	MultiSizer = new wxBoxSizer( wxVERTICAL );

	m_CatNotebook = new wxNotebook( MultiPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

    // Library Page
	m_LibPanel = new guLibPanel( m_CatNotebook, m_Db, m_PlayerPanel );
	m_CatNotebook->AddPage( m_LibPanel, _( "Library" ), true );

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
        m_LyricsPanel = new guLyricsPanel( m_CatNotebook );
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

    // FileSystem Page
//    if( Config->ReadBool( wxT( "ShowFileSys" ), true, wxT( "ViewPanels" ) ) )
//    {
//	      FileSysPanel = new wxPanel( CatNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//	      CatNotebook->AddPage( FileSysPanel, wxT("FileSystem"), false );
//    }

	MultiSizer->Add( m_CatNotebook, 1, wxEXPAND | wxALL, 2 );

	MultiPanel->SetSizer( MultiSizer );
	MultiPanel->Layout();
	MultiSizer->Fit( MultiPanel );

	m_PlayerSplitter->SplitVertically( m_PlayerPanel, MultiPanel, Config->ReadNum( wxT( "PlayerSashPos" ), 280, wxT( "Positions" ) ) );
	MainFrameSizer->Add( m_PlayerSplitter, 1, wxEXPAND, 2 );

	this->SetSizer( MainFrameSizer );
	this->Layout();


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
    Connect( ID_PLAYER_PLAYLIST_REPEATPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Connect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ), NULL, this );

    Connect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );

    Connect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

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

	m_CatNotebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );

    Connect( ID_MAINFRAME_SET_LIBTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetLibTracks ), NULL, this );
    Connect( ID_MAINFRAME_SET_RADIOSTATIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetRadioStations ), NULL, this );
    Connect( ID_MAINFRAME_SET_PLAYLISTTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPlayListTracks ), NULL, this );
    Connect( ID_MAINFRAME_SET_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPodcasts ), NULL, this );

    //Connect( wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEventHandler( guMainFrame::OnSysColorChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guMainFrame::~guMainFrame()
{
	Disconnect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );

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
    Disconnect( ID_PLAYER_PLAYLIST_REPEATPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ), NULL, this );
    Disconnect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ), NULL, this );

    Disconnect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ), NULL, this );

    Disconnect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ), NULL, this );

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

	m_CatNotebook->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( guMainFrame::OnPageChanged ), NULL, this );

    Disconnect( ID_MAINFRAME_SET_LIBTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetLibTracks ), NULL, this );
    Disconnect( ID_MAINFRAME_SET_RADIOSTATIONS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetRadioStations ), NULL, this );
    Disconnect( ID_MAINFRAME_SET_PLAYLISTTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPlayListTracks ), NULL, this );
    Disconnect( ID_MAINFRAME_SET_PODCASTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::SetPodcasts ), NULL, this );

    //Disconnect( wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEventHandler( guMainFrame::OnSysColorChanged ), NULL, this );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteNum( wxT( "PlayerSashPos" ), m_PlayerSplitter->GetSashPosition(), wxT( "Positions" ) );
        wxPoint MainWindowPos = GetPosition();
        Config->WriteNum( wxT( "MainWindowPosX" ), MainWindowPos.x, wxT( "Positions" ) );
        Config->WriteNum( wxT( "MainWindowPosY" ), MainWindowPos.y, wxT( "Positions" ) );
        wxSize MainWindowSize = GetSize();
        Config->WriteNum( wxT( "MainWindowSizeWidth" ), MainWindowSize.x, wxT( "Positions" ) );
        Config->WriteNum( wxT( "MainWindowSizeHeight" ), MainWindowSize.y, wxT( "Positions" ) );
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
	wxMenu * Menu;
	wxMenuItem * MenuItem;
	guConfig *      Config = ( guConfig * ) guConfig::Get();

	Menu = new wxMenu();
	MenuItem = new wxMenuItem( Menu, ID_MENU_UPDATE_LIBRARY, wxString( _("&Update Library" ) ) , _( "Update all songs from the directories configured" ), wxITEM_NORMAL );
	Menu->Append( MenuItem );

	MenuItem = new wxMenuItem( Menu, ID_MENU_UPDATE_PODCASTS, wxString( _("Update &Podcasts" ) ) , _( "Update the podcasts added" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
	Menu->Append( MenuItem );

	MenuItem = new wxMenuItem( Menu, ID_MENU_UPDATE_COVERS, wxString( _("Update Covers") ) , _( "Try to download all missing covers" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_ ) );
	Menu->Append( MenuItem );

	Menu->AppendSeparator();

	MenuItem = new wxMenuItem( Menu, ID_MENU_PREFERENCES, wxString( _( "&Preferences" ) ) , _( "Change the options of the application" ), wxITEM_NORMAL );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_pref_general ) );
	Menu->Append( MenuItem );

	Menu->AppendSeparator();

	MenuItem = new wxMenuItem( Menu, ID_MENU_QUIT, wxString( _( "&Quit" ) ) , _( "Exits the application" ), wxITEM_NORMAL );
	Menu->Append( MenuItem );

	MenuBar = new wxMenuBar( 0 );
	MenuBar->Append( Menu, _( "&Library" ) );

    Menu = new wxMenu();
//    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_LIBRARY, wxString( _( "&Library" ) ), _( "Show/Hide the library panel" ), wxITEM_CHECK );
//    Menu->Append( MenuItem );
//    MenuItem->Check( true );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_RADIO, wxString( _( "&Radio" ) ), _( "Show/Hide the radio panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( Config->ReadBool( wxT( "ShowRadio" ), true, wxT( "ViewPanels" ) ) );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_LASTFM, wxString( _( "Last.&fm" ) ), _( "Show/Hide the Last.fm panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( Config->ReadBool( wxT( "ShowLastfm" ), true, wxT( "ViewPanels" ) ) );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_LYRICS, wxString( _( "L&yrics" ) ), _( "Show/Hide the lyrics panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( Config->ReadBool( wxT( "ShowLyrics" ), true, wxT( "ViewPanels" ) ) );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_PLAYLISTS, wxString( _( "&PlayLists" ) ), _( "Show/Hide the playlists panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( Config->ReadBool( wxT( "ShowPlayLists" ), true, wxT( "ViewPanels" ) ) );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_PODCASTS, wxString( _( "P&odcasts" ) ), _( "Show/Hide the podcasts panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( Config->ReadBool( wxT( "ShowPodcasts" ), true, wxT( "ViewPanels" ) ) );

    MenuBar->Append( Menu, _( "&View" ) );

    Menu = new wxMenu();
    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_NEXTTRACK, wxString( _( "&Next Track" ) ), _( "Play the next track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_forward ) );
    Menu->Append( MenuItem );
    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PREVTRACK, wxString( _( "&Prev. Track" ) ), _( "Play the previous track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_skip_backward ) );
    Menu->Append( MenuItem );
    Menu->AppendSeparator();
    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PLAY, wxString( _( "&Play" ) ), _( "Play or Pause the current track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playback_start ) );
    Menu->Append( MenuItem );
//    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_STOP, wxString( _( "&Stop" ) ), _( "Stop the current played track" ), wxITEM_NORMAL );
//    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playback_pause ) );
//    Menu->Append( MenuItem );
    Menu->AppendSeparator();
    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SMARTPLAY, wxString( _( "&Smart Mode" ) ), _( "Update playlist based on Last.fm statics" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search_engine ) );
    Menu->Append( MenuItem );
    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY, wxString( _( "R&andomize" ) ), _( "Randomize the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_shuffle ) );
    Menu->Append( MenuItem );
    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_REPEATPLAY, wxString( _( "&Repeat" ) ), _( "Repeat the tracks in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_playlist_repeat ) );
    Menu->Append( MenuItem );
    MenuBar->Append( Menu, _( "&Control" ) );

    Menu = new wxMenu();
    MenuItem = new wxMenuItem( Menu, ID_MENU_ABOUT, wxString( _( "&About" ) ), _( "Show information about guayadeque music player" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_volume_high ) );
    Menu->Append( MenuItem );
    MenuBar->Append( Menu, _( "&Help" ) );

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
        m_PlayerPanel->OnSmartPlayButtonClick( event );
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
        m_PlayerPanel->OnRepeatPlayButtonClick( event );
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

// -------------------------------------------------------------------------------- //
int GetPageIndex( wxNotebook * Notebook, wxPanel * Page )
{
    int index;
    int count = Notebook->GetPageCount();
    for( index = 0; index < count; index++ )
    {
        if( Notebook->GetPage( index ) == Page )
            return index;
    }
    return -1;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLibrary( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
        m_CatNotebook->InsertPage( 0, m_LibPanel, _( "Library" ), true );
    }
    else
    {
        int PageIndex = GetPageIndex( m_CatNotebook, m_LibPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }
    }

}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewRadio( wxCommandEvent &event )
{
	guConfig *      Config = ( guConfig * ) guConfig::Get();
	Config->WriteBool( wxT( "ShowRadio" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        if( !m_RadioPanel )
            m_RadioPanel = new guRadioPanel( m_CatNotebook, m_Db, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 1, m_CatNotebook->GetPageCount() ), m_RadioPanel, _( "Radio" ), true );
    }
    else
    {
        int PageIndex = GetPageIndex( m_CatNotebook, m_RadioPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLastFM( wxCommandEvent &event )
{
	guConfig *      Config = ( guConfig * ) guConfig::Get();
	Config->WriteBool( wxT( "ShowLastfm" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        if( !m_LastFMPanel )
            m_LastFMPanel = new guLastFMPanel( m_CatNotebook, m_Db, m_DbCache, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 2, m_CatNotebook->GetPageCount() ), m_LastFMPanel, _( "Last.fm" ), true );
    }
    else
    {
        int PageIndex = GetPageIndex( m_CatNotebook, m_LastFMPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewLyrics( wxCommandEvent &event )
{
	guConfig *      Config = ( guConfig * ) guConfig::Get();
	Config->WriteBool( wxT( "ShowLyrics" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        if( !m_LyricsPanel )
            m_LyricsPanel = new guLyricsPanel( m_CatNotebook );

        m_CatNotebook->InsertPage( wxMin( 3, m_CatNotebook->GetPageCount() ), m_LyricsPanel, _( "Lyrics" ), true );
    }
    else
    {
        int PageIndex = GetPageIndex( m_CatNotebook, m_LyricsPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPodcasts( wxCommandEvent &event )
{
	guConfig *      Config = ( guConfig * ) guConfig::Get();
	Config->WriteBool( wxT( "ShowPodcasts" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        if( !m_PodcastsPanel )
            m_PodcastsPanel = new guPodcastPanel( m_CatNotebook, m_Db, this, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 5, m_CatNotebook->GetPageCount() ), m_PodcastsPanel, _( "Podcasts" ), true );
    }
    else
    {
        int PageIndex = GetPageIndex( m_CatNotebook, m_PodcastsPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnViewPlayLists( wxCommandEvent &event )
{
	guConfig *      Config = ( guConfig * ) guConfig::Get();
	Config->WriteBool( wxT( "ShowPlayLists" ), event.IsChecked(), wxT( "ViewPanels" ) );

    if( event.IsChecked() )
    {
        if( !m_PlayListPanel )
            m_PlayListPanel = new guPlayListPanel( m_CatNotebook, m_Db, m_PlayerPanel );

        m_CatNotebook->InsertPage( wxMin( 4, m_CatNotebook->GetPageCount() ), m_PlayListPanel, _( "PlayLists" ), true );
    }
    else
    {
        int PageIndex = GetPageIndex( m_CatNotebook, m_PlayListPanel );
        if( PageIndex >= 0 )
        {
            m_CatNotebook->RemovePage( PageIndex );
        }
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
void guMainFrame::OnPageChanged( wxNotebookEvent &event )
{
    m_CurrentPage = m_CatNotebook->GetCurrentPage();
    m_MainStatusBar->SetTrackCount( wxNOT_FOUND );
    m_LastCount = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnIdle( wxIdleEvent& WXUNUSED( event ) )
{
    if( !m_Initiated )
    {
        m_Initiated = true;
        //
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_PlayerSplitter->SetSashPosition( Config->ReadNum( wxT( "PlayerSashPos" ), 280, wxT( "Positions" ) ) );
        //Disconnect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::OnIdle ), NULL, this );


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

    if( m_CurrentPage == ( wxWindow * ) m_LibPanel )
    {
        if( m_LastCount != m_LibCount )
        {
            m_MainStatusBar->SetTrackCount( m_LibCount, wxT( "tracks" ) );
            m_LastCount = m_LibCount;
        }
    }
    else if( m_CurrentPage == ( wxWindow * ) m_RadioPanel )
    {
        if( m_LastCount != m_RadCount )
        {
            m_MainStatusBar->SetTrackCount( m_RadCount, wxT( "stations" ) );
            m_LastCount = m_RadCount;
        }
    }
    else if( m_CurrentPage == ( wxWindow * ) m_PlayListPanel )
    {
        if( m_LastCount != m_PLCount )
        {
            m_MainStatusBar->SetTrackCount( m_PLCount, wxT( "tracks" ) );
            m_LastCount = m_PLCount;
        }
    }
    else if( m_CurrentPage == ( wxWindow * ) m_PodcastsPanel )
    {
        if( m_LastCount != m_PodCount )
        {
            m_MainStatusBar->SetTrackCount( m_PodCount, wxT( "podcasts" ) );
            m_LastCount = m_PodCount;
        }
    }
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
void guMainFrame::SetLibTracks( wxCommandEvent &event )
{
    m_LibCount = event.GetInt();
    m_LastCount = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::SetRadioStations( wxCommandEvent &event )
{
    m_RadCount = event.GetInt();
    m_LastCount = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::SetPlayListTracks( wxCommandEvent &event )
{
    m_PLCount = event.GetInt();
    m_LastCount = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guMainFrame::SetPodcasts( wxCommandEvent &event )
{
    m_PodCount = event.GetInt();
    m_LastCount = wxNOT_FOUND;
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
