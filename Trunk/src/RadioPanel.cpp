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
#include "RadioPanel.h"

#include "AuiDockArt.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "LabelEditor.h"
#include "MainFrame.h"
#include "PlayListFile.h"
#include "RadioGenreEditor.h"
#include "RadioEditor.h"
#include "StatusBar.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/wfstream.h>
#include <wx/treectrl.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

#define guRADIO_TIMER_TEXTSEARCH        1
#define guRADIO_TIMER_TEXTSEARCH_VALUE  500

// -------------------------------------------------------------------------------- //
// guShoutcastItemData
// -------------------------------------------------------------------------------- //
class guShoutcastItemData : public wxTreeItemData
{
  private :
    int         m_Id;
    int         m_Source;
    wxString    m_Name;
    int         m_Flags;

  public :
    guShoutcastItemData( const int id, const int source, const wxString &name, int flags )
    {
        m_Id = id;
        m_Source = source;
        m_Name = name;
        m_Flags = flags;
    }

    int         GetId( void ) { return m_Id; }
    void        SetId( int id ) { m_Id = id; }
    int         GetSource( void ) { return m_Source; }
    void        SetSource( int source ) { m_Source = source; }
    int         GetFlags( void ) { return m_Flags; }
    void        SetFlags( int flags ) { m_Flags = flags; }
    wxString    GetName( void ) { return m_Name; }
    void        SetName( const wxString &name ) { m_Name = name; }

};

