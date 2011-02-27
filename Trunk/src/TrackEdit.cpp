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
#include "TrackEdit.h"

#include "Commands.h"
#include "Config.h"
#include "CoverEdit.h"
#include "Images.h"
#include "LastFM.h"
#include "TagInfo.h"
#include "MainFrame.h"
#include "Utils.h"

#include "wx/datetime.h"
#include <wx/notebook.h>
#include <wx/regex.h>

const wxEventType guTrackEditEvent = wxNewEventType();

// -------------------------------------------------------------------------------- //
void guImagePtrArrayClean( guImagePtrArray * images )
{
    int Index;
    int Count;
    if( images && ( Count = images->Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( ( * images )[ Index ] )
            {
                delete ( * images )[ Index ];
                ( * images )[ Index ] = NULL;
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
guTrackEditor::guTrackEditor( wxWindow * parent, guDbLibrary * db, guTrackArray * songs,
    guImagePtrArray * images, wxArrayString * lyrics )
{
    int index;
    int count;
	m_Db = db;

    wxPanel *           SongListPanel;
    wxPanel *           MainDetailPanel;
    wxStaticText *      ArStaticText;
    wxStaticText *      AlStaticText;
    wxStaticText *      TiStaticText;
    wxStaticText *      NuStaticText;
    wxStaticText *      GeStaticText;
    wxStaticText *      YeStaticText;
    wxPanel *           PicturePanel;
    wxPanel *           MBrainzPanel;
    wxStaticText *      MBAlbumStaticText;
    //wxStaticText *      MBAlbumDetailStaticText;
    wxStaticLine *      MBrainzStaticLine;

    m_LyricSearchEngine = NULL;
    m_LyricSearchContext = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( wxT( "TrackEditPosX" ), -1, wxT( "Positions" ) );
    WindowPos.y = Config->ReadNum( wxT( "TrackEditPosY" ), -1, wxT( "Positions" ) );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( wxT( "TrackEditSizeWidth" ), 625, wxT( "Positions" ) );
    WindowSize.y = Config->ReadNum( wxT( "TrackEditSizeHeight" ), 440, wxT( "Positions" ) );

    //wxDialog( parent, wxID_ANY, _( "Songs Editor" ), wxDefaultPosition, wxSize( 625, 440 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
    Create( parent, wxID_ANY, _( "Songs Editor" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

//	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_SongListSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_SongListSplitter->SetMinimumPaneSize( 150 );

	SongListPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxBoxSizer * SongsMainSizer;
	SongsMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* SongListSizer;
	SongListSizer = new wxStaticBoxSizer( new wxStaticBox( SongListPanel, wxID_ANY, _( " Songs " ) ), wxHORIZONTAL );

	m_SongListBox = new wxListBox( SongListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	SongListSizer->Add( m_SongListBox, 1, wxALL|wxEXPAND, 2 );

	wxBoxSizer* OrderSizer;
	OrderSizer = new wxBoxSizer( wxVERTICAL );

	m_MoveUpButton = new wxBitmapButton( SongListPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MoveUpButton->SetToolTip( _( "Move the track to the previous position" ) );
	m_MoveUpButton->Enable( false );

	OrderSizer->Add( m_MoveUpButton, 0, wxALL, 2 );

	m_MoveDownButton = new wxBitmapButton( SongListPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MoveUpButton->SetToolTip( _( "Move the track to the next position" ) );
	m_MoveDownButton->Enable( false );
	OrderSizer->Add( m_MoveDownButton, 0, wxALL, 2 );

	SongListSizer->Add( OrderSizer, 0, wxEXPAND, 5 );

	SongsMainSizer->Add( SongListSizer, 1, wxEXPAND|wxALL, 5 );

	SongListPanel->SetSizer( SongsMainSizer );
	SongListPanel->Layout();
	SongListSizer->Fit( SongListPanel );
	MainDetailPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* DetailSizer;
	DetailSizer = new wxBoxSizer( wxVERTICAL );

	m_MainNotebook = new wxNotebook( MainDetailPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

    //
    // Details
    //
	wxPanel * DetailPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxSizer * MainDetailSizer = new wxBoxSizer( wxVERTICAL );
	wxFlexGridSizer* DataFlexSizer;
	DataFlexSizer = new wxFlexGridSizer( 6, 3, 0, 0 );
	DataFlexSizer->AddGrowableCol( 2 );
	DataFlexSizer->SetFlexibleDirection( wxBOTH );
	DataFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_ArCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_ArCopyButton->SetToolTip( _( "Copy the Artist name to all the tracks you are editing" ) );
	DataFlexSizer->Add( m_ArCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	ArStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	ArStaticText->Wrap( -1 );
	DataFlexSizer->Add( ArStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

	m_ArtistComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_Artists, wxCB_DROPDOWN );
	DataFlexSizer->Add( m_ArtistComboBox, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	m_AACopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_AACopyButton->SetToolTip( _( "Copy the Album Artist name to all the tracks you are editing" ) );
	DataFlexSizer->Add( m_AACopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * AAStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "A. Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	AAStaticText->SetToolTip( _( "shows the Album Artist of the track" ) );
	AAStaticText->Wrap( -1 );
	DataFlexSizer->Add( AAStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

	m_AlbumArtistComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_AlbumArtists, wxCB_DROPDOWN );
	DataFlexSizer->Add( m_AlbumArtistComboBox, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	m_AlCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_AlCopyButton->SetToolTip( _( "Copy the Album name to all the tracks you are editing" ) );
	DataFlexSizer->Add( m_AlCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	AlStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Album:" ), wxDefaultPosition, wxDefaultSize, 0 );
	AlStaticText->Wrap( -1 );
	DataFlexSizer->Add( AlStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

	m_AlbumComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_Albums, wxCB_DROPDOWN );
	DataFlexSizer->Add( m_AlbumComboBox, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	m_TiCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_TiCopyButton->SetToolTip( _( "Copy the Title name to all the tracks you are editing" ) );
	DataFlexSizer->Add( m_TiCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	TiStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Title:" ), wxDefaultPosition, wxDefaultSize, 0 );
	TiStaticText->Wrap( -1 );
	DataFlexSizer->Add( TiStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

	m_TitleTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_TitleTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	m_CoCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CoCopyButton->SetToolTip( _( "Copy the composer name to all the tracks you are editing" ) );
	DataFlexSizer->Add( m_CoCopyButton, 0, wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * CoStaticText;
	CoStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Composer:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CoStaticText->Wrap( -1 );
	DataFlexSizer->Add( CoStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_CompComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_Composers, wxCB_DROPDOWN );
	DataFlexSizer->Add( m_CompComboBox, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	m_CommentCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CommentCopyButton->SetToolTip( _( "Copy the comment to all the tracks you are editing" ) );
	DataFlexSizer->Add( m_CommentCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * CommentStaticText;
	CommentStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Comment:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CommentStaticText->Wrap( -1 );
	DataFlexSizer->Add( CommentStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_CommentText = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, 54 ), wxTE_MULTILINE );
	DataFlexSizer->Add( m_CommentText, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );


	m_NuCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_numerate ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_NuCopyButton->SetToolTip( _( "Enumerate the tracks in the order they were added for editing" ) );
	DataFlexSizer->Add( m_NuCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	NuStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Number:" ), wxDefaultPosition, wxDefaultSize, 0 );
	NuStaticText->Wrap( -1 );
	DataFlexSizer->Add( NuStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

//	m_NumberTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
//	DataFlexSizer->Add( m_NumberTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	wxBoxSizer* DiskSizer;
	DiskSizer = new wxBoxSizer( wxHORIZONTAL );

	m_NumberTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DiskSizer->Add( m_NumberTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	DiskSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_DiCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DiCopyButton->SetToolTip( _( "Copy the disk to all the tracks you are editing" ) );
	DiskSizer->Add( m_DiCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * DiStaticText;
	DiStaticText = new wxStaticText( DetailPanel, wxID_ANY, _("Disk:"), wxDefaultPosition, wxDefaultSize, 0 );
	DiStaticText->Wrap( -1 );
	DiskSizer->Add( DiStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_DiskTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DiskSizer->Add( m_DiskTextCtrl, 0, wxTOP|wxRIGHT, 5 );

	DataFlexSizer->Add( DiskSizer, 1, wxEXPAND, 5 );

	m_GeCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_GeCopyButton->SetToolTip( _( "Copy the Genre name to all songs you are editing" ) );
	DataFlexSizer->Add( m_GeCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	GeStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Genre:" ), wxDefaultPosition, wxDefaultSize, 0 );
	GeStaticText->Wrap( -1 );
	DataFlexSizer->Add( GeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

	m_GenreComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_Genres, wxCB_DROPDOWN );
	DataFlexSizer->Add( m_GenreComboBox, 1, wxEXPAND|wxTOP|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_YeCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_YeCopyButton->SetToolTip( _( "Copy the Year to all songs you are editing" ) );
	DataFlexSizer->Add( m_YeCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	YeStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Year:" ), wxDefaultPosition, wxDefaultSize, 0 );
	YeStaticText->Wrap( -1 );
	DataFlexSizer->Add( YeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

	wxBoxSizer * RatingSizer;
	RatingSizer = new wxBoxSizer( wxHORIZONTAL );

	m_YearTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	RatingSizer->Add( m_YearTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	RatingSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_RaCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	RatingSizer->Add( m_RaCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxStaticText * RaStaticText;
	RaStaticText = new wxStaticText( DetailPanel, wxID_ANY, _("Rating:"), wxDefaultPosition, wxDefaultSize, 0 );
	RaStaticText->Wrap( -1 );
	RatingSizer->Add( RaStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    m_Rating = new guRating( DetailPanel, GURATING_STYLE_BIG );
	RatingSizer->Add( m_Rating, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	DataFlexSizer->Add( RatingSizer, 1, wxEXPAND, 5 );

	MainDetailSizer->Add( DataFlexSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* MoreDetailsSizer;
	MoreDetailsSizer = new wxBoxSizer( wxHORIZONTAL );

	m_DetailLeftInfoStaticText = new wxStaticText( DetailPanel, wxID_ANY, wxT("File Type\t:\nLength\t:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailLeftInfoStaticText->Wrap( -1 );
	MoreDetailsSizer->Add( m_DetailLeftInfoStaticText, 1, wxALL|wxEXPAND, 5 );

	m_DetailRightInfoStaticText = new wxStaticText( DetailPanel, wxID_ANY, wxT("BitRate\t:\nFileSize\t:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailRightInfoStaticText->Wrap( -1 );
	MoreDetailsSizer->Add( m_DetailRightInfoStaticText, 1, wxALL|wxEXPAND, 5 );

	MainDetailSizer->Add( MoreDetailsSizer, 1, wxEXPAND, 5 );

	DetailPanel->SetSizer( MainDetailSizer );
	DetailPanel->Layout();
	DataFlexSizer->Fit( DetailPanel );
	m_MainNotebook->AddPage( DetailPanel, _( "Details" ), true );

	//
	// Pictures
	//
	PicturePanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* PictureSizer;
	PictureSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* PictureBitmapSizer;
	PictureBitmapSizer = new wxStaticBoxSizer( new wxStaticBox( PicturePanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	m_PictureBitmap = new wxStaticBitmap( PicturePanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 250,250 ), wxSUNKEN_BORDER );
	PictureBitmapSizer->Add( m_PictureBitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	PictureSizer->Add( PictureBitmapSizer, 0, wxALIGN_CENTER_HORIZONTAL, 5 );


	wxBoxSizer* PictureButtonSizer;
	PictureButtonSizer = new wxBoxSizer( wxHORIZONTAL );

	m_AddPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_AddPicButton->SetToolTip( _( "Add a picture from file to the current track" ) );
	PictureButtonSizer->Add( m_AddPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_DelPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelPicButton->SetToolTip( _( "Delete the picture from the current track" ) );
	PictureButtonSizer->Add( m_DelPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_SavePicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_doc_save ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_SavePicButton->SetToolTip( _( "Save the current picture to file" ) );
	PictureButtonSizer->Add( m_SavePicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_SearchPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_SearchPicButton->SetToolTip( _( "Search the album cover" ) );
	PictureButtonSizer->Add( m_SearchPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

//	m_EditPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_edit ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	PictureButtonSizer->Add( m_EditPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	PictureButtonSizer->Add( 10, 0, 0, wxEXPAND, 5 );

	m_CopyPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CopyPicButton->SetToolTip( _( "Copy the current picute to all the tracks you are editing" ) );
	PictureButtonSizer->Add( m_CopyPicButton, 0, wxALL, 5 );

	PictureSizer->Add( PictureButtonSizer, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	PicturePanel->SetSizer( PictureSizer );
	PicturePanel->Layout();
	PictureSizer->Fit( PicturePanel );
	m_MainNotebook->AddPage( PicturePanel, _( "Pictures" ), false );

    //
    // Lyrics
    //
	wxPanel * LyricsPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * LyricsSizer;
	LyricsSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* LyricsTopSizer;
	LyricsTopSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * ArtistStaticText = new wxStaticText( LyricsPanel, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	ArtistStaticText->Wrap( -1 );
	LyricsTopSizer->Add( ArtistStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_LyricArtistTextCtrl = new wxTextCtrl( LyricsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LyricsTopSizer->Add( m_LyricArtistTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	wxStaticText * TrackStaticText = new wxStaticText( LyricsPanel, wxID_ANY, _( "Track:" ), wxDefaultPosition, wxDefaultSize, 0 );
	TrackStaticText->Wrap( -1 );
	LyricsTopSizer->Add( TrackStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_LyricTrackTextCtrl = new wxTextCtrl( LyricsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LyricsTopSizer->Add( m_LyricTrackTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_LyricReloadButton = new wxBitmapButton( LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search_again ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_LyricReloadButton->SetToolTip( _( "Search for lyrics" ) );
	LyricsTopSizer->Add( m_LyricReloadButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	LyricsSizer->Add( LyricsTopSizer, 0, wxEXPAND, 5 );


	m_LyricsTextCtrl = new wxTextCtrl( LyricsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE|wxTE_DONTWRAP|wxTE_MULTILINE );
	LyricsSizer->Add( m_LyricsTextCtrl, 1, wxALL|wxEXPAND, 5 );

	LyricsPanel->SetSizer( LyricsSizer );
	LyricsPanel->Layout();
	LyricsSizer->Fit( LyricsPanel );
	m_MainNotebook->AddPage( LyricsPanel, _( "Lyrics" ), false );


    //
    // MusicBrainz
    //
	MBrainzPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* MBrainzSizer;
	MBrainzSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* MBQuerySizer;
	MBQuerySizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText * MBQueryArtistStaticText;
	wxStaticText * MBQueryTitleStaticText;
	MBQueryArtistStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Artist:"), wxDefaultPosition, wxDefaultSize, 0 );
	MBQueryArtistStaticText->Wrap( -1 );
	MBQuerySizer->Add( MBQueryArtistStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_MBQueryArtistTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_MBQueryArtistTextCtrl->SetToolTip( _( "Type the artist name to search in musicbrainz" ) );
	MBQuerySizer->Add( m_MBQueryArtistTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MBQueryTitleStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	MBQueryTitleStaticText->Wrap( -1 );
	MBQuerySizer->Add( MBQueryTitleStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_MBQueryTitleTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_MBQueryTitleTextCtrl->SetToolTip( _( "Type the album name to search in musicbrainz" ) );
	MBQuerySizer->Add( m_MBQueryTitleTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	m_MBQueryClearButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_clear ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBQueryClearButton->SetToolTip( _( "Clear the search fields so it search using the music fingerprint" ) );
	MBQuerySizer->Add( m_MBQueryClearButton, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MBrainzSizer->Add( MBQuerySizer, 0, wxEXPAND, 5 );

	wxBoxSizer* MBrainzTopSizer;
	MBrainzTopSizer = new wxBoxSizer( wxHORIZONTAL );

	MBAlbumStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "Album:" ), wxDefaultPosition, wxDefaultSize, 0 );
	MBAlbumStaticText->Wrap( -1 );
	MBrainzTopSizer->Add( MBAlbumStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxArrayString m_MBrainzAlbumChoiceChoices;
	m_MBrainzAlbumChoice = new wxChoice( MBrainzPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MBrainzAlbumChoiceChoices, 0 );
	m_MBrainzAlbumChoice->SetToolTip( _( "Select the album found in musicbrainz" ) );
	m_MBrainzAlbumChoice->SetSelection( 0 );

	MBrainzTopSizer->Add( m_MBrainzAlbumChoice, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	m_MBrainzAddButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzAddButton->SetToolTip( _( "Search albums in musicbrainz" ) );
	//m_MBrainzAddButton->Enable( false );

	MBrainzTopSizer->Add( m_MBrainzAddButton, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_MBrainzCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzCopyButton->SetToolTip( _( "Copy the content of the album to the edited tracks" ) );
	m_MBrainzCopyButton->Enable( false );

	MBrainzTopSizer->Add( m_MBrainzCopyButton, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MBrainzSizer->Add( MBrainzTopSizer, 0, wxEXPAND, 5 );

	wxStaticBoxSizer* MBDetailSizer;
	MBDetailSizer = new wxStaticBoxSizer( new wxStaticBox( MBrainzPanel, wxID_ANY, _( " Details " ) ), wxVERTICAL );

	wxFlexGridSizer* MBDetailFlexGridSizer;
	MBDetailFlexGridSizer = new wxFlexGridSizer( 5, 3, 0, 0 );
	MBDetailFlexGridSizer->AddGrowableCol( 1 );
	MBDetailFlexGridSizer->SetFlexibleDirection( wxBOTH );
	MBDetailFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_MBrainzArtistStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_MBrainzArtistStaticText->Wrap( -1 );
	MBDetailFlexGridSizer->Add( m_MBrainzArtistStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_MBrainzArtistTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	MBDetailFlexGridSizer->Add( m_MBrainzArtistTextCtrl, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_MBrainzArCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzArCopyButton->SetToolTip( _( "Copy the artist to the edited tracks" ) );
	MBDetailFlexGridSizer->Add( m_MBrainzArCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_MBrainzAlbumArtistStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "A. Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_MBrainzAlbumArtistStaticText->Wrap( -1 );
	MBDetailFlexGridSizer->Add( m_MBrainzAlbumArtistStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_MBrainzAlbumArtistTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	MBDetailFlexGridSizer->Add( m_MBrainzAlbumArtistTextCtrl, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_MBrainzAlArCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzAlArCopyButton->SetToolTip( _( "Copy the album artist to the edited tracks" ) );
	MBDetailFlexGridSizer->Add( m_MBrainzAlArCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_MBrainzAlbumStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Album:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_MBrainzAlbumStaticText->Wrap( -1 );
	MBDetailFlexGridSizer->Add( m_MBrainzAlbumStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_MBrainzAlbumTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	MBDetailFlexGridSizer->Add( m_MBrainzAlbumTextCtrl, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_MBrainzAlCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzAlCopyButton->SetToolTip( _( "Copy the album to the edited tracks" ) );
	MBDetailFlexGridSizer->Add( m_MBrainzAlCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_MBrainzDateStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "Date:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_MBrainzDateStaticText->Wrap( -1 );
	MBDetailFlexGridSizer->Add( m_MBrainzDateStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	wxArrayString m_MBrainzDateChoiceChoices;
	m_MBrainzDateChoice = new wxChoice( MBrainzPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MBrainzDateChoiceChoices, 0 );
	m_MBrainzDateChoice->SetSelection( 0 );
	MBDetailFlexGridSizer->Add( m_MBrainzDateChoice, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_MBrainzDaCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzDaCopyButton->SetToolTip( _( "Copy the date to the edited tracks" ) );
	MBDetailFlexGridSizer->Add( m_MBrainzDaCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_MBrainzTitleStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "Title:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_MBrainzTitleStaticText->Wrap( -1 );
	MBDetailFlexGridSizer->Add( m_MBrainzTitleStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_MBrainzTitleTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	MBDetailFlexGridSizer->Add( m_MBrainzTitleTextCtrl, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_MBrainzTiCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzTiCopyButton->SetToolTip( _( "Copy the song names to the edited tracks" ) );
	MBDetailFlexGridSizer->Add( m_MBrainzTiCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_MBrainzLengthStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_MBrainzLengthStaticText->Wrap( -1 );
	MBDetailFlexGridSizer->Add( m_MBrainzLengthStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	wxBoxSizer* MBNumberSizer;
	MBNumberSizer = new wxBoxSizer( wxHORIZONTAL );

	m_MBrainzLengthTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	MBNumberSizer->Add( m_MBrainzLengthTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	MBNumberSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_MBrainzNumberStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Number:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_MBrainzNumberStaticText->Wrap( -1 );
	MBNumberSizer->Add( m_MBrainzNumberStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_MBrainzNumberTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	MBNumberSizer->Add( m_MBrainzNumberTextCtrl, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	MBDetailFlexGridSizer->Add( MBNumberSizer, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	m_MBrainzNuCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_MBrainzNuCopyButton->SetToolTip( _( "Copy the number to the edited tracks" ) );
	MBDetailFlexGridSizer->Add( m_MBrainzNuCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	MBDetailSizer->Add( MBDetailFlexGridSizer, 0, wxEXPAND, 5 );

	MBrainzStaticLine = new wxStaticLine( MBrainzPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MBDetailSizer->Add( MBrainzStaticLine, 0, wxEXPAND | wxALL, 5 );

	m_MBrainzInfoStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
	m_MBrainzInfoStaticText->Wrap( 398 );
	MBDetailSizer->Add( m_MBrainzInfoStaticText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );

	MBrainzSizer->Add( MBDetailSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	MBrainzPanel->SetSizer( MBrainzSizer );
	MBrainzPanel->Layout();
	MBrainzSizer->Fit( MBrainzPanel );
	m_MainNotebook->AddPage( MBrainzPanel, wxT( "MusicBrainz" ), false );


	DetailSizer->Add( m_MainNotebook, 1, wxEXPAND | wxALL, 5 );

	MainDetailPanel->SetSizer( DetailSizer );
	MainDetailPanel->Layout();
	DetailSizer->Fit( MainDetailPanel );
	m_SongListSplitter->SplitVertically( SongListPanel, MainDetailPanel, 200 );
	MainSizer->Add( m_SongListSplitter, 1, wxEXPAND, 5 );

    wxStdDialogButtonSizer *    ButtonsSizer;
    wxButton *                  ButtonsSizerOK;
    wxButton *                  ButtonsSizerCancel;
	ButtonsSizer = new wxStdDialogButtonSizer();
	ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();


	// --------------------------------------------------------------------
    m_NormalColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    m_ErrorColor = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );
	m_MBrainzThread = NULL;
	m_MBrainzAlbums = NULL;
	m_MBrainzReleases = new guMBReleaseArray();
	m_MBrainzCurTrack = 0;
	m_MBrainzCurAlbum = wxNOT_FOUND;
	m_MBQuerySetArtistEnabled = true;
	m_CurItem = 0;
	m_Items = songs;
	m_Images = images;
	m_Lyrics = lyrics;
	m_CurrentRating = -1;
	m_RatingChanged = false;
	m_GenreChanged = false;
	wxArrayString ItemsText;
	count = m_Items->Count();
	for( index = 0; index < count; index++ )
	{
        ItemsText.Add( ( * m_Items )[ index ].m_FileName );
        // Fill the initial Images of the files
        m_Images->Add( guTagGetPicture( ( * m_Items )[ index ].m_FileName ) );
        m_Lyrics->Add( guTagGetLyrics( ( * m_Items )[ index ].m_FileName ) );
	}
	m_SongListBox->InsertItems( ItemsText, 0 );
	m_SongListBox->SetFocus();

	// Connect Events
	Connect( wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnOKButton ) );
	m_MainNotebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( guTrackEditor::OnPageChanged ), NULL, this );

	m_SongListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guTrackEditor::OnSongListBoxSelected ), NULL, this );
	m_MoveUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMoveUpBtnClick ), NULL, this );
	m_MoveDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMoveDownBtnClick ), NULL, this );
	m_ArCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnArCopyButtonClicked ), NULL, this );
	m_AACopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAACopyButtonClicked ), NULL, this );
	m_AlCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAlCopyButtonClicked ), NULL, this );
	m_TiCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnTiCopyButtonClicked ), NULL, this );
	m_CoCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCoCopyButtonClicked ), NULL, this );
	m_NuCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnNuCopyButtonClicked ), NULL, this );
	m_DiCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnDiCopyButtonClicked ), NULL, this );
	m_GeCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGeCopyButtonClicked ), NULL, this );
	m_YeCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnYeCopyButtonClicked ), NULL, this );
	m_RaCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnRaCopyButtonClicked ), NULL, this );
    m_Rating->Connect( guEVT_RATING_CHANGED, guRatingEventHandler( guTrackEditor::OnRatingChanged ), NULL, this );
	m_CommentCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCommentCopyButtonClicked ), NULL, this );

	m_ArtistComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnArtistTextChanged ), NULL, this );
	m_AlbumArtistComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnAlbumArtistTextChanged ), NULL, this );
	m_AlbumComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnAlbumTextChanged ), NULL, this );
	m_CompComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnComposerTextChanged ), NULL, this );
	m_GenreComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnGenreTextChanged ), NULL, this );

	m_AddPicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAddImageClicked ), NULL, this );
	m_DelPicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnDelImageClicked ), NULL, this );
	m_SavePicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSaveImageClicked ), NULL, this );
	m_SearchPicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSearchImageClicked ), NULL, this );
	m_CopyPicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCopyImageClicked ), NULL, this );

	m_LyricReloadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSearchLyrics ), NULL, this );
	m_LyricArtistTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnTextUpdated ), NULL, this );
	m_LyricTrackTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnTextUpdated ), NULL, this );

	m_MBQueryArtistTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnMBQueryTextCtrlChanged ), NULL, this );
	m_MBQueryTitleTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnMBQueryTextCtrlChanged ), NULL, this );
	m_MBQueryClearButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBQueryClearButtonClicked ), NULL, this );

    m_MBrainzAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzAddButtonClicked ), NULL, this );
    Connect( guTRACKEDIT_EVENT_MBRAINZ_TRACKS, guTrackEditEvent, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumsFound ), NULL, this );
	m_MBrainzAlbumChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumChoiceSelected ), NULL, this );
	m_MBrainzCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzCopyButtonClicked ), NULL, this );

	m_MBrainzArCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzArtistCopyButtonClicked ), NULL, this );
	m_MBrainzAlArCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumArtistCopyButtonClicked ), NULL, this );
	m_MBrainzAlCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumCopyButtonClicked ), NULL, this );
	m_MBrainzDaCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzDateCopyButtonClicked ), NULL, this );
	m_MBrainzTiCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzTitleCopyButtonClicked ), NULL, this );
	m_MBrainzNuCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzNumberCopyButtonClicked ), NULL, this );

    Connect( ID_LYRICS_LYRICFOUND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackEditor::OnDownloadedLyric ), NULL, this );

    // Idle Events
	m_SongListSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guTrackEditor::SongListSplitterOnIdle ), NULL, this );

	//
    // Force the 1st listbox item to be selected
	m_SongListBox->SetSelection( 0 );
