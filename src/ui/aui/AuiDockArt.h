// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#ifndef __AUIDOCKART_H__
#define __AUIDOCKART_H__

#include <wx/aui/aui.h>
#include <wx/aui/dockart.h>
#include <wx/dc.h>
#include <wx/window.h>
#include <wx/gdicmn.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guAuiDockArt : public wxAuiDefaultDockArt
{
  protected :
    wxBitmap        m_CloseNormal;
    wxBitmap        m_CloseHighLight;

    void            DrawCaptionBackground( wxDC &dc, const wxRect &rect, bool active );

  public :
    guAuiDockArt();

    virtual void    DrawCaption( wxDC &dc, wxWindow * window, const wxString &text, const wxRect &rect, wxAuiPaneInfo &pane );
    virtual void    DrawPaneButton( wxDC &dc, wxWindow * window, int button, int button_state,
                        const wxRect &rect, wxAuiPaneInfo &pane );

};

}

#endif
// -------------------------------------------------------------------------------- //