// -------------------------------------------------------------------------------- //
guShoutcastSearch::guShoutcastSearch( wxWindow * parent, guShoutcastItemData * itemdata ) :
    wxDialog( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 435,237 ), wxDEFAULT_DIALOG_STYLE )
{
    m_ItemData = itemdata;
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* LabelSizer;
	LabelSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _(" Radio Search ") ), wxVERTICAL );

	wxBoxSizer* SearchSizer;
	SearchSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * SearchLabel = new wxStaticText( this, wxID_ANY, _("Search:"), wxDefaultPosition, wxDefaultSize, 0 );
	SearchLabel->Wrap( -1 );
	SearchSizer->Add( SearchLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_SearchTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_SearchTextCtrl->SetValue( m_ItemData->GetName() );
	SearchSizer->Add( m_SearchTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	LabelSizer->Add( SearchSizer, 0, wxEXPAND, 5 );

	m_SearchPlayChkBox = new wxCheckBox( this, wxID_ANY, _("Search in now playing info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SearchPlayChkBox->SetValue( m_ItemData->GetFlags() & guRADIO_SEARCH_FLAG_NOWPLAYING );
	LabelSizer->Add( m_SearchPlayChkBox, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_SearchGenreChkBox = new wxCheckBox( this, wxID_ANY, _("Search in genre names"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SearchGenreChkBox->SetValue( m_ItemData->GetFlags() & guRADIO_SEARCH_FLAG_GENRE );
	LabelSizer->Add( m_SearchGenreChkBox, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_SearchNameChkBox = new wxCheckBox( this, wxID_ANY, _("Search in station names"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SearchNameChkBox->SetValue( m_ItemData->GetFlags() & guRADIO_SEARCH_FLAG_STATION );
	LabelSizer->Add( m_SearchNameChkBox, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_AllGenresChkBox = new wxCheckBox( this, wxID_ANY, _("Include results from all genres"), wxDefaultPosition, wxDefaultSize, 0 );
	m_AllGenresChkBox->SetValue( m_ItemData->GetFlags() & guRADIO_SEARCH_FLAG_ALLGENRES );
	LabelSizer->Add( m_AllGenresChkBox, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	MainSizer->Add( LabelSizer, 1, wxEXPAND|wxALL, 5 );

	wxStdDialogButtonSizer * ButtonsSizer = new wxStdDialogButtonSizer();
	wxButton * ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	wxButton * ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	Connect( wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guShoutcastSearch::OnOkButton ) );
}

// -------------------------------------------------------------------------------- //
guShoutcastSearch::~guShoutcastSearch()
{
	Disconnect( wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guShoutcastSearch::OnOkButton ) );
}

// -------------------------------------------------------------------------------- //
void guShoutcastSearch::OnOkButton( wxCommandEvent &event )
{
    m_ItemData->SetName( m_SearchTextCtrl->GetValue() );

    int flags = guRADIO_SEARCH_FLAG_NONE;

    if( m_SearchPlayChkBox->IsChecked() )
        flags |= guRADIO_SEARCH_FLAG_NOWPLAYING;

    if( m_SearchGenreChkBox->IsChecked() )
        flags |= guRADIO_SEARCH_FLAG_GENRE;

    if( m_SearchNameChkBox->IsChecked() )
        flags |= guRADIO_SEARCH_FLAG_STATION;

    if( m_AllGenresChkBox->IsChecked() )
        flags |= guRADIO_SEARCH_FLAG_ALLGENRES;

    m_ItemData->SetFlags( flags );

    event.Skip();
}




// -------------------------------------------------------------------------------- //
// guRadioGenreTreeCtrl
// -------------------------------------------------------------------------------- //
class guRadioGenreTreeCtrl : public wxTreeCtrl
{
  private :
    guDbRadios *    m_Db;
    wxImageList *   m_ImageList;
    wxTreeItemId    m_RootId;
    wxTreeItemId    m_ManualId;
    wxTreeItemId    m_ShoutcastId;
    wxTreeItemId    m_ShoutcastGenreId;
    wxTreeItemId    m_ShoutcastSearchsId;

    void            OnContextMenu( wxTreeEvent &event );
    void            OnRadioGenreAdd( wxCommandEvent &event );
    void            OnRadioGenreEdit( wxCommandEvent &event );
    void            OnRadioGenreDelete( wxCommandEvent &event );
    void            OnKeyDown( wxKeyEvent &event );

  public :
    guRadioGenreTreeCtrl( wxWindow * parent, guDbRadios * db );
    ~guRadioGenreTreeCtrl();

    void            ReloadItems( void );
    wxTreeItemId *  GetShoutcastId( void ) { return &m_ShoutcastId; };
    wxTreeItemId *  GetShoutcastGenreId( void ) { return &m_ShoutcastGenreId; };
    wxTreeItemId *  GetShoutcastSearchId( void ) { return &m_ShoutcastSearchsId; };
    wxTreeItemId *  GetManualId( void ) { return &m_ManualId; };
    wxTreeItemId    GetItemId( wxTreeItemId * itemid, const int id );

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

      guRadioLabelListBox( wxWindow * parent, guDbRadios * NewDb, wxString Label );
      ~guRadioLabelListBox();

};

// -------------------------------------------------------------------------------- //
// guUpdateRadiosThread
// -------------------------------------------------------------------------------- //
class guUpdateRadiosThread : public wxThread
{
  private:
    guDbRadios *    m_Db;
    guRadioPanel *  m_RadioPanel;
    int             m_GaugeId;
    wxArrayInt      m_Ids;
    int             m_Source;

    void            CheckRadioStationsFilters( const int flags, const wxString &text, guRadioStations &stations );

  public:
    guUpdateRadiosThread( guDbRadios * db, guRadioPanel * radiopanel,
                                const wxArrayInt &ids, const int source, int gaugeid = wxNOT_FOUND );

    ~guUpdateRadiosThread(){};

    virtual ExitCode Entry();
};



#define guRADIOSTATIONS_COLUMN_NAME         0
#define guRADIOSTATIONS_COLUMN_BITRATE      1
#define guRADIOSTATIONS_COLUMN_LISTENERS    2
#define guRADIOSTATIONS_COLUMN_TYPE         3
#define guRADIOSTATIONS_COLUMN_NOWPLAYING   4

#define guRADIOSTATIONS_COLUMN_COUNT        5

// -------------------------------------------------------------------------------- //
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
class guRadioStationListBox : public guListView
{
  protected :
    guDbRadios *      m_Db;
    guRadioStations   m_Radios;
    int               m_StationsOrder;
    bool              m_StationsOrderDesc;

    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

    virtual wxArrayString       GetColumnNames( void );

  public :
    guRadioStationListBox( wxWindow * parent, guDbRadios * NewDb );
    ~guRadioStationListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual int inline          GetItemId( const int row ) const;
    virtual wxString inline     GetItemName( const int row ) const;

    void                        SetStationsOrder( int order );
    bool                        GetSelected( guRadioStation * radiostation ) const;
};




// -------------------------------------------------------------------------------- //
// guRadioGenreTreeCtrl
// -------------------------------------------------------------------------------- //
guRadioGenreTreeCtrl::guRadioGenreTreeCtrl( wxWindow * parent, guDbRadios * db ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_SINGLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT )
{
    m_Db = db;
    m_ImageList = new wxImageList();
    m_ImageList->Add( guImage( guIMAGE_INDEX_tiny_shoutcast ) );
    m_ImageList->Add( wxBitmap( guImage( guIMAGE_INDEX_tiny_net_radio ) ) );
    m_ImageList->Add( wxBitmap( guImage( guIMAGE_INDEX_tiny_search ) ) );

    AssignImageList( m_ImageList );

    m_RootId   = AddRoot( wxT( "Radios" ), -1, -1, NULL );
    m_ShoutcastId = AppendItem( m_RootId, _( "Shoutcast" ), 0, 0, NULL );
    m_ShoutcastGenreId = AppendItem( m_ShoutcastId, _( "Genre" ), 0, 0, NULL );
    m_ShoutcastSearchsId = AppendItem( m_ShoutcastId, _( "Searches" ), 2, 2, NULL );
    m_ManualId = AppendItem( m_RootId, _( "User Defined" ), 1, 1, NULL );

    SetIndent( 5 );

    Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guRadioGenreTreeCtrl::OnContextMenu ), NULL, this );
    Connect( ID_RADIO_GENRE_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreAdd ) );
    Connect( ID_RADIO_GENRE_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreEdit ) );
    Connect( ID_RADIO_GENRE_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreDelete ) );
    Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guRadioGenreTreeCtrl::OnKeyDown ), NULL, this );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guRadioGenreTreeCtrl::~guRadioGenreTreeCtrl()
{
    Disconnect( ID_RADIO_GENRE_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreAdd ) );
    Disconnect( ID_RADIO_GENRE_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreEdit ) );
    Disconnect( ID_RADIO_GENRE_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreDelete ) );
    Disconnect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guRadioGenreTreeCtrl::OnContextMenu ), NULL, this );

    Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guRadioGenreTreeCtrl::OnKeyDown ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::ReloadItems( void )
{
    DeleteChildren( m_ShoutcastGenreId );

    guListItems RadioGenres;
    m_Db->GetRadioGenres( guRADIO_SOURCE_GENRE, &RadioGenres );

    int index;
    int count = RadioGenres.Count();
    for( index = 0; index < count; index++ )
    {
        AppendItem( m_ShoutcastGenreId, RadioGenres[ index ].m_Name, -1, -1,
            new guShoutcastItemData( RadioGenres[ index ].m_Id, guRADIO_SOURCE_GENRE, RadioGenres[ index ].m_Name, 0 ) );
    }

    DeleteChildren( m_ShoutcastSearchsId );
    guListItems RadioSearchs;
    wxArrayInt RadioFlags;
    m_Db->GetRadioGenres( guRADIO_SOURCE_SEARCH, &RadioSearchs, true, &RadioFlags );
    count = RadioSearchs.Count();
    for( index = 0; index < count; index++ )
    {
        AppendItem( m_ShoutcastSearchsId, RadioSearchs[ index ].m_Name, -1, -1,
            new guShoutcastItemData( RadioSearchs[ index ].m_Id, guRADIO_SOURCE_SEARCH, RadioSearchs[ index ].m_Name, RadioFlags[ index ] ) );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPoint();

    wxTreeItemId ItemId = event.GetItem();
    guShoutcastItemData * ItemData = ( guShoutcastItemData * ) GetItemData( ItemId );


    if( ( ItemData && ( ItemData->GetSource() == guRADIO_SOURCE_GENRE ) ) || ( ItemId == * GetShoutcastGenreId() ) )
    {
        MenuItem = new wxMenuItem( &Menu, ID_RADIO_GENRE_ADD, _( "Add Genre" ), _( "Create a new genre" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu.Append( MenuItem );

        if( ItemData )
        {
            MenuItem = new wxMenuItem( &Menu, ID_RADIO_GENRE_EDIT, _( "Edit genre" ), _( "Change the selected genre" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_RADIO_GENRE_DELETE, _( "Delete genre" ), _( "Delete the selected search" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_del ) );
            Menu.Append( MenuItem );
        }

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_DOUPDATE, _( "Update Radio Stations" ), _( "Update the radio station lists" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_reload ) );
        Menu.Append( MenuItem );
    }
    else if( ( ItemData && ( ItemData->GetSource() == guRADIO_SOURCE_SEARCH ) ) || ( ItemId == * GetShoutcastSearchId() ) )
    {
        MenuItem = new wxMenuItem( &Menu, ID_RADIO_SEARCH_ADD, _( "Add Search" ), _( "Create a new search" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu.Append( MenuItem );

        if( ItemData )
        {
            MenuItem = new wxMenuItem( &Menu, ID_RADIO_SEARCH_EDIT, _( "Edit search" ), _( "Change the selected search" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_RADIO_SEARCH_DELETE, _( "Delete search" ), _( "Delete the selected search" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_del ) );
            Menu.Append( MenuItem );
        }

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_DOUPDATE, _( "Update Radio Stations" ), _( "Update the radio station lists" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_reload ) );
        Menu.Append( MenuItem );
    }
    else if( ItemId == * GetManualId() )
    {
        MenuItem = new wxMenuItem( &Menu, ID_RADIO_USER_ADD, _( "Add Radio" ), _( "Create a new radio" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu.Append( MenuItem );

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_USER_IMPORT, _( "Import" ), _( "Import the radio stations" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_USER_EXPORT, _( "Export" ), _( "Export all the radio stations" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu.Append( MenuItem );
    }
    else
    {
        MenuItem = new wxMenuItem( &Menu, ID_RADIO_DOUPDATE, _( "Update Radio Stations" ), _( "Update the radio station lists" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_reload ) );
        Menu.Append( MenuItem );
    }

    PopupMenu( &Menu, Point );
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnRadioGenreAdd( wxCommandEvent &event )
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
                    m_Db->AddRadioGenre( NewGenres[ index ], guRADIO_SOURCE_GENRE, guRADIO_SEARCH_FLAG_NONE );
                }
                ReloadItems();
            }
        }
        //
        RadioGenreEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnRadioGenreEdit( wxCommandEvent &event )
{
    wxTreeItemId ItemId = GetSelection();

    if( ItemId.IsOk() )
    {
        guShoutcastItemData * RadioGenreData = ( guShoutcastItemData * ) GetItemData( ItemId );

        if( RadioGenreData )
        {
            // Get the Index of the First Selected Item
            wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Genre Name: " ), _( "Enter the new Genre Name" ), RadioGenreData->GetName() );
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                m_Db->SetRadioGenre( RadioGenreData->GetId(), EntryDialog->GetValue() );
                ReloadItems();
            }
            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnRadioGenreDelete( wxCommandEvent &event )
{
    wxArrayTreeItemIds ItemIds;
    int index;
    int count;

    if( ( count = GetSelections( ItemIds ) ) )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected radio genres?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            guShoutcastItemData * RadioGenreData;
            for( index = 0; index < count; index++ )
            {
                RadioGenreData = ( guShoutcastItemData * ) GetItemData( ItemIds[ index ] );
                if( RadioGenreData )
                {
                    m_Db->DelRadioGenre( RadioGenreData->GetId() );
                }
            }
            ReloadItems();
        }
    }
}

// -------------------------------------------------------------------------------- //
wxTreeItemId guRadioGenreTreeCtrl::GetItemId( wxTreeItemId * itemid, const int id )
{
    wxTreeItemIdValue Cookie;
    wxTreeItemId CurItem = GetFirstChild( * itemid, Cookie );
    while( CurItem.IsOk() )
    {
        guShoutcastItemData * ItemData = ( guShoutcastItemData * ) GetItemData( CurItem );
        if( ItemData )
        {
            if( ItemData->GetId() == id )
                return CurItem;
        }
        else
        {
            wxTreeItemId ChildItem = GetItemId( &CurItem, id );
            if( ChildItem.IsOk() )
                return ChildItem;
        }
        CurItem = GetNextChild( * itemid, Cookie );
    }
    return CurItem;
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_GENRE_DELETE );
        wxPostEvent( this, CmdEvent );
        return;
    }
    event.Skip();
}





// -------------------------------------------------------------------------------- //
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
guRadioStationListBox::guRadioStationListBox( wxWindow * parent, guDbRadios * db ) :
    guListView( parent, wxLB_SINGLE | guLISTVIEW_COLUMN_SELECT|guLISTVIEW_COLUMN_SORTING )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_StationsOrder = Config->ReadNum( wxT( "StationsOrder" ), 0, wxT( "General" ) );
    m_StationsOrderDesc = Config->ReadNum( wxT( "StationsOrderDesc" ), false, wxT( "General" ) );;

    wxArrayString ColumnNames = GetColumnNames();
    // Create the Columns
    int ColId;
    wxString ColName;
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "RadioCol%u" ), index ), index, wxT( "RadioColumns" ) );

        ColName = ColumnNames[ ColId ];

        ColName += ( ( ColId == m_StationsOrder ) ? ( m_StationsOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "RadioColWidth%u" ), index ), 80, wxT( "RadioColumns" ) ),
            Config->ReadBool( wxString::Format( wxT( "RadioColShow%u" ), index ), true, wxT( "RadioColumns" ) )
            );
        InsertColumn( Column );
    }

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guRadioStationListBox::~guRadioStationListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    //int ColId;
    int index;
    int count = guRADIOSTATIONS_COLUMN_COUNT;
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "RadioCol%u" ), index ), ( * m_Columns )[ index ].m_Id, wxT( "RadioColumns" ) );
        Config->WriteNum( wxString::Format( wxT( "RadioColWidth%u" ), index ), ( * m_Columns )[ index ].m_Width, wxT( "RadioColumns" ) );
        Config->WriteBool( wxString::Format( wxT( "RadioColShow%u" ), index ), ( * m_Columns )[ index ].m_Enabled, wxT( "RadioColumns" ) );
    }

    Config->WriteNum( wxT( "StationsOrder" ), m_StationsOrder, wxT( "General" ) );
    Config->WriteBool( wxT( "StationsOrderDesc" ), m_StationsOrderDesc, wxT( "General" ) );;
}

