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
#include "TrackEdit.h"

#include "Config.h"
#include "Images.h"
#include "LastFM.h"
#include "TagInfo.h"
#include "Utils.h"

#include "wx/datetime.h"
#include <wx/notebook.h>

// -------------------------------------------------------------------------------- //
guTrackEditor::guTrackEditor( wxWindow* parent, DbLibrary * NewDb, guTrackArray * NewSongs, guImagePtrArray * images ) :
               wxDialog( parent, wxID_ANY, _( "Songs Editor" ), wxDefaultPosition, wxSize( 625, 400 ), wxDEFAULT_DIALOG_STYLE )
{
    wxPanel *           SongListPanel;
    wxPanel *           MainDetailPanel;
    wxNotebook *        MainNoteBook;
    wxPanel *           DetailPanel;
    wxStaticText *      ArStaticText;
    wxStaticText *      AlStaticText;
    wxStaticText *      TiStaticText;
    wxStaticText *      NuStaticText;
    wxStaticText *      GeStaticText;
    wxStaticText *      YeStaticText;
    wxPanel *           PicturePanel;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_SongListSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_SongListSplitter->SetMinimumPaneSize( 150 );

	SongListPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* SongListSizer;
	SongListSizer = new wxStaticBoxSizer( new wxStaticBox( SongListPanel, wxID_ANY, _( " Songs " ) ), wxVERTICAL );

	m_SongListBox = new wxListBox( SongListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	SongListSizer->Add( m_SongListBox, 1, wxALL|wxEXPAND, 2 );

	SongListPanel->SetSizer( SongListSizer );
	SongListPanel->Layout();
	SongListSizer->Fit( SongListPanel );
	MainDetailPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* DetailSizer;
	DetailSizer = new wxBoxSizer( wxVERTICAL );

	MainNoteBook = new wxNotebook( MainDetailPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	DetailPanel = new wxPanel( MainNoteBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* DataFlexSizer;
	DataFlexSizer = new wxFlexGridSizer( 6, 3, 0, 0 );
	DataFlexSizer->AddGrowableCol( 2 );
	DataFlexSizer->SetFlexibleDirection( wxHORIZONTAL );
	DataFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_ArCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_ArCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	ArStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	ArStaticText->Wrap( -1 );
	DataFlexSizer->Add( ArStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_ArtistTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_ArtistTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_AlCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_AlCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	AlStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Album:" ), wxDefaultPosition, wxDefaultSize, 0 );
	AlStaticText->Wrap( -1 );
	DataFlexSizer->Add( AlStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_AlbumTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_AlbumTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_TiCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_TiCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	TiStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Title:" ), wxDefaultPosition, wxDefaultSize, 0 );
	TiStaticText->Wrap( -1 );
	DataFlexSizer->Add( TiStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_TitleTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_TitleTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_NuCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, wxBitmap( guImage_numerate ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_NuCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	NuStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Number:" ), wxDefaultPosition, wxDefaultSize, 0 );
	NuStaticText->Wrap( -1 );
	DataFlexSizer->Add( NuStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_NumberTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_NumberTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	m_GeCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_GeCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	GeStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Genre:" ), wxDefaultPosition, wxDefaultSize, 0 );
	GeStaticText->Wrap( -1 );
	DataFlexSizer->Add( GeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_GenreTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_GenreTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_YeCopyButton = new wxBitmapButton( DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_YeCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	YeStaticText = new wxStaticText( DetailPanel, wxID_ANY, _( "Year:" ), wxDefaultPosition, wxDefaultSize, 0 );
	YeStaticText->Wrap( -1 );
	DataFlexSizer->Add( YeStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_YearTextCtrl = new wxTextCtrl( DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_YearTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	DetailPanel->SetSizer( DataFlexSizer );
	DetailPanel->Layout();
	DataFlexSizer->Fit( DetailPanel );
	MainNoteBook->AddPage( DetailPanel, _( "Details" ), true );
	PicturePanel = new wxPanel( MainNoteBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* PictureSizer;
	PictureSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* PictureBitmapSizer;
	PictureBitmapSizer = new wxStaticBoxSizer( new wxStaticBox( PicturePanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	m_PictureBitmap = new wxStaticBitmap( PicturePanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 250,250 ), wxSUNKEN_BORDER );
	PictureBitmapSizer->Add( m_PictureBitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	PictureSizer->Add( PictureBitmapSizer, 0, wxALIGN_CENTER_HORIZONTAL, 5 );


	wxBoxSizer* PictureButtonSizer;
	PictureButtonSizer = new wxBoxSizer( wxHORIZONTAL );

	m_AddPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, wxBitmap( guImage_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PictureButtonSizer->Add( m_AddPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_DelPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, wxBitmap( guImage_remove ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PictureButtonSizer->Add( m_DelPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_SavePicButton = new wxBitmapButton( PicturePanel, wxID_ANY, wxBitmap( guImage_document_save ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PictureButtonSizer->Add( m_SavePicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

//	m_EditPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, wxBitmap( guImage_gtk_edit ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	PictureButtonSizer->Add( m_EditPicButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	PictureButtonSizer->Add( 10, 0, 0, wxEXPAND, 5 );

	m_CopyPicButton = new wxBitmapButton( PicturePanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	PictureButtonSizer->Add( m_CopyPicButton, 0, wxALL, 5 );

	PictureSizer->Add( PictureButtonSizer, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	PicturePanel->SetSizer( PictureSizer );
	PicturePanel->Layout();
	PictureSizer->Fit( PicturePanel );
	MainNoteBook->AddPage( PicturePanel, _( "Pictures" ), false );

	DetailSizer->Add( MainNoteBook, 1, wxEXPAND | wxALL, 5 );

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
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();


	// --------------------------------------------------------------------
	m_CurItem = 0;
	m_Items = NewSongs;
	m_Images = images;
	m_Db = NewDb;
	wxArrayString ItemsText;
	int index;
	int count = m_Items->Count();
	for( index = 0; index < count; index++ )
	{
        ItemsText.Add( ( * m_Items )[ index ].m_FileName );
        // Fill the initial Images of the files
        m_Images->Add( ID3TagGetPicture( ( * m_Items )[ index ].m_FileName ) );
	}
	m_SongListBox->InsertItems( ItemsText, 0 );
	m_SongListBox->SetFocus();


	// Connect Events
	Connect( wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnButton ) );

	m_SongListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guTrackEditor::OnSongListBoxSelected ), NULL, this );
	m_ArCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnArCopyButtonClicked ), NULL, this );
	m_AlCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAlCopyButtonClicked ), NULL, this );
	m_TiCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnTiCopyButtonClicked ), NULL, this );
	m_NuCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnNuCopyButtonClicked ), NULL, this );
	m_GeCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGeCopyButtonClicked ), NULL, this );
	m_YeCopyButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnYeCopyButtonClicked ), NULL, this );
//	m_GetYearButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGetYearButtonClicked ), NULL, this);


	m_AddPicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAddImageClicked ), NULL, this );
	m_DelPicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnDelImageClicked ), NULL, this );
	m_SavePicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSaveImageClicked ), NULL, this );
	m_CopyPicButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCopyImageClicked ), NULL, this );

    // Idle Events
	m_SongListSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guTrackEditor::SongListSplitterOnIdle ), NULL, this );

	//
    // Force the 1st listbox item to be selected
	m_SongListBox->SetSelection( 0 );
    wxCommandEvent event( wxEVT_COMMAND_LISTBOX_SELECTED, m_SongListBox->GetId() );
    event.SetEventObject( m_SongListBox );
    event.SetExtraLong( 1 );
    event.SetInt( 0 );
    wxPostEvent( m_SongListBox, event );
}

