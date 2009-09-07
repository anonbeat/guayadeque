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
#include "TaskBar.h"
#include "Utils.h"

#include <wx/statline.h>
#include <wx/notebook.h>

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
    m_Db = new DbLibrary( wxGetHomeDir() + wxT( "/.guayadeque/guayadeque.db" ) );
    if( !m_Db )
    {
        guLogError( wxT( "Could not open the guayadeque database" ) );
        return;
    }

    m_Db->SetLibPath( Config->ReadAStr( wxT( "LibPath" ),
                                      wxGetHomeDir() + wxT( "/Music" ),
                                      wxT( "LibPaths" ) ) );

    m_LibUpdateThread = NULL;

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
	m_RadioPanel = new guRadioPanel( m_CatNotebook, m_Db, m_PlayerPanel );
	m_CatNotebook->AddPage( m_RadioPanel, _( "Radio" ), false );

    // LastFM Info Panel
	m_LastFMPanel = new guLastFMPanel( m_CatNotebook, m_Db, m_PlayerPanel );
	m_CatNotebook->AddPage( m_LastFMPanel, _( "Last.fm" ), false );

    // Lyrics Panel
    m_LyricsPanel = new guLyricsPanel( m_CatNotebook );
    m_CatNotebook->AddPage( m_LyricsPanel, _( "Lyrics" ), false );

    // PlayList Page
    m_PlayListPanel = new guPlayListPanel( m_CatNotebook, m_Db, m_PlayerPanel );
	m_CatNotebook->AddPage( m_PlayListPanel, _( "PlayLists" ), false );

    // FileSystem Page
//	FileSysPanel = new wxPanel( CatNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//	CatNotebook->AddPage( FileSysPanel, wxT("FileSystem"), false );

	MultiSizer->Add( m_CatNotebook, 1, wxEXPAND | wxALL, 2 );

	MultiPanel->SetSizer( MultiSizer );
	MultiPanel->Layout();
	MultiSizer->Fit( MultiPanel );

	m_PlayerSplitter->SplitVertically( m_PlayerPanel, MultiPanel, Config->ReadNum( wxT( "PlayerSashPos" ), 280, wxT( "Positions" ) ) );
	MainFrameSizer->Add( m_PlayerSplitter, 1, wxEXPAND, 2 );

	this->SetSizer( MainFrameSizer );
	this->Layout();

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

    //
	m_PlayerSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::PlayerSplitterOnIdle ), NULL, this );

    Connect( ID_MENU_UPDATE_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLibrary ) );
    Connect( ID_MENU_UPDATE_COVERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateCovers ) );
    Connect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ) );
    Connect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ) );
    Connect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ) );
    Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    Connect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ) );

    Connect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ) );
    Connect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ) );
    Connect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ) );
    Connect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ) );

	Connect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectAlbumName ), NULL, this );
	Connect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectArtistName ), NULL, this );

	Connect( ID_GENRE_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGenreSetSelection ), NULL, this );
	Connect( ID_ARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnArtistSetSelection ), NULL, this );
	Connect( ID_ALBUM_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumSetSelection ), NULL, this );

    Connect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlay ) );
    Connect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStop ) );
    Connect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextTrack ) );
    Connect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevTrack ) );
    Connect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSmartPlay ) );
    Connect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRandomize ) );
    Connect( ID_PLAYER_PLAYLIST_REPEATPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ) );
    Connect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ) );

    Connect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ) );

    Connect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ) );

    Connect( ID_MENU_VIEW_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLibrary ) );
    Connect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ) );
    Connect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ) );
    Connect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ) );
    Connect( ID_MENU_VIEW_PLAYLISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPlayLists ) );

    Connect( ID_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ) );
    Connect( ID_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ) );
    Connect( ID_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ) );
    Connect( ID_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ) );

    Connect( ID_PLAYLIST_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayListUpdated ) );

    // If the database need to be updated
    if( m_Db->NeedUpdate() || Config->ReadBool( wxT( "UpdateLibOnStart" ), false, wxT( "General" ) ) )
    {
        guLogMessage( wxT( "Database updating started." ) );
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_UPDATE_LIBRARY );
        wxPostEvent( this, event );
    }

}

