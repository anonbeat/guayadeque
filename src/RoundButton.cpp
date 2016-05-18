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
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "RoundButton.h"

#include "Images.h"
#include "Utils.h"

#include <wx/dcclient.h>

BEGIN_EVENT_TABLE( guRoundButton, wxControl )
    EVT_PAINT( guRoundButton::OnPaint)
    //EVT_ERASE_BACKGROUND( guRoundButton::OnEraseBack
    EVT_MOUSE_EVENTS( guRoundButton::OnMouseEvents )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guRoundButton::guRoundButton( wxWindow * parent, const wxImage &image, const wxImage &selimage, const int flags ) :
    wxControl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE )
{
    m_Bitmap = wxBitmap( image );
    m_HoverBitmap = wxBitmap( selimage );
    m_MouseIsOver = false;
    m_IsClicked = false;

    wxImage Image( image );
    Image.ConvertAlphaToMask();
    m_Region = wxRegion( Image );
}

// -------------------------------------------------------------------------------- //
guRoundButton::~guRoundButton()
{
}

// -------------------------------------------------------------------------------- //
wxSize guRoundButton::DoGetBestSize( void ) const
{
    wxSize RetVal;
    RetVal.x = m_Bitmap.GetWidth();
    RetVal.y = m_Bitmap.GetHeight();
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guRoundButton::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.DrawBitmap( IsEnabled() || !m_DisBitmap.IsOk() ? ( m_MouseIsOver ? m_HoverBitmap : m_Bitmap ) : m_DisBitmap, 0 + m_IsClicked, 0 + m_IsClicked, true );
}

// -------------------------------------------------------------------------------- //
void guRoundButton::OnMouseEvents( wxMouseEvent &event )
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
            // Send Clicked event
            wxCommandEvent ClickEvent( wxEVT_COMMAND_BUTTON_CLICKED, GetId() );
            ClickEvent.SetEventObject( this );
            AddPendingEvent( ClickEvent );
            m_IsClicked = false;
            m_MouseIsOver = false;
            NeedPaint = true;
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
    if( event.Leaving() )
    {
        m_IsClicked = false;
        m_MouseIsOver = false;
        NeedPaint = true;
    }
    if( NeedPaint )
        Refresh();
}

// -------------------------------------------------------------------------------- //
void guRoundButton::SetBitmapLabel( const wxImage &image )
{
    m_Bitmap = wxBitmap( image );
    Refresh();
}

// -------------------------------------------------------------------------------- //
void guRoundButton::SetBitmapHover( const wxImage &image )
{
    m_HoverBitmap = wxBitmap( image );
    Refresh();
}

// -------------------------------------------------------------------------------- //
void guRoundButton::SetBitmapDisabled( const wxImage &image )
{
    m_DisBitmap = wxBitmap( image );
    Refresh();
}

// -------------------------------------------------------------------------------- //
