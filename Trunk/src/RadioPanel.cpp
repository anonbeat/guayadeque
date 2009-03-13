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
      void GetItemsList( void );
      void GetContextMenu( wxMenu * Menu ) const;
      void OnRadioGenreAdd( wxCommandEvent &event );
      void OnRadioGenreEdit( wxCommandEvent &event );
      void OnRadioGenreDelete( wxCommandEvent &event );

    public :
      guRadioGenreListBox( wxWindow * parent, DbLibrary * NewDb, wxString Label );
      ~guRadioGenreListBox();

      int GetSelectedSongs( guTrackArray * Songs ) const; // For dag n drop support

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

  public:
    guUpdateRadiosThread( DbLibrary * db, guRadioPanel * radiopanel, int gaugeid = wxNOT_FOUND )
    {
        m_Db = db;
        m_RadioPanel = radiopanel;
        m_GaugeId = gaugeid;
    };

    ~guUpdateRadiosThread() {};

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
class guRadioStationListBox : public wxListCtrl
{
    protected :
      DbLibrary * m_Db;
      guRadioStations m_Radios;
      wxListItemAttr m_OddAttr;
      wxListItemAttr m_EveAttr;

      wxString          OnGetItemText( long item, long column ) const;
      wxListItemAttr *  OnGetItemAttr( long item ) const;
      void              OnBeginDrag( wxMouseEvent &event );
      void              OnContextMenu( wxContextMenuEvent& event );

    public :
                        guRadioStationListBox( wxWindow * parent, DbLibrary * NewDb );
                        ~guRadioStationListBox();

      void              ReloadItems( bool Reset = true );
      wxArrayInt        GetSelection( void ) const;
      guRadioStations   GetSelectedSongs() const;
      guRadioStations   GetAllSongs() const;
      void              ClearSelection();

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
    m_Items.Add( new guListItem( 0, _( "All" ) ) );
    m_Db->GetRadioGenres( &m_Items );
}

// -------------------------------------------------------------------------------- //
int guRadioGenreListBox::GetSelectedSongs( guTrackArray * Songs ) const
{
    //return Db->GetArtistsSongs( GetSelection(), Songs );
    return 0;
}

// -------------------------------------------------------------------------------- //
void guRadioGenreListBox::GetContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_RADIO_GENRE_ADD, _( "Add Genre" ), _( "Create a new genre" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_document_new ) );
    Menu->Append( MenuItem );

    if( GetSelection().Count() )
    {
        MenuItem = new wxMenuItem( Menu, ID_RADIO_GENRE_EDIT, _( "Edit genre" ), _( "Change selected genre" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_RADIO_GENRE_DELETE, _( "Delete genre" ), _( "Delete selected genre" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_edit_delete ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_RADIO_DOUPDATE, _( "Update Radio Stations" ), _( "Update the radio station lists" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
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
    wxArrayInt Selection = GetSelection();
    if( Selection.Count() )
    {
        // Get the Index of the First Selected Item
        int item = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Genre Name: " ), _( "Enter the new Genre Name" ), m_Items[ item ].m_Name );
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
    wxArrayInt Selection = GetSelection();
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

// -------------------------------------------------------------------------------- //
guRadioStationListBox::guRadioStationListBox( wxWindow * parent, DbLibrary * NewDb ) :
             wxListCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VIRTUAL )
{
    wxListItem ListItem;
    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_Db = NewDb;

    // Create the Columns
    ListItem.SetText( _( "Name" ) );
    ListItem.SetWidth( Config->ReadNum( wxT( "RadioColSize0" ), 400, wxT( "Positions" ) ) );
    InsertColumn( 0, ListItem );

    ListItem.SetText( _( "BitRate" ) );
    ListItem.SetWidth( Config->ReadNum( wxT( "RadioColSize1" ), 100, wxT( "Positions" ) ) );
    InsertColumn( 1, ListItem );

    ListItem.SetText( _( "Listeners*" ) );
    ListItem.SetWidth( Config->ReadNum( wxT( "RadioColSize2" ), 100, wxT( "Positions" ) ) );
    InsertColumn( 2, ListItem );

    //Connect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guRadioStationListBox::OnBeginDrag ), NULL, this );
    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guRadioStationListBox::OnContextMenu ), NULL, this );

    m_EveAttr = wxListItemAttr( wxColor( 0, 0, 0 ),
                              wxColor( 250, 250, 250 ),
                              wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

    m_OddAttr = wxListItemAttr( wxColor( 0, 0, 0 ),
                              wxColor( 240, 240, 240 ),
                              wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ) );

    SetBackgroundColour( wxColor( 250, 250, 250 ) );

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guRadioStationListBox::~guRadioStationListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    int Index;
    for( Index = 0; Index < 3; Index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "RadioColSize%u" ), Index ), GetColumnWidth( Index ), wxT( "Positions" ) );
    }

    m_Radios.Clear();

//    Disconnect( wxEVT_COMMAND_LIST_BEGIN_DRAG, wxMouseEventHandler( guRadioStationListBox::OnBeginDrag ), NULL, this );
    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guRadioStationListBox::OnContextMenu ), NULL, this );

}

