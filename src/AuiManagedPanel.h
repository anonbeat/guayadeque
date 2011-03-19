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
#ifndef AUIMANAGEDPANEL_H
#define AUIMANAGEDPANEL_H

#include <wx/aui/aui.h>
#include <wx/string.h>

// -------------------------------------------------------------------------------- //
class guAuiManagedPanel : public wxPanel
{
  protected :
    wxArrayString       m_PanelNames;
    wxArrayInt          m_PanelIds;
    wxArrayInt          m_PanelCmdIds;
    wxAuiManager        m_AuiManager;
    unsigned int        m_VisiblePanels;

  public :
    guAuiManagedPanel( wxWindow * parent );
    ~guAuiManagedPanel();

    virtual void        InitPanelData( void ) {}

    bool                IsPanelShown( const int panelid ) const { return ( m_VisiblePanels & panelid ); }
    virtual void        ShowPanel( const int panelid, bool show );
    virtual void        OnPaneClose( wxAuiManagerEvent &event );

    int                 VisiblePanels( void ) { return m_VisiblePanels; }
    virtual wxString    SavePerspective( void ) { return m_AuiManager.SavePerspective(); }
    virtual void        LoadPerspective( const wxString &layoutstr, const unsigned int visiblepanels );

    virtual bool        GetListViewColumnData( const int id, int * index, int * width, bool * enabled ) { return false; }
    virtual bool        SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return false; }

};

#endif
// -------------------------------------------------------------------------------- //
