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



#define guRADIOSTATIONS_COLUMN_NAME         0
#define guRADIOSTATIONS_COLUMN_BITRATE      1
#define guRADIOSTATIONS_COLUMN_LISTENERS    2

#define guRADIOSTATIONS_COLUMN_COUNT        3

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

    virtual wxArrayString       GetColumnNames( void );

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

    wxArrayString ColumnNames = GetColumnNames();
    // Create the Columns
    //int ColId;
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        guListViewColumn * Column = new guListViewColumn(
            ColumnNames[ index ] + ( ( index == m_StationsOrder ) ? ( m_StationsOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ),
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
    int count = guRADIOSTATIONS_COLUMN_COUNT;
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "RadioColSize%u" ), index ), GetColumnWidth( index ), wxT( "Positions" ) );
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
    return ColumnNames;
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

    wxArrayString ColumnNames = GetColumnNames();
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        SetColumnLabel( index,
            ColumnNames[ index ]  + ( ( index == m_StationsOrder ) ?
                ( m_StationsOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
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
    radiostation->m_IsUser      = m_Radios[ Selected ].m_IsUser;
    radiostation->m_Link        = m_Radios[ Selected ].m_Link;
    radiostation->m_Listeners   = m_Radios[ Selected ].m_Listeners;
    radiostation->m_Name        = m_Radios[ Selected ].m_Name;
    radiostation->m_Type        = m_Radios[ Selected ].m_Type;
    return true;
}

// -------------------------------------------------------------------------------- //
// guRadioLabelListBox
// -------------------------------------------------------------------------------- //
guRadioLabelListBox::guRadioLabelListBox( wxWindow * parent, guDbLibrary * NewDb, wxString Label ) :
    guListBox( parent, NewDb, Label, wxLB_MULTIPLE | guLISTVIEW_HIDE_HEADER )
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
              wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL ),
              m_TextChangedTimer( this, guRADIO_TIMER_TEXTSEARCH )
{
    m_Db = NewDb;
    m_PlayerPanel = NewPlayerPanel;

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
            wxAuiPaneInfo().Name( wxT( "RadioTextSearch" ) ).Caption( _( "Text Search" ) ).
            MinSize( 60, 28 ).MaxSize( -1, 28 ).Row( 0 ).Layer( 2 ).Position( 0 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Top() );



    wxPanel * GenrePanel;
	GenrePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * GenreSizer;
	GenreSizer = new wxBoxSizer( wxVERTICAL );


    m_GenresTreeCtrl = new guRadioGenreTreeCtrl( GenrePanel, m_Db );
	GenreSizer->Add( m_GenresTreeCtrl, 1, wxALL|wxEXPAND, 1 );

	GenrePanel->SetSizer( GenreSizer );
	GenrePanel->Layout();
	GenreSizer->Fit( GenrePanel );

	m_AuiManager.AddPane( GenrePanel,
            wxAuiPaneInfo().Name( wxT( "RadioGenres" ) ).Caption( _( "Genres" ) ).
            MinSize( 60, 60 ).Layer( 1 ).Row( 0 ).Position( 0 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Left() );



    wxPanel * LabelsPanel;
	LabelsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * LabelsSizer;
	LabelsSizer = new wxBoxSizer( wxVERTICAL );

	m_LabelsListBox = new guRadioLabelListBox( LabelsPanel, m_Db, _( "Labels" ) );
	LabelsSizer->Add( m_LabelsListBox, 1, wxALL|wxEXPAND, 1 );

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

    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchSelected ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    //m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );

    Connect( ID_RADIO_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsPlay ) );
    Connect( ID_RADIO_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueue ) );
    Connect( ID_RADIO_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueueAsNext ) );

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

    //
    Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED,  wxTreeEventHandler( guRadioPanel::OnRadioGenreListSelected ), NULL, this );

    m_GenresTreeCtrl->Disconnect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( guRadioPanel::OnRadioGenreListActivated ), NULL, this );

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

    m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guRadioPanel::OnSearchSelected ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    //m_InputTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler( guRadioPanel::OnSearchActivated ), NULL, this );
    m_InputTextCtrl->Disconnect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guRadioPanel::OnSearchCancelled ), NULL, this );

    Disconnect( ID_RADIO_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsPlay ) );
    Disconnect( ID_RADIO_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueue ) );
    Disconnect( ID_RADIO_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guRadioPanel::OnRadioStationsEnqueueAsNext ) );

    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guRadioPanel::OnPaneClose ), NULL, this );

    m_AuiManager.UnInit();
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
void guRadioPanel::OnRadioGenreListActivated( wxTreeEvent &event )
{
    wxTreeItemId ItemId = event.GetItem();
    guRadioGenreData * ItemData = ( guRadioGenreData * ) m_GenresTreeCtrl->GetItemData( ItemId );
    if( ItemData )
    {
        wxArrayInt RadioGenres;
        RadioGenres.Add( ItemData->GetId() );

        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
        int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge( _( "Radios" ) );
        guUpdateRadiosThread * UpdateRadiosThread = new guUpdateRadiosThread( m_Db, this, RadioGenres, GaugeId );
        if( UpdateRadiosThread )
        {
            UpdateRadiosThread->Create();
            UpdateRadiosThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
            UpdateRadiosThread->Run();
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
void guRadioPanel::OnRadioStationsEnqueueAsNext( wxCommandEvent &event )
{
    OnSelectStations( true, true );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSelectStations( bool enqueue, const bool asnext )
{
    guTrackArray   Tracks;
    guTrack *  NewSong;
    int Index;
    int Count;

    guRadioStation RadioStation;
    guStationPlayList PlayList;

    if( m_StationsListBox->GetSelected( &RadioStation ) )
    {
        guPlayListFile PlayListFile;
        if( RadioStation.m_SCId != wxNOT_FOUND )
        {
            guShoutCast ShoutCast;
            wxString StationUrl = ShoutCast.GetStationUrl( RadioStation.m_SCId );
            if( !StationUrl.IsEmpty() )
            {
                PlayListFile.Load( StationUrl );
            }
        }
        else
        {
            //guLogMessage( wxT( "Trying to load the link %s" ), RadioStation.m_Link.c_str() );
            PlayListFile.Load( RadioStation.m_Link );
        }

        if( ( Count = PlayListFile.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                NewSong = new guTrack();
                if( NewSong )
                {
                    guStationPlayListItem PlayListItem = PlayListFile.GetItem( Index );
                    NewSong->m_Type = guTRACK_TYPE_RADIOSTATION;
                    NewSong->m_FileName = PlayListItem.m_Location;
                    //NewSong->m_SongName = PlayList[ index ].m_Name;
                    NewSong->m_SongName = PlayListItem.m_Name.IsEmpty() ?
                                             RadioStation.m_Name : PlayListItem.m_Name;
                    //NewSong->m_AlbumName = RadioStation.m_Name;
                    NewSong->m_Length = 0;
                    NewSong->m_Rating = -1;
                    //NewSong->CoverId = guPLAYLIST_RADIOSTATION;
                    NewSong->m_CoverId = 0;
                    NewSong->m_Year = 0;
                    Tracks.Add( NewSong );
                }
            }

            if( Tracks.Count() )
            {
                if( enqueue )
                {
                    m_PlayerPanel->AddToPlayList( Tracks, true, asnext );
                }
                else
                {
                    m_PlayerPanel->SetPlayList( Tracks );
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
        RadioStation->m_IsUser = true;
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
// guUpdateRadiosThread
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
