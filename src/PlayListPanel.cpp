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

#include "DbLibrary.h"
#include "Images.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
class guPLNamesData : public wxTreeItemData
{
  private :
    int m_Id;
    int m_Type;
  public :
    guPLNamesData( const int id, const int type ) { m_Id = id; m_Type = type; };
    int     GetData( void ) { return m_Id; };
    void    SetData( int id ) { m_Id = id; };
    int     GetType( void ) { return m_Type; };
    void    SetType( int type ) { m_Type = type; };
};

// -------------------------------------------------------------------------------- //
guPLNamesTreeCtrl::guPLNamesTreeCtrl( wxWindow * parent, DbLibrary * db ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT|wxTR_SINGLE|wxSUNKEN_BORDER )
{
    m_Db = db;
    m_ImageList = new wxImageList();
    m_ImageList->Add( wxBitmap( guImage( guIMAGE_INDEX_track ) ) );
    m_ImageList->Add( wxBitmap( guImage( guIMAGE_INDEX_system_run ) ) );

    AssignImageList( m_ImageList );

    m_RootId   = AddRoot( wxT( "Playlists" ), -1, -1, NULL );
    m_StaticId = AppendItem( m_RootId, _( "Static playlists" ), 0, 0, NULL );
    m_DynamicId = AppendItem( m_RootId, _( "Dynamic playlists" ), 1, 1, NULL );

    SetIndent( 5 );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPLNamesTreeCtrl::~guPLNamesTreeCtrl()
{
}

// -------------------------------------------------------------------------------- //
void guPLNamesTreeCtrl::ReloadItems( void )
{
    int index;
    int count;
//    wxTreeItemId Selected = GetSelection();

    DeleteChildren( m_StaticId );
    DeleteChildren( m_DynamicId );

    guListItems m_StaticItems;
    m_Db->GetPlayLists( &m_StaticItems, GUPLAYLIST_STATIC );
    if( ( count = m_StaticItems.Count() ) )
    {
        for( index = 0; index < count; index++ )
        {
            AppendItem( m_StaticId, m_StaticItems[ index ].m_Name, 0, 0,
                                new guPLNamesData( m_StaticItems[ index ].m_Id, GUPLAYLIST_STATIC ) );
        }
    }

    guListItems m_DynamicItems;
    m_Db->GetPlayLists( &m_DynamicItems, GUPLAYLIST_DYNAMIC );
    if( ( count = m_DynamicItems.Count() ) )
    {
        for( index = 0; index < count; index++ )
        {
            AppendItem( m_DynamicId, m_DynamicItems[ index ].m_Name, 1, 1,
                                new guPLNamesData( m_DynamicItems[ index ].m_Id, GUPLAYLIST_DYNAMIC ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
// guPLTracksListBox
// -------------------------------------------------------------------------------- //
guPLTracksListBox::guPLTracksListBox( wxWindow * parent, DbLibrary * db ) :
  guSoListBox( parent, db )
{
}

// -------------------------------------------------------------------------------- //
guPLTracksListBox::~guPLTracksListBox()
{
}

// -------------------------------------------------------------------------------- //
void guPLTracksListBox::FillTracks( void )
{
    guLogMessage( wxT( "guPLTracksListBox FillTracks" ) );
    SetItemCount( 0 );
}

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

    wxStaticText * m_NameStaticText;
    m_NameStaticText = new wxStaticText( NamesPanel, wxID_ANY, _( "Play lists:" ) );
    NameSizer->Add( m_NameStaticText, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	m_NamesTreeCtrl = new guPLNamesTreeCtrl( NamesPanel, m_Db );
	NameSizer->Add( m_NamesTreeCtrl, 1, wxALL|wxEXPAND, 1 );

	NamesPanel->SetSizer( NameSizer );
	NamesPanel->Layout();
	NameSizer->Fit( NamesPanel );

    wxPanel *           DetailsPanel;
	DetailsPanel = new wxPanel( m_MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* DetailsSizer;
	DetailsSizer = new wxBoxSizer( wxVERTICAL );

	m_PLTracksListBox = new guPLTracksListBox( DetailsPanel, m_Db );
	DetailsSizer->Add( m_PLTracksListBox, 1, wxALL|wxEXPAND, 1 );

	DetailsPanel->SetSizer( DetailsSizer );
	DetailsPanel->Layout();
	DetailsSizer->Fit( DetailsPanel );
	m_MainSplitter->SplitVertically( NamesPanel, DetailsPanel, 176 );
	MainSizer->Add( m_MainSplitter, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	m_NamesTreeCtrl->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guPlayListPanel::OnPLNamesSelected ), NULL, this );


	m_MainSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPlayListPanel::MainSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guPlayListPanel::~guPlayListPanel()
{
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesSelected( wxTreeEvent& event )
{
    guPLNamesData * ItemData = ( guPLNamesData * ) m_NamesTreeCtrl->GetItemData( event.GetItem() );
    if( ItemData )
    {
        int PlayListId = ItemData->GetData();
        int PlayListType = ItemData->GetType();
        guLogMessage( wxT( "Item: %u type: %u" ), PlayListId, PlayListType );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::MainSplitterOnIdle( wxIdleEvent &event )
{
    m_MainSplitter->SetSashPosition( 176 );
	m_MainSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guPlayListPanel::MainSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
