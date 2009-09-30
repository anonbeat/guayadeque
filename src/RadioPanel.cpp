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
#include "RadioPanel.h"

#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "LabelEditor.h"
#include "MainFrame.h"
#include "RadioGenreEditor.h"
#include "Shoutcast.h"
#include "StatusBar.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
// guRadioGenreListBox
// -------------------------------------------------------------------------------- //
class guRadioGenreListBox : public guListBox
{
    private :
      virtual void  GetItemsList( void );
      virtual void  CreateContextMenu( wxMenu * Menu ) const;
      void          OnRadioGenreAdd( wxCommandEvent &event );
      void          OnRadioGenreEdit( wxCommandEvent &event );
      void          OnRadioGenreDelete( wxCommandEvent &event );

    public :
      guRadioGenreListBox( wxWindow * parent, DbLibrary * NewDb, wxString Label );
      ~guRadioGenreListBox();

};

// -------------------------------------------------------------------------------- //
// guRadioLabelListBox
// -------------------------------------------------------------------------------- //
class guRadioLabelListBox : public guListBox
{
    protected :

      virtual void  GetItemsList( void );
      virtual void  CreateContextMenu( wxMenu * Menu ) const;
      void          AddLabel( wxCommandEvent &event );
      void          DelLabel( wxCommandEvent &event );
      void          EditLabel( wxCommandEvent &event );

    public :

      guRadioLabelListBox( wxWindow * parent, DbLibrary * NewDb, wxString Label );
      ~guRadioLabelListBox();

};

// -------------------------------------------------------------------------------- //
// guUpdateRadiosThread
// -------------------------------------------------------------------------------- //
class guUpdateRadiosThread : public wxThread
{
  private:
    DbLibrary *     m_Db;
    guRadioPanel *  m_RadioPanel;
    int             m_GaugeId;
    wxArrayInt      m_GenresIds;

  public:
    guUpdateRadiosThread( DbLibrary * db, guRadioPanel * radiopanel,
                                const wxArrayInt &genres, int gaugeid = wxNOT_FOUND )
    {
        m_Db = db;
        m_RadioPanel = radiopanel;
        m_GenresIds = genres;
        m_GaugeId = gaugeid;
    };

    ~guUpdateRadiosThread()
    {
    };

    virtual ExitCode Entry();
};



wxString guRADIOSTATIONS_COLUMN_NAMES[] = {
    _( "Name" ),
    _( "BitRate" ),
    _( "Listeners" )
};

#define guRADIOSTATIONS_COLUMN_NAME         0
#define guRADIOSTATIONS_COLUMN_BITRATE      1
#define guRADIOSTATIONS_COLUMN_LISTENERS    2

// -------------------------------------------------------------------------------- //
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
class guRadioStationListBox : public guListView
{
  protected :
    DbLibrary *       m_Db;
    guRadioStations   m_Radios;
    int               m_StationsOrder;
    bool              m_StationsOrderDesc;

    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

  public :
    guRadioStationListBox( wxWindow * parent, DbLibrary * NewDb );
    ~guRadioStationListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual int inline          GetItemId( const int row ) const;
    virtual wxString inline     GetItemName( const int row ) const;

    void                        SetStationsOrder( int order );
};




// -------------------------------------------------------------------------------- //
// guRadioGenreListBox
// -------------------------------------------------------------------------------- //
guRadioGenreListBox::guRadioGenreListBox( wxWindow * parent, DbLibrary * NewDb, wxString Label ) :
             guListBox( parent, NewDb, Label )
{
    Connect( ID_RADIO_GENRE_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreListBox::OnRadioGenreAdd ) );
    Connect( ID_RADIO_GENRE_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreListBox::OnRadioGenreEdit ) );
    Connect( ID_RADIO_GENRE_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreListBox::OnRadioGenreDelete ) );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guRadioGenreListBox::~guRadioGenreListBox()
{
    Disconnect( ID_RADIO_GENRE_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreListBox::OnRadioGenreAdd ) );
    Disconnect( ID_RADIO_GENRE_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreListBox::OnRadioGenreEdit ) );
    Disconnect( ID_RADIO_GENRE_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreListBox::OnRadioGenreDelete ) );
}

