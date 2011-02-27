// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "AuiDockArt.h"

#include "Images.h"

// -------------------------------------------------------------------------------- //
guAuiDockArt::guAuiDockArt() : wxAuiDefaultDockArt()
{
    m_CloseNormal = guImage( guIMAGE_INDEX_tiny_close_normal );
    m_CloseHighLight = guImage( guIMAGE_INDEX_tiny_close_highlight );
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

