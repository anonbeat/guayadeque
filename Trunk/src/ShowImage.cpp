// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "ShowImage.h"

#include "Utils.h"

// -------------------------------------------------------------------------------- //
guShowImage::guShowImage( wxWindow * parent, wxImage * image, const wxPoint &pos ) :
    wxFrame( parent, wxID_ANY, wxEmptyString, pos, wxSize( image->GetWidth(), image->GetHeight() ), wxFRAME_NO_TASKBAR | wxTAB_TRAVERSAL )
{
    m_CapturedMouse = false;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition,
        wxSize( image->GetWidth(), image->GetHeight() ), 0 );
	MainSizer->Add( m_Bitmap, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    if( image )
    {
        m_Bitmap->SetBitmap( wxBitmap( image->Copy() ) );
        delete image;
    }

	Connect( wxEVT_ACTIVATE, wxActivateEventHandler( guShowImage::FrameActivate ) );
	Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guShowImage::OnClick ), NULL, this );
	Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( guShowImage::OnClick ), NULL, this );
	Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guShowImage::OnClick ), NULL, this );

	m_Bitmap->Connect( wxEVT_MOTION, wxMouseEventHandler( guShowImage::OnMouse ), NULL, this );
	Connect( wxEVT_MOTION, wxMouseEventHandler( guShowImage::OnMouse ), NULL, this );
	Connect( wxEVT_MOUSE_CAPTURE_LOST, wxMouseCaptureLostEventHandler( guShowImage::OnCaptureLost ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guShowImage::~guShowImage()
{
    if( m_CapturedMouse )
        ReleaseMouse();
}

// -------------------------------------------------------------------------------- //
void guShowImage::OnClick( wxMouseEvent &event )
{
    Close();
}

// -------------------------------------------------------------------------------- //
void guShowImage::OnCaptureLost( wxMouseCaptureLostEvent &event )
{
    Close();
}

// -------------------------------------------------------------------------------- //
void guShowImage::FrameActivate( wxActivateEvent &event )
{
    if( !event.GetActive() )
      Close();
}

// -------------------------------------------------------------------------------- //
void guShowImage::OnMouse( wxMouseEvent &event )
{
    int MouseX, MouseY;
    wxGetMousePosition( &MouseX, &MouseY );

    wxRect WinRect = m_Bitmap->GetScreenRect();
    //guLogMessage( wxT( "Mouse: %i %i   %i %i %i %i" ), MouseX, MouseY, WinRect.x, WinRect.y, WinRect.width, WinRect.height );
    if( !WinRect.Contains( MouseX, MouseY ) )
    {
        Close();
    }
    else
    {
        if( !m_CapturedMouse )
        {
            m_CapturedMouse = true;
            CaptureMouse();
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