// -------------------------------------------------------------------------------- //
guMainFrame::~guMainFrame()
{
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

    // destroy the mpris object
    if( m_MPRIS )
    {
        delete m_MPRIS;
    }

    Disconnect( ID_MENU_UPDATE_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLibrary ) );
    Disconnect( ID_MENU_UPDATE_COVERS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateCovers ) );
    Disconnect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnQuit ) );
    Disconnect( ID_LIBRARY_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::LibraryUpdated ) );
    Disconnect( ID_AUDIOSCROBBLE_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAudioScrobbleUpdate ) );
    Disconnect( ID_PLAYERPANEL_TRACKCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateTrack ) );
    Disconnect( ID_PLAYERPANEL_STATUSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerStatusChanged ) );
    Disconnect( ID_PLAYERPANEL_TRACKLISTCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerTrackListChanged ) );
    Disconnect( ID_PLAYERPANEL_CAPSCHANGED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlayerCapsChanged ) );

	Disconnect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectAlbumName ), NULL, this );
	Disconnect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSelectArtistName ), NULL, this );

	Disconnect( ID_GENRE_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGenreSetSelection ), NULL, this );
	Disconnect( ID_ARTIST_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnArtistSetSelection ), NULL, this );
	Disconnect( ID_ALBUM_SETSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAlbumSetSelection ), NULL, this );

    Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( guMainFrame::OnCloseWindow ), NULL, this );
    Disconnect( ID_MENU_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPreferences ) );

    Disconnect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPlay ) );
    Disconnect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnStop ) );
    Disconnect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnNextTrack ) );
    Disconnect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnPrevTrack ) );
    Disconnect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnSmartPlay ) );
    Disconnect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRandomize ) );
    Disconnect( ID_PLAYER_PLAYLIST_REPEATPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnRepeat ) );
    Disconnect( ID_MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnAbout ) );

    Disconnect( ID_MAINFRAME_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnCopyTracksTo ) );

    Disconnect( ID_LABEL_UPDATELABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnUpdateLabels ) );

    Disconnect( ID_MENU_VIEW_LIBRARY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLibrary ) );
    Disconnect( ID_MENU_VIEW_RADIO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewRadio ) );
    Disconnect( ID_MENU_VIEW_LASTFM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLastFM ) );
    Disconnect( ID_MENU_VIEW_LYRICS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewLyrics ) );
    Disconnect( ID_MENU_VIEW_PLAYLISTS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnViewPlayLists ) );

    Disconnect( ID_GAUGE_PULSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugePulse ) );
    Disconnect( ID_GAUGE_SETMAX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeSetMax ) );
    Disconnect( ID_GAUGE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeUpdate ) );
    Disconnect( ID_GAUGE_REMOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMainFrame::OnGaugeRemove ) );
}


