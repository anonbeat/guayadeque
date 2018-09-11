// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
#include "LocationPanel.h"

#include "Accelerators.h"
#include "Config.h"
#include "Images.h"

#include <wx/tokenzr.h>
#include <wx/artprov.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guLocationItemData
// -------------------------------------------------------------------------------- //
class guLocationItemData : public wxTreeItemData
{
  private :
    int         m_Id;
    bool        m_IsOpen;
    wxString    m_CollectionId;
    int         m_CollectionType;
    bool        m_IsEnabled;

  public :
    guLocationItemData( const int id, const bool open, const wxString &uniqueid = wxEmptyString,
                        const int type = wxNOT_FOUND, const bool isenabled = true )
    {
        m_Id = id;
        m_IsOpen = open;
        m_CollectionId = uniqueid;
        m_CollectionType = type;
        m_IsEnabled = isenabled;
    }

    int         GetId( void ) { return m_Id; }
    void        SetId( int id ) { m_Id = id; }
    int         GetOpen( void ) { return m_IsOpen; }
    void        SetOpen( const bool open ) { m_IsOpen = open; }
    wxString    GetCollectionId( void ) { return m_CollectionId; }
    void        SetCollectionId( const wxString &uniqueid ) { m_CollectionId = uniqueid; }
    int         GetCollectionType( void ) { return m_CollectionType; }
    void        SetCollectionType( const int type ) { m_CollectionType = type; }
    bool        GetIsEnabled( void ) { return m_IsEnabled; }
    void        SetIsEnabled( const bool enabled ) { m_IsEnabled = enabled; }
};

#define guLOCATION_PANEL_IMAGE_COUNT    9

// -------------------------------------------------------------------------------- //
// guLocationTreeCtrl
// -------------------------------------------------------------------------------- //
guLocationTreeCtrl::guLocationTreeCtrl( wxWindow * parent, guMainFrame * mainframe ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_SINGLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT|wxNO_BORDER )
{
    m_MainFrame = mainframe;
    m_LockCount = 0;

    m_ImageList = new wxImageList();
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_library ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_portable_device ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_net_radio ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_podcast ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_magnatune ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_jamendo ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_lastfm ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_tiny_shoutcast ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_loc_lyrics ) );

    AssignImageList( m_ImageList );

    m_RootId   = AddRoot( wxT( "Sources" ), -1, -1, NULL );

    wxFont FontBold = GetFont();
    FontBold.SetWeight( wxFONTWEIGHT_BOLD );

    m_LocalMusicId = AppendItem( m_RootId, _( "Local Music" ), 0 );

    m_OnlineMusicId = AppendItem( m_RootId, _( "Online Music" ), 2 );

    m_PortableDeviceId = AppendItem( m_RootId, _( "Portable Devices" ), 1 );

    m_ContextId = AppendItem( m_RootId, _( "Context" ), -1 );

    SetIndent( 5 );

    Bind( wxEVT_TREE_ITEM_MENU, &guLocationTreeCtrl::OnContextMenu, this );

    ReloadItems( true );
}

// -------------------------------------------------------------------------------- //
guLocationTreeCtrl::~guLocationTreeCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( CONFIG_KEY_MAIN_SOURCES_LOCAL_MUSIC, IsExpanded( m_LocalMusicId ), CONFIG_PATH_MAIN_SOURCES );
    Config->WriteBool( CONFIG_KEY_MAIN_SOURCES_ONLINE_MUSIC, IsExpanded( m_OnlineMusicId ), CONFIG_PATH_MAIN_SOURCES );
    Config->WriteBool( CONFIG_KEY_MAIN_SOURCES_PORTABLE_DEVICES, IsExpanded( m_PortableDeviceId ), CONFIG_PATH_MAIN_SOURCES );
    Config->WriteBool( CONFIG_KEY_MAIN_SOURCES_CONTEXT, IsExpanded( m_ContextId ), CONFIG_PATH_MAIN_SOURCES );

    Unbind( wxEVT_TREE_ITEM_MENU, &guLocationTreeCtrl::OnContextMenu, this );
}