// -------------------------------------------------------------------------------- //
guTrackEditor::~guTrackEditor()
{
	// Disconnect Events
	Disconnect( wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnButton ) );

	m_SongListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guTrackEditor::OnSongListBoxSelected ), NULL, this );
	m_ArCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnArCopyButtonClicked ), NULL, this );
	m_AlCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAlCopyButtonClicked ), NULL, this );
	m_TiCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnTiCopyButtonClicked ), NULL, this );
	m_NuCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnNuCopyButtonClicked ), NULL, this );
	m_GeCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGeCopyButtonClicked ), NULL, this );
	m_YeCopyButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnYeCopyButtonClicked ), NULL, this );
//	m_GetYearButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGetYearButtonClicked ), NULL, this);

	m_AddPicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnAddImageClicked ), NULL, this );
	m_DelPicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnDelImageClicked ), NULL, this );
	m_SavePicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnSaveImageClicked ), NULL, this );
	m_CopyPicButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnCopyImageClicked ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnButton( wxCommandEvent& event )
{
    WriteItemData();
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnSongListBoxSelected( wxCommandEvent& event )
{
    WriteItemData();
    m_CurItem = event.GetInt();
    ReadItemData();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::ReadItemData( void )
{
    if( m_CurItem >= 0 )
    {
        guTrack * Track = &( * m_Items )[ m_CurItem ];
        m_ArtistTextCtrl->SetValue( Track->m_ArtistName );
        m_AlbumTextCtrl->SetValue( Track->m_AlbumName );
        m_TitleTextCtrl->SetValue( Track->m_SongName );
        m_NumberTextCtrl->SetValue( wxString::Format( wxT( "%u" ), Track->m_Number ) );
        m_GenreTextCtrl->SetValue( Track->m_GenreName );
        m_YearTextCtrl->SetValue( wxString::Format( wxT( "%u" ), Track->m_Year ) );
    }
    else
    {
        m_ArtistTextCtrl->SetValue( wxEmptyString );
        m_AlbumTextCtrl->SetValue( wxEmptyString );
        m_TitleTextCtrl->SetValue( wxEmptyString );
        m_NumberTextCtrl->SetValue( wxEmptyString );
        m_GenreTextCtrl->SetValue( wxEmptyString );
        m_YearTextCtrl->SetValue( wxEmptyString );
    }
    RefreshImage();
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::WriteItemData( void )
{
    if( m_CurItem >= 0 )
    {
        if( m_ArtistTextCtrl->IsModified() )
          ( * m_Items )[ m_CurItem ].m_ArtistName = m_ArtistTextCtrl->GetLineText( 0 );
        if( m_AlbumTextCtrl->IsModified() )
          ( * m_Items )[ m_CurItem ].m_AlbumName = m_AlbumTextCtrl->GetLineText( 0 );
        if( m_TitleTextCtrl->IsModified() )
          ( * m_Items )[ m_CurItem ].m_SongName = m_TitleTextCtrl->GetLineText( 0 );
        if( m_NumberTextCtrl->IsModified() )
          m_NumberTextCtrl->GetLineText( 0 ).ToLong( ( long int * ) &( * m_Items )[ m_CurItem ].m_Number );
        if( m_GenreTextCtrl->IsModified() )
          ( * m_Items )[ m_CurItem ].m_GenreName = m_GenreTextCtrl->GetLineText( 0 );
        if( m_YearTextCtrl->IsModified() )
           m_YearTextCtrl->GetLineText( 0 ).ToLong( ( long * ) &( * m_Items )[ m_CurItem ].m_Year );
    }
}


// -------------------------------------------------------------------------------- //
void guTrackEditor::OnArCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_ArtistTextCtrl->GetLineText( 0 );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_ArtistName = CurData;
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnAlCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_AlbumTextCtrl->GetLineText( 0 );
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
void guTrackEditor::OnNuCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    //int CurData;
    //NumberTextCtrl->GetLineText( 0 ).ToLong( ( long int *) &CurData );
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
        ( * m_Items )[ index ].m_Number = ( index + 1 );
}

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnGeCopyButtonClicked( wxCommandEvent& event )
{
    int index;
    wxString CurData = m_GenreTextCtrl->GetLineText( 0 );
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

//// -------------------------------------------------------------------------------- //
//void guTrackEditor::OnGetYearButtonClicked( wxCommandEvent& event )
//{
//    guLastFM LastFM;
//    wxString Artist = m_ArtistTextCtrl->GetValue();
//    wxString Album = m_AlbumTextCtrl->GetValue();
//    if( !Artist.IsEmpty() && !Album.IsEmpty() )
//    {
//        guAlbumInfo AlbumInfo = LastFM.AlbumGetInfo( Artist, Album );
//        if( !AlbumInfo.m_ReleaseDate.IsEmpty() )
//        {
//            // "23 May 2005, 00:00"
//            //guLogMessage( wxT( "%s - %s Release Date: '%s'" ), Artist.c_str(), Album.c_str(), AlbumInfo.m_ReleaseDate.Trim().Trim( false ).c_str() );
//            int Pos = AlbumInfo.m_ReleaseDate.Find( ',' );
//            if( Pos != wxNOT_FOUND )
//            {
//                m_YearTextCtrl->SetValue( AlbumInfo.m_ReleaseDate.Mid( Pos - 4, 4 ) );
//            }
//        }
//    }
//}

// -------------------------------------------------------------------------------- //
void guTrackEditor::RefreshImage( void )
{
    wxImage Image;
    wxImage * pCurImage;
    if( ( m_CurItem >= 0 ) && ( pCurImage = ( * m_Images )[ m_CurItem ] ) )
    {
        Image = * pCurImage;
    }
    else
    {
        Image = wxImage( guImage_no_cover );
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
        }
        else
        {
            m_AddPicButton->Enable( true );
            m_DelPicButton->Enable( false );
            m_SavePicButton->Enable( false );
        }
        m_CopyPicButton->Enable( true );
    }
    else
    {
        m_AddPicButton->Enable( false );
        m_DelPicButton->Enable( false );
        m_SavePicButton->Enable( false );
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

    wxFileDialog * FileDialog = new wxFileDialog( this,
        wxT( "Select the filename to save" ),
        wxPathOnly( ( * m_Items )[ m_CurItem ].m_FileName ),
        wxT( "cover.jpg" ),
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
void guTrackEditor::SongListSplitterOnIdle( wxIdleEvent& )
{
    m_SongListSplitter->SetSashPosition( 200 );
    m_SongListSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guTrackEditor::SongListSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