// -------------------------------------------------------------------------------- //
wxArrayString guRadioStationListBox::GetColumnNames( void )
{
    wxArrayString ColumnNames;
    ColumnNames.Add( _( "Name" ) );
    ColumnNames.Add( _( "BitRate" ) );
    ColumnNames.Add( _( "Listeners" ) );
    ColumnNames.Add( _( "Format" ) );
    ColumnNames.Add( _( "Now Playing" ) );
    return ColumnNames;
}

// -------------------------------------------------------------------------------- //
wxString guRadioStationListBox::OnGetItemText( const int row, const int col ) const
{
    guRadioStation * Radio;
    Radio = &m_Radios[ row ];
    switch( ( * m_Columns )[ col ].m_Id )
    {
        case guRADIOSTATIONS_COLUMN_NAME :
          return Radio->m_Name;
          break;

        case guRADIOSTATIONS_COLUMN_BITRATE :
          return wxString::Format( wxT( "%u" ), Radio->m_BitRate );
          break;

        case guRADIOSTATIONS_COLUMN_LISTENERS :
          return wxString::Format( wxT( "%u" ), Radio->m_Listeners );

        case guRADIOSTATIONS_COLUMN_TYPE :
          return Radio->m_Type;
          break;

        case guRADIOSTATIONS_COLUMN_NOWPLAYING :
          return Radio->m_NowPlaying;
          break;
}
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
void guRadioStationListBox::GetItemsList( void )
{
    m_Db->GetRadioStations( &m_Radios );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
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
    wxArrayInt Selection = GetSelectedItems();

    if( Selection.Count() )
    {
        MenuItem = new wxMenuItem( Menu, ID_RADIO_PLAY, _( "Play" ), _( "Play current selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_RADIO_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_RADIO_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Add current selected songs to playlist as Next Tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( Menu, ID_RADIO_USER_ADD, _( "Add Radio" ), _( "Add a radio" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
    Menu->Append( MenuItem );

    if( Selection.Count() )
    {
        guRadioStation RadioStation;
        GetSelected( &RadioStation );
        if( RadioStation.m_Source == 1 )
        {
            MenuItem = new wxMenuItem( Menu, ID_RADIO_USER_EDIT, _( "Edit Radio" ), _( "Change the selected radio" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
            Menu->Append( MenuItem );

            MenuItem = new wxMenuItem( Menu, ID_RADIO_USER_DEL, _( "Delete Radio" ), _( "Delete the selected radio" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
            Menu->Append( MenuItem );

            Menu->AppendSeparator();
        }

        MenuItem = new wxMenuItem( Menu, ID_RADIO_EDIT_LABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected stations" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu->Append( MenuItem );
    }
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

    wxArrayString ColumnNames = GetColumnNames();
    int CurColId;
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        CurColId = GetColumnId( index );
        SetColumnLabel( index,
            ColumnNames[ CurColId ]  + ( ( order == CurColId ) ? ( m_StationsOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();


//    wxArrayString ColumnNames = m_SongListCtrl->GetColumnNames();
//    int CurColId;
//    int index;
//    int count = ColumnNames.Count();
//    for( index = 0; index < count; index++ )
//    {
//        CurColId = m_SongListCtrl->GetColumnId( index );
//        m_SongListCtrl->SetColumnLabel( index,
//            ColumnNames[ CurColId ]  + ( ( ColId == CurColId ) ? ( m_Db->GetSongsOrderDesc() ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
//    }
//
//    m_SongListCtrl->ReloadItems();
}

// -------------------------------------------------------------------------------- //
bool guRadioStationListBox::GetSelected( guRadioStation * radiostation ) const
{
    if( !m_Radios.Count() )
        return false;

    int Selected = GetSelection();
    if( Selected == wxNOT_FOUND )
    {
        Selected = 0;
    }
    radiostation->m_Id          = m_Radios[ Selected ].m_Id;
    radiostation->m_SCId        = m_Radios[ Selected ].m_SCId;
    radiostation->m_BitRate     = m_Radios[ Selected ].m_BitRate;
    radiostation->m_GenreId     = m_Radios[ Selected ].m_GenreId;
    radiostation->m_Source      = m_Radios[ Selected ].m_Source;
    radiostation->m_Link        = m_Radios[ Selected ].m_Link;
    radiostation->m_Listeners   = m_Radios[ Selected ].m_Listeners;
    radiostation->m_Name        = m_Radios[ Selected ].m_Name;
    radiostation->m_Type        = m_Radios[ Selected ].m_Type;
    return true;
}

// -------------------------------------------------------------------------------- //
// guRadioLabelListBox
// -------------------------------------------------------------------------------- //
guRadioLabelListBox::guRadioLabelListBox( wxWindow * parent, guDbRadios * db, wxString label ) :
    guListBox( parent, ( guDbLibrary * ) db, label, wxLB_MULTIPLE | guLISTVIEW_HIDE_HEADER )
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
    ( ( guDbRadios * ) m_Db )->GetRadioLabels( m_Items );
}

// -------------------------------------------------------------------------------- //
void guRadioLabelListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ADD, _( "Add Label" ), _( "Create a new label" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
    Menu->Append( MenuItem );

    if( GetSelectedCount() )
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
        ( ( guDbRadios * ) m_Db )->AddRadioLabel( EntryDialog->GetValue() );
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
                ( ( guDbRadios * ) m_Db )->DelRadioLabel( Selection[ Index ] );
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
            ( ( guDbRadios * ) m_Db )->SetRadioLabelName( Selection[ 0 ], EntryDialog->GetValue() );
            ReloadItems();
        }
        EntryDialog->Destroy();
    }
}




// -------------------------------------------------------------------------------- //
// guRadioPanel
// -------------------------------------------------------------------------------- //
guRadioPanel::guRadioPanel( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel ) :
              wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL ),
              m_TextChangedTimer( this, guRADIO_TIMER_TEXTSEARCH )
{
    m_Db = new guDbRadios( db );
    m_PlayerPanel = playerpanel;
    m_RadioPlayListLoadThread = NULL;

    guConfig *  Config = ( guConfig * ) guConfig::Get();

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

    m_VisiblePanels = Config->ReadNum( wxT( "RadVisiblePanels" ), guPANEL_RADIO_VISIBLE_DEFAULT, wxT( "Positions" ) );


	wxBoxSizer * SearchSizer;
	SearchSizer = new wxBoxSizer( wxHORIZONTAL );
    wxPanel * SearchPanel;
	SearchPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

//    wxStaticText *      SearchStaticText;
//	SearchStaticText = new wxStaticText( SearchPanel, wxID_ANY, _( "Search:" ), wxDefaultPosition, wxDefaultSize, 0 );
//	SearchStaticText->Wrap( -1 );
//	SearchSizer->Add( SearchStaticText, 0, wxALIGN_CENTER|wxALL, 5 );

    m_InputTextCtrl = new wxSearchCtrl( SearchPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    SearchSizer->Add( m_InputTextCtrl, 1, wxALIGN_CENTER, 5 );

    SearchPanel->SetSizer( SearchSizer );
    SearchPanel->Layout();
	SearchSizer->Fit( SearchPanel );

    m_AuiManager.AddPane( SearchPanel,
            wxAuiPaneInfo().Name( wxT( "RadioTextSearch" ) ).Caption( _( "Text Search" ) ).
            MinSize( 60, 28 ).MaxSize( -1, 28 ).Row( 0 ).Layer( 2 ).Position( 0 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );



    wxPanel * GenrePanel;
	GenrePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * GenreSizer;
	GenreSizer = new wxBoxSizer( wxVERTICAL );


    m_GenresTreeCtrl = new guRadioGenreTreeCtrl( GenrePanel, m_Db );
	GenreSizer->Add( m_GenresTreeCtrl, 1, wxEXPAND, 5 );

	GenrePanel->SetSizer( GenreSizer );
	GenrePanel->Layout();
	GenreSizer->Fit( GenrePanel );

	m_AuiManager.AddPane( GenrePanel,
            wxAuiPaneInfo().Name( wxT( "RadioGenres" ) ).Caption( _( "Genre" ) ).
            MinSize( 60, 60 ).Layer( 1 ).Row( 0 ).Position( 0 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Left() );

    wxPanel * LabelsPanel;
	LabelsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * LabelsSizer;
	LabelsSizer = new wxBoxSizer( wxVERTICAL );

	m_LabelsListBox = new guRadioLabelListBox( LabelsPanel, m_Db, _( "Labels" ) );
	LabelsSizer->Add( m_LabelsListBox, 1, wxEXPAND, 5 );

	LabelsPanel->SetSizer( LabelsSizer );
	LabelsPanel->Layout();
	LabelsSizer->Fit( LabelsPanel );

	m_AuiManager.AddPane( LabelsPanel,
            wxAuiPaneInfo().Name( wxT( "RadioLabels" ) ).Caption( _( "Labels" ) ).
            MinSize( 60, 60 ).Layer( 1 ).Row( 0 ).Position( 1 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Left() );



    wxPanel * StationsPanel;
	StationsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxBoxSizer * StationsSizer;
	StationsSizer = new wxBoxSizer( wxVERTICAL );

	m_StationsListBox = new guRadioStationListBox( StationsPanel, m_Db );

	StationsSizer->Add( m_StationsListBox, 1, wxEXPAND, 5 );

	StationsPanel->SetSizer( StationsSizer );
	StationsPanel->Layout();
	StationsSizer->Fit( StationsPanel );

    m_AuiManager.AddPane( StationsPanel, wxAuiPaneInfo().Name( wxT( "RadioStations" ) ).Caption( _( "Stations" ) ).
            MinSize( 50, 50 ).
            CenterPane() );



    wxString RadioLayout = Config->ReadStr( wxT( "Radio" ), wxEmptyString, wxT( "Positions" ) );
    if( Config->GetIgnoreLayouts() || RadioLayout.IsEmpty() )
    {
        m_AuiManager.Update();
        m_VisiblePanels = guPANEL_RADIO_VISIBLE_DEFAULT;
    }
    else
    {
        m_AuiManager.LoadPerspective( RadioLayout, true );
    }


	Connect( guRADIO_TIMER_TEXTSEARCH, wxEVT_TIMER, wxTimerEventHandler( guRadioPanel::OnTextChangedTimer ), NULL, this );

    Connect( wxEVT_COMMAND_TREE_SEL_CHANGED,  wxTreeEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );

    m_GenresTreeCtrl->Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( guRadioPanel::OnRadioGenreListActivated ), NULL, this );

    m_LabelsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioLabelListSelected ), NULL, this );

    //
    Connect( ID_RADIO_USER_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserAdd ), NULL, this );
    Connect( ID_RADIO_USER_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserEdit ), NULL, this );
    Connect( ID_RADIO_USER_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserDel ), NULL, this );

    Connect( ID_RADIO_SEARCH_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioSearchAdd ), NULL, this );
    Connect( ID_RADIO_SEARCH_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioSearchEdit ), NULL, this );
    Connect( ID_RADIO_SEARCH_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioSearchDel ), NULL, this );

    Connect( ID_RADIO_USER_EXPORT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserExport ), NULL, this );
    Connect( ID_RADIO_USER_IMPORT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserImport ), NULL, this );

    Connect( ID_RADIO_DOUPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdate ), NULL, this );
    Connect( ID_RADIO_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdated ), NULL, this );
    Connect( ID_RADIO_UPDATE_END, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdateEnd ), NULL, this );

    m_StationsListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guRadioPanel::OnStationListActivated ), NULL, this );
	m_StationsListBox->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guRadioPanel::OnStationListBoxColClick ), NULL, this );
    Connect( ID_RADIO_EDIT_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnStationsEditLabelsClicked ) );

    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchSelected ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    //m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );

    Connect( ID_RADIO_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsPlay ), NULL, this );
    Connect( ID_RADIO_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueue ), NULL, this );
    Connect( ID_RADIO_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueueAsNext ), NULL, this );

    Connect( ID_RADIO_PLAYLIST_LOADED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnStationPlayListLoaded ), NULL, this );

    m_AuiManager.Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guRadioPanel::OnPaneClose ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guRadioPanel::~guRadioPanel()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteNum( wxT( "RadVisiblePanels" ), m_VisiblePanels, wxT( "Positions" ) );
        Config->WriteStr( wxT( "Radio" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
    }

    m_RadioPlayListLoadThreadMutex.Lock();
    if( m_RadioPlayListLoadThread )
    {
        m_RadioPlayListLoadThread->Pause();
        m_RadioPlayListLoadThread->Delete();
    }
    m_RadioPlayListLoadThreadMutex.Unlock();

    //
	Disconnect( guRADIO_TIMER_TEXTSEARCH, wxEVT_TIMER, wxTimerEventHandler( guRadioPanel::OnTextChangedTimer ), NULL, this );

    Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED,  wxTreeEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );

    m_GenresTreeCtrl->Disconnect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( guRadioPanel::OnRadioGenreListActivated ), NULL, this );

    m_LabelsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioLabelListSelected ), NULL, this );

    //
    Disconnect( ID_RADIO_USER_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserAdd ), NULL, this );
    Disconnect( ID_RADIO_USER_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserEdit ), NULL, this );
    Disconnect( ID_RADIO_USER_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserDel ), NULL, this );

    Disconnect( ID_RADIO_SEARCH_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioSearchAdd ), NULL, this );
    Disconnect( ID_RADIO_SEARCH_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioSearchEdit ), NULL, this );
    Disconnect( ID_RADIO_SEARCH_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioSearchDel ), NULL, this );

    Disconnect( ID_RADIO_USER_EXPORT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserExport ), NULL, this );
    Disconnect( ID_RADIO_USER_IMPORT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserImport ), NULL, this );

    Disconnect( ID_RADIO_DOUPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdate ), NULL, this );
    Disconnect( ID_RADIO_UPDATED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdated ), NULL, this );
    Disconnect( ID_RADIO_UPDATE_END, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUpdateEnd ), NULL, this );

    m_StationsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guRadioPanel::OnStationListActivated ), NULL, this );
	m_StationsListBox->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guRadioPanel::OnStationListBoxColClick ), NULL, this );
    Disconnect( ID_RADIO_EDIT_LABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnStationsEditLabelsClicked ) );

    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchSelected ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    //m_InputTextCtrl->Disconnect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );

    Disconnect( ID_RADIO_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsPlay ), NULL, this );
    Disconnect( ID_RADIO_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueue ), NULL, this );
    Disconnect( ID_RADIO_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueueAsNext ), NULL, this );

    Disconnect( ID_RADIO_PLAYLIST_LOADED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnStationPlayListLoaded ), NULL, this );

    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guRadioPanel::OnPaneClose ), NULL, this );

    m_AuiManager.UnInit();

    if( m_Db )
    {
        delete m_Db;
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchActivated( wxCommandEvent& event )
{
    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();
    m_TextChangedTimer.Start( guRADIO_TIMER_TEXTSEARCH_VALUE, wxTIMER_ONE_SHOT );
//    wxArrayString Words = guSplitWords( m_InputTextCtrl->GetLineText( 0 ) );
//    m_Db->SetRaTeFilters( Words );
//    m_LabelsListBox->ReloadItems();
//    m_GenresTreeCtrl->ReloadItems();
//    m_StationsListBox->ReloadItems();
//    m_InputTextCtrl->ShowCancelButton( true );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchSelected( wxCommandEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    OnSelectStations( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchCancelled( wxCommandEvent &event ) // CLEAN SEARCH STR
{
//    wxArrayString Words;
    m_InputTextCtrl->Clear();
//    m_Db->SetRaTeFilters( Words );
//    m_LabelsListBox->ReloadItems();
//    m_GenresTreeCtrl->ReloadItems();
//    m_StationsListBox->ReloadItems();
//    m_InputTextCtrl->ShowCancelButton( false );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnTextChangedTimer( wxTimerEvent &event )
{
    wxString SearchString = m_InputTextCtrl->GetLineText( 0 );
    //guLogMessage( wxT( "Should do the search now: '%s'" ), SearchString.c_str() );
    if( !SearchString.IsEmpty() )
    {
        if( SearchString.Length() > 1 )
        {
            wxArrayString Words = guSplitWords( SearchString );
            m_Db->SetRaTeFilters( Words );
            m_LabelsListBox->ReloadItems();
            m_GenresTreeCtrl->ReloadItems();
            m_StationsListBox->ReloadItems();
        }

        m_InputTextCtrl->ShowCancelButton( true );
    }
    else
    {
        wxArrayString Words;
        m_Db->SetRaTeFilters( Words );
        m_LabelsListBox->ReloadItems();
        m_GenresTreeCtrl->ReloadItems();
        m_StationsListBox->ReloadItems();
        m_InputTextCtrl->ShowCancelButton( false );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListBoxColClick( wxListEvent &event )
{
    int ColId = m_StationsListBox->GetColumnId( event.m_col );
    m_StationsListBox->SetStationsOrder( ColId );
}


// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationsEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Stations;
    m_StationsListBox->GetSelectedItems( &Stations );
    if( Stations.Count() )
    {
        guArrayListItems LabelSets = m_Db->GetStationsLabels( m_StationsListBox->GetSelectedItems() );

        guLabelEditor * LabelEditor = new guLabelEditor( this, ( guDbLibrary * ) m_Db, _( "Stations Labels Editor" ), true, &Stations, &LabelSets );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                // Update the labels in the files
                m_Db->SetRadioStationsLabels( LabelSets );
            }
            LabelEditor->Destroy();

            m_LabelsListBox->ReloadItems( false );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListActivated( wxListEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    OnSelectStations( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioGenreListSelected( wxTreeEvent &event )
{
    wxTreeItemId ItemId = event.GetItem();

    guShoutcastItemData * ItemData = ( guShoutcastItemData * ) m_GenresTreeCtrl->GetItemData( ItemId );
    if( ItemData )
    {
        //
        m_Db->SetRadioSourceFilter( ItemData->GetSource() );

        wxArrayInt RadioGenres;
        RadioGenres.Add( ItemData->GetId() );
        m_Db->SetRadioGenresFilters( RadioGenres );
    }
    else if( ItemId == * m_GenresTreeCtrl->GetShoutcastGenreId() )
    {
        m_Db->SetRadioSourceFilter( guRADIO_SOURCE_GENRE );
    }
    else if( ItemId == * m_GenresTreeCtrl->GetShoutcastSearchId() )
    {
        m_Db->SetRadioSourceFilter( guRADIO_SOURCE_SEARCH );
    }
    else if( ItemId == * m_GenresTreeCtrl->GetManualId() )
    {
        m_Db->SetRadioSourceFilter( guRADIO_SOURCE_USER );
    }
    else
    {
        m_Db->SetRadioSourceFilter( guRADIO_SOURCE_GENRE );
    }
    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioGenreListActivated( wxTreeEvent &event )
{
    wxTreeItemId ItemId = event.GetItem();
    guShoutcastItemData * ItemData = ( guShoutcastItemData * ) m_GenresTreeCtrl->GetItemData( ItemId );
    if( ItemData )
    {
        wxArrayInt RadioGenres;
        RadioGenres.Add( ItemData->GetId() );

        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
        int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge( _( "Radios" ) );
        guUpdateRadiosThread * UpdateRadiosThread = new guUpdateRadiosThread( m_Db, this, RadioGenres, ItemData->GetSource(), GaugeId );
        if( !UpdateRadiosThread )
        {
            guLogError( wxT( "Could not create the radio update thread" ) );
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioLabelListSelected( wxListEvent &Event )
{
    m_Db->SetRadioLabelsFilters( m_LabelsListBox->GetSelectedItems() );
    m_GenresTreeCtrl->ReloadItems();
    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdate( wxCommandEvent &event )
{
    wxTreeItemId            ItemId;
    wxArrayInt              GenresIds;
    guShoutcastItemData *   ItemData;
    int Source = guRADIO_SOURCE_GENRE;

    wxSetCursor( * wxHOURGLASS_CURSOR );

    ItemId = m_GenresTreeCtrl->GetSelection();
    if( ItemId.IsOk() )
    {
        ItemData = ( guShoutcastItemData * ) m_GenresTreeCtrl->GetItemData( ItemId );
        if( ItemData )
        {
            Source = ItemData->GetSource();
            GenresIds.Add( ItemData->GetId() );
        }
        else if( ItemId == * m_GenresTreeCtrl->GetShoutcastSearchId() )
        {
            Source = guRADIO_SOURCE_SEARCH;
            guListItems Genres;
            m_Db->GetRadioGenres( guRADIO_SOURCE_SEARCH, &Genres );
            int index;
            int count = Genres.Count();
            for( index = 0; index < count; index++ )
            {
                GenresIds.Add( Genres[ index ].m_Id );
            }
        }
    }

    if( !GenresIds.Count() )
    {
        guListItems Genres;
        m_Db->GetRadioGenres( guRADIO_SOURCE_GENRE, &Genres );
        int index;
        int count = Genres.Count();
        for( index = 0; index < count; index++ )
        {
            GenresIds.Add( Genres[ index ].m_Id );
        }
    }

    if( GenresIds.Count() )
    {
        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
        int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge( _( "Radios" ) );
        guUpdateRadiosThread * UpdateRadiosThread = new guUpdateRadiosThread( m_Db, this, GenresIds, Source, GaugeId );
        if( !UpdateRadiosThread )
        {
            guLogError( wxT( "Could not create the radio update thread" ) );
        }
    }
    else
        wxSetCursor( wxNullCursor );
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
    wxSetCursor( wxNullCursor );
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
void guRadioPanel::OnRadioStationsEnqueueAsNext( wxCommandEvent &event )
{
    OnSelectStations( true, true );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSelectStations( bool enqueue, const bool asnext )
{
    wxString StationUrl;
    guRadioStation RadioStation;

    if( m_StationsListBox->GetSelected( &RadioStation ) )
    {
        if( RadioStation.m_SCId != wxNOT_FOUND )
        {
            guShoutCast ShoutCast;
            StationUrl = ShoutCast.GetStationUrl( RadioStation.m_SCId );
        }
        else
        {
            StationUrl = RadioStation.m_Link;
        }

        if( !StationUrl.IsEmpty() )
        {
            LoadStationUrl( StationUrl, enqueue, asnext );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::LoadStationUrl( const wxString &stationurl, const bool enqueue, const bool asnext )
{
    m_RadioPlayListLoadThreadMutex.Lock();
    if( m_RadioPlayListLoadThread )
    {
        m_RadioPlayListLoadThread->Pause();
        m_RadioPlayListLoadThread->Delete();
    }

    m_StationPlayListTracks.Empty();
    m_RadioPlayListLoadThread = new guRadioPlayListLoadThread( this, stationurl, &m_StationPlayListTracks, enqueue, asnext );
    if( !m_RadioPlayListLoadThread )
    {
        guLogError( wxT( "Could not create the download radio playlist thread" ) );
    }
    m_RadioPlayListLoadThreadMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationPlayListLoaded( wxCommandEvent &event )
{
    bool Enqueue = event.GetInt();
    bool AsNext = event.GetExtraLong();

    if( m_StationPlayListTracks.Count() )
    {
        if( Enqueue )
        {
            m_PlayerPanel->AddToPlayList( m_StationPlayListTracks, true, AsNext );
        }
        else
        {
            m_PlayerPanel->SetPlayList( m_StationPlayListTracks );
        }
    }
    else
    {
        wxMessageBox( _( "There are not entries for this Radio Station" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUserAdd( wxCommandEvent &event )
{
    guRadioEditor * RadioEditor = new guRadioEditor( this, _( "Edit Radio" ) );
    if( RadioEditor )
    {
        if( RadioEditor->ShowModal() == wxID_OK )
        {
            guRadioStation RadioStation;
            RadioStation.m_Id = wxNOT_FOUND;
            RadioStation.m_SCId = wxNOT_FOUND;
            RadioStation.m_BitRate = 0;
            RadioStation.m_GenreId = wxNOT_FOUND;
            RadioStation.m_Source = 1;
            RadioStation.m_Name = RadioEditor->GetName();
            RadioStation.m_Link = RadioEditor->GetLink();
            RadioStation.m_Listeners = 0;
            RadioStation.m_Type = wxEmptyString;
            m_Db->SetRadioStation( &RadioStation );
            //
            m_StationsListBox->ReloadItems();
        }
        RadioEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUserEdit( wxCommandEvent &event )
{
    guRadioStation RadioStation;
    m_StationsListBox->GetSelected( &RadioStation );

    guRadioEditor * RadioEditor = new guRadioEditor( this, _( "Edit Radio" ), RadioStation.m_Name, RadioStation.m_Link );
    if( RadioEditor )
    {
        if( RadioEditor->ShowModal() == wxID_OK )
        {
            RadioStation.m_Name = RadioEditor->GetName();
            RadioStation.m_Link = RadioEditor->GetLink();
            m_Db->SetRadioStation( &RadioStation );
            //
            m_StationsListBox->ReloadItems();
        }
        RadioEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUserDel( wxCommandEvent &event )
{
    guRadioStation RadioStation;
    m_StationsListBox->GetSelected( &RadioStation );
    m_Db->DelRadioStation( RadioStation.m_Id );
    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioSearchAdd( wxCommandEvent &event )
{
    guShoutcastItemData ShoutcastItem( 0, guRADIO_SOURCE_SEARCH, wxEmptyString, guRADIO_SEARCH_FLAG_DEFAULT );
    guShoutcastSearch * ShoutcastSearch = new guShoutcastSearch( this, &ShoutcastItem );
    if( ShoutcastSearch )
    {
        if( ShoutcastSearch->ShowModal() == wxID_OK )
        {
            int RadioSearchId = m_Db->AddRadioGenre( ShoutcastItem.GetName(), guRADIO_SOURCE_SEARCH, ShoutcastItem.GetFlags() );
            m_GenresTreeCtrl->ReloadItems();
            m_GenresTreeCtrl->Expand( m_GenresTreeCtrl->GetShoutcastSearchId() );
            wxTreeItemId ItemId = m_GenresTreeCtrl->GetItemId( m_GenresTreeCtrl->GetShoutcastSearchId(), RadioSearchId );
            if( ItemId.IsOk() )
            {
                m_GenresTreeCtrl->SelectItem( ItemId );
                OnRadioUpdate( event );
            }
        }
        ShoutcastSearch->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioSearchEdit( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_GenresTreeCtrl->GetSelection();
    guShoutcastItemData * ItemData = ( guShoutcastItemData * ) m_GenresTreeCtrl->GetItemData( ItemId );
    if( ItemData && ItemData->GetSource() == guRADIO_SOURCE_SEARCH )
    {
        guShoutcastSearch * ShoutcastSearch = new guShoutcastSearch( this, ItemData );
        if( ShoutcastSearch )
        {
            if( ShoutcastSearch->ShowModal() == wxID_OK )
            {
                m_Db->SetRadioGenre( ItemData->GetId(), ItemData->GetName(), guRADIO_SOURCE_SEARCH, ItemData->GetFlags() );
                m_GenresTreeCtrl->ReloadItems();
            }
            ShoutcastSearch->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioSearchDel( wxCommandEvent &event )
{
    wxArrayTreeItemIds ItemIds;
    int index;
    int count;

    if( ( count = m_GenresTreeCtrl->GetSelections( ItemIds ) ) )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected radio searches?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            guShoutcastItemData * RadioGenreData;
            for( index = 0; index < count; index++ )
            {
                RadioGenreData = ( guShoutcastItemData * ) m_GenresTreeCtrl->GetItemData( ItemIds[ index ] );
                if( RadioGenreData )
                {
                    m_Db->DelRadioGenre( RadioGenreData->GetId() );
                }
            }
            m_GenresTreeCtrl->ReloadItems();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUserExport( wxCommandEvent &event )
{
    guRadioStations UserStations;
    m_Db->GetUserRadioStations( &UserStations );
    int Index;
    int Count;
    if( ( Count = UserStations.Count() ) )
    {

        wxFileDialog * FileDialog = new wxFileDialog( this,
            wxT( "Select the output xml filename" ),
            wxGetHomeDir(),
            wxT( "RadioStations.xml" ),
            wxT( "*.xml;*.xml" ),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

        if( FileDialog )
        {
            if( FileDialog->ShowModal() == wxID_OK )
            {
                wxXmlDocument OutXml;
                //OutXml.SetRoot(  );
                wxXmlNode * RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "RadioStations" ) );

                for( Index = 0; Index < Count; Index++ )
                {
                    //guLogMessage( wxT( "Adding %s" ), UserStations[ Index ].m_Name.c_str() );
                    wxXmlNode * RadioNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "RadioStation" ) );

                    wxXmlNode * RadioName = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "Name" ) );
                    wxXmlNode * RadioNameVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "Name" ), UserStations[ Index ].m_Name );
                    RadioName->AddChild( RadioNameVal );
                    RadioNode->AddChild( RadioName );

                    wxXmlNode * RadioUrl = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "Url" ) );
                    wxXmlNode * RadioUrlVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "Url" ), UserStations[ Index ].m_Link );
                    RadioUrl->AddChild( RadioUrlVal );
                    RadioNode->AddChild( RadioUrl );

                    RootNode->AddChild( RadioNode );
                }
                OutXml.SetRoot( RootNode );
                OutXml.Save( FileDialog->GetPath() );
            }
            FileDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlRadioStation( wxXmlNode * node, guRadioStation * station )
{
    while( node )
    {
        if( node->GetName() == wxT( "Name" ) )
        {
            station->m_Name = node->GetNodeContent();
        }
        else if( node->GetName() == wxT( "Url" ) )
        {
            station->m_Link = node->GetNodeContent();
        }
        node = node->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlRadioStations( wxXmlNode * node, guRadioStations * stations )
{
    while( node && node->GetName() == wxT( "RadioStation" ) )
    {
        guRadioStation * RadioStation = new guRadioStation();

        RadioStation->m_Id = wxNOT_FOUND;
        RadioStation->m_SCId = wxNOT_FOUND;
        RadioStation->m_BitRate = 0;
        RadioStation->m_GenreId = wxNOT_FOUND;
        RadioStation->m_Source = 1;
        RadioStation->m_Listeners = 0;
        RadioStation->m_Type = wxEmptyString;

        ReadXmlRadioStation( node->GetChildren(), RadioStation );

        if( !RadioStation->m_Name.IsEmpty() && !RadioStation->m_Link.IsEmpty() )
            stations->Add( RadioStation );
        else
            delete RadioStation;

        node = node->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUserImport( wxCommandEvent &event )
{
    guRadioStations UserStations;

    wxFileDialog * FileDialog = new wxFileDialog( this,
        wxT( "Select the xml file" ),
        wxGetHomeDir(),
        wxEmptyString,
        wxT( "*.xml;*.xml" ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            wxFileInputStream Ins( FileDialog->GetPath() );
            wxXmlDocument XmlDoc( Ins );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName() == wxT( "RadioStations" ) )
            {
                ReadXmlRadioStations( XmlNode->GetChildren(), &UserStations );
                int Index;
                int Count;
                if( ( Count = UserStations.Count() ) )
                {
                    for( Index = 0; Index < Count; Index++ )
                    {
                        m_Db->SetRadioStation( &UserStations[ Index ] );
                    }
                    //
                    m_StationsListBox->ReloadItems();
                }
            }
        }
        FileDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
bool guRadioPanel::IsPanelShown( const int panelid ) const
{
    return ( m_VisiblePanels & panelid );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::ShowPanel( const int panelid, bool show )
{
    wxString PaneName;

    switch( panelid )
    {
        case guPANEL_RADIO_TEXTSEARCH :
            PaneName = wxT( "RadioTextSearch" );
            break;

        case guPANEL_RADIO_GENRES :
            PaneName = wxT( "RadioGenres" );
            break;

        case guPANEL_RADIO_LABELS :
            PaneName = wxT( "RadioLabels" );
            break;

////        case guPANEL_RADIO_STATIONS:
////            PaneName = wxT( "RadioStations" );
////            break;

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
void guRadioPanel::OnPaneClose( wxAuiManagerEvent &event )
{
    wxAuiPaneInfo * PaneInfo = event.GetPane();
    wxString PaneName = PaneInfo->name;
    int CmdId = 0;

    if( PaneName == wxT( "RadioTextSearch" ) )
    {
        CmdId = ID_MENU_VIEW_RAD_TEXTSEARCH;
    }
    else if( PaneName == wxT( "RadioLabels" ) )
    {
        CmdId = ID_MENU_VIEW_RAD_LABELS;
    }
    else if( PaneName == wxT( "RadioGenres" ) )
    {
        CmdId = ID_MENU_VIEW_RAD_GENRES;
    }
//    else if( PaneName == wxT( "RadioStations" ) )
//    {
//        CmdId = ID_MENU_VIEW_RAD_STATIONS;
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
void guRadioPanel::GetRadioCounter( wxLongLong * count )
{
    * count = m_StationsListBox->GetItemCount();
}



// -------------------------------------------------------------------------------- //
guRadioPlayListLoadThread::guRadioPlayListLoadThread( guRadioPanel * radiopanel,
        const wxString &stationurl, guTrackArray * tracks, const bool enqueue, const bool asnext )
{
    m_RadioPanel = radiopanel;
    m_StationUrl = stationurl;
    m_Tracks = tracks;
    m_Enqueue = enqueue;
    m_AsNext = asnext;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}


// -------------------------------------------------------------------------------- //
guRadioPlayListLoadThread::~guRadioPlayListLoadThread()
{
    if( !TestDestroy() )
    {
        m_RadioPanel->EndStationPlayListLoaded();
    }
}

// -------------------------------------------------------------------------------- //
guRadioPlayListLoadThread::ExitCode guRadioPlayListLoadThread::Entry()
{
    guTrack * NewSong;
    guPlayListFile PlayListFile;

    if( TestDestroy() )
        return 0;

    PlayListFile.Load( m_StationUrl );

    int Index;
    int Count;
    Count = PlayListFile.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( TestDestroy() )
            break;

        NewSong = new guTrack();
        if( NewSong )
        {
            guStationPlayListItem PlayListItem = PlayListFile.GetItem( Index );
            NewSong->m_Type = guTRACK_TYPE_RADIOSTATION;
            NewSong->m_FileName = PlayListItem.m_Location;
            //NewSong->m_SongName = PlayList[ index ].m_Name;
            NewSong->m_SongName = PlayListItem.m_Name;
            //NewSong->m_AlbumName = RadioStation.m_Name;
            NewSong->m_Length = 0;
            NewSong->m_Rating = -1;
            NewSong->m_Bitrate = 0;
            //NewSong->CoverId = guPLAYLIST_RADIOSTATION;
            NewSong->m_CoverId = 0;
            NewSong->m_Year = 0;
            m_Tracks->Add( NewSong );
        }
    }


    if( !TestDestroy() )
    {
        //m_RadioPanel->StationUrlLoaded( Tracks, m_Enqueue, m_AsNext );
        //guLogMessage( wxT( "Send Event for station '%s'" ), m_StationUrl.c_str() );
        wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_PLAYLIST_LOADED );
        Event.SetInt( m_Enqueue );
        Event.SetExtraLong( m_AsNext );
        wxPostEvent( m_RadioPanel, Event );
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
// guUpdateRadiosThread
// -------------------------------------------------------------------------------- //
bool inline guListItemsSearchName( guListItems &items, const wxString &name )
{
    int Index;
    int Count = items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( items[ Index ].m_Name.Lower() == name )
        {
            return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guUpdateRadiosThread::guUpdateRadiosThread( guDbRadios * db, guRadioPanel * radiopanel,
                                const wxArrayInt &ids, const int source, int gaugeid )
{
    m_Db = db;
    m_RadioPanel = radiopanel;
    m_Ids = ids;
    m_Source = source;
    m_GaugeId = gaugeid;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}


// -------------------------------------------------------------------------------- //
void guUpdateRadiosThread::CheckRadioStationsFilters( const int flags, const wxString &text, guRadioStations &stations )
{
    guListItems RadioGenres;
    m_Db->GetRadioGenres( guRADIO_SOURCE_GENRE, &RadioGenres );

    int Index;
    int Count = stations.Count();
    if( Count )
    {
        for( Index = Count - 1; Index >= 0; Index-- )
        {
            if( ( flags & guRADIO_SEARCH_FLAG_NOWPLAYING ) )
            {
                if( stations[ Index ].m_NowPlaying.Lower().Find( text ) == wxNOT_FOUND )
                {
                    stations.RemoveAt( Index );
                    continue;
                }
            }

            if( ( flags & guRADIO_SEARCH_FLAG_GENRE ) )
            {
                if( stations[ Index ].m_GenreName.Lower().Find( text ) == wxNOT_FOUND )
                {
                    stations.RemoveAt( Index );
                    continue;
                }
            }

            if( ( flags & guRADIO_SEARCH_FLAG_STATION ) )
            {
                if( stations[ Index ].m_Name.Lower().Find( text ) == wxNOT_FOUND )
                {
                    stations.RemoveAt( Index );
                    continue;
                }
            }

            // Need to check if the station genre already existed
            if( !( flags & guRADIO_SEARCH_FLAG_ALLGENRES ) )
            {
                if( !guListItemsSearchName( RadioGenres, stations[ Index ].m_GenreName.Lower() ) )
                {
                    stations.RemoveAt( Index );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
guUpdateRadiosThread::ExitCode guUpdateRadiosThread::Entry()
{
//    guListItems Genres;
    int index;
    int count;
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
    guShoutCast * ShoutCast = new guShoutCast();
    guRadioStations RadioStations;
    if( ShoutCast )
    {
        //
        guConfig * Config = ( guConfig * ) guConfig::Get();
        long MinBitRate;
        Config->ReadStr( wxT( "RadioMinBitRate" ), wxT( "128" ), wxT( "Radios" ) ).ToLong( &MinBitRate );

//        m_Db->GetRadioGenres( &Genres, false );
//        guLogMessage( wxT ( "Loaded the genres" ) );
        guListItems Genres;
        wxArrayInt  Flags;
        m_Db->GetRadioGenresList( m_Source, m_Ids, &Genres, &Flags );

        //
        m_Db->DelRadioStations( m_Source, m_Ids );
        //guLogMessage( wxT( "Deleted all radio stations" ) );
        count = Genres.Count();

        event.SetInt( m_GaugeId );
        event.SetExtraLong( count );
        wxPostEvent( wxTheApp->GetTopWindow(), event );

        for( index = 0; index < count; index++ )
        {
            guLogMessage( wxT( "Updating radiostations for genre '%s'" ), Genres[ index ].m_Name.c_str() );
            ShoutCast->GetStations( m_Source, Flags[ index ], Genres[ index ].m_Name, Genres[ index ].m_Id, &RadioStations, MinBitRate );

            if( m_Source == guRADIO_SOURCE_SEARCH )
            {
                CheckRadioStationsFilters( Flags[ index ], Genres[ index ].m_Name.Lower(), RadioStations );
            }

            m_Db->SetRadioStations( &RadioStations );

            RadioStations.Clear();

            //wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_UPDATED );
            event.SetId( ID_RADIO_UPDATED );
            wxPostEvent( m_RadioPanel, event );
            Sleep( 30 ); // Its wxThread::Sleep

//            wxCommandEvent event2( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_UPDATE );
            event.SetId( ID_STATUSBAR_GAUGE_UPDATE );
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

//    wxCommandEvent event2( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetId( ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
    //
    return 0;
}

// -------------------------------------------------------------------------------- //
