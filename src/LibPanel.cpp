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
#include "LibPanel.h"
#include "Config.h"
#include "Utils.h"
#include "Commands.h"
#include "LabelEditor.h"
#include "MainFrame.h"
#include "PlayListAppend.h"
#include "SelCoverFile.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "CoverEdit.h"
#include "Images.h"

#include <wx/event.h>
#include <wx/uri.h>

#define LISTCTRL_BORDER 1

// -------------------------------------------------------------------------------- //
guLibPanel::guLibPanel( wxWindow* parent, guDbLibrary * NewDb, guPlayerPanel * NewPlayerPanel )
       : wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 368,191 ), wxTAB_TRAVERSAL )
{
    wxPanel *           SearchPanel;
    wxPanel *           GenrePanel;
    wxPanel *           LabelsPanel;
    wxPanel *           AlbumPanel;
    wxPanel *           ArtistPanel;

	wxBoxSizer *        GenreSizer;
	wxBoxSizer *        LabelsSizer;
	wxBoxSizer *        ArtistSizer;
	wxBoxSizer *        AlbumSizer;
    wxPanel *           SongListPanel;
	wxBoxSizer *        SongListSizer;

    guConfig *          Config = ( guConfig * ) guConfig::Get();

    m_Db = NewDb;
    m_PlayerPanel = NewPlayerPanel;

    m_AuiManager.SetManagedWindow( this );
    m_AuiManager.SetFlags( wxAUI_MGR_ALLOW_FLOATING |
                           wxAUI_MGR_TRANSPARENT_DRAG |
                           wxAUI_MGR_TRANSPARENT_HINT );
    wxAuiDockArt * AuiDockArt = m_AuiManager.GetArtProvider();
    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );
    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR,
            wxAuiStepColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ), 130 ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR,
            wxAuiStepColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ), 130 ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_GRADIENT_TYPE,
            wxAUI_GRADIENT_VERTICAL );

    m_VisiblePanels = Config->ReadNum( wxT( "LibVisiblePanels" ), guPANEL_LIBRARY_VISIBLE_DEFAULT, wxT( "Positions" ) );
    //
    //
    //
