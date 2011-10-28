// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "AuiManagerPanel.h"

#include "AuiDockArt.h"
#include "AuiNotebook.h"
#include "Commands.h"
#include "MainFrame.h"
#include "Utils.h"

#include <wx/settings.h>

// -------------------------------------------------------------------------------- //
guAuiManagerPanel::guAuiManagerPanel( wxWindow * parent ) :
                        wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER )
{
    m_MenuBar = NULL;

    m_AuiManager.SetManagedWindow( this );
    m_AuiManager.SetArtProvider( new guAuiDockArt() );
    m_AuiManager.SetFlags( wxAUI_MGR_ALLOW_FLOATING |
                           wxAUI_MGR_TRANSPARENT_DRAG |
                           wxAUI_MGR_TRANSPARENT_HINT );

    wxAuiDockArt * AuiDockArt = m_AuiManager.GetArtProvider();

    wxColour BaseColor = wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR,
            BaseColor );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR,
            wxAuiStepColour( BaseColor, 140 ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR,
            BaseColor );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR,
            wxAuiStepColour( BaseColor, 140 ) );

    AuiDockArt->SetMetric( wxAUI_DOCKART_CAPTION_SIZE, 17 );
    AuiDockArt->SetMetric( wxAUI_DOCKART_PANE_BORDER_SIZE, 1 );

    AuiDockArt->SetMetric( wxAUI_DOCKART_GRADIENT_TYPE,
            wxAUI_GRADIENT_VERTICAL );

    m_AuiManager.Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guAuiManagerPanel::OnPaneClose ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guAuiManagerPanel::~guAuiManagerPanel()
{
    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guAuiManagerPanel::OnPaneClose ), NULL, this );

    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::ShowPanel( const int panelid, bool show )
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

        if( !m_MenuBar )
        {
            guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
            m_MenuBar = MainFrame->GetMenuBar();
        }
        if( m_MenuBar )
        {
            wxMenuItem * MenuItem = m_MenuBar->FindItem( m_PanelCmdIds[ PanelIndex ] );
            if( MenuItem )
            {
                MenuItem->Check( show );
            }
        }

        guLogMessage( wxT( "Id: %i Pane: %s Show:%i  Flags:%08X" ), panelid, PaneName.c_str(), show, m_VisiblePanels );
    }
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::OnPaneClose( wxAuiManagerEvent &event )
{
    wxAuiPaneInfo * PaneInfo = event.GetPane();
    int PanelIndex = m_PanelNames.Index( PaneInfo->name );
    if( PanelIndex != wxNOT_FOUND )
    {
        guLogMessage( wxT( "OnPaneClose: %s  %i" ), m_PanelNames[ PanelIndex ].c_str(), m_PanelCmdIds[ PanelIndex ] );
        ShowPanel( m_PanelIds[ PanelIndex ], false );
    }

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::LoadPerspective( const wxString &layoutstr, const unsigned int visiblepanels )
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
void guAuiManagerPanel::SaveLayout( wxXmlNode * xmlnode, const wxString &name )
{
    wxXmlNode * XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, name );

    wxXmlProperty * Property = new wxXmlProperty( wxT( "panels" ), wxString::Format( wxT( "%d" ), VisiblePanels() ),
               new wxXmlProperty( wxT( "layout" ), SavePerspective(),
               NULL ) );

    XmlNode->SetProperties( Property );

    wxXmlNode * Columns = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "columns" ) );
    int Index;
    int Count = GetListViewColumnCount();
    for( Index = 0; Index < Count; Index++ )
    {
        int  ColumnPos;
        int  ColumnWidth;
        bool ColumnEnabled;

        wxXmlNode * Column = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "column" ) );

        GetListViewColumnData( Index, &ColumnPos, &ColumnWidth, &ColumnEnabled );

        Property = new wxXmlProperty( wxT( "id" ), wxString::Format( wxT( "%d" ), Index ),
                   new wxXmlProperty( wxT( "pos" ), wxString::Format( wxT( "%d" ), ColumnPos ),
                   new wxXmlProperty( wxT( "width" ), wxString::Format( wxT( "%d" ), ColumnWidth ),
                   new wxXmlProperty( wxT( "enabled" ), wxString::Format( wxT( "%d" ), ColumnEnabled ),
                   NULL ) ) ) );
        Column->SetProperties( Property );

        Columns->AddChild( Column );
    }

    XmlNode->AddChild( Columns );

    xmlnode->AddChild( XmlNode );
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::LoadLayout( wxXmlNode * xmlnode )
{
    wxString Field;
    long VisiblePanels;
    wxString LayoutStr;

    xmlnode->GetPropVal( wxT( "panels" ), &Field );
    Field.ToLong( &VisiblePanels );
    xmlnode->GetPropVal( wxT( "layout" ), &LayoutStr );

    wxXmlNode * Columns = xmlnode->GetChildren();
    if( Columns && ( Columns->GetName() == wxT( "columns" ) ) )
    {
        wxXmlNode * Column = Columns->GetChildren();
        while( Column && ( Column->GetName() == wxT( "column" ) ) )
        {
            long ColumnId;
            long ColumnPos;
            long ColumnWidth;
            long ColumnEnabled;

            Column->GetPropVal( wxT( "id" ), &Field );
            Field.ToLong( &ColumnId );
            Column->GetPropVal( wxT( "pos" ), &Field );
            Field.ToLong( &ColumnPos );
            Column->GetPropVal( wxT( "width" ), &Field );
            Field.ToLong( &ColumnWidth );
            Column->GetPropVal( wxT( "enabled" ), &Field );
            Field.ToLong( &ColumnEnabled );

            SetListViewColumnData( ColumnId, ColumnPos, ColumnWidth, ColumnEnabled );

            Column = Column->GetNext();
        }
    }

    LoadPerspective( LayoutStr, VisiblePanels );
}

// -------------------------------------------------------------------------------- //
