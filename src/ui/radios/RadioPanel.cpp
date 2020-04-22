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
#include "RadioPanel.h"

#include "Accelerators.h"
#include "AuiDockArt.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "Images.h"
#include "LabelEditor.h"
#include "MainFrame.h"
#include "PlayListFile.h"
#include "Preferences.h"
//#include "RadioGenreEditor.h"
#include "RadioEditor.h"
#include "StatusBar.h"
#include "Settings.h"
#include "ShoutcastRadio.h"
#include "TagInfo.h"
#include "TuneInRadio.h"
#include "UserRadio.h"
#include "Utils.h"

#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

#define guRADIO_TIMER_TEXTSEARCH        1
#define guRADIO_TIMER_GENRESELECTED     2
#define guRADIO_TIMER_TEXTSEARCH_VALUE  500

// -------------------------------------------------------------------------------- //
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
class guRadioStationListBox : public guListView
{
  protected :
    guRadioPanel *              m_RadioPanel;
    guRadioStations             m_Radios;
    int                         m_StationsOrder;
    bool                        m_StationsOrderDesc;

    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

    virtual wxArrayString       GetColumnNames( void );

    void                        OnConfigUpdated( wxCommandEvent &event );
    void                        CreateAcceleratorTable();

  public :
    guRadioStationListBox( wxWindow * parent, guRadioPanel * radiopanel );
    ~guRadioStationListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual int inline          GetItemId( const int row ) const;
    virtual wxString inline     GetItemName( const int row ) const;

    void                        SetStationsOrder( int order );
    bool                        GetSelected( guRadioStation * radiostation ) const;

    void                        RefreshStations( void );
};




