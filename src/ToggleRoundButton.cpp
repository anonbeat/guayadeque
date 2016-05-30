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
#include "ToggleRoundButton.h"

#include "Images.h"
#include "Utils.h"

#include <wx/dcclient.h>
#include <wx/tglbtn.h>

BEGIN_EVENT_TABLE( guToggleRoundButton, wxControl )
    EVT_PAINT( guToggleRoundButton::OnPaint)
    //EVT_ERASE_BACKGROUND( guToggleRoundButton::OnEraseBack
    EVT_MOUSE_EVENTS( guToggleRoundButton::OnMouseEvents )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guToggleRoundButton::guToggleRoundButton( wxWindow * parent, const wxImage &image,
        const wxImage &selimage, const wxImage &hoverimage ) :
    wxControl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE )
{
    m_Bitmap = wxBitmap( selimage );
    m_DisBitmap = wxBitmap( image );
    m_HoverBitmap = wxBitmap( hoverimage );
    m_MouseIsOver = false;
    m_IsClicked = false;
    m_Value = false;

    CreateRegion();
}

// -------------------------------------------------------------------------------- //
guToggleRoundButton::~guToggleRoundButton()
{
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::CreateRegion( void )
{
    int Width = m_Bitmap.GetWidth();
    int Height = m_Bitmap.GetHeight();
    wxBitmap RegBmp( Width, Height );
    wxMemoryDC dc;
    dc.SelectObject( RegBmp );
    dc.SetBackground( *wxWHITE_BRUSH );
    dc.Clear();
    dc.SetBrush( *wxBLACK_BRUSH );
    dc.SetPen( *wxBLACK_PEN );
    dc.DrawCircle( Width / 2, Height / 2, wxMin( ( int ) ( Width / 2 ), ( int ) ( Height / 2 ) ) );
    dc.SelectObject( wxNullBitmap );

    m_Region = wxRegion( RegBmp, *wxWHITE );
}

// -------------------------------------------------------------------------------- //
wxSize guToggleRoundButton::DoGetBestSize( void ) const
{
    wxSize RetVal;
    RetVal.x = m_Bitmap.GetWidth();
    RetVal.y = m_Bitmap.GetHeight();
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.DrawBitmap( m_MouseIsOver ? m_HoverBitmap :
        ( m_Value ? m_Bitmap : m_DisBitmap ),
        0 + m_IsClicked, 0 + m_IsClicked, true );
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::OnMouseEvents( wxMouseEvent &event )
{
    bool NeedPaint = false;
    if( m_Region.Contains( event.GetPosition() ) )
    {
        if( !m_MouseIsOver )
        {
            m_MouseIsOver = true;
            //Refresh();
            NeedPaint = true;
        }

        //guLogMessage( wxT( "Event %i %i %i" ), event.LeftDown(), event.LeftIsDown(), event.LeftUp() );
        if( m_IsClicked != event.LeftIsDown() )
        {
            m_IsClicked = event.LeftIsDown();
            //Refresh();
            NeedPaint = true;
        }
        if( event.LeftUp() )
        {
            m_Value = !m_Value;
            NeedPaint = true;
            // Send Clicked event
            wxCommandEvent ClickEvent( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, GetId() );
            ClickEvent.SetEventObject( this );
            ClickEvent.SetInt( m_Value );
            AddPendingEvent( ClickEvent );
        }
    }
    else
    {
        if( m_MouseIsOver )
        {
            m_MouseIsOver = false;
            //Refresh();
            NeedPaint = true;
        }
        if( m_IsClicked )
        {
            m_IsClicked = false;
            //Refresh();
            NeedPaint = true;
        }
    }
    if( NeedPaint )
        Refresh();
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::SetBitmapLabel( const wxImage &image )
{
    m_Bitmap = wxBitmap( image );
    Refresh();
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::SetBitmapHover( const wxImage &image )
{
    m_HoverBitmap = wxBitmap( image );
    Refresh();
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::SetBitmapDisabled( const wxImage &image )
{
    m_DisBitmap = wxBitmap( image );
    Refresh();
}

// -------------------------------------------------------------------------------- //