// -------------------------------------------------------------------------------- //
int guLocationTreeCtrl::GetIconIndex( const wxString &iconstring )
{
    if( !iconstring.IsEmpty() )
    {
        //. GThemedIcon drive-removable-media-usb drive-removable-media drive-removable drive
        wxArrayString IconNames = wxStringTokenize( iconstring, wxT( " " ) );
        int Count = IconNames.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            //guLogMessage( wxT( "Trying to load the icon '%s'" ), IconNames[ Index ].c_str() );
            if( IconNames[ Index ] == wxT( "." ) || IconNames[ Index ] == wxT( "GThemedIcon" ) )
                continue;

            int IconPos = m_IconNames.Index( IconNames[ Index ] );
            if( IconPos == wxNOT_FOUND )
            {
                wxBitmap IconBitmap = wxArtProvider::GetBitmap( IconNames[ Index ], wxART_OTHER, wxSize( 24, 24 ) );
                if( IconBitmap.IsOk() )
                {
                    //guLogMessage( wxT( "The Icon was found...") );
                    IconPos = m_IconNames.Count();
                    m_IconNames.Add( IconNames[ Index ] );
                    m_ImageList->Add( IconBitmap );
                    return guLOCATION_PANEL_IMAGE_COUNT + IconPos;
                }
            }
            else
            {
                return guLOCATION_PANEL_IMAGE_COUNT + IconPos;
            }
        }
    }

    return 1;
}

