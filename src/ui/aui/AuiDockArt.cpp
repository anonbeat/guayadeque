// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "AuiDockArt.h"

#include "Images.h"
#include "Utils.h"

namespace Guayadeque {

wxString wxAuiChopText(wxDC& dc, const wxString& text, int max_size)
{
    wxCoord x,y;

    // first check if the text fits with no problems
    dc.GetTextExtent(text, &x, &y);
    if (x <= max_size)
        return text;

    size_t i, len = text.Length();
    size_t last_good_length = 0;
    for (i = 0; i < len; ++i)
    {
        wxString s = text.Left(i);
        s += wxT("...");

        dc.GetTextExtent(s, &x, &y);
        if (x > max_size)
            break;

        last_good_length = i;
    }

    wxString ret = text.Left(last_good_length);
    ret += wxT("...");
    return ret;
}


// -------------------------------------------------------------------------------- //
guAuiDockArt::guAuiDockArt() : wxAuiDefaultDockArt()
{
    m_CloseNormal = guImage( guIMAGE_INDEX_tiny_close_normal );
    m_CloseHighLight = guImage( guIMAGE_INDEX_tiny_close_highlight );
}

// -------------------------------------------------------------------------------- //
void inline DrawGradientRectangle( wxDC &dc, const wxRect &rect, const wxColour &start_color, const wxColour &end_color, int direction )
{
    dc.GradientFillLinear( rect, start_color, end_color, direction == wxAUI_GRADIENT_VERTICAL ? wxNORTH : wxWEST );
}


// -------------------------------------------------------------------------------- //
void guAuiDockArt::DrawCaptionBackground( wxDC &dc, const wxRect &rect, bool active )
{
    if( m_gradientType == wxAUI_GRADIENT_NONE )
    {
        if( active )
            dc.SetBrush( wxBrush( m_activeCaptionColour ) );
        else
            dc.SetBrush( wxBrush( m_inactiveCaptionColour ) );

        dc.DrawRectangle( rect.x, rect.y, rect.width, rect.height );
    }
    else
    {
        if( active )
        {
            // on mac the gradients are expected to become darker from the top
#ifdef __WXMAC__
            DrawGradientRectangle( dc, rect,
                                 m_activeCaptionColour,
                                 m_activeCaptionGradientColour,
                                 m_gradientType );
#else
            // on other platforms, active gradients become lighter at the top
            DrawGradientRectangle( dc, rect,
                                 m_activeCaptionColour,
                                 m_activeCaptionGradientColour,
                                 m_gradientType );
#endif
        }
        else
        {
#ifdef __WXMAC__
            // on mac the gradients are expected to become darker from the top
            DrawGradientRectangle( dc, rect,
                                 m_inactiveCaptionColour,
                                 m_inactiveCaptionGradientColour,
                                 m_gradientType );
#else
            // on other platforms, inactive gradients become lighter at the bottom
            DrawGradientRectangle( dc, rect,
                                 m_inactiveCaptionColour,
                                 m_inactiveCaptionGradientColour,
                                 m_gradientType );
#endif
        }
    }

    dc.SetPen( m_borderPen );
    int y = rect.y + rect.height - 1;
    dc.DrawLine( rect.x, y, rect.x + rect.width, y );
}


// -------------------------------------------------------------------------------- //
void guAuiDockArt::DrawCaption( wxDC &dc, wxWindow * window, const wxString &text, const wxRect &rect, wxAuiPaneInfo &pane )
{
    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.SetFont( m_captionFont );

    DrawCaptionBackground( dc, rect, bool( pane.state & wxAuiPaneInfo::optionActive ) );

    dc.SetTextForeground( pane.state & wxAuiPaneInfo::optionActive ? m_activeCaptionTextColour : m_inactiveCaptionTextColour );

    wxCoord w, h;
    dc.GetTextExtent( wxT( "ABCDEFHXfgkj" ), &w, &h );

    wxRect clip_rect = rect;
    clip_rect.width -= 3; // text offset
    clip_rect.width -= 2; // button padding
    if( pane.HasCloseButton() )
        clip_rect.width -= m_buttonSize;
    if( pane.HasPinButton() )
        clip_rect.width -= m_buttonSize;
    if( pane.HasMaximizeButton() )
        clip_rect.width -= m_buttonSize;

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
        dc.DrawBitmap( ( ( button_state & ( wxAUI_BUTTON_STATE_HOVER | wxAUI_BUTTON_STATE_PRESSED ) ) ? m_CloseHighLight : m_CloseNormal ),
            rect.x + Offset, rect.y + 2 + Offset, true );
        return;
    }
    wxAuiDefaultDockArt::DrawPaneButton( dc, window, button, button_state, rect, pane );
}

}

// -------------------------------------------------------------------------------- //
