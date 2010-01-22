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
#include "RadioEditor.h"
#include "StatusBar.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/wfstream.h>
#include <wx/treectrl.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

// -------------------------------------------------------------------------------- //
// guRadioGenreTreeCtrl
// -------------------------------------------------------------------------------- //
class guRadioGenreData : public wxTreeItemData
{
  private :
    int         m_Id;
    wxString    m_Name;
  public :
    guRadioGenreData( const int id, const wxString &name ) { m_Id = id; m_Name = name; };
    int         GetId( void ) { return m_Id; };
    void        SetId( int id ) { m_Id = id; };
    wxString    GetName( void ) { return m_Name; };
    void        SetName( const wxString &name ) { m_Name = name; };
};

// -------------------------------------------------------------------------------- //
// guRadioGenreTreeCtrl
// -------------------------------------------------------------------------------- //
class guRadioGenreTreeCtrl : public wxTreeCtrl
{
  private :
    guDbLibrary *   m_Db;
    wxImageList *   m_ImageList;
    wxTreeItemId    m_RootId;
    wxTreeItemId    m_ManualId;
    wxTreeItemId    m_ShoutcastId;

    void            OnContextMenu( wxTreeEvent &event );
    void            OnRadioGenreAdd( wxCommandEvent &event );
    void            OnRadioGenreEdit( wxCommandEvent &event );
    void            OnRadioGenreDelete( wxCommandEvent &event );

  public :
    guRadioGenreTreeCtrl( wxWindow * parent, guDbLibrary * db );
    ~guRadioGenreTreeCtrl();

    void            ReloadItems( void );
    wxTreeItemId *  GetShoutcastId( void ) { return &m_ShoutcastId; };
    wxTreeItemId *  GetManualId( void ) { return &m_ManualId; };

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

      guRadioLabelListBox( wxWindow * parent, guDbLibrary * NewDb, wxString Label );
      ~guRadioLabelListBox();

};

// -------------------------------------------------------------------------------- //
// guUpdateRadiosThread
// -------------------------------------------------------------------------------- //
class guUpdateRadiosThread : public wxThread
{
  private:
    guDbLibrary *     m_Db;
    guRadioPanel *  m_RadioPanel;
    int             m_GaugeId;
    wxArrayInt      m_GenresIds;

