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
#include "MediaViewer.h"

#include "Accelerators.h"
#include "CopyTo.h"
#include "CoverEdit.h"
#include "DynamicPlayList.h"
#include "Images.h"
#include "ImportFiles.h"
#include "LibUpdate.h"
#include "MainFrame.h"
#include "SelCoverFile.h"
#include "Settings.h"
#include "SmartMode.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Transcode.h"
#include "Utils.h"

namespace Guayadeque {

#define     guMEDIAVIEWER_TIMER_TEXTSEARCH        2
#define     guMEDIAVIEWER_TIMER_TEXTCHANGED       500

// -------------------------------------------------------------------------------- //
guMediaViewer::guMediaViewer( wxWindow * parent, guMediaCollection &collection, const int basecommand,
                guMainFrame * mainframe, const int mode, guPlayerPanel * playerpanel ) :
    wxPanel( parent ),
    m_TextChangedTimer( this, guMEDIAVIEWER_TIMER_TEXTSEARCH )
{
    m_MediaCollection = new guMediaCollection( collection );
    m_MainFrame = mainframe;
    m_PlayerPanel = playerpanel;
    m_BaseCommand = basecommand;
    m_ViewMode = wxNOT_FOUND;
    m_ConfigPath = wxT( "mediaviewers/mediaviewer_" ) + collection.m_UniqueId;
    m_UpdateThread = NULL;
    m_CleanThread = NULL;
    m_ContextMenuFlags = guCONTEXTMENU_DEFAULT;
    m_IsDefault = false;
    m_CopyToPattern = NULL;
    m_UpdateCoversThread = NULL;
    guLogMessage( wxT( "MediaViewer '%s' => '%s'" ), collection.m_Name.c_str(), collection.m_UniqueId.c_str() );

    if( !wxDirExists( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId ) )
    {
        wxMkdir( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId, 0770 );
        guLogMessage( wxT( "Created collection folder '%s'" ), wxString( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId ).c_str() );
    }

    //LoadMediaDb();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_LibPanel = NULL;
    m_AlbumBrowser = NULL;
    m_TreeViewPanel = NULL;
    m_PlayListPanel = NULL;
    m_DoneClearSearchText = false;

    m_InstantSearchEnabled = Config->ReadBool( CONFIG_KEY_GENERAL_INSTANT_TEXT_SEARCH, true, CONFIG_PATH_GENERAL );
    m_EnterSelectSearchEnabled = !Config->ReadBool( CONFIG_KEY_GENERAL_TEXT_SEARCH_ENTER, false, CONFIG_PATH_GENERAL );

    if( !m_MediaCollection->m_DefaultCopyAction.IsEmpty() )
    {
        wxArrayString Options = Config->ReadAStr( CONFIG_KEY_COPYTO_OPTION, wxEmptyString, CONFIG_PATH_COPYTO );
        int Index;
        int Count;

        if( ( Count = Options.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                if( Options[ Index ].BeforeFirst( wxT( ':' ) ) == m_MediaCollection->m_DefaultCopyAction )
                {
                    m_CopyToPattern = new guCopyToPattern( Options[ Index ] );
                }
            }
        }
    }

    Bind( wxEVT_TIMER, &guMediaViewer::OnTextChangedTimer, this, guMEDIAVIEWER_TIMER_TEXTSEARCH );

    Bind( wxEVT_MENU, &guMediaViewer::OnCleanFinished, this, ID_LIBRARY_CLEANFINISHED );
    Bind( wxEVT_MENU, &guMediaViewer::OnLibraryUpdated, this, ID_LIBRARY_UPDATED );

    Bind( wxEVT_MENU, &guMediaViewer::OnGenreSetSelection, this, ID_GENRE_SETSELECTION );
    Bind( wxEVT_MENU, &guMediaViewer::OnArtistSetSelection, this, ID_ARTIST_SETSELECTION );
    Bind( wxEVT_MENU, &guMediaViewer::OnAlbumArtistSetSelection, this, ID_ALBUMARTIST_SETSELECTION );
    Bind( wxEVT_MENU, &guMediaViewer::OnComposerSetSelection, this, ID_COMPOSER_SETSELECTION );
    Bind( wxEVT_MENU, &guMediaViewer::OnAlbumSetSelection, this, ID_ALBUM_SETSELECTION );

    Bind( wxEVT_MENU, &guMediaViewer::OnUpdateLabels, this, ID_LABEL_UPDATELABELS );

    Bind( wxEVT_MENU, &guMediaViewer::OnSmartAddTracks, this, ID_SMARTMODE_ADD_TRACKS );
}

// -------------------------------------------------------------------------------- //
guMediaViewer::~guMediaViewer()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( wxT( "ViewMode" ), m_ViewMode, m_ConfigPath );
    Config->WriteNum( wxT( "Filter" ), m_FilterChoice->GetSelection(), m_ConfigPath + wxT( "/albumbrowser" ) );
    Config->WriteAStr( wxT( "Filter"), m_DynFilterArray, m_ConfigPath + wxT( "/albumbrowser/filters") );

    Unbind( wxEVT_TIMER, &guMediaViewer::OnTextChangedTimer, this, guMEDIAVIEWER_TIMER_TEXTSEARCH );

    Unbind( wxEVT_MENU, &guMediaViewer::OnCleanFinished, this, ID_LIBRARY_CLEANFINISHED );
    Unbind( wxEVT_MENU, &guMediaViewer::OnLibraryUpdated, this, ID_LIBRARY_UPDATED );

    Unbind( wxEVT_MENU, &guMediaViewer::OnGenreSetSelection, this, ID_GENRE_SETSELECTION );
    Unbind( wxEVT_MENU, &guMediaViewer::OnArtistSetSelection, this, ID_ARTIST_SETSELECTION );
    Unbind( wxEVT_MENU, &guMediaViewer::OnAlbumArtistSetSelection, this, ID_ALBUMARTIST_SETSELECTION );
    Unbind( wxEVT_MENU, &guMediaViewer::OnComposerSetSelection, this, ID_COMPOSER_SETSELECTION );
    Unbind( wxEVT_MENU, &guMediaViewer::OnAlbumSetSelection, this, ID_ALBUM_SETSELECTION );

    Unbind( wxEVT_MENU, &guMediaViewer::OnUpdateLabels, this, ID_LABEL_UPDATELABELS );

    Unbind( wxEVT_MENU, &guMediaViewer::OnSmartAddTracks, this, ID_SMARTMODE_ADD_TRACKS );

    Unbind( guConfigUpdatedEvent, &guMediaViewer::OnConfigUpdated, this, ID_CONFIG_UPDATED );
    m_FilterChoice->Unbind( wxEVT_CHOICE, &guMediaViewer::OnFilterSelected, this );
    m_AddFilterButton->Unbind( wxEVT_BUTTON, &guMediaViewer::OnAddFilterClicked, this );
    m_DelFilterButton->Unbind( wxEVT_BUTTON, &guMediaViewer::OnDelFilterClicked, this );
    m_EditFilterButton->Unbind( wxEVT_BUTTON, &guMediaViewer::OnEditFilterClicked, this );

