// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "SplashWin.h"

#include "EventCommandIds.h"
#include "Images.h"
#include "Settings.h"
#include "Utils.h"
#include "Version.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guSplashFrame::guSplashFrame( wxWindow * parent, int timeout ) :
    wxFrame( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 500,275 ), wxSTAY_ON_TOP | wxNO_BORDER | wxFRAME_TOOL_WINDOW )
{
    CentreOnScreen();

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    m_Bitmap = new wxBitmap( guImage( guIMAGE_INDEX_splash ) );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

    wxColour FontColor = wxColour( 100, 100, 100 );
    wxString Version = wxT( ID_GUAYADEQUE_VERSION );
#ifdef ID_GUAYADEQUE_REVISION
    Version += wxT( "-" ID_GUAYADEQUE_REVISION );
#endif
	m_Version = new wxStaticText( this, wxID_ANY, Version, wxDefaultPosition, wxDefaultSize, 0 );
    wxFont CurFont = m_Version->GetFont();
    CurFont.SetPointSize( CurFont.GetPointSize() + 2 );
    m_Version->SetFont( CurFont );
	m_Version->Wrap( -1 );
	m_Version->SetForegroundColour( FontColor );
	m_Version->SetBackgroundColour( * wxWHITE );
    MainSizer->Add( m_Version, 0, wxALIGN_RIGHT|wxTOP|wxRIGHT|wxLEFT, 5 );

    MainSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    m_Email = new wxHyperlinkCtrl( this, wxID_ANY, guSPLASH_NAME wxT( " " ) guSPLASH_EMAIL, wxT( "mailto:" ) guSPLASH_EMAIL, wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_RIGHT );
	m_Email->SetHoverColour( FontColor );
	m_Email->SetNormalColour( FontColor );
	m_Email->SetVisitedColour( FontColor );
	m_Email->SetBackgroundColour( * wxWHITE );
    m_Email->SetCanFocus( false );
    MainSizer->Add( m_Email, 0, wxALIGN_RIGHT|wxRIGHT|wxLEFT, 5 );

    m_HomePage = new wxHyperlinkCtrl( this, wxID_ANY, guSPLASH_HOMEPAGE, guSPLASH_HOMEPAGE, wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_RIGHT );
	m_HomePage->SetHoverColour( FontColor );
	m_HomePage->SetNormalColour( FontColor );
	m_HomePage->SetVisitedColour( FontColor );
	m_HomePage->SetBackgroundColour( * wxWHITE );
    m_HomePage->SetCanFocus( false );
    MainSizer->Add( m_HomePage, 0, wxALIGN_RIGHT|wxRIGHT|wxLEFT, 5 );

    m_Donate = new wxHyperlinkCtrl( this, wxID_ANY, _( "Please Donate!" ), guSPLASH_DONATION_LINK );

    m_Donate->SetHoverColour( FontColor );
    m_Donate->SetNormalColour( FontColor );
    m_Donate->SetVisitedColour( FontColor );
    m_Donate->SetBackgroundColour( * wxWHITE );
    m_Donate->SetCanFocus( false );
    MainSizer->Add( m_Donate, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    Show( true );
    SetThemeEnabled( false );
    SetBackgroundStyle( wxBG_STYLE_ERASE );

    m_Timer.SetOwner( this );
    m_Timer.Start( timeout, wxTIMER_ONE_SHOT );

	// Bind Events
    Bind( wxEVT_ERASE_BACKGROUND,  &guSplashFrame::OnEraseBackground, this );
	Bind( wxEVT_LEFT_DOWN, &guSplashFrame::OnSplashClick, this );
	Bind( wxEVT_TIMER, &guSplashFrame::OnTimeout, this );
}

// -------------------------------------------------------------------------------- //
guSplashFrame::~guSplashFrame()
{
    if( m_Bitmap )
    {
        delete m_Bitmap;
    }

    // Unbind Events
    Unbind( wxEVT_ERASE_BACKGROUND,  &guSplashFrame::OnEraseBackground, this );
    Unbind( wxEVT_LEFT_DOWN, &guSplashFrame::OnSplashClick, this );
    Unbind( wxEVT_TIMER, &guSplashFrame::OnTimeout, this );
}

// -------------------------------------------------------------------------------- //
void guSplashFrame::OnSplashClick( wxMouseEvent &event )
{
    Close();
}

// -------------------------------------------------------------------------------- //
void guSplashFrame::OnCloseWindow( wxCloseEvent &event )
{
    m_Timer.Stop();
    this->Destroy();
}

// -------------------------------------------------------------------------------- //
void guSplashFrame::OnTimeout( wxTimerEvent &event )
{
    //guLogMessage( wxT( "Splash timed out" ) );
    Close();
}

// -------------------------------------------------------------------------------- //
void guSplashFrame::OnEraseBackground( wxEraseEvent &event )
{
    wxDC * dc = event.GetDC();
    if( dc )
    {
        dc->DrawBitmap( * m_Bitmap, 0, 0, false );
    }
}

}

// -------------------------------------------------------------------------------- //