//    wxCommandEvent event( wxEVT_COMMAND_LISTBOX_SELECTED, m_SongListBox->GetId() );
//    event.SetEventObject( m_SongListBox );
//    event.SetExtraLong( 1 );
//    event.SetInt( 0 );
//    wxPostEvent( m_SongListBox, event );
    m_CurItem = 0;
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
guTrackEditor::~guTrackEditor()
{
    // Save the window position and size
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( wxT( "TrackEditSashPos" ), m_SongListSplitter->GetSashPosition(), wxT( "Positions" ) );
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( wxT( "TrackEditPosX" ), WindowPos.x, wxT( "Positions" ) );
    Config->WriteNum( wxT( "TrackEditPosY" ), WindowPos.y, wxT( "Positions" ) );
    wxSize WindowSize = GetSize();
    Config->WriteNum( wxT( "TrackEditSizeWidth" ), WindowSize.x, wxT( "Positions" ) );
    Config->WriteNum( wxT( "TrackEditSizeHeight" ), WindowSize.y, wxT( "Positions" ) );

	Disconnect( wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnOKButton ) );
	m_MainNotebook->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( guTrackEditor::OnPageChanged ), NULL, this );

	m_SongListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guTrackEditor::OnSongListBoxSelected ), NULL, this );
	m_MoveUpButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMoveUpBtnClick ), NULL, this );
	m_MoveDownButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMoveDownBtnClick ), NULL, this );
	m_ArCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnArCopyButtonClicked ), NULL, this );
	m_AACopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAACopyButtonClicked ), NULL, this );
	m_AlCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAlCopyButtonClicked ), NULL, this );
	m_TiCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnTiCopyButtonClicked ), NULL, this );
	m_CoCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCoCopyButtonClicked ), NULL, this );
	m_NuCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnNuCopyButtonClicked ), NULL, this );
	m_DiCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnDiCopyButtonClicked ), NULL, this );
	m_GeCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGeCopyButtonClicked ), NULL, this );
	m_YeCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnYeCopyButtonClicked ), NULL, this );
	m_RaCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnRaCopyButtonClicked ), NULL, this );
    m_Rating->Disconnect( guEVT_RATING_CHANGED, guRatingEventHandler( guTrackEditor::OnRatingChanged ), NULL, this );
	m_CommentCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCommentCopyButtonClicked ), NULL, this );

	m_ArtistComboBox->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnArtistTextChanged ), NULL, this );
	m_AlbumArtistComboBox->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnAlbumArtistTextChanged ), NULL, this );
	m_AlbumComboBox->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnAlbumTextChanged ), NULL, this );
	m_CompComboBox->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnComposerTextChanged ), NULL, this );
	m_GenreComboBox->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnGenreTextChanged ), NULL, this );

	m_AddPicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAddImageClicked ), NULL, this );
	m_DelPicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnDelImageClicked ), NULL, this );
	m_SavePicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSaveImageClicked ), NULL, this );
	m_SearchPicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSearchImageClicked ), NULL, this );
	m_CopyPicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCopyImageClicked ), NULL, this );

	m_LyricReloadButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSearchLyrics ), NULL, this );
	m_LyricArtistTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnTextUpdated ), NULL, this );
	m_LyricTrackTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnTextUpdated ), NULL, this );

	m_MBQueryArtistTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnMBQueryTextCtrlChanged ), NULL, this );
	m_MBQueryTitleTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guTrackEditor::OnMBQueryTextCtrlChanged ), NULL, this );
	m_MBQueryClearButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBQueryClearButtonClicked ), NULL, this );

    m_MBrainzAddButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzAddButtonClicked ), NULL, this );
    Disconnect( guTRACKEDIT_EVENT_MBRAINZ_TRACKS, guTrackEditEvent, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumsFound ), NULL, this );
	m_MBrainzAlbumChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumChoiceSelected ), NULL, this );
	m_MBrainzCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzCopyButtonClicked ), NULL, this );

	m_MBrainzArCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzArtistCopyButtonClicked ), NULL, this );
	m_MBrainzAlArCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumArtistCopyButtonClicked ), NULL, this );
	m_MBrainzAlCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzAlbumCopyButtonClicked ), NULL, this );
	m_MBrainzDaCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzDateCopyButtonClicked ), NULL, this );
	m_MBrainzTiCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzTitleCopyButtonClicked ), NULL, this );
	m_MBrainzNuCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnMBrainzNumberCopyButtonClicked ), NULL, this );

    Disconnect( ID_LYRICS_LYRICFOUND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackEditor::OnDownloadedLyric ), NULL, this );

    // Idle Events
	m_SongListSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guTrackEditor::SongListSplitterOnIdle ), NULL, this );

    m_MBrainzThreadMutex.Lock();
    if( m_MBrainzThread )
    {
        m_MBrainzThread->Pause();
        m_MBrainzThread->Delete();
    }
    m_MBrainzThreadMutex.Unlock();

    if( m_MBrainzReleases )
        delete m_MBrainzReleases;
    if( m_MBrainzAlbums )
        delete m_MBrainzAlbums;
    if( m_LyricSearchContext )
        delete m_LyricSearchContext;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnPageChanged( wxNotebookEvent &event )
{
    WriteItemData();

    wxCommandEvent CmdEvent;
    CmdEvent.SetInt( m_CurItem );
    OnSongListBoxSelected( CmdEvent );

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnOKButton( wxCommandEvent& event )
{
    WriteItemData();
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnSongListBoxSelected( wxCommandEvent& event )
{
    WriteItemData();
    m_CurItem = event.GetInt();
    if( m_LyricSearchContext )
    {
        delete m_LyricSearchContext;
        m_LyricSearchContext = NULL;
    }
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMoveUpBtnClick( wxCommandEvent &event )
{
    wxString FileName = m_SongListBox->GetString( m_CurItem );
    guTrack * MovedTrack = m_Items->Detach( m_CurItem );
    wxImage * MovedImage = ( * m_Images )[ m_CurItem ];
    wxString MovedLyric = ( * m_Lyrics )[ m_CurItem ];
    m_Images->RemoveAt( m_CurItem );
    m_Lyrics->RemoveAt( m_CurItem );
    m_SongListBox->SetString( m_CurItem, m_SongListBox->GetString( m_CurItem - 1 ) );
    m_CurItem--;
    m_Items->Insert( MovedTrack, m_CurItem );
    m_Images->Insert( MovedImage, m_CurItem );
    m_Lyrics->Insert( MovedLyric, m_CurItem );
    m_SongListBox->SetString( m_CurItem, FileName );

    m_SongListBox->SetSelection( m_CurItem );

    event.SetInt( m_CurItem );
    OnSongListBoxSelected( event );

    MBCheckCountAndLengths();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMoveDownBtnClick( wxCommandEvent &event )
{
    wxString FileName = m_SongListBox->GetString( m_CurItem );
    guTrack * MovedTrack = m_Items->Detach( m_CurItem );
    wxImage * MovedImage = ( * m_Images )[ m_CurItem ];
    wxString MovedLyric = ( * m_Lyrics )[ m_CurItem ];
    m_Images->RemoveAt( m_CurItem );
    m_Lyrics->RemoveAt( m_CurItem );
    m_SongListBox->SetString( m_CurItem, m_SongListBox->GetString( m_CurItem + 1 ) );
    m_CurItem++;
    m_Items->Insert( MovedTrack, m_CurItem );
    m_Images->Insert( MovedImage, m_CurItem );
    m_Lyrics->Insert( MovedLyric, m_CurItem );
    m_SongListBox->SetString( m_CurItem, FileName );

    m_SongListBox->SetSelection( m_CurItem );

    event.SetInt( m_CurItem );
    OnSongListBoxSelected( event );

    MBCheckCountAndLengths();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::ReadItemData( void )
{
    //guLogMessage( wxT( "ReadItemData: %i" ), m_CurItem );
    if( m_CurItem >= 0 )
    {
        guTrack * Track = &( * m_Items )[ m_CurItem ];
        m_AlbumArtistComboBox->SetValue( Track->m_AlbumArtist );
        m_ArtistComboBox->SetValue( Track->m_ArtistName );
        m_AlbumComboBox->SetValue( Track->m_AlbumName );
        m_TitleTextCtrl->SetValue( Track->m_SongName );
        m_CompComboBox->SetValue( Track->m_Composer );
        if( Track->m_Number )
            m_NumberTextCtrl->SetValue( wxString::Format( wxT( "%u" ), Track->m_Number ) );
        else
            m_NumberTextCtrl->SetValue( wxEmptyString );
        m_DiskTextCtrl->SetValue( Track->m_Disk );
        m_GenreComboBox->SetValue( Track->m_GenreName );
        if( Track->m_Year )
            m_YearTextCtrl->SetValue( wxString::Format( wxT( "%u" ), Track->m_Year ) );
        else
            m_YearTextCtrl->SetValue( wxEmptyString );
        m_Rating->SetRating( Track->m_Rating );
        m_CommentText->SetValue( Track->m_Comments );
        m_DetailLeftInfoStaticText->SetLabel( wxString::Format( _( "File Type\t: %s\n"
                                               "Length\t: %s" ),
                                               Track->m_FileName.AfterLast( wxT( '.' ) ).c_str(),
                                               LenToString( Track->m_Length ).c_str() ) );
        m_DetailRightInfoStaticText->SetLabel( wxString::Format( _( "BitRate\t: %u Kbps\n"
                                               "File Size\t: %s" ),
                                               Track->m_Bitrate,
                                               SizeToString( guGetFileSize( Track->m_FileName ) ).c_str() ) );

        if( m_MBQuerySetArtistEnabled )
        {
            m_MBQueryArtistTextCtrl->SetValue( Track->m_ArtistName );
            m_MBQueryTitleTextCtrl->SetValue( Track->m_AlbumName );
        }

        m_MoveUpButton->Enable( m_CurItem > 0 );
        m_MoveDownButton->Enable( m_CurItem < ( int ) ( m_Items->Count() - 1 ) );

        m_LyricArtistTextCtrl->SetValue( Track->m_ArtistName );
        m_LyricTrackTextCtrl->SetValue( Track->m_SongName );
        m_LyricsTextCtrl->SetValue( ( * m_Lyrics )[ m_CurItem ] );
    }
    else
    {
        m_AlbumArtistComboBox->SetValue( wxEmptyString );
        m_ArtistComboBox->SetValue( wxEmptyString );
        m_AlbumComboBox->SetValue( wxEmptyString );
        m_TitleTextCtrl->SetValue( wxEmptyString );
        m_CompComboBox->SetValue( wxEmptyString );
        m_NumberTextCtrl->SetValue( wxEmptyString );
        m_DiskTextCtrl->SetValue( wxEmptyString );
        m_GenreComboBox->SetValue( wxEmptyString );
        m_YearTextCtrl->SetValue( wxEmptyString );
        m_Rating->SetRating( -1 );
        m_CommentText->SetValue( wxEmptyString );
        m_DetailLeftInfoStaticText->SetLabel( wxEmptyString );
        m_DetailRightInfoStaticText->SetLabel( wxEmptyString );
        m_MBQueryArtistTextCtrl->SetValue( wxEmptyString );
        m_MBQueryTitleTextCtrl->SetValue( wxEmptyString );

        m_MoveUpButton->Enable( false );
        m_MoveDownButton->Enable( false );

        m_LyricArtistTextCtrl->SetValue( wxEmptyString );
        m_LyricTrackTextCtrl->SetValue( wxEmptyString );
        m_LyricsTextCtrl->SetValue( wxEmptyString );
    }
    UpdateArtists();
    UpdateAlbumArtists();
    UpdateAlbums();
    UpdateComposers();
    UpdateGenres();
    m_ArtistChanged = false;
    m_AlbumArtistChanged = false;
    m_AlbumChanged = false;
    m_CompChanged = false;
    m_GenreChanged = false;
    m_RatingChanged = false;
    RefreshImage();
    UpdateMBrainzTrackInfo();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::WriteItemData( void )
{
    //guLogMessage( wxT( "WriteItemData: %i" ), m_CurItem );
    if( m_CurItem >= 0 )
    {
        long LongValue;
        if( m_AlbumArtistChanged )
          ( * m_Items )[ m_CurItem ].m_AlbumArtist = m_AlbumArtistComboBox->GetValue();
        //if( m_ArtistTextCtrl->IsModified() )
        if( m_ArtistChanged )
          ( * m_Items )[ m_CurItem ].m_ArtistName = m_ArtistComboBox->GetValue();
        if( m_AlbumChanged )
          ( * m_Items )[ m_CurItem ].m_AlbumName = m_AlbumComboBox->GetValue();
        if( m_TitleTextCtrl->IsModified() )
          ( * m_Items )[ m_CurItem ].m_SongName = m_TitleTextCtrl->GetLineText( 0 );
        if( m_CompChanged )
          ( * m_Items )[ m_CurItem ].m_Composer = m_CompComboBox->GetValue();
        if( m_NumberTextCtrl->IsModified() )
        {
          m_NumberTextCtrl->GetLineText( 0 ).ToLong( &LongValue );
          ( * m_Items )[ m_CurItem ].m_Number = LongValue;
        }
        if( m_DiskTextCtrl->IsModified() )
          ( * m_Items )[ m_CurItem ].m_Disk = m_DiskTextCtrl->GetLineText( 0 );
        if( m_GenreChanged )
          ( * m_Items )[ m_CurItem ].m_GenreName = m_GenreComboBox->GetValue();
        if( m_YearTextCtrl->IsModified() )
        {
           m_YearTextCtrl->GetLineText( 0 ).ToLong( &LongValue );
           ( * m_Items )[ m_CurItem ].m_Year = LongValue;
        }
        if( m_RatingChanged )
            ( * m_Items )[ m_CurItem ].m_Rating = m_Rating->GetRating();
        if( m_CommentText->IsModified() )
          ( * m_Items )[ m_CurItem ].m_Comments = m_CommentText->GetValue();
        if( m_LyricsTextCtrl->IsModified() )
            ( * m_Lyrics )[ m_CurItem ] = m_LyricsTextCtrl->GetValue();
    }
}


// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAACopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_AlbumArtistComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_AlbumArtist = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnArCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_ArtistComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_ArtistName = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_AlbumComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_AlbumName = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnTiCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_TitleTextCtrl->GetLineText( 0 );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_SongName = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnCoCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_CompComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_Composer = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnNuCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    //int CurData;
    //NumberTextCtrl->GetLineText( 0 ).ToLong( ( long int *) &CurData );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_Number = ( index + 1 );
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnDiCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_DiskTextCtrl->GetLineText( 0 );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_Disk = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnGeCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_GenreComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_GenreName = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnYeCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    long Year;
    m_YearTextCtrl->GetLineText( 0 ).ToLong( &Year );
    //guLogMessage( wxT( "Year set to : %u" ), Year );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_Year = Year;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnRaCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_Rating = m_Rating->GetRating();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnCommentCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_CommentText->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_Comments = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnRatingChanged( guRatingEvent &event )
{
    m_RatingChanged = true;
    m_CurrentRating = event.GetInt();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::RefreshImage( void )
{
    wxImage Image;
    wxImage * pCurImage = NULL;
    if( ( m_CurItem >= 0 ) && ( pCurImage = ( * m_Images )[ m_CurItem ] ) )
    {
        Image = * pCurImage;
    }
    else
    {
        Image = guImage( guIMAGE_INDEX_no_cover );
    }
    Image.Rescale( 250, 250, wxIMAGE_QUALITY_HIGH );
    m_PictureBitmap->SetBitmap( Image );

    if( m_CurItem >= 0 )
    {
        if( pCurImage )
        {
            m_AddPicButton->Enable( false );
            m_DelPicButton->Enable( true );
            m_SavePicButton->Enable( true );
            m_SearchPicButton->Enable( false );
        }
        else
        {
            m_AddPicButton->Enable( true );
            m_DelPicButton->Enable( false );
            m_SavePicButton->Enable( false );
            m_SearchPicButton->Enable( true );
        }
        m_CopyPicButton->Enable( true );
    }
    else
    {
        m_AddPicButton->Enable( false );
        m_DelPicButton->Enable( false );
        m_SavePicButton->Enable( false );
        m_SearchPicButton->Enable( false );
        m_CopyPicButton->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAddImageClicked( wxCommandEvent &event )
{
    wxASSERT( m_CurItem >= 0 );

    wxFileDialog * FileDialog = new wxFileDialog( this,
        wxT( "Select the filename to save" ),
        wxPathOnly( ( * m_Items )[ m_CurItem ].m_FileName ),
        wxT( "cover.jpg" ),
        wxT( "*.jpg;*.png" ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_PREVIEW );

    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            //CoverImage->SaveFile( AlbumPath + wxT( "cover.jpg" ), wxBITMAP_TYPE_JPEG );
            wxString FileName = FileDialog->GetPath();
            //guLogMessage( wxT( "File Open : '%s'" ), FileName.c_str() );
            wxImage * pCurImage = new wxImage();
            pCurImage->LoadFile( FileName );
            if( pCurImage->IsOk() )
            {
                ( * m_Images )[ m_CurItem ] = pCurImage;
                RefreshImage();
            }
        }
        FileDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnDelImageClicked( wxCommandEvent &event )
{
    wxASSERT( m_CurItem >= 0 );
    wxImage * pCurImage = ( * m_Images )[ m_CurItem ];
    ( * m_Images )[ m_CurItem ] = NULL;
    delete pCurImage;
    RefreshImage();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnSaveImageClicked( wxCommandEvent &event )
{
    wxASSERT( m_CurItem >= 0 );

    wxImage * pCurImage = ( * m_Images )[ m_CurItem ];
    wxASSERT( pCurImage );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
    wxString CoverName = ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );

    wxFileDialog * FileDialog = new wxFileDialog( this,
        wxT( "Select the filename to save" ),
        wxPathOnly( ( * m_Items )[ m_CurItem ].m_FileName ),
        CoverName,
        wxT( "*.jpg;*.png" ),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            //CoverImage->SaveFile( AlbumPath + wxT( "cover.jpg" ), wxBITMAP_TYPE_JPEG );
            wxString FileName = FileDialog->GetPath();
            //guLogMessage( wxT( "File Save to : '%s'" ), FileName.c_str() );
            if( FileName.EndsWith( wxT( ".png" ) ) )
            {
                pCurImage->SaveFile( FileName, wxBITMAP_TYPE_PNG );
            }
            else
            {
                pCurImage->SaveFile( FileName, wxBITMAP_TYPE_JPEG );
            }
        }
        FileDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnSearchImageClicked( wxCommandEvent &event )
{
    wxASSERT( m_CurItem >= 0 );

    wxString AlbumName = ( * m_Items )[ m_CurItem ].m_AlbumName;
    wxString ArtistName = ( * m_Items )[ m_CurItem ].m_ArtistName;

    AlbumName = RemoveSearchFilters( AlbumName );

    guCoverEditor * CoverEditor = new guCoverEditor( this, ArtistName, AlbumName );
    if( CoverEditor )
    {
        if( CoverEditor->ShowModal() == wxID_OK )
        {
            wxImage * SelectedCover = CoverEditor->GetSelectedCoverImage();
            if( SelectedCover )
            {
                ( * m_Images )[ m_CurItem ] = new wxImage( * SelectedCover );
                RefreshImage();
            }
        }
        CoverEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnCopyImageClicked( wxCommandEvent &event )
{
    wxASSERT( m_CurItem >= 0 );
    wxImage * pCurImage = ( * m_Images )[ m_CurItem ];
    wxASSERT( pCurImage );

    int index;
    int count = m_Images->Count();
    for( index = 0; index < count; index++ )
    {
        if( index != m_CurItem )
        {
            if( ( * m_Images )[ index ] )
                delete ( * m_Images )[ index ];
            ( * m_Images )[ index ] = pCurImage ? ( new wxImage( * pCurImage ) ) : NULL;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::FinishedMusicBrainzSearch( void )
{
    m_MBrainzThreadMutex.Lock();
    m_MBrainzThread = NULL;
    m_MBrainzThreadMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAddButtonClicked( wxCommandEvent &event )
{
    if( m_MBQuerySetArtistEnabled )
        m_MBQuerySetArtistEnabled = false;

    if( !m_MBQueryArtistTextCtrl->IsEmpty() ||
        !m_MBQueryTitleTextCtrl->IsEmpty() )
    {
        guMBReleaseArray * Releases = new guMBReleaseArray();
        guMusicBrainz * MusicBrainz = new guMusicBrainz();

        wxASSERT( Releases );
        wxASSERT( MusicBrainz );

        wxSetCursor( * wxHOURGLASS_CURSOR );
        m_MBrainzAddButton->Enable( false );
        wxTheApp->Yield();

        MusicBrainz->GetReleases( Releases, m_MBQueryArtistTextCtrl->GetValue(), m_MBQueryTitleTextCtrl->GetValue() );

        if( m_MBrainzReleases && m_MBrainzReleases->Count() )
        {
            m_MBrainzReleases->Empty();
            m_MBrainzAlbumChoice->Clear();
            //m_MBrainzAlbumChoice->
        }

        int Index;
        int Count = Releases->Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guMBRelease * Release = &( * Releases )[ Index ];
            //guLogMessage( wxT( "Release %u : %s" ), Index, Release->m_Id.c_str() );
            if( FindMBrainzReleaseId( Release->m_Id ) == wxNOT_FOUND )
            {
                guMBRelease * NewRelease = new guMBRelease();
                MusicBrainz->GetRelease( NewRelease, Release->m_Id );
                m_MBrainzReleases->Add( NewRelease );
                m_MBrainzAlbumChoice->Append( NewRelease->m_Title );

                if( m_MBrainzAlbumChoice->GetSelection() == wxNOT_FOUND )
                {
                    m_MBrainzAlbumChoice->SetSelection( 0 );
                    event.SetInt( 0 );
                    OnMBrainzAlbumChoiceSelected( event );

                    m_MBrainzAddButton->SetBitmapLabel( guBitmap( guIMAGE_INDEX_tiny_search_again ) );
                }
            }
            wxTheApp->Yield();
        }
        m_MBrainzAddButton->Enable( true );
        wxSetCursor( * wxSTANDARD_CURSOR );

        delete MusicBrainz;
        delete Releases;
    }
    else if( m_MBrainzCurTrack < ( int ) m_Items->Count() )
    {
        if( wxFileExists( ( * m_Items )[ m_MBrainzCurTrack ].m_FileName ) )
        {
            wxSetCursor( * wxHOURGLASS_CURSOR );
            m_MBrainzAddButton->Enable( false );
            m_MBrainzThreadMutex.Lock();
            if( m_MBrainzThread )
            {
                m_MBrainzThread->Pause();
                m_MBrainzThread->Delete();
            }
            m_MBrainzThread = new guMusicBrainzMetadataThread( this, m_MBrainzCurTrack );
            m_MBrainzThreadMutex.Unlock();
        //guLogMessage( wxT( "Albums search thread created" ) );
        }
        else
        {
            guLogError( wxT( "Could not find the track '%s'" ), ( * m_Items )[ m_CurItem ].m_FileName.c_str() );
        }
        m_MBrainzCurTrack++;
    }
}

// -------------------------------------------------------------------------------- //
int guTrackEditor::FindMBrainzReleaseId( const wxString releaseid )
{
    int Index;
    int Count;
//    if( !m_MBrainzReleases )
//    {
//        m_MBrainzReleases = new guMBReleaseArray();
//    }
    if( ( Count = m_MBrainzReleases->Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_MBrainzReleases->Item( Index ).m_Id == releaseid )
            {
                return Index;
            }
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAlbumsFound( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnMBrainzAlbumsFound..." ) );
    //
    guMBTrackArray * Tracks = ( guMBTrackArray * ) event.GetClientData();
    if( Tracks )
    {
        int Index;
        int Count;
        if( ( Count = Tracks->Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                guMBTrack * MBTrack = &Tracks->Item( Index );
                if( FindMBrainzReleaseId( MBTrack->m_ReleaseId ) == wxNOT_FOUND )
                {
                    m_MBrainzAlbumChoice->Append( Tracks->Item( Index ).m_ReleaseName );
                    guMBRelease * MBRelease = new guMBRelease();
                    guMusicBrainz * MusicBrainz = new guMusicBrainz();
                    MusicBrainz->GetRelease( MBRelease, Tracks->Item( Index ).m_ReleaseId );
                    m_MBrainzReleases->Add( MBRelease );
//                    // Check the release track count and the track lengths
//                    // to mark the items as very possible to be wrong
//                    if( CheckTracksLengths( MBRelease->m_Tracks, m_Items ) ||
//                        MBRelease->m_Tracks.Count() != m_Items->Count() )
//                    {
//                        m_MBrainzAlbumChoice->Set
//                    }
                    delete MusicBrainz;
                }
            }
            if( m_MBrainzAlbumChoice->GetSelection() == wxNOT_FOUND )
            {
                m_MBrainzAlbumChoice->SetSelection( 0 );
                event.SetInt( 0 );
                OnMBrainzAlbumChoiceSelected( event );
            }
            m_MBrainzAddButton->SetBitmapLabel( guBitmap( guIMAGE_INDEX_tiny_search_again ) );
        }
        m_MBrainzAddButton->Enable( true );

        delete Tracks;
    }
    wxSetCursor( *wxSTANDARD_CURSOR );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::UpdateMBrainzTrackInfo( void )
{
    if( m_MBrainzCurAlbum >= 0 && m_CurItem >= 0 )
    {
        guMBTrackArray * MBTracks = &m_MBrainzReleases->Item( m_MBrainzCurAlbum ).m_Tracks;
        if( m_CurItem < ( int ) MBTracks->Count() )
        {
            guMBTrack * MBTrack = &MBTracks->Item( m_CurItem );
            guTrack * Track = &m_Items->Item( m_CurItem );

            // Artist
            if( !MBTrack->m_ArtistName.IsEmpty() )
            {
                m_MBrainzArtistStaticText->SetForegroundColour( Track->m_ArtistName == MBTrack->m_ArtistName ?
                                        m_NormalColor : m_ErrorColor );
                m_MBrainzArtistTextCtrl->SetValue( MBTrack->m_ArtistName );
            }
            else
            {
                m_MBrainzArtistStaticText->SetForegroundColour( Track->m_ArtistName == m_MBrainzReleases->Item( m_MBrainzCurAlbum ).m_ArtistName ?
                                        m_NormalColor : m_ErrorColor );
                m_MBrainzArtistTextCtrl->SetValue( m_MBrainzReleases->Item( m_MBrainzCurAlbum ).m_ArtistName );
            }

            // Album Artist
            m_MBrainzAlbumArtistStaticText->SetForegroundColour( Track->m_AlbumArtist == m_MBrainzReleases->Item( m_MBrainzCurAlbum ).m_ArtistName ?
                                        m_NormalColor : m_ErrorColor );

            // ALbum
            m_MBrainzAlbumStaticText->SetForegroundColour( Track->m_AlbumName == m_MBrainzReleases->Item( m_MBrainzCurAlbum ).m_Title ?
                                        m_NormalColor : m_ErrorColor );

            // Title
            m_MBrainzTitleStaticText->SetForegroundColour( Track->m_SongName == MBTrack->m_Title ?
                                        m_NormalColor : m_ErrorColor );
            m_MBrainzTitleTextCtrl->SetValue( MBTrack->m_Title );

            // Year
            if( Track->m_Year )
            {
                m_MBrainzDateStaticText->SetForegroundColour( m_MBrainzDateChoice->GetStringSelection().Find( wxString::Format( wxT( "%u" ), Track->m_Year ) ) == wxNOT_FOUND ?
                                        m_ErrorColor : m_NormalColor );
            }
            else
            {
                m_MBrainzDateStaticText->SetForegroundColour( m_MBrainzDateChoice->GetStringSelection().IsEmpty() ?
                                        m_NormalColor : m_ErrorColor );
            }
            // Length
            m_MBrainzLengthStaticText->SetForegroundColour(
                    GetTrackLengthDiff( Track->m_Length * 1000, MBTrack->m_Length ) > guMBRAINZ_MAX_TIME_DIFF ?
                                        m_ErrorColor : m_NormalColor );
            m_MBrainzLengthTextCtrl->SetValue( LenToString( ( unsigned int ) MBTrack->m_Length / 1000 ) );

            // Number
            m_MBrainzNumberStaticText->SetForegroundColour( Track->m_Number == MBTrack->m_Number ?
                                        m_NormalColor : m_ErrorColor );
            m_MBrainzNumberTextCtrl->SetValue( wxString::Format( wxT( "%u" ), MBTrack->m_Number ) );

            return;
        }
    }
    m_MBrainzTitleTextCtrl->SetValue( wxEmptyString );
    m_MBrainzNumberTextCtrl->SetValue( wxEmptyString );
}

// -------------------------------------------------------------------------------- //
int guTrackEditor::CheckTracksLengths( guMBTrackArray * mbtracks, guTrackArray * tracks )
{
    int RetVal = 0;
    int Index;
    int Count = wxMin( tracks->Count(), mbtracks->Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        if( GetTrackLengthDiff( tracks->Item( Index ).m_Length * 1000,
                             mbtracks->Item( Index ).m_Length ) > guMBRAINZ_MAX_TIME_DIFF )
        {
            RetVal++;
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::MBCheckCountAndLengths( void )
{
    if( m_MBrainzReleases && m_MBrainzReleases->Count() )
    {
        guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
        // Check the number of tracks
        wxString InfoText;
        if( MBRelease->m_Tracks.Count() != m_Items->Count() )
        {
            InfoText = wxString::Format( _( "Error: The album have %u tracks and you are editing %u" ),
                            MBRelease->m_Tracks.Count(), m_Items->Count() );
        }
        if( CheckTracksLengths( &MBRelease->m_Tracks, m_Items ) )
        {
            InfoText += _( "\n"
                             "Warning: The length of some edited tracks don't match" );
        }
        m_MBrainzInfoStaticText->SetLabel( InfoText );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAlbumChoiceSelected( wxCommandEvent &event )
{
    //guLogMessage( wxT( "MusicBrainzAlbumSelected..." ) );
    m_MBrainzCurAlbum = event.GetInt();
    if( m_MBrainzCurAlbum >= 0 )
    {
        guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
        m_MBrainzArtistTextCtrl->SetValue( MBRelease->m_ArtistName );
        m_MBrainzAlbumArtistTextCtrl->SetValue( MBRelease->m_ArtistName );
        m_MBrainzDateChoice->Clear();
        m_MBrainzDateChoice->Append( MBRelease->m_Events );
        m_MBrainzDateChoice->SetSelection( 0 );
        UpdateMBrainzTrackInfo();
        m_MBrainzCopyButton->Enable( true );
        m_MBrainzAlbumTextCtrl->SetValue( MBRelease->m_Title );

        MBCheckCountAndLengths();
    }
    else
    {
        m_MBrainzCopyButton->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzCopyButtonClicked( wxCommandEvent &event )
{
    if( !( m_MBrainzReleases && m_MBrainzReleases->Count() && m_MBrainzCurAlbum >= 0 ) )
        return;

    guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
    int Index;
    int Count = wxMin( m_Items->Count(), MBRelease->m_Tracks.Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBTrack * MBTrack = &MBRelease->m_Tracks[ Index ];
        Track->m_ArtistName = MBTrack->m_ArtistName.IsEmpty() ? MBRelease->m_ArtistName : MBTrack->m_ArtistName;
        Track->m_AlbumArtist = MBRelease->m_ArtistName;
        Track->m_AlbumName = MBRelease->m_Title;
        Track->m_SongName = MBTrack->m_Title;
        Track->m_Number = MBTrack->m_Number;
        if( m_MBrainzDateChoice->GetCount() )
        {
            wxRegEx RegEx( wxT( "[0-9]{4}" ), wxRE_EXTENDED );
            if( RegEx.IsValid() && RegEx.Matches( m_MBrainzDateChoice->GetStringSelection() ) && RegEx.GetMatchCount() == 1 )
            {
                long LongValue;
                RegEx.GetMatch( m_MBrainzDateChoice->GetStringSelection(), 0 ).ToLong( &LongValue );
                Track->m_Year = LongValue;
            }
        }
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzArtistCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBrainzReleases && m_MBrainzReleases->Count() && m_MBrainzCurAlbum >= 0 ) )
        return;
    guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
    int Index;
    int Count = wxMin( m_Items->Count(), MBRelease->m_Tracks.Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBTrack * MBTrack = &MBRelease->m_Tracks[ Index ];
        Track->m_ArtistName = MBTrack->m_ArtistName.IsEmpty() ? MBRelease->m_ArtistName : MBTrack->m_ArtistName;
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAlbumArtistCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBrainzReleases && m_MBrainzReleases->Count() && m_MBrainzCurAlbum >= 0 ) )
        return;
    guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
    int Index;
    int Count = wxMin( m_Items->Count(), MBRelease->m_Tracks.Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        //guMBTrack * MBTrack = &MBRelease->m_Tracks[ Index ];
        Track->m_AlbumArtist = MBRelease->m_ArtistName;
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAlbumCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBrainzReleases && m_MBrainzReleases->Count() && m_MBrainzCurAlbum >= 0 ) )
        return;
    guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
    int Index;
    int Count = wxMin( m_Items->Count(), MBRelease->m_Tracks.Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        //guMBTrack * MBTrack = &MBRelease->m_Tracks[ Index ];
        Track->m_AlbumName = MBRelease->m_Title;
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzDateCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBrainzReleases && m_MBrainzReleases->Count() && m_MBrainzCurAlbum >= 0 ) )
        return;
    guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
    int Index;
    int Count = wxMin( m_Items->Count(), MBRelease->m_Tracks.Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        //guMBTrack * MBTrack = &MBRelease->m_Tracks[ Index ];
        if( m_MBrainzDateChoice->GetCount() )
        {
            wxRegEx RegEx( wxT( "[0-9]{4}" ), wxRE_EXTENDED );
            if( RegEx.IsValid() && RegEx.Matches( m_MBrainzDateChoice->GetStringSelection() ) && RegEx.GetMatchCount() == 1 )
            {
                long LongValue;
                RegEx.GetMatch( m_MBrainzDateChoice->GetStringSelection(), 0 ).ToLong( &LongValue );
                Track->m_Year = LongValue;
            }
        }
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzTitleCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBrainzReleases && m_MBrainzReleases->Count() && m_MBrainzCurAlbum >= 0 ) )
        return;
    guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
    int Index;
    int Count = wxMin( m_Items->Count(), MBRelease->m_Tracks.Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBTrack * MBTrack = &MBRelease->m_Tracks[ Index ];
        Track->m_SongName = MBTrack->m_Title;
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzNumberCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBrainzReleases && m_MBrainzReleases->Count() && m_MBrainzCurAlbum >= 0 ) )
        return;
    guMBRelease * MBRelease = &m_MBrainzReleases->Item( m_MBrainzCurAlbum );
    int Index;
    int Count = wxMin( m_Items->Count(), MBRelease->m_Tracks.Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBTrack * MBTrack = &MBRelease->m_Tracks[ Index ];
        Track->m_Number = MBTrack->m_Number;
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBQueryClearButtonClicked( wxCommandEvent &event )
{
    if( m_MBQuerySetArtistEnabled )
        m_MBQuerySetArtistEnabled = false;
    m_MBQueryArtistTextCtrl->SetValue( wxEmptyString );
    m_MBQueryTitleTextCtrl->SetValue( wxEmptyString );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBQueryTextCtrlChanged( wxCommandEvent& event )
{
    if( m_MBQuerySetArtistEnabled )
        m_MBQuerySetArtistEnabled = false;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::SongListSplitterOnIdle( wxIdleEvent& )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_SongListSplitter->SetSashPosition( Config->ReadNum( wxT( "TrackEditSashPos" ), 200, wxT( "Positions" ) ) );
    m_SongListSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guTrackEditor::SongListSplitterOnIdle ), NULL, this );

    m_GetComboDataThread = new guTrackEditorGetComboDataThread( this, m_Db );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnTextUpdated( wxCommandEvent& event )
{
    bool Enabled = !m_LyricArtistTextCtrl->IsEmpty() &&
        !m_LyricTrackTextCtrl->IsEmpty();

    m_LyricReloadButton->Enable( Enabled );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnSearchLyrics( wxCommandEvent &event )
{
    if( m_LyricArtistTextCtrl->IsEmpty() ||
        m_LyricTrackTextCtrl->IsEmpty() )
        return;

    if( !m_LyricSearchEngine )
    {
        m_LyricSearchEngine = ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->LyricSearchEngine();
    }

    if( m_LyricSearchEngine )
    {
        if( !m_LyricSearchContext )
        {
            guTrack Track = m_Items->Item(  m_CurItem );
            Track.m_ArtistName = m_LyricArtistTextCtrl->GetValue();
            Track.m_SongName = m_LyricTrackTextCtrl->GetValue();
            m_LyricSearchContext = m_LyricSearchEngine->CreateContext( this, &Track, false );
        }
        if( m_LyricSearchContext )
        {
            m_LyricSearchEngine->SearchStart( m_LyricSearchContext );
        }
    }

    m_LyricReloadButton->Enable( false );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnDownloadedLyric( wxCommandEvent &event )
{
    wxString * Content = ( wxString * ) event.GetClientData();
    if( Content )
    {
        m_LyricsTextCtrl->SetValue( * Content );
        ( * m_Lyrics )[ m_CurItem ] = * Content;
        delete Content;
    }
    m_LyricReloadButton->Enable( true );
}

// -------------------------------------------------------------------------------- //
void inline guUpdateComboBoxEntries( wxComboBox * combobox, wxSortedArrayString &itemlist )
{
    combobox->Clear();
    wxString FilterText = combobox->GetValue().Lower();
    if( FilterText.IsEmpty() )
    {
        combobox->Append( itemlist );
    }
    else
    {
        int index;
        int count = itemlist.Count();
        wxArrayString SetItems;
        for( index = 0; index < count; index++ )
        {
            if( itemlist[ index ].Lower().Find( FilterText ) != wxNOT_FOUND )
            {
                SetItems.Add( itemlist[ index ] );
            }
        }
        combobox->Append( SetItems );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnArtistTextChanged( wxCommandEvent &event )
{
    m_ArtistChanged = true;
    guUpdateComboBoxEntries( m_ArtistComboBox, m_Artists );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlbumArtistTextChanged( wxCommandEvent &event )
{
    m_AlbumArtistChanged = true;
    guUpdateComboBoxEntries( m_AlbumArtistComboBox, m_AlbumArtists );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlbumTextChanged( wxCommandEvent &event )
{
    m_AlbumChanged = true;
    guUpdateComboBoxEntries( m_AlbumComboBox, m_Albums );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnComposerTextChanged( wxCommandEvent &event )
{
    m_CompChanged = true;
    guUpdateComboBoxEntries( m_CompComboBox, m_Composers );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnGenreTextChanged( wxCommandEvent &event )
{
    m_GenreChanged = true;
    guUpdateComboBoxEntries( m_GenreComboBox, m_Genres );
}

// -------------------------------------------------------------------------------- //
// guMusicBrainzMetadataThread
// -------------------------------------------------------------------------------- //
guMusicBrainzMetadataThread::guMusicBrainzMetadataThread( guTrackEditor * trackeditor, int trackindex ) : wxThread()
{
    m_TrackEditor = trackeditor;
    m_Track = &( * m_TrackEditor->m_Items )[ trackindex ];

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guMusicBrainzMetadataThread::~guMusicBrainzMetadataThread()
{
    if( !TestDestroy() )
        m_TrackEditor->FinishedMusicBrainzSearch();
}

// -------------------------------------------------------------------------------- //
guMusicBrainzMetadataThread::ExitCode guMusicBrainzMetadataThread::Entry()
{
    if( !TestDestroy() )
    {
        guMBTrackArray * FoundTracks = new guMBTrackArray();
        guMusicBrainz * MusicBrainz = new guMusicBrainz();

        if( !FoundTracks || !MusicBrainz )
        {
            guLogError( wxT( "Could not create Musicbrainz object" ) );
            return 0;
        }

        MusicBrainz->GetTracks( FoundTracks, m_Track, m_Track->m_Length * 1000 );

        wxCommandEvent event( guTrackEditEvent, guTRACKEDIT_EVENT_MBRAINZ_TRACKS );
        event.SetClientData( FoundTracks );
        wxPostEvent( m_TrackEditor, event );
        //guLogMessage( wxT( "The event was sent..." ) );

        delete MusicBrainz;
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
// guTrackEditorGetComboDataThread
// -------------------------------------------------------------------------------- //
guTrackEditorGetComboDataThread::guTrackEditorGetComboDataThread( guTrackEditor * editor, guDbLibrary * db ) : wxThread()
{
    m_TrackEditor = editor;
    m_Db = db;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guTrackEditorGetComboDataThread::~guTrackEditorGetComboDataThread()
{
}

// -------------------------------------------------------------------------------- //
void guTrackEditorGetComboDataThread::FillArrayStrings( wxSortedArrayString &array, const guListItems &items )
{
    int Index;
    int Count;
    Count = items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        wxString CurText = items[ Index ].m_Name;
        if( !CurText.IsEmpty() && array.Index( CurText ) == wxNOT_FOUND )
            array.Add( CurText );
        if( TestDestroy() )
            break;
    }
}

// -------------------------------------------------------------------------------- //
guTrackEditorGetComboDataThread::ExitCode guTrackEditorGetComboDataThread::Entry()
{
    if( TestDestroy() )
        return 0;

    guListItems Artists;
    m_Db->GetArtists( &Artists, true );
    FillArrayStrings( m_TrackEditor->m_Artists, Artists );
    if( TestDestroy() )
        return 0;
    m_TrackEditor->UpdateArtists();

    //
    guListItems AlbumArtists;
    m_Db->GetAlbumArtists( &AlbumArtists, true );
    if( TestDestroy() )
        return 0;
    FillArrayStrings( m_TrackEditor->m_AlbumArtists, AlbumArtists );
    if( TestDestroy() )
        return 0;
    m_TrackEditor->UpdateAlbumArtists();

    //
    //
    guListItems Albums;
    m_Db->GetAlbums( &Albums, true );
    if( TestDestroy() )
        return 0;
    FillArrayStrings( m_TrackEditor->m_Albums, Albums );
    if( TestDestroy() )
        return 0;
    m_TrackEditor->UpdateAlbums();

    //
    //
    guListItems Composers;
    m_Db->GetComposers( &Composers, true );
    if( TestDestroy() )
        return 0;
    FillArrayStrings( m_TrackEditor->m_Composers, Composers );
    if( TestDestroy() )
        return 0;
    m_TrackEditor->UpdateComposers();

    //
    //
    guListItems Genres;
    m_Db->GetGenres( &Genres, true );
    if( TestDestroy() )
        return 0;
    FillArrayStrings( m_TrackEditor->m_Genres, Genres );
    if( TestDestroy() )
        return 0;
    m_TrackEditor->UpdateGenres();

    return 0;
}

// -------------------------------------------------------------------------------- //
