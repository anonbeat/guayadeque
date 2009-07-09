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

#include "Commands.h"
#include "Config.h"
#include "DbLibrary.h"
#include "DynamicPlayList.h"
#include "Images.h"
#include "LabelEditor.h"
#include "TagInfo.h"
#include "TrackEdit.h"
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

    Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guPLNamesTreeCtrl::OnContextMenu ), NULL, this );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPLNamesTreeCtrl::~guPLNamesTreeCtrl()
{
    Disconnect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guPLNamesTreeCtrl::OnContextMenu ), NULL, this );
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
void guPLNamesTreeCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPoint();

    wxTreeItemId ItemId = event.GetItem();
    guPLNamesData * ItemData = NULL;

    if( ItemId.IsOk() )
    {
        ItemData = ( guPLNamesData * ) GetItemData( ItemId );

        if( ItemData )
        {
            MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_PLAY, _( "Play" ), _( "Play current selected songs" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to the playlist" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
            Menu.Append( MenuItem );

            Menu.AppendSeparator();
        }
    }

    MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_NEWPLAYLIST, _( "New Dynamic Playlist" ), _( "Create a new dynamic playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
    Menu.Append( MenuItem );

    if( ItemData )
    {
        if( ItemData->GetType() == GUPLAYLIST_DYNAMIC )
        {
            MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_EDIT, _( "Edit Playlist" ), _( "Edit the selected playlist" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
            Menu.Append( MenuItem );
        }

        MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_RENAME, _( "Rename Playlist" ), _( "Change the name of the selected playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_DELETE, _( "Delete Playlist" ), _( "Delete the selected playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu.Append( MenuItem );

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_PLAYLIST_COPYTO, _( "Copy to..." ), _( "Copy the current playlist to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu.Append( MenuItem );
    }

    PopupMenu( &Menu, Point );

    event.Skip();
}

// -------------------------------------------------------------------------------- //
// guPLTracksListBox
// -------------------------------------------------------------------------------- //
guPLTracksListBox::guPLTracksListBox( wxWindow * parent, DbLibrary * db, wxString confname ) :
  guSoListBox( parent, db, confname )
{
    m_PLId = wxNOT_FOUND;
    m_PLType = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
guPLTracksListBox::~guPLTracksListBox()
{
}

// -------------------------------------------------------------------------------- //
void guPLTracksListBox::FillTracks( void )
{
    if( m_PLId > 0 )
    {
        SetItemCount( m_Db->GetPlayListSongs( m_PLId, m_PLType, &m_Songs ) );
    }
    else
    {
        SetItemCount( 0 );
    }
}

// -------------------------------------------------------------------------------- //
void guPLTracksListBox::SetPlayList( int plid, int pltype )
{
    m_PLId = plid;
    m_PLType = pltype;
    ReloadItems();
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

	m_PLTracksListBox = new guPLTracksListBox( DetailsPanel, m_Db, wxT( "PLColSize" ) );
	DetailsSizer->Add( m_PLTracksListBox, 1, wxALL|wxEXPAND, 1 );

	DetailsPanel->SetSizer( DetailsSizer );
	DetailsPanel->Layout();
	DetailsSizer->Fit( DetailsPanel );
	m_MainSplitter->SplitVertically( NamesPanel, DetailsPanel, 176 );
	MainSizer->Add( m_MainSplitter, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guPlayListPanel::OnPLNamesSelected ), NULL, this );
	Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( guPlayListPanel::OnPLNamesActivated ), NULL, this );
    Connect( ID_PLAYLIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesPlay ) );
    Connect( ID_PLAYLIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesEnqueue ) );
    Connect( ID_PLAYLIST_NEWPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesNewPlaylist ) );
    Connect( ID_PLAYLIST_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesEditPlaylist ) );
    Connect( ID_PLAYLIST_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesRenamePlaylist ) );
    Connect( ID_PLAYLIST_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesDeletePlaylist ) );
    Connect( ID_PLAYLIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesCopyTo ) );


    m_PLTracksListBox->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( guPlayListPanel::OnPLTracksActivated ), NULL, this );
    Connect( ID_SONG_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksPlayClicked ) );
    Connect( ID_SONG_PLAYALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksPlayAllClicked ) );
    Connect( ID_SONG_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksQueueClicked ) );
    Connect( ID_SONG_ENQUEUEALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksQueueAllClicked ) );
    Connect( ID_SONG_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksEditLabelsClicked ) );
    Connect( ID_SONG_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksEditTracksClicked ) );
    Connect( ID_SONG_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksCopyToClicked ) );

	m_MainSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPlayListPanel::MainSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guPlayListPanel::~guPlayListPanel()
{
	Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guPlayListPanel::OnPLNamesSelected ), NULL, this );
	Disconnect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( guPlayListPanel::OnPLNamesActivated ), NULL, this );
    Disconnect( ID_PLAYLIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesPlay ) );
    Disconnect( ID_PLAYLIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesEnqueue ) );
    Disconnect( ID_PLAYLIST_NEWPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesNewPlaylist ) );
    Disconnect( ID_PLAYLIST_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesEditPlaylist ) );
    Disconnect( ID_PLAYLIST_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesRenamePlaylist ) );
    Disconnect( ID_PLAYLIST_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesDeletePlaylist ) );
    Disconnect( ID_PLAYLIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLNamesCopyTo ) );

    m_PLTracksListBox->Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( guPlayListPanel::OnPLTracksActivated ), NULL, this );
    Disconnect( ID_SONG_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksPlayClicked ) );
    Disconnect( ID_SONG_PLAYALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksPlayAllClicked ) );
    Disconnect( ID_SONG_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksQueueClicked ) );
    Disconnect( ID_SONG_ENQUEUEALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksQueueAllClicked ) );
    Disconnect( ID_SONG_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksEditLabelsClicked ) );
    Disconnect( ID_SONG_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksEditTracksClicked ) );
    Disconnect( ID_SONG_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPlayListPanel::OnPLTracksCopyToClicked ) );
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesSelected( wxTreeEvent& event )
{
    guPLNamesData * ItemData = ( guPLNamesData * ) m_NamesTreeCtrl->GetItemData( event.GetItem() );
    if( ItemData )
    {
        m_PLTracksListBox->SetPlayList( ItemData->GetData(), ItemData->GetType() );
    }
    else
    {
        m_PLTracksListBox->SetPlayList( -1, -1 );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesActivated( wxTreeEvent& event )
{
    guPLNamesData * ItemData = ( guPLNamesData * ) m_NamesTreeCtrl->GetItemData( event.GetItem() );
    if( ItemData )
    {
        guTrackArray Tracks = m_PLTracksListBox->GetAllSongs();
        m_PlayerPanel->SetPlayList( Tracks );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesPlay( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_NamesTreeCtrl->GetSelection();
    if( ItemId.IsOk() )
    {
        guTrackArray Tracks = m_PLTracksListBox->GetAllSongs();
        m_PlayerPanel->SetPlayList( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesEnqueue( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_NamesTreeCtrl->GetSelection();
    if( ItemId.IsOk() )
    {
        guTrackArray Tracks = m_PLTracksListBox->GetAllSongs();
        m_PlayerPanel->AddToPlayList( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesNewPlaylist( wxCommandEvent &event )
{
    guDynPlayList DynPlayList;
    guDynPlayListEditor * PlayListEditor = new guDynPlayListEditor( this, &DynPlayList );
    if( PlayListEditor->ShowModal() == wxID_OK )
    {
        PlayListEditor->FillPlayListEditData();

        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "PlayList Name: " ),
          _( "Enter the new playlist name" ), wxT( "New Dynamic Playlist" ) );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            m_Db->CreateDynamicPlayList( EntryDialog->GetValue(), &DynPlayList );
            m_NamesTreeCtrl->ReloadItems();
        }
        EntryDialog->Destroy();
    }
    PlayListEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesEditPlaylist( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_NamesTreeCtrl->GetSelection();
    if( ItemId.IsOk() )
    {
        guPLNamesData * ItemData = ( guPLNamesData * ) m_NamesTreeCtrl->GetItemData( ItemId );
        guDynPlayList DynPlayList;
        m_Db->GetDynamicPlayList( ItemData->GetData(), &DynPlayList );
        guDynPlayListEditor * PlayListEditor = new guDynPlayListEditor( this, &DynPlayList );
        if( PlayListEditor->ShowModal() == wxID_OK )
        {
            PlayListEditor->FillPlayListEditData();
            m_Db->UpdateDynPlayList( ItemData->GetData(), &DynPlayList );
            m_NamesTreeCtrl->ReloadItems();
        }
        PlayListEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesRenamePlaylist( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_NamesTreeCtrl->GetSelection();
    if( ItemId.IsOk() )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "PlayList Name: " ),
          _( "Enter the new playlist name" ), m_NamesTreeCtrl->GetItemText( ItemId ) );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            guPLNamesData * ItemData = ( guPLNamesData * ) m_NamesTreeCtrl->GetItemData( ItemId );
            wxASSERT( ItemData );
            m_Db->SetPlayListName( ItemData->GetData(), EntryDialog->GetValue() );
            m_NamesTreeCtrl->SetItemText( ItemId, EntryDialog->GetValue() );
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesDeletePlaylist( wxCommandEvent &event )
{
    if( wxMessageBox( _( "Are you sure to delete the selected Playlist?" ),
                      _( "Confirm" ),
                      wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
    {
        wxTreeItemId ItemId = m_NamesTreeCtrl->GetSelection();
        if( ItemId.IsOk() )
        {
            guPLNamesData * ItemData = ( guPLNamesData * ) m_NamesTreeCtrl->GetItemData( ItemId );
            wxASSERT( ItemData );
            m_Db->DeletePlayList( ItemData->GetData() );
            m_NamesTreeCtrl->Delete( ItemId );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLNamesCopyTo( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_NamesTreeCtrl->GetSelection();
    if( ItemId.IsOk() )
    {
        guTrackArray * Tracks = new guTrackArray( m_PLTracksListBox->GetAllSongs() );
        event.SetId( ID_MAINFRAME_COPYTO );
        event.SetClientData( ( void * ) Tracks );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksActivated( wxListEvent &event )
{
    guTrackArray Tracks = m_PLTracksListBox->GetSelectedSongs();
    if( Tracks.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
            {
                m_PlayerPanel->AddToPlayList( Tracks );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Tracks );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksPlayClicked( wxCommandEvent &event )
{
    guTrackArray Tracks = m_PLTracksListBox->GetSelectedSongs();
    if( !Tracks.Count() )
        Tracks = m_PLTracksListBox->GetAllSongs();
    m_PlayerPanel->SetPlayList( Tracks );
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksPlayAllClicked( wxCommandEvent &event )
{
    guTrackArray Tracks = m_PLTracksListBox->GetAllSongs();
    m_PlayerPanel->SetPlayList( Tracks );
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksQueueClicked( wxCommandEvent &event )
{
    guTrackArray Tracks = m_PLTracksListBox->GetSelectedSongs();
    if( !Tracks.Count() )
        Tracks = m_PLTracksListBox->GetAllSongs();
    m_PlayerPanel->AddToPlayList( Tracks );
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksQueueAllClicked( wxCommandEvent &event )
{
    guTrackArray Tracks = m_PLTracksListBox->GetAllSongs();
    m_PlayerPanel->AddToPlayList( Tracks );
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Labels;
    wxArrayInt SongIds;

    m_Db->GetLabels( &Labels, true );

    SongIds = m_PLTracksListBox->GetSelection();

    guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Songs Labels Editor" ),
                         Labels, m_Db->GetSongsLabels( SongIds ) );
    if( LabelEditor )
    {
        if( LabelEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongsLabels( SongIds, LabelEditor->GetCheckedIds() );
        }
        LabelEditor->Destroy();
        m_PLTracksListBox->ReloadItems();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks = m_PLTracksListBox->GetSelectedSongs();
    guImagePtrArray Images;
    if( !Tracks.Count() )
        return;

    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Tracks );
            UpdateImages( Tracks, Images );
            m_PLTracksListBox->ReloadItems();
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::OnPLTracksCopyToClicked( wxCommandEvent &event )
{
    guTrackArray Tracks = m_PLTracksListBox->GetSelectedSongs();

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) new guTrackArray( Tracks ) );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
void guPlayListPanel::MainSplitterOnIdle( wxIdleEvent &event )
{
    m_MainSplitter->SetSashPosition( 176 );
	m_MainSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guPlayListPanel::MainSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
