// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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

#include "AuiDockArt.h"
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

#define     LISTCTRL_BORDER 1

#define     guPANEL_TIMER_SELECTION         1
#define     guPANEL_TIMER_TEXTSEARCH        2

#define     guPANEL_TIMER_SELCHANGED        50
#define     guPANEL_TIMER_TEXTCHANGED       500


// -------------------------------------------------------------------------------- //
guLibPanel::guLibPanel( wxWindow* parent, guDbLibrary * NewDb, guPlayerPanel * NewPlayerPanel ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 368,191 ), wxTAB_TRAVERSAL ),
    m_SelChangedTimer( this, guPANEL_TIMER_SELECTION ),
    m_TextChangedTimer( this, guPANEL_TIMER_TEXTSEARCH )
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


	//
	// Years
	//
	wxPanel * YearPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * YearsSizer = new wxBoxSizer( wxVERTICAL );

	m_YearListCtrl = new guYeListBox( YearPanel, m_Db, _( "Years" ) );
	YearsSizer->Add( m_YearListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	YearPanel->SetSizer( YearsSizer );
	YearPanel->Layout();
	YearsSizer->Fit( YearPanel );

    m_AuiManager.AddPane( YearPanel, wxAuiPaneInfo().Name( wxT( "Years" ) ).Caption( _( "Years" ) ).
            MinSize( 50, 50 ).Row( 2 ).
            Position( 0 ).Hide().
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );

	//
	// Ratings
	//
	wxPanel * RatingPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * RatingSizer = new wxBoxSizer( wxVERTICAL );

	m_RatingListCtrl = new guRaListBox( RatingPanel, m_Db, _( "Ratings" ) );
	RatingSizer->Add( m_RatingListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	RatingPanel->SetSizer( RatingSizer );
	RatingPanel->Layout();
	RatingSizer->Fit( RatingPanel );

    m_AuiManager.AddPane( RatingPanel, wxAuiPaneInfo().Name( wxT( "Ratings" ) ).Caption( _( "Ratings" ) ).
            MinSize( 50, 50 ).Row( 2 ).
            Position( 1 ).Hide().
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );

	//
	// PlayCounts
	//
	wxPanel * PlayCountPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * PlayCountSizer = new wxBoxSizer( wxVERTICAL );

	m_PlayCountListCtrl = new guPcListBox( PlayCountPanel, m_Db, _( "Play Counts" ) );
	PlayCountSizer->Add( m_PlayCountListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	PlayCountPanel->SetSizer( PlayCountSizer );
	PlayCountPanel->Layout();
	PlayCountSizer->Fit( PlayCountPanel );

    m_AuiManager.AddPane( PlayCountPanel, wxAuiPaneInfo().Name( wxT( "PlayCounts" ) ).Caption( _( "Play Counts" ) ).
            MinSize( 50, 50 ).Row( 2 ).
            Position( 2 ).Hide().
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );

	//
	// Composers
	//
	wxPanel * ComposerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * ComposerSizer = new wxBoxSizer( wxVERTICAL );

	m_ComposerListCtrl = new guCoListBox( ComposerPanel, m_Db, _( "Composers" ) );
	ComposerSizer->Add( m_ComposerListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	ComposerPanel->SetSizer( ComposerSizer );
	ComposerPanel->Layout();
	ComposerSizer->Fit( ComposerPanel );

    m_AuiManager.AddPane( ComposerPanel, wxAuiPaneInfo().Name( wxT( "Composers" ) ).Caption( _( "Composers" ) ).
            MinSize( 50, 50 ).Row( 2 ).
            Position( 3 ).Hide().
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );

	//
	// Album Artists
	//
	wxPanel * AlbumArtistPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * AlbumArtistSizer = new wxBoxSizer( wxVERTICAL );

	m_AlbumArtistListCtrl = new guAAListBox( AlbumArtistPanel, m_Db, _( "Album Artist" ) );
	AlbumArtistSizer->Add( m_AlbumArtistListCtrl, 1, wxALL|wxEXPAND, LISTCTRL_BORDER );

	AlbumArtistPanel->SetSizer( AlbumArtistSizer );
	AlbumArtistPanel->Layout();
	AlbumArtistSizer->Fit( AlbumArtistPanel );

    m_AuiManager.AddPane( AlbumArtistPanel, wxAuiPaneInfo().Name( wxT( "AlbumArtists" ) ).Caption( _( "Album Artist" ) ).
            MinSize( 50, 50 ).Row( 2 ).
            Position( 4 ).Hide().
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );

	//
	// Songs
	//
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


	Connect( guPANEL_TIMER_SELECTION, wxEVT_TIMER, wxTimerEventHandler( guLibPanel::OnSelChangedTimer ), NULL, this );
	Connect( guPANEL_TIMER_TEXTSEARCH, wxEVT_TIMER, wxTimerEventHandler( guLibPanel::OnTextChangedTimer ), NULL, this );
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

    m_YearListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnYearListSelected ), NULL, this );
    m_YearListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnYearListActivated ), NULL, this );

    m_RatingListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnRatingListSelected ), NULL, this );
    m_RatingListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnRatingListActivated ), NULL, this );

    m_PlayCountListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnPlayCountListSelected ), NULL, this );
    m_PlayCountListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnPlayCountListActivated ), NULL, this );

    m_ComposerListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnComposerListSelected ), NULL, this );
    m_ComposerListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnComposerListActivated ), NULL, this );

    m_AlbumArtistListCtrl->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnAlbumArtistListSelected ), NULL, this );
    m_AlbumArtistListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnAlbumArtistListActivated ), NULL, this );

    m_SongListCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnSongListActivated ), NULL, this );
    m_SongListCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guLibPanel::OnSongListColClicked ), NULL, this );

    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guLibPanel::OnSearchSelected ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    //m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guLibPanel::OnSearchCancelled ), NULL, this );

    Connect( ID_GENRE_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenrePlayClicked ), NULL, this );
    Connect( ID_GENRE_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreQueueClicked ), NULL, this );
    Connect( ID_GENRE_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreQueueAsNextClicked ), NULL, this );
    Connect( ID_GENRE_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreCopyToClicked ), NULL, this );

    Connect( ID_LABEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelPlayClicked ), NULL, this );
    Connect( ID_LABEL_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelQueueClicked ), NULL, this );
    Connect( ID_LABEL_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelQueueAsNextClicked ), NULL, this );
    //Connect( ID_LABEL_CLEARSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelClearSelectClicked ) );
    Connect( ID_LABEL_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelCopyToClicked ), NULL, this );

    Connect( ID_ARTIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistPlayClicked ), NULL, this );
    Connect( ID_ARTIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistQueueClicked ), NULL, this );
    Connect( ID_ARTIST_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistQueueAsNextClicked ), NULL, this );
    Connect( ID_ARTIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditLabelsClicked ), NULL, this );
    Connect( ID_ARTIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditTracksClicked ), NULL, this );
    Connect( ID_ARTIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistCopyToClicked ), NULL, this );

    Connect( ID_ALBUM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumPlayClicked ), NULL, this );
    Connect( ID_ALBUM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumQueueClicked ), NULL, this );
    Connect( ID_ALBUM_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumQueueAsNextClicked ), NULL, this );
    Connect( ID_ALBUM_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditLabelsClicked ), NULL, this );
    Connect( ID_ALBUM_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditTracksClicked ), NULL, this );
    Connect( ID_ALBUM_MANUALCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDownloadCoverClicked ), NULL, this );
    Connect( ID_ALBUM_SELECT_COVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumSelectCoverClicked ), NULL, this );
    Connect( ID_ALBUM_COVER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDeleteCoverClicked ), NULL, this );
    Connect( ID_ALBUM_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumCopyToClicked ), NULL, this );

    Connect( ID_YEAR_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListPlayClicked ), NULL, this );
    Connect( ID_YEAR_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListQueueClicked ), NULL, this );
    Connect( ID_YEAR_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListQueueAsNextClicked ), NULL, this );
    Connect( ID_YEAR_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListEditTracksClicked ), NULL, this );
    Connect( ID_YEAR_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListCopyToClicked ), NULL, this );

    Connect( ID_RATING_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListPlayClicked ), NULL, this );
    Connect( ID_RATING_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListQueueClicked ), NULL, this );
    Connect( ID_RATING_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListQueueAsNextClicked ), NULL, this );
    Connect( ID_RATING_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListEditTracksClicked ), NULL, this );
    Connect( ID_RATING_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListCopyToClicked ), NULL, this );

    Connect( ID_PLAYCOUNT_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListPlayClicked ), NULL, this );
    Connect( ID_PLAYCOUNT_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListQueueClicked ), NULL, this );
    Connect( ID_PLAYCOUNT_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListQueueAsNextClicked ), NULL, this );
    Connect( ID_PLAYCOUNT_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListEditTracksClicked ), NULL, this );
    Connect( ID_PLAYCOUNT_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListCopyToClicked ), NULL, this );

    Connect( ID_COMPOSER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListPlayClicked ), NULL, this );
    Connect( ID_COMPOSER_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListQueueClicked ), NULL, this );
    Connect( ID_COMPOSER_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListQueueAsNextClicked ), NULL, this );
    Connect( ID_COMPOSER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListEditTracksClicked ), NULL, this );
    Connect( ID_COMPOSER_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListCopyToClicked ), NULL, this );

    Connect( ID_ALBUMARTIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListPlayClicked ), NULL, this );
    Connect( ID_ALBUMARTIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListQueueClicked ), NULL, this );
    Connect( ID_ALBUMARTIST_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListQueueAsNextClicked ), NULL, this );
    Connect( ID_ALBUMARTIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListEditTracksClicked ), NULL, this );
    Connect( ID_ALBUMARTIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListCopyToClicked ), NULL, this );

    Connect( ID_SONG_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayClicked ), NULL, this );
    Connect( ID_SONG_PLAYALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayAllClicked ), NULL, this );
    Connect( ID_SONG_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueClicked ), NULL, this );
    Connect( ID_SONG_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAsNextClicked ), NULL, this );
    Connect( ID_SONG_ENQUEUEALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAllClicked ), NULL, this );
    Connect( ID_SONG_ENQUEUEALL_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAllAsNextClicked ), NULL, this );
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

	Disconnect( guPANEL_TIMER_SELECTION, wxEVT_TIMER, wxTimerEventHandler( guLibPanel::OnSelChangedTimer ), NULL, this );
	Disconnect( guPANEL_TIMER_TEXTSEARCH, wxEVT_TIMER, wxTimerEventHandler( guLibPanel::OnTextChangedTimer ), NULL, this );
    //
    m_GenreListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnGenreListSelected ), NULL, this );
    //m_GenreListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnGenreListSelected ), NULL, this );
    m_GenreListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnGenreListActivated ), NULL, this );

    m_LabelsListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnLabelListSelected ), NULL, this );
    //m_LabelsListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnLabelListSelected ), NULL, this );
    m_LabelsListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnLabelListActivated ), NULL, this );

    m_ArtistListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnArtistListSelected ), NULL, this );
    //m_ArtistListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guLibPanel::OnArtistListSelected ), NULL, this );
    m_ArtistListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnArtistListActivated ), NULL, this );

    m_AlbumListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnAlbumListSelected ), NULL, this );
    m_AlbumListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnAlbumListActivated ), NULL, this );

    m_YearListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnYearListSelected ), NULL, this );
    m_YearListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnYearListActivated ), NULL, this );

    m_RatingListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnRatingListSelected ), NULL, this );
    m_RatingListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnRatingListActivated ), NULL, this );

    m_PlayCountListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnPlayCountListSelected ), NULL, this );
    m_PlayCountListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnPlayCountListActivated ), NULL, this );

    m_ComposerListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnComposerListSelected ), NULL, this );
    m_ComposerListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnComposerListActivated ), NULL, this );

    m_AlbumArtistListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guLibPanel::OnAlbumArtistListSelected ), NULL, this );
    m_AlbumArtistListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnAlbumArtistListActivated ), NULL, this );

    m_SongListCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guLibPanel::OnSongListActivated ), NULL, this );
    m_SongListCtrl->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guLibPanel::OnSongListColClicked ), NULL, this );

    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guLibPanel::OnSearchSelected ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    //m_InputTextCtrl->Disconnect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guLibPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guLibPanel::OnSearchCancelled ), NULL, this );

    Disconnect( ID_GENRE_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenrePlayClicked ), NULL, this );
    Disconnect( ID_GENRE_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreQueueClicked ), NULL, this );
    Disconnect( ID_GENRE_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreQueueAsNextClicked ), NULL, this );
    Disconnect( ID_GENRE_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnGenreCopyToClicked ), NULL, this );

    Disconnect( ID_LABEL_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelPlayClicked ), NULL, this );
    Disconnect( ID_LABEL_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelQueueClicked ), NULL, this );
    Disconnect( ID_LABEL_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelQueueAsNextClicked ), NULL, this );
    //Disconnect( ID_LABEL_CLEARSELECTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelClearSelectClicked ) );
    Disconnect( ID_LABEL_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnLabelCopyToClicked ), NULL, this );

    Disconnect( ID_ARTIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistPlayClicked ), NULL, this );
    Disconnect( ID_ARTIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistQueueClicked ), NULL, this );
    Disconnect( ID_ARTIST_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistQueueAsNextClicked ), NULL, this );
    Disconnect( ID_ARTIST_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditLabelsClicked ), NULL, this );
    Disconnect( ID_ARTIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistEditTracksClicked ), NULL, this );
    Disconnect( ID_ARTIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnArtistCopyToClicked ), NULL, this );

    Disconnect( ID_ALBUM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumPlayClicked ), NULL, this );
    Disconnect( ID_ALBUM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumQueueClicked ), NULL, this );
    Disconnect( ID_ALBUM_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumQueueAsNextClicked ), NULL, this );
    Disconnect( ID_ALBUM_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditLabelsClicked ), NULL, this );
    Disconnect( ID_ALBUM_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumEditTracksClicked ), NULL, this );
    Disconnect( ID_ALBUM_MANUALCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDownloadCoverClicked ), NULL, this );
    Disconnect( ID_ALBUM_SELECT_COVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumSelectCoverClicked ), NULL, this );
    Disconnect( ID_ALBUM_COVER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumDeleteCoverClicked ), NULL, this );
    Disconnect( ID_ALBUM_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumCopyToClicked ), NULL, this );

    Disconnect( ID_YEAR_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListPlayClicked ), NULL, this );
    Disconnect( ID_YEAR_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListQueueClicked ), NULL, this );
    Disconnect( ID_YEAR_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListQueueAsNextClicked ), NULL, this );
    Disconnect( ID_YEAR_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListEditTracksClicked ), NULL, this );
    Disconnect( ID_YEAR_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnYearListCopyToClicked ), NULL, this );

    Disconnect( ID_RATING_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListPlayClicked ), NULL, this );
    Disconnect( ID_RATING_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListQueueClicked ), NULL, this );
    Disconnect( ID_RATING_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListQueueAsNextClicked ), NULL, this );
    Disconnect( ID_RATING_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListEditTracksClicked ), NULL, this );
    Disconnect( ID_RATING_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnRatingListCopyToClicked ), NULL, this );

    Disconnect( ID_PLAYCOUNT_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListPlayClicked ), NULL, this );
    Disconnect( ID_PLAYCOUNT_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListQueueClicked ), NULL, this );
    Disconnect( ID_PLAYCOUNT_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListQueueAsNextClicked ), NULL, this );
    Disconnect( ID_PLAYCOUNT_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListEditTracksClicked ), NULL, this );
    Disconnect( ID_PLAYCOUNT_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnPlayCountListCopyToClicked ), NULL, this );

    Disconnect( ID_COMPOSER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListPlayClicked ), NULL, this );
    Disconnect( ID_COMPOSER_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListQueueClicked ), NULL, this );
    Disconnect( ID_COMPOSER_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListQueueAsNextClicked ), NULL, this );
    Disconnect( ID_COMPOSER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListEditTracksClicked ), NULL, this );
    Disconnect( ID_COMPOSER_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnComposerListCopyToClicked ), NULL, this );

    Disconnect( ID_ALBUMARTIST_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListPlayClicked ), NULL, this );
    Disconnect( ID_ALBUMARTIST_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListQueueClicked ), NULL, this );
    Disconnect( ID_ALBUMARTIST_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListQueueAsNextClicked ), NULL, this );
    Disconnect( ID_ALBUMARTIST_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListEditTracksClicked ), NULL, this );
    Disconnect( ID_ALBUMARTIST_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnAlbumArtistListCopyToClicked ), NULL, this );

    Disconnect( ID_SONG_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayClicked ), NULL, this );
    Disconnect( ID_SONG_PLAYALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongPlayAllClicked ), NULL, this );
    Disconnect( ID_SONG_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueClicked ), NULL, this );
    Disconnect( ID_SONG_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAsNextClicked ), NULL, this );
    Disconnect( ID_SONG_ENQUEUEALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAllClicked ), NULL, this );
    Disconnect( ID_SONG_ENQUEUEALL_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongQueueAllAsNextClicked ), NULL, this );
    Disconnect( ID_SONG_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditLabelsClicked ), NULL, this );
    Disconnect( ID_SONG_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongsEditTracksClicked ), NULL, this );
    Disconnect( ID_SONG_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongCopyToClicked ), NULL, this );
    Disconnect( ID_SONG_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSavePlayListClicked ), NULL, this );

    Disconnect( ID_SONG_BROWSE_GENRE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectGenre ), NULL, this );
    Disconnect( ID_SONG_BROWSE_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectArtist ), NULL, this );
    Disconnect( ID_SONG_BROWSE_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLibPanel::OnSongSelectAlbum ), NULL, this );

    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guLibPanel::OnPaneClose ), NULL, this );

    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guLibPanel::ReloadControls( wxCommandEvent &event )
{
    //guLogMessage( wxT( "ReloadControls..." ) );
    m_Db->LoadCache();
    m_UpdateLock = true;
    m_LabelsListCtrl->ReloadItems( false );
    m_GenreListCtrl->ReloadItems( false );
    m_AlbumArtistListCtrl->ReloadItems( false );
    m_ArtistListCtrl->ReloadItems( false );
    m_ComposerListCtrl->ReloadItems( false );
    m_AlbumListCtrl->ReloadItems( false );
    m_YearListCtrl->ReloadItems( false );
    m_RatingListCtrl->ReloadItems( false );
    m_PlayCountListCtrl->ReloadItems( false );
    m_SongListCtrl->ReloadItems();
    m_UpdateLock = false;
}