// -------------------------------------------------------------------------------- //
void guRadioGenreListBox::GetItemsList( void )
{
    m_Db->GetRadioGenres( m_Items );
}

// -------------------------------------------------------------------------------- //
void guRadioGenreListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_RADIO_GENRE_ADD, _( "Add Genre" ), _( "Create a new genre" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
    Menu->Append( MenuItem );

    if( GetSelectedItems().Count() )
    {
        MenuItem = new wxMenuItem( Menu, ID_RADIO_GENRE_EDIT, _( "Edit genre" ), _( "Change selected genre" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_RADIO_GENRE_DELETE, _( "Delete genre" ), _( "Delete selected genre" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_RADIO_DOUPDATE, _( "Update Radio Stations" ), _( "Update the radio station lists" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
    Menu->Append( MenuItem );

}

// -------------------------------------------------------------------------------- //
void guRadioGenreListBox::OnRadioGenreAdd( wxCommandEvent &event )
{
    int index;
    int count;
    guRadioGenreEditor * RadioGenreEditor = new guRadioGenreEditor( this, m_Db );
    if( RadioGenreEditor )
    {
        //
        if( RadioGenreEditor->ShowModal() == wxID_OK )
        {
            wxArrayString NewGenres = RadioGenreEditor->GetGenres();
            if( ( count = NewGenres.Count() ) )
            {
                //
                for( index = 0; index < count; index++ )
                {
                    m_Db->AddRadioGenre( NewGenres[ index ] );
                }
                ReloadItems();
            }
        }
        //
        RadioGenreEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreListBox::OnRadioGenreEdit( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        // Get the Index of the First Selected Item
        unsigned long cookie;
        int item = GetFirstSelected( cookie );
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Genre Name: " ), _( "Enter the new Genre Name" ), ( * m_Items )[ item ].m_Name );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            m_Db->SetRadioGenreName( Selection[ 0 ], EntryDialog->GetValue() );
            ReloadItems();
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreListBox::OnRadioGenreDelete( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems();
    int Count = Selection.Count();
    if( Count )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected radio genres?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            for( int Index = 0; Index < Count; Index++ )
            {
                m_Db->DelRadioGenre( Selection[ Index ] );
            }
            ReloadItems();
        }
    }
}


// -------------------------------------------------------------------------------- //
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
guRadioStationListBox::guRadioStationListBox( wxWindow * parent, DbLibrary * db ) :
    guListView( parent, wxLB_SINGLE )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_StationsOrder = Config->ReadNum( wxT( "StationsOrder" ), 0, wxT( "General" ) );
    m_StationsOrderDesc = Config->ReadNum( wxT( "StationsOrderDesc" ), false, wxT( "General" ) );;

    // Create the Columns
    int ColId;
    int index;
    int count = sizeof( guRADIOSTATIONS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        guListViewColumn * Column = new guListViewColumn(
            guRADIOSTATIONS_COLUMN_NAMES[ index ] + ( ( index == m_StationsOrder ) ? ( m_StationsOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ),
            index,
            Config->ReadNum( wxString::Format( wxT( "RadioColSize%u" ), index ), 80, wxT( "Positions" ) )
        );
        InsertColumn( Column );
    }

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guRadioStationListBox::~guRadioStationListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    int index;
    int count = sizeof( guRADIOSTATIONS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "RadioColSize%u" ), index ), GetColumnWidth( index ), wxT( "Positions" ) );
    }

    Config->WriteNum( wxT( "StationsOrder" ), m_StationsOrder, wxT( "General" ) );
    Config->WriteBool( wxT( "StationsOrderDesc" ), m_StationsOrderDesc, wxT( "General" ) );;
}

// -------------------------------------------------------------------------------- //
wxString guRadioStationListBox::OnGetItemText( const int row, const int col ) const
{
    guRadioStation * Radio;
    Radio = &m_Radios[ row ];
    switch( col )
    {
        case guRADIOSTATIONS_COLUMN_NAME :
          return Radio->m_Name;
          break;

        case guRADIOSTATIONS_COLUMN_BITRATE :
          return wxString::Format( wxT( "%u" ), Radio->m_BitRate );
          break;

        case guRADIOSTATIONS_COLUMN_LISTENERS :
          return wxString::Format( wxT( "%u" ), Radio->m_Listeners );

    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
void guRadioStationListBox::GetItemsList( void )
{
    m_Db->GetRadioStations( &m_Radios );
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::ReloadItems( bool reset )
{
    wxASSERT( m_Db );

    //
    wxArrayInt Selection;
    int FirstVisible = GetFirstVisibleLine();

    if( reset )
        SetSelection( -1 );
    else
        Selection = GetSelectedItems( false );

    m_Radios.Empty();

    GetItemsList();

    SetItemCount( m_Radios.Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToLine( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    MenuItem = new wxMenuItem( Menu, ID_RADIO_PLAY, _( "Play" ), _( "Play current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_RADIO_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_RADIO_EDIT_LABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected stations" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
    Menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
int inline guRadioStationListBox::GetItemId( const int row ) const
{
    return m_Radios[ row ].m_Id;
}

// -------------------------------------------------------------------------------- //
wxString inline guRadioStationListBox::GetItemName( const int row ) const
{
    return m_Radios[ row ].m_Name;
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::SetStationsOrder( int order )
{
    if( m_StationsOrder != order )
    {
        m_StationsOrder = order;
        m_StationsOrderDesc = ( order != 0 );
    }
    else
        m_StationsOrderDesc = !m_StationsOrderDesc;

    m_Db->SetRadioStationsOrder( m_StationsOrder );

    int index;
    int count = 3;
    for( index = 0; index < count; index++ )
    {
        SetColumnLabel( index,
            guRADIOSTATIONS_COLUMN_NAMES[ index ]  + ( ( index == m_StationsOrder ) ?
                ( m_StationsOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
// guRadioLabelListBox
// -------------------------------------------------------------------------------- //
guRadioLabelListBox::guRadioLabelListBox( wxWindow * parent, DbLibrary * NewDb, wxString Label ) : guListBox( parent, NewDb, Label )
{
    Connect( ID_LABEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioLabelListBox::AddLabel ) );
    Connect( ID_LABEL_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioLabelListBox::DelLabel ) );
    Connect( ID_LABEL_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioLabelListBox::EditLabel ) );
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guRadioLabelListBox::~guRadioLabelListBox()
{
    Disconnect( ID_LABEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioLabelListBox::AddLabel ) );
    Disconnect( ID_LABEL_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioLabelListBox::DelLabel ) );
    Disconnect( ID_LABEL_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioLabelListBox::EditLabel ) );
}

// -------------------------------------------------------------------------------- //
void guRadioLabelListBox::GetItemsList( void )
{
    m_Db->GetRadioLabels( m_Items );
}

// -------------------------------------------------------------------------------- //
void guRadioLabelListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ADD, _( "Add Label" ), _( "Create a new label" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
    Menu->Append( MenuItem );

    if( GetSelectedItems().Count() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LABEL_EDIT, _( "Edit Label" ), _( "Change selected label" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LABEL_DELETE, _( "Delete label" ), _( "Delete selected labels" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu->Append( MenuItem );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioLabelListBox::AddLabel( wxCommandEvent &event )
{
    //wxMessageBox( wxT( "AddLabel" ), wxT( "Information" ) );
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ), _( "Please enter the label name" ) );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        //wxMessageBox( EntryDialog->GetValue(), wxT( "Entered..." ) );
        m_Db->AddRadioLabel( EntryDialog->GetValue() );
        ReloadItems();
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guRadioLabelListBox::DelLabel( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems();
    int Count = Selection.Count();
    if( Count )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected labels?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            for( int Index = 0; Index < Count; Index++ )
            {
                m_Db->DelRadioLabel( Selection[ Index ] );
            }
            ReloadItems();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioLabelListBox::EditLabel( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        // Get the Index of the First Selected Item
        unsigned long cookie;
        int item = GetFirstSelected( cookie );
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ), _( "Enter the new label name" ), ( * m_Items )[ item ].m_Name );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            m_Db->SetRadioLabelName( Selection[ 0 ], EntryDialog->GetValue() );
            ReloadItems();
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
// guRadioPanel
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
guRadioPanel::guRadioPanel( wxWindow* parent, DbLibrary * NewDb, guPlayerPanel * NewPlayerPanel ) :
              wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = NewDb;
    m_PlayerPanel = NewPlayerPanel;

    guConfig *  Config = ( guConfig * ) guConfig::Get();

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* SearchSizer;
	SearchSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SearchStaticText = new wxStaticText( this, wxID_ANY, wxT("Search:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SearchStaticText->Wrap( -1 );
	SearchSizer->Add( m_SearchStaticText, 0, wxALIGN_CENTER|wxALL, 5 );

	m_InputTextPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	m_InputTextPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

	wxBoxSizer* m_InputTextSizer;
	m_InputTextSizer = new wxBoxSizer( wxHORIZONTAL );

	m_InputTextLeftBitmap = new wxStaticBitmap( m_InputTextPanel, wxID_ANY, guImage( guIMAGE_INDEX_search ), wxDefaultPosition, wxDefaultSize, 0 );
	m_InputTextSizer->Add( m_InputTextLeftBitmap, 0, wxALL, 0 );

	m_InputTextCtrl = new wxTextCtrl( m_InputTextPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxNO_BORDER );
	m_InputTextSizer->Add( m_InputTextCtrl, 1, wxALL, 2 );

	m_InputTextClearBitmap = new wxStaticBitmap( m_InputTextPanel, wxID_ANY, guImage( guIMAGE_INDEX_edit_clear ), wxDefaultPosition, wxDefaultSize, 0 );
	m_InputTextSizer->Add( m_InputTextClearBitmap, 0, wxALL, 0 );
	m_InputTextClearBitmap->Enable( false );

	m_InputTextPanel->SetSizer( m_InputTextSizer );
	m_InputTextPanel->Layout();
	m_InputTextSizer->Fit( m_InputTextPanel );
	SearchSizer->Add( m_InputTextPanel, 1, wxEXPAND | wxALL, 2 );

	MainSizer->Add( SearchSizer, 0, wxEXPAND, 5 );

	wxStaticLine * SearchStaticline;
	SearchStaticline = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MainSizer->Add( SearchStaticline, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* ListsSizer;
	ListsSizer = new wxBoxSizer( wxVERTICAL );

	m_StationsSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_StationsSplitter->SetMinimumPaneSize( 110 );
	m_StationsSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guRadioPanel::StationsSplitterOnIdle ), NULL, this );

	wxPanel * LeftPanel;
	LeftPanel = new wxPanel( m_StationsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* GenMainSizer;
	GenMainSizer = new wxBoxSizer( wxVERTICAL );

	m_GenreSplitter = new wxSplitterWindow( LeftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_GenreSplitter->SetMinimumPaneSize( 100 );
	m_GenreSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guRadioPanel::GenreSplitterOnIdle ), NULL, this );

    wxPanel * GenrePanel;
	GenrePanel = new wxPanel( m_GenreSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* GenreSizer;
	GenreSizer = new wxBoxSizer( wxVERTICAL );

    m_GenresListBox = new guRadioGenreListBox( GenrePanel, m_Db, _( "Genres" ) );
	//m_GenresListBox->SetBackgroundColour( wxColour( 250, 250, 250 ) );

	GenreSizer->Add( m_GenresListBox, 1, wxALL|wxEXPAND, 1 );

	GenrePanel->SetSizer( GenreSizer );
	GenrePanel->Layout();
	GenreSizer->Fit( GenrePanel );

    wxPanel * LabelsPanel;
	LabelsPanel = new wxPanel( m_GenreSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* LabelsSizer;
	LabelsSizer = new wxBoxSizer( wxVERTICAL );

	m_LabelsListBox = new guRadioLabelListBox( LabelsPanel, m_Db, _( "Labels" ) );
	//m_LabelsListBox->SetBackgroundColour( wxColour( 250, 250, 250 ) );

	LabelsSizer->Add( m_LabelsListBox, 1, wxALL|wxEXPAND, 1 );

	LabelsPanel->SetSizer( LabelsSizer );
	LabelsPanel->Layout();
	LabelsSizer->Fit( LabelsPanel );
	m_GenreSplitter->SplitHorizontally( GenrePanel, LabelsPanel, Config->ReadNum( wxT( "RadioGenreSashPos" ), 200, wxT( "Positions" ) ) );
	GenMainSizer->Add( m_GenreSplitter, 1, wxEXPAND, 5 );

	LeftPanel->SetSizer( GenMainSizer );
	LeftPanel->Layout();
	GenMainSizer->Fit( LeftPanel );

    wxPanel * StationsPanel;
	StationsPanel = new wxPanel( m_StationsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
//	StationsPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );

	wxBoxSizer* StationsSizer;
	StationsSizer = new wxBoxSizer( wxVERTICAL );

	m_StationsListBox = new guRadioStationListBox( StationsPanel, m_Db );

	StationsSizer->Add( m_StationsListBox, 1, wxALL|wxEXPAND, 1 );

	StationsPanel->SetSizer( StationsSizer );
	StationsPanel->Layout();
	StationsSizer->Fit( StationsPanel );
	m_StationsSplitter->SplitVertically( LeftPanel, StationsPanel, Config->ReadNum( wxT( "RadioStationsSashPos" ), 180, wxT( "Positions" ) ) );
	ListsSizer->Add( m_StationsSplitter, 1, wxEXPAND, 0 );

	MainSizer->Add( ListsSizer, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    m_GenresListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );

    m_LabelsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioLabelListSelected ), NULL, this );

    //
    Connect( ID_RADIO_DOUPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdate ) );
    Connect( ID_RADIO_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdated ) );
    Connect( ID_RADIO_UPDATE_END, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdateEnd ) );

    m_StationsListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guRadioPanel::OnStationListActivated ), NULL, this );
	m_StationsListBox->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guRadioPanel::OnStationListBoxColClick ), NULL, this );
    Connect( ID_RADIO_EDIT_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnStationsEditLabelsClicked ) );

    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextClearBitmap->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );

    Connect( ID_RADIO_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsPlay ) );
    Connect( ID_RADIO_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueue ) );
}

// -------------------------------------------------------------------------------- //
guRadioPanel::~guRadioPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteNum( wxT( "RadioGenreSashPos" ), m_GenreSplitter->GetSashPosition(), wxT( "Positions" ) );
        Config->WriteNum( wxT( "RadioStationSashPos" ), m_StationsSplitter->GetSashPosition(), wxT( "Positions" ) );
    }

    //
    m_GenresListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );

    m_LabelsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioLabelListSelected ), NULL, this );
    //
    Disconnect( ID_RADIO_DOUPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdate ) );
    Disconnect( ID_RADIO_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdated ) );
    Disconnect( ID_RADIO_UPDATE_END, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdateEnd ) );

    m_StationsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guRadioPanel::OnStationListActivated ), NULL, this );
	m_StationsListBox->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guRadioPanel::OnStationListBoxColClick ), NULL, this );
    Disconnect( ID_RADIO_EDIT_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnStationsEditLabelsClicked ) );

    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextClearBitmap->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );

    Disconnect( ID_RADIO_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsPlay ) );
    Disconnect( ID_RADIO_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueue ) );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchActivated( wxCommandEvent& event )
{
    wxArrayString Words = guSplitWords( m_InputTextCtrl->GetLineText( 0 ) );
    m_Db->SetRaTeFilters( Words );
    m_LabelsListBox->ReloadItems();
    m_GenresListBox->ReloadItems();
    m_StationsListBox->ReloadItems();
    m_InputTextClearBitmap->Enable();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchCancelled( wxMouseEvent &event ) // CLEAN SEARCH STR
{
    wxArrayString Words;
    m_InputTextCtrl->Clear();
    m_Db->SetRaTeFilters( Words );
    m_LabelsListBox->ReloadItems();
    m_GenresListBox->ReloadItems();
    m_StationsListBox->ReloadItems();
    m_InputTextClearBitmap->Disable();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListBoxColClick( wxListEvent &event )
{
    int col = event.GetColumn();
    if( col < 0 )
        return;
    m_StationsListBox->SetStationsOrder( col );
}


// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationsEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Labels;
    wxArrayInt Stations;

    m_Db->GetRadioLabels( &Labels, true );

    Stations = m_StationsListBox->GetSelectedItems();
    guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Stations Labels Editor" ), Labels, m_Db->GetStationsLabels( Stations ) );
    if( LabelEditor )
    {
        if( LabelEditor->ShowModal() == wxID_OK )
        {
            // Update the labels for the stations
            m_Db->SetRadioStationsLabels( Stations, LabelEditor->GetCheckedIds() );
        }
        LabelEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListActivated( wxListEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    OnSelectStations( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioGenreListSelected( wxListEvent &Event )
{
    m_Db->SetRadioGenresFilters( m_GenresListBox->GetSelectedItems() );
    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioLabelListSelected( wxListEvent &Event )
{
    m_Db->SetRadioLabelsFilters( m_LabelsListBox->GetSelectedItems() );
    m_GenresListBox->ReloadItems();
    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdate( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Radio Update fired" ) );
    m_GenresListBox->SetCursor( wxCURSOR_WATCH );

    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge();
    wxArrayInt GenresIds = m_GenresListBox->GetSelectedItems();
    if( !GenresIds.Count() )
    {
        guListItems Genres;
        m_Db->GetRadioGenres( &Genres );
        int index;
        int count = Genres.Count();
        for( index = 0; index < count; index++ )
        {
            GenresIds.Add( Genres[ index ].m_Id );
        }
    }

    if( GenresIds.Count() )
    {
        guUpdateRadiosThread * UpdateRadiosThread = new guUpdateRadiosThread( m_Db, this, GenresIds, GaugeId );
        if( UpdateRadiosThread )
        {
            UpdateRadiosThread->Create();
            UpdateRadiosThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
            UpdateRadiosThread->Run();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdated( wxCommandEvent &event )
{
    //GenresListBox->ReloadItems( false );
    m_StationsListBox->ReloadItems( false );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdateEnd( wxCommandEvent &event )
{
    m_GenresListBox->SetCursor( wxCURSOR_ARROW );
    //GenresListBox->Enable();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioStationsPlay( wxCommandEvent &event )
{
    OnSelectStations();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioStationsEnqueue( wxCommandEvent &event )
{
    OnSelectStations( true );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSelectStations( bool enqueue )
{
    guShoutCast ShoutCast;
    guTrackArray   Songs;
    guTrack *  NewSong;
    int index;
    int count;

    wxArrayInt Selected = m_StationsListBox->GetSelectedItems();
    if( Selected.Count() )
    {
        //TODO: Download the station in a thread
        guStationPlayLists PlayList = ShoutCast.GetStationPlayList( Selected[ 0 ] );
        if( ( count = PlayList.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                NewSong = new guTrack;
                if( NewSong )
                {
                    NewSong->m_SongId = guPLAYLIST_RADIOSTATION;
                    NewSong->m_FileName = PlayList[ index ].m_Url;
                    NewSong->m_SongName = PlayList[ index ].m_Name;
                    NewSong->m_Length = 0;
                    NewSong->m_Rating = -1;
                    //NewSong->CoverId = guPLAYLIST_RADIOSTATION;
                    NewSong->m_CoverId = 0;
                    NewSong->m_Year = 0;
                    Songs.Add( NewSong );
                }
            }

            if( Songs.Count() )
            {
                if( enqueue )
                {
                    m_PlayerPanel->AddToPlayList( Songs );
                }
                else
                {
                    m_PlayerPanel->SetPlayList( Songs );
                }
            }
        }
        else
        {
            wxMessageBox( _( "There are not entries for this Radio Station" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::GenreSplitterOnIdle( wxIdleEvent& )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_GenreSplitter->SetSashPosition( Config->ReadNum( wxT( "RadioGenreSashPos" ), 100, wxT( "Positions" ) ) );
    m_GenreSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guRadioPanel::GenreSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::StationsSplitterOnIdle( wxIdleEvent& )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_StationsSplitter->SetSashPosition( Config->ReadNum( wxT( "RadioStationsSashPos" ), 180, wxT( "Positions" ) ) );
    m_StationsSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guRadioPanel::StationsSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
// guUpdateRadiosThread
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
guUpdateRadiosThread::ExitCode guUpdateRadiosThread::Entry()
{
//    guListItems Genres;
    int index;
    int count;
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    guShoutCast * ShoutCast = new guShoutCast();
    guRadioStations RadioStations;
    if( ShoutCast )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        long MinBitRate;
        Config->ReadStr( wxT( "RadioMinBitRate" ), wxT( "128" ), wxT( "Radios" ) ).ToLong( &MinBitRate );
        //
//        m_Db->GetRadioGenres( &Genres, false );
//        guLogMessage( wxT ( "Loaded the genres" ) );
        guListItems Genres;
        m_Db->GetRadioGenresList( &Genres, m_GenresIds );

        //
        m_Db->DelRadioStations( m_GenresIds );
        //guLogMessage( wxT( "Deleted all radio stations" ) );
        count = Genres.Count();

        event.SetInt( m_GaugeId );
        event.SetExtraLong( count );
        wxPostEvent( wxTheApp->GetTopWindow(), event );

        for( index = 0; index < count; index++ )
        {
            guLogMessage( wxT( "Updating radiostations for genre '%s'" ), Genres[ index ].m_Name.c_str() );
            ShoutCast->GetStations( Genres[ index ].m_Name, Genres[ index ].m_Id, &RadioStations, MinBitRate );
            m_Db->SetRadioStations( RadioStations );
            RadioStations.Empty();

            //wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_UPDATED );
            event.SetId( ID_RADIO_UPDATED );
            wxPostEvent( m_RadioPanel, event );
            Sleep( 30 ); // Its wxThread::Sleep

//            wxCommandEvent event2( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_UPDATE );
            event.SetId( ID_GAUGE_UPDATE );
            event.SetInt( m_GaugeId );
            event.SetExtraLong( index );
            wxPostEvent( wxTheApp->GetTopWindow(), event );
        }

        delete ShoutCast;
    }
//    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_UPDATE_END );
    event.SetId( ID_RADIO_UPDATE_END );
    wxPostEvent( m_RadioPanel, event );
//    wxMilliSleep( 1 );

//    wxCommandEvent event2( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
    event.SetId( ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
    //
    return 0;
}

// -------------------------------------------------------------------------------- //