// -------------------------------------------------------------------------------- //
// guRadioGenreTreeCtrl
// -------------------------------------------------------------------------------- //
guRadioGenreTreeCtrl::guRadioGenreTreeCtrl( wxWindow * parent, guRadioPanel * radiopanel ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_SINGLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT|wxNO_BORDER )
{
    m_RadioPanel = radiopanel;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_ImageList = NULL;

    m_RootId   = AddRoot( wxT( "Radios" ), -1, -1, NULL );

    SetIndent( 10 );

    Bind( wxEVT_TREE_ITEM_MENU, &guRadioGenreTreeCtrl::OnContextMenu, this );
    Bind( wxEVT_KEY_DOWN, &guRadioGenreTreeCtrl::OnKeyDown, this );

    Bind( guConfigUpdatedEvent, &guRadioGenreTreeCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
guRadioGenreTreeCtrl::~guRadioGenreTreeCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_RADIO_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::ReloadProviders( guRadioProviderArray * radioproviders )
{
    m_ImageList = new wxImageList();
    DeleteChildren( m_RootId );
    int Index;
    int Count = m_RadioPanel->GetProviderCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guRadioProvider * RadioProvider = m_RadioPanel->GetProvider( Index );
        RadioProvider->RegisterImages( m_ImageList );
        RadioProvider->RegisterItems( this, m_RootId );
    }

    AssignImageList( m_ImageList );
}

// -------------------------------------------------------------------------------- //
void guRadioGenreTreeCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    m_RadioPanel->OnContextMenu( &Menu );
    if( Menu.GetMenuItemCount() )
    {
        wxPoint Point = event.GetPoint();

        PopupMenu( &Menu, Point );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
wxTreeItemId guRadioGenreTreeCtrl::GetItemId( wxTreeItemId * itemid, const int id )
{
    wxTreeItemIdValue Cookie;
    wxTreeItemId CurItem = GetFirstChild( * itemid, Cookie );
    while( CurItem.IsOk() )
    {
        guRadioItemData * ItemData = ( guRadioItemData * ) GetItemData( CurItem );
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
        // TODO : Limit to genres and searches
        wxCommandEvent CmdEvent( wxEVT_MENU, ID_RADIO_GENRE_DELETE );
        wxPostEvent( this, CmdEvent );
        return;
    }
    event.Skip();
}





// -------------------------------------------------------------------------------- //
// guRadioStationListBox
// -------------------------------------------------------------------------------- //
guRadioStationListBox::guRadioStationListBox( wxWindow * parent, guRadioPanel * radiopanel ) :
    guListView( parent, wxLB_SINGLE | guLISTVIEW_COLUMN_SELECT|guLISTVIEW_COLUMN_SORTING )
{
    m_RadioPanel = radiopanel;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_StationsOrder = m_RadioPanel->GetStationsOrder();
    m_StationsOrderDesc = m_RadioPanel->GetStationsOrderDesc();

    wxArrayString ColumnNames = GetColumnNames();
    // Create the Columns
    int ColId;
    wxString ColName;
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "id%u" ), index ), index, wxT( "radios/columns/ids" ) );

        ColName = ColumnNames[ ColId ];

        ColName += ( ( ColId == m_StationsOrder ) ? ( m_StationsOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "width%u" ), index ), 80, wxT( "radios/columns/widths" ) ),
            Config->ReadBool( wxString::Format( wxT( "show%u" ), index ), true, wxT( "radios/columns/shows" ) )
            );
        InsertColumn( Column );
    }

    Bind( guConfigUpdatedEvent, &guRadioStationListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guRadioStationListBox::~guRadioStationListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    //int ColId;
    int index;
    int count = guRADIOSTATIONS_COLUMN_COUNT;
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "id%u" ), index ), ( * m_Columns )[ index ].m_Id, wxT( "radios/columns/ids" ) );
        Config->WriteNum( wxString::Format( wxT( "width%u" ), index ), ( * m_Columns )[ index ].m_Width, wxT( "radios/columns/widths" ) );
        Config->WriteBool( wxString::Format( wxT( "show%u" ), index ), ( * m_Columns )[ index ].m_Enabled, wxT( "radios/columns/shows" ) );
    }

}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_RADIO_USER_EDIT );
    RealAccelCmds.Add( ID_RADIO_EDIT_LABELS );
    RealAccelCmds.Add( ID_RADIO_PLAY );
    RealAccelCmds.Add( ID_RADIO_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_RADIO_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_RADIO_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_RADIO_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_RADIO_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
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

        case guRADIOSTATIONS_COLUMN_BITRATE :
            if( Radio->m_BitRate )
                return wxString::Format( wxT( "%u" ), Radio->m_BitRate );
          break;

        case guRADIOSTATIONS_COLUMN_LISTENERS :
            if( Radio->m_Listeners )
                return wxString::Format( wxT( "%u" ), Radio->m_Listeners );
          break;

        case guRADIOSTATIONS_COLUMN_TYPE :
          return Radio->m_Type;

        case guRADIOSTATIONS_COLUMN_NOWPLAYING :
          return Radio->m_NowPlaying;
    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
void guRadioStationListBox::GetItemsList( void )
{
    m_RadioPanel->GetProviderStations( &m_Radios );

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::ReloadItems( bool reset )
{
    //
    wxArrayInt Selection;
    int FirstVisible = GetVisibleRowsBegin();

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
      ScrollToRow( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::RefreshStations( void )
{
    SetItemCount( m_Radios.Count() );

    RefreshAll();

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guRadioStationListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedCount();

    if( SelCount )
    {
//        Menu->AppendSeparator();
        MenuItem = new wxMenuItem( Menu, ID_RADIO_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                            _( "Play current selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_RADIO_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                            _( "Add current selected songs to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_RADIO_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_RADIO_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_RADIO_ENQUEUE_AFTER_ARTIST,
                                _( "Current Artist" ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_RADIO_ADD_TO_USER,
                            _( "Add to User Radio" ),
                            _( "Add selected radios as user radios" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );

        Menu->Append( MenuItem );
    }

    m_RadioPanel->OnContextMenu( Menu, true, SelCount );
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

//    m_Db->SetRadioStationsOrder( m_StationsOrder );
    m_RadioPanel->SetStationsOrder( m_StationsOrder, m_StationsOrderDesc );

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
// guRadioPanel
// -------------------------------------------------------------------------------- //
guRadioPanel::guRadioPanel( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel ) :
                guAuiManagerPanel( parent ),
                m_GenreSelectTimer( this, guRADIO_TIMER_GENRESELECTED ),
                m_TextChangedTimer( this, guRADIO_TIMER_TEXTSEARCH )
{
    m_Db = new guDbRadios( guPATH_RADIOS_DBNAME );
    m_PlayerPanel = playerpanel;
    m_RadioPlayListLoadThread = NULL;

    guConfig *  Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_MinBitRate = Config->ReadNum( CONFIG_KEY_RADIOS_MIN_BITRATE, 128, CONFIG_PATH_RADIOS );
    m_StationsOrder = Config->ReadNum( CONFIG_KEY_RADIOS_STATIONS_ORDER, 0, CONFIG_PATH_RADIOS );
    m_StationsOrderDesc = Config->ReadNum( CONFIG_KEY_RADIOS_STATIONS_ORDERDESC, false, CONFIG_PATH_RADIOS );;

    m_RadioProviders = new guRadioProviderArray();

    InitPanelData();

    m_VisiblePanels = Config->ReadNum( CONFIG_KEY_RADIOS_VISIBLE_PANELS, guPANEL_RADIO_VISIBLE_DEFAULT, CONFIG_PATH_RADIOS );
    m_InstantSearchEnabled = Config->ReadBool( CONFIG_KEY_GENERAL_INSTANT_TEXT_SEARCH, true, CONFIG_PATH_GENERAL );
    m_EnterSelectSearchEnabled = !Config->ReadBool( CONFIG_KEY_GENERAL_TEXT_SEARCH_ENTER, false, CONFIG_PATH_GENERAL );


	wxBoxSizer * SearchSizer;
	SearchSizer = new wxBoxSizer( wxHORIZONTAL );
    wxPanel * SearchPanel;
	SearchPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    m_InputTextCtrl = new wxSearchCtrl( SearchPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, 28 ), wxTE_PROCESS_ENTER );
    SearchSizer->Add( m_InputTextCtrl, 1, wxALIGN_CENTER|wxTOP|wxBOTTOM, 5 );

    SearchPanel->SetSizer( SearchSizer );
    SearchPanel->Layout();
	SearchSizer->Fit( SearchPanel );

    m_AuiManager.AddPane( SearchPanel,
            wxAuiPaneInfo().Name( wxT( "RadioTextSearch" ) ).Caption( _( "Text Search" ) ).
            Direction( 1 ).Layer( 2 ).Row( 0 ).Position( 0 ).BestSize( 111, 26 ).MinSize( 60, 26 ).MaxSize( -1, 26 ).
            CloseButton( Config->ReadBool( CONFIG_KEY_GENERAL_SHOW_CLOSE_BUTTON, true, CONFIG_PATH_GENERAL ) ).
            Dockable( true ).Top() );

    wxPanel * GenrePanel;
	GenrePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * GenreSizer;
	GenreSizer = new wxBoxSizer( wxVERTICAL );

    m_GenresTreeCtrl = new guRadioGenreTreeCtrl( GenrePanel, this );
	GenreSizer->Add( m_GenresTreeCtrl, 1, wxEXPAND, 5 );

	GenrePanel->SetSizer( GenreSizer );
	GenrePanel->Layout();
	GenreSizer->Fit( GenrePanel );

	m_AuiManager.AddPane( GenrePanel,
            wxAuiPaneInfo().Name( wxT( "RadioGenres" ) ).Caption( _( "Genre" ) ).
            Direction( 4 ).Layer( 1 ).Row( 0 ).Position( 0 ).BestSize( 220, 60 ).MinSize( 60, 60 ).
            CloseButton( Config->ReadBool( CONFIG_KEY_GENERAL_SHOW_CLOSE_BUTTON, true, CONFIG_PATH_GENERAL ) ).
            Dockable( true ).Left() );

    wxPanel * StationsPanel;
	StationsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxBoxSizer * StationsSizer;
	StationsSizer = new wxBoxSizer( wxVERTICAL );

	m_StationsListBox = new guRadioStationListBox( StationsPanel, this );

	StationsSizer->Add( m_StationsListBox, 1, wxEXPAND, 5 );

	StationsPanel->SetSizer( StationsSizer );
	StationsPanel->Layout();
	StationsSizer->Fit( StationsPanel );

    m_AuiManager.AddPane( StationsPanel, wxAuiPaneInfo().Name( wxT( "RadioStations" ) ).Caption( _( "Stations" ) ).
            MinSize( 50, 50 ).
            CenterPane() );



    wxString RadioLayout = Config->ReadStr( CONFIG_KEY_RADIOS_LAST_LAYOUT, wxEmptyString, CONFIG_PATH_RADIOS );
    if( Config->GetIgnoreLayouts() || RadioLayout.IsEmpty() )
    {
        m_VisiblePanels = guPANEL_RADIO_VISIBLE_DEFAULT;
        RadioLayout = wxT( "layout2|name=RadioTextSearch;caption=" ) + wxString( _( "Text Search" ) );
        RadioLayout += wxT( ";state=2099196;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=111;besth=26;minw=60;minh=26;maxw=-1;maxh=26;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        RadioLayout += wxT( "name=RadioGenres;caption=" ) + wxString( _( "Genres" ) ) + wxT( ";state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=220;besth=60;minw=60;minh=60;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
//        RadioLayout += wxT( "name=RadioLabels;caption=" ) + wxString( _( "Labels" ) ) + wxT( ";state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=220;besth=60;minw=60;minh=60;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        RadioLayout += wxT( "name=RadioStations;caption=" ) + wxString( _( "Stations" ) ) + wxT( ";state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=50;besth=50;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        RadioLayout += wxT( "dock_size(1,2,0)=47|dock_size(4,1,0)=179|dock_size(5,0,0)=52|" );
        //m_AuiManager.Update();
    }

    m_AuiManager.LoadPerspective( RadioLayout, true );

    Bind( wxEVT_TIMER, &guRadioPanel::OnTextChangedTimer, this, guRADIO_TIMER_TEXTSEARCH );

    Bind( wxEVT_TREE_SEL_CHANGED, &guRadioPanel::OnRadioGenreListSelected, this );

    Bind( wxEVT_MENU, &guRadioPanel::OnRadioUpdate, this, ID_RADIO_DOUPDATE );
    Bind( wxEVT_MENU, &guRadioPanel::OnRadioUpdated, this, ID_RADIO_UPDATED );
    Bind( wxEVT_MENU, &guRadioPanel::OnRadioUpdateEnd, this, ID_RADIO_UPDATE_END );
    Bind( wxEVT_MENU, &guRadioPanel::OnRadioCreateItems, this, ID_RADIO_CREATE_TREE_ITEM );
    Bind( wxEVT_MENU, &guRadioPanel::OnLoadStationsFinished, this, ID_RADIO_LOADING_STATIONS_FINISHED );

    m_StationsListBox->Bind( wxEVT_LISTBOX_DCLICK, &guRadioPanel::OnStationListActivated, this );
    m_StationsListBox->Bind( wxEVT_LIST_COL_CLICK, &guRadioPanel::OnStationListBoxColClick, this );

    m_InputTextCtrl->Bind( wxEVT_TEXT_ENTER, &guRadioPanel::OnSearchSelected, this );
    m_InputTextCtrl->Bind( wxEVT_TEXT, &guRadioPanel::OnSearchActivated, this );
    m_InputTextCtrl->Bind( wxEVT_SEARCHCTRL_CANCEL_BTN, &guRadioPanel::OnSearchCancelled, this );

    Bind( wxEVT_MENU, &guRadioPanel::OnRadioStationsPlay, this, ID_RADIO_PLAY );
    Bind( wxEVT_MENU, &guRadioPanel::OnRadioStationsEnqueue, this, ID_RADIO_ENQUEUE_AFTER_ALL, ID_RADIO_ENQUEUE_AFTER_ARTIST );

    Bind( wxEVT_MENU, &guRadioPanel::OnStationPlayListLoaded, this, ID_RADIO_PLAYLIST_LOADED );

    Bind( guConfigUpdatedEvent, &guRadioPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_TIMER, &guRadioPanel::OnGenreSelectTimeout , this, guRADIO_TIMER_GENRESELECTED );

    Bind( wxEVT_MENU, &guRadioPanel::OnRadioStationsAddToUser, this, ID_RADIO_ADD_TO_USER );


    // Create shoutcast radio provider
    RegisterRadioProvider( new guShoutcastRadioProvider( this, m_Db ) );
    // Create tunein radio provider
    RegisterRadioProvider( new guTuneInRadioProvider( this, m_Db ) );
    // Create user radio provider
    RegisterRadioProvider( new guUserRadioProvider( this, m_Db ), true );
}

// -------------------------------------------------------------------------------- //
guRadioPanel::~guRadioPanel()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( CONFIG_KEY_RADIOS_VISIBLE_PANELS, m_VisiblePanels, CONFIG_PATH_RADIOS );
    Config->WriteStr( CONFIG_KEY_RADIOS_LAST_LAYOUT, m_AuiManager.SavePerspective(), CONFIG_PATH_RADIOS );
    Config->WriteNum( CONFIG_KEY_RADIOS_STATIONS_ORDER, m_StationsOrder, CONFIG_PATH_RADIOS );
    Config->WriteBool( CONFIG_KEY_RADIOS_STATIONS_ORDERDESC, m_StationsOrderDesc, CONFIG_PATH_RADIOS );;

    m_RadioPlayListLoadThreadMutex.Lock();
    if( m_RadioPlayListLoadThread )
    {
        m_RadioPlayListLoadThread->Pause();
        m_RadioPlayListLoadThread->Delete();
    }
    m_RadioPlayListLoadThreadMutex.Unlock();


    if( m_RadioProviders )
    {
        delete m_RadioProviders;
    }

    if( m_Db )
    {
        delete m_Db;
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::InitPanelData( void )
{
    m_PanelNames.Add( wxT( "RadioTextSearch" ) );
    m_PanelNames.Add( wxT( "RadioLabels" ) );
    m_PanelNames.Add( wxT( "RadioGenres" ) );

    m_PanelIds.Add( guPANEL_RADIO_TEXTSEARCH );
//    m_PanelIds.Add( guPANEL_RADIO_LABELS );
    m_PanelIds.Add( guPANEL_RADIO_GENRES );

    m_PanelCmdIds.Add( ID_MENU_VIEW_RAD_TEXTSEARCH );
//    m_PanelCmdIds.Add( ID_MENU_VIEW_RAD_LABELS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_RAD_GENRES );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_GENERAL )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_InstantSearchEnabled = Config->ReadBool( CONFIG_KEY_GENERAL_INSTANT_TEXT_SEARCH, true, CONFIG_PATH_GENERAL );
        m_EnterSelectSearchEnabled = !Config->ReadBool( CONFIG_KEY_GENERAL_TEXT_SEARCH_ENTER, false, CONFIG_PATH_GENERAL );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::RegisterRadioProvider( guRadioProvider * radioprovider, const bool reload )
{
    m_RadioProviders->Add( radioprovider );

    if( reload )
    {
        ReloadProviders();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::UnRegisterRadioProvider( guRadioProvider * radioprovider, const bool reload )
{
    // This makes de radioprovider to be destroyed
    m_RadioProviders->Remove( radioprovider );

    delete radioprovider;

    if( reload )
    {
        ReloadProviders();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::SetStationsOrder( const int columnid, const bool desc )
{
    m_StationsOrder = columnid;
    m_StationsOrderDesc = desc;
    int Index;
    int Count = GetProviderCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guRadioProvider * RadioProvider = GetProvider( Index );
        RadioProvider->SetStationsOrder( columnid, desc );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::ReloadProviders( void )
{
    m_GenresTreeCtrl->ReloadProviders( m_RadioProviders );
}

// -------------------------------------------------------------------------------- //
guRadioProvider * guRadioPanel::GetProvider( const wxTreeItemId &itemid )
{
    int Index;
    int Count = GetProviderCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guRadioProvider * RadioProvider = GetProvider( Index );
        if( RadioProvider->HasItemId( itemid ) )
        {
            return RadioProvider;
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
wxTreeItemId guRadioPanel::GetSelectedGenre( void )
{
    return m_GenresTreeCtrl->GetSelection();
}

// -------------------------------------------------------------------------------- //
guRadioItemData * guRadioPanel::GetSelectedData( const wxTreeItemId &itemid )
{
    return ( guRadioItemData * ) m_GenresTreeCtrl->GetItemData( itemid );
}

// -------------------------------------------------------------------------------- //
guRadioItemData * guRadioPanel::GetSelectedData( void )
{
    return GetSelectedData( GetSelectedGenre() );
}

// -------------------------------------------------------------------------------- //
wxTreeItemId guRadioPanel::GetItemParent( const wxTreeItemId &item ) const
{
    return m_GenresTreeCtrl->GetItemParent( item );
}

// -------------------------------------------------------------------------------- //
int guRadioPanel::GetProviderStations( guRadioStations * stations )
{
    wxTreeItemId SelectedItemId = m_GenresTreeCtrl->GetSelection();
    if( SelectedItemId.IsOk() )
    {
        guRadioProvider * RadioProvider = GetProvider( SelectedItemId );
        if( RadioProvider )
        {
            return RadioProvider->GetStations( stations, m_MinBitRate );
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
bool guRadioPanel::OnContextMenu( wxMenu * menu, const bool forstations, const int selcount )
{
    wxTreeItemId SelectedItemId = m_GenresTreeCtrl->GetSelection();
    if( SelectedItemId.IsOk() )
    {
        guRadioProvider * RadioProvider = GetProvider( SelectedItemId );
        if( RadioProvider )
        {
            return RadioProvider->OnContextMenu( menu, SelectedItemId, forstations, selcount );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchActivated( wxCommandEvent& event )
{
    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();

    if( !m_InstantSearchEnabled )
        return;

    m_TextChangedTimer.Start( guRADIO_TIMER_TEXTSEARCH_VALUE, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchSelected( wxCommandEvent& event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();

    if( !DoTextSearch() || !m_EnterSelectSearchEnabled || !m_InstantSearchEnabled )
        return;

    OnSelectStations( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false, CONFIG_PATH_GENERAL ) );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSearchCancelled( wxCommandEvent &event ) // CLEAN SEARCH STR
{
    m_InputTextCtrl->Clear();

    if( !m_InstantSearchEnabled )
        DoTextSearch();
}

// -------------------------------------------------------------------------------- //
bool guRadioPanel::DoTextSearch( void )
{
    wxString SearchString = m_InputTextCtrl->GetLineText( 0 );
    guLogMessage( wxT( "Should do the search now: '%s'" ), SearchString.c_str() );
    if( !SearchString.IsEmpty() )
    {
        wxArrayString Words = guSplitWords( SearchString );
        int Index;
        int Count = m_RadioProviders->Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guRadioProvider * RadioProvicer = GetProvider( Index );
            RadioProvicer->SetSearchText( Words );
        }

        m_StationsListBox->ReloadItems();

        m_InputTextCtrl->ShowCancelButton( true );
        return true;
    }
    else
    {
        wxArrayString Words;
        int Index;
        int Count = m_RadioProviders->Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guRadioProvider * RadioProvicer = GetProvider( Index );
            RadioProvicer->SetSearchText( Words );
        }

        m_StationsListBox->ReloadItems();

        m_InputTextCtrl->ShowCancelButton( false );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnTextChangedTimer( wxTimerEvent &event )
{
    guLogMessage( wxT( "OnTextChangedTimer..." ) );
    DoTextSearch();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListBoxColClick( wxListEvent &event )
{
    int ColId = m_StationsListBox->GetColumnId( event.m_col );
    m_StationsListBox->SetStationsOrder( ColId );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationListActivated( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    OnSelectStations( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false, CONFIG_PATH_GENERAL ) );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::RefreshStations( void )
{
    m_StationsListBox->RefreshStations();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioCreateItems( wxCommandEvent &event )
{
    wxTreeItemId SelectedItemId = m_GenresTreeCtrl->GetSelection();
    if( SelectedItemId.IsOk() )
    {
        guRadioProvider * RadioProvider = GetProvider( SelectedItemId );
        if( RadioProvider )
        {
            wxArrayString Items = RadioProvider->GetPendingItems();
            int Index;
            int Count = Items.Count();
            //guLogMessage( wxT( "Processing %i Pending Items" ), Count );
            for( Index = 0; Index < Count; Index++ )
            {
                wxArrayString StationParam = wxStringTokenize( Items[ Index ], wxT( "|" ) );
                if( StationParam.Count() == 2 )
                {
                    m_GenresTreeCtrl->AppendItem( SelectedItemId, StationParam[ 0 ], -1, -1,
                        new guRadioItemData( -1, guRADIO_SOURCE_TUNEIN, StationParam[ 0 ], StationParam[ 1 ], 0 ) );
                }
            }
            m_GenresTreeCtrl->Expand( SelectedItemId );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guRadioPanel::GetSelectedStation( guRadioStation * station )
{
    return m_StationsListBox->GetSelected( station );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::StartLoadingStations( void )
{
    wxSetCursor( * wxHOURGLASS_CURSOR );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::EndLoadingStations( void )
{
    wxSetCursor( * wxSTANDARD_CURSOR );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnLoadStationsFinished( wxCommandEvent &event )
{
    EndLoadingStations();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnGenreSelectTimeout( wxTimerEvent &event )
{
    ReloadStations();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::ReloadStations( void )
{
    StartLoadingStations();

    guRadioProvider * SelectedProvider = GetProvider( GetSelectedGenre() );

    // If there was a previos search in progress from other provider cancell it...
    int Count = m_RadioProviders->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guRadioProvider * RadioProvider = m_RadioProviders->Item( Index );
        if( RadioProvider != SelectedProvider )
            RadioProvider->CancellSearchStations();
    }

    m_StationsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioGenreListSelected( wxTreeEvent &event )
{
    m_GenreSelectTimer.Start( 50, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdate( wxCommandEvent &event )
{
    wxTreeItemId ItemId = GetSelectedGenre();
    guRadioProvider * RadioProvider = GetProvider( ItemId );
    if( RadioProvider )
    {
        RadioProvider->DoUpdate();
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdated( wxCommandEvent &event )
{
    RefreshStations();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioUpdateEnd( wxCommandEvent &event )
{
    //wxSetCursor( * wxSTANDARD_CURSOR );
    //GenresListBox->Enable();
    ReloadStations();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioStationsPlay( wxCommandEvent &event )
{
    OnSelectStations( false );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioStationsEnqueue( wxCommandEvent &event )
{
    OnSelectStations( true, event.GetId() - ID_RADIO_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnSelectStations( bool enqueue, const int aftercurrent )
{
    wxString StationUrl;
    guRadioStation RadioStation;

    if( GetSelectedStation( &RadioStation ) )
    {
        guLogMessage( wxT( "Selected %s" ), RadioStation.m_Name.c_str() );
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
            LoadStationUrl( StationUrl, enqueue, aftercurrent );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnRadioStationsAddToUser( wxCommandEvent &event )
{
    guRadioStation RadioStation;

    if( GetSelectedStation( &RadioStation ) )
    {

        RadioStation.m_Id = wxNOT_FOUND;
        if( RadioStation.m_SCId != wxNOT_FOUND )
        {
            guShoutCast ShoutCast;
            RadioStation.m_Link = ShoutCast.GetStationUrl( RadioStation.m_SCId );
        }
        RadioStation.m_SCId = wxNOT_FOUND;
        RadioStation.m_BitRate = 0;
        RadioStation.m_GenreId = wxNOT_FOUND;
        RadioStation.m_Source = 1;
        RadioStation.m_Listeners = 0;
        RadioStation.m_Type = wxEmptyString;

        m_Db->SetRadioStation( &RadioStation );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::EndStationPlayListLoaded( void )
{
    wxMutexLocker MutexLocker( m_RadioPlayListLoadThreadMutex );
    m_RadioPlayListLoadThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::LoadStationUrl( const wxString &stationurl, const bool enqueue, const int aftercurrent )
{
    wxMutexLocker MutexLocker( m_RadioPlayListLoadThreadMutex );

    if( m_RadioPlayListLoadThread )
    {
        m_RadioPlayListLoadThread->Pause();
        m_RadioPlayListLoadThread->Delete();
    }

    m_StationPlayListTracks.Empty();
    m_RadioPlayListLoadThread = new guRadioPlayListLoadThread( this, stationurl.c_str(), &m_StationPlayListTracks, enqueue, aftercurrent );
    if( !m_RadioPlayListLoadThread )
    {
        guLogError( wxT( "Could not create the download radio playlist thread" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnStationPlayListLoaded( wxCommandEvent &event )
{
    bool Enqueue = event.GetInt();
    int  AfterCurrent = event.GetExtraLong();

    if( m_StationPlayListTracks.Count() )
    {
        if( Enqueue )
        {
            m_PlayerPanel->AddToPlayList( m_StationPlayListTracks, true, AfterCurrent );
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
void guRadioPanel::GetRadioCounter( wxLongLong * count )
{
    * count = m_StationsListBox->GetItemCount();
}

// -------------------------------------------------------------------------------- //
void guRadioPanel::OnGoToSearch( wxCommandEvent &event )
{
    if( !( m_VisiblePanels & guPANEL_RADIO_TEXTSEARCH ) )
    {
        ShowPanel( guPANEL_RADIO_TEXTSEARCH, true );
    }

    if( FindFocus() != m_InputTextCtrl )
        m_InputTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
bool guRadioPanel::GetListViewColumnData( const int id, int * index, int * width, bool * enabled )
{
    return m_StationsListBox->GetColumnData( id, index, width, enabled );
}

// -------------------------------------------------------------------------------- //
bool guRadioPanel::SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh )
{
    return m_StationsListBox->SetColumnData( id, index, width, enabled, refresh );
}



// -------------------------------------------------------------------------------- //
// guShoutcastSearch
// -------------------------------------------------------------------------------- //
guShoutcastSearch::guShoutcastSearch( wxWindow * parent, guRadioItemData * itemdata ) :
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
	ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
	ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	ButtonsSizerOK->SetDefault();

    Bind( wxEVT_BUTTON, &guShoutcastSearch::OnOkButton, this, wxID_OK );

	m_SearchTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
guShoutcastSearch::~guShoutcastSearch()
{
    Unbind( wxEVT_BUTTON, &guShoutcastSearch::OnOkButton, this, wxID_OK );
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
// guRadioPlayListLoadThread
// -------------------------------------------------------------------------------- //
guRadioPlayListLoadThread::guRadioPlayListLoadThread( guRadioPanel * radiopanel,
        const wxChar * stationurl, guTrackArray * tracks, const bool enqueue, const int aftercurrent ) :
        m_StationUrl( stationurl )
{
    m_RadioPanel = radiopanel;
    //m_StationUrl = stationurl;
    m_Tracks = tracks;
    m_Enqueue = enqueue;
    m_AfterCurrent = aftercurrent;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 10 );
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
    guPlaylistFile PlayListFile;

    if( TestDestroy() )
        return 0;

    guLogMessage( wxT( "StationUrl: '%s'" ), m_StationUrl.c_str() );
    PlayListFile.Load( m_StationUrl );

    int Count = PlayListFile.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( TestDestroy() )
            break;

        NewSong = new guTrack();
        if( NewSong )
        {
            guPlaylistItem PlayListItem = PlayListFile.GetItem( Index );
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
        wxCommandEvent Event( wxEVT_MENU, ID_RADIO_PLAYLIST_LOADED );
        Event.SetInt( m_Enqueue );
        Event.SetExtraLong( m_AfterCurrent );
        wxPostEvent( m_RadioPanel, Event );
    }

    return 0;
}

}

// -------------------------------------------------------------------------------- //