//	LibraryMainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer *        SearchSizer;
	SearchSizer = new wxBoxSizer( wxHORIZONTAL );
	SearchPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    wxStaticText *      SearchStaticText;
	SearchStaticText = new wxStaticText( SearchPanel, wxID_ANY, _( "Search:" ), wxDefaultPosition, wxDefaultSize, 0 );
	SearchStaticText->Wrap( -1 );
	SearchSizer->Add( SearchStaticText, 0, wxALIGN_CENTER|wxALL, 5 );

    m_InputTextCtrl = new wxSearchCtrl( SearchPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    SearchSizer->Add( m_InputTextCtrl, 1, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );

    SearchPanel->SetSizer( SearchSizer );
    SearchPanel->Layout();
	SearchSizer->Fit( SearchPanel );

    m_AuiManager.AddPane( SearchPanel,
            wxAuiPaneInfo().Name( wxT( "TextSearch" ) ).Caption( _( "Text Search" ) ).
            MinSize( 60, 28 ).MaxSize( -1, 28 ).Row( 0 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );


	GenreSizer = new wxBoxSizer( wxVERTICAL );
	GenrePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	m_GenreListCtrl = new guGeListBox( GenrePanel, m_Db, _( "Genres" ) );
	GenreSizer->Add( m_GenreListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	GenrePanel->SetSizer( GenreSizer );
	GenrePanel->Layout();
	GenreSizer->Fit( GenrePanel );

    m_AuiManager.AddPane( GenrePanel, wxAuiPaneInfo().Name( wxT( "Genres" ) ).Caption( _( "Genres" ) ).
            MinSize( 50, 50 ).Row( 1 ).
            Position( 1 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );



	LabelsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	LabelsSizer = new wxBoxSizer( wxVERTICAL );

	m_LabelsListCtrl = new guTaListBox( LabelsPanel, m_Db, _( "Labels" ) );
	LabelsSizer->Add( m_LabelsListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	LabelsPanel->SetSizer( LabelsSizer );
	LabelsPanel->Layout();
	LabelsSizer->Fit( LabelsPanel );

    m_AuiManager.AddPane( LabelsPanel, wxAuiPaneInfo().Name( wxT( "Labels" ) ).Caption( _( "Labels" ) ).
            MinSize( 50, 50 ).Row( 1 ).
            Position( 0 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );

	ArtistPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	ArtistSizer = new wxBoxSizer( wxVERTICAL );

	m_ArtistListCtrl = new guArListBox( ArtistPanel, m_Db, _( "Artists" ) );
	ArtistSizer->Add( m_ArtistListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	ArtistPanel->SetSizer( ArtistSizer );
	ArtistPanel->Layout();
	ArtistSizer->Fit( ArtistPanel );

    m_AuiManager.AddPane( ArtistPanel, wxAuiPaneInfo().Name( wxT( "Artists" ) ).Caption( _( "Artists" ) ).
            MinSize( 50, 50 ).Row( 1 ).
            Position( 2 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );


	AlbumPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	AlbumSizer = new wxBoxSizer( wxVERTICAL );

	m_AlbumListCtrl = new guAlListBox( AlbumPanel, m_Db, _( "Albums" ) );
	AlbumSizer->Add( m_AlbumListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	AlbumPanel->SetSizer( AlbumSizer );
	AlbumPanel->Layout();
	AlbumSizer->Fit( AlbumPanel );

    m_AuiManager.AddPane( AlbumPanel, wxAuiPaneInfo().Name( wxT( "Albums" ) ).Caption( _( "Albums" ) ).
            MinSize( 50, 50 ).Row( 1 ).
            Position( 3 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );


	SongListPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	SongListSizer = new wxBoxSizer( wxVERTICAL );

	m_SongListCtrl = new guSoListBox( SongListPanel, m_Db, wxT( "Song" ), guLISTVIEW_COLUMN_SELECT|guLISTVIEW_COLUMN_SORTING );
	//m_SongListCtrl->ReloadItems();
	SongListSizer->Add( m_SongListCtrl, 1, wxEXPAND, 5 );

	SongListPanel->SetSizer( SongListSizer );
	SongListPanel->Layout();
	SongListSizer->Fit( SongListPanel );

    m_AuiManager.AddPane( SongListPanel, wxAuiPaneInfo().Name( wxT( "Tracks" ) ).Caption( _( "Tracks" ) ).
            MinSize( 50, 50 ).
            CenterPane() );


    wxString LibraryLayout = Config->ReadStr( wxT( "Library" ), wxEmptyString, wxT( "Positions" ) );
    if( Config->GetIgnoreLayouts() || LibraryLayout.IsEmpty() )
    {
        m_AuiManager.Update();
        m_VisiblePanels = guPANEL_LIBRARY_VISIBLE_DEFAULT;
    }
    else
    {
        m_AuiManager.LoadPerspective( LibraryLayout, true );
    }

    //
    m_UpdateLock = false;


    //
    m_GenreListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnGenreListSelected ), NULL, this );
    //m_GenreListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnGenreListSelected ), NULL, this );
    m_GenreListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnGenreListActivated ), NULL, this );

    m_LabelsListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnLabelListSelected ), NULL, this );
    //m_LabelsListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnLabelListSelected ), NULL, this );
    m_LabelsListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnLabelListActivated ), NULL, this );

    m_ArtistListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnArtistListSelected ), NULL, this );
    //m_ArtistListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnArtistListSelected ), NULL, this );
    m_ArtistListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnArtistListActivated ), NULL, this );

    m_AlbumListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnAlbumListSelected ), NULL, this );
    m_AlbumListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnAlbumListActivated ), NULL, this );

    m_SongListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnSongListActivated ), NULL, this );
    m_SongListCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guLibPanel::OnSongListColClicked ), NULL, this );

    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guLibPanel::OnSearchCancelled ), NULL, this );

    Connect( ID_GENRE_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenrePlayClicked ), NULL, this );
    Connect( ID_GENRE_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreQueueClicked ), NULL, this );
    Connect( ID_GENRE_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreCopyToClicked ), NULL, this );

    Connect( ID_LABEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelPlayClicked ), NULL, this );
    Connect( ID_LABEL_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelQueueClicked ), NULL, this );
    //Connect( ID_LABEL_CLEARSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelClearSelectClicked ) );
    Connect( ID_LABEL_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelCopyToClicked ), NULL, this );

    Connect( ID_ARTIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistPlayClicked ), NULL, this );
    Connect( ID_ARTIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistQueueClicked ), NULL, this );
    Connect( ID_ARTIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditLabelsClicked ), NULL, this );
    Connect( ID_ARTIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditTracksClicked ), NULL, this );
    Connect( ID_ARTIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistCopyToClicked ), NULL, this );

    Connect( ID_ALBUM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumPlayClicked ), NULL, this );
    Connect( ID_ALBUM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumQueueClicked ), NULL, this );
    Connect( ID_ALBUM_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditLabelsClicked ), NULL, this );
    Connect( ID_ALBUM_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditTracksClicked ), NULL, this );
    Connect( ID_ALBUM_MANUALCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDownloadCoverClicked ), NULL, this );
    Connect( ID_ALBUM_SELECT_COVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumSelectCoverClicked ), NULL, this );
    Connect( ID_ALBUM_COVER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDeleteCoverClicked ), NULL, this );
    Connect( ID_ALBUM_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumCopyToClicked ), NULL, this );

    Connect( ID_SONG_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayClicked ), NULL, this );
    Connect( ID_SONG_PLAYALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayAllClicked ), NULL, this );
    Connect( ID_SONG_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueClicked ), NULL, this );
    Connect( ID_SONG_ENQUEUEALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAllClicked ), NULL, this );
    Connect( ID_SONG_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditLabelsClicked ), NULL, this );
    Connect( ID_SONG_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditTracksClicked ), NULL, this );
    Connect( ID_SONG_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongCopyToClicked ), NULL, this );
    Connect( ID_SONG_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSavePlayListClicked ), NULL, this );

    Connect( ID_SONG_BROWSE_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectGenre ), NULL, this );
    Connect( ID_SONG_BROWSE_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectArtist ), NULL, this );
    Connect( ID_SONG_BROWSE_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectAlbum ), NULL, this );

    m_AuiManager.Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guLibPanel::OnPaneClose ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guLibPanel::~guLibPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteNum( wxT( "LibVisiblePanels" ), m_VisiblePanels, wxT( "Positions" ) );
        Config->WriteStr( wxT( "Library" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
    }

    // Disconnect all controls
    m_GenreListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED,  wxListEventHandler( guLibPanel::OnGenreListSelected ), NULL, this );
    //m_GenreListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnGenreListSelected ), NULL, this );
    m_GenreListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( guLibPanel::OnGenreListActivated ), NULL, this );

    m_LabelsListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED,  wxListEventHandler( guLibPanel::OnLabelListSelected ), NULL, this );
    //m_LabelsListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnLabelListSelected ), NULL, this );
    m_LabelsListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( guLibPanel::OnLabelListActivated ), NULL, this );

    m_ArtistListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED,  wxListEventHandler( guLibPanel::OnArtistListSelected ), NULL, this );
    //m_ArtistListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnArtistListSelected ), NULL, this );
    m_ArtistListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( guLibPanel::OnArtistListActivated ), NULL, this );

    m_SongListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnSongListActivated ), NULL, this );
    m_SongListCtrl->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guLibPanel::OnSongListColClicked ), NULL, this );

    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guLibPanel::OnSearchCancelled ), NULL, this );

    Disconnect( ID_GENRE_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenrePlayClicked ) );
    Disconnect( ID_GENRE_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreQueueClicked ) );
    Disconnect( ID_GENRE_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreCopyToClicked ) );

    Disconnect( ID_LABEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelPlayClicked ) );
    Disconnect( ID_LABEL_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelQueueClicked ) );
