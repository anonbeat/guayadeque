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
#include "TaskBar.h"
#include "Images.h"
#include "Commands.h"
#include "Utils.h"

#include <wx/menu.h>

// ---------------------------------------------------------------------- //
guTaskBarIcon::guTaskBarIcon( guMainFrame * NewMainFrame, guPlayerPanel * NewPlayerPanel ) : wxTaskBarIcon()
{
    m_MainFrame = NewMainFrame;
    m_PlayerPanel = NewPlayerPanel;
    //
    Connect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYERPANEL_NEXTALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYERPANEL_PREVALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYER_PLAYLIST_REPEATPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYER_PLAYLIST_REPEATTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Connect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );

    Connect( wxEVT_TASKBAR_LEFT_DOWN, wxTaskBarIconEventHandler( guTaskBarIcon::OnClick ), NULL, this );
}

// ---------------------------------------------------------------------- //
guTaskBarIcon::~guTaskBarIcon()
{
    Disconnect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYERPANEL_NEXTALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYERPANEL_PREVALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYER_PLAYLIST_SMARTPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYER_PLAYLIST_REPEATPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYER_PLAYLIST_REPEATTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );
    Disconnect( ID_PLAYER_PLAYLIST_RANDOMPLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::SendEventToMainFrame ) );

    Disconnect( wxEVT_TASKBAR_LEFT_DOWN, wxTaskBarIconEventHandler( guTaskBarIcon::OnClick ), NULL, this );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::SendEventToMainFrame( wxCommandEvent &event )
{
    wxPostEvent( m_MainFrame, event );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::OnClick( wxTaskBarIconEvent &event )
{
    if( m_MainFrame )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( !m_MainFrame->IsShown() )
        {
            m_MainFrame->Show( true );
            if( m_MainFrame->IsIconized() )
                m_MainFrame->Iconize( false );
        }
        else if( Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) )
        {
            m_MainFrame->Show( false );
        }
        else
        {
            m_MainFrame->Iconize( !m_MainFrame->IsIconized() );
        }
    }
}

// ---------------------------------------------------------------------- //
wxMenu * guTaskBarIcon::CreatePopupMenu()
{
    wxMenu * Menu = new wxMenu;
    wxMenuItem * MenuItem;

    if( m_PlayerPanel )
    {
        bool IsPaused = ( m_PlayerPanel->GetState() == guMEDIASTATE_PLAYING );
        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PLAY, IsPaused ? _( "Pause" ) : _( "Play" ), _( "Play current playlist" ) );
        MenuItem->SetBitmap( guImage( IsPaused ? guIMAGE_INDEX_player_normal_pause :
                                                 guIMAGE_INDEX_player_normal_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_STOP, _( "Stop" ), _( "Play current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_stop ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_NEXTTRACK, _( "Next Track" ), _( "Skip to next track in current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_next ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_NEXTALBUM, _( "Next Album" ), _( "Skip to next album track in current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_next ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PREVTRACK, _( "Prev Track" ), _( "Skip to previous track in current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_prev ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PREVTRACK, _( "Prev Album" ), _( "Skip to previous album track in current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_prev ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SMARTPLAY, _( "&Smart Mode" ), _( "Update playlist based on Last.fm statics" ), wxITEM_CHECK );
        Menu->Append( MenuItem );
        MenuItem->Check( m_PlayerPanel->GetPlaySmart() );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_REPEATPLAYLIST, _( "&Repeat Playlist" ), _( "Repeat the tracks in the playlist" ), wxITEM_CHECK );
        Menu->Append( MenuItem );
        MenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_PLAYLIST );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_REPEATTRACK, _( "&Repeat Track" ), _( "Repeat the current track in the playlist" ), wxITEM_CHECK );
        Menu->Append( MenuItem );
        MenuItem->Check( m_PlayerPanel->GetPlayLoop() == guPLAYER_PLAYLOOP_TRACK );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY, _( "R&andomize" ), _( "Randomize the playlist" ), wxITEM_NORMAL );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( Menu, ID_MENU_QUIT, _( "Exit" ), _( "Exit this program" ) );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_stop ) );
    Menu->Append( MenuItem );

    return Menu;
}

// ---------------------------------------------------------------------- //