  public:
    guUpdateRadiosThread( guDbLibrary * db, guRadioPanel * radiopanel,
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
    guDbLibrary *       m_Db;
    guRadioStations   m_Radios;
    int               m_StationsOrder;
    bool              m_StationsOrderDesc;

    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

  public :
    guRadioStationListBox( wxWindow * parent, guDbLibrary * NewDb );
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
guRadioGenreTreeCtrl::guRadioGenreTreeCtrl( wxWindow * parent, guDbLibrary * db ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT|wxTR_MULTIPLE|wxSUNKEN_BORDER )
{
    m_Db = db;
    m_ImageList = new wxImageList();
    m_ImageList->Add( guImage( guIMAGE_INDEX_tiny_shoutcast ) );
    m_ImageList->Add( wxBitmap( guImage( guIMAGE_INDEX_tiny_net_radio ) ) );

    AssignImageList( m_ImageList );

    m_RootId   = AddRoot( wxT( "Radios" ), -1, -1, NULL );
    m_ShoutcastId = AppendItem( m_RootId, _( "Shoutcast" ), 0, 0, NULL );
    m_ManualId = AppendItem( m_RootId, _( "User Defined" ), 1, 1, NULL );

    SetIndent( 5 );

    Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guRadioGenreTreeCtrl::OnContextMenu ), NULL, this );
    Connect( ID_RADIO_GENRE_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreAdd ) );
    Connect( ID_RADIO_GENRE_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreEdit ) );
    Connect( ID_RADIO_GENRE_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreDelete ) );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guRadioGenreTreeCtrl::~guRadioGenreTreeCtrl()
{
    Disconnect( ID_RADIO_GENRE_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreAdd ) );
    Disconnect( ID_RADIO_GENRE_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreEdit ) );
    Disconnect( ID_RADIO_GENRE_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioGenreTreeCtrl::OnRadioGenreDelete ) );
    Disconnect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guRadioGenreTreeCtrl::OnContextMenu ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::ReloadItems( void )
{
    DeleteChildren( m_ShoutcastId );

    guListItems RadioGenres;
    m_Db->GetRadioGenres( &RadioGenres );

    int index;
    int count = RadioGenres.Count();

    for( index = 0; index < count; index++ )
    {
        AppendItem( m_ShoutcastId, RadioGenres[ index ].m_Name, -1, -1,
            new guRadioGenreData( RadioGenres[ index ].m_Id, RadioGenres[ index ].m_Name ) );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPoint();

    wxTreeItemId ItemId = event.GetItem();
    guRadioGenreData * ItemData = ( guRadioGenreData * ) GetItemData( ItemId );


    if( ItemData || ( ItemId == * GetShoutcastId() ) )
    {

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_GENRE_ADD, _( "Add Genre" ), _( "Create a new genre" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
        Menu.Append( MenuItem );

        if( ItemData )
        {
            MenuItem = new wxMenuItem( &Menu, ID_RADIO_GENRE_EDIT, _( "Edit genre" ), _( "Change selected genre" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_RADIO_GENRE_DELETE, _( "Delete genre" ), _( "Delete selected genre" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
            Menu.Append( MenuItem );
        }

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_DOUPDATE, _( "Update Radio Stations" ), _( "Update the radio station lists" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu.Append( MenuItem );
    }
    else
    {
        MenuItem = new wxMenuItem( &Menu, ID_RADIO_USER_ADD, _( "Add Radio" ), _( "Create a new radio" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_new ) );
        Menu.Append( MenuItem );

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_USER_IMPORT, _( "Import" ), _( "Import the radio stations" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_RADIO_USER_EXPORT, _( "Export" ), _( "Export all the radio stations" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
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
void guRadioGenreTreeCtrl::OnRadioGenreEdit( wxCommandEvent &event )
{
    wxTreeItemId ItemId = GetSelection();

    if( ItemId.IsOk() )
    {
        guRadioGenreData * RadioGenreData = ( guRadioGenreData * ) GetItemData( ItemId );

        if( RadioGenreData )
        {
            // Get the Index of the First Selected Item
            wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Genre Name: " ), _( "Enter the new Genre Name" ), RadioGenreData->GetName() );
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                m_Db->SetRadioGenreName( RadioGenreData->GetId(), EntryDialog->GetValue() );
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
            guRadioGenreData * RadioGenreData;
            for( index = 0; index < count; index++ )
            {
                RadioGenreData = ( guRadioGenreData * ) GetItemData( ItemIds[ index ] );
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
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
guRadioStationListBox::guRadioStationListBox( wxWindow * parent, guDbLibrary * db ) :
    guListView( parent, wxLB_SINGLE )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_StationsOrder = Config->ReadNum( wxT( "StationsOrder" ), 0, wxT( "General" ) );
    m_StationsOrderDesc = Config->ReadNum( wxT( "StationsOrderDesc" ), false, wxT( "General" ) );;

    // Create the Columns
    //int ColId;
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

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SET_RADIOSTATIONS );
    event.SetInt( m_Radios.Count() );
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
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_RADIO_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
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
        if( RadioStation.m_IsUser )
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
bool guRadioStationListBox::GetSelected( guRadioStation * radiostation ) const
{
    int Selected = GetSelection();
    if( Selected != wxNOT_FOUND )
    {
        radiostation->m_Id          = m_Radios[ Selected ].m_Id;
        radiostation->m_SCId        = m_Radios[ Selected ].m_SCId;
        radiostation->m_BitRate     = m_Radios[ Selected ].m_BitRate;
        radiostation->m_GenreId     = m_Radios[ Selected ].m_GenreId;
        radiostation->m_IsUser      = m_Radios[ Selected ].m_IsUser;
        radiostation->m_Link        = m_Radios[ Selected ].m_Link;
        radiostation->m_Listeners   = m_Radios[ Selected ].m_Listeners;
        radiostation->m_Name        = m_Radios[ Selected ].m_Name;
        radiostation->m_Type        = m_Radios[ Selected ].m_Type;
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
// guRadioLabelListBox
// -------------------------------------------------------------------------------- //
guRadioLabelListBox::guRadioLabelListBox( wxWindow * parent, guDbLibrary * NewDb, wxString Label ) : guListBox( parent, NewDb, Label )
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
guRadioPanel::guRadioPanel( wxWindow* parent, guDbLibrary * NewDb, guPlayerPanel * NewPlayerPanel ) :
              wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = NewDb;
    m_PlayerPanel = NewPlayerPanel;

    guConfig *  Config = ( guConfig * ) guConfig::Get();

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* SearchSizer;
	SearchSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SearchStaticText = new wxStaticText( this, wxID_ANY, _("Search:"), wxDefaultPosition, wxDefaultSize, 0 );
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

    wxStaticText * GenreLabel = new wxStaticText( GenrePanel, wxID_ANY, _( "Genres " ), wxDefaultPosition, wxDefaultSize, 0 );
    GenreSizer->Add( GenreLabel, 0, wxALL|wxEXPAND, 5 );

    m_GenresTreeCtrl = new guRadioGenreTreeCtrl( GenrePanel, m_Db );
	//m_GenresListBox->SetBackgroundColour( wxColour( 250, 250, 250 ) );

	GenreSizer->Add( m_GenresTreeCtrl, 1, wxALL|wxEXPAND, 1 );

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

    Connect( wxEVT_COMMAND_TREE_SEL_CHANGED,  wxTreeEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );

    m_LabelsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioLabelListSelected ), NULL, this );

    //
    Connect( ID_RADIO_USER_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserAdd ) );
    Connect( ID_RADIO_USER_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserEdit ) );
    Connect( ID_RADIO_USER_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserDel ) );

    Connect( ID_RADIO_USER_EXPORT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserExport ) );
    Connect( ID_RADIO_USER_IMPORT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserImport ) );

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
    Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED,  wxTreeEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );

    m_LabelsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guRadioPanel::OnRadioLabelListSelected ), NULL, this );
    //
    Disconnect( ID_RADIO_USER_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserAdd ) );
    Disconnect( ID_RADIO_USER_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserEdit ) );
    Disconnect( ID_RADIO_USER_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioUserDel ) );

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
    m_GenresTreeCtrl->ReloadItems();
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
    m_GenresTreeCtrl->ReloadItems();
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
    guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Stations Labels Editor" ), true, Labels, m_Db->GetStationsLabels( Stations ) );
    if( LabelEditor )
    {
        if( LabelEditor->ShowModal() == wxID_OK )
        {
            // Update the labels for the stations
            m_Db->SetRadioStationsLabels( Stations, LabelEditor->GetCheckedIds() );
            m_LabelsListBox->ReloadItems( false );
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
void guRadioPanel::OnRadioGenreListSelected( wxTreeEvent &event )
{
    wxTreeItemId ItemId = event.GetItem();

    guRadioGenreData * ItemData = ( guRadioGenreData * ) m_GenresTreeCtrl->GetItemData( event.GetItem() );
    if( ItemData )
    {
        wxArrayInt RadioGenres;
        RadioGenres.Add( ItemData->GetId() );
        m_Db->SetRadioGenresFilters( RadioGenres );
    }
    else
    {
        //guLogMessage( wxT( "Selecting Radios... Manual? %u" ), ( ItemId == * m_GenresTreeCtrl->GetManualId() ) );
        m_Db->SetRadioIsUserFilter( ( ItemId == * m_GenresTreeCtrl->GetManualId() ) );
    }
    m_StationsListBox->ReloadItems();
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
    wxArrayTreeItemIds ItemIds;
    wxArrayInt         GenresIds;
    guRadioGenreData * ItemData;
    int index;
    int count;

    wxSetCursor( * wxHOURGLASS_CURSOR );

    if( ( count = m_GenresTreeCtrl->GetSelections( ItemIds ) ) )
    {
        for( index = 0; index < count; index++ )
        {
            ItemData = ( guRadioGenreData * ) m_GenresTreeCtrl->GetItemData( ItemIds[ index ] );
            if( ItemData )
            {
                GenresIds.Add( ItemData->GetId() );
            }
        }
    }

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
        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
        int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge( _( "Radios" ) );
        guUpdateRadiosThread * UpdateRadiosThread = new guUpdateRadiosThread( m_Db, this, GenresIds, GaugeId );
        if( UpdateRadiosThread )
        {
            UpdateRadiosThread->Create();
            UpdateRadiosThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
            UpdateRadiosThread->Run();
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
guStationPlayLists GetStationM3uPlayList( const guRadioStation * RadioStation )
{
    guStationPlayLists PlayList;
    wxString M3uFile;
    wxCurlHTTP          http;
    char *              Buffer = NULL;
    //
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: */*" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, RadioStation->m_Link );
    if( Buffer )
    {
        M3uFile = wxString( Buffer, wxConvUTF8 );
        free( Buffer );
        if( !M3uFile.IsEmpty() )
        {
            //guLogMessage( wxT( "Content...\n%s" ), M3uFile.c_str() );
            wxArrayString Lines = wxStringTokenize( M3uFile );

            //if( Lines[ 0 ].Find( wxT( "#EXTM3U" ) ) != wxNOT_FOUND )
            //{
                int index;
                int count = Lines.Count();
                wxString StreamName = wxEmptyString;
                for( index = 0; index < count; index++ )
                {
                    Lines[ index ].Trim( wxString::both );
                    if( Lines[ index ].IsEmpty() || ( Lines[ index ].Find( wxT( "#EXTM3U" ) ) != wxNOT_FOUND ) )
                    {
                        continue;
                    }
                    else if( Lines[ index ].Find( wxT( "#EXTINF" ) ) != wxNOT_FOUND )
                    {
                        if( Lines[ index ].Find( wxT( "," ) ) != wxNOT_FOUND )
                            StreamName = Lines[ index ].AfterLast( wxT( ',' ) );
                    }
                    else
                    {
                        guStationPlayList * StationPlayList = new guStationPlayList();
                        if( StationPlayList )
                        {
                            StationPlayList->m_Name = StreamName.IsEmpty() ? RadioStation->m_Name : StreamName;
                            StationPlayList->m_Url = Lines[ index ];
                            StreamName = wxEmptyString;
                            PlayList.Add( StationPlayList );
                        }
                    }
                }
            //}
            //else
            //{
            //    guLogMessage( wxT( "M3u header tag not found...\n%s" ), M3uFile.c_str() );
            //}
        }
        else
        {
            guLogMessage( wxT( "Empty M3u file %s" ), RadioStation->m_Link.c_str() );
        }
    }
    return PlayList;
}

// -------------------------------------------------------------------------------- //
void ReadAsxEntry( wxXmlNode * XmlNode, guStationPlayLists * playlist, const wxString &title )
{
    wxString StreamLink;
    wxString StreamTitle;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "ref" ) )
        {
            XmlNode->GetPropVal( wxT( "href" ), &StreamLink );
        }
        else if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            StreamTitle = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }

    if( !StreamLink.IsEmpty() )
    {
        guStationPlayList * StationPlayList = new guStationPlayList();
        if( StationPlayList )
        {
            StationPlayList->m_Name = StreamTitle.IsEmpty() ? title : StreamTitle;
            StationPlayList->m_Url = StreamLink;
            playlist->Add( StationPlayList );
        }
    }
}

// -------------------------------------------------------------------------------- //
void ReadAsxPlayList( wxXmlNode * XmlNode, guStationPlayLists * playlist )
{
    wxString Title;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName().Lower() == wxT( "entry" ) )
        {
            ReadAsxEntry( XmlNode->GetChildren(), playlist, Title );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
guStationPlayLists GetStationAsxPlayList( const guRadioStation * RadioStation )
{
    guStationPlayLists PlayList;
    wxCurlHTTP          http;
    char *              Buffer = NULL;
    wxString Content;
    //
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: */*" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, RadioStation->m_Link );
    if( Buffer )
    {
        guLogMessage( wxT( "Got the asx web content...%s" ), RadioStation->m_Link.c_str() );
        Content = wxString( Buffer, wxConvUTF8 );
        if( Content.IsEmpty() )
            Content = wxString( Buffer, wxConvISO8859_1 );
        if( Content.IsEmpty() )
            Content = wxString( Buffer, wxConvLibc );
        free( Buffer );
        guLogMessage( wxT( "ASX:\n%s" ), Content.c_str() );
        if( !Content.IsEmpty() )
        {
            wxStringInputStream InStr( Content );
            wxXmlDocument XmlDoc( InStr );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName().Lower() == wxT( "asx" ) )
            {
                ReadAsxPlayList( XmlNode->GetChildren(), &PlayList );
            }
        }
    }
    return PlayList;
}

// -------------------------------------------------------------------------------- //
void ReadXspfTrack( wxXmlNode * XmlNode, guStationPlayLists * playlist, wxString &title )
{
    wxString StreamLink;
    wxString StreamTitle;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "location" ) )
        {
            StreamLink = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            StreamTitle = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }

    if( !StreamLink.IsEmpty() )
    {
        guStationPlayList * StationPlayList = new guStationPlayList();
        if( StationPlayList )
        {
            StationPlayList->m_Name = StreamTitle.IsEmpty() ? title : StreamTitle;
            StationPlayList->m_Url = StreamLink;
            playlist->Add( StationPlayList );
        }
    }
}

// -------------------------------------------------------------------------------- //
void ReadXspfTrackList( wxXmlNode * XmlNode, guStationPlayLists * playlist, wxString &title )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "track" ) )
        {
            ReadXspfTrack( XmlNode->GetChildren(), playlist, title );
        }
        XmlNode = XmlNode->GetNext();
    }

}

