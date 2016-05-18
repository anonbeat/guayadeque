// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
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
#include "StaticBitmap.h"

#include "Utils.h"

const wxEventType guStaticBitmapMouseOverEvent = wxNewEventType();

#define guSTATICBITMAP_MOUSE_OVER_TIMEOUT   500

// -------------------------------------------------------------------------------- //
guStaticBitmap::guStaticBitmap( wxWindow * parent, wxWindowID id, const wxBitmap &label, const wxPoint &pos, const wxSize &size, long style ) :
    wxStaticBitmap( parent, id, label, pos, size, style )
{
    m_MouseOverTimer = new guStaticBitmapTimer( this );

    Connect( wxEVT_MOTION, wxMouseEventHandler( guStaticBitmap::OnMouse ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guStaticBitmap::~guStaticBitmap()
{
    Disconnect( wxEVT_MOTION, wxMouseEventHandler( guStaticBitmap::OnMouse ), NULL, this );

    if( m_MouseOverTimer )
    {
        delete m_MouseOverTimer;
    }
}

// -------------------------------------------------------------------------------- //
void guStaticBitmap::OnMouse( wxMouseEvent &event )
{
    if( m_MouseOverTimer->IsRunning() )
        m_MouseOverTimer->Stop();

    m_MouseOverTimer->Start( guSTATICBITMAP_MOUSE_OVER_TIMEOUT, wxTIMER_ONE_SHOT );

    event.Skip();
}

// -------------------------------------------------------------------------------- //
// guStaticBitmapTimer
// -------------------------------------------------------------------------------- //
void guStaticBitmapTimer::Notify()
{
    int MouseX, MouseY;
    wxGetMousePosition( &MouseX, &MouseY );

    wxRect WinRect = m_Bitmap->GetScreenRect();
    if( WinRect.Contains( MouseX, MouseY ) )
    {
        wxCommandEvent event( guStaticBitmapMouseOverEvent, guEVT_STATICBITMAP_MOUSE_OVER );
        event.SetEventObject( this );
        m_Bitmap->GetEventHandler()->AddPendingEvent( event );
    }
}

// -------------------------------------------------------------------------------- //
