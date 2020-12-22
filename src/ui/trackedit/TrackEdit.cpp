// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "TrackEdit.h"

#include "EventCommandIds.h"
#include "Config.h"
#include "CoverEdit.h"
#include "Images.h"
#include "LastFM.h"
#include "MainFrame.h"
#include "Utils.h"

#include "wx/datetime.h"
#include <wx/notebook.h>
#include <wx/regex.h>

namespace Guayadeque {

#define guTRACKEDIT_TIMER_SELECTION_ID        10
#define guTRACKEDIT_TIMER_SELECTION_TIME      50
#define guTRACKEDIT_TIMER_TEXTCHANGED_TIME    500
#define guTRACKEDIT_TIMER_ARTIST_ID           11
#define guTRACKEDIT_TIMER_ALBUMARTIST_ID      12
#define guTRACKEDIT_TIMER_ALBUM_ID            13
#define guTRACKEDIT_TIMER_COMPOSER_ID         14
#define guTRACKEDIT_TIMER_GENRE_ID            15

const wxEventType guTrackEditEvent = wxNewEventType();

// -------------------------------------------------------------------------------- //
void guImagePtrArrayClean( guImagePtrArray * images )
{
    int Count;
    if( images && ( Count = images->Count() ) )
    {
        for( int Index = 0; Index < Count; Index++ )
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
    guImagePtrArray * images, wxArrayString * lyrics, wxArrayInt * changedflags ) :
    m_SelectedTimer( this, guTRACKEDIT_TIMER_SELECTION_ID ),
    m_ArtistChangedTimer( this, guTRACKEDIT_TIMER_ARTIST_ID ),
    m_AlbumArtistChangedTimer( this, guTRACKEDIT_TIMER_ALBUMARTIST_ID ),
    m_AlbumChangedTimer( this, guTRACKEDIT_TIMER_ALBUM_ID ),
    m_ComposerChangedTimer( this, guTRACKEDIT_TIMER_COMPOSER_ID ),
    m_GenreChangedTimer( this, guTRACKEDIT_TIMER_GENRE_ID )
{
    int index;
    int count;
    m_Db = db;

    wxPanel *           SongListPanel;
    wxStaticText *      ArStaticText;
    wxStaticText *      AlStaticText;
    wxStaticText *      TiStaticText;
    wxStaticText *      NuStaticText;
    wxStaticText *      GeStaticText;
    wxStaticText *      YeStaticText;
    wxPanel *           PicturePanel;
    wxPanel *           MBrainzPanel;
    wxStaticText *      MBAlbumStaticText;
    wxStaticLine *      MBrainzStaticLine;

    m_LyricSearchEngine = NULL;
    m_LyricSearchContext = NULL;
    m_GetComboDataThread = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( CONFIG_KEY_POSITIONS_TRACKEDIT_POSX, -1, CONFIG_PATH_POSITIONS );
    WindowPos.y = Config->ReadNum( CONFIG_KEY_POSITIONS_TRACKEDIT_POSY, -1, CONFIG_PATH_POSITIONS );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( CONFIG_KEY_POSITIONS_TRACKEDIT_WIDTH, 625, CONFIG_PATH_POSITIONS );
    WindowSize.y = Config->ReadNum( CONFIG_KEY_POSITIONS_TRACKEDIT_HEIGHT, 440, CONFIG_PATH_POSITIONS );

    //wxDialog( parent, wxID_ANY, _( "Songs Editor" ), wxDefaultPosition, wxSize( 625, 440 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
    Create( parent, wxID_ANY, _( "Songs Editor" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

//  this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    m_SongListSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
    m_SongListSplitter->SetMinimumPaneSize( 150 );

    SongListPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    wxBoxSizer * SongsMainSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer * SongListSizer = new wxStaticBoxSizer( new wxStaticBox( SongListPanel, wxID_ANY, _( " Songs " ) ), wxHORIZONTAL );

    m_SongListBox = new wxListBox( SongListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
    SongListSizer->Add( m_SongListBox, 1, wxALL|wxEXPAND, 2 );

    wxBoxSizer * OrderSizer = new wxBoxSizer( wxVERTICAL );

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
    //MainDetailPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxScrolledWindow * MainDetailPanel = new wxScrolledWindow( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer * DetailSizer = new wxBoxSizer( wxVERTICAL );

    m_MainNotebook = new wxNotebook( MainDetailPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

    //
    // Details
    //
    wxPanel * DetailPanel = new wxPanel( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxSizer * MainDetailSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer * DataFlexSizer = new wxFlexGridSizer( 3, 0, 0 );
    DataFlexSizer->AddGrowableCol( 2 );
    DataFlexSizer->SetFlexibleDirection( wxBOTH );
    DataFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_ArCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_ArCopyButton->SetToolTip( _( "Copy the artist name to all the tracks you are editing" ) );
    DataFlexSizer->Add( m_ArCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    ArStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
    ArStaticText->Wrap( -1 );
    DataFlexSizer->Add( ArStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

    wxArrayString DummyArray;
    m_ArtistComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, DummyArray, wxCB_DROPDOWN );
    DataFlexSizer->Add( m_ArtistComboBox, 1, wxEXPAND|wxTOP|wxRIGHT, 5 );

    m_AACopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_AACopyButton->SetToolTip( _( "Copy the album artist name to all the tracks you are editing" ) );
    DataFlexSizer->Add( m_AACopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    wxStaticText * AAStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "A. Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
    AAStaticText->SetToolTip( _( "shows the album artist of the track" ) );
    AAStaticText->Wrap( -1 );
    DataFlexSizer->Add( AAStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

    m_AlbumArtistComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, DummyArray, wxCB_DROPDOWN );
    DataFlexSizer->Add( m_AlbumArtistComboBox, 1, wxEXPAND|wxTOP|wxRIGHT, 5 );

    m_AlCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_AlCopyButton->SetToolTip( _( "Copy the album name to all the tracks you are editing" ) );
    DataFlexSizer->Add( m_AlCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    AlStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Album:" ), wxDefaultPosition, wxDefaultSize, 0 );
    AlStaticText->Wrap( -1 );
    DataFlexSizer->Add( AlStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

    m_AlbumComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, DummyArray, wxCB_DROPDOWN );
    DataFlexSizer->Add( m_AlbumComboBox, 1, wxEXPAND|wxTOP|wxRIGHT, 5 );

    m_TiCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_TiCopyButton->SetToolTip( _( "Copy the title to all the tracks you are editing" ) );
    DataFlexSizer->Add( m_TiCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    TiStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Title:" ), wxDefaultPosition, wxDefaultSize, 0 );
    TiStaticText->Wrap( -1 );
    DataFlexSizer->Add( TiStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

    m_TitleTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    DataFlexSizer->Add( m_TitleTextCtrl, 0, wxEXPAND|wxTOP|wxRIGHT, 5 );

    m_CoCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_CoCopyButton->SetToolTip( _( "Copy the composer to all the tracks you are editing" ) );
    DataFlexSizer->Add( m_CoCopyButton, 0, wxTOP|wxRIGHT|wxLEFT, 5 );

    wxStaticText * CoStaticText;
    CoStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Composer:" ), wxDefaultPosition, wxDefaultSize, 0 );
    CoStaticText->Wrap( -1 );
    DataFlexSizer->Add( CoStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_CompComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, DummyArray, wxCB_DROPDOWN );
    DataFlexSizer->Add( m_CompComboBox, 1, wxEXPAND|wxTOP|wxRIGHT, 5 );

    m_CommentCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_CommentCopyButton->SetToolTip( _( "Copy the comment to all the tracks you are editing" ) );
    DataFlexSizer->Add( m_CommentCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    wxStaticText * CommentStaticText;
    CommentStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Comment:" ), wxDefaultPosition, wxDefaultSize, 0 );
    CommentStaticText->Wrap( -1 );
    DataFlexSizer->Add( CommentStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_CommentText = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, 54 ), wxTE_MULTILINE );
    DataFlexSizer->Add( m_CommentText, 1, wxEXPAND|wxTOP|wxRIGHT, 5 );


    m_NuCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_NuCopyButton->SetToolTip( _( "Copy the number to all the tracks you are editing" ) );
    DataFlexSizer->Add( m_NuCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    NuStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Number:" ), wxDefaultPosition, wxDefaultSize, 0 );
    NuStaticText->Wrap( -1 );
    DataFlexSizer->Add( NuStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

//  m_NumberTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
//  DataFlexSizer->Add( m_NumberTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

    wxBoxSizer* DiskSizer;
    DiskSizer = new wxBoxSizer( wxHORIZONTAL );

    m_NumberTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    DiskSizer->Add( m_NumberTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

    m_NuOrderButton= new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_numerate ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_NuOrderButton->SetToolTip( _( "Enumerate the tracks in the order they were added for editing" ) );
    DiskSizer->Add( m_NuOrderButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

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
    m_GeCopyButton->SetToolTip( _( "Copy the genre name to all songs you are editing" ) );
    DataFlexSizer->Add( m_GeCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    GeStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Genre:" ), wxDefaultPosition, wxDefaultSize, 0 );
    GeStaticText->Wrap( -1 );
    DataFlexSizer->Add( GeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxRIGHT, 5 );

    m_GenreComboBox = new wxComboBox( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, DummyArray, wxCB_DROPDOWN );
    DataFlexSizer->Add( m_GenreComboBox, 1, wxEXPAND|wxTOP|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_YeCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_YeCopyButton->SetToolTip( _( "Copy the year to all songs you are editing" ) );
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
    RatingSizer->Add( RaStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

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

//  m_EditPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_edit ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//  PictureButtonSizer->Add( m_EditPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    PictureButtonSizer->Add( 10, 0, 0, wxEXPAND, 5 );

    m_CopyPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_CopyPicButton->SetToolTip( _( "Copy the current picture to all the tracks you are editing" ) );
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

    wxArrayString m_MBAlbumChoiceChoices;
    m_MBAlbumChoice = new wxChoice( MBrainzPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MBAlbumChoiceChoices, 0 );
    m_MBAlbumChoice->SetToolTip( _( "Select the album found in musicbrainz" ) );
    m_MBAlbumChoice->SetSelection( 0 );

    MBrainzTopSizer->Add( m_MBAlbumChoice, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    m_MBAddButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBAddButton->SetToolTip( _( "Search albums in musicbrainz" ) );
    //m_MBAddButton->Enable( false );

    MBrainzTopSizer->Add( m_MBAddButton, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_MBCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBCopyButton->SetToolTip( _( "Copy the content of the album to the edited tracks" ) );
    m_MBCopyButton->Enable( false );

    MBrainzTopSizer->Add( m_MBCopyButton, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    MBrainzSizer->Add( MBrainzTopSizer, 0, wxEXPAND, 5 );

    wxStaticBoxSizer* MBDetailSizer;
    MBDetailSizer = new wxStaticBoxSizer( new wxStaticBox( MBrainzPanel, wxID_ANY, _( " Details " ) ), wxVERTICAL );

    wxFlexGridSizer* MBDetailFlexGridSizer;
    MBDetailFlexGridSizer = new wxFlexGridSizer( 3, 0, 0 );
    MBDetailFlexGridSizer->AddGrowableCol( 1 );
    MBDetailFlexGridSizer->SetFlexibleDirection( wxBOTH );
    MBDetailFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_MBArtistStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_MBArtistStaticText->Wrap( -1 );
    MBDetailFlexGridSizer->Add( m_MBArtistStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_MBArtistTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    MBDetailFlexGridSizer->Add( m_MBArtistTextCtrl, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    m_MBArCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBArCopyButton->SetToolTip( _( "Copy the artist to the edited tracks" ) );
    MBDetailFlexGridSizer->Add( m_MBArCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_MBAlbumArtistStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "A. Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_MBAlbumArtistStaticText->Wrap( -1 );
    MBDetailFlexGridSizer->Add( m_MBAlbumArtistStaticText, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_MBAlbumArtistTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    MBDetailFlexGridSizer->Add( m_MBAlbumArtistTextCtrl, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    m_MBAlArCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBAlArCopyButton->SetToolTip( _( "Copy the album artist to the edited tracks" ) );
    MBDetailFlexGridSizer->Add( m_MBAlArCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_MBAlbumStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Album:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MBAlbumStaticText->Wrap( -1 );
    MBDetailFlexGridSizer->Add( m_MBAlbumStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    m_MBAlbumTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    MBDetailFlexGridSizer->Add( m_MBAlbumTextCtrl, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    m_MBAlCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBAlCopyButton->SetToolTip( _( "Copy the album to the edited tracks" ) );
    MBDetailFlexGridSizer->Add( m_MBAlCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_MBYearStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "Year:" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_MBYearStaticText->Wrap( -1 );
    MBDetailFlexGridSizer->Add( m_MBYearStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    m_MBYearTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    MBDetailFlexGridSizer->Add( m_MBYearTextCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    m_MBDaCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBDaCopyButton->SetToolTip( _( "Copy the date to the edited tracks" ) );
    MBDetailFlexGridSizer->Add( m_MBDaCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_MBTitleStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _( "Title:" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_MBTitleStaticText->Wrap( -1 );
    MBDetailFlexGridSizer->Add( m_MBTitleStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    m_MBTitleTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    MBDetailFlexGridSizer->Add( m_MBTitleTextCtrl, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    m_MBTiCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBTiCopyButton->SetToolTip( _( "Copy the song names to the edited tracks" ) );
    MBDetailFlexGridSizer->Add( m_MBTiCopyButton, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_MBLengthStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MBLengthStaticText->Wrap( -1 );
    MBDetailFlexGridSizer->Add( m_MBLengthStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

    wxBoxSizer* MBNumberSizer;
    MBNumberSizer = new wxBoxSizer( wxHORIZONTAL );

    m_MBLengthTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    MBNumberSizer->Add( m_MBLengthTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

    MBNumberSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    m_MBNumberStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, _("Number:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MBNumberStaticText->Wrap( -1 );
    MBNumberSizer->Add( m_MBNumberStaticText, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_MBNumberTextCtrl = new wxTextCtrl( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    MBNumberSizer->Add( m_MBNumberTextCtrl, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    MBDetailFlexGridSizer->Add( MBNumberSizer, 1, wxEXPAND, 5 );

    m_MBNuCopyButton = new wxBitmapButton( MBrainzPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_MBNuCopyButton->SetToolTip( _( "Copy the number to the edited tracks" ) );
    MBDetailFlexGridSizer->Add( m_MBNuCopyButton, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

    MBDetailSizer->Add( MBDetailFlexGridSizer, 0, wxEXPAND, 5 );

    MBrainzStaticLine = new wxStaticLine( MBrainzPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    MBDetailSizer->Add( MBrainzStaticLine, 0, wxEXPAND | wxALL, 5 );

    m_MBInfoStaticText = new wxStaticText( MBrainzPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
    m_MBInfoStaticText->Wrap( 398 );
    MBDetailSizer->Add( m_MBInfoStaticText, 1, wxALL|wxEXPAND, 5 );

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
    ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
    ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
    ButtonsSizer->Realize();
    MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    this->SetSizer( MainSizer );
    this->Layout();

    MainDetailPanel->SetScrollRate( 20, 20 );


    ButtonsSizerOK->SetDefault();


    // --------------------------------------------------------------------
    m_NormalColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    m_ErrorColor = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );
    m_AcousticId = NULL;
    m_MBAlbums = NULL;
    m_MBCurTrack = 0;
    m_MBCurAlbum = wxNOT_FOUND;
    m_MBQuerySetArtistEnabled = true;
    m_CurItem = 0;
    m_NextItem = wxNOT_FOUND;
    m_Items = songs;
    m_Images = images;
    m_Lyrics = lyrics;
    m_ChangedFlags = changedflags;
    m_CurrentRating = -1;
    m_RatingChanged = false;
    m_GenreChanged = false;
    wxArrayString ItemsText;

    // Generate the list of tacks, pictures and lyrics
    count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        ItemsText.Add( ( * m_Items )[ index ].m_FileName );
        // Fill the initial Images of the files
        m_Images->Add( guTagGetPicture( ( * m_Items )[ index ].m_FileName ) );
        m_Lyrics->Add( guTagGetLyrics( ( * m_Items )[ index ].m_FileName ) );
        m_ChangedFlags->Add( guTRACK_CHANGED_DATA_NONE );
    }
    m_SongListBox->InsertItems( ItemsText, 0 );
    m_SongListBox->SetFocus();

    // Idle Events
    m_SongListSplitter->Bind( wxEVT_IDLE, &guTrackEditor::SongListSplitterOnIdle, this );

    // Bind Events
    Bind( wxEVT_BUTTON, &guTrackEditor::OnOKButton, this, wxID_OK );
    m_MainNotebook->Bind( wxEVT_NOTEBOOK_PAGE_CHANGED, &guTrackEditor::OnPageChanged, this );

    m_SongListBox->Bind( wxEVT_LISTBOX, &guTrackEditor::OnSongListBoxSelected, this );
    m_MoveUpButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMoveUpBtnClick, this );
    m_MoveDownButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMoveDownBtnClick, this );
    m_ArCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnArCopyButtonClicked, this );
    m_AACopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnAACopyButtonClicked, this );
    m_AlCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnAlCopyButtonClicked, this );
    m_TiCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnTiCopyButtonClicked, this );
    m_CoCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnCoCopyButtonClicked, this );
    m_NuCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnNuCopyButtonClicked, this );
    m_NuOrderButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnNuOrderButtonClicked, this );
    m_DiCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnDiCopyButtonClicked, this );
    m_GeCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnGeCopyButtonClicked, this );
    m_YeCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnYeCopyButtonClicked, this );
    m_RaCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnRaCopyButtonClicked, this );
    m_Rating->Bind( guEVT_RATING_CHANGED, &guTrackEditor::OnRatingChanged, this );
    m_CommentCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnCommentCopyButtonClicked, this );

    m_ArtistComboBox->Bind( wxEVT_TEXT, &guTrackEditor::OnArtistTextChanged, this );
    m_AlbumArtistComboBox->Bind( wxEVT_TEXT, &guTrackEditor::OnAlbumArtistTextChanged, this );
    m_AlbumComboBox->Bind( wxEVT_TEXT, &guTrackEditor::OnAlbumTextChanged, this );
    m_CompComboBox->Bind( wxEVT_TEXT, &guTrackEditor::OnComposerTextChanged, this );
    m_GenreComboBox->Bind( wxEVT_TEXT, &guTrackEditor::OnGenreTextChanged, this );

    m_AddPicButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnAddImageClicked, this );
    m_DelPicButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnDelImageClicked, this );
    m_SavePicButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnSaveImageClicked, this );
    m_SearchPicButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnSearchImageClicked, this );
    m_CopyPicButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnCopyImageClicked, this );

    m_LyricReloadButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnSearchLyrics, this );
    m_LyricArtistTextCtrl->Bind( wxEVT_TEXT, &guTrackEditor::OnTextUpdated, this );
    m_LyricTrackTextCtrl->Bind( wxEVT_TEXT, &guTrackEditor::OnTextUpdated, this );

    m_MBQueryArtistTextCtrl->Bind( wxEVT_TEXT, &guTrackEditor::OnMBQueryTextCtrlChanged, this );
    m_MBQueryTitleTextCtrl->Bind( wxEVT_TEXT, &guTrackEditor::OnMBQueryTextCtrlChanged, this );
    m_MBQueryClearButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBQueryClearButtonClicked, this );

    m_MBAddButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzAddButtonClicked, this );
    m_MBAlbumChoice->Bind( wxEVT_CHOICE, &guTrackEditor::OnMBrainzAlbumChoiceSelected, this );
    m_MBCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzCopyButtonClicked, this );

    m_MBArCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzArtistCopyButtonClicked, this );
    m_MBAlArCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzAlbumArtistCopyButtonClicked, this );
    m_MBAlCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzAlbumCopyButtonClicked, this );
    m_MBDaCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzYearCopyButtonClicked, this );
    m_MBTiCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzTitleCopyButtonClicked, this );
    m_MBNuCopyButton->Bind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzNumberCopyButtonClicked, this );

    Bind( wxEVT_MENU, &guTrackEditor::OnDownloadedLyric, this, ID_LYRICS_LYRICFOUND );

    Bind( wxEVT_TIMER, &guTrackEditor::OnSelectTimeout, this, guTRACKEDIT_TIMER_SELECTION_ID );
    Bind( wxEVT_TIMER, &guTrackEditor::OnArtistChangedTimeout, this, guTRACKEDIT_TIMER_ARTIST_ID );
    Bind( wxEVT_TIMER, &guTrackEditor::OnAlbumArtistChangedTimeout, this, guTRACKEDIT_TIMER_ALBUMARTIST_ID );
    Bind( wxEVT_TIMER, &guTrackEditor::OnAlbumChangedTimeout, this, guTRACKEDIT_TIMER_ALBUM_ID );
    Bind( wxEVT_TIMER, &guTrackEditor::OnComposerChangedTimeout, this, guTRACKEDIT_TIMER_COMPOSER_ID );
    Bind( wxEVT_TIMER, &guTrackEditor::OnGenreChangedTimeout, this, guTRACKEDIT_TIMER_GENRE_ID );

    //
    // Force the 1st listbox item to be selected
    m_SongListBox->SetSelection( 0 );
//    wxCommandEvent event( wxEVT_LISTBOX, m_SongListBox->GetId() );
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
    Config->WriteNum( CONFIG_KEY_POSITIONS_TRACKEDIT_SASHPOS, m_SongListSplitter->GetSashPosition(), CONFIG_PATH_POSITIONS );
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( CONFIG_KEY_POSITIONS_TRACKEDIT_POSX, WindowPos.x, CONFIG_PATH_POSITIONS );
    Config->WriteNum( CONFIG_KEY_POSITIONS_TRACKEDIT_POSY, WindowPos.y, CONFIG_PATH_POSITIONS );
    wxSize WindowSize = GetSize();
    Config->WriteNum( CONFIG_KEY_POSITIONS_TRACKEDIT_WIDTH, WindowSize.x, CONFIG_PATH_POSITIONS );
    Config->WriteNum( CONFIG_KEY_POSITIONS_TRACKEDIT_HEIGHT, WindowSize.y, CONFIG_PATH_POSITIONS );

    // Unbind Events
    Unbind( wxEVT_BUTTON, &guTrackEditor::OnOKButton, this, wxID_OK );
    m_MainNotebook->Unbind( wxEVT_NOTEBOOK_PAGE_CHANGED, &guTrackEditor::OnPageChanged, this );

    m_SongListBox->Unbind( wxEVT_LISTBOX, &guTrackEditor::OnSongListBoxSelected, this );
    m_MoveUpButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMoveUpBtnClick, this );
    m_MoveDownButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMoveDownBtnClick, this );
    m_ArCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnArCopyButtonClicked, this );
    m_AACopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnAACopyButtonClicked, this );
    m_AlCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnAlCopyButtonClicked, this );
    m_TiCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnTiCopyButtonClicked, this );
    m_CoCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnCoCopyButtonClicked, this );
    m_NuCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnNuCopyButtonClicked, this );
    m_NuOrderButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnNuOrderButtonClicked, this );
    m_DiCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnDiCopyButtonClicked, this );
    m_GeCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnGeCopyButtonClicked, this );
    m_YeCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnYeCopyButtonClicked, this );
    m_RaCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnRaCopyButtonClicked, this );
    m_Rating->Unbind( guEVT_RATING_CHANGED, &guTrackEditor::OnRatingChanged, this );
    m_CommentCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnCommentCopyButtonClicked, this );

    m_ArtistComboBox->Unbind( wxEVT_TEXT, &guTrackEditor::OnArtistTextChanged, this );
    m_AlbumArtistComboBox->Unbind( wxEVT_TEXT, &guTrackEditor::OnAlbumArtistTextChanged, this );
    m_AlbumComboBox->Unbind( wxEVT_TEXT, &guTrackEditor::OnAlbumTextChanged, this );
    m_CompComboBox->Unbind( wxEVT_TEXT, &guTrackEditor::OnComposerTextChanged, this );
    m_GenreComboBox->Unbind( wxEVT_TEXT, &guTrackEditor::OnGenreTextChanged, this );

    m_AddPicButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnAddImageClicked, this );
    m_DelPicButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnDelImageClicked, this );
    m_SavePicButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnSaveImageClicked, this );
    m_SearchPicButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnSearchImageClicked, this );
    m_CopyPicButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnCopyImageClicked, this );

    m_LyricReloadButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnSearchLyrics, this );
    m_LyricArtistTextCtrl->Unbind( wxEVT_TEXT, &guTrackEditor::OnTextUpdated, this );
    m_LyricTrackTextCtrl->Unbind( wxEVT_TEXT, &guTrackEditor::OnTextUpdated, this );

    m_MBQueryArtistTextCtrl->Unbind( wxEVT_TEXT, &guTrackEditor::OnMBQueryTextCtrlChanged, this );
    m_MBQueryTitleTextCtrl->Unbind( wxEVT_TEXT, &guTrackEditor::OnMBQueryTextCtrlChanged, this );
    m_MBQueryClearButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBQueryClearButtonClicked, this );

    m_MBAddButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzAddButtonClicked, this );
    m_MBAlbumChoice->Unbind( wxEVT_CHOICE, &guTrackEditor::OnMBrainzAlbumChoiceSelected, this );
    m_MBCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzCopyButtonClicked, this );

    m_MBArCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzArtistCopyButtonClicked, this );
    m_MBAlArCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzAlbumArtistCopyButtonClicked, this );
    m_MBAlCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzAlbumCopyButtonClicked, this );
    m_MBDaCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzYearCopyButtonClicked, this );
    m_MBTiCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzTitleCopyButtonClicked, this );
    m_MBNuCopyButton->Unbind( wxEVT_BUTTON, &guTrackEditor::OnMBrainzNumberCopyButtonClicked, this );

    Unbind( wxEVT_MENU, &guTrackEditor::OnDownloadedLyric, this, ID_LYRICS_LYRICFOUND );

    Unbind( wxEVT_TIMER, &guTrackEditor::OnSelectTimeout, this, guTRACKEDIT_TIMER_SELECTION_ID );
    Unbind( wxEVT_TIMER, &guTrackEditor::OnArtistChangedTimeout, this, guTRACKEDIT_TIMER_ARTIST_ID );
    Unbind( wxEVT_TIMER, &guTrackEditor::OnAlbumArtistChangedTimeout, this, guTRACKEDIT_TIMER_ALBUMARTIST_ID );
    Unbind( wxEVT_TIMER, &guTrackEditor::OnAlbumChangedTimeout, this, guTRACKEDIT_TIMER_ALBUM_ID );
    Unbind( wxEVT_TIMER, &guTrackEditor::OnComposerChangedTimeout, this, guTRACKEDIT_TIMER_COMPOSER_ID );
    Unbind( wxEVT_TIMER, &guTrackEditor::OnGenreChangedTimeout, this, guTRACKEDIT_TIMER_GENRE_ID );

    if( m_GetComboDataThread )
    {
        m_GetComboDataThread->Pause();
        m_GetComboDataThread->Delete();
    }

    if( m_LyricSearchContext )
        delete m_LyricSearchContext;

    if( m_AcousticId )
        delete m_AcousticId;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnPageChanged( wxNotebookEvent &event )
{
    //WriteItemData();

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
    m_NextItem = event.GetInt();

    m_SelectedTimer.Start( guTRACKEDIT_TIMER_SELECTION_TIME, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnSelectTimeout( wxTimerEvent &event )
{
    WriteItemData();
    m_CurItem = m_NextItem;
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
    int MovedFlag = ( * m_ChangedFlags )[ m_CurItem ];
    m_Images->RemoveAt( m_CurItem );
    m_Lyrics->RemoveAt( m_CurItem );
    m_ChangedFlags->RemoveAt( m_CurItem );
    m_SongListBox->SetString( m_CurItem, m_SongListBox->GetString( m_CurItem - 1 ) );
    m_CurItem--;
    m_Items->Insert( MovedTrack, m_CurItem );
    m_Images->Insert( MovedImage, m_CurItem );
    m_Lyrics->Insert( MovedLyric, m_CurItem );
    m_ChangedFlags->Insert( MovedFlag, m_CurItem );
    m_SongListBox->SetString( m_CurItem, FileName );

    m_SongListBox->SetSelection( m_CurItem );

    event.SetInt( m_CurItem );
    OnSongListBoxSelected( event );

}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMoveDownBtnClick( wxCommandEvent &event )
{
    wxString FileName = m_SongListBox->GetString( m_CurItem );
    guTrack * MovedTrack = m_Items->Detach( m_CurItem );
    wxImage * MovedImage = ( * m_Images )[ m_CurItem ];
    wxString MovedLyric = ( * m_Lyrics )[ m_CurItem ];
    int MovedFlag = ( * m_ChangedFlags )[ m_CurItem ];
    m_Images->RemoveAt( m_CurItem );
    m_Lyrics->RemoveAt( m_CurItem );
    m_ChangedFlags->RemoveAt( m_CurItem );
    m_SongListBox->SetString( m_CurItem, m_SongListBox->GetString( m_CurItem + 1 ) );
    m_CurItem++;
    m_Items->Insert( MovedTrack, m_CurItem );
    m_Images->Insert( MovedImage, m_CurItem );
    m_Lyrics->Insert( MovedLyric, m_CurItem );
    m_ChangedFlags->Insert( MovedFlag, m_CurItem );
    m_SongListBox->SetString( m_CurItem, FileName );

    m_SongListBox->SetSelection( m_CurItem );

    event.SetInt( m_CurItem );
    OnSongListBoxSelected( event );

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
            m_MBQueryArtistTextCtrl->SetValue( Track->m_AlbumArtist.IsEmpty() ? Track->m_ArtistName : Track->m_AlbumArtist );
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
void guTrackEditor::SetTagField( wxString &field, const wxString &newval, int &changedflags, const int flagval )
{
    if( field != newval )
    {
        field = newval;
        if( !( changedflags & flagval ) )
            changedflags |= flagval;
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::SetTagField( int &field, const int newval, int &changedflags, const int flagval )
{
    if( field != newval )
    {
        field = newval;
        if( !( changedflags & flagval ) )
            changedflags |= flagval;
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::WriteItemData( void )
{
    //guLogMessage( wxT( "WriteItemData: %i" ), m_CurItem );
    if( m_CurItem >= 0 )
    {
        int ChangedFlag = ( * m_ChangedFlags )[ m_CurItem ];
        long LongValue;

        guTrack &Track = ( * m_Items )[ m_CurItem ];

        if( m_AlbumArtistChanged )
        {
          SetTagField( Track.m_AlbumArtist, m_AlbumArtistComboBox->GetValue(), ChangedFlag );
        }

        if( m_ArtistChanged )
        {
          SetTagField( Track.m_ArtistName, m_ArtistComboBox->GetValue(), ChangedFlag );
        }

        if( m_AlbumChanged )
        {
          SetTagField( Track.m_AlbumName, m_AlbumComboBox->GetValue(), ChangedFlag );
        }

        if( m_TitleTextCtrl->IsModified() )
        {
          SetTagField( Track.m_SongName, m_TitleTextCtrl->GetValue(), ChangedFlag );
        }

        if( m_CompChanged )
        {
          SetTagField( Track.m_Composer, m_CompComboBox->GetValue(), ChangedFlag );
        }

        if( m_NumberTextCtrl->IsModified() )
        {
          m_NumberTextCtrl->GetValue().ToLong( &LongValue );
          SetTagField( Track.m_Number, LongValue, ChangedFlag );
        }

        if( m_DiskTextCtrl->IsModified() )
        {
          SetTagField( Track.m_Disk, m_DiskTextCtrl->GetValue(), ChangedFlag );
        }

        if( m_GenreChanged )
        {
          SetTagField( Track.m_GenreName, m_GenreComboBox->GetValue(), ChangedFlag );
        }

        if( m_YearTextCtrl->IsModified() )
        {
          m_YearTextCtrl->GetValue().ToLong( &LongValue );
          SetTagField( Track.m_Year, LongValue, ChangedFlag );
        }

        if( m_RatingChanged )
        {
          bool EmbeddRatings = Track.m_MediaViewer && Track.m_MediaViewer->GetEmbeddMetadata();
          SetTagField( Track.m_Rating, m_Rating->GetRating(), ChangedFlag,
                      EmbeddRatings ? guTRACK_CHANGED_DATA_TAGS | guTRACK_CHANGED_DATA_RATING : guTRACK_CHANGED_DATA_TAGS );
        }

        if( m_CommentText->IsModified() )
        {
          SetTagField( Track.m_Comments, m_CommentText->GetValue(), ChangedFlag );
        }

        if( m_LyricsTextCtrl->IsModified() )
        {
          SetTagField( ( * m_Lyrics )[ m_CurItem ], m_LyricsTextCtrl->GetValue(), ChangedFlag, guTRACK_CHANGED_DATA_LYRICS );
        }

        ( * m_ChangedFlags )[ m_CurItem ] = ChangedFlag;
    }
}


// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAACopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_AlbumArtistComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_AlbumArtist, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnArCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_ArtistComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_ArtistName, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_AlbumComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_AlbumName, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnTiCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_TitleTextCtrl->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_SongName, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnCoCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_CompComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_Composer, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnNuOrderButtonClicked( wxCommandEvent& event )
{
    int index;
    //int CurData;
    //NumberTextCtrl->GetValue().ToLong( ( long int *) &CurData );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_Number, ( index + 1 ), ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnNuCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    long CurData;
    m_NumberTextCtrl->GetValue().ToLong( &CurData );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_Number, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnDiCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_DiskTextCtrl->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_Disk, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnGeCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_GenreComboBox->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_GenreName, CurData, ( * m_ChangedFlags )[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnYeCopyButtonClicked( wxCommandEvent& event )
{
    long Year;
    wxString YearStr = m_YearTextCtrl->GetValue();
    if( YearStr.IsEmpty() || !YearStr.ToLong( &Year ) )
    {
        Year = 0;
    }

    //guLogMessage( wxT( "Year set to : %u" ), Year );
    int Count = m_Items->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        SetTagField( ( * m_Items )[ Index ].m_Year, Year, ( * m_ChangedFlags )[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnRaCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    int count = m_Items->Count();
    int CurData = m_Rating->GetRating();
    for( index = 0; index < count; index++ )
    {
        guTrack &Track = ( * m_Items )[ index ];
        bool EmbeddRatings = Track.m_MediaViewer && Track.m_MediaViewer->GetEmbeddMetadata();
        SetTagField( ( * m_Items )[ index ].m_Rating, CurData, ( * m_ChangedFlags )[ index ],
                EmbeddRatings ? guTRACK_CHANGED_DATA_TAGS | guTRACK_CHANGED_DATA_RATING : guTRACK_CHANGED_DATA_TAGS );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnCommentCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_CommentText->GetValue();
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        SetTagField( ( * m_Items )[ index ].m_Comments, CurData, ( * m_ChangedFlags )[ index ] );
    }
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
                if( !( ( * m_ChangedFlags )[ m_CurItem ] & guTRACK_CHANGED_DATA_IMAGES ) )
                    ( * m_ChangedFlags )[ m_CurItem ] |= guTRACK_CHANGED_DATA_IMAGES;
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
    if( !( ( * m_ChangedFlags )[ m_CurItem ] & guTRACK_CHANGED_DATA_IMAGES ) )
        ( * m_ChangedFlags )[ m_CurItem ] |= guTRACK_CHANGED_DATA_IMAGES;
    delete pCurImage;
    RefreshImage();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnSaveImageClicked( wxCommandEvent &event )
{
    wxASSERT( m_CurItem >= 0 );

    wxImage * pCurImage = m_Images->Item( m_CurItem );
    guTrack * CurTrack = &m_Items->Item( m_CurItem );
    wxASSERT( pCurImage );

    wxString CoverName;
    if( CurTrack->m_MediaViewer )
    {
        CoverName = CurTrack->m_MediaViewer->CoverName();
    }

    if( CoverName.IsEmpty() )
    {
        CoverName = wxT( "cover" );
    }

    CoverName += wxT( ".jpg" );

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
                if( !( ( * m_ChangedFlags )[ m_CurItem ] & guTRACK_CHANGED_DATA_IMAGES ) )
                    ( * m_ChangedFlags )[ m_CurItem ] |= guTRACK_CHANGED_DATA_IMAGES;
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

            if( !( ( * m_ChangedFlags )[ index ] & guTRACK_CHANGED_DATA_IMAGES ) )
                ( * m_ChangedFlags )[ index ] |= guTRACK_CHANGED_DATA_IMAGES;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAddButtonClicked( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnMBrainzAddButtonClicked..." ) );

    if( m_MBQuerySetArtistEnabled )
        m_MBQuerySetArtistEnabled = false;

    if( !m_MBQueryArtistTextCtrl->IsEmpty() ||
        !m_MBQueryTitleTextCtrl->IsEmpty() )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );

        guMusicBrainz * MusicBrainz = new guMusicBrainz();

        m_MBAddButton->Enable( false );

        if( !m_MBAlbums )
        {
            m_MBAlbums = new guMBReleaseArray();
        }

        MusicBrainz->GetRecordReleases( m_MBQueryArtistTextCtrl->GetValue(), m_MBQueryTitleTextCtrl->GetValue(), m_MBAlbums );

        ReloadMBAlbums();

        delete MusicBrainz;

        m_MBAddButton->Enable( true );
        wxSetCursor( * wxSTANDARD_CURSOR );
    }
    else if( m_MBCurTrack < ( int ) m_Items->Count() )
    {
        guTrack & CurTrack = m_Items->Item( m_MBCurTrack );

        if( wxFileExists( CurTrack.m_FileName ) )
        {
            guLogMessage( wxT( "Searching for the acousticId for the current track " ) );

            wxSetCursor( * wxHOURGLASS_CURSOR );
            m_MBAddButton->Enable( false );

            if( !m_AcousticId )
            {
                m_AcousticId = new guAcousticId( this );
            }

            m_MBRecordingId.Clear();

            m_AcousticId->SearchTrack( &CurTrack );
            // It will return on OnAcousticIdMBIdFound or OnAcousticIdError()
        }
        else
        {
            guLogError( wxT( "Could not find the track '%s'" ), CurTrack.m_FileName.c_str() );
        }
        m_MBCurTrack++;
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::ReloadMBAlbums()
{
    int Count = m_MBAlbums->Count();
    //guLogMessage( wxT( "Got %i releases" ), Count );
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            guMBRelease * MBRelease = &m_MBAlbums->Item( Index );
            m_MBAlbumChoice->Append( MBRelease->m_ArtistName + " " + MBRelease->m_Title );
        }

        if( m_MBAlbumChoice->GetSelection() == wxNOT_FOUND )
        {
            m_MBAlbumChoice->SetSelection( 0 );
            wxCommandEvent event;
            event.SetInt( 0 );
            OnMBrainzAlbumChoiceSelected( event );
        }
    }
    else
    {
        wxMessageBox( wxT( "MusicBrainz: Could not get any track information." ), wxT( "MusicBrainz Error" ) );
    }
}

void guTrackEditor::OnAcousticIdMBIdFound( const wxString &mbid )
{
    //guLogMessage( wxT( "TrackEditor::OnAcousticIdMBIdFound: %s" ), mbid.c_str() );
    guMusicBrainz * MusicBrainz = new guMusicBrainz();
    if( MusicBrainz )
    {
        if( !m_MBAlbums )
        {
            m_MBAlbums = new guMBReleaseArray();
        }

        if( m_MBAlbums )
        {
            m_MBAlbumChoice->Clear();

            MusicBrainz->GetRecordReleases( mbid, m_MBAlbums );

            ReloadMBAlbums();
        }
        else
        {
            guLogMessage( wxT( "Could not create the releases musibrainz object." ) );
        }

        delete MusicBrainz;
    }
    else
    {
        guLogMessage( wxT( "Could not create the MusicBrainz object." ) );
    }

    m_MBAddButton->SetBitmapLabel( guBitmap( guIMAGE_INDEX_tiny_search_again ) );
    m_MBAddButton->Enable( true );

    wxSetCursor( *wxSTANDARD_CURSOR );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAcousticIdError( const int status )
{
    guLogMessage( wxT( "AcousticIdError: %i" ), status );

    m_MBAddButton->Enable( true );
    wxSetCursor( *wxSTANDARD_CURSOR );
}


// -------------------------------------------------------------------------------- //
void guTrackEditor::UpdateMBrainzTrackInfo( void )
{
    if( m_MBCurAlbum >= 0 && m_CurItem >= 0 )
    {
        guMBRelease &CurRelease = m_MBAlbums->Item( m_MBCurAlbum );

        guMBRecordingArray * MBRecordings = &CurRelease.m_Recordings;
        if( MBRecordings && ( m_CurItem < ( int ) MBRecordings->Count() ) )
        {
            guMBRecording * MBRecording = &MBRecordings->Item( m_CurItem );
            guTrack * Track = &m_Items->Item( m_CurItem );

            // Artist
            if( !MBRecording->m_ArtistName.IsEmpty() )
            {
                m_MBArtistStaticText->SetForegroundColour( Track->m_ArtistName == MBRecording->m_ArtistName ?
                                                           m_NormalColor : m_ErrorColor );
                m_MBArtistTextCtrl->SetValue( MBRecording->m_ArtistName );
            }
            else
            {
                m_MBArtistStaticText->SetForegroundColour( Track->m_ArtistName == CurRelease.m_ArtistName ?
                                                           m_NormalColor : m_ErrorColor );
                m_MBArtistTextCtrl->SetValue( CurRelease.m_ArtistName );
            }

            // Album Artist
            m_MBAlbumArtistStaticText->SetForegroundColour( Track->m_AlbumArtist == CurRelease.m_ArtistName ?
                                                            m_NormalColor : m_ErrorColor );

            // ALbum
            m_MBAlbumStaticText->SetForegroundColour( Track->m_AlbumName == CurRelease.m_Title ?
                                                      m_NormalColor : m_ErrorColor );

            // Title
            m_MBTitleStaticText->SetForegroundColour( Track->m_SongName == MBRecording->m_Title ?
                                                      m_NormalColor : m_ErrorColor );
            m_MBTitleTextCtrl->SetValue( MBRecording->m_Title );

            // Year
            m_MBYearTextCtrl->SetValue( wxString::Format( wxT( "%u" ), CurRelease.m_Year ) );
            if( Track->m_Year )
            {
                m_MBYearStaticText->SetForegroundColour( ( CurRelease.m_Year == Track->m_Year ) ?
                                                         m_ErrorColor : m_NormalColor );
            }
            else
            {
                m_MBYearStaticText->SetForegroundColour( m_MBYearTextCtrl->IsEmpty() ?
                                                         m_NormalColor : m_ErrorColor );
            }

            // Length
            m_MBLengthStaticText->SetForegroundColour(
                    GetTrackLengthDiff( Track->m_Length, MBRecording->m_Length ) > guMBRAINZ_MAX_TIME_DIFF ?
                                        m_ErrorColor : m_NormalColor );
            m_MBLengthTextCtrl->SetValue( LenToString( MBRecording->m_Length ) );

            // Number
            m_MBNumberStaticText->SetForegroundColour( Track->m_Number == MBRecording->m_Number ?
                                        m_NormalColor : m_ErrorColor );
            m_MBNumberTextCtrl->SetValue( wxString::Format( wxT( "%u" ), MBRecording->m_Number ) );

            return;
        }
    }
    m_MBTitleTextCtrl->SetValue( wxEmptyString );
    m_MBNumberTextCtrl->SetValue( wxEmptyString );
}

// -------------------------------------------------------------------------------- //
int guTrackEditor::CheckTracksLengths( guMBRecordingArray * mbtracks, guTrackArray * tracks )
{
    int RetVal = 0;
    int Index;
    int Count = wxMin( tracks->Count(), mbtracks->Count() );
    for( Index = 0; Index < Count; Index++ )
    {
        if( GetTrackLengthDiff( tracks->Item( Index ).m_Length,
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
    if( m_MBAlbums && m_MBAlbums->Count() )
    {
        guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
        // Check the number of tracks
        wxString InfoText;
        if( MBRelease.m_Recordings.Count() != m_Items->Count() )
        {
            InfoText = wxString::Format( _( "Error: The album have %lu tracks and you are editing %lu" ),
                            MBRelease.m_Recordings.Count(), m_Items->Count() );
        }
        if( CheckTracksLengths( &MBRelease.m_Recordings, m_Items ) )
        {
            InfoText += "\n";
            InfoText += _( "Warning: The length of some edited tracks don't match" );
        }
        if( InfoText.IsEmpty() )
        {
            InfoText += _( "Everything ok" );
        }
        m_MBInfoStaticText->SetLabel( InfoText );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAlbumChoiceSelected( wxCommandEvent &event )
{
    m_MBCurAlbum = event.GetInt();
    guLogMessage( wxT( "MusicBrainzAlbumSelected... %i" ), m_MBCurAlbum );
    if( m_MBCurAlbum >= 0 )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );

        if( m_MBAlbums && m_MBAlbums->Count() )
        {
            guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );

            if( !MBRelease.m_Recordings.Count() )
            {
                guMusicBrainz * MusicBrainz = new guMusicBrainz();
                if( MusicBrainz )
                {
                    MusicBrainz->GetRecordings( MBRelease );

                    delete MusicBrainz;
                }
                else
                {
                    guLogMessage( wxT( "Could not create the MusicBrainz object." ) );
                }
            }

            m_MBArtistTextCtrl->SetValue( MBRelease.m_ArtistName );
            m_MBAlbumArtistTextCtrl->SetValue( MBRelease.m_ArtistName );

            UpdateMBrainzTrackInfo();
            m_MBCopyButton->Enable( true );
            m_MBAlbumTextCtrl->SetValue( MBRelease.m_Title );

            MBCheckCountAndLengths();
        }

        wxSetCursor( * wxSTANDARD_CURSOR );
    }
    else
    {
        m_MBCopyButton->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzCopyButtonClicked( wxCommandEvent &event )
{
    if( !( m_MBAlbums && m_MBAlbums->Count() && m_MBCurAlbum >= 0 ) )
        return;
    guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
    int Count = wxMin( m_Items->Count(), MBRelease.m_Recordings.Count() );
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBRecording &MBRecording = MBRelease.m_Recordings[ Index ];

        SetTagField( Track->m_ArtistName, MBRecording.m_ArtistName.IsEmpty() ? MBRelease.m_ArtistName :
                     MBRecording.m_ArtistName, ( * m_ChangedFlags )[ Index ] );
        SetTagField( Track->m_AlbumArtist, MBRelease.m_ArtistName, ( * m_ChangedFlags )[ Index ] );
        SetTagField( Track->m_AlbumName, MBRelease.m_Title, ( * m_ChangedFlags )[ Index ] );
        SetTagField( Track->m_SongName, MBRecording.m_Title, ( * m_ChangedFlags )[ Index ] );
        SetTagField( Track->m_Number, MBRecording.m_Number, ( * m_ChangedFlags )[ Index ] );
        SetTagField( Track->m_Year, MBRelease.m_Year, ( * m_ChangedFlags )[ Index ] );
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzArtistCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBAlbums && m_MBAlbums->Count() && m_MBCurAlbum >= 0 ) )
        return;
    guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
    int Count = wxMin( m_Items->Count(), MBRelease.m_Recordings.Count() );
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBRecording &MBRecording = MBRelease.m_Recordings[ Index ];
        SetTagField( Track->m_ArtistName, MBRecording.m_ArtistName.IsEmpty() ? MBRelease.m_ArtistName : MBRecording.m_ArtistName,
                    ( * m_ChangedFlags )[ Index ] );
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAlbumArtistCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBAlbums && m_MBAlbums->Count() && m_MBCurAlbum >= 0 ) )
        return;
    guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
    int Count = wxMin( m_Items->Count(), MBRelease.m_Recordings.Count() );
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        SetTagField( Track->m_AlbumArtist, MBRelease.m_ArtistName, ( * m_ChangedFlags )[ Index ] );
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzAlbumCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBAlbums && m_MBAlbums->Count() && m_MBCurAlbum >= 0 ) )
        return;
    guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
    int Count = wxMin( m_Items->Count(), MBRelease.m_Recordings.Count() );
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        SetTagField( Track->m_AlbumName, MBRelease.m_Title, ( * m_ChangedFlags )[ Index ] );
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzYearCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBAlbums && m_MBAlbums->Count() && m_MBCurAlbum >= 0 ) )
        return;
    guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
    int Count = wxMin( m_Items->Count(), MBRelease.m_Recordings.Count() );
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        SetTagField( Track->m_Year, MBRelease.m_Year, ( * m_ChangedFlags )[ Index ] );
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzTitleCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBAlbums && m_MBAlbums->Count() && m_MBCurAlbum >= 0 ) )
        return;
    guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
    int Count = wxMin( m_Items->Count(), MBRelease.m_Recordings.Count() );
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBRecording &MBRecording = MBRelease.m_Recordings[ Index ];
        SetTagField( Track->m_SongName, MBRecording.m_Title, ( * m_ChangedFlags )[ Index ] );
    }
    // Refresh the Details Window
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnMBrainzNumberCopyButtonClicked( wxCommandEvent& event )
{
    if( !( m_MBAlbums && m_MBAlbums->Count() && m_MBCurAlbum >= 0 ) )
        return;
    guMBRelease &MBRelease = m_MBAlbums->Item( m_MBCurAlbum );
    int Count = wxMin( m_Items->Count(), MBRelease.m_Recordings.Count() );
    for( int Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &m_Items->Item( Index );
        guMBRecording &MBRecording = MBRelease.m_Recordings[ Index ];
        SetTagField( Track->m_Number, MBRecording.m_Number, ( * m_ChangedFlags )[ Index ] );
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
    //guLogMessage( wxT( "SplitterOnIdle..." ) );
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_SongListSplitter->SetSashPosition( Config->ReadNum( CONFIG_KEY_POSITIONS_TRACKEDIT_SASHPOS, 200, CONFIG_PATH_POSITIONS ) );
    // Idle Events
    m_SongListSplitter->Unbind( wxEVT_IDLE, &guTrackEditor::SongListSplitterOnIdle, this );

    m_GetComboDataThread = new guTrackEditorGetComboDataThread( this, m_Db );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnTextUpdated( wxCommandEvent& event )
{
    bool Enabled = !m_LyricArtistTextCtrl->GetValue().IsEmpty() &&
        !m_LyricTrackTextCtrl->GetValue().IsEmpty();

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
        m_LyricSearchEngine = ( ( guMainFrame * ) guMainFrame::GetMainFrame() )->LyricSearchEngine();
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
        //( * m_Lyrics )[ m_CurItem ] = * Content;
        SetTagField( ( * m_Lyrics )[ m_CurItem ], * Content, ( * m_ChangedFlags )[ m_CurItem ], guTRACK_CHANGED_DATA_LYRICS );
        delete Content;
    }
    m_LyricReloadButton->Enable( true );
}

// -------------------------------------------------------------------------------- //
void inline guUpdateComboBoxEntries( wxComboBox * combobox, wxSortedArrayString &itemlist, int curitem, wxString &lastvalue )
{
    //guLogMessage( wxT( "guUpdateComboBoxEntries: %li %i '%s'" ), itemlist.Count(), curitem, lastvalue.c_str() );
    wxString FilterText = combobox->GetValue().Lower();

    if( FilterText == lastvalue )
        return;

    // Seems Clear is used for clear text what makes a call to this method and so on...
    // So call to the wxItemContainer method
    combobox->wxItemContainer::Clear();

    if( curitem == wxNOT_FOUND )
        return;

    wxArrayString SetItems;
    if( FilterText.IsEmpty() )
    {
#if wxUSE_STL
        int Count = itemlist.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            SetItems.Add( itemlist[ Index ] );
        }
        combobox->Append( SetItems );
#else
        combobox->Append( itemlist );
#endif
    }
    else
    {
        int Count = itemlist.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            if( itemlist[ Index ].Lower().Find( FilterText ) != wxNOT_FOUND )
            {
                SetItems.Add( itemlist[ Index ] );
            }
        }
        combobox->Append( SetItems );
    }
    lastvalue = FilterText;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnArtistChangedTimeout( wxTimerEvent &event )
{
    guUpdateComboBoxEntries( m_ArtistComboBox, m_Artists, m_CurItem, m_LastArtist );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlbumArtistChangedTimeout( wxTimerEvent &event )
{
    guUpdateComboBoxEntries( m_AlbumArtistComboBox, m_AlbumArtists, m_CurItem, m_LastAlbumArtist );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlbumChangedTimeout( wxTimerEvent &event )
{
    guUpdateComboBoxEntries( m_AlbumComboBox, m_Albums, m_CurItem, m_LastAlbum );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnComposerChangedTimeout( wxTimerEvent &event )
{
    guUpdateComboBoxEntries( m_CompComboBox, m_Composers, m_CurItem, m_LastComposer );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnGenreChangedTimeout( wxTimerEvent &event )
{
    guUpdateComboBoxEntries( m_GenreComboBox, m_Genres, m_CurItem, m_LastGenre );
}


// -------------------------------------------------------------------------------- //
void guTrackEditor::OnArtistTextChanged( wxCommandEvent &event )
{
    m_ArtistChanged = true;
    m_ArtistChangedTimer.Start( guTRACKEDIT_TIMER_TEXTCHANGED_TIME, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlbumArtistTextChanged( wxCommandEvent &event )
{
    m_AlbumArtistChanged = true;
    m_AlbumArtistChangedTimer.Start( guTRACKEDIT_TIMER_TEXTCHANGED_TIME, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlbumTextChanged( wxCommandEvent &event )
{
    m_AlbumChanged = true;
    m_AlbumChangedTimer.Start( guTRACKEDIT_TIMER_TEXTCHANGED_TIME, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnComposerTextChanged( wxCommandEvent &event )
{
    m_CompChanged = true;
    m_ComposerChangedTimer.Start( guTRACKEDIT_TIMER_TEXTCHANGED_TIME, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnGenreTextChanged( wxCommandEvent &event )
{
    m_GenreChanged = true;
    m_GenreChangedTimer.Start( guTRACKEDIT_TIMER_TEXTCHANGED_TIME, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
guTrack * guTrackEditor::GetTrack( const int index )
{
    if( m_Items && ( index >= 0 ) && ( index < ( int ) m_Items->Count() ) )
    {
        return &m_Items->Item( index );
    }
    return NULL;
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
    if( !TestDestroy() && m_TrackEditor )
    {
        m_TrackEditor->m_GetComboDataThread = NULL;
    }
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

}

// -------------------------------------------------------------------------------- //
