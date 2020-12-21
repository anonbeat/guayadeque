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
#ifndef __AUINOTEBOOK_H__
#define __AUINOTEBOOK_H__

#include "Utils.h"

#include <wx/control.h>
#include <wx/aui/auibook.h>
#include <wx/aui/tabmdi.h>
#include <wx/aui/dockart.h>

namespace Guayadeque {

#undef wxUSE_MDI

// CODE FROM auibook.cpp
extern wxColour wxAuiStepColour( const wxColour &c, int percent );

//wxBitmap wxAuiBitmapFromBits(const unsigned char bits[], int w, int h,
//                             const wxColour& color);
extern wxString wxAuiChopText( wxDC &dc, const wxString &text, int max_size );

// wxTabFrame is an interesting case.  It's important that all child pages
// of the multi-notebook control are all actually children of that control
// (and not grandchildren).  wxTabFrame facilitates this.  There is one
// instance of wxTabFrame for each tab control inside the multi-notebook.
// It's important to know that wxTabFrame is not a real window, but it merely
// used to capture the dimensions/positioning of the internal tab control and
// it's managed page windows

// -------------------------------------------------------------------------------- //
class wxTabFrame : public wxWindow
{
  public:

    wxTabFrame()
    {
        m_tabs = NULL;
        m_rect = wxRect(0,0,200,200);
        m_tab_ctrl_height = 16;
    }

    ~wxTabFrame()
    {
        wxDELETE(m_tabs);
    }

    void SetTabCtrlHeight(int h)
    {
        m_tab_ctrl_height = h;
    }

    void DoSetSize(int x, int y,
                   int width, int height,
                   int WXUNUSED(sizeFlags = wxSIZE_AUTO))
    {
        m_rect = wxRect(x, y, width, height);
        DoSizing();
    }

    void DoGetClientSize(int* x, int* y) const
    {
        *x = m_rect.width;
        *y = m_rect.height;
    }

    bool Show( bool WXUNUSED(show = true) ) { return false; }

    void DoSizing()
    {
        if (!m_tabs)
            return;

        if (m_tabs->GetFlags() & wxAUI_NB_BOTTOM)
        {
            m_tab_rect = wxRect (m_rect.x, m_rect.y + m_rect.height - m_tab_ctrl_height, m_rect.width, m_tab_ctrl_height);
            m_tabs->SetSize     (m_rect.x, m_rect.y + m_rect.height - m_tab_ctrl_height, m_rect.width, m_tab_ctrl_height);
            m_tabs->SetRect     (wxRect(0, 0, m_rect.width, m_tab_ctrl_height));
        }
        else //TODO: if (GetFlags() & wxAUI_NB_TOP)
        {
            m_tab_rect = wxRect (m_rect.x, m_rect.y, m_rect.width, m_tab_ctrl_height);
            m_tabs->SetSize     (m_rect.x, m_rect.y, m_rect.width, m_tab_ctrl_height);
            m_tabs->SetRect     (wxRect(0, 0,        m_rect.width, m_tab_ctrl_height));
        }
        // TODO: else if (GetFlags() & wxAUI_NB_LEFT){}
        // TODO: else if (GetFlags() & wxAUI_NB_RIGHT){}

        m_tabs->Refresh();
        m_tabs->Update();

        wxAuiNotebookPageArray& pages = m_tabs->GetPages();
        size_t i, page_count = pages.GetCount();

        for (i = 0; i < page_count; ++i)
        {
            wxAuiNotebookPage& page = pages.Item(i);
            if (m_tabs->GetFlags() & wxAUI_NB_BOTTOM)
            {
               page.window->SetSize(m_rect.x, m_rect.y,
                                    m_rect.width, m_rect.height - m_tab_ctrl_height);
            }
            else //TODO: if (GetFlags() & wxAUI_NB_TOP)
            {
                page.window->SetSize(m_rect.x, m_rect.y + m_tab_ctrl_height,
                                    m_rect.width, m_rect.height - m_tab_ctrl_height);
            }
            // TODO: else if (GetFlags() & wxAUI_NB_LEFT){}
            // TODO: else if (GetFlags() & wxAUI_NB_RIGHT){}

#if wxUSE_MDI
            if (page.window->IsKindOf(CLASSINFO(wxAuiMDIChildFrame)))
            {
                wxAuiMDIChildFrame* wnd = (wxAuiMDIChildFrame*)page.window;
                wnd->ApplyMDIChildFrameRect();
            }
#endif
        }
    }

    void DoGetSize(int* x, int* y) const
    {
        if (x)
            *x = m_rect.GetWidth();
        if (y)
            *y = m_rect.GetHeight();
    }

    void Update()
    {
        // does nothing
    }

public:

    wxRect m_rect;
    wxRect m_tab_rect;
    wxAuiTabCtrl* m_tabs;
    int m_tab_ctrl_height;
};


// -------------------------------------------------------------------------------- //
class guAuiTabArt : public wxAuiDefaultTabArt
{
  protected :
    wxColour    m_BgColor;
    wxColour    m_SelBgColor;
    wxColour    m_TextFgColor;
    wxColour    m_SelTextFgColour;
  public:
    guAuiTabArt();

    virtual ~guAuiTabArt();

    wxAuiTabArt * Clone();
    virtual void DrawTab( wxDC &dc,
                         wxWindow * wnd,
                         const wxAuiNotebookPage &pane,
                         const wxRect &in_rect,
                         int close_button_state,
                         wxRect * out_tab_rect,
                         wxRect * out_button_rect,
                         int * x_extent );
    virtual void DrawBackground( wxDC &dc, wxWindow * wnd, const wxRect & rect );

};

WX_DEFINE_ARRAY_PTR( wxWindow *, guWindowArray );

// -------------------------------------------------------------------------------- //
class guAuiNotebook : public wxAuiNotebook
{
  protected :
    wxArrayString   m_PageIds;
    guWindowArray   m_PagePtrs;

    wxString        GetPageId( const wxWindow * page );

  public:
    guAuiNotebook();

    guAuiNotebook(wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxAUI_NB_DEFAULT_STYLE);

    virtual ~guAuiNotebook();

    virtual bool UpdateTabCtrlHeight();

    wxString SavePerspective( void );
 	bool LoadPerspective( const wxString &layout );

    wxAuiPaneInfo & GetPane( wxWindow * window );
    wxAuiPaneInfo & GetPane( const wxString &name );
    wxAuiPaneInfoArray & GetAllPanes( void );

    void        AddId( wxWindow * ptr, const wxString &id );
};

}

#endif
// -------------------------------------------------------------------------------- //