//    Disconnect( ID_LABEL_CLEARSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelClearSelectClicked ) );
    Disconnect( ID_LABEL_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelCopyToClicked ) );

    Disconnect( ID_ARTIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistPlayClicked ) );
    Disconnect( ID_ARTIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistQueueClicked ) );
    Disconnect( ID_ARTIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditLabelsClicked ) );
    Disconnect( ID_ARTIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditTracksClicked ) );
    Disconnect( ID_ARTIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistCopyToClicked ) );

    Disconnect( ID_ALBUM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumPlayClicked ) );
    Disconnect( ID_ALBUM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumQueueClicked ) );
    Disconnect( ID_ALBUM_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditLabelsClicked ) );
    Disconnect( ID_ALBUM_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditTracksClicked ) );
    Disconnect( ID_ALBUM_MANUALCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDownloadCoverClicked ) );
    Disconnect( ID_ALBUM_SELECT_COVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumSelectCoverClicked ) );
    Disconnect( ID_ALBUM_COVER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDeleteCoverClicked ) );
    Disconnect( ID_ALBUM_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumCopyToClicked ) );

    Disconnect( ID_SONG_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayClicked ) );
    Disconnect( ID_SONG_PLAYALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayAllClicked ) );
    Disconnect( ID_SONG_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueClicked ) );
    Disconnect( ID_SONG_ENQUEUEALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAllClicked ) );
    Disconnect( ID_SONG_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditLabelsClicked ) );
    Disconnect( ID_SONG_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditTracksClicked ) );
    Disconnect( ID_SONG_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongCopyToClicked ) );
    Disconnect( ID_SONG_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSavePlayListClicked ) );

    Disconnect( ID_SONG_BROWSE_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectGenre ) );
    Disconnect( ID_SONG_BROWSE_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectArtist ) );
    Disconnect( ID_SONG_BROWSE_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectAlbum ) );

    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guLibPanel::OnPaneClose ), NULL, this );

    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guLibPanel::ReloadControls( wxCommandEvent &event )
{
    m_Db->LoadCache();
    m_UpdateLock = true;
    m_LabelsListCtrl->ReloadItems( false );
    m_GenreListCtrl->ReloadItems( false );
    m_ArtistListCtrl->ReloadItems( false );
    m_AlbumListCtrl->ReloadItems( false );
    m_SongListCtrl->ReloadItems();
    m_UpdateLock = false;
}

