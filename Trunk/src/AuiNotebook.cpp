// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
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
#include "AuiNotebook.h"

// -------------------------------------------------------------------------------- //
guAuiNotebook::guAuiNotebook() : wxAuiNotebook()
{
}

// -------------------------------------------------------------------------------- //
guAuiNotebook::guAuiNotebook(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style) :
    wxAuiNotebook( parent, id, pos, size, style )
{
}

// -------------------------------------------------------------------------------- //
guAuiNotebook::~guAuiNotebook()
{
}

// -------------------------------------------------------------------------------- //
wxString guAuiNotebook::SavePerspective( void )
{
    // Build list of panes/tabs
    wxString tabs;
    wxAuiPaneInfoArray &all_panes = m_mgr.GetAllPanes();
    const size_t pane_count = all_panes.GetCount();

    for( size_t i = 0; i < pane_count; ++i )
    {
        wxAuiPaneInfo &pane = all_panes.Item( i );
        if( pane.name == wxT( "dummy" ) )
            continue;

        wxTabFrame * tabframe = ( wxTabFrame * ) pane.window;

        if( !tabs.empty() )
            tabs += wxT( "|" );
        tabs += pane.name;
        tabs += wxT( "=" );

        // add tab id's
        size_t page_count = tabframe->m_tabs->GetPageCount();
        for( size_t p = 0; p < page_count; ++p )
        {
            wxAuiNotebookPage &page = tabframe->m_tabs->GetPage( p );
            const size_t page_idx = m_tabs.GetIdxFromWindow( page.window );

            if( p )
                tabs += wxT( "," );

            if( ( int ) page_idx == m_curpage )
                tabs += wxT( "*" );
            else if( ( int ) p == tabframe->m_tabs->GetActivePage() )
                tabs += wxT( "+" );
            tabs += wxString::Format( wxT( "%u" ), page_idx );
        }
    }
    tabs += wxT( "@" );

    // Add frame perspective
    tabs += m_mgr.SavePerspective();

    return tabs;
}

// -------------------------------------------------------------------------------- //
bool guAuiNotebook::LoadPerspective( const wxString &layout )
{
    // Remove all tab ctrls (but still keep them in main index)
    const size_t tab_count = m_tabs.GetPageCount();
    for( size_t i = 0; i < tab_count; ++i )
    {
        wxWindow * wnd = m_tabs.GetWindowFromIdx( i );

        // find out which onscreen tab ctrl owns this tab
        wxAuiTabCtrl * ctrl;
        int ctrl_idx;
        if( !FindTab( wnd, &ctrl, &ctrl_idx ) )
            return false;

        // remove the tab from ctrl
        if( !ctrl->RemovePage( wnd ) )
            return false;
    }
    RemoveEmptyTabFrames();

    size_t sel_page = 0;

    wxString tabs = layout.BeforeFirst( wxT( '@' ) );
    while( 1 )
    {
        const wxString tab_part = tabs.BeforeFirst( wxT( '|' ) );

        // if the string is empty, we're done parsing
        if( tab_part.empty() )
            break;

        // Get pane name
        const wxString pane_name = tab_part.BeforeFirst( wxT( '=' ) );

        // create a new tab frame
        wxTabFrame * new_tabs = new wxTabFrame;
        new_tabs->m_tabs = new wxAuiTabCtrl( this,
                                    m_tab_id_counter++,
                                    wxDefaultPosition,
                                    wxDefaultSize,
                                    wxNO_BORDER | wxWANTS_CHARS );
        new_tabs->m_tabs->SetArtProvider( m_tabs.GetArtProvider()->Clone() );
        new_tabs->SetTabCtrlHeight( m_tab_ctrl_height );
        new_tabs->m_tabs->SetFlags( m_flags );
        wxAuiTabCtrl * dest_tabs = new_tabs->m_tabs;

        // create a pane info structure with the information
        // about where the pane should be added
        wxAuiPaneInfo pane_info = wxAuiPaneInfo().Name( pane_name ).Bottom().CaptionVisible( false );
        m_mgr.AddPane( new_tabs, pane_info );

        // Get list of tab id's and move them to pane
        wxString tab_list = tab_part.AfterFirst( wxT( '=' ) );
        while( 1 )
        {
            wxString tab = tab_list.BeforeFirst( wxT( ',' ) );
            if( tab.empty() )
                break;
            tab_list = tab_list.AfterFirst( wxT( ',' ) );

            // Check if this page has an 'active' marker
            const wxChar c = tab[ 0 ];
            if( c == wxT( '+' ) || c == wxT( '*' ) )
            {
                tab = tab.Mid( 1 );
            }

            const size_t tab_idx = wxAtoi( tab.c_str() );
            if( tab_idx >= GetPageCount() )
                continue;

            // Move tab to pane
            wxAuiNotebookPage &page = m_tabs.GetPage( tab_idx );
            const size_t newpage_idx = dest_tabs->GetPageCount();
            dest_tabs->InsertPage( page.window, page, newpage_idx );

            if( c == wxT( '+' ) )
                dest_tabs->SetActivePage( newpage_idx );
            else if( c == wxT( '*' ) )
                sel_page = tab_idx;
        }
        dest_tabs->DoShowHide();

        tabs = tabs.AfterFirst( wxT( '|' ) );
    }

    // Load the frame perspective
    const wxString frames = layout.AfterFirst( wxT( '@' ) );
    m_mgr.LoadPerspective( frames );

    // Force refresh of selection
    m_curpage = -1;
    SetSelection( sel_page );

    return true;
}

// -------------------------------------------------------------------------------- //
wxAuiPaneInfo & guAuiNotebook::GetPane( wxWindow * window )
{
    return m_mgr.GetPane( window );
}

// -------------------------------------------------------------------------------- //
