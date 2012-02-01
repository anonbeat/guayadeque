// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
#include "AuiDockArt.h"

#include "Images.h"
#include "Utils.h"

extern wxString wxAuiChopText( wxDC &dc, const wxString &text, int max_size );

// -------------------------------------------------------------------------------- //
guAuiDockArt::guAuiDockArt() : wxAuiDefaultDockArt()
{
    m_CloseNormal = guImage( guIMAGE_INDEX_tiny_close_normal );
    m_CloseHighLight = guImage( guIMAGE_INDEX_tiny_close_highlight );
}

// -------------------------------------------------------------------------------- //
void inline DrawGradientRectangle( wxDC &dc, const wxRect &rect, const wxColour &start_color, const wxColour &end_color, int direction )
{
    dc.GradientFillLinear( rect, start_color, end_color, direction == wxAUI_GRADIENT_VERTICAL ? wxSOUTH : wxEAST );
}


// -------------------------------------------------------------------------------- //
void guAuiDockArt::DrawCaptionBackground( wxDC &dc, const wxRect &rect, bool active )
{
    if( m_gradient_type == wxAUI_GRADIENT_NONE )
    {
        if( active )
            dc.SetBrush( wxBrush( m_active_caption_colour ) );
        else
            dc.SetBrush( wxBrush( m_inactive_caption_colour ) );

        dc.DrawRectangle( rect.x, rect.y, rect.width, rect.height );
    }
    else
    {
        if( active )
        {
            // on mac the gradients are expected to become darker from the top
#ifdef __WXMAC__
            DrawGradientRectangle( dc, rect,
                                 m_active_caption_colour,
                                 m_active_caption_gradient_colour,
                                 m_gradient_type );
#else
            // on other platforms, active gradients become lighter at the top
            DrawGradientRectangle( dc, rect,
                                 m_active_caption_gradient_colour,
                                 m_active_caption_colour,
                                 m_gradient_type );
#endif
        }
        else
        {
#ifdef __WXMAC__
            // on mac the gradients are expected to become darker from the top
            DrawGradientRectangle( dc, rect,
                                 m_inactive_caption_gradient_colour,
                                 m_inactive_caption_colour,
                                 m_gradient_type );
#else
            // on other platforms, inactive gradients become lighter at the bottom
            DrawGradientRectangle( dc, rect,
                                 m_inactive_caption_colour,
                                 m_inactive_caption_gradient_colour,
                                 m_gradient_type );
#endif
        }
    }
}


// -------------------------------------------------------------------------------- //
void guAuiDockArt::DrawCaption( wxDC &dc, wxWindow * window, const wxString &text, const wxRect &rect, wxAuiPaneInfo &pane )
{
    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.SetFont( m_caption_font );

    DrawCaptionBackground( dc, rect, bool( pane.state & wxAuiPaneInfo::optionActive ) );

    dc.SetTextForeground( pane.state & wxAuiPaneInfo::optionActive ? m_active_caption_text_colour : m_inactive_caption_text_colour );

    wxCoord w, h;
    dc.GetTextExtent( wxT( "ABCDEFHXfgkj" ), &w, &h );

    wxRect clip_rect = rect;
    clip_rect.width -= 3; // text offset
    clip_rect.width -= 2; // button padding
    if( pane.HasCloseButton() )
        clip_rect.width -= m_button_size;
    if( pane.HasPinButton() )
        clip_rect.width -= m_button_size;
    if( pane.HasMaximizeButton() )
        clip_rect.width -= m_button_size;

    wxString draw_text = wxAuiChopText( dc, text, clip_rect.width );

    dc.SetClippingRegion( clip_rect );
    dc.DrawText( draw_text, rect.x + 3, rect.y + ( rect.height / 2 ) - ( h / 2 ) - 1 );
    dc.DestroyClippingRegion();
}

// -------------------------------------------------------------------------------- //
void guAuiDockArt::DrawPaneButton( wxDC &dc, wxWindow * window, int button, int button_state,
                  const wxRect &rect, wxAuiPaneInfo &pane )
{
    if( button == wxAUI_BUTTON_CLOSE )
    {
        int Offset = 0;
        if( ( button_state & wxAUI_BUTTON_STATE_PRESSED ) )
            Offset++;
        dc.DrawBitmap( ( ( button_state & wxAUI_BUTTON_STATE_HOVER ) ? m_CloseHighLight : m_CloseNormal ),
            rect.x + Offset, rect.y + 1 + Offset, true );
        return;
    }
    wxAuiDefaultDockArt::DrawPaneButton( dc, window, button, button_state, rect, pane );
}

// -------------------------------------------------------------------------------- //