// -------------------------------------------------------------------------------- //
// TextSearch Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSearchActivated( wxCommandEvent& event )
{
    wxString SearchString = m_InputTextCtrl->GetLineText( 0 );
    if( !SearchString.IsEmpty() )
    {
        wxArrayString Words = guSplitWords( SearchString );

        m_Db->SetTeFilters( Words );
        m_UpdateLock = true;
        m_LabelsListCtrl->ReloadItems();
        m_GenreListCtrl->ReloadItems();
        m_ArtistListCtrl->ReloadItems();
        m_AlbumListCtrl->ReloadItems();
        m_SongListCtrl->ReloadItems();
        m_UpdateLock = false;
        m_InputTextCtrl->ShowCancelButton( true );
    }
    else
    {
        OnSearchCancelled( event );
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSearchCancelled( wxCommandEvent &event ) // CLEAN SEARCH STR
{
    wxArrayString Words;
    //guLogMessage( wxT( "guLibPanel::SearchCancelled" ) );
    m_InputTextCtrl->Clear();
    m_Db->SetTeFilters( Words );
    m_UpdateLock = true;
    m_LabelsListCtrl->ReloadItems( false );
    m_GenreListCtrl->ReloadItems( false );
    m_ArtistListCtrl->ReloadItems( false );
    m_AlbumListCtrl->ReloadItems( false );
    m_SongListCtrl->ReloadItems( false );
    m_UpdateLock = false;
    m_InputTextCtrl->ShowCancelButton( false );
}

// -------------------------------------------------------------------------------- //
// GenreListBox Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnGenreListSelected( wxListEvent &event )
{
    //wxLongLong time = wxGetLocalTimeMillis();
    m_Db->SetGeFilters( m_GenreListCtrl->GetSelectedItems() );

    if( !m_UpdateLock )
    {
        m_UpdateLock = true;
        m_ArtistListCtrl->ReloadItems();
        m_AlbumListCtrl->ReloadItems();
        m_SongListCtrl->ReloadItems();
        m_UpdateLock = false;
    }
    //time = wxGetLocalTimeMillis() - time;;
    //guLogWarning( wxT( "Genre Time : %u ms" ), time.GetLo() );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnGenreListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_GenreListCtrl->GetSelectedSongs( &Songs );
    if( Songs.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
            {
                m_PlayerPanel->AddToPlayList( Songs );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Songs );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnGenrePlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_GenreListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnGenreQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_GenreListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnGenreCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_GenreListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
// LabelsListBox Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnLabelListSelected( wxListEvent &event )
{
    m_Db->SetTaFilters( m_LabelsListCtrl->GetSelectedItems() );
    if( !m_UpdateLock )
    {
        m_UpdateLock = true;
        m_GenreListCtrl->ReloadItems();
        m_ArtistListCtrl->ReloadItems();
        m_AlbumListCtrl->ReloadItems();
        m_SongListCtrl->ReloadItems();
        m_UpdateLock = false;
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnLabelListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_LabelsListCtrl->GetSelectedSongs( &Songs );
    if( Songs.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
            {
                m_PlayerPanel->AddToPlayList( Songs );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Songs );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnLabelPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_LabelsListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnLabelQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_LabelsListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

//// -------------------------------------------------------------------------------- //
//void guLibPanel::OnLabelClearSelectClicked( wxCommandEvent &event )
//{
//    m_LabelsListCtrl->ClearSelection();
//}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnLabelCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_LabelsListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::UpdateLabels( void )
{
    m_UpdateLock = true;
    m_LabelsListCtrl->ReloadItems( false );
    m_UpdateLock = false;
}

// -------------------------------------------------------------------------------- //
// ArtistListBox Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistListSelected( wxListEvent &event )
{
    m_Db->SetArFilters( m_ArtistListCtrl->GetSelectedItems() );
    if( !m_UpdateLock )
    {
        m_UpdateLock = true;
        m_AlbumListCtrl->ReloadItems();
        m_SongListCtrl->ReloadItems();
        m_UpdateLock = false;
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_ArtistListCtrl->GetSelectedSongs( &Songs );
    if( Songs.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
            {
                m_PlayerPanel->AddToPlayList( Songs );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Songs );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_ArtistListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_ArtistListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Labels;
    wxArrayInt Artists;

    Artists = m_ArtistListCtrl->GetSelectedItems();
    if( Artists.Count() )
    {
        m_Db->GetLabels( &Labels, true );

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Artist Labels Editor" ), false, Labels, m_Db->GetArtistsLabels( Artists ) );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                // Update the labels in the artists files
                m_Db->UpdateArtistsLabels( Artists, LabelEditor->GetCheckedIds() );
            }
            LabelEditor->Destroy();
            //m_UpdateLock = true;
            m_LabelsListCtrl->ReloadItems( false );
            //m_UpdateLock = false;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Labels;
    wxArrayInt  Albums;

    m_Db->GetLabels( &Labels, true );

    Albums = m_AlbumListCtrl->GetSelectedItems();
    guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Albums Labels Editor" ), false,
                                         Labels, m_Db->GetAlbumsLabels( Albums ) );
    if( LabelEditor )
    {
        if( LabelEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateAlbumsLabels( Albums, LabelEditor->GetCheckedIds() );
        }
        LabelEditor->Destroy();
        //m_UpdateLock = true;
        m_LabelsListCtrl->ReloadItems( false );
        //m_UpdateLock = false;
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongsEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Labels;
    wxArrayInt SongIds;

    m_Db->GetLabels( &Labels, true );

    SongIds = m_SongListCtrl->GetSelectedItems();
    guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Songs Labels Editor" ), false,
                         Labels, m_Db->GetSongsLabels( SongIds ) );
    if( LabelEditor )
    {
        if( LabelEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongsLabels( SongIds, LabelEditor->GetCheckedIds() );
        }
        LabelEditor->Destroy();
        //m_UpdateLock = true;
        m_LabelsListCtrl->ReloadItems( false );
        //m_UpdateLock = false;
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_ArtistListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------s------------------- //
// AlbumListBox Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumListSelected( wxListEvent &event )
{
    m_Db->SetAlFilters( m_AlbumListCtrl->GetSelectedItems() );
    if( !m_UpdateLock )
    {
        m_UpdateLock = true;
        m_SongListCtrl->ReloadItems();
        m_UpdateLock = false;
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_AlbumListCtrl->GetSelectedSongs( &Songs );
    if( Songs.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false , wxT( "General" )) )
            {
                m_PlayerPanel->AddToPlayList( Songs );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Songs );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_AlbumListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_AlbumListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    m_ArtistListCtrl->GetSelectedSongs( &Tracks );
    if( !Tracks.Count() )
        return;

    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Tracks );
            UpdateImages( Tracks, Images );
            UpdateLyrics( Tracks, Lyrics );

            // Update the track in database, playlist, etc
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_LIBRARY, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    //m_AlbumListCtrl->GetSelectedSongs( &Songs );
    m_Db->GetAlbumsSongs( m_AlbumListCtrl->GetSelectedItems(), &Tracks, true );
    if( !Tracks.Count() )
        return;
    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Tracks );
            UpdateImages( Tracks, Images );
            UpdateLyrics( Tracks, Lyrics );

            // Update the track in database, playlist, etc
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_LIBRARY, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumDownloadCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        wxString AlbumName;
        wxString ArtistName;
        wxString AlbumPath;
        if( !m_Db->GetAlbumInfo( Albums[ 0 ], &AlbumName, &ArtistName, &AlbumPath ) )
        {
            wxMessageBox( _( "Could not find the Album in the songs library.\n"\
                             "You should update the library." ), _( "Error" ), wxICON_ERROR | wxOK );
            return;
        }

        AlbumName = RemoveSearchFilters( AlbumName );

        guCoverEditor * CoverEditor = new guCoverEditor( this, ArtistName, AlbumName );
        if( CoverEditor )
        {
            if( CoverEditor->ShowModal() == wxID_OK )
            {
                //guLogMessage( wxT( "About to download cover from selected url" ) );
                wxImage * CoverImage = CoverEditor->GetSelectedCoverImage();
                if( CoverImage )
                {
                    CoverImage->SaveFile( AlbumPath + wxT( "cover.jpg" ), wxBITMAP_TYPE_JPEG );
                    m_Db->SetAlbumCover( Albums[ 0 ], AlbumPath + wxT( "cover.jpg" ) );
                    //AlbumListCtrl->ClearSelection();
                    //Db->SetAlFilters( wxArrayInt() );
                    m_AlbumListCtrl->ReloadItems( false );
                    //guLogMessage( wxT( "Cover downloaded ok\n" ) );
                }
            }
            CoverEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumSelectCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        guSelCoverFile * SelCoverFile = new guSelCoverFile( this, m_Db, Albums[ 0 ] );
        if( SelCoverFile )
        {
            if( SelCoverFile->ShowModal() == wxID_OK )
            {
                wxString CoverFile = SelCoverFile->GetSelFile();
                if( !CoverFile.IsEmpty() )
                {
                    wxURI Uri( CoverFile );
                    if( Uri.IsReference() )
                    {
                        wxImage CoverImage( CoverFile );
                        if( CoverImage.IsOk() )
                        {
                            if( CoverImage.SaveFile( SelCoverFile->GetAlbumPath() + wxT( "/cover.jpg" ), wxBITMAP_TYPE_JPEG ) )
                            {
                                m_Db->SetAlbumCover( Albums[ 0 ], SelCoverFile->GetAlbumPath() + wxT( "/cover.jpg" ) );
                                m_AlbumListCtrl->ReloadItems( false );
                            }
                        }
                        else
                        {
                            guLogError( wxT( "Could not load the imate '%s'" ), CoverFile.c_str() );
                        }
                    }
                    else
                    {
                        if( DownloadImage( CoverFile, SelCoverFile->GetAlbumPath() + wxT( "/cover.jpg" ) ) )
                        {
                            m_Db->SetAlbumCover( Albums[ 0 ], SelCoverFile->GetAlbumPath() + wxT( "/cover.jpg" ) );
                            m_AlbumListCtrl->ReloadItems( false );
                        }
                        else
                        {
                            guLogError( wxT( "Failed to download file '%s'" ), CoverFile.c_str() );
                        }
                    }
                }
            }
            delete SelCoverFile;
        }
    }
}


// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumDeleteCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected album cover?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            int CoverId = m_Db->GetAlbumCoverId( Albums[ 0 ] );
            if( CoverId > 0 )
            {
                wxString CoverPath = m_Db->GetCoverPath( CoverId );
                wxASSERT( !CoverPath.IsEmpty() );
                if( !wxRemoveFile( CoverPath ) )
                {
                    guLogError( wxT( "Could not remove the cover file '%s'" ), CoverPath.c_str() );
                }
            }
            m_Db->SetAlbumCover( Albums[ 0 ], wxEmptyString );
            m_AlbumListCtrl->ReloadItems( false );
            //bool guDbLibrary::GetAlbumInfo( const int AlbumId, wxString * AlbumName, wxString * ArtistName, wxString * AlbumPath )
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_AlbumListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
// SongListBox Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetSelectedSongs( &Songs );
    if( Songs.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
            {
                m_PlayerPanel->AddToPlayList( Songs );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Songs );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetSelectedSongs( &Songs );
    if( !Songs.Count() )
    {
        m_SongListCtrl->GetAllSongs( &Songs );
    }
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongPlayAllClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetAllSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetSelectedSongs( &Songs );
    if( !Songs.Count() )
    {
        m_SongListCtrl->GetAllSongs( &Songs );
    }
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongQueueAllClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetAllSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongsEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );
    guImagePtrArray Images;
    wxArrayString Lyrics;
    if( !Tracks.Count() )
        return;
    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Tracks );
            UpdateImages( Tracks, Images );
            UpdateLyrics( Tracks, Lyrics );

            // Update the track in database, playlist, etc
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_LIBRARY, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongCopyToClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );

    if( !Tracks.Count() )
        m_SongListCtrl->GetAllSongs( &Tracks );

    if( Tracks.Count() )
    {
        event.SetId( ID_MAINFRAME_COPYTO );
        event.SetClientData( ( void * ) new guTrackArray( Tracks ) );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSavePlayListClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt NewSongs;
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );

    if( ( count = Tracks.Count() ) )
    {
        for( index = 0; index < count; index++ )
        {
            NewSongs.Add( Tracks[ index ].m_SongId );
        }
    }
    else
    {
        m_SongListCtrl->GetAllSongs( &Tracks );
        count = Tracks.Count();
        for( index = 0; index < count; index++ )
        {
            NewSongs.Add( Tracks[ index ].m_SongId );
        }
    }

    if( NewSongs.Count() );
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists,GUPLAYLIST_STATIC );

        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( wxTheApp->GetTopWindow(), m_Db, &NewSongs, &PlayLists );

        if( PlayListAppendDlg->ShowModal() == wxID_OK )
        {
            int Selected = PlayListAppendDlg->GetSelectedPlayList();
            if( Selected == -1 )
            {
                wxString PLName = PlayListAppendDlg->GetPlaylistName();
                if( PLName.IsEmpty() )
                {
                    PLName = _( "UnNamed" );
                }
                m_Db->CreateStaticPlayList( PLName, NewSongs );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, NewSongs );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                {
                    m_Db->AppendStaticPlayList( PLId, NewSongs );
                }
            }
            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongListColClicked( wxListEvent &event )
{
    int ColId = m_SongListCtrl->GetColumnId( event.m_col );
    m_Db->SetSongsOrder( ( guTRACKS_ORDER ) ColId );

    // Create the Columns
    wxArrayString ColumnNames = m_SongListCtrl->GetColumnNames();
    int CurColId;
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        CurColId = m_SongListCtrl->GetColumnId( index );
        m_SongListCtrl->SetColumnLabel( index,
            ColumnNames[ CurColId ]  + ( ( ColId == CurColId ) ? ( m_Db->GetSongsOrderDesc() ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    m_SongListCtrl->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSelectGenre( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );
    wxArrayInt * Genres = new wxArrayInt();
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Genres->Index( Tracks[ index ].m_GenreId ) == wxNOT_FOUND )
        {
            Genres->Add( Tracks[ index ].m_GenreId );
        }
    }
    SelectGenres( Genres );
    delete Genres;
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSelectArtist( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );
    wxArrayInt * Artists = new wxArrayInt();
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Artists->Index( Tracks[ index ].m_ArtistId ) == wxNOT_FOUND )
        {
            Artists->Add( Tracks[ index ].m_ArtistId );
        }
    }
    SelectArtists( Artists );
    delete Artists;
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSelectAlbum( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );
    wxArrayInt * Albums = new wxArrayInt();

    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Albums->Index( Tracks[ index ].m_AlbumId ) == wxNOT_FOUND )
        {
            Albums->Add( Tracks[ index ].m_AlbumId );
        }
    }
    SelectAlbums( Albums );
    delete Albums;
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectAlbumName( const wxString &album )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_AlbumListCtrl->SelectAlbumName( album );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectArtistName( const wxString &artist )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_ArtistListCtrl->SelectArtistName( artist );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectGenres( wxArrayInt * genres )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_GenreListCtrl->SetSelectedItems( * genres );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectArtists( wxArrayInt * artists )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_ArtistListCtrl->SetSelectedItems( * artists );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectAlbums( wxArrayInt * albums )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_AlbumListCtrl->SetSelectedItems( * albums );
}

// -------------------------------------------------------------------------------- //
bool guLibPanel::IsPanelShown( const int panelid ) const
{
    return ( m_VisiblePanels & panelid );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::ShowPanel( const int panelid, bool show )
{
    wxString PaneName;

    switch( panelid )
    {
        case guPANEL_LIBRARY_TEXTSEARCH :
            PaneName = wxT( "TextSearch" );
            break;

        case guPANEL_LIBRARY_LABELS :
            PaneName = wxT( "Labels" );
            break;

        case guPANEL_LIBRARY_GENRES :
            PaneName = wxT( "Genres" );
            break;

        case guPANEL_LIBRARY_ARTISTS :
            PaneName = wxT( "Artists" );
            break;

        case guPANEL_LIBRARY_ALBUMS :
            PaneName = wxT( "Albums" );
            break;

////        case guPANEL_LIBRARY_TRACKS :
////            PaneName = wxT( "Tracks" );
////            break;

//        case guPANEL_LIBRARY_YEARS :
//            PaneName = wxT( "Years" );
//            break;
//
//        case guPANEL_LIBRARY_RATINGS :
//            PaneName = wxT( "Rattings" );
//            break;
//
//        case guPANEL_LIBRARY_COVERBROWSER :
//            PaneName = wxT( "CoverBrowser" );
//            break;
//
//        case guPANEL_LIBRARY_COVERFLOW :
//            PaneName = wxT( "CoverFlow" );
//            break;
        default :
            return;

    }

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

// -------------------------------------------------------------------------------- //
void guLibPanel::OnPaneClose( wxAuiManagerEvent &event )
{
    wxAuiPaneInfo * PaneInfo = event.GetPane();
    wxString PaneName = PaneInfo->name;
    int CmdId = 0;

    if( PaneName == wxT( "TextSearch" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_TEXTSEARCH;
    }
    else if( PaneName == wxT( "Labels" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_LABELS;
    }
    else if( PaneName == wxT( "Genres" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_GENRES;
    }
    else if( PaneName == wxT( "Artists" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_ARTISTS;
    }
    else if( PaneName == wxT( "Albums" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_ALBUMS;
    }
//    else if( PaneName == wxT( "Tracks" ) )
//    {
//        CmdId = ID_MENU_VIEW_LIB_LABELS;
//    }

    guLogMessage( wxT( "OnPaneClose: %s  %i" ), PaneName.c_str(), CmdId );
    if( CmdId )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, CmdId );
        AddPendingEvent( evt );
    }

    event.Veto();
}

// -------------------------------------------------------------------------------- //
