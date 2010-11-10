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
#include "LocationPanel.h"

#include "Config.h"
#include "Images.h"

#include <wx/tokenzr.h>
#include <wx/artprov.h>

// -------------------------------------------------------------------------------- //
// guShoutcastItemData
// -------------------------------------------------------------------------------- //
class guLocationItemData : public wxTreeItemData
{
  private :
    int         m_Id;
    bool        m_IsOpen;

  public :
    guLocationItemData( const int id, const bool open )
    {
        m_Id = id;
        m_IsOpen = open;
    }

    int         GetId( void ) { return m_Id; }
    void        SetId( int id ) { m_Id = id; }
    int         GetOpen( void ) { return m_IsOpen; }
    void        SetOpen( const bool open ) { m_IsOpen = open; }
};

#define guLOCATION_PANEL_IMAGE_COUNT    9

// -------------------------------------------------------------------------------- //
// guLocationTreeCtrl
// -------------------------------------------------------------------------------- //
guLocationTreeCtrl::guLocationTreeCtrl( wxWindow * parent, guMainFrame * mainframe ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_SINGLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT )
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

    //SetIndent( 2 );

    wxFont FontBold = GetFont();
    FontBold.SetWeight( wxFONTWEIGHT_BOLD );

    m_MyMusicId = AppendItem( m_RootId, _( "My Music" ), 0, -1, NULL );

    m_PortableDeviceId = AppendItem( m_RootId, _( "Portable Devices" ), 1, -1, NULL );

    m_OnlineRadioId = AppendItem( m_RootId, _( "Radios" ), 2, -1, NULL );

    m_OnlineStoreId = AppendItem( m_RootId, _( "Stores" ), -1, -1, NULL );

    m_PodcastId = AppendItem( m_RootId, _( "Podcasts" ), 3, -1, new guLocationItemData( ID_MENU_VIEW_PODCASTS, false ) );

    m_ContextId = AppendItem( m_RootId, _( "Context" ), -1, -1, NULL );

    SetIndent( 5 );

    Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guLocationTreeCtrl::OnContextMenu ), NULL, this );
    Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guLocationTreeCtrl::OnKeyDown ), NULL, this );

    ReloadItems( true );
}