// -------------------------------------------------------------------------------- //
void guMainFrame::CreateMenu()
{
	wxMenuBar * MenuBar;
	wxMenu * Menu;
	wxMenuItem * MenuItem;

	Menu = new wxMenu();
	MenuItem = new wxMenuItem( Menu, ID_MENU_UPDATE_LIBRARY, wxString( _("&Update Library" ) ) , _( "Update all songs from the directories configured" ), wxITEM_NORMAL );
	Menu->Append( MenuItem );

	MenuItem = new wxMenuItem( Menu, ID_MENU_UPDATE_COVERS, wxString( _("Update Covers") ) , _( "Try to download all missing covers" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
	Menu->Append( MenuItem );

	Menu->AppendSeparator();

	MenuItem = new wxMenuItem( Menu, ID_MENU_PREFERENCES, wxString( _( "&Preferences" ) ) , _( "Change the options of the application" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
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
    MenuItem->Check( true );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_LASTFM, wxString( _( "Last.&fm" ) ), _( "Show/Hide the Last.fm panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( true );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_LYRICS, wxString( _( "L&yrics" ) ), _( "Show/Hide the lyrics panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( true );

    MenuItem = new wxMenuItem( Menu, ID_MENU_VIEW_PLAYLISTS, wxString( _( "&PlayLists" ) ), _( "Show/Hide the playlists panel" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( true );

    MenuBar->Append( Menu, _( "&View" ) );

    Menu = new wxMenu();
    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_NEXTTRACK, wxString( _( "&Next Track" ) ), _( "Play the next track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_skip_forward ) );
    Menu->Append( MenuItem );
    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PREVTRACK, wxString( _( "&Prev. Track" ) ), _( "Play the previous track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_skip_backward ) );
    Menu->Append( MenuItem );
    Menu->AppendSeparator();
    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PLAY, wxString( _( "&Play" ) ), _( "Play or Pause the current track in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );
    MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_STOP, wxString( _( "&Stop" ) ), _( "Stop the current played track" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_stop ) );
    Menu->Append( MenuItem );
    Menu->AppendSeparator();
    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SMARTPLAY, wxString( _( "&Smart Mode" ) ), _( "Update playlist based on Last.fm statics" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playlist_smart ) );
    Menu->Append( MenuItem );
    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY, wxString( _( "R&andomize" ) ), _( "Randomize the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playlist_shuffle ) );
    Menu->Append( MenuItem );
    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_REPEATPLAY, wxString( _( "&Repeat" ) ), _( "Repeat the tracks in the playlist" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playlist_repeat ) );
    Menu->Append( MenuItem );
    MenuBar->Append( Menu, _( "&Control" ) );

    Menu = new wxMenu();
    MenuItem = new wxMenuItem( Menu, ID_MENU_ABOUT, wxString( _( "&About" ) ), _( "Show information about guayadeque music player" ), wxITEM_NORMAL );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_guayadeque ) );
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
        }
        PrefDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnCloseWindow( wxCloseEvent &event )
{
    //guLogMessage( wxT( "OnCloseWindow called..." ) );
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
        wxArrayString * Params = ( wxArrayString * ) event.GetClientData();
        //Params->Clear();
        delete Params;
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
    int GaugeId = ( ( guStatusBar * ) GetStatusBar() )->AddGauge();
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
    m_LibUpdateThread = new guLibUpdateThread( m_Db );
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
                    int GaugeId = ( ( guStatusBar * ) GetStatusBar() )->AddGauge();
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
        else
        {
            delete Tracks;
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
    if( event.IsChecked() )
    {
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
    if( event.IsChecked() )
    {
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
    if( event.IsChecked() )
    {
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
void guMainFrame::OnViewPlayLists( wxCommandEvent &event )
{
    if( event.IsChecked() )
    {
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
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugePulse( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Pulse message for gauge %u" ), event.GetInt() );
    guStatusBar * StatusBar = ( guStatusBar * ) GetStatusBar();
    StatusBar->Pulse( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugeSetMax( wxCommandEvent &event )
{
    //guLogMessage( wxT( "SetMax message for gauge %u to %u" ), event.GetInt(), event.GetExtraLong() );
    guStatusBar * StatusBar = ( guStatusBar * ) GetStatusBar();
    StatusBar->SetTotal( event.GetInt(), event.GetExtraLong() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugeUpdate( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Update message for gauge %u" ), event.GetInt() );
    guStatusBar * StatusBar = ( guStatusBar * ) GetStatusBar();
    StatusBar->SetValue( event.GetInt(), event.GetExtraLong() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::OnGaugeRemove( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Remove message for gauge %u" ), event.GetInt() );
    guStatusBar * StatusBar = ( guStatusBar * ) GetStatusBar();
    StatusBar->RemoveGauge( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::PlayerSplitterOnIdle( wxIdleEvent& WXUNUSED( event ) )
{
    //
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_PlayerSplitter->SetSashPosition( Config->ReadNum( wxT( "PlayerSashPos" ), 280, wxT( "Positions" ) ) );
    m_PlayerSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guMainFrame::PlayerSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guMainFrame::CreateTaskBarIcon( void )
{
    m_TaskBarIcon = new guTaskBarIcon( this, m_PlayerPanel );
    if( m_TaskBarIcon )
    {
        wxIcon AppIcon;
        AppIcon.CopyFromBitmap( guImage( guIMAGE_INDEX_guayadeque ) );
        m_TaskBarIcon->SetIcon( AppIcon );
    }
}

// -------------------------------------------------------------------------------- //
// guUpdateCoversThread
// -------------------------------------------------------------------------------- //
guUpdateCoversThread::guUpdateCoversThread( DbLibrary * db, int gaugeid ) : wxThread()
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
bool FindCoverLink( DbLibrary * Db, int AlbumId, const wxString &Album, const wxString &Artist, const wxString &Path )
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
        if( ( * m_Tracks )[ index ].m_SongId != guPLAYLIST_RADIOSTATION )
        {
            FileName = FilePattern;
            FileName.Replace( wxT( "{a}" ), ( * m_Tracks )[ index ].m_ArtistName );
            FileName.Replace( wxT( "{b}" ), ( * m_Tracks )[ index ].m_AlbumName );
            FileName.Replace( wxT( "{f}" ), wxFileNameFromPath( ( * m_Tracks )[ index ].m_FileName ) );
            FileName.Replace( wxT( "{g}" ), ( * m_Tracks )[ index ].m_GenreName );
            FileName.Replace( wxT( "{n}" ), wxString::Format( wxT( "%02u" ), ( * m_Tracks )[ index ].m_Number ) );
            FileName.Replace( wxT( "{t}" ), ( * m_Tracks )[ index ].m_SongName );
            FileName.Replace( wxT( "{y}" ), wxString::Format( wxT( "%u" ), ( * m_Tracks )[ index ].m_Year ) );

            if( ( * m_Tracks )[ index ].m_FileName.Lower().EndsWith( wxT( ".mp3" ) ) )
            {
                FileName = FileName + wxT( ".mp3" );
            }

            FileName = m_DestDir + FileName;

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
            //guLogMessage( wxT( "File: '%s' " ), FileName.c_str() );
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
