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
#include "AuiManagedPanel.h"

#include "AuiDockArt.h"
#include "Commands.h"
#include "Utils.h"

#include <wx/settings.h>

// -------------------------------------------------------------------------------- //
guAuiManagedPanel::guAuiManagedPanel( wxWindow * parent ) :
                        wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_AuiManager.SetManagedWindow( this );
    m_AuiManager.SetArtProvider( new guAuiDockArt() );
    m_AuiManager.SetFlags( wxAUI_MGR_ALLOW_FLOATING |
                           wxAUI_MGR_TRANSPARENT_DRAG |
                           wxAUI_MGR_TRANSPARENT_HINT );

    wxAuiDockArt * AuiDockArt = m_AuiManager.GetArtProvider();

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DSHADOW ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVEBORDER ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DSHADOW ) );

    AuiDockArt->SetMetric( wxAUI_DOCKART_GRADIENT_TYPE,
            wxAUI_GRADIENT_VERTICAL );

    m_AuiManager.Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guAuiManagedPanel::OnPaneClose ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guAuiManagedPanel::~guAuiManagedPanel()
{
    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guAuiManagedPanel::OnPaneClose ), NULL, this );

    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guAuiManagedPanel::ShowPanel( const int panelid, bool show )
{
    int PanelIndex = m_PanelIds.Index( panelid );
    if( PanelIndex != wxNOT_FOUND )
    {
        wxString PaneName = m_PanelNames[ PanelIndex ];

        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( PaneName );
        if( PaneInfo.IsOk() )
        {
            if( show )
                PaneInfo.Show();
            else
                PaneInfo.Hide();

            m_AuiManager.Update();
        }

        if( show )
            m_VisiblePanels |= panelid;
        else
            m_VisiblePanels ^= panelid;

        guLogMessage( wxT( "Id: %i Pane: %s Show:%i  Flags:%08X" ), panelid, PaneName.c_str(), show, m_VisiblePanels );
    }
}

// -------------------------------------------------------------------------------- //
void guAuiManagedPanel::OnPaneClose( wxAuiManagerEvent &event )
{
    wxAuiPaneInfo * PaneInfo = event.GetPane();
    int PanelIndex = m_PanelNames.Index( PaneInfo->name );
    if( PanelIndex != wxNOT_FOUND )
    {
        guLogMessage( wxT( "OnPaneClose: %s  %i" ), m_PanelNames[ PanelIndex ].c_str(), m_PanelCmdIds[ PanelIndex ] );
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, m_PanelCmdIds[ PanelIndex ] );
        AddPendingEvent( evt );

    }

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guAuiManagedPanel::LoadPerspective( const wxString &layoutstr, const unsigned int visiblepanels )
{
    int Index;
    int Count = m_PanelIds.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        int PanelId = m_PanelIds[ Index ];
        if( ( visiblepanels & PanelId ) != ( m_VisiblePanels & PanelId ) )
        {
            ShowPanel( PanelId, ( visiblepanels & PanelId ) );
        }
    }

    m_AuiManager.LoadPerspective( layoutstr, true );
}

// -------------------------------------------------------------------------------- //