    m_LibrarySelButton->Unbind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );
    m_AlbumBrowserSelButton->Unbind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );
    m_TreeViewSelButton->Unbind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );
    m_PlaylistsSelButton->Unbind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );

    m_SearchTextCtrl->Unbind( wxEVT_TEXT_ENTER, &guMediaViewer::OnSearchSelected, this );
    m_SearchTextCtrl->Unbind( wxEVT_TEXT, &guMediaViewer::OnSearchActivated, this );
    m_SearchTextCtrl->Unbind( wxEVT_SEARCHCTRL_CANCEL_BTN, &guMediaViewer::OnSearchCancelled, this );

    //m_MainFrame->MediaViewerClosed( m_MediaCollection->m_UniqueId, this );
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_MEDIAVIEWER_CLOSED );
    CmdEvent.SetClientData( this );
    wxPostEvent( m_MainFrame, CmdEvent );

    if( m_UpdateCoversThread )
    {
        m_UpdateCoversThread->Pause();
        m_UpdateCoversThread->Delete();
        m_UpdateCoversThread = NULL;
    }

    if( m_MediaCollection )
        delete m_MediaCollection;

    if( m_CopyToPattern )
        delete m_CopyToPattern;

    if( m_Db )
        delete m_Db;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::InitMediaViewer( const int mode )
{
    LoadMediaDb();

    CreateControls();

    if( mode != wxNOT_FOUND )
    {
        SetViewMode( mode );
    }
    else
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        SetViewMode( Config->ReadNum( wxT( "ViewMode" ), guMEDIAVIEWER_MODE_LIBRARY, m_ConfigPath ) );
    }

    CreateAcceleratorTable();

    m_MainFrame->MediaViewerCreated( m_MediaCollection->m_UniqueId, this );

    if( m_MediaCollection->m_UpdateOnStart )
    {
        //UpdateLibrary();
        wxCommandEvent Event( wxEVT_MENU, m_BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY );
        wxPostEvent( this, Event );
    }

    m_SearchTextCtrl->SetFocus();

    SetDropTarget( new guMediaViewerDropTarget( this ) );

    Layout();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::CreateControls( void )
{
    wxBoxSizer *  MainSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer * TopSizer = new wxBoxSizer( wxHORIZONTAL );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    int FilterSelected = Config->ReadNum( wxT( "Filter" ), 0, m_ConfigPath + wxT( "/albumbrowser" ) );
    m_DynFilterArray = Config->ReadAStr( wxT( "Filter"), wxEmptyString, m_ConfigPath + wxT( "/albumbrowser/filters" ) );

    if( FilterSelected > ( int ) m_DynFilterArray.Count() )
        FilterSelected = 0;

    wxArrayString FilterNames;
    FilterNames.Add( _( "All Albums" ) );

    int Index;
    int Count = m_DynFilterArray.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        FilterNames.Add( unescape_configlist_str( m_DynFilterArray[ Index ].BeforeFirst( wxT( ':' ) ) ) );
    }

    m_LibrarySelButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_mv_library ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_LibrarySelButton->SetToolTip( _( "Library view" ) );
    TopSizer->Add( m_LibrarySelButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5 );

    m_AlbumBrowserSelButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_mv_albumbrowser ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_AlbumBrowserSelButton->SetToolTip( _( "Album Browser view" ) );
    TopSizer->Add( m_AlbumBrowserSelButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );

    m_TreeViewSelButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_mv_treeview ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_TreeViewSelButton->SetToolTip( _( "Tree view" ) );
    TopSizer->Add( m_TreeViewSelButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );

    m_PlaylistsSelButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_mv_playlists ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_PlaylistsSelButton->SetToolTip( _( "Playlists" ) );
    TopSizer->Add( m_PlaylistsSelButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    m_SearchTextCtrl = new wxSearchCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, 28 ), wxTE_PROCESS_ENTER );
    //m_SearchTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,  wxTE_PROCESS_ENTER );
    TopSizer->Add( m_SearchTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

//////////////////////////
    m_FiltersSizer = new wxBoxSizer( wxHORIZONTAL );

    m_FilterChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, FilterNames, 0 );
    m_FilterChoice->SetSelection( FilterSelected );
    m_FilterChoice->SetToolTip( _( "View the current defined filters" ) );
    //m_FilterChoice->SetMinSize( wxSize( 100, -1 ) );
    m_FiltersSizer->Add( m_FilterChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_AddFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_AddFilterButton->SetToolTip( _( "Add a new album browser filter" ) );
    m_FiltersSizer->Add( m_AddFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );

    m_DelFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_DelFilterButton->SetToolTip( _( "Delete the current selected filter" ) );
    m_DelFilterButton->Enable( FilterSelected > 0 );
    m_FiltersSizer->Add( m_DelFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );

    m_EditFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_EditFilterButton->SetToolTip( _( "Edit the current selected filter" ) );
    m_EditFilterButton->Enable( FilterSelected > 0 );
    m_FiltersSizer->Add( m_EditFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    TopSizer->Add( m_FiltersSizer, 0, wxEXPAND, 5 );
//////////////////////////////

    MainSizer->Add( TopSizer, 0, wxEXPAND, 5 );
    SetSizer( MainSizer );

    Bind( guConfigUpdatedEvent, &guMediaViewer::OnConfigUpdated, this, ID_CONFIG_UPDATED );
    m_FilterChoice->Bind( wxEVT_CHOICE, &guMediaViewer::OnFilterSelected, this );
    m_AddFilterButton->Bind( wxEVT_BUTTON, &guMediaViewer::OnAddFilterClicked, this );
    m_DelFilterButton->Bind( wxEVT_BUTTON, &guMediaViewer::OnDelFilterClicked, this );
    m_EditFilterButton->Bind( wxEVT_BUTTON, &guMediaViewer::OnEditFilterClicked, this );

    m_LibrarySelButton->Bind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );
    m_AlbumBrowserSelButton->Bind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );
    m_TreeViewSelButton->Bind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );
    m_PlaylistsSelButton->Bind( wxEVT_BUTTON, &guMediaViewer::OnViewChanged, this );

    m_SearchTextCtrl->Bind( wxEVT_TEXT_ENTER, &guMediaViewer::OnSearchSelected, this );
    m_SearchTextCtrl->Bind( wxEVT_TEXT, &guMediaViewer::OnSearchActivated, this );
    m_SearchTextCtrl->Bind( wxEVT_SEARCHCTRL_CANCEL_BTN, &guMediaViewer::OnSearchCancelled, this );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_MENU_UPDATE_LIBRARY );
    AliasAccelCmds.Add( ID_MENU_UPDATE_LIBRARYFORCED );
    AliasAccelCmds.Add( ID_MENU_UPDATE_COVERS );

    RealAccelCmds.Add( m_BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY );
    RealAccelCmds.Add( m_BaseCommand + guCOLLECTION_ACTION_RESCAN_LIBRARY );
    RealAccelCmds.Add( m_BaseCommand + guCOLLECTION_ACTION_SEARCH_COVERS );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::LoadMediaDb( void )
{
    m_Db = new guDbLibrary( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId + wxT( "/guayadeque.db" ) );
    m_Db->SetMediaViewer( this );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_GENERAL )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_InstantSearchEnabled = Config->ReadBool( CONFIG_KEY_GENERAL_INSTANT_TEXT_SEARCH, true, CONFIG_PATH_GENERAL );
        m_EnterSelectSearchEnabled = !Config->ReadBool( CONFIG_KEY_GENERAL_TEXT_SEARCH_ENTER, false, CONFIG_PATH_GENERAL );
    }

    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetCollection( guMediaCollection &collection, const int basecommand )
{
    if( m_MediaCollection )
        delete m_MediaCollection;
    m_MediaCollection = new guMediaCollection( collection );

    m_BaseCommand = basecommand;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnViewChanged( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnViewChanged... %i" ), event.GetInt() );
    if( event.GetEventObject() == m_LibrarySelButton )
    {
        SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );
    }
    else if( event.GetEventObject() == m_AlbumBrowserSelButton )
    {
        SetViewMode( guMEDIAVIEWER_MODE_ALBUMBROWSER );
    }
    else if( event.GetEventObject() == m_TreeViewSelButton )
    {
        SetViewMode( guMEDIAVIEWER_MODE_TREEVIEW );
    }
    else if( event.GetEventObject() == m_PlaylistsSelButton )
    {
        SetViewMode( guMEDIAVIEWER_MODE_PLAYLISTS );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetPlayerPanel( guPlayerPanel * playerpanel )
{
    m_PlayerPanel = playerpanel;
    if( m_LibPanel )
        m_LibPanel->SetPlayerPanel( playerpanel );
    if( m_AlbumBrowser )
        m_AlbumBrowser->SetPlayerPanel( playerpanel );
    if( m_TreeViewPanel )
        m_TreeViewPanel->SetPlayerPanel( playerpanel );
    if( m_PlayListPanel )
        m_PlayListPanel->SetPlayerPanel( playerpanel );
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::CreateLibraryView( void )
{
    m_LibPanel = new guLibPanel( this, this );
    m_LibPanel->SetBaseCommand( m_BaseCommand + 1 );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::CreateAlbumBrowserView( void )
{
    m_AlbumBrowser = new guAlbumBrowser( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::CreateTreeView( void )
{
    m_TreeViewPanel = new guTreeViewPanel( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::CreatePlayListView( void )
{
    m_PlayListPanel = new guPlayListPanel( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetViewMode( const int mode )
{
    guLogMessage( wxT( "SetViewMode %i => %i" ), m_ViewMode, mode );
    if( mode != m_ViewMode )
    {
        Freeze();

        switch( m_ViewMode )
        {
            case guMEDIAVIEWER_MODE_LIBRARY :
            {
                m_LibPanel->Hide();
                break;
            }

            case guMEDIAVIEWER_MODE_ALBUMBROWSER :
            {
                m_AlbumBrowser->Hide();
                break;
            }

            case guMEDIAVIEWER_MODE_TREEVIEW :
            {
                m_TreeViewPanel->Hide();
                break;
            }

            case guMEDIAVIEWER_MODE_PLAYLISTS :
            {
                m_PlayListPanel->Hide();
                break;
            }
        }

        m_ViewMode = mode;

        switch( m_ViewMode )
        {
            case guMEDIAVIEWER_MODE_LIBRARY :
            {
                if( !m_LibPanel )
                {
                    CreateLibraryView();
                    GetSizer()->Add( m_LibPanel, 1, wxEXPAND|wxALL, 5 );
                }
                else
                {
                    m_LibPanel->Show( true );
                }
                break;
            }

            case guMEDIAVIEWER_MODE_ALBUMBROWSER :
            {
                if( !m_AlbumBrowser )
                {
                    CreateAlbumBrowserView();
                    GetSizer()->Add( m_AlbumBrowser, 1, wxEXPAND|wxALL, 5 );

                    int SelectedFilter = m_FilterChoice->GetSelection();
                    SetFilter( SelectedFilter ? m_DynFilterArray[ SelectedFilter - 1 ] : wxT( "" ) );
                }
                else
                {
                    m_AlbumBrowser->Show( true );
                }
                break;
            }

            case guMEDIAVIEWER_MODE_TREEVIEW :
            {
                if( !m_TreeViewPanel )
                {
                    CreateTreeView();
                    GetSizer()->Add( m_TreeViewPanel, 1, wxEXPAND|wxALL, 5 );
                }
                else
                {
                    m_TreeViewPanel->Show( true );
                }
                break;
            }

            case guMEDIAVIEWER_MODE_PLAYLISTS :
            {
                if( !m_PlayListPanel )
                {
                    CreatePlayListView();
                    GetSizer()->Add( m_PlayListPanel, 1, wxEXPAND|wxALL, 5 );
                }
                else
                {
                    m_PlayListPanel->Show( true );
                }
                break;
            }
        }

        m_FiltersSizer->Show( m_ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER );
        m_FilterChoice->Enable( m_ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER );
        m_AddFilterButton->Enable( m_ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER );
        m_DelFilterButton->Enable( ( m_ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER ) && ( m_FilterChoice->GetSelection() > 0 ) );
        m_EditFilterButton->Enable( ( m_ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER ) && ( m_FilterChoice->GetSelection() > 0 ) );

        m_LibrarySelButton->Enable( m_ViewMode != guMEDIAVIEWER_MODE_LIBRARY );
        m_AlbumBrowserSelButton->Enable( m_ViewMode != guMEDIAVIEWER_MODE_ALBUMBROWSER );
        m_TreeViewSelButton->Enable( m_ViewMode != guMEDIAVIEWER_MODE_TREEVIEW );
        m_PlaylistsSelButton->Enable( m_ViewMode != guMEDIAVIEWER_MODE_PLAYLISTS );

        DoTextSearch();

        Layout();

        Thaw();
    }

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );

    SetMenuState( m_ViewMode != guMEDIAVIEWER_MODE_NONE );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetMenuState( const bool enabled )
{
    wxMenuBar * MenuBar = m_MainFrame->GetMenuBar();
    if( MenuBar )
    {
        wxMenuItem * MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_COLLECTION );
        if( MenuItem )
        {
            MenuItem->Enable( true );
            MenuItem->Check( enabled );
        }

        bool IsEnabled = enabled && ( m_ViewMode == guMEDIAVIEWER_MODE_LIBRARY );
        int VisiblePanels = IsEnabled ? m_LibPanel->VisiblePanels() : 0;
        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIBRARY );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
            MenuItem->Check( IsEnabled );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_LABELS );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_LABELS ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_GENRES );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_GENRES ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_ARTISTS );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_ARTISTS ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_COMPOSERS );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_COMPOSERS ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_ALBUMARTISTS );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_ALBUMARTISTS ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_ALBUMS );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_ALBUMS ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_YEARS );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_YEARS ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_RATINGS );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_RATINGS ) );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_LIB_PLAYCOUNT );
        if( MenuItem )
        {
            MenuItem->Enable( IsEnabled );
            MenuItem->Check( IsEnabled && ( VisiblePanels & guPANEL_LIBRARY_PLAYCOUNT ) );
        }

        IsEnabled = m_ViewMode == guMEDIAVIEWER_MODE_ALBUMBROWSER;
        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_ALBUMBROWSER );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
            MenuItem->Check( IsEnabled );
        }

        IsEnabled = m_ViewMode == guMEDIAVIEWER_MODE_TREEVIEW;
        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_TREEVIEW );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
            MenuItem->Check( IsEnabled );
        }

        IsEnabled = m_ViewMode == guMEDIAVIEWER_MODE_PLAYLISTS;
        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_PLAYLISTS );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
            MenuItem->Check( IsEnabled );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_ADD_PATH );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_IMPORT );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_RESCAN_LIBRARY );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_SEARCH_COVERS );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
        }

        MenuItem = MenuBar->FindItem( m_BaseCommand + guCOLLECTION_ACTION_VIEW_PROPERTIES );
        if( MenuItem )
        {
            MenuItem->Enable( enabled );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnSearchActivated( wxCommandEvent& event )
{
    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();

    if( m_DoneClearSearchText )
    {
        m_DoneClearSearchText = false;
        return;
    }

    if( !m_InstantSearchEnabled )
        return;

    m_TextChangedTimer.Start( guMEDIAVIEWER_TIMER_TEXTCHANGED, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnSearchCancelled( wxCommandEvent &event ) // CLEAN SEARCH STR
{
    guLogMessage( wxT( "OnSearchCancelled..." ) );

    m_SearchTextCtrl->Clear();
    m_SearchText.Empty();

    if( !m_InstantSearchEnabled )
        DoTextSearch();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnSearchSelected( wxCommandEvent& event )
{
    guLogMessage( wxT( "OnSearchSelected... %i  %i" ), m_EnterSelectSearchEnabled, m_InstantSearchEnabled );
    // perform text search immediately
    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();

    m_SearchText = m_SearchTextCtrl->GetLineText( 0 );

    if( !DoTextSearch() || m_EnterSelectSearchEnabled || !m_InstantSearchEnabled )
        return;

    // if text search was successful, possibly enqueue results
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        PlayAllTracks( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false, CONFIG_PATH_GENERAL ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::ClearSearchText( void )
{
    guLogMessage( wxT( "ClearSearchText..." ) );
    m_DoneClearSearchText = true;
    m_SearchTextCtrl->Clear();
    m_SearchText.Empty();
    m_SearchTextCtrl->ShowCancelButton( false );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::GoToSearch( void )
{
    if( FindFocus() != m_SearchTextCtrl )
        m_SearchTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnTextChangedTimer( wxTimerEvent &event )
{
    guLogMessage( wxT( "OnTextChangedTimer..." ) );
    m_SearchText = m_SearchTextCtrl->GetLineText( 0 );
    DoTextSearch();
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::DoTextSearch( void )
{
    Freeze();
    bool RetVal = false;
    wxString SearchText = m_SearchText;
    switch( m_ViewMode )
    {
        case guMEDIAVIEWER_MODE_LIBRARY :
        {
            RetVal = m_LibPanel->DoTextSearch( SearchText );
            break;
        }

        case guMEDIAVIEWER_MODE_ALBUMBROWSER :
        {
            RetVal = m_AlbumBrowser->DoTextSearch( SearchText );
            break;
        }

        case guMEDIAVIEWER_MODE_TREEVIEW :
        {
            RetVal = m_TreeViewPanel->DoTextSearch( SearchText );
            break;
        }

        case guMEDIAVIEWER_MODE_PLAYLISTS :
        {
            RetVal = m_PlayListPanel->DoTextSearch( SearchText );
            break;
        }
    }

    if( RetVal )
    {
        m_SearchTextCtrl->ShowCancelButton( true );
    }
    else
    {
        if( SearchText.IsEmpty() )
        {
            m_SearchTextCtrl->ShowCancelButton( false );
        }
    }

    Thaw();
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::PlayAllTracks( const bool enqueue )
{
    switch( m_ViewMode )
    {
        case guMEDIAVIEWER_MODE_LIBRARY :
        {
            m_LibPanel->PlayAllTracks( enqueue );
            break;
        }

        case guMEDIAVIEWER_MODE_ALBUMBROWSER :
        {
            break;
        }

        case guMEDIAVIEWER_MODE_TREEVIEW :
        {
            break;
        }

        case guMEDIAVIEWER_MODE_PLAYLISTS :
        {
            break;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnAddFilterClicked( wxCommandEvent &event )
{
    guDynPlayList NewPlayList;

    NewPlayList.m_Name = _( "New Filter" );

    guDynPlayListEditor * PlayListEditor = new guDynPlayListEditor( this, &NewPlayList, true );
    if( PlayListEditor->ShowModal() == wxID_OK )
    {
        PlayListEditor->FillPlayListEditData();

        m_DynFilterArray.Add( escape_configlist_str( NewPlayList.m_Name ) + wxT( ":" ) + NewPlayList.ToString() );
        m_FilterChoice->Append( NewPlayList.m_Name );
        m_FilterChoice->GetContainingSizer()->Layout();

        m_FilterChoice->SetSelection( m_FilterChoice->GetCount() - 1 );
        event.SetInt( m_FilterChoice->GetCount() - 1 );
        OnFilterSelected( event );
    }

    PlayListEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnDelFilterClicked( wxCommandEvent &event )
{
    int Selected = m_FilterChoice->GetSelection();
    if( Selected )
    {
        m_DynFilterArray.RemoveAt( Selected  - 1 );
        m_FilterChoice->Delete( Selected );
        m_FilterChoice->GetContainingSizer()->Layout();

        m_FilterChoice->SetSelection( 0 );
        event.SetInt( 0 );
        OnFilterSelected( event );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnEditFilterClicked( wxCommandEvent &event )
{
    int Selected = m_FilterChoice->GetSelection();
    wxString CurFilterStr = m_DynFilterArray[ Selected - 1 ];

    guDynPlayList EditPlayList;
    EditPlayList.m_Name = CurFilterStr.BeforeFirst( wxT( ':' ) );
    EditPlayList.FromString( CurFilterStr.AfterFirst( wxT( ':' ) ) ); // = m_DynFilter;

    guDynPlayListEditor * PlayListEditor = new guDynPlayListEditor( this, &EditPlayList, true );
    if( PlayListEditor->ShowModal() == wxID_OK )
    {
        PlayListEditor->FillPlayListEditData();

        if( !EditPlayList.IsEmpty() )
        {
            m_DynFilterArray[ Selected - 1 ] = escape_configlist_str( EditPlayList.m_Name ) + wxT( ":" ) + EditPlayList.ToString();
            m_FilterChoice->SetString( Selected, EditPlayList.m_Name );

            SetFilter( m_DynFilterArray[ Selected - 1 ] );
        }
        else    // This should never happen
        {
            guLogError( wxT( "Empty dynamic playlit?" ) );
        }
    }

    PlayListEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnFilterSelected( wxCommandEvent &event )
{
    int Selected = event.GetInt();
    if( Selected )
    {
        m_DelFilterButton->Enable( true );
        m_EditFilterButton->Enable( true );

        SetFilter( m_DynFilterArray[ Selected - 1 ] );
    }
    else
    {
        m_DelFilterButton->Enable( false );
        m_EditFilterButton->Enable( false );
        SetFilter( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetFilter( const wxString &filterstr )
{
    if( m_AlbumBrowser )
    {
        m_AlbumBrowser->SetFilter( filterstr );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu * Menu = new wxMenu();
    wxMenuItem * MenuItem;

    int BaseCommand = GetBaseCommand();
    int Type = GetType();

    if( ( Type == guMEDIA_COLLECTION_TYPE_NORMAL ) ||
        ( Type == guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE ) ||
        ( Type == guMEDIA_COLLECTION_TYPE_IPOD ) )
    {
        MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_ADD_PATH, _( "Add Path" ), _( "Add path to the collection" ), wxITEM_NORMAL );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_IMPORT, _( "Import Files" ), _( "Import files into the collection" ), wxITEM_NORMAL );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY, _( "Update" ), _( "Update the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_RESCAN_LIBRARY, _( "Rescan" ), _( "Rescan the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_SEARCH_COVERS, _( "Search Covers" ), _( "Search the collection missing covers" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_VIEW_PROPERTIES, _( "Properties" ), _( "Show collection properties" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( Menu, _( "Collection" ) );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::CreateCopyToMenu( wxMenu * menu )
{
    m_MainFrame->CreateCopyToMenu( menu );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::ShowPanel( const int id, const bool enabled )
{
    if( m_ViewMode == guMEDIAVIEWER_MODE_LIBRARY )
    {
        m_LibPanel->ShowPanel( m_LibPanel->PanelId( id - guCOLLECTION_ACTION_VIEW_LIB_LABELS ), enabled );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::HandleCommand( const int command )
{
    switch( command )
    {
        case guCOLLECTION_ACTION_UPDATE_LIBRARY :
        {
            UpdateLibrary();
            break;
        }

        case guCOLLECTION_ACTION_RESCAN_LIBRARY :
        {
            UpgradeLibrary();
            break;
        }

        case guCOLLECTION_ACTION_SEARCH_COVERS :
        {
            UpdateCovers();
            break;
        }

        case guCOLLECTION_ACTION_ADD_PATH :
        {
            OnAddPath();
            break;
        }

        case guCOLLECTION_ACTION_IMPORT :
        {
            ImportFiles();
            break;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::EditProperties( void )
{
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_LIBRARY );
    wxPostEvent( m_MainFrame, CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpgradeLibrary( void )
{
    m_MediaCollection->m_LastUpdate = 0;

    UpdateLibrary();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdateLibrary( const wxString &path )
{
    int GaugeId;
    GaugeId = m_MainFrame->AddGauge( m_MediaCollection->m_Name, false );

    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guLibUpdateThread( this, GaugeId, path );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdateFinished( void )
{
    if( m_UpdateThread )
        m_UpdateThread = NULL;

    SetLastUpdate();

    CleanLibrary();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::CleanLibrary( void )
{
    if( m_CleanThread )
    {
        m_CleanThread->Pause();
        m_CleanThread->Delete();
    }

    m_CleanThread = new guLibCleanThread( this );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::CleanFinished( void )
{
    m_CleanThread = NULL;

    LibraryUpdated();
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetLastUpdate( void )
{
    m_MediaCollection->m_LastUpdate = wxDateTime::Now().GetTicks();

    guMediaCollection * Collection = m_MainFrame->FindCollection( m_MediaCollection->m_UniqueId );
    if( Collection )
    {
        Collection->m_LastUpdate = m_MediaCollection->m_LastUpdate;
        m_MainFrame->SaveCollections();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::LibraryUpdated( void )
{
    if( m_LibPanel )
    {
        m_LibPanel->ReloadControls();
    }

    if( m_AlbumBrowser )
    {
        m_AlbumBrowser->RefreshAll();
    }

    if( m_TreeViewPanel )
    {
        m_TreeViewPanel->RefreshAll();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdateCovers( void )
{
    int GaugeId = m_MainFrame->AddGauge( m_MediaCollection->m_Name, false );

    if( m_UpdateCoversThread )
    {
        m_UpdateCoversThread->Pause();
        m_UpdateCoversThread->Delete();
    }

    m_UpdateCoversThread = new guUpdateCoversThread( this, GaugeId );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdateCoversFinished( void )
{
    if( m_UpdateCoversThread )
        m_UpdateCoversThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnAddPath( void )
{
    wxDirDialog * DirDialog = new wxDirDialog( this, _( "Select library path" ), wxGetHomeDir() );
    if( DirDialog )
    {
        if( DirDialog->ShowModal() == wxID_OK )
        {
            wxString PathValue = DirDialog->GetPath();
            if( !PathValue.IsEmpty() )
            {
                if( !PathValue.EndsWith( wxT( "/" ) ) )
                    PathValue += '/';

                //guLogMessage( wxT( "LibPaths: '%s'" ), LibPaths[ 0 ].c_str() );
                //guLogMessage( wxT( "Add Path: '%s'" ), PathValue.c_str() );
                //guLogMessage( wxT( "Exists  : %i" ), m_Db->PathExists( PathValue ) );
                //guLogMessage( wxT( "Index   : %i" ), LibPaths.Index( PathValue ) );

                if( !CheckFileLibPath( m_MediaCollection->m_Paths, PathValue ) )
                {
                    m_MediaCollection->m_Paths.Add( PathValue );
                    guMediaCollection * Collection = m_MainFrame->FindCollection( m_MediaCollection->m_UniqueId );
                    if( Collection )
                    {
                        Collection->m_Paths.Add( PathValue );
                        m_MainFrame->SaveCollections();
                    }

                    UpdateLibrary();
                }
                else
                {
                    wxMessageBox( _( "This Path is already in the library" ),
                        _( "Adding path error" ),
                        wxICON_EXCLAMATION | wxOK  );
                }
            }
        }
        DirDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::ImportFiles( guTrackArray * tracks )
{
    if( tracks )
    {
        guImportFiles * ImportFiles = new guImportFiles( this, this, tracks );
        if( ImportFiles )
        {
            if( ImportFiles->ShowModal() == wxID_OK  )
            {
                m_MainFrame->ImportFiles( this, tracks, ImportFiles->GetCopyToOption(), ImportFiles->GetCopyToPath() );
            }
            else
            {
                delete tracks;
            }
            ImportFiles->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void AddImportFiles( guTrackArray * tracks, const wxString &filename )
{
    //guLogMessage( wxT( "AddImportFiles : '%s'" ), filename.c_str() );
    if( wxFileExists( filename ) )
    {
        if( guIsValidAudioFile( filename.Lower() ) )
        {
            guTrack * Track = new guTrack();
            if( Track->ReadFromFile( filename ) )
            {
                Track->m_Type = guTRACK_TYPE_NOTDB;
                tracks->Add( Track );
            }
            else
            {
                delete Track;
            }
        }
    }
    else if( wxDirExists( filename ) )
    {
        wxString FileName;
        wxString DirName = filename;
        wxDir Dir;
        if( !DirName.EndsWith( wxT( "/" ) ) )
            DirName += wxT( "/" );

        Dir.Open( DirName );
        if( Dir.IsOpened() )
        {
            if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
            {
                do {
                    if( ( FileName[ 0 ] != '.' ) )
                    {
                        AddImportFiles( tracks, DirName + FileName );
                    }
                } while( Dir.GetNext( &FileName ) );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::ImportFiles( const wxArrayString &files )
{
    int Index;
    int Count = files.Count();
    if( Count )
    {
        guTrackArray * Tracks = new guTrackArray();
        for( Index = 0; Index < Count; Index++ )
        {
            AddImportFiles( Tracks, files[ Index ] );
        }
        ImportFiles( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SaveLayout( wxXmlNode * xmlnode )
{
    // MediaViewer
    //
    wxXmlNode * XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "mediaviewer" ) );

    wxXmlAttribute * Property = new wxXmlAttribute( wxT( "name" ), m_MediaCollection->m_Name,
                               new wxXmlAttribute( wxT( "id" ), m_MediaCollection->m_UniqueId,
                               new wxXmlAttribute( wxT( "mode" ), wxString::Format( wxT( "%i" ), m_ViewMode ),
                               NULL ) ) );

    XmlNode->SetAttributes( Property );

    if( m_LibPanel )
    {
        m_LibPanel->SaveLayout( XmlNode, wxT( "library" ) );
    }

    if( m_TreeViewPanel )
    {
        m_TreeViewPanel->SaveLayout( XmlNode, wxT( "tree" ) );
    }

    if( m_PlayListPanel )
    {
        m_PlayListPanel->SaveLayout( XmlNode, wxT( "playlist" ) );
    }

    xmlnode->AddChild( XmlNode );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::LoadLayout( wxXmlNode * xmlnode )
{
    if( xmlnode && xmlnode->GetName() == wxT( "mediaviewer" ) )
    {
        wxString Mode;
        long     ViewMode;
        xmlnode->GetAttribute( wxT( "mode" ), &Mode );

        wxXmlNode * XmlNode = xmlnode->GetChildren();
        while( XmlNode )
        {
            wxString NodeName = XmlNode->GetName();
            if( NodeName == wxT( "library" ) )
            {
                if( !m_LibPanel )
                {
                    m_LibPanel = new guLibPanel( this, this );
                    m_LibPanel->SetBaseCommand( m_BaseCommand + 1 );
                    GetSizer()->Add( m_LibPanel, 1, wxEXPAND, 5 );
                }
                m_LibPanel->Hide();
                m_LibPanel->LoadLayout( XmlNode );
            }
            else if( NodeName == wxT( "tree" ) )
            {
                if( !m_TreeViewPanel )
                {
                    m_TreeViewPanel = new guTreeViewPanel( this, this );
                    GetSizer()->Add( m_TreeViewPanel, 1, wxEXPAND, 5 );
                }
                m_TreeViewPanel->Hide();
                m_TreeViewPanel->LoadLayout( XmlNode );
            }
            else if( NodeName == wxT( "playlist" ) )
            {
                if( !m_PlayListPanel )
                {
                    m_PlayListPanel = new guPlayListPanel( this, this );
                    GetSizer()->Add( m_PlayListPanel, 1, wxEXPAND, 5 );
                }
                m_PlayListPanel->Hide();
                m_PlayListPanel->LoadLayout( XmlNode );
            }

            XmlNode = XmlNode->GetNext();
        }

        if( Mode.ToLong( &ViewMode ) )
        {
            SetViewMode( guMEDIAVIEWER_MODE_NONE );
            SetViewMode( ViewMode );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewer::GetSelInfo( void )
{
    wxLongLong SelCount = 0;
    wxLongLong SelLength = 0;
    wxLongLong SelSize = 0;

    switch( m_ViewMode )
    {
        case guMEDIAVIEWER_MODE_LIBRARY :
        {
            m_Db->GetTracksCounters( &SelCount, &SelLength, &SelSize );

            wxString SelInfo = wxString::Format( wxT( "%llu " ), SelCount.GetValue() );
            SelInfo += SelCount == 1 ? _( "track" ) : _( "tracks" );
            SelInfo += wxString::Format( wxT( ",   %s,   %s" ),
                LenToString( SelLength.GetValue() ).c_str(),
                SizeToString( SelSize.GetValue() ).c_str() );

            return SelInfo;
        }

        case guMEDIAVIEWER_MODE_ALBUMBROWSER :
        {
            return m_AlbumBrowser->GetSelInfo();
        }

        case guMEDIAVIEWER_MODE_TREEVIEW :
        {
            m_TreeViewPanel->GetTreeViewCounters( &SelCount, &SelLength, &SelSize );
            if( SelCount > 0 )
            {
                wxString SelInfo = wxString::Format( wxT( "%llu " ), SelCount.GetValue() );
                SelInfo += SelCount == 1 ? _( "track" ) : _( "tracks" );
                SelInfo += wxString::Format( wxT( ",   %s,   %s" ),
                    LenToString( SelLength.GetValue() ).c_str(),
                    SizeToString( SelSize.GetValue() ).c_str() );

                return SelInfo;
            }
            break;
        }

        case guMEDIAVIEWER_MODE_PLAYLISTS :
        {
            if( m_PlayListPanel->GetPlayListCounters( &SelCount, &SelLength, &SelSize ) )
            {
                wxString SelInfo = wxString::Format( wxT( "%llu " ), SelCount.GetValue() );
                SelInfo += SelCount == 1 ? _( "track" ) : _( "tracks" );
                SelInfo += wxString::Format( wxT( ",   %s,   %s" ),
                    LenToString( SelLength.GetValue() ).c_str(),
                    SizeToString( SelSize.GetValue() ).c_str() );
                return SelInfo;
            }
            break;
        }
    }

    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdatedTrack( const int updatedby, const guTrack * track )
{
    if( m_LibPanel )
        m_LibPanel->UpdatedTrack( track );

    if( m_TreeViewPanel )
        m_TreeViewPanel->UpdatedTrack( track );

    if( m_PlayListPanel )
            m_PlayListPanel->UpdatedTrack( track );

    //if( m_AlbumBrowser )
    //        m_AlbumBrowser->UpdatedTracks( tracks );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdatedTracks( const int updatedby, const guTrackArray * tracks )
{
    if( m_LibPanel )
        m_LibPanel->UpdatedTracks( tracks );

    if( m_TreeViewPanel )
        m_TreeViewPanel->UpdatedTracks( tracks );

    if( m_PlayListPanel )
            m_PlayListPanel->UpdatedTracks( tracks );

    if( m_MainFrame )
    {
        m_MainFrame->UpdatedTracks( guUPDATED_TRACKS_MEDIAVIEWER, tracks );
    }
    //if( m_AlbumBrowser )
    //        m_AlbumBrowser->UpdatedTracks( tracks );
}

// -------------------------------------------------------------------------------- //
int guMediaViewer::CopyTo( guTrack * track, guCopyToAction &copytoaction, wxString &filename, const int index )
{
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::DownloadAlbumCover( const int albumid )
{
    wxString AlbumName;
    wxString ArtistName;
    wxString AlbumPath;
    if( !m_Db->GetAlbumInfo( albumid, &AlbumName, &ArtistName, &AlbumPath ) )
    {
        wxMessageBox( _( "Could not find the Album in the songs library.\n"\
                         "You should update the library." ), _( "Error" ), wxICON_ERROR | wxOK );
        return;
    }

    AlbumName = RemoveSearchFilters( AlbumName );

    guCoverEditor * CoverEditor = new guCoverEditor( this, ArtistName, AlbumName );
    if( CoverEditor )
    {
        if( CoverEditor->ShowModal() == wxID_OK )
        {
            //guLogMessage( wxT( "About to download cover from selected url" ) );
            wxImage * CoverImage = CoverEditor->GetSelectedCoverImage();
            if( CoverImage )
            {
                wxArrayString SearchCovers = GetCoverWords();
                wxString CoverName = AlbumPath + ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );
                CoverImage->SaveFile( CoverName, wxBITMAP_TYPE_JPEG );

                SetAlbumCover( albumid, CoverName );

                if( CoverEditor->EmbedToFiles() )
                {
                    EmbedAlbumCover( albumid );
                }
            }
        }
        CoverEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SelectAlbumCover( const int albumid )
{
    guSelCoverFile * SelCoverFile = new guSelCoverFile( this, this, albumid );
    if( SelCoverFile )
    {
        if( SelCoverFile->ShowModal() == wxID_OK )
        {
            wxString CoverFile = SelCoverFile->GetSelFile();
            if( !CoverFile.IsEmpty() )
            {
                if( SetAlbumCover( albumid, SelCoverFile->GetAlbumPath(), CoverFile ) )
                {
                    if( SelCoverFile->EmbedToFiles() )
                    {
                        EmbedAlbumCover( albumid );
                    }
                }
            }
        }
        SelCoverFile->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::EmbedAlbumCover( const int albumid )
{
    wxImage * CoverImage = NULL;
    int CoverId = m_Db->GetAlbumCoverId( albumid );
    if( CoverId > 0 )
    {
        wxString CoverPath = m_Db->GetCoverPath( CoverId );

        CoverImage = new wxImage( CoverPath, wxBITMAP_TYPE_ANY );
        if( CoverImage )
        {
            if( !CoverImage->IsOk() )
            {
                delete CoverImage;
                return;
            }

            guImageResize( CoverImage, 600 );
        }
        else
            return;
    }

    guTrackArray AlbumTracks;
    wxArrayInt Albums;
    Albums.Add( albumid );
    m_Db->GetAlbumsSongs( Albums, &AlbumTracks );
    NormalizeTracks( &AlbumTracks );

    int TrackIndex;
    int TrackCount;
    if( ( TrackCount = AlbumTracks.Count() ) )
    {
        for( TrackIndex = 0; TrackIndex < TrackCount; TrackIndex++ )
        {
            if( !AlbumTracks[ TrackIndex ].m_Offset )
                guTagSetPicture( AlbumTracks[ TrackIndex ].m_FileName, CoverImage );
        }
    }

    if( CoverImage )
        delete CoverImage;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::SetAlbumCover( const int albumid, const wxString &coverpath, const bool update )
{
    m_Db->SetAlbumCover( albumid, coverpath );

    if( update )
        AlbumCoverChanged( albumid );

    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewer::GetCoverName( const int albumid )
{
    return m_MediaCollection->m_CoverWords.Count() ? m_MediaCollection->m_CoverWords[ 0 ] : wxT( "cover" );
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg )
{
    wxString CoverName = albumpath + GetCoverName( albumid ) + wxT( ".jpg" );

    int MaxSize = GetCoverMaxSize();
    if( MaxSize )
    {
        coverimg->Rescale( MaxSize, MaxSize, wxIMAGE_QUALITY_HIGH );
    }

    if( coverimg->SaveFile( CoverName, wxBITMAP_TYPE_JPEG ) )
    {
        SetAlbumCover( albumid, CoverName );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath )
{
    wxString CoverName = albumpath + GetCoverName( albumid ) + wxT( ".jpg" );
    int MaxSize = GetCoverMaxSize();

    wxURI Uri( coverpath );
    if( Uri.IsReference() )
    {
        wxImage CoverImage( coverpath );
        if( CoverImage.IsOk() )
        {
            if( MaxSize )
            {
                CoverImage.Rescale( MaxSize, MaxSize, wxIMAGE_QUALITY_HIGH );
            }

            if( ( coverpath == CoverName ) || CoverImage.SaveFile( CoverName, wxBITMAP_TYPE_JPEG ) )
            {
                SetAlbumCover( albumid, CoverName );
                return true;
            }
        }
        else
        {
            guLogError( wxT( "Could not load the image '%s'" ), coverpath.c_str() );
        }
    }
    else
    {
        if( DownloadImage( coverpath, CoverName, GetCoverType(), MaxSize, MaxSize ) )
        {
            SetAlbumCover( albumid, CoverName );
            return true;
        }
        else
        {
            guLogError( wxT( "Failed to download file '%s'" ), coverpath.c_str() );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::DeleteAlbumCover( const int albumid )
{
    int CoverId = m_Db->GetAlbumCoverId( albumid );
    if( CoverId > 0 )
    {
        wxString CoverPath = m_Db->GetCoverPath( CoverId );
        if( !wxRemoveFile( CoverPath ) )
        {
            guLogError( wxT( "Could not remove the cover file '%s'" ), CoverPath.c_str() );
        }
    }

    SetAlbumCover( albumid, wxEmptyString, false );

    AlbumCoverChanged( albumid, true );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::DeleteAlbumCover( const wxArrayInt &albumids )
{
    int Index;
    int Count = albumids.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        DeleteAlbumCover( albumids[ Index ] );
    }

    if( Count )
        AlbumCoverChanged( albumids[ 0 ], true );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::AlbumCoverChanged( const int albumid, const bool deleted )
{
    if( m_LibPanel )
    {
        m_LibPanel->AlbumCoverChanged();
    }

    if( m_AlbumBrowser )
    {
        m_AlbumBrowser->AlbumCoverChanged( albumid );
    }

    wxCommandEvent evt( wxEVT_MENU, ID_ALBUM_COVER_CHANGED );
    evt.SetInt( albumid );
    evt.SetExtraLong( deleted );
    evt.SetClientData( this );
    wxPostEvent( m_MainFrame, evt );
}

// -------------------------------------------------------------------------------- //
wxImage * guMediaViewer::GetAlbumCover( const int albumid, int &coverid, wxString &coverpath,
            const wxString &artistname, const wxString &albumname )
{
    if( !coverid )
        coverid = m_Db->GetAlbumCoverId( albumid );
    if( coverid )
    {
        coverpath = m_Db->GetCoverPath( coverid );
        if( wxFileExists( coverpath ) )
        {
            wxImage * CoverImage = new wxImage( coverpath, wxBITMAP_TYPE_JPEG );
            if( CoverImage )
            {
                if( CoverImage->IsOk() )
                {
                    return CoverImage;
                }
                delete CoverImage;
            }
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::FindMissingCover( const int albumid, const wxString &artistname,
                        const wxString &albumname, const wxString &albumpath  )
{
    bool RetVal = false;

    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        wxString AlbumName;
        guAlbumInfo AlbumInfo;

        // Remove from album name expressions like (cd1),(cd2) etc
        AlbumName = RemoveSearchFilters( albumname );

        AlbumInfo = LastFM->AlbumGetInfo( artistname, albumname );

        // Try to download the cover
        if( LastFM->IsOk() )
        {
            if( !AlbumInfo.m_ImageLink.IsEmpty() )
            {
                //guLogMessage( wxT( "ImageLink : %s" ), AlbumInfo.ImageLink.c_str() );
                // Save the cover into the target directory.
                // Changed to DownloadImage to convert all images to jpg
                wxString CoverName = albumpath + GetCoverName( albumid ) + wxT( ".jpg" );
                if( !DownloadImage( AlbumInfo.m_ImageLink, CoverName ) )
                {
                    guLogWarning( wxT( "Could not download cover file" ) );
                }
                else
                {
//                    DownloadedCovers++;
                    m_Db->SetAlbumCover( albumid, CoverName );
                    //guLogMessage( wxT( "Cover file downloaded for %s - %s" ), Artist.c_str(), AlbumName.c_str() );
                    RetVal = true;
                }
            }
        }
        else
        {
            // There was en error...
            guLogError( wxT( "Error getting the cover for %s - %s (%u)" ),
                     artistname.c_str(),
                     albumname.c_str(),
                     LastFM->GetLastError() );
        }
        delete LastFM;
    }

    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetSelection( const int type, const int id )
{
    if( m_ViewMode != guMEDIAVIEWER_MODE_LIBRARY )
        SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );

    guLogMessage( wxT( "SetSelection( %i, %i )" ), type, id );
    m_LibPanel->SetSelection( type, id );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::PlayListUpdated( void )
{
    if( IsDefault() )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_PLAYLIST_UPDATED );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
bool guIsDirectoryEmpty( const wxString &path )
{
    wxString FileName;
    wxString Path= path;
    wxDir Dir;

    if( !Path.EndsWith( wxT( "/" ) ) )
        Path += wxT( "/" );

    Dir.Open( Path );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
        {
            do {
                if( FileName == wxT( "." ) ||
                    FileName == wxT( ".." ) )
                    continue;
                if( Dir.Exists( Path + FileName ) )
                {
                    if( !guIsDirectoryEmpty( Path + FileName ) )
                        return false;
                }
                else
                {
                    return false;
                }
            } while( Dir.GetNext( &FileName ) );
        }
    }

    return true;
}

// -------------------------------------------------------------------------------- //
bool guRmDirRecursive( const wxString &path )
{
    wxString FileName;
    wxString Path= path;
    wxDir Dir;

    if( !Path.EndsWith( wxT( "/" ) ) )
        Path += wxT( "/" );

    Dir.Open( Path );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
        {
            do {
                if( FileName == wxT( "." ) ||
                    FileName == wxT( ".." ) )
                    continue;
                if( Dir.Exists( Path + FileName ) )
                {
                    if( !guRmDirRecursive( Path + FileName ) )
                        return false;
                }
                else
                {
                    return false;
                }
            } while( Dir.GetNext( &FileName ) );
        }
    }

    if( !wxRmdir( Path ) )
    {
        guLogMessage( wxT( "Could not delete the dir %s" ), Path.c_str() );
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::DeleteTracks( const guTrackArray * tracks )
{
    m_Db->DeleteLibraryTracks( tracks, false );
    //
    wxArrayString DeletePaths;
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &CurTrack = tracks->Item( Index );
        if( CurTrack.m_Offset )
            continue;

        if( DeletePaths.Index( wxPathOnly( CurTrack.m_FileName ) ) == wxNOT_FOUND )
        {
            DeletePaths.Add( wxPathOnly( CurTrack.m_FileName ) );
        }

        if( !wxRemoveFile( CurTrack.m_FileName ) )
        {
            guLogMessage( wxT( "Error deleting '%s'" ), CurTrack.m_FileName.c_str() );
        }
        guLogMessage( wxT( "Deleted '%s'" ), CurTrack.m_FileName.c_str() );
    }

    if( ( Count = DeletePaths.Count() ) )
    {
        wxArrayString LibPaths = GetPaths();
        for( Index = 0; Index < ( int ) LibPaths.Count(); Index++ )
        {
            guLogMessage( wxT( "Libath: %s"), LibPaths[ Index ].c_str() );
        }

        for( Index = 0; Index < Count; Index++ )
        {
            wxString CurPath = DeletePaths[ Index ];
            while( !CurPath.IsEmpty() && LibPaths.Index( CurPath + wxT( "/" ) ) == wxNOT_FOUND )
            {
                guLogMessage( wxT( "Deleting '%s'" ), CurPath.c_str() );
                if( guIsDirectoryEmpty( CurPath ) )
                {
                    if( !guRmDirRecursive( CurPath ) )
                    {
                        guLogMessage( wxT( "Error removing dir '%s'" ), DeletePaths[ Index ].c_str() );
                        break;
                    }
                }
                else
                {
                    break;
                }
                CurPath = CurPath.BeforeLast( wxT( '/' ) );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                    const wxArrayString &lyrics, const wxArrayInt &changedflags )
{
    guUpdateTracks( tracks, images, lyrics, changedflags );
    m_Db->UpdateSongs( &tracks, changedflags );
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnGenreSetSelection( wxCommandEvent &event )
{
    wxArrayInt * genres = ( wxArrayInt * ) event.GetClientData();
    if( genres )
    {
        SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );
        m_LibPanel->SelectGenres( genres );
        delete genres;
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnArtistSetSelection( wxCommandEvent &event )
{
    wxArrayInt * artists = ( wxArrayInt * ) event.GetClientData();
    if( artists )
    {
        SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );
        m_LibPanel->SelectArtists( artists );
        delete artists;
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnAlbumArtistSetSelection( wxCommandEvent &event )
{
    wxArrayInt * Ids = ( wxArrayInt * ) event.GetClientData();
    if( Ids )
    {
        SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );
        m_LibPanel->SelectAlbumArtists( Ids );
        delete Ids;
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnComposerSetSelection( wxCommandEvent &event )
{
    wxArrayInt * Ids = ( wxArrayInt * ) event.GetClientData();
    if( Ids )
    {
        SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );
        m_LibPanel->SelectComposers( Ids );
        delete Ids;
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnAlbumSetSelection( wxCommandEvent &event )
{
    wxArrayInt * albums = ( wxArrayInt * ) event.GetClientData();
    if( albums )
    {
        SetViewMode( guMEDIAVIEWER_MODE_LIBRARY );
        m_LibPanel->SelectAlbums( albums );
        delete albums;
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnUpdateLabels( wxCommandEvent &event )
{
    if( m_LibPanel )
    {
        m_LibPanel->UpdateLabels();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::UpdatePlaylists( void )
{
    wxCommandEvent evt( wxEVT_MENU, ID_PLAYLIST_UPDATED );
    wxPostEvent( m_MainFrame, evt );

    if( m_PlayListPanel )
    {
        m_PlayListPanel->UpdatePlaylists();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::SetTracksRating( guTrackArray &tracks, const int rating )
{
    int Index;
    int Count;
    if( ( Count = tracks.Count() ) )
    {
        wxArrayInt ChangedFlags;
        ChangedFlags.Add( guTRACK_CHANGED_DATA_RATING, Count );

        for( Index = 0; Index < Count; Index++ )
        {
            tracks[ Index ].m_Rating = rating;
        }

        if( GetEmbeddMetadata() )
        {
            guImagePtrArray Images;
            wxArrayString Lyrics;
            guUpdateTracks( tracks, Images, Lyrics, ChangedFlags );
        }

        m_Db->SetTracksRating( &tracks, rating );

        UpdatedTracks( guUPDATED_TRACKS_NONE, &tracks );
    }
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewer::AudioPath( void )
{
    if( m_CopyToPattern && !m_CopyToPattern->m_Path.IsEmpty() )
    {
        return m_CopyToPattern->m_Path;
    }

    if( m_MediaCollection->m_Paths.Count() )
        return m_MediaCollection->m_Paths[ 0 ];

    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewer::Pattern( void )
{
    if( m_CopyToPattern )
    {
        return m_CopyToPattern->m_Pattern;
    }

    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
int guMediaViewer::AudioFormats( void )
{
    return guPORTABLEMEDIA_AUDIO_FORMAT_ALL;
}

// -------------------------------------------------------------------------------- //
int guMediaViewer::TranscodeFormat( void )
{
    if( m_CopyToPattern )
    {
        return m_CopyToPattern->m_Format;
    }

    return guTRANSCODE_FORMAT_KEEP;
}

// -------------------------------------------------------------------------------- //
int guMediaViewer::TranscodeScope( void )
{
    return guPORTABLEMEDIA_TRANSCODE_SCOPE_NOT_SUPPORTED;
}

// -------------------------------------------------------------------------------- //
int guMediaViewer::TranscodeQuality( void )
{
    if( m_CopyToPattern )
    {
        return m_CopyToPattern->m_Quality;
    }

    return guTRANSCODE_QUALITY_KEEP;
}

// -------------------------------------------------------------------------------- //
int guMediaViewer::PlaylistFormats( void )
{
    return 0;
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewer::PlaylistPath( void )
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewer::MoveFiles( void )
{
    if( m_CopyToPattern )
    {
        return m_CopyToPattern->m_MoveFiles;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::OnSmartAddTracks( wxCommandEvent &event )
{
    guLogMessage( wxT( "guMediaViewer::OnSmartAddTracks" ) );
    guTrackArray * Tracks = ( guTrackArray * ) event.GetClientData();
    if( Tracks )
    {
        wxArrayInt TrackIds;
        int Index;
        int Count = Tracks->Count();
        for( Index = 0; Index < Count; Index++ )
        {
            TrackIds.Add( Tracks->Item( Index ).m_SongId );
        }

        guListItems PlayLists;

        if( m_SmartPlaylistId == wxNOT_FOUND )
        {
            m_Db->CreateStaticPlayList( m_SmartPlaylistName, TrackIds );
        }
        else
        {
            m_Db->AppendStaticPlayList( m_SmartPlaylistId, TrackIds );
        }

        UpdatePlaylists();

        delete Tracks;
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewer::CreateSmartPlaylist( const wxString &artistname, const wxString &trackname )
{
    wxString PlaylistName = artistname + wxT( " " ) + trackname + wxT( " " );
    PlaylistName += _( " smart playlist" );
    guGenSmartPlaylist * GenSmartPlaylist = new guGenSmartPlaylist( this, this, PlaylistName );
    if( GenSmartPlaylist )
    {
        if( GenSmartPlaylist->ShowModal() == wxID_OK )
        {
            m_SmartArtistsList.Clear();
            m_SmartTracksList.Clear();

            int GaugeId = m_MainFrame->AddGauge( _( "Playlist" ) );

            guSmartModeThread * SmartModeThread = new guSmartModeThread( m_Db, this,
                    artistname, trackname,
                    &m_SmartTracksList, &m_SmartArtistsList,
                    100, 20,
                    GenSmartPlaylist->GetLimitValue(), GenSmartPlaylist->GetLimitType(),
                    GenSmartPlaylist->GetAllowFilter(), GenSmartPlaylist->GetDenyFilter(),
                    GaugeId );

            if( !SmartModeThread )
                guLogMessage( wxT( "Could not create the smart mode thread" ) );

            m_SmartPlaylistId = GenSmartPlaylist->GetPlayListId();
            m_SmartPlaylistName = GenSmartPlaylist->GetPlaylistName();
        }

        GenSmartPlaylist->Destroy();
    }
}


// -------------------------------------------------------------------------------- //
// guUpdateCoversThread
// -------------------------------------------------------------------------------- //
guUpdateCoversThread::guUpdateCoversThread( guMediaViewer * mediaviewer, int gaugeid ) : wxThread()
{
    m_MediaViewer = mediaviewer;
    m_GaugeId = gaugeid;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guUpdateCoversThread::~guUpdateCoversThread()
{
    wxCommandEvent event( wxEVT_MENU, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( m_MediaViewer->GetMainFrame(), event );

    if( !TestDestroy() )
    {
        m_MediaViewer->UpdateCoversFinished();
    }
}

// -------------------------------------------------------------------------------- //
guUpdateCoversThread::ExitCode guUpdateCoversThread::Entry()
{
    guCoverInfos MissingCovers;
    int Index;
    int Count;

    wxCommandEvent event( wxEVT_MENU, ID_STATUSBAR_GAUGE_SETMAX );
    event.SetInt( m_GaugeId );

    if( ( Count = m_MediaViewer->GetDb()->GetEmptyCovers( MissingCovers ) ) )
    {
        event.SetExtraLong( Count );
        wxPostEvent( m_MediaViewer->GetMainFrame(), event );

        event.SetId( ID_STATUSBAR_GAUGE_UPDATE );
        event.SetInt( m_GaugeId );

        for( Index = 0; Index < Count; Index++ )
        {
            guCoverInfo &CurrentCover = MissingCovers[ Index ];

            m_MediaViewer->FindMissingCover( CurrentCover.m_AlbumId, CurrentCover.m_ArtistName, CurrentCover.m_AlbumName, CurrentCover.m_AlbumPath );

            event.SetExtraLong( Index );
            wxPostEvent( m_MediaViewer->GetMainFrame(), event );

            Sleep( 500 ); // Dont hammer cover service provider
        }

    }

    return 0;
}

// -------------------------------------------------------------------------------- //
// guMediaViewerDropTarget
// -------------------------------------------------------------------------------- //
guMediaViewerDropTarget::guMediaViewerDropTarget( guMediaViewer * mediaviewer ) : wxDropTarget()
{
    m_MediaViewer = mediaviewer;

    wxDataObjectComposite * DataObject = new wxDataObjectComposite();
    wxCustomDataObject * TracksDataObject = new wxCustomDataObject( wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) );
    DataObject->Add( TracksDataObject, true );
    wxFileDataObject * FileDataObject = new wxFileDataObject();
    DataObject->Add( FileDataObject, false );
    SetDataObject( DataObject );
}

// -------------------------------------------------------------------------------- //
guMediaViewerDropTarget::~guMediaViewerDropTarget()
{
}

// -------------------------------------------------------------------------------- //
wxDragResult guMediaViewerDropTarget::OnData( wxCoord x, wxCoord y, wxDragResult def )
{
    if( def == wxDragError || def == wxDragNone || def == wxDragCancel )
        return def;

    if( !GetData() )
    {
        guLogMessage( wxT( "Error getting drop data" ) );
        return wxDragError;
    }

    guDataObjectComposite * DataObject = ( guDataObjectComposite * ) m_dataObject;

    wxDataFormat ReceivedFormat = DataObject->GetReceivedFormat();
    //guLogMessage( wxT( "ReceivedFormat: '%s'" ), ReceivedFormat.GetId().c_str() );
    if( ReceivedFormat == wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) )
    {
        guTrackArray * Tracks;
        if( !DataObject->GetDataHere( ReceivedFormat, &Tracks ) )
        {
          guLogMessage( wxT( "Error getting tracks data..." ) );
        }
        else
        {
            m_MediaViewer->ImportFiles( new guTrackArray( * Tracks ) );
        }
    }
    else if( ReceivedFormat == wxDataFormat( wxDF_FILENAME ) )
    {
        wxFileDataObject * FileDataObject = ( wxFileDataObject * ) DataObject->GetDataObject( wxDataFormat( wxDF_FILENAME ) );
        if( FileDataObject )
        {
            m_MediaViewer->ImportFiles( FileDataObject->GetFilenames() );
        }
    }

    return def;
}

}

// -------------------------------------------------------------------------------- //