// -------------------------------------------------------------------------------- //
void guLocationTreeCtrl::ReloadItems( const bool loadstate )
{
    if( m_LockCount )
        return;

    wxTreeItemId CurrentItem;
    int VisiblePanels = m_MainFrame->VisiblePanels();

    bool LocalMusicExpanded;
    bool OnlineMusicExpanded;
    bool PortableDeviceExpanded;
    bool ContextExpanded;

    if( loadstate )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        LocalMusicExpanded = Config->ReadBool( CONFIG_KEY_MAIN_SOURCES_LOCAL_MUSIC, true, CONFIG_PATH_MAIN_SOURCES );
        OnlineMusicExpanded = Config->ReadBool( CONFIG_KEY_MAIN_SOURCES_ONLINE_MUSIC, true, CONFIG_PATH_MAIN_SOURCES );
        PortableDeviceExpanded = Config->ReadBool( CONFIG_KEY_MAIN_SOURCES_PORTABLE_DEVICES, true, CONFIG_PATH_MAIN_SOURCES );
        ContextExpanded = Config->ReadBool( CONFIG_KEY_MAIN_SOURCES_CONTEXT, true, CONFIG_PATH_MAIN_SOURCES );
    }
    else
    {
        LocalMusicExpanded = IsExpanded( m_LocalMusicId );
        OnlineMusicExpanded = IsExpanded( m_OnlineMusicId );
        PortableDeviceExpanded = IsExpanded( m_PortableDeviceId );
        ContextExpanded = IsExpanded( m_ContextId );
    }

    wxFont BoldFont = GetFont();
    BoldFont.SetWeight( wxFONTWEIGHT_BOLD );

    wxColour DisableItemColor = wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT );

    // Delete all the previous childrens
    DeleteChildren( m_LocalMusicId );
    DeleteChildren( m_OnlineMusicId );
    DeleteChildren( m_PortableDeviceId );
    DeleteChildren( m_ContextId );

    //
    // Online Music Locations
    //
    CurrentItem = AppendItem( m_OnlineMusicId, _( "Radios" ), 7, -1,
                              new guLocationItemData( ID_MENU_VIEW_RADIO, ( VisiblePanels & guPANEL_MAIN_RADIOS ) ) );
    if( VisiblePanels & guPANEL_MAIN_RADIOS )
        SetItemFont( CurrentItem, BoldFont );

    CurrentItem = AppendItem( m_OnlineMusicId, _( "Podcasts" ), 3, -1,
                              new guLocationItemData( ID_MENU_VIEW_PODCASTS, ( VisiblePanels & guPANEL_MAIN_PODCASTS ) ) );
    if( VisiblePanels & guPANEL_MAIN_PODCASTS )
        SetItemFont( CurrentItem, BoldFont );

    CurrentItem = AppendItem( m_PortableDeviceId, _( "Audio CD" ), GetIconIndex( "media-cdrom-audio" ), -1,
                              new guLocationItemData( ID_MENU_VIEW_AUDIOCD, ( VisiblePanels & guPANEL_MAIN_AUDIOCD ) ) );
    if( VisiblePanels & guPANEL_MAIN_AUDIOCD )
        SetItemFont( CurrentItem, BoldFont );

    const guMediaCollectionArray & Collections = m_MainFrame->GetMediaCollections();
    int Count = Collections.Count();
    int CollectionBaseCommand = ID_COLLECTIONS_BASE;
    for( int Index = 0; Index < Count; Index++ )
    {
        guMediaCollection &Collection = Collections[ Index ];

        bool IsActive = m_MainFrame->IsCollectionActive( Collection.m_UniqueId );
        bool IsPresent = true;

        switch( Collection.m_Type )
        {
            case guMEDIA_COLLECTION_TYPE_NORMAL :
            {
                CurrentItem = AppendItem( m_LocalMusicId, Collection.m_Name, -1, -1,
                                          new guLocationItemData( CollectionBaseCommand, IsActive, Collection.m_UniqueId, guMEDIA_COLLECTION_TYPE_NORMAL ) );
                break;
            }

            case guMEDIA_COLLECTION_TYPE_JAMENDO :
            {
                CurrentItem = AppendItem( m_OnlineMusicId, wxT( "Jamendo" ), 5, -1,
                                          new guLocationItemData( CollectionBaseCommand, IsActive, Collection.m_UniqueId, guMEDIA_COLLECTION_TYPE_JAMENDO ) );
                break;
            }

            case guMEDIA_COLLECTION_TYPE_MAGNATUNE :
            {
                CurrentItem = AppendItem( m_OnlineMusicId, wxT( "Magnatune" ), 4, -1,
                                          new guLocationItemData( CollectionBaseCommand, IsActive, Collection.m_UniqueId, guMEDIA_COLLECTION_TYPE_MAGNATUNE ) );
                break;
            }

            case guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE :
            case guMEDIA_COLLECTION_TYPE_IPOD :
            {
                IsPresent = m_MainFrame->IsCollectionPresent( Collection.m_UniqueId );
                CurrentItem = AppendItem( m_PortableDeviceId, Collection.m_Name,
                                          GetIconIndex( m_MainFrame->GetCollectionIconString( Collection.m_UniqueId ) ), -1,
                                          new guLocationItemData( CollectionBaseCommand, IsActive && IsPresent, Collection.m_UniqueId, Collection.m_Type,
                                                                 IsPresent ) );
                break;
            }
        }

        if( IsActive )
        {
            SetItemFont( CurrentItem, BoldFont );
        }
        else if( !IsPresent )
        {
            SetItemTextColour( CurrentItem, DisableItemColor );
        }

        CollectionBaseCommand += guCOLLECTION_ACTION_COUNT;
    }

    CurrentItem = AppendItem( m_LocalMusicId, _( "File Browser" ), -1, -1,
                              new guLocationItemData( ID_MENU_VIEW_FILEBROWSER, ( VisiblePanels & guPANEL_MAIN_FILEBROWSER ) ) );
    if( VisiblePanels & guPANEL_MAIN_FILEBROWSER )
        SetItemFont( CurrentItem, BoldFont );

    //
    // Context Locations
    //
    CurrentItem = AppendItem( m_ContextId, wxT( "Last.fm" ), 6, -1, new guLocationItemData( ID_MENU_VIEW_LASTFM, ( VisiblePanels & guPANEL_MAIN_LASTFM ) ) );
    if( VisiblePanels & guPANEL_MAIN_LASTFM )
        SetItemFont( CurrentItem, BoldFont );

    CurrentItem = AppendItem( m_ContextId, _( "Lyrics" ), 8, -1, new guLocationItemData( ID_MENU_VIEW_LYRICS, ( VisiblePanels & guPANEL_MAIN_LYRICS ) ) );
    if( VisiblePanels & guPANEL_MAIN_LYRICS )
        SetItemFont( CurrentItem, BoldFont );

    //
    //
    if( LocalMusicExpanded )
        Expand( m_LocalMusicId );
    if( OnlineMusicExpanded )
        Expand( m_OnlineMusicId );
    if( PortableDeviceExpanded )
        Expand( m_PortableDeviceId );
    if( ContextExpanded )
        Expand( m_ContextId );
}

