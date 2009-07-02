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
#include "PlayListPanel.h"


// -------------------------------------------------------------------------------- //
// guPLNamesListBox
// -------------------------------------------------------------------------------- //
//class guPLNamesListBox : public guListBox
//{
//};


// -------------------------------------------------------------------------------- //
// guPlayListPanel
// -------------------------------------------------------------------------------- //
guPlayListPanel::guPlayListPanel( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel ) :
                 wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_PlayerPanel = playerpanel;

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_MainSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );

    wxPanel *           NamesPanel;
	NamesPanel = new wxPanel( m_MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* NameSizer;
	NameSizer = new wxBoxSizer( wxVERTICAL );

	m_NamesListCtrl = new wxListCtrl( NamesPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_LIST|wxLC_VIRTUAL );
	NameSizer->Add( m_NamesListCtrl, 1, wxALL|wxEXPAND, 5 );

	NamesPanel->SetSizer( NameSizer );
	NamesPanel->Layout();
	NameSizer->Fit( NamesPanel );

    wxPanel *           DetailsPanel;
	DetailsPanel = new wxPanel( m_MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* DetailsSizer;
	DetailsSizer = new wxBoxSizer( wxVERTICAL );

	m_DetailsListCtrl = new wxListCtrl( DetailsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_LIST|wxLC_VIRTUAL );
	DetailsSizer->Add( m_DetailsListCtrl, 1, wxALL|wxEXPAND, 5 );

	DetailsPanel->SetSizer( DetailsSizer );
	DetailsPanel->Layout();
	DetailsSizer->Fit( DetailsPanel );
	m_MainSplitter->SplitVertically( NamesPanel, DetailsPanel, 176 );
	MainSizer->Add( m_MainSplitter, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();


	m_MainSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPlayListPanel::MainSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guPlayListPanel::~guPlayListPanel()
{
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::MainSplitterOnIdle( wxIdleEvent &event )
{
    m_MainSplitter->SetSashPosition( 176 );
	m_MainSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guPlayListPanel::MainSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
