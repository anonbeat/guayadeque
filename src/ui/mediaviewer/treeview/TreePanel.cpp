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
#include "TreePanel.h"

#include "Accelerators.h"
#include "AuiNotebook.h"
#include "AuiDockArt.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "DbLibrary.h"
#include "DynamicPlayList.h"
#include "EditWithOptions.h"
#include "FileRenamer.h"
#include "Images.h"
#include "LabelEditor.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "PlayListAppend.h"
#include "PlayListFile.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "TreeViewFilterEditor.h"
#include "Utils.h"

#include <wx/tokenzr.h>

namespace Guayadeque {

//#define guTREEVIEW_TIMER_TEXTSEARCH        5
//#define guTREEVIEW_TIMER_TEXTSEARCH_VALUE  500
#define guTREEVIEW_TIMER_TREEITEMSELECTED  6
#define guTREEVIEW_TIMER_TREEITEMSELECTED_VALUE  50

BEGIN_EVENT_TABLE( guTreeViewTreeCtrl, wxTreeCtrl )
    EVT_TREE_BEGIN_DRAG( wxID_ANY, guTreeViewTreeCtrl::OnBeginDrag )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guTreeViewTreeCtrl::guTreeViewTreeCtrl( wxWindow * parent, guDbLibrary * db, guTreeViewPanel * treeviewpanel ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT|wxTR_MULTIPLE|wxTR_TWIST_BUTTONS|wxNO_BORDER )
{
    m_Db = db;
    m_TreeViewPanel = treeviewpanel;
    m_ConfigPath = treeviewpanel->ConfigPath();


    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_FilterEntries = Config->ReadAStr( wxT( "Filter" ), wxEmptyString, m_ConfigPath + wxT( "/sortings" ) );
    m_CurrentFilter = Config->ReadNum( wxT( "TreeViewFilter" ), 0, m_ConfigPath );
    if( !m_FilterEntries.Count() )
    {
        m_FilterEntries.Add( wxString( _( "Genres,Artists,Albums" ) ) + wxT( ":2:3:6") );
        m_FilterEntries.Add( wxString( _( "Genres,Year,Artists,Albums" ) ) + wxT( ":2:7:3:6") );
        m_CurrentFilter = 0;
    }

    if( ( m_CurrentFilter < 0 ) || ( m_CurrentFilter >= ( int ) m_FilterEntries.Count() ) )
    {
        m_CurrentFilter = 0;
    }

    m_ImageList = new wxImageList();
    m_ImageList->Add( wxBitmap( guImage( guIMAGE_INDEX_track ) ) );
    m_ImageList->Add( wxBitmap( guImage( guIMAGE_INDEX_filter ) ) );
    AssignImageList( m_ImageList );

    m_RootId   = AddRoot( wxT( "Sortings" ), -1, -1, NULL );
    m_FiltersId = AppendItem( m_RootId, _( "Sortings" ), 1, 1, NULL );
    m_LibraryId = AppendItem( m_RootId, _( "Library" ), 0, 0, NULL );

    SetIndent( 10 );

    Bind( wxEVT_MENU, &guTreeViewTreeCtrl::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Bind( wxEVT_TREE_ITEM_MENU, &guTreeViewTreeCtrl::OnContextMenu, this );
    Bind( wxEVT_MENU, &guTreeViewTreeCtrl::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    Bind( guConfigUpdatedEvent, &guTreeViewTreeCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();

    LoadFilterLayout();
    ReloadFilters();

    ReloadItems();

    Expand( m_FiltersId );
    Expand( m_LibraryId );
}

// -------------------------------------------------------------------------------- //
guTreeViewTreeCtrl::~guTreeViewTreeCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteAStr( wxT( "Filter" ), m_FilterEntries, m_ConfigPath + wxT( "/sortings" ) );
    Config->WriteNum( wxT( "TreeViewFilter" ), m_CurrentFilter, m_ConfigPath );

    Unbind( wxEVT_MENU, &guTreeViewTreeCtrl::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Unbind( wxEVT_TREE_ITEM_MENU, &guTreeViewTreeCtrl::OnContextMenu, this );
    Unbind( wxEVT_MENU, &guTreeViewTreeCtrl::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    Unbind( guConfigUpdatedEvent, &guTreeViewTreeCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_TREEVIEW_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_TREEVIEW_EDITLABELS );
    RealAccelCmds.Add( ID_TREEVIEW_EDITTRACKS );
    RealAccelCmds.Add( ID_TREEVIEW_PLAY );
    RealAccelCmds.Add( ID_TREEVIEW_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_TREEVIEW_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_TREEVIEW_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_TREEVIEW_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_TREEVIEW_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

wxTreeItemId LastTreeItemId[ 10 ];

// -------------------------------------------------------------------------------- //
void AddTreeItems( wxTreeCtrl * treectrl, wxTreeItemId &curitem, wxSQLite3ResultSet &dbres, const wxArrayInt &filters, wxArrayInt &lastdata, const int curfilter, const int respos )
{
    //guLogMessage( wxT( "%02i  %02i  %02i  (%02i)" ), curfilter, filters[ curfilter ], respos, lastdata[ curfilter ] );
    if( lastdata[ curfilter ] != dbres.GetInt( respos ) )
    {
        lastdata[ curfilter ] = dbres.GetInt( respos );
        for( int Index = curfilter + 1; Index < ( int ) filters.Count(); Index++ )
            lastdata[ Index ] = wxNOT_FOUND;

        LastTreeItemId[ curfilter ] = treectrl->AppendItem( curitem,
                ( filters[ curfilter ] < guLIBRARY_ELEMENT_YEARS ) ? dbres.GetString( respos + 1 ) : wxString::Format( wxT( "%i" ), lastdata[ curfilter ] ),
                wxNOT_FOUND,
                wxNOT_FOUND,
                new guTreeViewData( lastdata[ curfilter ], filters[ curfilter ] ) );
    }

    if( curfilter < ( int ) filters.Count() - 1 )
    {
        AddTreeItems( treectrl, LastTreeItemId[ curfilter ], dbres, filters, lastdata, curfilter + 1, respos + ( ( filters[ curfilter ] < guLIBRARY_ELEMENT_YEARS ) ? 2 : 1 ) );
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::LoadFilterLayout( void )
{
    m_FilterLayout.Empty();
    wxArrayString FilterItems = wxStringTokenize( m_FilterEntries[ m_CurrentFilter ].AfterFirst( wxT( ':' ) ), wxT( ':' ) );

    int Index;
    int Count = FilterItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        long Value;
        FilterItems[ Index ].ToLong( &Value );
        m_FilterLayout.Add( Value );
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::ReloadFilters( void )
{
    DeleteChildren( m_FiltersId );

    wxFont BoldFont = GetFont();
    BoldFont.SetWeight( wxFONTWEIGHT_BOLD );

    int Index;
    int Count = m_FilterEntries.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        wxTreeItemId CurItem = AppendItem( m_FiltersId, m_FilterEntries[ Index ].BeforeFirst( wxT( ':' ) ), wxNOT_FOUND, wxNOT_FOUND,
                   new guTreeViewData( Index, Index ) );
        if( Index == m_CurrentFilter )
            SetItemFont( CurItem, BoldFont );
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::ReloadItems( void )
{
    DeleteChildren( m_LibraryId );

    wxArrayInt LastDataIds;

    LastDataIds.Add( wxNOT_FOUND, m_FilterLayout.Count() );

    wxString Query = wxT( "SELECT DISTINCT " );
    wxString Sorting = wxT( " ORDER BY " );
    int Index;
    int Count = m_FilterLayout.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        switch( m_FilterLayout[ Index ] )
        {
            case guLIBRARY_ELEMENT_GENRES :
                Query += wxT( "song_genreid, song_genre" );
                Sorting += wxT( "song_genre" );
                break;

            case guLIBRARY_ELEMENT_ARTISTS :
                Query += wxT( "song_artistid, song_artist" );
                Sorting += wxT( "song_artist" );
                break;

            case guLIBRARY_ELEMENT_COMPOSERS :
                Query += wxT( "song_composerid, song_composer" );
                Sorting += wxT( "song_composer" );
                break;

            case guLIBRARY_ELEMENT_ALBUMARTISTS :
                Query += wxT( "song_albumartistid, song_albumartist" );
                Sorting += wxT( "song_albumartist" );
                break;

            case guLIBRARY_ELEMENT_ALBUMS :
                Query += wxT( "song_albumid, song_album" );
                Sorting += wxT( "song_album" );
                break;

            case guLIBRARY_ELEMENT_YEARS :
                Query += wxT( "song_year" );
                Sorting += wxT( "song_year DESC" );
                break;

            case guLIBRARY_ELEMENT_RATINGS :
                Query += wxT( "song_rating" );
                Sorting += wxT( "song_rating DESC" );
                break;

            case guLIBRARY_ELEMENT_PLAYCOUNT :
                Query += wxT( "song_playcount" );
                Sorting += wxT( "song_playcount DESC" );
                break;
        }

        Query += wxT( ", " );
        Sorting += wxT( ", " );
    }
    Query.RemoveLast( 2 );
    Sorting.RemoveLast( 2 );
    Query += wxT( " FROM songs " );
    if( m_TextFilters.Count() )
    {
        Query += wxT( "WHERE " ) + TextFilterToSQL( m_TextFilters );
    }

    wxSQLite3ResultSet dbRes;

    dbRes = m_Db->ExecuteQuery( Query + Sorting );

    while( dbRes.NextRow() )
    {
        AddTreeItems( this, m_LibraryId, dbRes, m_FilterLayout, LastDataIds, 0, 0 );
    }

    dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void AddTreeViewCommands( wxMenu * Menu, int ItemType )
{
    if( Menu && ( ( ItemType == guLIBRARY_ELEMENT_ARTISTS ) ||
        ( ItemType == guLIBRARY_ELEMENT_ALBUMARTISTS ) ||
        ( ItemType == guLIBRARY_ELEMENT_ALBUMS ) ) )
    {
        wxMenu * SubMenu;
        wxMenuItem * MenuItem = NULL;

        int index;
        int count;
        SubMenu = new wxMenu();

        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Commands = Config->ReadAStr( CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, CONFIG_PATH_COMMANDS_EXECS );
        wxArrayString Names = Config->ReadAStr( CONFIG_KEY_COMMANDS_NAME, wxEmptyString, CONFIG_PATH_COMMANDS_NAMES );
        if( ( count = Commands.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                if( ( ItemType == guLIBRARY_ELEMENT_ALBUMS ) ||
                    ( ItemType == guLIBRARY_ELEMENT_ARTISTS ) ||
                    ( ItemType == guLIBRARY_ELEMENT_ALBUMARTISTS ) )
                {
                    MenuItem = new wxMenuItem( Menu, ID_COMMANDS_BASE + index, Names[ index ], Commands[ index ] );
                }
                SubMenu->Append( MenuItem );
            }

            SubMenu->AppendSeparator();
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, ID_MENU_PREFERENCES_COMMANDS, _( "Preferences" ), _( "Add commands in preferences" ) );
            SubMenu->Append( MenuItem );
        }

        Menu->AppendSubMenu( SubMenu, _( "Commands" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPoint();
    wxArrayTreeItemIds SelectedItems;
    int SelectCount = GetSelections( SelectedItems );

    wxTreeItemId ItemId = event.GetItem();
    guTreeViewData * ItemData = NULL;

    if( ItemId.IsOk() )
    {
        if( ItemId == m_FiltersId || GetItemParent( ItemId ) == m_FiltersId )
        {
            MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_FILTER_NEW, _( "Create" ), _( "Create a new filter" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_FILTER_EDIT, _( "Edit" ), _( "Edit the selected filter" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_FILTER_DELETE, _( "Delete" ), _( "Delete the selected filter" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
            Menu.Append( MenuItem );

            Menu.AppendSeparator();
        }
        else
        {
            ItemData = ( guTreeViewData * ) GetItemData( ItemId );
            if( ItemData )
            {
                int ContextMenuFlags = GetContextMenuFlags();

                MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_PLAY, _( "Play" ), _( "Play current selected songs" ) );
                MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
                Menu.Append( MenuItem );

                MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_ENQUEUE_AFTER_ALL,
                                wxString( _( "Enqueue" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                                _( "Add current selected songs to the playlist" ) );
                MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
                Menu.Append( MenuItem );

                wxMenu * EnqueueMenu = new wxMenu();

                MenuItem = new wxMenuItem( EnqueueMenu, ID_TREEVIEW_ENQUEUE_AFTER_TRACK,
                                        wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                                        _( "Add current selected tracks to playlist after the current track" ) );
                MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
                EnqueueMenu->Append( MenuItem );

                MenuItem = new wxMenuItem( EnqueueMenu, ID_TREEVIEW_ENQUEUE_AFTER_ALBUM,
                                        wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                                        _( "Add current selected tracks to playlist after the current album" ) );
                MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
                EnqueueMenu->Append( MenuItem );

                MenuItem = new wxMenuItem( EnqueueMenu, ID_TREEVIEW_ENQUEUE_AFTER_ARTIST,
                                        wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                                        _( "Add current selected tracks to playlist after the current artist" ) );
                MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
                EnqueueMenu->Append( MenuItem );

                Menu.Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

                Menu.AppendSeparator();

                int ItemType = ItemData->m_Type;
                //guLogMessage( wxT( "SelCount: %i  Type: %i" ), SelectCount, ItemType );
                if( SelectCount == 1 )
                {
                    if( ItemType == guLIBRARY_ELEMENT_ARTISTS ||
                        ItemType == guLIBRARY_ELEMENT_ALBUMS )
                    {
                        MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_EDITLABELS,
                                                wxString( _( "Edit Labels" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITLABELS ),
                                                _( "Edit the labels assigned to the selected items" ) );
                        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
                        Menu.Append( MenuItem );
                    }
                }

                if( ContextMenuFlags & guCONTEXTMENU_EDIT_TRACKS )
                {
                    MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_EDITTRACKS,
                                        wxString( _( "Edit Songs" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                                        _( "Edit the selected tracks" ) );
                    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
                    Menu.Append( MenuItem );
                }

                Menu.AppendSeparator();

                MenuItem = new wxMenuItem( &Menu, ID_TREEVIEW_SAVETOPLAYLIST,
                                        wxString( _( "Save to Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                                        _( "Save the selected tracks to playlist" ) );
                MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
                Menu.Append( MenuItem );

                if( ( ContextMenuFlags & guCONTEXTMENU_COPY_TO ) ||
                    ( ( SelectCount == 1 ) && ContextMenuFlags & ( guCONTEXTMENU_LINKS | guCONTEXTMENU_COMMANDS ) ) )
                {

                    Menu.AppendSeparator();

                    if( ContextMenuFlags & guCONTEXTMENU_COPY_TO )
                    {
                        m_TreeViewPanel->CreateCopyToMenu( &Menu );
                    }

                    if( SelectCount == 1 )
                    {
                        if( ItemType == guLIBRARY_ELEMENT_ARTISTS ||
                            ItemType == guLIBRARY_ELEMENT_ALBUMARTISTS ||
                            ItemType == guLIBRARY_ELEMENT_ALBUMS )
                        {
                            if( ContextMenuFlags & guCONTEXTMENU_LINKS )
                            {
                                AddOnlineLinksMenu( &Menu );
                            }

                            if( ContextMenuFlags & guCONTEXTMENU_COMMANDS )
                            {
                                AddTreeViewCommands( &Menu, ItemType );
                            }
                        }
                    }
                }

                m_TreeViewPanel->CreateContextMenu( &Menu, ItemType );
            }
            else
            {
                return;
            }
        }
    }

    PopupMenu( &Menu, Point );

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::OnBeginDrag( wxTreeEvent &event )
{
    wxArrayTreeItemIds SelectedItems;
    if( GetSelections( SelectedItems ) )
    {
        guTrackArray Tracks;
        m_TreeViewPanel->GetAllTracks( &Tracks );
        int Index;
        int Count;
        if( ( Count = Tracks.Count() ) )
        {
            wxFileDataObject Files;

            for( Index = 0; Index < Count; Index++ )
            {
                Files.AddFile( Tracks[ Index ].m_FileName );
            }

            wxDropSource source( Files, this );

            wxDragResult Result = source.DoDragDrop();
            if( Result )
            {
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::GetItemFilterEntry( const wxTreeItemId &treeitemid, guTreeViewFilterEntry &filterentry )
{
    wxTreeItemId CurItemId = treeitemid;

    if( treeitemid == m_FiltersId || GetItemParent( treeitemid ) == m_FiltersId )
        return;

    while( CurItemId.IsOk() )
    {
        guTreeViewData * TreeViewData = ( guTreeViewData * ) GetItemData( CurItemId );
        if( TreeViewData )
        {
            filterentry.Add( new guTreeViewFilterItem( TreeViewData->m_Type, TreeViewData->m_Id ) );
        }
        else
        {
            return;
        }
        CurItemId = GetItemParent( CurItemId );
        if( IsSelected( CurItemId ) )
        {
            filterentry.Empty();
            return;
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guTreeViewTreeCtrl::IsFilterItem( const wxTreeItemId &treeitemid )
{
    wxTreeItemId CurItemId = treeitemid;
    while( CurItemId.IsOk() )
    {
        if( CurItemId == m_LibraryId || GetItemParent( CurItemId ) == m_LibraryId )
            return true;

        CurItemId = GetItemParent( CurItemId );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
wxArrayString GetSelectedAlbumPaths( guDbLibrary * db, const int id, const int type )
{
    wxArrayString RetVal;
    wxArrayInt SelectedItems;
    SelectedItems.Add( id );
    switch( type )
    {
        case guLIBRARY_ELEMENT_ARTISTS :
        {
            wxArrayInt AlbumList;
            db->GetArtistsAlbums( SelectedItems, &AlbumList );
            RetVal = db->GetAlbumsPaths( AlbumList );
            break;
        }

        case guLIBRARY_ELEMENT_ALBUMS :
        {
            RetVal = db->GetAlbumsPaths( SelectedItems );
            break;
        }

        case guLIBRARY_ELEMENT_ALBUMARTISTS :
        {
            wxArrayInt AlbumList;
            db->GetAlbumArtistsAlbums( SelectedItems, &AlbumList );
            RetVal = db->GetAlbumsPaths( AlbumList );
            break;
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int GetSelectedCoverId( guDbLibrary * db, const int id, const int type )
{
    int RetVal = 0;
    wxArrayInt SelectedItems;
    SelectedItems.Add( id );
    switch( type )
    {
        case guLIBRARY_ELEMENT_ARTISTS :
        {
            wxArrayInt AlbumList;
            db->GetArtistsAlbums( SelectedItems, &AlbumList );
            RetVal = db->GetAlbumCoverId( AlbumList[ 0 ] );
            break;
        }

        case guLIBRARY_ELEMENT_ALBUMS :
        {
            RetVal = db->GetAlbumCoverId( id );
            break;
        }

        case guLIBRARY_ELEMENT_ALBUMARTISTS :
        {
            wxArrayInt AlbumList;
            db->GetAlbumArtistsAlbums( SelectedItems, &AlbumList );
            RetVal = db->GetAlbumCoverId( AlbumList[ 0 ] );
            break;
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int GetSelectedTracks( guDbLibrary * db, guTrackArray * tracks, const int id, const int type )
{
    wxArrayInt SelectedItems;
    SelectedItems.Add( id );
    switch( type )
    {
        case guLIBRARY_ELEMENT_ARTISTS :
        {
            return db->GetArtistsSongs( SelectedItems, tracks );
        }

        case guLIBRARY_ELEMENT_ALBUMS :
        {
            return db->GetAlbumsSongs( SelectedItems, tracks );
        }

        case guLIBRARY_ELEMENT_ALBUMARTISTS :
        {
            return db->GetAlbumArtistsSongs( SelectedItems, tracks );
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
wxString GetAlbumSearchText( guDbLibrary * db, const int id, const int type )
{
    wxString Query;
    wxSQLite3ResultSet dbRes;

    wxString RetVal;
    Query = wxString::Format( wxT( "SELECT song_artist, song_album FROM songs WHERE song_albumid = %i LIMIT 1" ), id );

    dbRes = db->ExecuteQuery( Query );

    if( dbRes.NextRow() )
    {
        RetVal = wxString::Format( wxT( "\"%s\" \"%s\"" ),
            dbRes.GetString( 0 ).c_str(),
            dbRes.GetString( 1 ).c_str() );
    }
    dbRes.Finalize();
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::OnSearchLinkClicked( wxCommandEvent &event )
{
    const wxTreeItemId &CurItemId = GetFocusedItem();
    guTreeViewData * TreeViewData = ( guTreeViewData * ) GetItemData( CurItemId );
    int ItemType = TreeViewData->GetType();

    if( ItemType == guLIBRARY_ELEMENT_ALBUMS )
    {
        ExecuteOnlineLink( event.GetId(), GetAlbumSearchText( m_Db, TreeViewData->GetData(), TreeViewData->GetType() ) );
    }
    else
    {
        ExecuteOnlineLink( event.GetId(), GetItemText( CurItemId ) );
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewTreeCtrl::OnCommandClicked( wxCommandEvent &event )
{
    const wxTreeItemId &CurItemId = GetFocusedItem();
    guTreeViewData * TreeViewData = ( guTreeViewData * ) GetItemData( CurItemId );
//    int ItemType = TreeViewData->GetType();

    int Index;
    int Count;
    wxArrayInt Selection;
    Selection.Add( TreeViewData->GetData() );

    Index = event.GetId();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        wxArrayString Commands = Config->ReadAStr( CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, CONFIG_PATH_COMMANDS_EXECS );

        //guLogMessage( wxT( "CommandId: %u" ), Index );
        Index -= ID_COMMANDS_BASE;
        wxString CurCmd = Commands[ Index ];

        if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
        {
            wxArrayString AlbumPaths = GetSelectedAlbumPaths( m_Db, TreeViewData->GetData(), TreeViewData->GetType() );
            Count = AlbumPaths.Count();
            wxString Paths = wxEmptyString;
            for( Index = 0; Index < Count; Index++ )
            {
                AlbumPaths[ Index ].Replace( wxT( " " ), wxT( "\\ " ) );
                Paths += wxT( " " ) + AlbumPaths[ Index ];
            }
            CurCmd.Replace( guCOMMAND_ALBUMPATH, Paths.Trim( false ) );
        }

        if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
        {
            int CoverId = GetSelectedCoverId( m_Db, TreeViewData->GetData(), TreeViewData->GetType() );
            wxString CoverPath = wxEmptyString;
            if( CoverId > 0 )
            {
                CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
            }
            CurCmd.Replace( guCOMMAND_COVERPATH, CoverPath );
        }

        if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
        {
            guTrackArray Songs;
            wxString SongList = wxEmptyString;
            if( GetSelectedTracks( m_Db, &Songs, TreeViewData->GetData(), TreeViewData->GetType() ) )
            {
                Count = Songs.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    SongList += wxT( " \"" ) + Songs[ Index ].m_FileName + wxT( "\"" );
                }
                CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
            }
        }

        //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
        guExecute( CurCmd );
    }
}

// -------------------------------------------------------------------------------- //
int guTreeViewTreeCtrl::GetContextMenuFlags( void )
{
    return m_TreeViewPanel->GetContextMenuFlags();
}




// -------------------------------------------------------------------------------- //
// guTreeViewPanel
// -------------------------------------------------------------------------------- //
guTreeViewPanel::guTreeViewPanel( wxWindow * parent, guMediaViewer * mediaviewer ) :
            guAuiManagerPanel( parent ),
            //m_TextChangedTimer( this, guTREEVIEW_TIMER_TEXTSEARCH ),
            m_TreeItemSelectedTimer( this, guTREEVIEW_TIMER_TREEITEMSELECTED )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer->GetDb();
    m_PlayerPanel = mediaviewer->GetPlayerPanel();
    m_ConfigPath = m_MediaViewer->ConfigPath() + wxT( "/treeview" );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_VisiblePanels = Config->ReadNum( wxT( "VisiblePanels" ), guPANEL_PLAYLIST_VISIBLE_DEFAULT, m_ConfigPath );

    InitPanelData();

    CreateControls();
}

// -------------------------------------------------------------------------------- //
guTreeViewPanel::~guTreeViewPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( wxT( "VisiblePanels" ), m_VisiblePanels, m_ConfigPath );
    Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), m_ConfigPath );

}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::CreateControls( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    wxPanel * NamesPanel;
	NamesPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * NameSizer;
	NameSizer = new wxBoxSizer( wxVERTICAL );

	m_TreeViewCtrl = new guTreeViewTreeCtrl( NamesPanel, m_Db, this );
	//m_TreeViewCtrl->ExpandAll();
	NameSizer->Add( m_TreeViewCtrl, 1, wxEXPAND, 5 );

	NamesPanel->SetSizer( NameSizer );
	NamesPanel->Layout();
	NameSizer->Fit( NamesPanel );

    m_AuiManager.AddPane( NamesPanel,
            wxAuiPaneInfo().Name( wxT( "TreeViewFilters" ) ).Caption( _( "Tree" ) ).
            MinSize( 50, 50 ).CloseButton( false ).
            Dockable( true ).Left() );


    wxPanel *  DetailsPanel;
	DetailsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * DetailsSizer;
	DetailsSizer = new wxBoxSizer( wxVERTICAL );

	m_TVTracksListBox = new guTVSoListBox( DetailsPanel, m_MediaViewer, m_ConfigPath, guLISTVIEW_COLUMN_SELECT|guLISTVIEW_COLUMN_SORTING );
	DetailsSizer->Add( m_TVTracksListBox, 1, wxEXPAND, 5 );

	DetailsPanel->SetSizer( DetailsSizer );
	DetailsPanel->Layout();
	DetailsSizer->Fit( DetailsPanel );

    m_AuiManager.AddPane( DetailsPanel, wxAuiPaneInfo().Name( wxT( "TreeViewTracks" ) ).Caption( wxT( "TreeView" ) ).
            MinSize( 50, 50 ).
            CenterPane() );

    wxString TreeViewLayout = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, m_ConfigPath );
    if( Config->GetIgnoreLayouts() || TreeViewLayout.IsEmpty() )
    {
        m_VisiblePanels = guPANEL_TREEVIEW_VISIBLE_DEFAULT;
        TreeViewLayout = wxT( "layout2|name=TreeViewFilters;caption=" ) + wxString( _( "Tree" ) );
        TreeViewLayout += wxT( ";state=2044;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=180;besth=350;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        TreeViewLayout += wxT( "name=TreeViewTracks;caption=PlayList;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=50;besth=50;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        TreeViewLayout += wxT( "dock_size(1,2,0)=47|dock_size(4,0,0)=186|dock_size(5,0,0)=52|" );
    }

    m_AuiManager.LoadPerspective( TreeViewLayout, true );


    Bind( wxEVT_TREE_SEL_CHANGED, &guTreeViewPanel::OnTreeViewSelected, this );
    Bind( wxEVT_TREE_ITEM_ACTIVATED, &guTreeViewPanel::OnTreeViewActivated, this );

    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewNewFilter, this, ID_TREEVIEW_FILTER_NEW );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewEditFilter, this, ID_TREEVIEW_FILTER_EDIT );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewDeleteFilter, this, ID_TREEVIEW_FILTER_DELETE );

    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewPlay, this, ID_TREEVIEW_PLAY );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewEnqueue, this, ID_TREEVIEW_ENQUEUE_AFTER_ALL, ID_TREEVIEW_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewEditLabels, this, ID_TREEVIEW_EDITLABELS );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewEditTracks, this, ID_TREEVIEW_EDITTRACKS );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewSaveToPlayList, this, ID_TREEVIEW_SAVETOPLAYLIST );
    m_TreeViewCtrl->Bind( wxEVT_MENU, &guTreeViewPanel::OnTreeViewCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    m_TVTracksListBox->Bind( wxEVT_LIST_COL_CLICK, &guTreeViewPanel::OnTrackListColClicked, this );

    m_TVTracksListBox->Bind( wxEVT_LISTBOX_DCLICK, &guTreeViewPanel::OnTVTracksActivated, this );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksDeleteLibrary, this, ID_TRACKS_DELETE_LIBRARY );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksDeleteDrive, this, ID_TRACKS_DELETE_DRIVE );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksPlayClicked, this, ID_TRACKS_PLAY );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSetRating, this, ID_TRACKS_ENQUEUE_AFTER_ALL, ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksEditLabelsClicked, this, ID_TRACKS_EDITLABELS );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksEditTracksClicked, this, ID_TRACKS_EDITTRACKS );
    m_TVTracksListBox->Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSavePlayListClicked, this, ID_TRACKS_SAVETOPLAYLIST );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSetRating, this, ID_TRACKS_SET_RATING_0, ID_TRACKS_SET_RATING_5 );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSetField, this, ID_TRACKS_SET_COLUMN );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksEditField, this, ID_TRACKS_EDIT_COLUMN );

    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSelectGenre, this, ID_TRACKS_BROWSE_GENRE );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSelectArtist, this, ID_TRACKS_BROWSE_ARTIST );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSelectAlbumArtist, this, ID_TRACKS_BROWSE_ALBUMARTIST );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSelectComposer, this, ID_TRACKS_BROWSE_COMPOSER );
    Bind( wxEVT_MENU, &guTreeViewPanel::OnTVTracksSelectAlbum, this, ID_TRACKS_BROWSE_ALBUM );

    Bind( wxEVT_TIMER, &guTreeViewPanel::OnTreeItemSelectedTimer , this, guTREEVIEW_TIMER_TREEITEMSELECTED );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::InitPanelData()
{
}

// -------------------------------------------------------------------------------- //
int guTreeViewPanel::GetContextMenuFlags( void )
{
    return m_MediaViewer->GetContextMenuFlags();
}

// -------------------------------------------------------------------------------- //
bool guTreeViewPanel::DoTextSearch( const wxString &textsearch )
{
    if( m_LastSearchString != textsearch )
    {
        m_LastSearchString = textsearch; //m_InputTextCtrl->GetValue();
        if( !m_LastSearchString.IsEmpty() )
        {
            if( m_LastSearchString.Length() > 0 )
            {
                wxArrayString TextFilters = guSplitWords( m_LastSearchString );
                m_TreeViewCtrl->SetTextFilters( TextFilters );
    //            m_TreeViewCtrl->ExpandAll();
                m_TreeViewCtrl->ReloadItems();
                m_TVTracksListBox->SetTextFilters( TextFilters );
                m_TVTracksListBox->ReloadItems( false );
            }
    //        m_InputTextCtrl->ShowCancelButton( true );
            return true;
        }
        else
        {
            m_TreeViewCtrl->ClearTextFilters();
    //        m_TreeViewCtrl->ExpandAll();
            m_TreeViewCtrl->ReloadItems();

            m_TVTracksListBox->ClearTextFilters();
            m_TVTracksListBox->ReloadItems( false );

    //        m_InputTextCtrl->ShowCancelButton( false );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeItemSelectedTimer( wxTimerEvent &event )
{
    guTreeViewFilterArray FilterArray;

    wxArrayTreeItemIds Selections;
    if( m_TreeViewCtrl->GetSelections( Selections ) )
    {
        int Index;
        int Count = Selections.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guTreeViewFilterEntry FilterEntry;
            m_TreeViewCtrl->GetItemFilterEntry( Selections[ Index ], FilterEntry );
            if( FilterEntry.Count() )
            {
                FilterArray.Add( FilterEntry );
            }
        }
    }
    m_TVTracksListBox->SetFilters( FilterArray );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewSelected( wxTreeEvent& event )
{
    if( m_TreeItemSelectedTimer.IsRunning() )
        m_TreeItemSelectedTimer.Stop();

    m_TreeItemSelectedTimer.Start( guTREEVIEW_TIMER_TREEITEMSELECTED_VALUE, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewActivated( wxTreeEvent& event )
{
    const wxTreeItemId &CurItemId = event.GetItem();
    if( m_TreeViewCtrl->IsFilterEntry( CurItemId ) )
    {
        guTreeViewData * ItemData = ( guTreeViewData * ) m_TreeViewCtrl->GetItemData( CurItemId );
        if( ItemData )
        {
            m_TreeViewCtrl->SetCurrentFilter( ItemData->GetData() );
            m_TreeViewCtrl->ReloadItems();
            return;
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewPlay( wxCommandEvent &event )
{
    if( m_TreeViewCtrl->IsFilterItem( m_TreeViewCtrl->GetFocusedItem() ) )
    {
        guTrackArray Tracks;
        m_TVTracksListBox->GetAllSongs( &Tracks );
        if( Tracks.Count() )
        {
            NormalizeTracks( &Tracks );
            m_PlayerPanel->SetPlayList( Tracks );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewEnqueue( wxCommandEvent &event )
{
    if( m_TreeViewCtrl->IsFilterItem( m_TreeViewCtrl->GetFocusedItem() ) )
    {
        guTrackArray Tracks;
        m_TVTracksListBox->GetAllSongs( &Tracks );
        if( Tracks.Count() )
        {
            NormalizeTracks( &Tracks );
            m_PlayerPanel->AddToPlayList( Tracks, true, event.GetId() - ID_TREEVIEW_ENQUEUE_AFTER_ALL );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewEditLabels( wxCommandEvent &event )
{
    const wxTreeItemId &CurItemId = m_TreeViewCtrl->GetFocusedItem();
    if( m_TreeViewCtrl->IsFilterItem( CurItemId ) )
    {
        guTreeViewData * ItemData = ( guTreeViewData * ) m_TreeViewCtrl->GetItemData( CurItemId );
        if( ItemData )
        {
            if( ( ItemData->GetType() == guLIBRARY_ELEMENT_ARTISTS ) ||
                ( ItemData->GetType() == guLIBRARY_ELEMENT_ALBUMS ) )
            {
                guArrayListItems LabelSets;
                wxArrayInt IdList;
                IdList.Add( ItemData->GetData() );
                int ItemType = ItemData->GetType();
                if( ItemType == guLIBRARY_ELEMENT_ARTISTS )
                {
                    LabelSets = m_Db->GetArtistsLabels( IdList );
                }
                else if( ItemType == guLIBRARY_ELEMENT_ALBUMS )
                {
                    LabelSets = m_Db->GetAlbumsLabels( IdList );
                }

                guListItems ListItems;
                ListItems.Add( new guListItem( ItemData->GetData(), m_TreeViewCtrl->GetItemText( CurItemId ) ) );

                guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db,
                                                                ItemType == guLIBRARY_ELEMENT_ARTISTS ? _( "Artist Labels Editor" ) : _( "Albums Labels Editor" ),
                                                                false, &ListItems, &LabelSets );
                if( LabelEditor )
                {
                    if( LabelEditor->ShowModal() == wxID_OK )
                    {
                        // Update the labels in the artists files
                        if( ItemType == guLIBRARY_ELEMENT_ARTISTS )
                        {
                            m_Db->UpdateArtistsLabels( LabelSets );
                        }
                        else
                        {
                            m_Db->UpdateAlbumsLabels( LabelSets );
                        }
                    }

                    LabelEditor->Destroy();

                    wxCommandEvent event( wxEVT_MENU, ID_LABEL_UPDATELABELS );
                    wxPostEvent( m_MediaViewer, event );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewEditTracks( wxCommandEvent &event )
{
    const wxTreeItemId &CurItemId = m_TreeViewCtrl->GetFocusedItem();
    if( CurItemId.IsOk() && m_TreeViewCtrl->IsFilterItem( CurItemId ) )
    {
        guTrackArray Tracks;
        m_TVTracksListBox->GetAllSongs( &Tracks );
        if( Tracks.Count() )
        {
            NormalizeTracks( &Tracks );


            guImagePtrArray Images;
            wxArrayString Lyrics;
            wxArrayInt ChangedFlags;

            guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics, &ChangedFlags );
            if( TrackEditor )
            {
                if( TrackEditor->ShowModal() == wxID_OK )
                {
                    //guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
                    //m_Db->UpdateSongs( &Tracks, ChangedFlags );
                    m_MediaViewer->UpdateTracks( Tracks, Images, Lyrics, ChangedFlags );

                    m_TVTracksListBox->ReloadItems();

                    // Update the track in database, playlist, etc
                    m_MediaViewer->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
                }

                guImagePtrArrayClean( &Images );

                TrackEditor->Destroy();
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewSaveToPlayList( wxCommandEvent &event )
{
    const wxTreeItemId &CurItemId = m_TreeViewCtrl->GetFocusedItem();
    if( m_TreeViewCtrl->IsFilterItem( CurItemId ) )
    {
        guTrackArray Tracks;
        m_TVTracksListBox->GetAllSongs( &Tracks );
        if( Tracks.Count() )
        {
            wxArrayInt NewSongs;
            //NormalizeTracks( &Tracks );
            int Index;
            int Count = Tracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                NewSongs.Add( Tracks[ Index ].m_SongId );
            }

            guListItems PlayLists;
            m_Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );

            guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( guMainFrame::GetMainFrame(), m_Db, &NewSongs, &PlayLists );

            if( PlayListAppendDlg->ShowModal() == wxID_OK )
            {
                int Selected = PlayListAppendDlg->GetSelectedPlayList();
                if( Selected == -1 )
                {
                    wxString PLName = PlayListAppendDlg->GetPlaylistName();
                    if( PLName.IsEmpty() )
                    {
                        PLName = _( "UnNamed" );
                    }
                    int PLId = m_Db->CreateStaticPlayList( PLName, NewSongs );
                    m_Db->UpdateStaticPlayListFile( PLId );
                }
                else
                {
                    int PLId = PlayLists[ Selected ].m_Id;
                    wxArrayInt OldSongs;
                    m_Db->GetPlayListSongIds( PLId, &OldSongs );
                    if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                    {
                        m_Db->UpdateStaticPlayList( PLId, NewSongs );
                        m_Db->AppendStaticPlayList( PLId, OldSongs );
                    }
                    else                                                // END
                    {
                        m_Db->AppendStaticPlayList( PLId, NewSongs );
                    }
                    m_Db->UpdateStaticPlayListFile( PLId );
                }
                SendPlayListUpdatedEvent();
            }
            PlayListAppendDlg->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewNewFilter( wxCommandEvent &event )
{
    guTreeViewFilterEditor * FilterEditor = new guTreeViewFilterEditor( this, wxString( _( "New Filter" ) ) + wxT( ":" ) );
    if( FilterEditor )
    {
        if( FilterEditor->ShowModal() == wxID_OK )
        {
            m_TreeViewCtrl->AppendFilterEntry( FilterEditor->GetTreeViewFilterEntry() );
        }
        FilterEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewEditFilter( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_TreeViewCtrl->GetFocusedItem();
    if( ItemId.IsOk() && m_TreeViewCtrl->IsFilterEntry( ItemId ) )
    {
        guTreeViewData * ItemData = ( guTreeViewData * ) m_TreeViewCtrl->GetItemData( ItemId );
        if( ItemData )
        {
            guTreeViewFilterEditor * FilterEditor = new guTreeViewFilterEditor( this, m_TreeViewCtrl->GetFilterEntry( ItemData->GetData() ) );
            if( FilterEditor )
            {
                if( FilterEditor->ShowModal() == wxID_OK )
                {
                    m_TreeViewCtrl->SetFilterEntry( ItemData->GetData(), FilterEditor->GetTreeViewFilterEntry() );
                }
                FilterEditor->Destroy();
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewDeleteFilter( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_TreeViewCtrl->GetFocusedItem();
    if( ItemId.IsOk() && m_TreeViewCtrl->IsFilterEntry( ItemId ) )
    {
        guTreeViewData * ItemData = ( guTreeViewData * ) m_TreeViewCtrl->GetItemData( ItemId );
        if( ItemData )
        {
            if( wxMessageBox( _( "Are you sure to delete the selected filter?" ),
                              _( "Confirm" ),
                              wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT, this ) == wxYES )
            {
                m_TreeViewCtrl->DeleteFilterEntry( ItemData->GetData() );
            }
        }
    }

}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTreeViewCopyTo( wxCommandEvent &event )
{
    const wxTreeItemId &ItemId = m_TreeViewCtrl->GetFocusedItem();
    if( ItemId.IsOk() && m_TreeViewCtrl->IsFilterItem( ItemId ) )
    {
        guTrackArray * Tracks = new guTrackArray();
        m_TVTracksListBox->GetAllSongs( Tracks );
        NormalizeTracks( Tracks );

        int Index = event.GetId() - ID_COPYTO_BASE;
        if( Index >= guCOPYTO_DEVICE_BASE )
        {
            Index -= guCOPYTO_DEVICE_BASE;
            event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
        }
        else
        {
            event.SetId( ID_MAINFRAME_COPYTO );
        }
        event.SetInt( Index );
        event.SetClientData( ( void * ) Tracks );
        wxPostEvent( guMainFrame::GetMainFrame(), event );
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksActivated( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    if( Tracks.Count() )
    {
        NormalizeTracks( &Tracks );

        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false, CONFIG_PATH_GENERAL ) )
            {
                m_PlayerPanel->AddToPlayList( Tracks );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Tracks );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksPlayClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    if( !Tracks.Count() )
        m_TVTracksListBox->GetAllSongs( &Tracks );
    NormalizeTracks( &Tracks );
    m_PlayerPanel->SetPlayList( Tracks );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksQueueClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    if( !Tracks.Count() )
        m_TVTracksListBox->GetAllSongs( &Tracks );
    NormalizeTracks( &Tracks );
    m_PlayerPanel->AddToPlayList( Tracks, true, event.GetId() - ID_TRACKS_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksEditLabelsClicked( wxCommandEvent &event )
{
    guListItems Tracks;
    m_TVTracksListBox->GetSelectedItems( &Tracks );
    if( Tracks.Count() )
    {
        guArrayListItems LabelSets = m_Db->GetSongsLabels( m_TVTracksListBox->GetSelectedItems() );

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Tracks Labels Editor" ), false, &Tracks, &LabelSets );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                // Update the labels in the files
                m_Db->UpdateSongsLabels( LabelSets );
            }
            LabelEditor->Destroy();
            m_TVTracksListBox->ReloadItems( false );
        }
    }

}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    if( !Tracks.Count() )
        return;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics, &ChangedFlags );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            //guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
            //m_Db->UpdateSongs( &Tracks, ChangedFlags );
            m_MediaViewer->UpdateTracks( Tracks, Images, Lyrics, ChangedFlags );

            m_TVTracksListBox->ReloadItems();

            // Update the track in database, playlist, etc
            m_MediaViewer->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        guImagePtrArrayClean( &Images );
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_TVTracksListBox->GetSelectedSongs( Tracks );

    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
    {
        event.SetId( ID_MAINFRAME_COPYTO );
    }
    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( guMainFrame::GetMainFrame(), event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSavePlayListClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt NewSongs;
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );

    if( ( count = Tracks.Count() ) )
    {
        for( index = 0; index < count; index++ )
        {
            NewSongs.Add( Tracks[ index ].m_SongId );
        }
    }
    else
    {
        m_TVTracksListBox->GetAllSongs( &Tracks );
        count = Tracks.Count();
        for( index = 0; index < count; index++ )
        {
            NewSongs.Add( Tracks[ index ].m_SongId );
        }
    }

    if( NewSongs.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );

        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( guMainFrame::GetMainFrame(), m_Db, &NewSongs, &PlayLists );

        if( PlayListAppendDlg->ShowModal() == wxID_OK )
        {
            int Selected = PlayListAppendDlg->GetSelectedPlayList();
            if( Selected == -1 )
            {
                wxString PLName = PlayListAppendDlg->GetPlaylistName();
                if( PLName.IsEmpty() )
                {
                    PLName = _( "UnNamed" );
                }
                int PLId = m_Db->CreateStaticPlayList( PLName, NewSongs );
                m_Db->UpdateStaticPlayListFile( PLId );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, NewSongs );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                {
                    m_Db->AppendStaticPlayList( PLId, NewSongs );
                }
                m_Db->UpdateStaticPlayListFile( PLId );
            }
            SendPlayListUpdatedEvent();
        }
        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSetRating( wxCommandEvent &event )
{
    int Rating = event.GetId() - ID_TRACKS_SET_RATING_0;
    //guLogMessage( wxT( "OnSongSetRating( %i )" ), Rating );

    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );

    m_MediaViewer->SetTracksRating( Tracks, Rating );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSetField( wxCommandEvent &event )
{
    int ColumnId = m_TVTracksListBox->GetColumnId( m_TVTracksListBox->GetLastColumnClicked() );
    //guLogMessage( wxT( "guTreeViewPanel::OnSongSetField %i" ), ColumnId );

    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );

    wxVariant NewData = m_TVTracksListBox->GetLastDataClicked();

    //guLogMessage( wxT( "Setting Data to : %s" ), NewData.GetString().c_str() );

    // This should be done in a thread for huge selections of tracks...
    wxArrayInt ChangedFlags;
    int Index;
    int Count = Tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        ChangedFlags.Add( guTRACK_CHANGED_DATA_TAGS );
        guTrack * Track = &Tracks[ Index ];
        switch( ColumnId )
        {
            case guSONGS_COLUMN_NUMBER :
                Track->m_Number = NewData.GetLong();
                break;

            case guSONGS_COLUMN_TITLE :
                Track->m_SongName = NewData.GetString();
                break;

            case guSONGS_COLUMN_ARTIST :
                Track->m_ArtistName = NewData.GetString();
                break;

            case guSONGS_COLUMN_ALBUMARTIST :
                Track->m_AlbumArtist = NewData.GetString();
                break;

            case guSONGS_COLUMN_ALBUM :
                Track->m_AlbumName = NewData.GetString();
                break;

            case guSONGS_COLUMN_GENRE :
                Track->m_GenreName = NewData.GetString();
                break;

            case guSONGS_COLUMN_COMPOSER :
                Track->m_Composer = NewData.GetString();
                break;

            case guSONGS_COLUMN_DISK :
                Track->m_Disk = NewData.GetString();
                break;

            case guSONGS_COLUMN_YEAR :
                Track->m_Year = NewData.GetLong();
                break;

        }
    }

    m_Db->UpdateSongs( &Tracks, ChangedFlags );

    m_MediaViewer->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksEditField( wxCommandEvent &event )
{
    int ColumnId = m_TVTracksListBox->GetColumnId( m_TVTracksListBox->GetLastColumnClicked() );
    //guLogMessage( wxT( "guLibPanel::OnSongSetField %i" ), ColumnId );

    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );

    wxString Label = m_TVTracksListBox->GetColumnNames()[ ColumnId ];
    wxVariant DefValue = m_TVTracksListBox->GetLastDataClicked();

    wxArrayString Items;

    int Index;
    int Count = Tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        wxVariant Value;
        guTrack * Track = &Tracks[ Index ];

        switch( ColumnId )
        {
            case guSONGS_COLUMN_NUMBER :
                Value = ( long ) Track->m_Number;
                break;

            case guSONGS_COLUMN_TITLE :
                Value = Track->m_SongName;
                break;

            case guSONGS_COLUMN_ARTIST :
                Value = Track->m_ArtistName;
                break;

            case guSONGS_COLUMN_ALBUMARTIST :
                Value = Track->m_AlbumArtist;
                break;

            case guSONGS_COLUMN_ALBUM :
                Value = Track->m_AlbumName;
                break;

            case guSONGS_COLUMN_GENRE :
                Value = Track->m_GenreName;
                break;

            case guSONGS_COLUMN_COMPOSER :
                Value = Track->m_Composer;
                break;

            case guSONGS_COLUMN_DISK :
                Value = Track->m_Disk;
                break;

            case guSONGS_COLUMN_YEAR :
                Value = ( long ) Track->m_Year;
                break;
        }
        if( Items.Index( Value.GetString() ) == wxNOT_FOUND )
            Items.Add( Value.GetString() );
    }

    guEditWithOptions * FieldEditor = new guEditWithOptions( this, _( "Field Editor" ), Label, DefValue.GetString(), Items );

    if( FieldEditor )
    {
        if( FieldEditor->ShowModal() == wxID_OK )
        {
            DefValue = FieldEditor->GetData();

            //guLogMessage( wxT( "Setting Data to : %s" ), DefValue.GetString().c_str() );

            // This should be done in a thread for huge selections of tracks...
            wxArrayInt ChangedFlags;
            int Index;
            int Count = Tracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                ChangedFlags.Add( guTRACK_CHANGED_DATA_TAGS );
                guTrack * Track = &Tracks[ Index ];
                switch( ColumnId )
                {
                    case guSONGS_COLUMN_NUMBER :
                        Track->m_Number = DefValue.GetLong();
                        break;

                    case guSONGS_COLUMN_TITLE :
                        Track->m_SongName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_ARTIST :
                        Track->m_ArtistName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_ALBUMARTIST :
                        Track->m_AlbumArtist = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_ALBUM :
                        Track->m_AlbumName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_GENRE :
                        Track->m_GenreName = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_COMPOSER :
                        Track->m_Composer = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_DISK :
                        Track->m_Disk = DefValue.GetString();
                        break;

                    case guSONGS_COLUMN_YEAR :
                        Track->m_Year = DefValue.GetLong();
                        break;
                }
            }

            m_Db->UpdateSongs( &Tracks, ChangedFlags );

            m_MediaViewer->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
        }
        FieldEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSelectGenre( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    wxArrayInt * Genres = new wxArrayInt();
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Genres->Index( Tracks[ index ].m_GenreId ) == wxNOT_FOUND )
        {
            Genres->Add( Tracks[ index ].m_GenreId );
        }
    }

    event.SetId( ID_GENRE_SETSELECTION );
    event.SetClientData( ( void * ) Genres );
    wxPostEvent( m_MediaViewer, event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSelectArtist( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    wxArrayInt * Artists = new wxArrayInt();
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Artists->Index( Tracks[ index ].m_ArtistId ) == wxNOT_FOUND )
        {
            Artists->Add( Tracks[ index ].m_ArtistId );
        }
    }
    event.SetId( ID_ARTIST_SETSELECTION );
    event.SetClientData( ( void * ) Artists );
    wxPostEvent( m_MediaViewer, event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSelectAlbumArtist( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    wxArrayInt * Ids = new wxArrayInt();
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Ids->Index( Tracks[ index ].m_AlbumArtistId ) == wxNOT_FOUND )
        {
            Ids->Add( Tracks[ index ].m_AlbumArtistId );
        }
    }
    event.SetId( ID_ALBUMARTIST_SETSELECTION );
    event.SetClientData( ( void * ) Ids );
    wxPostEvent( m_MediaViewer, event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSelectComposer( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    wxArrayInt * Ids = new wxArrayInt();
    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Ids->Index( Tracks[ index ].m_ComposerId ) == wxNOT_FOUND )
        {
            Ids->Add( Tracks[ index ].m_ComposerId );
        }
    }
    event.SetId( ID_COMPOSER_SETSELECTION );
    event.SetClientData( ( void * ) Ids );
    wxPostEvent( m_MediaViewer, event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksSelectAlbum( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );
    wxArrayInt * Albums = new wxArrayInt();

    int index;
    int count = Tracks.Count();
    for( index = 0; index < count; index++ )
    {
        if( Albums->Index( Tracks[ index ].m_AlbumId ) == wxNOT_FOUND )
        {
            Albums->Add( Tracks[ index ].m_AlbumId );
        }
    }
    event.SetId( ID_ALBUM_SETSELECTION );
    event.SetClientData( ( void * ) Albums );
    wxPostEvent( m_MediaViewer, event );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksDeleteLibrary( wxCommandEvent &event )
{
    if( m_TVTracksListBox->GetSelectedCount() )
    {
        if( wxMessageBox( wxT( "Are you sure to remove the selected tracks from your library?" ),
            wxT( "Remove tracks from library" ), wxICON_QUESTION|wxYES|wxNO|wxNO_DEFAULT ) == wxYES )
        {
            guTrackArray Tracks;
            m_TVTracksListBox->GetSelectedSongs( &Tracks );
            //
            m_Db->DeleteLibraryTracks( &Tracks, true );

            m_TVTracksListBox->ReloadItems();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTVTracksDeleteDrive( wxCommandEvent &event )
{
    if( m_TVTracksListBox->GetSelectedCount() )
    {
        if( wxMessageBox( wxT( "Are you sure to delete the selected tracks from your drive?\nThis will permanently erase the selected tracks." ),
            wxT( "Remove tracks from drive" ), wxICON_QUESTION|wxYES|wxNO|wxNO_DEFAULT ) == wxYES )
        {
            guTrackArray Tracks;
            m_TVTracksListBox->GetSelectedSongs( &Tracks );
            //
            m_Db->DeleteLibraryTracks( &Tracks, false );
            //
            int Index;
            int Count = Tracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( !wxRemoveFile( Tracks[ Index ].m_FileName ) )
                {
                    guLogMessage( wxT( "Error deleting '%s'" ), Tracks[ Index ].m_FileName.c_str() );
                }
            }

            m_TVTracksListBox->ReloadItems();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::SendPlayListUpdatedEvent( void )
{
//    wxCommandEvent evt( wxEVT_MENU, ID_PLAYLIST_UPDATED );
//    wxPostEvent( guMainFrame::GetMainFrame(), evt );
    m_MediaViewer->UpdatePlaylists();
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnGoToSearch( wxCommandEvent &event )
{
    m_MediaViewer->GoToSearch();
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::OnTrackListColClicked( wxListEvent &event )
{
    guLogMessage( wxT( "OnTrackListColClicked..." ) );
    int ColId = m_TVTracksListBox->GetColumnId( event.m_col );

    m_TVTracksListBox->SetTracksOrder( ColId );

    // Create the Columns
    wxArrayString ColumnNames = m_TVTracksListBox->GetColumnNames();
    int CurColId;
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        CurColId = m_TVTracksListBox->GetColumnId( index );
        m_TVTracksListBox->SetColumnLabel( index,
            ColumnNames[ CurColId ]  + ( ( ColId == CurColId ) ? ( m_TVTracksListBox->GetTracksOrderDesc() ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    m_TVTracksListBox->ReloadItems( false );
}


// -------------------------------------------------------------------------------- //
void guTreeViewPanel::RefreshAll( void )
{
    m_TreeViewCtrl->ReloadItems();

    m_TVTracksListBox->ReloadItems( false );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    m_MediaViewer->NormalizeTracks( tracks, isdrag );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::CreateCopyToMenu( wxMenu * menu )
{
    m_MediaViewer->CreateCopyToMenu( menu );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::CreateContextMenu( wxMenu * menu, const int windowid )
{
    m_MediaViewer->CreateContextMenu( menu, windowid );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::UpdatedTracks( const guTrackArray * tracks )
{
    m_TreeViewCtrl->ReloadItems();

    m_TVTracksListBox->UpdatedTracks( tracks );
}

// -------------------------------------------------------------------------------- //
void guTreeViewPanel::UpdatedTrack( const guTrack * track )
{
    m_TreeViewCtrl->ReloadItems();

    m_TVTracksListBox->UpdatedTrack( track );
}

}

// -------------------------------------------------------------------------------- //