// -------------------------------------------------------------------------------- //
void CreateMenuRadio( wxMenu * menu, const int visiblepanels, guRadioPanel * radiopanel )
{
    wxMenuItem * MenuItem;
    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_RADIO,
                    wxString( _( "Show" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_RADIO ),
                    _( "Show/Hide the radio panel" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( visiblepanels & guPANEL_MAIN_RADIOS );

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_RAD_TEXTSEARCH, _( "Text Search" ), _( "Show/Hide the radio text search" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( radiopanel && radiopanel->IsPanelShown( guPANEL_RADIO_TEXTSEARCH ) );
    MenuItem->Enable( visiblepanels & guPANEL_MAIN_RADIOS );

//    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_RAD_LABELS, _( "Labels" ), _( "Show/Hide the radio labels" ), wxITEM_CHECK );
//    menu->Append( MenuItem );
//    MenuItem->Check( radiopanel && radiopanel->IsPanelShown( guPANEL_RADIO_LABELS ) );
//    MenuItem->Enable( visiblepanels & guPANEL_MAIN_RADIOS );

    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_RAD_GENRES, _( "Genres" ), _( "Show/Hide the radio genres" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( radiopanel && radiopanel->IsPanelShown( guPANEL_RADIO_GENRES ) );
    MenuItem->Enable( visiblepanels & guPANEL_MAIN_RADIOS );

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_RADIO_DOUPDATE, _( "Update" ), _( "Update the radio station lists" ) );
	menu->Append( MenuItem );
    MenuItem->Enable( visiblepanels & guPANEL_MAIN_RADIOS );

    menu->AppendSeparator();

	MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_RAD_PROPERTIES,
                                _( "Properties" ),
                                _( "Set the radio preferences" ), wxITEM_NORMAL );
	menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
void CreateMenuPodcasts( wxMenu * menu, const int visiblepanels, guPodcastPanel * podcastpanel )
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_PODCASTS,
                                        wxString( _( "Show" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_VIEW_PODCASTS ),
                                        _( "Show/Hide the podcasts panel" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( visiblepanels & guPANEL_MAIN_PODCASTS );

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_POD_CHANNELS, _( "Channels" ), _( "Show/Hide the podcasts channels" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( podcastpanel && podcastpanel->IsPanelShown( guPANEL_PODCASTS_CHANNELS ) );
    MenuItem->Enable( visiblepanels & guPANEL_MAIN_PODCASTS );

    MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_POD_DETAILS, _( "Details" ), _( "Show/Hide the podcasts details" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( podcastpanel && podcastpanel->IsPanelShown( guPANEL_PODCASTS_DETAILS ) );
    MenuItem->Enable( visiblepanels & guPANEL_MAIN_PODCASTS );

    menu->AppendSeparator();

	MenuItem = new wxMenuItem( menu, ID_MENU_UPDATE_PODCASTS,
                                wxString( _( "Update" ) ) + guAccelGetCommandKeyCodeString( ID_MENU_UPDATE_PODCASTS ),
                                _( "Update the podcasts added" ), wxITEM_NORMAL );
	menu->Append( MenuItem );
    MenuItem->Enable( visiblepanels & guPANEL_MAIN_PODCASTS );

    menu->AppendSeparator();

	MenuItem = new wxMenuItem( menu, ID_MENU_VIEW_POD_PROPERTIES,
                                _( "Properties" ),
                                _( "Set the podcasts preferences" ), wxITEM_NORMAL );
	menu->Append( MenuItem );
    MenuItem->Enable( visiblepanels & guPANEL_MAIN_PODCASTS );
}

// -------------------------------------------------------------------------------- //
void CreateMenuCollection( wxMenu * menu, const wxString &uniqueid, const int collectiontype,
                guMediaViewer * mediaviewer, const bool ispresent, const int basecommand )
{
    int ViewMode = mediaviewer ? mediaviewer->GetViewMode() : guMEDIAVIEWER_MODE_NONE;
    bool IsEnabled = ( ViewMode != guMEDIAVIEWER_MODE_NONE );

    int VisiblePanels = 0;
    if( ViewMode == guMEDIAVIEWER_MODE_LIBRARY )
    {
        VisiblePanels = mediaviewer->GetLibPanel()->VisiblePanels();
    }

    wxMenuItem * MenuItem = new wxMenuItem( menu, basecommand, _( "Show" ), _( "Open the music collection" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Check( IsEnabled );
    MenuItem->Enable( ispresent );

    menu->AppendSeparator();

    if( IsEnabled )
    {
        //
        //
        wxMenu * SubMenu = new wxMenu();

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIBRARY, _( "Show" ), _( "View the collection library" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        //MenuItem->Enable( IsEnabled );
        MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );

        SubMenu->AppendSeparator();

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_LABELS, _( "Labels" ), _( "View the library labels" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_LABELS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_GENRES, _( "Genres" ), _( "View the library genres" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_GENRES );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_ARTISTS, _( "Artists" ), _( "View the library artists" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_ARTISTS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_COMPOSERS, _( "Composers" ), _( "View the library composers" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_COMPOSERS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_ALBUMARTISTS, _( "Album Artists" ), _( "View the library album artists" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_ALBUMARTISTS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_ALBUMS, _( "Albums" ), _( "View the library albums" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_ALBUMS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_YEARS, _( "Years" ), _( "View the library years" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_YEARS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_RATINGS, _( "Ratings" ), _( "View the library ratings" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_RATINGS );

        MenuItem = new wxMenuItem( SubMenu, basecommand + guCOLLECTION_ACTION_VIEW_LIB_PLAYCOUNT, _( "Play Counts" ), _( "View the library play counts" ), wxITEM_CHECK );
        SubMenu->Append( MenuItem );
        MenuItem->Enable( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        MenuItem->Check( VisiblePanels & guPANEL_LIBRARY_PLAYCOUNT );

        menu->AppendSubMenu( SubMenu, _( "Library" ) );
    }
    else
    {
        MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_VIEW_LIBRARY, _( "Library" ), _( "View the collection library" ), wxITEM_CHECK );
        menu->Append( MenuItem );
        MenuItem->Enable( false );
        MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
    }

    MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_VIEW_ALBUMBROWSER, _( "Album Browser" ), _( "View the collection album browser" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );
    MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER );

    MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_VIEW_TREEVIEW, _( "Tree" ), _( "View the collection tree view" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );
    MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_TREEVIEW );

    MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_VIEW_PLAYLISTS, _( "Playlists" ), _( "View the collection playlists" ), wxITEM_CHECK );
    menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );
    MenuItem->Check( ViewMode == guMEDIAVIEWER_MODE_PLAYLISTS );

    if( ( collectiontype == guMEDIA_COLLECTION_TYPE_NORMAL ) ||
        ( collectiontype == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
        ( collectiontype == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        menu->AppendSeparator();

        MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_ADD_PATH, _( "Add Path" ), _( "Add path to the collection" ), wxITEM_NORMAL );
        menu->Append( MenuItem );
        MenuItem->Enable( IsEnabled );

        MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_IMPORT, _( "Import Files" ), _( "Import files into the collection" ), wxITEM_NORMAL );
        menu->Append( MenuItem );
        MenuItem->Enable( IsEnabled );
    }

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_UPDATE_LIBRARY,
                              _( "Update" ),
                              _( "Update the collection library" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );

    MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_RESCAN_LIBRARY,
                              _( "Rescan" ),
                              _( "Rescan the collection library" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );

    MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_SEARCH_COVERS,
                              _( "Search Covers" ),
                              _( "Search the collection missing covers" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    MenuItem->Enable( IsEnabled );

    if( ( collectiontype == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) || ( collectiontype == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        menu->AppendSeparator();

        MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_UNMOUNT, _( "Unmount" ), _( "Unmount the device" ) );
        menu->Append( MenuItem );
        MenuItem->Enable( ispresent );
    }

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, basecommand + guCOLLECTION_ACTION_VIEW_PROPERTIES, _( "Properties" ), _( "Show collection properties" ), wxITEM_NORMAL );
    menu->Append( MenuItem );
    if( ( collectiontype == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) || ( collectiontype == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        MenuItem->Enable( IsEnabled );
    }
}

// -------------------------------------------------------------------------------- //
void guLocationTreeCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxTreeItemId ItemId = event.GetItem();
    guLocationItemData * ItemData = ( guLocationItemData * ) GetItemData( ItemId );
    if( ItemData )
    {
        wxMenu Menu;

        switch( ItemData->GetId() )
        {
            case ID_MENU_VIEW_RADIO :
            {
                CreateMenuRadio( &Menu, m_MainFrame->VisiblePanels(), m_MainFrame->GetRadioPanel() );
                break;
            }

            case ID_MENU_VIEW_PODCASTS :
            {
                CreateMenuPodcasts( &Menu, m_MainFrame->VisiblePanels(), m_MainFrame->GetPodcastsPanel() );
                break;
            }

            case ID_MENU_VIEW_LASTFM :
            case ID_MENU_VIEW_LYRICS :
            case ID_MENU_VIEW_FILEBROWSER :
            case ID_MENU_VIEW_AUDIOCD :
            {
                return;
            }

            default :
            {
                wxString CollectionId = ItemData->GetCollectionId();
                int CollectionType = ItemData->GetCollectionType();

                bool IsPresent = true;
                if( ( CollectionType == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
                    ( CollectionType == guMEDIA_COLLECTION_TYPE_IPOD ) )
                {
                    IsPresent = m_MainFrame->IsCollectionPresent( CollectionId );
                }

                CreateMenuCollection( &Menu, CollectionId, CollectionType,
                                      m_MainFrame->FindCollectionMediaViewer( CollectionId ),
                                      IsPresent, ItemData->GetId() );
                break;
            }
        }

        wxPoint Point = event.GetPoint();

        PopupMenu( &Menu, Point );
    }
}

//// -------------------------------------------------------------------------------- //
//void guLocationTreeCtrl::OnKeyDown( wxKeyEvent &event )
//{
//    event.Skip();
//}




// -------------------------------------------------------------------------------- //
// guLocationPanel
// -------------------------------------------------------------------------------- //
guLocationPanel::guLocationPanel( wxWindow * parent ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_MainFrame = ( guMainFrame * ) parent;

    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    m_LocationTreeCtrl = new guLocationTreeCtrl( this, m_MainFrame );
    MainSizer->Add( m_LocationTreeCtrl, 1, wxEXPAND, 5 );

	SetSizer( MainSizer );
	Layout();
	MainSizer->Fit( this );

    m_LocationTreeCtrl->Bind( wxEVT_TREE_ITEM_ACTIVATED, &guLocationPanel::OnLocationItemActivated, this );
    m_LocationTreeCtrl->Bind( wxEVT_TREE_SEL_CHANGED, &guLocationPanel::OnLocationItemChanged, this );
}

// -------------------------------------------------------------------------------- //
guLocationPanel::~guLocationPanel()
{
    m_LocationTreeCtrl->Unbind( wxEVT_TREE_ITEM_ACTIVATED, &guLocationPanel::OnLocationItemActivated, this );
    m_LocationTreeCtrl->Unbind( wxEVT_TREE_SEL_CHANGED, &guLocationPanel::OnLocationItemChanged, this );
}

// -------------------------------------------------------------------------------- //
void guLocationPanel::CollectionsUpdated( void )
{
    m_LocationTreeCtrl->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guLocationPanel::OnPanelVisibleChanged( void )
{
    m_LocationTreeCtrl->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guLocationPanel::OnLocationItemActivated( wxTreeEvent &event )
{
    wxTreeItemId ItemId = event.GetItem();
    guLocationItemData * ItemData = ( guLocationItemData * ) m_LocationTreeCtrl->GetItemData( ItemId );
    if( ItemData && ItemData->GetIsEnabled() )
    {
        wxCommandEvent event( wxEVT_MENU, ItemData->GetId() );
        event.SetInt( !ItemData->GetOpen() );
        wxPostEvent( m_MainFrame, event );
    }
    else
        event.Skip();
}

// -------------------------------------------------------------------------------- //
void guLocationPanel::OnLocationItemChanged( wxTreeEvent &event )
{
    wxTreeItemId ItemId = event.GetItem();
    guLocationItemData * ItemData = ( guLocationItemData * ) m_LocationTreeCtrl->GetItemData( ItemId );
    if( ItemData )
    {
        if( ItemData->GetOpen() && ItemData->GetId() )
        {
            wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_SELECT_LOCATION );
            event.SetInt( ItemData->GetId() );
            wxPostEvent( m_MainFrame, event );
        }
    }
    event.Skip();
}

}

// -------------------------------------------------------------------------------- //