// -------------------------------------------------------------------------------- //
// TextSearch Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSearchActivated( wxCommandEvent& event )
{
    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();
    m_TextChangedTimer.Start( guPANEL_TIMER_TEXTCHANGED, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSearchCancelled( wxCommandEvent &event ) // CLEAN SEARCH STR
{
    m_InputTextCtrl->Clear();
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSearchSelected( wxCommandEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
        {
            OnSongQueueAllClicked( event );
        }
        else
        {
            OnSongPlayAllClicked( event );
        }
    }
}

// -------------------------------------------------------------------------------- //
// GenreListBox Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnGenreListSelected( wxListEvent &event )
{
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_GENRES;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    //wxLongLong time = wxGetLocalTimeMillis();
//    m_Db->SetGeFilters( m_GenreListCtrl->GetSelectedItems(), m_UpdateLock );
//
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_ArtistListCtrl->ReloadItems();
//        m_AlbumListCtrl->ReloadItems();
//        m_YearListCtrl->ReloadItems();
//        m_RatingListCtrl->ReloadItems( false );
//        m_SongListCtrl->ReloadItems();
//        m_UpdateLock = false;
//    }
//    //time = wxGetLocalTimeMillis() - time;;
//    //guLogWarning( wxT( "Genre Time : %u ms" ), time.GetLo() );
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
void guLibPanel::OnGenreQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_GenreListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
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
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_LABELS;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    m_Db->SetLaFilters( m_LabelsListCtrl->GetSelectedItems(), m_UpdateLock );
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_GenreListCtrl->ReloadItems();
//        m_ArtistListCtrl->ReloadItems();
//        m_AlbumListCtrl->ReloadItems();
//        m_YearListCtrl->ReloadItems();
//        m_RatingListCtrl->ReloadItems( false );
//        m_SongListCtrl->ReloadItems();
//        //
//        //
//        m_UpdateLock = false;
//    }
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

// -------------------------------------------------------------------------------- //
void guLibPanel::OnLabelQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_LabelsListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
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
    m_GenreListCtrl->ReloadItems( false );
    m_AlbumArtistListCtrl->ReloadItems( false );
    m_ArtistListCtrl->ReloadItems( false );
    m_ComposerListCtrl->ReloadItems( false );
    m_AlbumListCtrl->ReloadItems( false );
    m_RatingListCtrl->ReloadItems( false );
    m_YearListCtrl->ReloadItems( false );
    m_PlayCountListCtrl->ReloadItems( false );
    m_SongListCtrl->ReloadItems( true );
    m_UpdateLock = false;
}

// -------------------------------------------------------------------------------- //
// ArtistListBox Events
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistListSelected( wxListEvent &event )
{
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_ARTISTS;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    m_Db->SetArFilters( m_ArtistListCtrl->GetSelectedItems(), m_UpdateLock );
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_AlbumListCtrl->ReloadItems();
//        m_YearListCtrl->ReloadItems();
//        m_RatingListCtrl->ReloadItems( false );
//        m_SongListCtrl->ReloadItems();
//        m_UpdateLock = false;
//    }
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
void guLibPanel::OnArtistQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_ArtistListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnArtistEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Artists;
    m_ArtistListCtrl->GetSelectedItems( &Artists );
    if( Artists.Count() )
    {
        guArrayListItems LabelSets = m_Db->GetArtistsLabels( m_ArtistListCtrl->GetSelectedItems() );

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Artist Labels Editor" ), false, &Artists, &LabelSets );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                // Update the labels in the artists files
                m_Db->UpdateArtistsLabels( LabelSets );
            }

            UpdateLabels();

            LabelEditor->Destroy();
        }
    }
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        TrackEditor->Destroy();
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
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_ALBUMS;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    m_Db->SetAlFilters( m_AlbumListCtrl->GetSelectedItems(), m_UpdateLock );
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_YearListCtrl->ReloadItems();
//        m_RatingListCtrl->ReloadItems( false );
//        m_SongListCtrl->ReloadItems();
//        m_UpdateLock = false;
//    }
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
void guLibPanel::OnAlbumQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_AlbumListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Albums;
    m_AlbumListCtrl->GetSelectedItems( &Albums );
    if( Albums.Count() )
    {
        guArrayListItems LabelSets = m_Db->GetAlbumsLabels( m_AlbumListCtrl->GetSelectedItems() );

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Albums Labels Editor" ),
                            false, &Albums, &LabelSets );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                m_Db->UpdateAlbumsLabels( LabelSets );
            }
            LabelEditor->Destroy();

            UpdateLabels();
        }
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
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
                    guConfig * Config = ( guConfig * ) guConfig::Get();
                    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
                    wxString CoverName = AlbumPath + ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );
                    CoverImage->SaveFile( CoverName, wxBITMAP_TYPE_JPEG );
                    m_Db->SetAlbumCover( Albums[ 0 ], CoverName );
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
                    guConfig * Config = ( guConfig * ) guConfig::Get();
                    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
                    wxString CoverName = SelCoverFile->GetAlbumPath() + ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );

                    wxURI Uri( CoverFile );
                    if( Uri.IsReference() )
                    {
                        wxImage CoverImage( CoverFile );
                        if( CoverImage.IsOk() )
                        {
                            if( CoverImage.SaveFile( CoverName, wxBITMAP_TYPE_JPEG ) )
                            {
                                m_Db->SetAlbumCover( Albums[ 0 ], CoverName );
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
                        if( DownloadImage( CoverFile, CoverName ) )
                        {
                            m_Db->SetAlbumCover( Albums[ 0 ], CoverName );
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
void guLibPanel::OnSongQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetSelectedSongs( &Songs );
    if( !Songs.Count() )
    {
        m_SongListCtrl->GetAllSongs( &Songs );
    }
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongQueueAllClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetAllSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongQueueAllAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_SongListCtrl->GetAllSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSongsEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Tracks;
    m_SongListCtrl->GetSelectedItems( &Tracks, false );
    if( Tracks.Count() )
    {
        guArrayListItems LabelSets = m_Db->GetSongsLabels( m_SongListCtrl->GetSelectedItems() );

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Tracks Labels Editor" ), false, &Tracks, &LabelSets );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                // Update the labels in the files
                m_Db->UpdateSongsLabels( LabelSets );
            }

            UpdateLabels();

            LabelEditor->Destroy();
        }
    }
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
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
void guLibPanel::SelectTrack( const int trackid )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, false  );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_SongListCtrl->ReloadItems();
    m_SongListCtrl->SetSelection( m_Db->GetTrackIndex( trackid ) );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectAlbum( const int albumid )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, false );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_AlbumListCtrl->SetSelection( m_AlbumListCtrl->FindAlbum( albumid ) );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectArtist( const int artistid )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, false );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_ArtistListCtrl->SetSelection( m_ArtistListCtrl->FindArtist( artistid ) );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectYear( const int year )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, false );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_YearListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_YearListCtrl->SetSelection( m_YearListCtrl->FindYear( year ) );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectAlbumName( const wxString &album )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, m_UpdateLock );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_AlbumListCtrl->SelectAlbumName( album );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectArtistName( const wxString &artist )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, m_UpdateLock );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_ArtistListCtrl->SelectArtistName( artist );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectGenres( wxArrayInt * genres )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, m_UpdateLock );
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
    m_Db->SetTeFilters( Words, m_UpdateLock );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_ArtistListCtrl->SetSelectedItems( * artists );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::SelectAlbums( wxArrayInt * albums )
{
    wxArrayString Words;
    m_UpdateLock = true;
    m_Db->SetTeFilters( Words, m_UpdateLock );
    m_LabelsListCtrl->ReloadItems();
    m_GenreListCtrl->ReloadItems();
    m_AlbumArtistListCtrl->ReloadItems();
    m_ArtistListCtrl->ReloadItems();
    m_ComposerListCtrl->ReloadItems();
    m_AlbumListCtrl->ReloadItems();
    m_UpdateLock = false;
    m_AlbumListCtrl->SetSelectedItems( * albums );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnYearListSelected( wxListEvent &event )
{
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_YEARS;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    m_Db->SetYeFilters( m_YearListCtrl->GetSelectedItems() );
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_SongListCtrl->ReloadItems();
//        m_UpdateLock = false;
//    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnYearListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_YearListCtrl->GetSelectedSongs( &Songs );
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
void guLibPanel::OnYearListPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_YearListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnYearListQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_YearListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnYearListQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_YearListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnYearListEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    m_YearListCtrl->GetSelectedSongs( &Tracks );
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnYearListCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_YearListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

//
// Rating List Box
// -------------------------------------------------------------------------------- //
void guLibPanel::OnRatingListSelected( wxListEvent &event )
{
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_RATINGS;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    m_Db->SetRaFilters( m_RatingListCtrl->GetSelectedItems() );
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_SongListCtrl->ReloadItems();
//        m_UpdateLock = false;
//    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnRatingListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_RatingListCtrl->GetSelectedSongs( &Songs );
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
void guLibPanel::OnRatingListPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_RatingListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnRatingListQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_RatingListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnRatingListQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_RatingListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnRatingListEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    m_RatingListCtrl->GetSelectedSongs( &Tracks );
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnRatingListCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_RatingListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

//
// PlayCount List Box
// -------------------------------------------------------------------------------- //
void guLibPanel::OnPlayCountListSelected( wxListEvent &event )
{
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_PLAYCOUNT;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    m_Db->SetRaFilters( m_PlayCountListCtrl->GetSelectedItems() );
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_SongListCtrl->ReloadItems();
//        m_UpdateLock = false;
//    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnPlayCountListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_PlayCountListCtrl->GetSelectedSongs( &Songs );
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
void guLibPanel::OnPlayCountListPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_PlayCountListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnPlayCountListQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_PlayCountListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnPlayCountListQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_PlayCountListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnPlayCountListEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    m_PlayCountListCtrl->GetSelectedSongs( &Tracks );
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnPlayCountListCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_PlayCountListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

//
// Composers List Box
// -------------------------------------------------------------------------------- //
void guLibPanel::OnComposerListSelected( wxListEvent &event )
{
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_COMPOSERS;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
//    m_Db->SetRaFilters( m_ComposerListCtrl->GetSelectedItems() );
//    if( !m_UpdateLock )
//    {
//        m_UpdateLock = true;
//        m_SongListCtrl->ReloadItems();
//        m_UpdateLock = false;
//    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnComposerListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_ComposerListCtrl->GetSelectedSongs( &Songs );
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
void guLibPanel::OnComposerListPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_ComposerListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnComposerListQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_ComposerListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnComposerListQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_ComposerListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnComposerListEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    m_ComposerListCtrl->GetSelectedSongs( &Tracks );
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnComposerListCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_ComposerListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

//
// Album Artists List Box
// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumArtistListSelected( wxListEvent &event )
{
    if( m_UpdateLock )
        return;
    m_SelChangedObject = guPANEL_LIBRARY_ALBUMARTISTS;
    if( m_SelChangedTimer.IsRunning() )
        m_SelChangedTimer.Stop();
    m_SelChangedTimer.Start( guPANEL_TIMER_SELCHANGED, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumArtistListActivated( wxListEvent &event )
{
    guTrackArray Songs;
    m_AlbumArtistListCtrl->GetSelectedSongs( &Songs );
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
void guLibPanel::OnAlbumArtistListPlayClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_AlbumArtistListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->SetPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumArtistListQueueClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_AlbumArtistListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumArtistListQueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    m_AlbumArtistListCtrl->GetSelectedSongs( &Songs );
    m_PlayerPanel->AddToPlayList( Songs, true, true );
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumArtistListEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    m_AlbumArtistListCtrl->GetSelectedSongs( &Tracks );
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
            ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnAlbumArtistListCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_AlbumArtistListCtrl->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

//
//
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

        case guPANEL_LIBRARY_YEARS :
            PaneName = wxT( "Years" );
            break;

        case guPANEL_LIBRARY_RATINGS :
            PaneName = wxT( "Ratings" );
            break;

        case guPANEL_LIBRARY_PLAYCOUNT :
            PaneName = wxT( "PlayCounts" );
            break;

        case guPANEL_LIBRARY_COMPOSERS :
            PaneName = wxT( "Composers" );
            break;

        case guPANEL_LIBRARY_ALBUMARTISTS :
            PaneName = wxT( "AlbumArtists" );
            break;

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
    else if( PaneName == wxT( "Composers" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_COMPOSERS;
    }
    else if( PaneName == wxT( "AlbumArtists" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_ALBUMARTISTS;
    }
    else if( PaneName == wxT( "Albums" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_ALBUMS;
    }
    else if( PaneName == wxT( "Years" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_YEARS;
    }
    else if( PaneName == wxT( "Ratings" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_RATINGS;
    }
    else if( PaneName == wxT( "PlayCounts" ) )
    {
        CmdId = ID_MENU_VIEW_LIB_PLAYCOUNT;
    }

    guLogMessage( wxT( "OnPaneClose: %s  %i" ), PaneName.c_str(), CmdId );
    if( CmdId )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, CmdId );
        AddPendingEvent( evt );
    }

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnSelChangedTimer( wxTimerEvent &event )
{
    DoSelectionChanged();
    m_SelChangedObject = 0;
}

// -------------------------------------------------------------------------------- //
void guLibPanel::OnTextChangedTimer( wxTimerEvent &event )
{
    wxString SearchString = m_InputTextCtrl->GetLineText( 0 );
    if( !SearchString.IsEmpty() )
    {
        if( SearchString.Length() > 1 )
        {
            wxArrayString Words = guSplitWords( SearchString );

            m_Db->SetTeFilters( Words, m_UpdateLock );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_LabelsListCtrl->ReloadItems();
                m_GenreListCtrl->ReloadItems();
                m_AlbumArtistListCtrl->ReloadItems();
                m_ArtistListCtrl->ReloadItems();
                m_ComposerListCtrl->ReloadItems();
                m_AlbumListCtrl->ReloadItems();
                m_YearListCtrl->ReloadItems();
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            m_InputTextCtrl->ShowCancelButton( true );
        }
    }
    else
    {
        wxArrayString Words;
        //guLogMessage( wxT( "guLibPanel::SearchCancelled" ) );
        //m_InputTextCtrl->Clear();
        m_UpdateLock = true;
        m_Db->SetTeFilters( Words, m_UpdateLock );
    //    if( !m_UpdateLock )
    //    {
            m_LabelsListCtrl->ReloadItems( false );
            m_GenreListCtrl->ReloadItems( false );
            m_AlbumArtistListCtrl->ReloadItems();
            m_ArtistListCtrl->ReloadItems( false );
            m_ComposerListCtrl->ReloadItems( false );
            m_AlbumListCtrl->ReloadItems( false );
            m_YearListCtrl->ReloadItems( false );
            m_RatingListCtrl->ReloadItems( false );
            m_PlayCountListCtrl->ReloadItems( false );
            m_SongListCtrl->ReloadItems( false );
            m_UpdateLock = false;
    //    }
        m_InputTextCtrl->ShowCancelButton( false );
    }
}

// -------------------------------------------------------------------------------- //
void guLibPanel::DoSelectionChanged( void )
{
    switch( m_SelChangedObject )
    {
        case guPANEL_LIBRARY_TEXTSEARCH :
        {
            break;
        }

        case guPANEL_LIBRARY_LABELS :
        {
            m_Db->SetLaFilters( m_LabelsListCtrl->GetSelectedItems(), m_UpdateLock );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_GenreListCtrl->ReloadItems();
                m_ComposerListCtrl->ReloadItems();
                m_AlbumArtistListCtrl->ReloadItems();
                m_ArtistListCtrl->ReloadItems();
                m_AlbumListCtrl->ReloadItems();
                m_YearListCtrl->ReloadItems();
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                //
                //
                m_UpdateLock = false;
            }
            break;
        }

        case guPANEL_LIBRARY_GENRES :
        {
            //wxLongLong time = wxGetLocalTimeMillis();
            m_Db->SetGeFilters( m_GenreListCtrl->GetSelectedItems(), m_UpdateLock );

            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_ComposerListCtrl->ReloadItems();
                m_AlbumArtistListCtrl->ReloadItems();
                m_ArtistListCtrl->ReloadItems();
                m_YearListCtrl->ReloadItems();
                m_AlbumListCtrl->ReloadItems();
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            //time = wxGetLocalTimeMillis() - time;;
            //guLogWarning( wxT( "Genre Time : %u ms" ), time.GetLo() );
            break;
        }

        case guPANEL_LIBRARY_ARTISTS :
        {
            m_Db->SetArFilters( m_ArtistListCtrl->GetSelectedItems(), m_UpdateLock );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_AlbumListCtrl->ReloadItems();
                m_YearListCtrl->ReloadItems();
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            break;
        }

        case guPANEL_LIBRARY_YEARS :
        {
            m_Db->SetYeFilters( m_YearListCtrl->GetSelectedItems(), m_UpdateLock );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_AlbumListCtrl->ReloadItems();
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            break;
        }

        case guPANEL_LIBRARY_ALBUMS :
        {
            m_Db->SetAlFilters( m_AlbumListCtrl->GetSelectedItems(), m_UpdateLock );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            break;
        }

        case guPANEL_LIBRARY_RATINGS :
        {
            m_Db->SetRaFilters( m_RatingListCtrl->GetSelectedItems() );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            break;
        }

        case guPANEL_LIBRARY_PLAYCOUNT :
        {
            m_Db->SetPcFilters( m_PlayCountListCtrl->GetSelectedItems() );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            break;
        }

        case guPANEL_LIBRARY_COMPOSERS :
        {
            m_Db->SetCoFilters( m_ComposerListCtrl->GetSelectedItems(), m_UpdateLock );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_AlbumArtistListCtrl->ReloadItems();
                m_ArtistListCtrl->ReloadItems();
                m_YearListCtrl->ReloadItems();
                m_AlbumListCtrl->ReloadItems();
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            break;
        }

        case guPANEL_LIBRARY_ALBUMARTISTS :
        {
            m_Db->SetAAFilters( m_AlbumArtistListCtrl->GetSelectedItems(), m_UpdateLock );
            if( !m_UpdateLock )
            {
                m_UpdateLock = true;
                m_ComposerListCtrl->ReloadItems();
                m_ArtistListCtrl->ReloadItems();
                m_YearListCtrl->ReloadItems();
                m_AlbumListCtrl->ReloadItems();
                m_RatingListCtrl->ReloadItems();
                m_PlayCountListCtrl->ReloadItems();
                m_SongListCtrl->ReloadItems();
                m_UpdateLock = false;
            }
            break;
        }
    }
}

// -------------------------------------------------------------------------------- //