// -------------------------------------------------------------------------------- //
void ReadXspfPlayList( wxXmlNode * XmlNode, guStationPlayLists * playlist )
{
    wxString Title;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName().Lower() == wxT( "tracklist" ) )
        {
            ReadXspfTrackList( XmlNode->GetChildren(), playlist, Title );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
guStationPlayLists GetStationXspfPlayList( const guRadioStation * RadioStation )
{
    guStationPlayLists PlayList;
    wxCurlHTTP          http;
    char *              Buffer = NULL;
    wxString Content;
    //
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: */*" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, RadioStation->m_Link );
    if( Buffer )
    {
        Content = wxString( Buffer, wxConvUTF8 );
        free( Buffer );
        //guLogMessage( wxT( "%s" ), Content.c_str() );
        if( !Content.IsEmpty() )
        {
            wxStringInputStream InStr( Content );
            wxXmlDocument XmlDoc( InStr );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName().Lower() == wxT( "playlist" ) )
            {
                ReadXspfPlayList( XmlNode->GetChildren(), &PlayList );
            }
        }
    }
    return PlayList;
}

// -------------------------------------------------------------------------------- //
guStationPlayLists guRadioPanel::GetPlayList( const guRadioStation * RadioStation )
{
    guShoutCast ShoutCast;
    wxASSERT( RadioStation );

    guStationPlayLists PlayList;
    if( RadioStation->m_SCId != wxNOT_FOUND )
    {
        return ShoutCast.GetStationPlayList( RadioStation->m_SCId );
    }

    // Its not a Shoutcast radio so try to find out the playlist format
//    if( guIsValidAudioFile( RadioStation->m_Link ) )
//    {
//        guStationPlayList * NewStation = new guStationPlayList();
//        if( NewStation )
//        {
//            NewStation->m_Name = RadioStation->m_Name;
//            NewStation->m_Url  = RadioStation->m_Link;
//            PlayList.Add( NewStation );
//        }
//    }
//    else if( RadioStation->m_Link.Lower().EndsWith( wxT( ".pls" ) ) )
    if( RadioStation->m_Link.Lower().EndsWith( wxT( ".pls" ) ) )
    {
        PlayList = ShoutCast.GetStationPlayList( RadioStation->m_Link );
    }
    else if( RadioStation->m_Link.Lower().EndsWith( wxT( ".m3u" ) ) )
    {
        PlayList = GetStationM3uPlayList( RadioStation );
    }
    else if( RadioStation->m_Link.Lower().EndsWith( wxT( ".asx" ) ) )
    {
        PlayList = GetStationAsxPlayList( RadioStation );
    }
    else if( RadioStation->m_Link.Lower().EndsWith( wxT( ".xspf" ) ) )
    {
        PlayList = GetStationXspfPlayList( RadioStation );
    }
    else
    {
        //guLogMessage( wxT( "Dunno how to handle the radio %s" ), RadioStation->m_Link.c_str() );
        guStationPlayList * NewStation = new guStationPlayList();
        if( NewStation )
        {
            NewStation->m_Name = RadioStation->m_Name;
            NewStation->m_Url  = RadioStation->m_Link;
            PlayList.Add( NewStation );
        }
    }
    return PlayList;
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSelectStations( bool enqueue )
{
    guTrackArray   Songs;
    guTrack *  NewSong;
    int index;
    int count;

    guRadioStation RadioStation;
    guStationPlayLists PlayList;

    if( m_StationsListBox->GetSelected( &RadioStation ) )
    {
        PlayList = GetPlayList( &RadioStation );

        if( ( count = PlayList.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                NewSong = new guTrack();
                if( NewSong )
                {
                    NewSong->m_Type = guTRACK_TYPE_RADIOSTATION;
                    NewSong->m_FileName = PlayList[ index ].m_Url;
                    //NewSong->m_SongName = PlayList[ index ].m_Name;
                    NewSong->m_SongName = PlayList[ index ].m_Name.IsEmpty() ?
                                              RadioStation.m_Name : PlayList[ index ].m_Name;
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
void guRadioPanel::OnRadioUserAdd( wxCommandEvent &event )
{
    guRadioEditor * RadioEditor = new guRadioEditor( this );
    if( RadioEditor )
    {
        if( RadioEditor->ShowModal() == wxID_OK )
        {
            guRadioStation RadioStation;
            RadioStation.m_Id = wxNOT_FOUND;
            RadioStation.m_SCId = wxNOT_FOUND;
            RadioStation.m_BitRate = 0;
            RadioStation.m_GenreId = wxNOT_FOUND;
            RadioStation.m_IsUser = true;
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
            wxT( "output.xml" ),
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
                OutXml.Save( FileDialog->GetFilename() );
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
        wxT( "Select the output xml filename" ),
        wxGetHomeDir(),
        wxEmptyString,
        wxT( "*.xml;*.xml" ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            wxFileInputStream Ins( FileDialog->GetFilename() );
            wxXmlDocument XmlDoc( Ins );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName() == wxT( "RadioStations" ) )
            {
                ReadXmlRadioStations( XmlNode->GetChildren(), &UserStations );
                if( UserStations.Count() )
                {
                }
            }
        }
        FileDialog->Destroy();
    }
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
            m_Db->SetRadioStations( &RadioStations );
            RadioStations.Clear();

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
