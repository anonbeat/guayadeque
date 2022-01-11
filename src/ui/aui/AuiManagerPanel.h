// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
#ifndef __AUIMANAGERPANEL_H__
#define __AUIMANAGERPANEL_H__

#include <wx/aui/aui.h>
#include <wx/string.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guAuiManagerPanel : public wxPanel
{
  protected :
    wxArrayString       m_PanelNames;
    wxArrayInt          m_PanelIds;
    wxArrayInt          m_PanelCmdIds;
    wxAuiManager        m_AuiManager;
    unsigned int        m_VisiblePanels;
    wxMenuBar *         m_MenuBar;

  public :
    guAuiManagerPanel( wxWindow * parent );
    ~guAuiManagerPanel();

    virtual void        InitPanelData( void ) {}

    bool                IsPanelShown( const int panelid ) const { return ( m_VisiblePanels & panelid ); }
    virtual void        ShowPanel( const int panelid, bool show );
    virtual void        OnPaneClose( wxAuiManagerEvent &event );
    int                 PanelId( const int index ) const { return m_PanelIds[ index ]; }

    int                 VisiblePanels( void ) const { return m_VisiblePanels; }
    virtual wxString    SavePerspective( void ) { return m_AuiManager.SavePerspective(); }
    virtual void        LoadPerspective( const wxString &layoutstr, const unsigned int visiblepanels );

    virtual int         GetListViewColumnCount( void ) { return 0; }
    virtual bool        GetListViewColumnData( const int id, int * index, int * width, bool * enabled ) { return false; }
    virtual bool        SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return false; }

    virtual void        SaveLayout( wxXmlNode * xmlnode, const wxString &name );
    virtual void        LoadLayout( wxXmlNode * xmlnode );

};

}

#endif
// -------------------------------------------------------------------------------- //
