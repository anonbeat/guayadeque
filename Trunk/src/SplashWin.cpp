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
#include "SplashWin.h"

#include "Commands.h"
#include "Images.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guSplashFrame::guSplashFrame( wxWindow * parent, int timeout ) :
    wxFrame( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 500,275 ), wxSTAY_ON_TOP | wxNO_BORDER | wxFRAME_TOOL_WINDOW )
{
    CentreOnScreen();

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

//	m_SplashBitmap = new wxStaticBitmap( this, wxID_ANY, wxBitmap( guImage_splash ), wxDefaultPosition, wxDefaultSize, 0 );
//	MainSizer->Add( m_SplashBitmap, 1, wxEXPAND, 0 );

    m_Bitmap = new wxBitmap( guImage( guIMAGE_INDEX_splash ) );

	this->SetSizer( MainSizer );
	this->Layout();

    Show( true );
    SetThemeEnabled( false );
    SetBackgroundStyle( wxBG_STYLE_CUSTOM );

    m_Timer.SetOwner( this );
    m_Timer.Start( timeout, wxTIMER_ONE_SHOT );

	// Connect Events
    Connect( wxEVT_PAINT, wxPaintEventHandler( guSplashFrame::OnPaint ), NULL, this );
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
    Disconnect( wxEVT_PAINT, wxPaintEventHandler( guSplashFrame::OnPaint ), NULL, this );
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
    wxString Version = wxT( ID_GUAYADEQUE_VERSION );
#ifdef ID_GUAYADEQUE_REVISION
    Version += wxT( "-" ID_GUAYADEQUE_REVISION );
#endif
    wxString Credits = wxT( "J.Rios anonbeat@gmail.com" );

    wxFont Font( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.SetTextForeground( wxColor( 60, 60, 60 ) );
    dc.SetFont( Font );

    wxCoord width, height;

    dc.GetTextExtent( Version,  &width, &height, 0, 0, &Font );
    dc.DrawText( Version, 493 - width, 270 - height );

    dc.GetTextExtent( Credits,  &width, &height, 0, 0, &Font );
    dc.DrawText( Credits, 493 - width, 270 - ( height * 2 ) );
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

// -------------------------------------------------------------------------------- //