// -------------------------------------------------------------------------------- //
guLocationTreeCtrl::~guLocationTreeCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( wxT( "MyMusicExpanded" ), IsExpanded( m_MyMusicId ), wxT( "MainSources") );
    Config->WriteBool( wxT( "RadiosExpanded" ), IsExpanded( m_OnlineRadioId ), wxT( "MainSources") );
    Config->WriteBool( wxT( "StoresExpanded" ), IsExpanded( m_OnlineStoreId ), wxT( "MainSources") );
    Config->WriteBool( wxT( "ContextExpanded" ), IsExpanded( m_ContextId ), wxT( "MainSources") );

    Disconnect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guLocationTreeCtrl::OnContextMenu ), NULL, this );
    Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guLocationTreeCtrl::OnKeyDown ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLocationTreeCtrl::ReloadItems( const bool loadstate )
{
    if( m_LockCount )
        return;

    guLogMessage( wxT( "guLocationTreeCtrl::ReloadItems" ) );
    wxTreeItemId CurrentItem;
    int VisiblePanels = m_MainFrame->VisiblePanels();

    bool MyMusicExpanded;
    bool RadiosExpanded;
    bool StoresExpanded;
    bool ContextExpanded;
    if( loadstate )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        MyMusicExpanded = Config->ReadBool( wxT( "MyMusicExpanded" ), true, wxT( "MainSources") );
        RadiosExpanded = Config->ReadBool( wxT( "RadiosExpanded" ), true, wxT( "MainSources") );
        StoresExpanded = Config->ReadBool( wxT( "StoresExpanded" ), true, wxT( "MainSources") );
        ContextExpanded = Config->ReadBool( wxT( "ContextExpanded" ), true, wxT( "MainSources") );
    {
        MyMusicExpanded = IsExpanded( m_MyMusicId );
        RadiosExpanded = IsExpanded( m_OnlineRadioId );
        StoresExpanded = IsExpanded( m_OnlineStoreId );
        ContextExpanded = IsExpanded( m_ContextId );
    }

    //
    // My Local Music Locations
    }
    else
    {
        MyMusicExpanded = IsExpanded( m_MyMusicId );
        RadiosExpanded = IsExpanded( m_OnlineRadioId );
        StoresExpanded = IsExpanded( m_OnlineStoreId );
        ContextExpanded = IsExpanded( m_ContextId );
    }

    //
    // My Local Music Locations
    //
    DeleteChildren( m_MyMusicId );

    wxFont BoldFont = GetFont();
    BoldFont.SetWeight( wxFONTWEIGHT_BOLD );
    CurrentItem = AppendItem( m_MyMusicId, _( "Library" ), -1, -1, new guLocationItemData( ID_MENU_VIEW_LIBRARY, ( VisiblePanels & guPANEL_MAIN_LIBRARY ) ) );
    if( VisiblePanels & guPANEL_MAIN_LIBRARY )
        SetItemFont( CurrentItem, BoldFont );
    CurrentItem = AppendItem( m_MyMusicId, _( "Playlists" ), -1, -1, new guLocationItemData( ID_MENU_VIEW_PLAYLISTS, ( VisiblePanels & guPANEL_MAIN_PLAYLISTS ) ) );
    if( VisiblePanels & guPANEL_MAIN_PLAYLISTS )
        SetItemFont( CurrentItem, BoldFont );
    CurrentItem = AppendItem( m_MyMusicId, _( "Album Browser" ), -1, -1, new guLocationItemData( ID_MENU_VIEW_ALBUMBROWSER, ( VisiblePanels & guPANEL_MAIN_ALBUMBROWSER ) ) );
    if( VisiblePanels & guPANEL_MAIN_ALBUMBROWSER )
        SetItemFont( CurrentItem, BoldFont );
    CurrentItem = AppendItem( m_MyMusicId, _( "File Browser" ), -1, -1, new guLocationItemData( ID_MENU_VIEW_FILEBROWSER, ( VisiblePanels & guPANEL_MAIN_FILEBROWSER ) ) );
    if( VisiblePanels & guPANEL_MAIN_FILEBROWSER )
        SetItemFont( CurrentItem, BoldFont );

    if( MyMusicExpanded )
        Expand( m_MyMusicId );

    //
    // Portable Device Locations
    //
    DeleteChildren( m_PortableDeviceId );

    wxArrayString VolumeNames = m_MainFrame->PortableDeviceVolumeNames();
    int Index;
    int Count = VolumeNames.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        bool IconFound = false;

        int DeviceBaseCmd = ID_MENU_VIEW_PORTABLE_DEVICE + ( Index * guPORTABLEDEVICE_COMMANDS_COUNT );
        guPortableMediaViewCtrl * PortableMediaViewCtrl = m_MainFrame->GetPortableMediaViewCtrl( DeviceBaseCmd );
        if( PortableMediaViewCtrl )
        {
            wxString IconString = PortableMediaViewCtrl->IconString();
            if( !IconString.IsEmpty() )
            {
                //. GThemedIcon drive-removable-media-usb drive-removable-media drive-removable drive
                wxArrayString IconNames = wxStringTokenize( IconString, wxT( " " ) );
                int IconIndex;
                int IconCount = IconNames.Count();
                for( IconIndex = 0; IconIndex < IconCount; IconIndex++ )
                {
                    guLogMessage( wxT( "Trying to load the icon '%s'" ), IconNames[ IconIndex ].c_str() );
                    if( IconNames[ IconIndex ] == wxT( "." ) || IconNames[ IconIndex ] == wxT( "GThemedIcon" ) )
                        continue;

                    wxBitmap IconBitmap = wxArtProvider::GetBitmap( IconNames[ IconIndex ], wxART_OTHER, wxSize( 24, 24 ) );
                    if( IconBitmap.IsOk() )
                    {
                        guLogMessage( wxT( "The Icon was found...") );
                        int IconPos = m_IconNames.Index( IconNames[ IconIndex ] );
                        if( IconPos == wxNOT_FOUND )
                        {
                            IconPos = m_IconNames.Count();
                            m_IconNames.Add( IconNames[ IconIndex ] );
                            m_ImageList->Add( IconBitmap );
                        }
                        CurrentItem = AppendItem( m_PortableDeviceId, VolumeNames[ Index ], IconPos + guLOCATION_PANEL_IMAGE_COUNT, -1, NULL );
                        IconFound = true;
                        break;
                    }
                }
            }
        }

        if( !IconFound )
        {
            CurrentItem = AppendItem( m_PortableDeviceId, VolumeNames[ Index ], -1, -1, new guLocationItemData( DeviceBaseCmd, PortableMediaViewCtrl ) );
        }

        if( PortableMediaViewCtrl )
        {
            SetItemFont( CurrentItem, BoldFont );
            wxTreeItemId CurrentSubItem;
            int VisiblePanels = PortableMediaViewCtrl->VisiblePanels();
            CurrentSubItem = AppendItem( CurrentItem, _( "Library" ), -1, -1, new guLocationItemData( DeviceBaseCmd, ( VisiblePanels & guPANEL_MAIN_LIBRARY ) ) );
            if( VisiblePanels & guPANEL_MAIN_LIBRARY )
                SetItemFont( CurrentSubItem, BoldFont );
            CurrentSubItem = AppendItem( CurrentItem, _( "Playlists" ), -1, -1, new guLocationItemData( DeviceBaseCmd + 18, ( VisiblePanels & guPANEL_MAIN_PLAYLISTS ) ) );
            if( VisiblePanels & guPANEL_MAIN_PLAYLISTS )
                SetItemFont( CurrentSubItem, BoldFont );
            CurrentSubItem = AppendItem( CurrentItem, _( "Album Browser" ), -1, -1, new guLocationItemData( DeviceBaseCmd + 19, ( VisiblePanels & guPANEL_MAIN_ALBUMBROWSER ) ) );
            if( VisiblePanels & guPANEL_MAIN_ALBUMBROWSER )
                SetItemFont( CurrentSubItem, BoldFont );
            Expand( CurrentItem );
        }

        //BaseCmd = ID_MENU_VIEW_PORTABLE_DEVICE + ( Index * guPORTABLEDEVICE_COMMANDS_COUNT );
        //CreatePortableMediaDeviceMenu( menu, VolumeNames[ Index ], BaseCmd );
    }

    Expand( m_PortableDeviceId );

    //
    // Online Radio Locations
    //
    DeleteChildren( m_OnlineRadioId );
    CurrentItem = AppendItem( m_OnlineRadioId, _( "Shoutcast" ), 7, -1, new guLocationItemData( ID_MENU_VIEW_RADIO, ( VisiblePanels & guPANEL_MAIN_RADIOS ) ) );
    if( VisiblePanels & guPANEL_MAIN_RADIOS )
        SetItemFont( CurrentItem, BoldFont );

    if( RadiosExpanded )
        Expand( m_OnlineRadioId );

    //
    // Online Stores Locations
    //
    DeleteChildren( m_OnlineStoreId );
    CurrentItem = AppendItem( m_OnlineStoreId, _( "Magnatune" ), 4, -1, new guLocationItemData( ID_MENU_VIEW_MAGNATUNE, ( VisiblePanels & guPANEL_MAIN_MAGNATUNE ) ) );
    if( VisiblePanels & guPANEL_MAIN_MAGNATUNE )
        SetItemFont( CurrentItem, BoldFont );
    CurrentItem = AppendItem( m_OnlineStoreId, _( "Jamendo" ), 5, -1, new guLocationItemData( ID_MENU_VIEW_JAMENDO, ( VisiblePanels & guPANEL_MAIN_MAGNATUNE ) ) );
    if( VisiblePanels & guPANEL_MAIN_JAMENDO )
        SetItemFont( CurrentItem, BoldFont );
    if( StoresExpanded )
        Expand( m_OnlineStoreId );

    //
    // Podcasts Locations
    //
    //DeleteChildren( m_PodcastId );
    guLocationItemData * PodcastItemData = ( guLocationItemData * ) GetItemData( m_PodcastId );
    PodcastItemData->SetOpen( ( VisiblePanels & guPANEL_MAIN_PODCASTS ) );
    SetItemFont( m_PodcastId, ( VisiblePanels & guPANEL_MAIN_PODCASTS ) ? BoldFont : GetFont() );


    //
    // Context Locations
    //
    DeleteChildren( m_ContextId );
    CurrentItem = AppendItem( m_ContextId, wxT( "Last.fm" ), 6, -1, new guLocationItemData( ID_MENU_VIEW_LASTFM, ( VisiblePanels & guPANEL_MAIN_LASTFM ) ) );
    if( VisiblePanels & guPANEL_MAIN_LASTFM )
        SetItemFont( CurrentItem, BoldFont );

    CurrentItem = AppendItem( m_ContextId, _( "Lyrics" ), 8, -1, new guLocationItemData( ID_MENU_VIEW_LYRICS, ( VisiblePanels & guPANEL_MAIN_LYRICS ) ) );
    if( VisiblePanels & guPANEL_MAIN_LYRICS )
        SetItemFont( CurrentItem, BoldFont );
    if( ContextExpanded )
        Expand( m_ContextId );


}

// -------------------------------------------------------------------------------- //
void guLocationTreeCtrl::OnContextMenu( wxTreeEvent &event )
{
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guLocationTreeCtrl::OnKeyDown( wxKeyEvent &event )
{
    event.Skip();
}




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

    m_LocationTreeCtrl->Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( guLocationPanel::OnLocationItemActivated ), NULL, this );
	m_LocationTreeCtrl->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guLocationPanel::OnLocationItemChanged ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guLocationPanel::~guLocationPanel()
{
}

// -------------------------------------------------------------------------------- //
void guLocationPanel::OnPortableDeviceChanged( void )
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
    if( ItemData )
    {
        guLogMessage( wxT( "Sending the event %i" ), ItemData->GetId() );
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ItemData->GetId() );
        event.SetInt( !ItemData->GetOpen() );
        wxPostEvent( m_MainFrame, event );
    }
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
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_LOCATION );
            event.SetInt( ItemData->GetId() );
            wxPostEvent( m_MainFrame, event );
        }
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
