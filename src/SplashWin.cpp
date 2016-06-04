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

	// Connect Events
//    Connect( wxEVT_PAINT, wxPaintEventHandler( guSplashFrame::OnPaint ), NULL, this );
    Connect( wxEVT_ERASE_BACKGROUND,  wxEraseEventHandler( guSplashFrame::OnEraseBackground ), NULL, this );
	Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guSplashFrame::OnSplashClick ), NULL, this );
	Connect( wxEVT_TIMER, wxTimerEventHandler( guSplashFrame::OnTimeout ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guSplashFrame::~guSplashFrame()
{
    if( m_Bitmap )
    {
        delete m_Bitmap;
    }

	// Disconnect Events
//    Disconnect( wxEVT_PAINT, wxPaintEventHandler( guSplashFrame::OnPaint ), NULL, this );
    Disconnect( wxEVT_ERASE_BACKGROUND,  wxEraseEventHandler( guSplashFrame::OnEraseBackground ), NULL, this );
	Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guSplashFrame::OnSplashClick ), NULL, this );
	Disconnect( wxEVT_TIMER, wxTimerEventHandler( guSplashFrame::OnTimeout ), NULL, this );
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
void guSplashFrame::DoPaint( wxDC &dc )
{
//    wxString Version = wxT( ID_GUAYADEQUE_VERSION );
//#ifdef ID_GUAYADEQUE_REVISION
//    Version += wxT( "-" ID_GUAYADEQUE_REVISION );
//#endif
//    wxString Credits = wxT( "J.Rios anonbeat@gmail.com" );
//
//    wxFont Font( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
//
//    dc.SetBackgroundMode( wxTRANSPARENT );
//    dc.SetTextForeground( wxColour( 60, 60, 60 ) );
//    dc.SetFont( Font );
//
//    wxCoord width, height;
//
//    dc.GetTextExtent( Version,  &width, &height, 0, 0, &Font );
//    dc.DrawText( Version, 493 - width, 270 - height );
//
//    dc.GetTextExtent( Credits,  &width, &height, 0, 0, &Font );
//    dc.DrawText( Credits, 493 - width, 270 - ( height * 2 ) );
}

// -------------------------------------------------------------------------------- //
void guSplashFrame::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    DoPaint( dc );
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
