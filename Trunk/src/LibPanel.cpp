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
#include "TagInfo.h"
#include "TrackEdit.h"
#include "CoverEdit.h"
#include "Images.h"

#include <wx/event.h>

#define LISTCTRL_BORDER 1

// -------------------------------------------------------------------------------- //
guLibPanel::guLibPanel( wxWindow* parent, DbLibrary * NewDb, guPlayerPanel * NewPlayerPanel )
       : wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 368,191 ), wxTAB_TRAVERSAL )
{
    wxStaticText *      SearchStaticText;
    wxStaticLine *      SearchStaticline;
    wxPanel *           SelectorPanel;
    wxPanel *           GenreLabelsPanel;
    wxPanel *           ArtistAlbumPanel;
    wxPanel *           SongListPanel;
    wxPanel *           InputTextPanel;

   	wxBoxSizer *        LibraryMainSizer;
	wxBoxSizer *        SearchSizer;
	wxBoxSizer *        SelectorSizer;
	wxBoxSizer *        GenreLabelsSizer;
	wxBoxSizer *        GenreSizer;
	wxBoxSizer *        LabelsSizer;
	wxBoxSizer *        ArtistAlbumSizer;
	wxBoxSizer *        ArtistSizer;
	wxBoxSizer *        AlbumSizer;
	wxBoxSizer *        SongListSizer;
	wxBoxSizer *        InputTextSizer;

    guConfig *  Config = ( guConfig * ) guConfig::Get();

    m_Db = NewDb;
    m_PlayerPanel = NewPlayerPanel;

    //
    //
    //
	LibraryMainSizer = new wxBoxSizer( wxVERTICAL );

	SearchSizer = new wxBoxSizer( wxHORIZONTAL );

	SearchStaticText = new wxStaticText( this, wxID_ANY, _( "Search:" ), wxDefaultPosition, wxDefaultSize, 0 );
	SearchStaticText->Wrap( -1 );
	SearchSizer->Add( SearchStaticText, 0, wxALIGN_CENTER|wxALL, 5 );

	InputTextPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	InputTextPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

	InputTextSizer = new wxBoxSizer( wxHORIZONTAL );

	m_InputTextLeftBitmap = new wxStaticBitmap( InputTextPanel, wxID_ANY, guImage( guIMAGE_INDEX_search ), wxDefaultPosition, wxDefaultSize, 0 );
	InputTextSizer->Add( m_InputTextLeftBitmap, 0, wxALL, 0 );

	m_InputTextCtrl = new wxTextCtrl( InputTextPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxNO_BORDER );
	//m_InputTextCtrl->SetBackgroundColour( wxColor( 250, 250, 250 ) );
	InputTextSizer->Add( m_InputTextCtrl, 1, wxALL, 2 );

	m_InputTextClearBitmap = new wxStaticBitmap( InputTextPanel, wxID_ANY, guImage( guIMAGE_INDEX_edit_clear ), wxDefaultPosition, wxDefaultSize, 0 );
	m_InputTextClearBitmap->Disable();
	InputTextSizer->Add( m_InputTextClearBitmap, 0, wxALL, 0 );

	InputTextPanel->SetSizer( InputTextSizer );
	InputTextPanel->Layout();
	InputTextSizer->Fit( InputTextPanel );

	SearchSizer->Add( InputTextPanel, 1, wxEXPAND | wxALL, 2 );

	LibraryMainSizer->Add( SearchSizer, 0, wxEXPAND, 2 );

	SearchStaticline = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	LibraryMainSizer->Add( SearchStaticline, 0, wxEXPAND, 5 );

	m_SongListSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
    m_SongListSplitter->SetMinimumPaneSize( 200 );

	m_SongListSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::SongListSplitterOnIdle ), NULL, this );


	SelectorPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	SelectorSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SelGenreSplitter = new wxSplitterWindow( SelectorPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
    m_SelGenreSplitter->SetMinimumPaneSize( 180 );

	m_SelGenreSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::SelGenreSplitterOnIdle ), NULL, this );

	GenreLabelsPanel = new wxPanel( m_SelGenreSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	GenreLabelsSizer = new wxBoxSizer( wxVERTICAL );

	m_GenreLabelsSplitter = new wxSplitterWindow( GenreLabelsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
    m_GenreLabelsSplitter->SetMinimumPaneSize( 75 );

	m_GenreLabelsSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::GenreLabelsSplitterOnIdle ), NULL, this );

	m_GenrePanel = new wxPanel( m_GenreLabelsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    //m_GenrePanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );

	GenreSizer = new wxBoxSizer( wxVERTICAL );

	//GenreListCtrl = new wxListCtrl( GenrePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	m_GenreListCtrl = new guGeListBox( m_GenrePanel, m_Db, _( "Genres" ) );
	GenreSizer->Add( m_GenreListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	m_GenrePanel->SetSizer( GenreSizer );
	m_GenrePanel->Layout();
	GenreSizer->Fit( m_GenrePanel );
	m_LabelsPanel = new wxPanel( m_GenreLabelsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    //m_LabelsPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	LabelsSizer = new wxBoxSizer( wxVERTICAL );

	//LabelsListCtrl = new wxListCtrl( LabelsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	m_LabelsListCtrl = new guTaListBox( m_LabelsPanel, m_Db, _( "Labels" ) );
	LabelsSizer->Add( m_LabelsListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	m_LabelsPanel->SetSizer( LabelsSizer );
	m_LabelsPanel->Layout();
	LabelsSizer->Fit( m_LabelsPanel );

	m_GenreLabelsSplitter->SplitHorizontally( m_GenrePanel, m_LabelsPanel, Config->ReadNum( wxT( "LabelSashPos" ), 145, wxT( "Positions" ) ) );
	GenreLabelsSizer->Add( m_GenreLabelsSplitter, 1, wxEXPAND, 5 );

	GenreLabelsPanel->SetSizer( GenreLabelsSizer );
	GenreLabelsPanel->Layout();
	GenreLabelsSizer->Fit( GenreLabelsPanel );
	ArtistAlbumPanel = new wxPanel( m_SelGenreSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	ArtistAlbumSizer = new wxBoxSizer( wxVERTICAL );

	m_ArtistAlbumSplitter = new wxSplitterWindow( ArtistAlbumPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_ArtistAlbumSplitter->SetMinimumPaneSize( 75 );

	m_ArtistAlbumSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::ArtistAlbumSplitterOnIdle ), NULL, this );
	m_ArtistPanel = new wxPanel( m_ArtistAlbumSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	//m_ArtistPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	ArtistSizer = new wxBoxSizer( wxVERTICAL );

	//ArtistListCtrl = new wxListCtrl( ArtistPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	m_ArtistListCtrl = new guArListBox( m_ArtistPanel, m_Db, _( "Artists" ) );
	ArtistSizer->Add( m_ArtistListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	m_ArtistPanel->SetSizer( ArtistSizer );
	m_ArtistPanel->Layout();
	ArtistSizer->Fit( m_ArtistPanel );
	m_AlbumPanel = new wxPanel( m_ArtistAlbumSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    //m_AlbumPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	AlbumSizer = new wxBoxSizer( wxVERTICAL );

	//AlbumListCtrl = new wxListCtrl( AlbumPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	m_AlbumListCtrl = new guAlListBox( m_AlbumPanel, m_Db, _( "Albums" ) );
	AlbumSizer->Add( m_AlbumListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	m_AlbumPanel->SetSizer( AlbumSizer );
	m_AlbumPanel->Layout();
	AlbumSizer->Fit( m_AlbumPanel );
	m_ArtistAlbumSplitter->SplitVertically( m_ArtistPanel, m_AlbumPanel, Config->ReadNum( wxT( "ArtistSashPos" ), 145, wxT( "Positions" ) ) );
	ArtistAlbumSizer->Add( m_ArtistAlbumSplitter, 1, wxEXPAND, 5 );

	ArtistAlbumPanel->SetSizer( ArtistAlbumSizer );
	ArtistAlbumPanel->Layout();
	ArtistAlbumSizer->Fit( ArtistAlbumPanel );
	m_SelGenreSplitter->SplitVertically( GenreLabelsPanel, ArtistAlbumPanel, Config->ReadNum( wxT( "GenreSashPos" ), 180, wxT( "Positions" ) ) );
	SelectorSizer->Add( m_SelGenreSplitter, 1, wxEXPAND, 5 );

	SelectorPanel->SetSizer( SelectorSizer );
	SelectorPanel->Layout();
	SelectorSizer->Fit( SelectorPanel );

	SongListPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    //SongListPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	SongListSizer = new wxBoxSizer( wxVERTICAL );

	//SongListCtrl = new wxListCtrl( SongListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	m_SongListCtrl = new guSoListBox( SongListPanel, m_Db, wxT( "Song" ), guLISTVIEW_COLUMN_SELECT|guLISTVIEW_COLUMN_SORTING );
	//m_SongListCtrl->ReloadItems();
	SongListSizer->Add( m_SongListCtrl, 1, wxEXPAND, 5 );

	SongListPanel->SetSizer( SongListSizer );
	SongListPanel->Layout();
	SongListSizer->Fit( SongListPanel );
	m_SongListSplitter->SplitHorizontally( SelectorPanel, SongListPanel, Config->ReadNum( wxT( "SongSashPos" ), 300, wxT( "Positions" ) ) );
	LibraryMainSizer->Add( m_SongListSplitter, 1, wxEXPAND, 5 );

	SetSizer( LibraryMainSizer );
	Layout();
	LibraryMainSizer->Fit( this );
    //
    m_UpdateLock = false;

    //
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
    m_InputTextClearBitmap->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( guLibPanel::OnSearchCancelled ), NULL, this );

    Connect( ID_GENRE_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenrePlayClicked ) );
    Connect( ID_GENRE_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreQueueClicked ) );
    Connect( ID_GENRE_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreCopyToClicked ) );

    Connect( ID_LABEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelPlayClicked ) );
    Connect( ID_LABEL_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelQueueClicked ) );
    //Connect( ID_LABEL_CLEARSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelClearSelectClicked ) );
    Connect( ID_LABEL_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelCopyToClicked ) );

    Connect( ID_ARTIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistPlayClicked ) );
    Connect( ID_ARTIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistQueueClicked ) );
    Connect( ID_ARTIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditLabelsClicked ) );
    Connect( ID_ARTIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditTracksClicked ) );
    Connect( ID_ARTIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistCopyToClicked ) );

    Connect( ID_ALBUM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumPlayClicked ) );
    Connect( ID_ALBUM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumQueueClicked ) );
    Connect( ID_ALBUM_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditLabelsClicked ) );
    Connect( ID_ALBUM_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditTracksClicked ) );
    Connect( ID_ALBUM_MANUALCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDownloadCoverClicked ) );
    Connect( ID_ALBUM_COVER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDeleteCoverClicked ) );
    Connect( ID_ALBUM_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumCopyToClicked ) );

    Connect( ID_SONG_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayClicked ) );
    Connect( ID_SONG_PLAYALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayAllClicked ) );
    Connect( ID_SONG_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueClicked ) );
    Connect( ID_SONG_ENQUEUEALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAllClicked ) );
    Connect( ID_SONG_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditLabelsClicked ) );
    Connect( ID_SONG_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditTracksClicked ) );
    Connect( ID_SONG_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongCopyToClicked ) );
    Connect( ID_SONG_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSavePlayListClicked ) );

    Connect( ID_SONG_BROWSE_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectGenre ) );
    Connect( ID_SONG_BROWSE_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectArtist ) );
    Connect( ID_SONG_BROWSE_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectAlbum ) );
}

