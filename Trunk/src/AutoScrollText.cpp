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
#include "AutoScrollText.h"

#include "Utils.h"

#include <wx/dcclient.h>
#include <wx/region.h>

#define guSCROLL_START_TIMEOUT  400
#define guSCROLL_TIMEOUT        250

BEGIN_EVENT_TABLE( guAutoScrollText, wxControl )
    EVT_PAINT( guAutoScrollText::OnPaint)
    EVT_MOUSE_EVENTS( guAutoScrollText::OnMouseEvents )
    EVT_SIZE( guAutoScrollText::OnSize )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guAutoScrollText::guAutoScrollText( wxWindow * parent, const wxString &label ) :
    wxControl( parent, wxID_ANY )
{
    m_ScrollPos = 0;
    m_ScrollQuantum = 1;
    m_VisWidth = 0;
    m_AllowScroll = false;
    m_StartTimer.SetOwner( this );
    m_ScrollTimer.SetOwner( this );
    //SetBackgroundColour( * wxCYAN );

    SetLabel( label );

    Connect( wxEVT_TIMER, wxTimerEventHandler( guAutoScrollText::OnTimer ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guAutoScrollText::~guAutoScrollText()
{
    Disconnect( wxEVT_TIMER, wxTimerEventHandler( guAutoScrollText::OnTimer ), NULL, this );
}

// -------------------------------------------------------------------------------- //
wxSize guAutoScrollText::DoGetBestSize() const
{
    return m_LabelExtent;
}

// -------------------------------------------------------------------------------- //
void guAutoScrollText::SetLabel( const wxString &label )
{
    m_Label = label;
    GetTextExtent( label, &m_LabelExtent.x, &m_LabelExtent.y );
    m_ScrollPos = 0;
    m_ScrollQuantum = 1;
    if( m_ScrollTimer.IsRunning() && !( m_LabelExtent.x > m_VisWidth ) )
    {
        m_ScrollTimer.Stop();
    }
    Refresh();
}

// -------------------------------------------------------------------------------- //
void guAutoScrollText::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.SetTextForeground( GetForegroundColour() );
    dc.SetFont( GetFont() );
    dc.DrawText( m_Label.Mid( m_ScrollPos ), 2, 2 );
}

// -------------------------------------------------------------------------------- //
void guAutoScrollText::OnSize( wxSizeEvent &event )
{
    wxRect Size = event.GetSize();
    //guLogMessage( wxT( "Size Event: %i, %i" ), Size.GetWidth(), Size.GetHeight() );
    m_VisWidth = Size.GetWidth();
    event.Skip();
}
// -------------------------------------------------------------------------------- //
void guAutoScrollText::OnMouseEvents( wxMouseEvent &event )
{
    if( event.Moving() && ( m_LabelExtent.x > m_VisWidth ) )
    {
        if( !m_ScrollTimer.IsRunning() )
        {
            if( m_StartTimer.IsRunning() )
                m_StartTimer.Stop();
            m_StartTimer.Start( guSCROLL_START_TIMEOUT, wxTIMER_ONE_SHOT );
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
bool inline MouseIsOver( const wxRect &rect )
{
    int MouseX, MouseY;
    wxGetMousePosition( &MouseX, &MouseY );
    return rect.Contains( MouseX, MouseY );
}

// -------------------------------------------------------------------------------- //
void guAutoScrollText::OnTimer( wxTimerEvent &event )
{
    if( ( wxTimer * ) event.GetEventObject() == &m_StartTimer )
    {
        if( MouseIsOver( GetScreenRect() ) )
        {
            m_ScrollTimer.Start( guSCROLL_TIMEOUT, wxTIMER_CONTINUOUS );
        }
    }
    else
    {
        if( !MouseIsOver( GetScreenRect() ) )
        {
            if( m_ScrollQuantum > 0 )
                m_ScrollQuantum = -1;
            if( !m_ScrollPos )
            {
                m_ScrollTimer.Stop();
                return;
            }

        }
        else if( m_ScrollQuantum > 0 )
        {
            int ActualWidth;
            GetTextExtent( m_Label.Mid( m_ScrollPos ), &ActualWidth, NULL );
            if( ( ActualWidth + 16 ) < m_VisWidth )
            {
                m_ScrollQuantum = -1;
            }
        }
        else
        {
            if( !m_ScrollPos )
            {
                m_ScrollQuantum = 1;
            }
        }
        m_ScrollPos += m_ScrollQuantum;
        //guLogMessage( wxT( "Actual ScrollPos: %i  ScrollQ: %i" ), m_ScrollPos, m_ScrollQuantum );
        Refresh();
    }
}

// -------------------------------------------------------------------------------- //