// -------------------------------------------------------------------------------- //
wxString guRadioStationListBox::OnGetItemText( long item, long column ) const
{
//    wxString RetVal;
    guRadioStation * Radio;
    Radio = &m_Radios[ item ];
    switch( column )
    {
        case 0 :
          return Radio->m_Name;
          break;
        case 1 :
          return wxString::Format( wxT( "%u" ), Radio->m_BitRate );
          break;
        case 2 :
          return wxString::Format( wxT( "%u" ), Radio->m_Listeners );
    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
wxListItemAttr * guRadioStationListBox::OnGetItemAttr( long item ) const
{
    return ( wxListItemAttr * ) ( item & 1 ? &m_EveAttr : &m_OddAttr );
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::ReloadItems( bool Reset )
{
    long ItemCount;
    if( !m_Db )
        return;

    long FirstVisible = GetTopItem();
    if( Reset )
        ClearSelection();

    if( m_Radios.Count() )
        m_Radios.Empty();

    ItemCount = m_Db->GetRadioStations( &m_Radios );
    SetItemCount( ItemCount );

    if( !Reset )
    {
        EnsureVisible( FirstVisible );
        RefreshItems( FirstVisible, FirstVisible + GetCountPerPage() );
    }

    wxListItem ListItem;
    GetColumn( 0, ListItem );
    ListItem.SetText( wxString::Format( _( "Name (%u)" ), ItemCount ) );
    SetColumn( 0, ListItem );
}

// -------------------------------------------------------------------------------- //
wxArrayInt guRadioStationListBox::GetSelection( void ) const
{
    wxArrayInt RetVal;
    long item = -1;
    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        // this item is selected - do whatever is needed with it
        RetVal.Add( m_Radios[ item ].m_Id );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guRadioStations guRadioStationListBox::GetSelectedSongs() const
{
    guRadioStations RetVal;
    long item = -1;
    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        // this item is selected - do whatever is needed with it
        RetVal.Add( new guRadioStation( m_Radios[ item ] ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guRadioStations guRadioStationListBox::GetAllSongs() const
{
    guRadioStations RetVal;
    RetVal = m_Radios;
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::ClearSelection()
{
    long item = -1;
    while( ( item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != wxNOT_FOUND )
    {
        // this item is selected - do whatever is needed with it
        SetItemState( item, false, -1 );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::OnContextMenu( wxContextMenuEvent& event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPosition();
    // If from keyboard
    if( Point.x == -1 && Point.y == -1 )
    {
        wxSize Size = GetSize();
        Point.x = Size.x / 2;
        Point.y = Size.y / 2;
    }
    else
    {
        Point = ScreenToClient( Point );
    }
    MenuItem = new wxMenuItem( &Menu, ID_SONG_PLAY, _( "Play" ), _( "Play current selected songs" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_media_playback_start ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_SONG_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
    Menu.Append( MenuItem );

//    Menu.AppendSeparator();
//
//    MenuItem = new wxMenuItem( &Menu, ID_SONG_EDITTAGS, wxT( "Edit Tags" ), wxT( "Edit the tags assigned to the selected songs" ) );
//    MenuItem->SetBitmap( wxBitmap( GU_CONFIG_IMAGES_DIR + wxT("/images/gtk-edit.png"), wxBITMAP_TYPE_ANY ) );
//    Menu.Append( MenuItem );

    PopupMenu( &Menu, Point.x, Point.y );
}


// -------------------------------------------------------------------------------- //
// guRadioPanel
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
guRadioPanel::guRadioPanel( wxWindow* parent, DbLibrary * NewDb, guPlayerPanel * NewPlayerPanel ) :
              wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
	wxBoxSizer * MainSizer;
	wxBoxSizer * SearchSizer;
	wxBoxSizer * InputTextSizer;
	wxBoxSizer * ListsSizer;
	wxBoxSizer * GenresSizer;
	wxBoxSizer * RadiosSizer;

    m_Db = NewDb;
    m_PlayerPanel = NewPlayerPanel;

    guConfig *  Config = ( guConfig * ) guConfig::Get();


	MainSizer = new wxBoxSizer( wxVERTICAL );

	SearchSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SearchStaticText = new wxStaticText( this, wxID_ANY, _( "Search:" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_SearchStaticText->Wrap( -1 );
	SearchSizer->Add( m_SearchStaticText, 0, wxALIGN_CENTER|wxALL, 5 );

	m_InputTextPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	m_InputTextPanel->SetBackgroundColour( wxColor( 250, 250, 250 ) );

	InputTextSizer = new wxBoxSizer( wxHORIZONTAL );

	m_InputTextLeftBitmap = new wxStaticBitmap( m_InputTextPanel, wxID_ANY, wxBitmap( guImage_search ), wxDefaultPosition, wxDefaultSize, 0 );
	InputTextSizer->Add( m_InputTextLeftBitmap, 0, wxALL, 0 );

	m_InputTextCtrl = new wxTextCtrl( m_InputTextPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxNO_BORDER );
    m_InputTextCtrl->SetBackgroundColour( wxColor( 250, 250, 250 ) );
	InputTextSizer->Add( m_InputTextCtrl, 1, wxALL, 2 );

	m_InputTextClearBitmap = new wxStaticBitmap( m_InputTextPanel, wxID_ANY, wxBitmap( guImage_edit_clear ), wxDefaultPosition, wxDefaultSize, 0 );
	m_InputTextClearBitmap->Disable();
	InputTextSizer->Add( m_InputTextClearBitmap, 0, wxALL, 0 );

	m_InputTextPanel->SetSizer( InputTextSizer );
	m_InputTextPanel->Layout();
	InputTextSizer->Fit( m_InputTextPanel );
	SearchSizer->Add( m_InputTextPanel, 1, wxEXPAND | wxALL, 2 );

	MainSizer->Add( SearchSizer, 0, wxEXPAND, 5 );

	m_SearchStaticline = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MainSizer->Add( m_SearchStaticline, 0, wxEXPAND | wxALL, 5 );

	ListsSizer = new wxBoxSizer( wxVERTICAL );

	m_ListsSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_ListsSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guRadioPanel::ListsSplitterOnIdle ), NULL, this );
    m_ListsSplitter->SetMinimumPaneSize( 100 );

	m_GenresPanel = new wxPanel( m_ListsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_GenresPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	GenresSizer = new wxBoxSizer( wxVERTICAL );

	//GenresListBox = new wxListCtrl( GenresPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_LIST|wxLC_SINGLE_SEL|wxLC_VIRTUAL );
	m_GenresListBox = new guRadioGenreListBox( m_GenresPanel, m_Db, _( "Genres" ) );
	GenresSizer->Add( m_GenresListBox, 1, wxALL|wxEXPAND, 1 );

	m_GenresPanel->SetSizer( GenresSizer );
	m_GenresPanel->Layout();
	GenresSizer->Fit( m_GenresPanel );
	m_RadiosPanel = new wxPanel( m_ListsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_RadiosPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	RadiosSizer = new wxBoxSizer( wxVERTICAL );

	m_StationsListBox = new guRadioStationListBox( m_RadiosPanel, m_Db );
	RadiosSizer->Add( m_StationsListBox, 1, wxALL|wxEXPAND, 1 );

	m_RadiosPanel->SetSizer( RadiosSizer );
	m_RadiosPanel->Layout();
	RadiosSizer->Fit( m_RadiosPanel );

	m_ListsSplitter->SplitVertically( m_GenresPanel, m_RadiosPanel, Config->ReadNum( wxT( "RadioSashPos" ), 180, wxT( "Positions" ) )  );

	ListsSizer->Add( m_ListsSplitter, 1, wxEXPAND, 2 );

	MainSizer->Add( ListsSizer, 1, wxEXPAND, 5 );

	SetSizer( MainSizer );
	Layout();

    m_GenresListBox->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );
    m_GenresListBox->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );
    //
    Connect( ID_RADIO_DOUPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdate ) );
    Connect( ID_RADIO_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdated ) );
    Connect( ID_RADIO_UPDATE_END, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdateEnd ) );

    m_StationsListBox->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( guRadioPanel::OnStationListActivated ), NULL, this );
	m_StationsListBox->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guRadioPanel::OnStationListBoxColClick ), NULL, this );

    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextClearBitmap->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guRadioPanel::~guRadioPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        //guLogMessage( wxT( "guRadioPanel::guConfig Save %u" ), ListsSplitter->GetSashPosition() );
        Config->WriteNum( wxT( "RadioSashPos" ), m_ListsSplitter->GetSashPosition(), wxT( "Positions" ) );
    }

    //
    m_GenresListBox->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );
    m_GenresListBox->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,  wxListEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );
    //
    Disconnect( ID_RADIO_DOUPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdate ) );
    Disconnect( ID_RADIO_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdated ) );
    Disconnect( ID_RADIO_UPDATE_END, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdateEnd ) );

    m_StationsListBox->Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( guRadioPanel::OnStationListActivated ), NULL, this );
	m_StationsListBox->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guRadioPanel::OnStationListBoxColClick ), NULL, this );

    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextClearBitmap->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchActivated( wxCommandEvent& event )
{
    wxArrayString Words = guSplitWords( m_InputTextCtrl->GetLineText( 0 ) );
    m_Db->SetRaTeFilters( Words );
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
    m_GenresListBox->ReloadItems();
    m_StationsListBox->ReloadItems();
    m_InputTextClearBitmap->Disable();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListBoxColClick( wxListEvent &event )
{
    wxListItem ListItem;
    wxString LabelStr;

    int col = event.GetColumn();
    if( col == 1 ) // BitRate
    {
        m_Db->SetRadioStatonsOrder( RADIOSTATIONS_ORDER_BITRATE );
    }
    else if( col == 2 ) // Listeners
    {
        m_Db->SetRadioStatonsOrder( RADIOSTATIONS_ORDER_LISTENERS );
    }
    else
    {
        event.Skip();
        return;
    }

    m_StationsListBox->GetColumn( 1, ListItem );
    LabelStr = _( "Bitrate" );
    if( col == 1 )
      LabelStr += wxT( "*" );
    ListItem.SetText( LabelStr );
    m_StationsListBox->SetColumn( 1, ListItem );

    m_StationsListBox->GetColumn( 2, ListItem );
    LabelStr = _( "Listeners" );
    if( col == 2 )
      LabelStr += wxT( "*" );
    ListItem.SetText( LabelStr );
    m_StationsListBox->SetColumn( 2, ListItem );

    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListActivated( wxListEvent &event )
{
    guShoutCast ShoutCast;
    guTrackArray   Songs;
    guTrack *  NewSong;
    int index;
    int count;

//    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
//    int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge();
//
//    wxCommandEvent GaugeEvent( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
//    GaugeEvent.SetInt( GaugeId );
//    GaugeEvent.SetExtraLong( 5 );
//    MainFrame->AddPendingEvent( GaugeEvent );

    wxArrayInt Selected = m_StationsListBox->GetSelection();
    if( Selected.Count() )
    {
//        GaugeEvent.SetId( ID_GAUGE_UPDATE );
//        GaugeEvent.SetExtraLong( 1 );
//        wxPostEvent( MainFrame, GaugeEvent );

        guStationPlayLists PlayList = ShoutCast.GetStationPlayList( Selected[ 0 ] );
        if( ( count = PlayList.Count() ) )
        {
//            //GaugeEvent.SetId( ID_GAUGE_UPDATE );
//            GaugeEvent.SetExtraLong( 2 );
//            wxPostEvent( MainFrame, GaugeEvent );

            for( index = 0; index < count; index++ )
            {
                NewSong = new guTrack;
                if( NewSong )
                {
                    NewSong->m_SongId = guPLAYLIST_RADIOSTATION;
                    NewSong->m_FileName = PlayList[ index ].m_Url;
                    NewSong->m_SongName = PlayList[ index ].m_Name;
                    NewSong->m_Length = 0;
                    //NewSong->CoverId = guPLAYLIST_RADIOSTATION;
                    NewSong->m_CoverId = 0;
                    Songs.Add( NewSong );
                }
            }

//            //GaugeEvent.SetId( ID_GAUGE_UPDATE );
//            GaugeEvent.SetExtraLong( 3 );
//            wxPostEvent( MainFrame, GaugeEvent );

            if( Songs.Count() )
              m_PlayerPanel->SetPlayList( Songs );

//            //GaugeEvent.SetId( ID_GAUGE_UPDATE );
//            GaugeEvent.SetExtraLong( 4 );
//            wxPostEvent( MainFrame, GaugeEvent );

        }
    }

//    GaugeEvent.SetId( ID_GAUGE_REMOVE );
//    //GaugeEvent.SetExtraLong( 5 );
//    wxPostEvent( MainFrame, GaugeEvent );

}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioGenreListSelected( wxListEvent &Event )
{
    m_Db->SetRadioGenresFilters( m_GenresListBox->GetSelection() );
    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdate( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Radio Update fired" ) );
    m_GenresListBox->SetCursor( wxCURSOR_WATCH );

    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge();

    guUpdateRadiosThread * UpdateRadiosThread = new guUpdateRadiosThread( m_Db, this, GaugeId );
    if( UpdateRadiosThread )
    {
        UpdateRadiosThread->Create();
        UpdateRadiosThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        UpdateRadiosThread->Run();
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
void guRadioPanel::ListsSplitterOnIdle( wxIdleEvent& )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_ListsSplitter->SetSashPosition( Config->ReadNum( wxT( "RadioSashPos" ), 180, wxT( "Positions" ) ) );
    m_ListsSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guRadioPanel::ListsSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
// guUpdateRadiosThread
// -------------------------------------------------------------------------------- //

// -------------------------------------------------------------------------------- //
guUpdateRadiosThread::ExitCode guUpdateRadiosThread::Entry()
{
    guListItems Genres;
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
        m_Db->GetRadioGenres( &Genres, false );
        guLogMessage( wxT ( "Loaded the genres" ) );
        //
        m_Db->DelRadioStations();
        guLogMessage( wxT( "Deleted all radio stations" ) );
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