// -------------------------------------------------------------------------------- //
guLibPanel::~guLibPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        //printf( "guLibPanel::guConfig Save\n" );
        Config->WriteNum( wxT( "ArtistSashPos" ), m_ArtistAlbumSplitter->GetSashPosition(), wxT( "Positions" ) );
        Config->WriteNum( wxT( "GenreSashPos" ), m_SelGenreSplitter->GetSashPosition(), wxT( "Positions" ) );
        Config->WriteNum( wxT( "LabelSashPos" ), m_GenreLabelsSplitter->GetSashPosition(), wxT( "Positions" ) );
        Config->WriteNum( wxT( "SongSashPos" ), m_SongListSplitter->GetSashPosition(), wxT( "Positions" ) );
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
    m_InputTextClearBitmap->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( guLibPanel::OnSearchCancelled ), NULL, this );

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
}

// -------------------------------------------------------------------------------- //
void guLibPanel::ReloadControls( wxCommandEvent &event )
{
    m_Db->LoadCache();
    m_LabelsListCtrl->ReloadItems( false );
    m_GenreListCtrl->ReloadItems( false );
    m_ArtistListCtrl->ReloadItems( false );
    m_AlbumListCtrl->ReloadItems( false );
    m_SongListCtrl->ReloadItems();
}

