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
    Connect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnPlay ) );
    Connect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnStop ) );
    Connect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnNextTrack ) );
    Connect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnPrevTrack ) );
    Connect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnQuit ) );
    Connect( wxEVT_TASKBAR_LEFT_DOWN, wxTaskBarIconEventHandler( guTaskBarIcon::OnClick ), NULL, this );
}

// ---------------------------------------------------------------------- //
guTaskBarIcon::~guTaskBarIcon()
{
    Disconnect( ID_PLAYERPANEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnPlay ) );
    Disconnect( ID_PLAYERPANEL_STOP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnStop ) );
    Disconnect( ID_PLAYERPANEL_NEXTTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnNextTrack ) );
    Disconnect( ID_PLAYERPANEL_PREVTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnPrevTrack ) );
    Disconnect( ID_MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaskBarIcon::OnQuit ) );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::OnPlay( wxCommandEvent &event )
{
    if( m_PlayerPanel )
      m_PlayerPanel->OnPlayButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::OnStop( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnStopButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::OnNextTrack( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnNextTrackButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::OnPrevTrack( wxCommandEvent &event )
{
    if( m_PlayerPanel )
        m_PlayerPanel->OnPrevTrackButtonClick( event );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::OnQuit( wxCommandEvent &event )
{
    if( m_MainFrame )
        m_MainFrame->OnQuit( event );
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
    wxMenu * RetVal = new wxMenu;
    wxMenuItem * MenuItem;

    if( m_PlayerPanel )
    {
        bool IsPaused = ( m_PlayerPanel->GetState() == wxMEDIASTATE_PLAYING );
        MenuItem = new wxMenuItem( RetVal, ID_PLAYERPANEL_PLAY, IsPaused ? _( "Pause" ) : _( "Play" ), _( "Play current playlist" ) );
        MenuItem->SetBitmap( guImage( IsPaused ? guIMAGE_INDEX_playback_pause :
                                                 guIMAGE_INDEX_playback_start ) );
        RetVal->Append( MenuItem );

        MenuItem = new wxMenuItem( RetVal, ID_PLAYERPANEL_STOP, _( "Stop" ), _( "Play current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_stop ) );
        RetVal->Append( MenuItem );

        MenuItem = new wxMenuItem( RetVal, ID_PLAYERPANEL_NEXTTRACK, _( "Next Track" ), _( "Skip to next track in current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_skip_forward ) );
        RetVal->Append( MenuItem );

        MenuItem = new wxMenuItem( RetVal, ID_PLAYERPANEL_PREVTRACK, _( "Prev Track" ), _( "Skip to previous track in current playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_skip_forward ) );
        RetVal->Append( MenuItem );

        RetVal->AppendSeparator();
    }

    MenuItem = new wxMenuItem( RetVal, ID_MENU_QUIT, _( "Exit" ), _( "Exit this program" ) );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_stop ) );
    RetVal->Append( MenuItem );

    return RetVal;
}

// ---------------------------------------------------------------------- //
