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
#include "Utils.h"

#include "wx/datetime.h"

// -------------------------------------------------------------------------------- //
guTrackEditor::guTrackEditor( wxWindow* parent, DbLibrary * NewDb, guTrackArray * NewSongs ) :
               wxDialog( parent, wxID_ANY, _( "Songs Editor" ), wxDefaultPosition, wxSize( 625, 370 ), wxDEFAULT_DIALOG_STYLE )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	MainSizer->Add( 0, 30, 0, wxEXPAND, 5 );

	m_SongListSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_SongListSplitter->SetMinimumPaneSize( 150 );
	m_SongListSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guTrackEditor::SongListSplitterOnIdle ), NULL, this );
	m_SongListPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//	wxBoxSizer* SongListSizer;
//	SongListSizer = new wxBoxSizer( wxVERTICAL );
	wxStaticBoxSizer * SongListSizer;
	SongListSizer = new wxStaticBoxSizer( new wxStaticBox( m_SongListPanel, wxID_ANY, _(" Songs ") ), wxVERTICAL );

	m_SongListBox = new wxListBox( m_SongListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	SongListSizer->Add( m_SongListBox, 1, wxALL|wxEXPAND, 2 );

	m_SongListPanel->SetSizer( SongListSizer );
	m_SongListPanel->Layout();
	SongListSizer->Fit( m_SongListPanel );
	m_DetailPanel = new wxPanel( m_SongListSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* DetailSizer;
	DetailSizer = new wxBoxSizer( wxVERTICAL );

//    wxStaticLine * TopStaticLine;
//	TopStaticLine = new wxStaticLine( m_DetailPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
//	DetailSizer->Add( TopStaticLine, 0, wxEXPAND | wxALL, 5 );

	wxStaticBoxSizer* DataSizer;
	DataSizer = new wxStaticBoxSizer( new wxStaticBox( m_DetailPanel, wxID_ANY, _( " Details " ) ), wxVERTICAL );

	wxFlexGridSizer* DataFlexSizer;
	DataFlexSizer = new wxFlexGridSizer( 6, 3, 0, 0 );
	DataFlexSizer->AddGrowableCol( 2 );
	DataFlexSizer->SetFlexibleDirection( wxHORIZONTAL );
	DataFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_ArCopyButton = new wxBitmapButton( m_DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_ArCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_ArStaticText = new wxStaticText( m_DetailPanel, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_ArStaticText->Wrap( -1 );
	DataFlexSizer->Add( m_ArStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_ArtistTextCtrl = new wxTextCtrl( m_DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_ArtistTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_AlCopyButton = new wxBitmapButton( m_DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_AlCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_AlStaticText = new wxStaticText( m_DetailPanel, wxID_ANY, _( "Album:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlStaticText->Wrap( -1 );
	DataFlexSizer->Add( m_AlStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_AlbumTextCtrl = new wxTextCtrl( m_DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_AlbumTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_TiCopyButton = new wxBitmapButton( m_DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_TiCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_TiStaticText = new wxStaticText( m_DetailPanel, wxID_ANY, _( "Title:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_TiStaticText->Wrap( -1 );
	DataFlexSizer->Add( m_TiStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_TitleTextCtrl = new wxTextCtrl( m_DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_TitleTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_NuCopyButton = new wxBitmapButton( m_DetailPanel, wxID_ANY, wxBitmap( guImage_numerate ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_NuCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_NuStaticText = new wxStaticText( m_DetailPanel, wxID_ANY, _( "Number:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_NuStaticText->Wrap( -1 );
	DataFlexSizer->Add( m_NuStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_NumberTextCtrl = new wxTextCtrl( m_DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_NumberTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_GeCopyButton = new wxBitmapButton( m_DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_GeCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_GeStaticText = new wxStaticText( m_DetailPanel, wxID_ANY, _( "Genre:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_GeStaticText->Wrap( -1 );
	DataFlexSizer->Add( m_GeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_GenreTextCtrl = new wxTextCtrl( m_DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DataFlexSizer->Add( m_GenreTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10 );

	m_YeCopyButton = new wxBitmapButton( m_DetailPanel, wxID_ANY, wxBitmap( guImage_edit_copy ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	DataFlexSizer->Add( m_YeCopyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_YeStaticText = new wxStaticText( m_DetailPanel, wxID_ANY, _( "Year:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_YeStaticText->Wrap( -1 );
	DataFlexSizer->Add( m_YeStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

//	m_YearTextCtrl = new wxTextCtrl( m_DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
//	DataFlexSizer->Add( m_YearTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxBoxSizer* YearSizer;
	YearSizer = new wxBoxSizer( wxHORIZONTAL );

	m_YearTextCtrl = new wxTextCtrl( m_DetailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	YearSizer->Add( m_YearTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	m_GetYearButton = new wxBitmapButton( m_DetailPanel, wxID_ANY, wxBitmap( guImage_search ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_GetYearButton->SetToolTip( _( "Search in Last.fm the year of the current album" ) );
	YearSizer->Add( m_GetYearButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	DataFlexSizer->Add( YearSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	DataSizer->Add( DataFlexSizer, 1, wxEXPAND, 5 );

	DetailSizer->Add( DataSizer, 1, wxEXPAND, 5 );

//    wxStaticLine * BottomStaticLine;
//	BottomStaticLine = new wxStaticLine( m_DetailPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
//	DetailSizer->Add( BottomStaticLine, 0, wxEXPAND | wxALL, 5 );

	m_DetailPanel->SetSizer( DetailSizer );
	m_DetailPanel->Layout();
	DetailSizer->Fit( m_DetailPanel );
	m_SongListSplitter->SplitVertically( m_SongListPanel, m_DetailPanel, 179 );
	MainSizer->Add( m_SongListSplitter, 1, wxEXPAND, 5 );

    wxStdDialogButtonSizer* ButtonsSizer;
    wxButton* ButtonsSizerOK;
    wxButton* ButtonsSizerCancel;
	ButtonsSizer = new wxStdDialogButtonSizer();
	ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	//
	m_CurItem = 0;
	m_Items = NewSongs;
	m_Db = NewDb;
	wxArrayString ItemsText;
	int index;
	int count = m_Items->Count();
	for( index = 0; index < count; index++ )
	{
        ItemsText.Add( ( * m_Items )[ index ].m_FileName );
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
	m_GetYearButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGetYearButtonClicked ), NULL, this);

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
	m_GetYearButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guTrackEditor::OnGetYearButtonClicked ), NULL, this);
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
        m_ArtistTextCtrl->SetValue( ( * m_Items )[ m_CurItem ].m_ArtistName );
        m_AlbumTextCtrl->SetValue( ( * m_Items )[ m_CurItem ].m_AlbumName );
        m_TitleTextCtrl->SetValue( ( * m_Items )[ m_CurItem ].m_SongName );
        m_NumberTextCtrl->SetValue( wxString::Format( wxT( "%u" ), ( * m_Items )[ m_CurItem ].m_Number ) );
        m_GenreTextCtrl->SetValue( ( * m_Items )[ m_CurItem ].m_GenreName );
        m_YearTextCtrl->SetValue( wxString::Format( wxT( "%u" ), ( * m_Items )[ m_CurItem ].m_Year ) );
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

// -------------------------------------------------------------------------------- //
void guTrackEditor::OnGetYearButtonClicked( wxCommandEvent& event )
{
    guLastFM LastFM;
    wxString Artist = m_ArtistTextCtrl->GetValue();
    wxString Album = m_AlbumTextCtrl->GetValue();
    if( !Artist.IsEmpty() && !Album.IsEmpty() )
    {
        guAlbumInfo AlbumInfo = LastFM.AlbumGetInfo( Artist, Album );
        if( !AlbumInfo.m_ReleaseDate.IsEmpty() )
        {
            // "23 May 2005, 00:00"
            //guLogMessage( wxT( "%s - %s Release Date: '%s'" ), Artist.c_str(), Album.c_str(), AlbumInfo.m_ReleaseDate.Trim().Trim( false ).c_str() );
            int Pos = AlbumInfo.m_ReleaseDate.Find( ',' );
            if( Pos != wxNOT_FOUND )
            {
                m_YearTextCtrl->SetValue( AlbumInfo.m_ReleaseDate.Mid( Pos - 4, 4 ) );
            }
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