// -------------------------------------------------------------------------------- //
// TextSearch Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSearchActivated( wxCommandEvent& event )
{
    wxArrayString Words = guSplitWords( m_InputTextCtrl->GetLineText( 0 ) );

    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_SongListCtrl->ReloadItems();
    m_InputTextClearBitmap->Enable();
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSearchCancelled( wxMouseEvent &event ) // CLEAN SEARCH STR
{
    wxArrayString Words;
    //guLogMessage( wxT( "guLibPanel::SearchCancelled" ) );
    m_InputTextCtrl->Clear();
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_SongListCtrl->ReloadItems();
    m_InputTextClearBitmap->Disable();
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

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Artist Labels Editor" ), Labels, m_Db->GetArtistsLabels( Artists ) );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                // Update the labels in the artists files
                m_Db->UpdateArtistsLabels( Artists, LabelEditor->GetCheckedIds() );
            }
            LabelEditor->Destroy();
            m_UpdateLock = true;
            m_LabelsListCtrl->ReloadItems( false );
            m_UpdateLock = false;
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
    guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Albums Labels Editor" ),
                                         Labels, m_Db->GetAlbumsLabels( Albums ) );
    if( LabelEditor )
    {
        if( LabelEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateAlbumsLabels( Albums, LabelEditor->GetCheckedIds() );
        }
        LabelEditor->Destroy();
        m_UpdateLock = true;
        m_LabelsListCtrl->ReloadItems( false );
        m_UpdateLock = false;
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongsEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Labels;
    wxArrayInt SongIds;

    m_Db->GetLabels( &Labels, true );

    SongIds = m_SongListCtrl->GetSelectedItems();
    guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Songs Labels Editor" ),
                         Labels, m_Db->GetSongsLabels( SongIds ) );
    if( LabelEditor )
    {
        if( LabelEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongsLabels( SongIds, LabelEditor->GetCheckedIds() );
        }
        LabelEditor->Destroy();
        m_UpdateLock = true;
        m_LabelsListCtrl->ReloadItems( false );
        m_UpdateLock = false;
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
    guTrackArray Songs;
    guImagePtrArray Images;
    m_ArtistListCtrl->GetSelectedSongs( &Songs );
    if( !Songs.Count() )
        return;
    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Songs, &Images );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Songs );
            UpdateImages( Songs, Images );
            m_PlayerPanel->UpdatedTracks( &Songs );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    guImagePtrArray Images;
    m_AlbumListCtrl->GetSelectedSongs( &Songs );
    if( !Songs.Count() )
        return;
    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Songs, &Images );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Songs );
            UpdateImages( Songs, Images );
            m_PlayerPanel->UpdatedTracks( &Songs );
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
                CoverImage->SaveFile( AlbumPath + wxT( "cover.jpg" ), wxBITMAP_TYPE_JPEG );
                m_Db->SetAlbumCover( Albums[ 0 ], AlbumPath + wxT( "cover.jpg" ) );
                //AlbumListCtrl->ClearSelection();
                //Db->SetAlFilters( wxArrayInt() );
                m_AlbumListCtrl->ReloadItems( false );
                //guLogMessage( wxT( "Cover downloaded ok\n" ) );
            }
            CoverEditor->Destroy();
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
            //bool DbLibrary::GetAlbumInfo( const int AlbumId, wxString * AlbumName, wxString * ArtistName, wxString * AlbumPath )
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
    guTrackArray Songs;
    m_SongListCtrl->GetSelectedSongs( &Songs );
    guImagePtrArray Images;
    if( !Songs.Count() )
        return;
    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Songs, &Images );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            m_Db->UpdateSongs( &Songs );
            UpdateImages( Songs, Images );
            m_PlayerPanel->UpdatedTracks( &Songs );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongCopyToClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) new guTrackArray( Tracks ) );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSavePlayListClicked( wxCommandEvent &event )
{
    int index;
    int count;
    guTrackArray Tracks;
    m_SongListCtrl->GetAllSongs( &Tracks );
    if( ( count = Tracks.Count() ) )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( wxTheApp->GetTopWindow(), _( "PlayList Name: " ), _( "Enter the new playlist name" ) );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            wxArrayInt SongIds;
            for( index = 0; index < count; index++ )
            {
                SongIds.Add( Tracks[ index ].m_SongId );
            }
            m_Db->CreateStaticPlayList( EntryDialog->GetValue(), SongIds );

            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongListColClicked( wxListEvent &event )
{
    int ColId = m_SongListCtrl->GetColumnId( event.m_col );
    m_Db->SetSongsOrder( ( guTRACKS_ORDER ) ColId );

    // Create the Columns
    int CurColId;
    int index;
    int count = sizeof( guSONGS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        CurColId = m_SongListCtrl->GetColumnId( index );
        m_SongListCtrl->SetColumnLabel( index,
            guSONGS_COLUMN_NAMES[ CurColId ]  + ( ( ColId == CurColId ) ? ( m_Db->GetSongsOrderDesc() ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    m_SongListCtrl->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSelectGenre( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );
    wxArrayInt Genres;
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Genres.Index( Tracks[ index ].m_GenreId ) == wxNOT_FOUND )
        {
            Genres.Add( Tracks[ index ].m_GenreId );
        }
    }
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_GenreListCtrl->SetSelectedItems( Genres );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSelectArtist( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );
    wxArrayInt Artists;
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Artists.Index( Tracks[ index ].m_ArtistId ) == wxNOT_FOUND )
        {
            Artists.Add( Tracks[ index ].m_ArtistId );
        }
    }
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_ArtistListCtrl->SetSelectedItems( Artists );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongSelectAlbum( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );
    wxArrayInt Albums;
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Albums.Index( Tracks[ index ].m_AlbumId ) == wxNOT_FOUND )
        {
            Albums.Add( Tracks[ index ].m_AlbumId );
        }
    }
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_AlbumListCtrl->SetSelectedItems( Albums );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumNameDClicked( wxCommandEvent &event )
{
    wxString * AlbumName = ( wxString * ) event.GetClientData();
    if( AlbumName )
    {
        // Reset all controls
        wxArrayString Words;
        m_UpdateLock = true;
        m_Db->SetTeFilters( Words );
        m_LabelsListCtrl->ReloadItems();
        m_GenreListCtrl->ReloadItems();
        m_ArtistListCtrl->ReloadItems();
        m_UpdateLock = false;
        m_AlbumListCtrl->ReloadItems();
        m_AlbumListCtrl->SelectAlbumName( * AlbumName );
        m_AlbumListCtrl->SetFocus();
        //m_SongListCtrl->ReloadItems();

        delete AlbumName;
        //m_SongListCtrl->ReloadItems();
         // Reset all controls

    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistNameDClicked( wxCommandEvent &event )
{
    wxString * ArtistName = ( wxString * ) event.GetClientData();
    if( ArtistName )
    {
        // Reset all controls
        wxArrayString Words;
        m_UpdateLock = true;
        m_Db->SetTeFilters( Words );
        m_LabelsListCtrl->ReloadItems();
        m_GenreListCtrl->ReloadItems();
        m_ArtistListCtrl->ReloadItems();
        m_UpdateLock = false;
        m_ArtistListCtrl->SelectArtistName( * ArtistName );
        m_ArtistListCtrl->SetFocus();
        delete ArtistName;
    }
}

// -------------------------------------------------------------------------------- //
// OnIdle Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::SongListSplitterOnIdle( wxIdleEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_SongListSplitter->SetSashPosition( Config->ReadNum( wxT( "SongSashPos" ), 300, wxT( "Positions" ) ) );
    m_SongListSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::SongListSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelGenreSplitterOnIdle( wxIdleEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_SelGenreSplitter->SetSashPosition( Config->ReadNum( wxT( "GenreSashPos" ), 180, wxT( "Positions" ) ) );
    m_SelGenreSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::SelGenreSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::GenreLabelsSplitterOnIdle( wxIdleEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_GenreLabelsSplitter->SetSashPosition( Config->ReadNum( wxT( "LabelSashPos" ), 145, wxT( "Positions" ) ) );
    m_GenreLabelsSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::GenreLabelsSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::ArtistAlbumSplitterOnIdle( wxIdleEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_ArtistAlbumSplitter->SetSashPosition( Config->ReadNum( wxT( "ArtistSashPos" ), 145, wxT( "Positions" ) ) );
    m_ArtistAlbumSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guLibPanel::ArtistAlbumSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
