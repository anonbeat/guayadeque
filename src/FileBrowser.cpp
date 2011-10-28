// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
//	anonbeat@gmail.com
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
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "FileBrowser.h"

#include "Accelerators.h"
#include "AuiDockArt.h"
#include "Config.h"
#include "FileRenamer.h"
#include "Images.h"
#include "LibUpdate.h"
#include "MainFrame.h"
#include "PlayListAppend.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Utils.h"

#include <wx/aui/aui.h>
#include <wx/arrimpl.cpp>
#include <wx/artprov.h>
#include <wx/clipbrd.h>

WX_DEFINE_OBJARRAY(guFileItemArray);

// -------------------------------------------------------------------------------- //
// guGenericDirCtrl
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guGenericDirCtrl, wxGenericDirCtrl)
    EVT_TREE_BEGIN_LABEL_EDIT( wxID_ANY, guGenericDirCtrl::OnBeginRenameDir )
    EVT_TREE_END_LABEL_EDIT( wxID_ANY, guGenericDirCtrl::OnEndRenameDir )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guGenericDirCtrl::guGenericDirCtrl( wxWindow * parent, guMainFrame * mainframe, const int showpaths  ) :
              wxGenericDirCtrl( parent, wxID_ANY, wxDirDialogDefaultFolderStr,
                wxDefaultPosition, wxDefaultSize, wxDIRCTRL_SELECT_FIRST|wxDIRCTRL_DIR_ONLY|wxDIRCTRL_3D_INTERNAL|wxNO_BORDER|wxDIRCTRL_EDIT_LABELS ,
                wxEmptyString, 0, wxTreeCtrlNameStr )
{
    m_MainFrame = mainframe;
    m_ShowPaths = showpaths;
    m_FileBrowserDirCtrl = ( guFileBrowserDirCtrl * ) parent;
    wxImageList * ImageList = GetTreeCtrl()->GetImageList();
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_library ) );
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_podcast ) );
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_record ) );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guGenericDirCtrl::OnConfigUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guGenericDirCtrl::~guGenericDirCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guGenericDirCtrl::OnConfigUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & ( guPREFERENCE_PAGE_FLAG_LIBRARY | guPREFERENCE_PAGE_FLAG_RECORD | guPREFERENCE_PAGE_FLAG_PODCASTS ) )
    {
        if( m_ShowPaths != guFILEBROWSER_SHOWPATH_SYSTEM )
        {
            wxString CurPath = GetPath();
            ReCreateTree();
            SetPath( CurPath );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::SetupSections()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    guLogMessage( wxT( "Tree Flag %08X" ), m_ShowPaths );

    if( m_ShowPaths & guFILEBROWSER_SHOWPATH_LOCATIONS )
    {
        const guMediaCollectionArray &  Collections = m_MainFrame->GetMediaCollections();
        int Index;
        int Count = Collections.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            const guMediaCollection & Collection = Collections[ Index ];
            if( m_MainFrame->IsCollectionActive( Collection.m_UniqueId ) )
            {
                int PathIndex;
                int PathCount = Collection.m_Paths.Count();
                for( PathIndex = 0; PathIndex < PathCount; PathIndex++ )
                {
                    wxString LibName = Collection.m_Paths[ PathIndex ];
                    if( LibName.EndsWith( wxT( "/" ) ) )
                        LibName.RemoveLast();
                    AddSection( LibName, wxFileNameFromPath( LibName ), guDIR_IMAGE_INDEX_LIBRARY );
                }
            }
        }

        wxString Path = Config->ReadStr( wxT( "Path" ), wxEmptyString, wxT( "podcasts" ) );
        if( !Path.IsEmpty() )
        {
            wxString Name = Path;
            if( Name.EndsWith( wxT( "/" ) ) )
                Name.RemoveLast();
            AddSection( Path, wxFileNameFromPath( Name ), guDIR_IMAGE_INDEX_PODCASTS );
        }

        Path = Config->ReadStr( wxT( "Path" ), wxEmptyString, wxT( "record" ) );
        if( !Path.IsEmpty() )
        {
            wxString Name = Path;
            if( Name.EndsWith( wxT( "/" ) ) )
                Name.RemoveLast();
            AddSection( Path, wxFileNameFromPath( Name ), guDIR_IMAGE_INDEX_RECORDS );
        }
    }

    if( m_ShowPaths & guFILEBROWSER_SHOWPATH_SYSTEM )
    {
        AddSection( wxT( "/" ), wxT( "/" ), guDIR_IMAGE_INDEX_FOLDER );
    }
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnBeginRenameDir( wxTreeEvent &event )
{
    OnBeginEditItem( event );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnEndRenameDir( wxTreeEvent &event )
{
    OnEndEditItem( event );
    wxTreeCtrl * TreeCtrl = GetTreeCtrl();
    TreeCtrl->SelectItem( m_RenameItemId );
    m_FileBrowserDirCtrl->RenamedDir( m_RenameName, GetPath() );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::FolderRename( void )
{
    wxTreeCtrl * TreeCtrl = GetTreeCtrl();
    m_RenameItemId = TreeCtrl->GetSelection();
    m_RenameName = GetPath();
    TreeCtrl->EditLabel( m_RenameItemId );
}

// -------------------------------------------------------------------------------- //
// guFileBrowserDirCtrl
// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::guFileBrowserDirCtrl( wxWindow * parent, guMainFrame * mainframe, guDbLibrary * db, const wxString &dirpath ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER )
{
    m_MainFrame = mainframe;
    m_DefaultDb = db;
    m_Db = NULL;
    m_MediaViewer = NULL;
    m_AddingFolder = false;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

    int ShowPaths = Config->ReadNum( wxT( "ShowPaths" ), guFILEBROWSER_SHOWPATH_LOCATIONS, wxT( "filebrowser" ) );
	m_DirCtrl = new guGenericDirCtrl( this, m_MainFrame, ShowPaths );
	SetPath( dirpath, NULL );

	m_DirCtrl->ShowHidden( false );
	MainSizer->Add( m_DirCtrl, 1, wxEXPAND, 5 );

	wxBoxSizer * DirBtnSizer = new wxBoxSizer( wxHORIZONTAL );


	DirBtnSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_ShowLibPathsBtn = new wxToggleBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_library ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_ShowLibPathsBtn->SetToolTip( ShowPaths == guFILEBROWSER_SHOWPATH_SYSTEM ?
                          _( "See used locations" ) :
                          _( "See system files" ) );
	m_ShowLibPathsBtn->SetValue( ShowPaths & guFILEBROWSER_SHOWPATH_LOCATIONS );
	DirBtnSizer->Add( m_ShowLibPathsBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MainSizer->Add( DirBtnSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    m_DirCtrl->Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guFileBrowserDirCtrl::OnContextMenu ), NULL, this );
	m_ShowLibPathsBtn->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guFileBrowserDirCtrl::OnShowLibPathsClick ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guFileBrowserDirCtrl::OnConfigUpdated ), NULL, this );

    CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::~guFileBrowserDirCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteBool( wxT( "ShowLibPaths" ), m_ShowLibPathsBtn->GetValue(), wxT( "filebrowser" ) );
    m_DirCtrl->Disconnect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guFileBrowserDirCtrl::OnContextMenu ), NULL, this );
	m_ShowLibPathsBtn->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guFileBrowserDirCtrl::OnShowLibPathsClick ), NULL, this );

    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guFileBrowserDirCtrl::OnConfigUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_SONG_PLAY );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ARTIST );

    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_SAVEPLAYLIST );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_EDITTRACKS );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_PLAY );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void AppendFolderCommands( wxMenu * menu )
{
    wxMenu * SubMenu;
    int Index;
    int Count;
    wxMenuItem * MenuItem;

    SubMenu = new wxMenu();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );
    wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "commands/names" ) );
    if( ( Count = Commands.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( ( Commands[ Index ].Find( wxT( "{bc}" ) ) == wxNOT_FOUND ) )
            {
                MenuItem = new wxMenuItem( menu, ID_COMMANDS_BASE + Index, Names[ Index ], Commands[ Index ] );
                SubMenu->Append( MenuItem );
            }
        }

        SubMenu->AppendSeparator();
    }
    else
    {
        MenuItem = new wxMenuItem( SubMenu, ID_MENU_PREFERENCES_COMMANDS, _( "Preferences" ), _( "Add commands in preferences" ) );
        SubMenu->Append( MenuItem );
    }
    menu->AppendSubMenu( SubMenu, _( "Commands" ) );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPoint();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_PLAY ),
                            _( "Play the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ALL ),
                            _( "Add the selected folder to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu.Append( MenuItem );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    Menu.Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_EDITTRACKS,
                            wxString( _( "Edit Tracks" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                            _( "Edit the tracks in the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST,
                            wxString( _( "Save to Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                            _( "Add the tracks in the selected folder to a playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_COPY,
                            _( "Copy" ),
                            _( "Copy the selected folder to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    Menu.Append( MenuItem );
    //MenuItem->Enable( false );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_PASTE,
                            _( "Paste" ),
                            _( "Paste to the selected folder" ) );
    Menu.Append( MenuItem );
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_FILENAME ) )
        {
            wxFileDataObject data;
            MenuItem->Enable( wxTheClipboard->GetData( data ) );
        }
        wxTheClipboard->Close();
    }

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_NEW, _( "New Folder" ), _( "Create a new folder" ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_RENAME, _( "Rename" ), _( "Rename the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_DELETE, _( "Remove" ), _( "Remove the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    MainFrame->CreateCopyToMenu( &Menu );

    AppendFolderCommands( &Menu );

    PopupMenu( &Menu, Point );
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::RenamedDir( const wxString &oldname, const wxString &newname )
{
    guLogMessage( wxT( "'%s' -> '%s'  (%i)" ), oldname.c_str(), newname.c_str(), m_Db != NULL );

    if( m_AddingFolder )
    {
        wxTreeCtrl * TreeCtrl = m_DirCtrl->GetTreeCtrl();
        if( newname.IsEmpty() )
        {
            TreeCtrl->Delete( TreeCtrl->GetSelection() );
        }
        //m_DirCtrl->ReCreateTree();
        m_AddingFolder = false;
    }
    else
    {
        if( oldname != newname )
        {
            if( m_Db )
                m_Db->UpdatePaths( oldname, newname );
            else
                m_DefaultDb->UpdatePaths( oldname, newname );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::FolderNew( void )
{
    wxTreeCtrl * TreeCtrl = m_DirCtrl->GetTreeCtrl();
    wxTreeItemId FolderParent = TreeCtrl->GetSelection();
    if( FolderParent.IsOk() )
    {
        m_AddingFolder = true;

        wxString NewDirName = GetPath() + _( "New Folder" );
        int Index = 1;
        while( wxDirExists( NewDirName ) )
        {
            NewDirName = GetPath() + _( "New Folder" );
            NewDirName += wxString::Format( wxT( "%i" ), Index++ );
        }

        if( wxMkdir( NewDirName, 0770 ) )
        {
            TreeCtrl->Collapse( FolderParent );
            //TreeCtrl->Expand( m_AddFolderParent );
            if( m_DirCtrl->ExpandPath( NewDirName ) )
            {
                wxTreeItemId Selected = TreeCtrl->GetSelection();
                if( Selected.IsOk() )
                {
                    wxTextCtrl * TextCtrl = TreeCtrl->EditLabel( Selected );
                    if( TextCtrl )
                    {
                        TextCtrl->SetSelection( -1, -1 );
                    }
                }
                else
                {
                    guLogMessage( wxT( "No Selected item" ) );
                }
            }
        }
        else
        {
            guLogError( wxT( "Could not create the new directory" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool RemoveDirItems( const wxString &path, wxArrayString * deletefiles )
{
    wxString FileName;
    wxString CurPath = path;
    if( !CurPath.EndsWith( wxT( "/" ) ) )
        CurPath += wxT( "/" );
    //guLogMessage( wxT( "Deleting folder %s" ), CurPath.c_str() );
    wxDir Dir( CurPath );
    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_HIDDEN | wxDIR_DIRS | wxDIR_DOTDOT ) )
        {
            do {
                if( FileName != wxT( "." ) && FileName != wxT( ".." ) )
                {
                    if( wxDirExists( CurPath + FileName ) )
                    {
                        if( !RemoveDirItems( CurPath + FileName, deletefiles ) )
                            return false;
                        //guLogMessage( wxT( "Removing Dir: %s" ), ( CurPath + FileName ).c_str() );
                        if( !wxRmdir( CurPath + FileName ) )
                            return false;
                    }
                    else
                    {
                        //guLogMessage( wxT( "Removing file: %s" ), ( CurPath + FileName ).c_str() );
                        if( !wxRemoveFile( CurPath + FileName ) )
                            return false;
                        deletefiles->Add( CurPath + FileName );
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::FolderDelete( void )
{
    wxTreeCtrl * TreeCtrl = m_DirCtrl->GetTreeCtrl();
    wxTreeItemId FolderId = TreeCtrl->GetSelection();
    wxDirItemData * FolderData = ( wxDirItemData *  ) TreeCtrl->GetItemData( FolderId );
    if( wxMessageBox( _( "Are you sure to delete the selected path ?" ),
                     _( "Confirm" ),
                     wxICON_QUESTION | wxYES_NO, this ) == wxYES )
    {
        wxArrayString DeleteFiles;
        if( RemoveDirItems( FolderData->m_path, &DeleteFiles ) && wxRmdir( FolderData->m_path ) )
        {
            TreeCtrl->Delete( FolderId );
        }
        else
        {
            wxMessageBox( _( "Error deleting the folder " ) + FolderData->m_path,
                _( "Error" ), wxICON_ERROR | wxOK, this );
        }
        //m_Db->DoCleanUp();
        if( m_Db )
            m_Db->CleanFiles( DeleteFiles );
        else
            m_DefaultDb->CleanFiles( DeleteFiles );
    }
}


// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnShowLibPathsClick( wxCommandEvent& event )
{
    int ShowPaths = 0;
    if( m_ShowLibPathsBtn->GetValue() )
        ShowPaths |= guFILEBROWSER_SHOWPATH_LOCATIONS;
    if( !ShowPaths )
        ShowPaths |= guFILEBROWSER_SHOWPATH_SYSTEM;

    wxString CurPath = GetPath();
    m_DirCtrl->SetShowPaths( ShowPaths );
    m_DirCtrl->ReCreateTree();
    m_DirCtrl->SetPath( CurPath );

    m_ShowLibPathsBtn->SetToolTip( ShowPaths == guFILEBROWSER_SHOWPATH_SYSTEM ?
                          _( "See used locations" ) :
                          _( "See system files" ) );
}


// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::CollectionsUpdated( void )
{
    if( m_DirCtrl->GetShowPaths() & guFILEBROWSER_SHOWPATH_LOCATIONS )
    {
        wxString CurPath = GetPath();
        m_DirCtrl->ReCreateTree();
        m_DirCtrl->SetPath( CurPath );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::SetPath( const wxString &path, guMediaViewer * mediaviewer )
{
    guLogMessage( wxT( "guFileBrowserDirCtrl::SetPath( %s )" ), path.c_str() );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
    m_DirCtrl->SetPath( path );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
}

// -------------------------------------------------------------------------------- //
// guFilesListBox
// -------------------------------------------------------------------------------- //
bool guAddDirItems( const wxString &path, wxArrayString &files )
{
    wxString FileName;
    wxString CurPath = path;
    if( !CurPath.EndsWith( wxT( "/" ) ) )
        CurPath += wxT( "/" );
    guLogMessage( wxT( "Searching in folder %s" ), CurPath.c_str() );
    wxDir Dir( CurPath );
    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_HIDDEN | wxDIR_DIRS | wxDIR_DOTDOT ) )
        {
            do {
                if( FileName != wxT( "." ) && FileName != wxT( ".." ) )
                {
                    if( wxDirExists( CurPath + FileName ) )
                    {
                        if( !guAddDirItems( CurPath + FileName, files ) )
                            return false;
                    }
                    else
                    {
                        files.Add( CurPath + FileName );
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guFilesListBox::guFilesListBox( wxWindow * parent, guDbLibrary * db ) :
    guListView( parent, wxLB_MULTIPLE | guLISTVIEW_COLUMN_SELECT | guLISTVIEW_COLUMN_SORTING | guLISTVIEW_ALLOWDRAG )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_Order = Config->ReadNum( wxT( "Order" ), 0, wxT( "filebrowser" ) );
    m_OrderDesc = Config->ReadNum( wxT( "OrderDesc" ), false, wxT( "filebrowser" ) );

    int ColId;
    wxString ColName;
    wxArrayString ColumnNames = GetColumnNames();
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "Id%u" ), index ), index, wxT( "filebrowser/columns/ids" ) );

        ColName = ColumnNames[ ColId ];

        ColName += ( ( ColId == m_Order ) ? ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "Width%u" ), index ), 80, wxT( "filebrowser/columns/widths" ) ),
            Config->ReadBool( wxString::Format( wxT( "Show%u" ), index ), true, wxT( "filebrowser/columns/shows" ) )
            );
        InsertColumn( Column );
    }

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guFilesListBox::OnConfigUpdated ), NULL, this );

    CreateAcceleratorTable();

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guFilesListBox::~guFilesListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    wxArrayString ColumnNames = GetColumnNames();
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "Id%u" ), index ),
                          ( * m_Columns )[ index ].m_Id, wxT( "filebrowser/columns/ids" ) );
        Config->WriteNum( wxString::Format( wxT( "Width%u" ), index ),
                          ( * m_Columns )[ index ].m_Width, wxT( "filebrowser/columns/widths" ) );
        Config->WriteBool( wxString::Format( wxT( "Show%u" ), index ),
                           ( * m_Columns )[ index ].m_Enabled, wxT( "filebrowser/columns/shows" ) );
    }

    Config->WriteNum( wxT( "Order" ), m_Order, wxT( "filebrowser" ) );
    Config->WriteBool( wxT( "OrderDesc" ), m_OrderDesc, wxT( "filebrowser" ) );

    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guFilesListBox::OnConfigUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_SONG_PLAY );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_AFTER_ARTIST );

    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_SAVEPLAYLIST );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_EDITTRACKS );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_PLAY );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
wxArrayString guFilesListBox::GetColumnNames( void )
{
    wxArrayString ColumnNames;
    ColumnNames.Add( _( "Name" ) );
    ColumnNames.Add( _( "Size" ) );
    ColumnNames.Add( _( "Modified" ) );
    return ColumnNames;
}

// -------------------------------------------------------------------------------- //
wxString guFilesListBox::OnGetItemText( const int row, const int col ) const
{
//    guLogMessage( wxT( "GetItem: %i  %i" ), row, col );
//    if( row < 0 || col < 0 )
//        return wxEmptyString;

    guFileItem * FileItem;
    FileItem = &m_Files[ row ];
    switch( ( * m_Columns )[ col ].m_Id )
    {
//        case guFILEBROWSER_COLUMN_TYPE :
//            return wxEmptyString;

        case guFILEBROWSER_COLUMN_NAME :
          return FileItem->m_Name;

        case guFILEBROWSER_COLUMN_SIZE :
          return SizeToString( FileItem->m_Size );

        case guFILEBROWSER_COLUMN_TIME :
        {
          wxDateTime FileTime;
          FileTime.Set( ( time_t ) FileItem->m_Time );
          return FileTime.FormatDate();
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( ( * m_Columns )[ col ].m_Id == guFILEBROWSER_COLUMN_NAME )
    {
        guFileItem * FileItem = &m_Files[ row ];
        dc.SetBackgroundMode( wxTRANSPARENT );
        int ImageIndex = guDIR_IMAGE_INDEX_OTHER;
        if( FileItem->m_Type == guFILEITEM_TYPE_FOLDER )
        {
            ImageIndex = guDIR_IMAGE_INDEX_FOLDER;
        }
        else if( FileItem->m_Type == guFILEITEM_TYPE_AUDIO )
        {
            ImageIndex = guDIR_IMAGE_INDEX_AUDIO;
        }
        else if( FileItem->m_Type == guFILEITEM_TYPE_IMAGE )
        {
            ImageIndex = guDIR_IMAGE_INDEX_IMAGE;
        }
        m_TreeImageList->Draw( ImageIndex, dc, rect.x + 1, rect.y + 1, wxIMAGELIST_DRAW_TRANSPARENT );
        wxRect TextRect = rect;
        TextRect.x += 20; // 16 + 4
        TextRect.width -= 20;
        guListView::DrawItem( dc, TextRect, row, col );
    }
    else
    {
        guListView::DrawItem( dc, rect, row, col );
    }
}


// -------------------------------------------------------------------------------- //
void inline GetFileDetails( const wxString &filename, guFileItem * fileitem )
{
    wxStructStat St;
    wxStat( filename, &St );
    fileitem->m_Type = ( ( St.st_mode & S_IFMT ) == S_IFDIR ) ? 0 : 3;
    fileitem->m_Size = St.st_size;
    fileitem->m_Time = St.st_ctime;
}

//// -------------------------------------------------------------------------------- //
//static int wxCMPFUNC_CONV CompareFileTypeA( guFileItem ** item1, guFileItem ** item2 )
//{
//    if( ( * item1 )->m_Name == wxT( ".." ) )
//        return -1;
//    else if( ( * item2 )->m_Name == wxT( ".." ) )
//        return 1;
//
//    if( ( * item1 )->m_Type == ( * item2 )->m_Type )
//        return 0;
//    else if( ( * item1 )->m_Type > ( * item2 )->m_Type )
//        return -1;
//    else
//        return 1;
//}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareFileTypeD( guFileItem ** item1, guFileItem ** item2 )
{
    if( ( * item1 )->m_Name == wxT( ".." ) )
        return -1;
    else if( ( * item2 )->m_Name == wxT( ".." ) )
        return 1;

    if( ( * item1 )->m_Type == ( * item2 )->m_Type )
        return 0;
    else if( ( * item1 )->m_Type > ( * item2 )->m_Type )
        return 1;
    else
        return -1;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareFileNameA( guFileItem ** item1, guFileItem ** item2 )
{
    int type = CompareFileTypeD( item1, item2 );
    if( !type )
        return ( * item1 )->m_Name.Cmp( ( * item2 )->m_Name );
    return type;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareFileNameD( guFileItem ** item1, guFileItem ** item2 )
{
    int type = CompareFileTypeD( item1, item2 );
    if( !type )
        return ( * item2 )->m_Name.Cmp( ( * item1 )->m_Name );
    return type;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareFileSizeA( guFileItem ** item1, guFileItem ** item2 )
{
    int type = CompareFileTypeD( item1, item2 );
    if( !type )
    {
        if( ( * item1 )->m_Size == ( * item2 )->m_Size )
            return 0;
        else if( ( * item1 )->m_Size > ( * item2 )->m_Size )
            return 1;
        else
            return -1;
    }
    return type;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareFileSizeD( guFileItem ** item1, guFileItem ** item2 )
{
    int type = CompareFileTypeD( item1, item2 );
    if( !type )
    {
        if( ( * item1 )->m_Size == ( * item2 )->m_Size )
            return 0;
        else if( ( * item2 )->m_Size > ( * item1 )->m_Size )
            return 1;
        else
            return -1;
    }
    return type;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareFileTimeA( guFileItem ** item1, guFileItem ** item2 )
{
    int type = CompareFileTypeD( item1, item2 );
    if( !type )
    {
        if( ( * item1 )->m_Time == ( * item2 )->m_Time )
            return 0;
        else if( ( * item1 )->m_Time > ( * item2 )->m_Time )
            return 1;
        else
            return -1;
    }
    return type;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareFileTimeD( guFileItem ** item1, guFileItem ** item2 )
{
    int type = CompareFileTypeD( item1, item2 );
    if( !type )
    {
        if( ( * item1 )->m_Time == ( * item2 )->m_Time )
            return 0;
        else if( ( * item2 )->m_Time > ( * item1 )->m_Time )
            return 1;
        else
            return -1;
    }
    return type;
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetPathSordedItems( const wxString &path, guFileItemArray * items,
    const int order, const bool orderdesc, const bool recursive ) const
{
    wxString Path = path;
    if( !Path.EndsWith( wxT( "/" ) ) )
        Path += wxT( "/" );
    if( !path.IsEmpty() && wxDirExists( Path ) )
    {
        wxDir Dir( Path );
        if( Dir.IsOpened() )
        {
            wxString FileName;
            if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS | wxDIR_DOTDOT ) )
            {
                do {
                    if( FileName != wxT( "." ) )
                    {
                        if( recursive && wxDirExists( Path + FileName ) )
                        {
                            if( FileName != wxT( ".." ) )
                            {
                                GetPathSordedItems( Path + FileName, items, order, orderdesc, recursive );
                            }
                        }
                        else
                        {
                            guFileItem * FileItem = new guFileItem();
                            if( recursive )
                                FileItem->m_Name = Path;
                            FileItem->m_Name += FileName;
                            GetFileDetails( Path + FileName, FileItem );
                            if( guIsValidAudioFile( FileName.Lower() ) )
                                FileItem->m_Type = guFILEITEM_TYPE_AUDIO;
                            else if( guIsValidImageFile( FileName.Lower() ) )
                                FileItem->m_Type = guFILEITEM_TYPE_IMAGE;
                            items->Add( FileItem );
                        }
                    }
                } while( Dir.GetNext( &FileName ) );
            }

            switch( order )
            {
                case guFILEBROWSER_COLUMN_NAME :
                {
                    items->Sort( orderdesc ? CompareFileNameD : CompareFileNameA );
                    break;
                }

                case guFILEBROWSER_COLUMN_SIZE :
                {
                    items->Sort( orderdesc ? CompareFileSizeD : CompareFileSizeA );
                    break;
                }

                case guFILEBROWSER_COLUMN_TIME :
                {
                    items->Sort( orderdesc ? CompareFileTimeD : CompareFileTimeA );
                    break;
                }
            }
        }
    }
    return items->Count();
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::GetItemsList( void )
{
    GetPathSordedItems( m_CurDir, &m_Files, m_Order, m_OrderDesc );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::ReloadItems( bool reset )
{
    //
    wxArrayInt Selection;
    int FirstVisible = 0;

    if( reset )
    {
        SetSelection( -1 );
    }
    else
    {
        Selection = GetSelectedItems( false );
        FirstVisible = GetFirstVisibleLine();
    }

    m_Files.Empty();

    GetItemsList();

    SetItemCount( m_Files.Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToLine( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void AppendItemsCommands( wxMenu * menu, int selcount, int seltype )
{
    wxMenu * SubMenu;
    int Index;
    int Count;
    wxMenuItem * MenuItem;

    SubMenu = new wxMenu();

    guLogMessage( wxT( "AppendItemCommands: %i  %i" ), selcount, seltype );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );
    wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "commands/names" ) );
    if( ( Count = Commands.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( ( Commands[ Index ].Find( wxT( "{bc}" ) ) == wxNOT_FOUND ) ||
                ( ( selcount == 1 ) && ( seltype == guFILEITEM_TYPE_IMAGE ) ) )
            {
                MenuItem = new wxMenuItem( menu, ID_COMMANDS_BASE + Index, Names[ Index ], Commands[ Index ] );
                SubMenu->Append( MenuItem );
            }
        }

        SubMenu->AppendSeparator();
    }
    else
    {
        MenuItem = new wxMenuItem( SubMenu, ID_MENU_PREFERENCES_COMMANDS, _( "Preferences" ), _( "Add commands in preferences" ) );
        SubMenu->Append( MenuItem );
    }
    menu->AppendSubMenu( SubMenu, _( "Commands" ) );
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    wxArrayInt Selection = GetSelectedItems( false );
    int SelCount;
    if( ( SelCount = Selection.Count() ) )
    {
        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_PLAY ),
                            _( "Play current selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ALL ),
                            _( "Add current selected files to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );
    }

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_EDITTRACKS,
                            wxString( _( "Edit Tracks" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                            _( "Edit the current selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_SAVEPLAYLIST,
                            wxString( _( "Save to Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                            _( "Add the current selected tracks to a playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_COPY, _( "Copy" ), _( "Copy the selected folder to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_PASTE, _( "Paste" ), _( "Paste to the selected dir" ) );
    Menu->Append( MenuItem );
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_FILENAME ) )
        {
            wxFileDataObject data;
            MenuItem->Enable( wxTheClipboard->GetData( data ) );
        }
        wxTheClipboard->Close();
    }

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_RENAME, _( "Rename Files" ), _( "Rename the current selected file" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_DELETE, _( "Remove" ), _( "Delete the selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
        MainFrame->CreateCopyToMenu( Menu );

        AppendItemsCommands( Menu, SelCount, SelCount ? GetType( Selection[ 0 ] ) : guFILEITEM_TYPE_FILE );
    }
}

// -------------------------------------------------------------------------------- //
int inline guFilesListBox::GetItemId( const int row ) const
{
    return row;
}

// -------------------------------------------------------------------------------- //
wxString inline guFilesListBox::GetItemName( const int row ) const
{
    return m_Files[ row ].m_Name;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::SetOrder( int columnid )
{
    if( m_Order != columnid )
    {
        m_Order = columnid;
        m_OrderDesc = ( columnid != 0 );
    }
    else
        m_OrderDesc = !m_OrderDesc;

    wxArrayString ColumnNames = GetColumnNames();
    int CurColId;
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        CurColId = GetColumnId( index );
        SetColumnLabel( index,
            ColumnNames[ CurColId ]  + ( ( CurColId == m_Order ) ?
                ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetSelectedSongs( guTrackArray * tracks ) const
{
    wxArrayString Files = GetSelectedFiles( true );
    return GetTracksFromFiles( Files, tracks );
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetAllSongs( guTrackArray * tracks ) const
{
    wxArrayString Files = GetAllFiles( true );
    return GetTracksFromFiles( Files, tracks );
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetTracksFromFiles( const wxArrayString &files, guTrackArray * tracks ) const
{
    int Index;
    int Count;
    if( ( Count = files.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            wxString FileName = files[ Index ];
            //guLogMessage( wxT( "File: %s" ), FileName.c_str() );
            wxURI Uri( FileName );

            if( Uri.IsReference() )
            {
                if( guIsValidAudioFile( FileName ) )
                {
                    guTrack * Track = new guTrack();
                    Track->m_FileName = FileName;

                    if( !m_Db || !m_Db->FindTrackFile( FileName, Track ) )
                    {
                        guPodcastItem PodcastItem;
                        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
                        guDbPodcasts * DbPodcasts = MainFrame->GetPodcastsDb();
                        if( DbPodcasts->GetPodcastItemFile( FileName, &PodcastItem ) )
                        {
                            delete Track;
                            continue;
                        }
                        else
                        {
                            if( Track->ReadFromFile( FileName ) )
                            {
                                Track->m_Type = guTRACK_TYPE_NOTDB;
                            }
                            else
                            {
                                guLogError( wxT( "Could not read tags from file '%s'" ), FileName.c_str() );
                            }
                        }
                    }

                    tracks->Add( Track );
                }
            }
        }
    }
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
wxArrayString guFilesListBox::GetSelectedFiles( const bool recursive ) const
{
    wxArrayString Files;
    wxArrayInt Selection = GetSelectedItems( false );
    int Index;
    int Count;
    if( ( Count = Selection.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_Files[ Selection[ Index ] ].m_Name != wxT( ".." ) )
            {
                if( recursive && ( m_Files[ Selection[ Index ] ].m_Type == guFILEITEM_TYPE_FOLDER ) )
                {
                    //guAddDirItems( m_CurDir + m_Files[ Selection[ Index ] ].m_Name, Files );
                    guFileItemArray DirFiles;
                    if( GetPathSordedItems( m_CurDir + m_Files[ Selection[ Index ] ].m_Name,
                                            &DirFiles, m_Order, m_OrderDesc, true ) )
                    {
                        int FileIndex;
                        int FileCount = DirFiles.Count();
                        for( FileIndex = 0; FileIndex < FileCount; FileIndex++ )
                        {
                            Files.Add( DirFiles[ FileIndex ].m_Name );
                        }
                    }
                }
                else
                {
                    Files.Add( m_CurDir + m_Files[ Selection[ Index ] ].m_Name );
                }
            }
        }
    }
    return Files;
}

// -------------------------------------------------------------------------------- //
wxArrayString guFilesListBox::GetAllFiles( const bool recursive ) const
{
    wxArrayString Files;
    int Index;
    int Count = m_Files.Count();
    if( Count )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_Files[ Index ].m_Name != wxT( ".." ) )
            {
                if( recursive && ( m_Files[ Index ].m_Type == guFILEITEM_TYPE_FOLDER ) )
                {
                    //guAddDirItems( m_CurDir + m_Files[ Selection[ Index ] ].m_Name, Files );
                    guFileItemArray DirFiles;
                    if( GetPathSordedItems( m_CurDir + m_Files[ Index ].m_Name,
                                            &DirFiles, m_Order, m_OrderDesc, true ) )
                    {
                        int FileIndex;
                        int FileCount = DirFiles.Count();
                        for( FileIndex = 0; FileIndex < FileCount; FileIndex++ )
                        {
                            Files.Add( DirFiles[ FileIndex ].m_Name );
                        }
                    }
                }
                else
                {
                    Files.Add( m_CurDir + m_Files[ Index ].m_Name );
                }
            }
        }
    }
    return Files;
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetDragFiles( wxFileDataObject * files )
{
    wxArrayString SelectFiles = GetSelectedFiles( true );
    int index;
    int count = SelectFiles.Count();
    for( index = 0; index < count; index++ )
    {
       files->AddFile( guFileDnDEncode( SelectFiles[ index ] ) );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::SetPath( const wxString &path, guMediaViewer * mediaviewer )
{
    guLogMessage( wxT( "guFilesListBox::SetPath( %s )" ), path.c_str() );
    m_CurDir = path;
    if( !m_CurDir.EndsWith( wxT( "/" ) ) )
        m_CurDir += wxT( "/" );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
wxString guFilesListBox::GetPath( const int item, const bool absolute ) const
{
    wxString RetVal;
    //guLogMessage( wxT( "GetPath( %i )" ), item );
    if( item >= 0 )
    {
        if( !absolute )
        {
            return m_CurDir + m_Files[ item ].m_Name;
        }
        else
        {
            if( m_Files[ item ].m_Name == wxT( ".." ) )
            {
                RetVal = m_CurDir.BeforeLast( wxT( '/' ) ).BeforeLast( wxT( '/' ) );
                //guLogMessage( wxT( "1) Path : %s" ), RetVal.c_str() );
                return RetVal;
            }

            wxFileName FileName( m_Files[ item ].m_Name );
            FileName.MakeAbsolute( m_CurDir );
            //guLogMessage( wxT( "Path : %s" ), FileName.GetFullPath().c_str() );
            return FileName.GetFullPath();
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetType( const int item ) const
{
    if( item >= 0 )
    {
        return m_Files[ item ].m_Type;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
bool guFilesListBox::GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
    int Index;
    int Count;
    * count = * len = * size = 0;
    Count = m_Files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( ( m_Files[ Index ].m_Type == guFILEITEM_TYPE_FOLDER ) )
        {
            if( ( m_Files[ Index ].m_Name != wxT( ".." ) ) )
                ( * len )++;
        }
        else
        {
            * size += m_Files[ Index ].m_Size;
            ( * count )++;
        }
    }
    return true;
}


// -------------------------------------------------------------------------------- //
// guFileBrowserFileCtrl
// -------------------------------------------------------------------------------- //
guFileBrowserFileCtrl::guFileBrowserFileCtrl( wxWindow * parent, guDbLibrary * db, guFileBrowserDirCtrl * dirctrl ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxNO_BORDER )
{
    m_DefaultDb = db;
    m_Db = NULL;
    m_MediaViewer = NULL;
    m_DirCtrl = dirctrl;
    wxImageList * ImageList = dirctrl->GetImageList();
    ImageList->Add( wxArtProvider::GetBitmap( wxT( "audio-x-generic" ), wxART_OTHER, wxSize( 16, 16 ) ) );
    ImageList->Add( wxArtProvider::GetBitmap( wxT( "image-x-generic" ), wxART_OTHER, wxSize( 16, 16 ) ) );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_FilesListBox = new guFilesListBox( this, db );
	m_FilesListBox->SetTreeImageList( ImageList );
	MainSizer->Add( m_FilesListBox, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

}

// -------------------------------------------------------------------------------- //
guFileBrowserFileCtrl::~guFileBrowserFileCtrl()
{
}

// -------------------------------------------------------------------------------- //
void guFileBrowserFileCtrl::SetPath( const wxString &path, guMediaViewer * mediaviewer )
{
    guLogMessage( wxT( "guFileBrowserFileCtrl::SetPath( %s )" ), path.c_str() );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
    m_FilesListBox->SetPath( path, mediaviewer );
}



// -------------------------------------------------------------------------------- //
// guFileBrowser
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE( guFileBrowser, wxPanel )
    EVT_TREE_BEGIN_DRAG( wxID_ANY, guFileBrowser::OnDirBeginDrag)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guFileBrowser::guFileBrowser( wxWindow * parent, guMainFrame * mainframe, guDbLibrary * db, guPlayerPanel * playerpanel ) :
    guAuiManagerPanel( parent )
{
    m_MainFrame = mainframe;
    m_DefaultDb = db;
    m_Db = NULL;
    m_MediaViewer = NULL;
    m_PlayerPanel = playerpanel;

    guConfig *  Config = ( guConfig * ) guConfig::Get();

    m_VisiblePanels = Config->ReadNum( wxT( "VisiblePanels" ), guPANEL_FILEBROWSER_VISIBLE_DEFAULT, wxT( "filebrowser" ) );


    wxString LastPath = Config->ReadStr( wxT( "Path" ), wxEmptyString, wxT( "filebrowser" ) );
    m_DirCtrl = new guFileBrowserDirCtrl( this, m_MainFrame, m_Db, LastPath );
    guLogMessage( wxT( "LastPath: '%s'" ), LastPath.c_str() );

    m_AuiManager.AddPane( m_DirCtrl,
            wxAuiPaneInfo().Name( wxT( "FileBrowserDirCtrl" ) ).Caption( _( "Directories" ) ).
            MinSize( 60, 28 ).Row( 0 ).Layer( 0 ).Position( 0 ).
            CloseButton( false ).
            Dockable( true ).Left() );

    m_FilesCtrl = new guFileBrowserFileCtrl( this, db, m_DirCtrl );
    m_FilesCtrl->SetPath( LastPath, NULL );

    m_AuiManager.AddPane( m_FilesCtrl,
            wxAuiPaneInfo().Name( wxT( "FileBrowserFilesCtrl" ) ).
            Dockable( true ).CenterPane() );

    wxString FileBrowserLayout = Config->ReadStr( wxT( "LastLayout" ), wxEmptyString, wxT( "filebrowser" ) );
    if( Config->GetIgnoreLayouts() || FileBrowserLayout.IsEmpty() )
    {
        m_VisiblePanels = guPANEL_FILEBROWSER_VISIBLE_DEFAULT;
        FileBrowserLayout = wxT( "layout2|name=FileBrowserDirCtrl;caption=" ) + wxString( _( "Directories" ) );
        FileBrowserLayout += wxT( ";state=2044;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=60;besth=28;minw=60;minh=28;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        FileBrowserLayout += wxT( "name=FileBrowserFilesCtrl;caption=;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=20;besth=20;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1| " );
        FileBrowserLayout += wxT( "dock_size(5,0,0)=10|dock_size(4,0,0)=266|" );
        //m_AuiManager.Update();
    }

    m_AuiManager.LoadPerspective( FileBrowserLayout, true );

	m_DirCtrl->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guFileBrowser::OnDirItemChanged ), NULL, this );
    m_FilesCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,  wxListEventHandler( guFileBrowser::OnFileItemActivated ), NULL, this );
	m_FilesCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guFileBrowser::OnFilesColClick ), NULL, this );

    Connect( ID_FILESYSTEM_FOLDER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPlay ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEnqueue ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_NEW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderNew ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderRename ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderDelete ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopy ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPaste ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEditTracks ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderSaveToPlayList ), NULL, this );
    m_DirCtrl->Connect( ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopyTo ), NULL, this );

    m_DirCtrl->Connect( ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCommand ) );

    Connect( ID_FILESYSTEM_ITEMS_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsPlay ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEnqueue ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEditTracks ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsSaveToPlayList ), NULL, this );
    m_FilesCtrl->Connect( ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCopyTo ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsRename ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsDelete ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCopy ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsPaste ), NULL, this );

    m_FilesCtrl->Connect( ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCommand ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guFileBrowser::~guFileBrowser()
{
    guConfig *  Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( wxT( "VisiblePanels" ), m_VisiblePanels, wxT( "filebrowser" ) );
    Config->WriteStr( wxT( "LastLayout" ), m_AuiManager.SavePerspective(), wxT( "filebrowser" ) );
    Config->WriteStr( wxT( "Path" ), m_DirCtrl->GetPath(), wxT( "filebrowser" ) );

	m_DirCtrl->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guFileBrowser::OnDirItemChanged ), NULL, this );
    m_FilesCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,  wxListEventHandler( guFileBrowser::OnFileItemActivated ), NULL, this );
	m_FilesCtrl->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guFileBrowser::OnFilesColClick ), NULL, this );

    Disconnect( ID_FILESYSTEM_FOLDER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPlay ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEnqueue ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_NEW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderNew ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderRename ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderDelete ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopy ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPaste ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEditTracks ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderSaveToPlayList ), NULL, this );
    m_DirCtrl->Disconnect( ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopyTo ), NULL, this );

    m_DirCtrl->Disconnect( ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCommand ) );

    Disconnect( ID_FILESYSTEM_ITEMS_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsPlay ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEnqueue ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEditTracks ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsSaveToPlayList ), NULL, this );
    m_FilesCtrl->Disconnect( ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCopyTo ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsRename ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsDelete ), NULL, this );

    m_FilesCtrl->Disconnect( ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCommand ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guMediaViewer * FindMediaViewerByPath( guMainFrame * mainframe, const wxString curpath )
{
    const guMediaCollectionArray &Collections = mainframe->GetMediaCollections();
    int Index;
    int Count = Collections.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        const guMediaCollection & Collection = Collections[ Index ];
        if( mainframe->IsCollectionActive( Collection.m_UniqueId ) )
        {
            int PathIndex;
            int PathCount = Collection.m_Paths.Count();
            for( PathIndex = 0; PathIndex < PathCount; PathIndex++ )
            {
                guLogMessage( wxT( "%s == %s" ), curpath.c_str(), Collection.m_Paths[ PathIndex ].c_str() );
                if( curpath.StartsWith( Collection.m_Paths[ PathIndex ] ) )
                {
                    return mainframe->FindCollectionMediaViewer( Collection.m_UniqueId );
                }
            }
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnDirItemChanged( wxTreeEvent &event )
{
    wxString CurPath = m_DirCtrl->GetPath();
    if( !CurPath.EndsWith( wxT( "/" ) ) )
        CurPath.Append( wxT( "/" ) );

    guLogMessage( wxT( "guFileBrowser::OnDirItemChanged( '%s' )" ), CurPath.c_str() );

    if( m_DirCtrl->GetShowPaths() && guFILEBROWSER_SHOWPATH_LOCATIONS )
    {
        m_MediaViewer = FindMediaViewerByPath( m_MainFrame, CurPath );
        m_Db = m_MediaViewer ? m_MediaViewer->GetDb() : NULL;
//        guLogMessage( wxT( "'%s' ==>> '%i'" ), CurPath.c_str(), MediaViewer != NULL );
    }
    else
    {
        m_Db = NULL;
    }

    m_FilesCtrl->SetPath( CurPath, m_MediaViewer );
    m_DirCtrl->SetMediaViewer( m_MediaViewer );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFileItemActivated( wxListEvent &Event )
{
    wxArrayInt Selection = m_FilesCtrl->GetSelectedItems( false );
    if( Selection.Count() )
    {
        if( m_FilesCtrl->GetType( Selection[ 0 ] ) == guFILEITEM_TYPE_FOLDER )
        {
            m_DirCtrl->SetPath( m_FilesCtrl->GetPath( Selection[ 0 ] ), m_MediaViewer );
        }
        else
        {
            wxArrayString Files;
            Files.Add( m_FilesCtrl->GetPath( Selection[ 0 ] ) );

            guConfig * Config = ( guConfig * ) guConfig::Get();
            if( Config )
            {
                if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false , wxT( "general" )) )
                {
                    m_PlayerPanel->AddToPlayList( Files );
                }
                else
                {
                    m_PlayerPanel->SetPlayList( Files );
                }
            }

        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFilesColClick( wxListEvent &event )
{
    int col = event.GetColumn();
    if( col < 0 )
        return;
    m_FilesCtrl->SetOrder( col );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnDirBeginDrag( wxTreeEvent &event )
{
    wxFileDataObject Files;

    wxArrayString FolderFiles = m_FilesCtrl->GetAllFiles( true );
    int Index;
    int Count;
    if( ( Count = FolderFiles.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            Files.AddFile( FolderFiles[ Index ] );
        }

        wxDropSource source( Files, this );

        wxDragResult Result = source.DoDragDrop();
        if( Result )
        {
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderPlay( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetAllFiles( true );
    m_PlayerPanel->SetPlayList( Files );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderEnqueue( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetAllFiles( true );
    if( Files.Count() )
    {
        m_PlayerPanel->AddToPlayList( Files, true, event.GetId() - ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderNew( wxCommandEvent &event )
{
    m_DirCtrl->FolderNew();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderRename( wxCommandEvent &event )
{
    m_DirCtrl->FolderRename();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderDelete( wxCommandEvent &event )
{
    m_DirCtrl->FolderDelete();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderCopy( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnFolderCopy" ) );
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->Clear();
        wxFileDataObject * FileObject = new wxFileDataObject();
        //wxCustomDataObject * GnomeCopiedObject = new wxCustomDataObject( wxDataFormat( wxT( "x-special/gnome-copied-files" ) ) );
        //wxCustomDataObject * UriListObject = new wxCustomDataObject( wxDataFormat( wxT( "text/uri-list" ) ) );
        wxTextDataObject * TextObject = new wxTextDataObject();
        wxDataObjectComposite * CompositeObject = new wxDataObjectComposite();

        wxString Path = m_DirCtrl->GetPath();
        Path.Replace( wxT( "#" ), wxT( "%23" ) );

        TextObject->SetText( Path );
        FileObject->AddFile( Path );

        //Path = wxT( "file://" ) + Path;
        //UriListObject->SetData( Path.Length(), Path.char_str() );

        //Path = wxT( "copy\n" ) + Path;
        //GnomeCopiedObject->SetData( Path.Length(), Path.char_str() );

        //guLogMessage( wxT( "Copy: '%s'" ), Path.c_str() );

        CompositeObject->Add( FileObject );
        //CompositeObject->Add( GnomeCopiedObject );
        //CompositeObject->Add( UriListObject );
        CompositeObject->Add( TextObject );

        //if( !wxTheClipboard->AddData( CustomObject ) )
        if( !wxTheClipboard->AddData( CompositeObject ) )
        //if( !wxTheClipboard->AddData( TextObject ) )
        {
            delete FileObject;
            delete TextObject;
            //delete GnomeCopiedObject;
            //delete UriListObject;
            delete CompositeObject;
            guLogError( wxT( "Can't copy the folder to the clipboard" ) );
        }
        guLogMessage( wxT( "Copied the data to the clipboard..." ) );
        wxTheClipboard->Close();
    }
    else
    {
        guLogError( wxT( "Could not open the clipboard object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderPaste( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnFolderPaste" ) );
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {

//        if( wxTheClipboard->IsSupported( wxDataFormat(  wxT( "text/uri-list" ) ) ) )
//        {
//            guLogMessage( wxT( "Supported format uri-list.." ) );
//
//            wxCustomDataObject CustomDataObject( wxDataFormat( wxT( "text/uri-list" ) ) );
//            if( wxTheClipboard->GetData( CustomDataObject ) )
//            {
//                guLogMessage( wxT( "Custom Data: (%i) '%s'" ), CustomDataObject.GetSize(), wxString( ( const char * ) CustomDataObject.GetData(), wxConvUTF8 ).c_str() );
//            }
//        }
//
//        if( wxTheClipboard->IsSupported( wxDataFormat( wxT( "x-special/gnome-copied-files" ) ) ) )
//        {
//            guLogMessage( wxT( "Supported format x-special..." ) );
//
//            wxCustomDataObject CustomDataObject( wxDataFormat( wxT( "x-special/gnome-copied-files" ) ) ); //( wxT( "Supported format x-special..." ) );
//            if( wxTheClipboard->GetData( CustomDataObject ) )
//            {
//                guLogMessage( wxT( "Custom Data: (%i) '%s'" ), CustomDataObject.GetSize(), wxString( ( const char * ) CustomDataObject.GetData(), wxConvUTF8 ).c_str() );
//            }
//        }
//        else if( wxTheClipboard->IsSupported( wxDF_FILENAME ) )
        if( wxTheClipboard->IsSupported( wxDF_FILENAME ) )
        {
            wxFileDataObject FileObject;
            if( wxTheClipboard->GetData( FileObject ) )
            {
                wxArrayString Files = FileObject.GetFilenames();
                wxArrayString FromFiles;
                //guLogMessage( wxT( "Pasted: %s" ), Files[ 0 ].c_str() );
                int Index;
                int Count = Files.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    if( wxDirExists( Files[ Index ] ) )
                    {
                        //wxFileName::Mkdir( m_DirCtrl->GetPath() + wxT( "/" ) + wxFileNameFromPath( Files[ Index ] ), 0770, wxPATH_MKDIR_FULL );

                        guAddDirItems( Files[ Index ], FromFiles );

                        int FromIndex;
                        int FromCount = FromFiles.Count();
                        for( FromIndex = 0; FromIndex < FromCount; FromIndex++ )
                        {
                            wxString DestName = FromFiles[ FromIndex ];
                            DestName.Replace( wxPathOnly( Files[ Index ] ), m_DirCtrl->GetPath() );
                            wxFileName::Mkdir( wxPathOnly( DestName ), 0770, wxPATH_MKDIR_FULL );
                            guLogMessage( wxT( "Copy file %s to %s" ), FromFiles[ FromIndex ].c_str(), DestName.c_str() );
                            wxCopyFile( FromFiles[ FromIndex ], DestName );
                        }

                    }
                    else
                    {
                        wxCopyFile( Files[ Index ], m_DirCtrl->GetPath() + wxT( "/" ) + wxFileNameFromPath( Files[ Index ] ) );
                    }
                }
                wxString CurPath = m_DirCtrl->GetPath();
                m_DirCtrl->CollapsePath( CurPath );
                m_DirCtrl->ExpandPath( CurPath );
            }
            else
            {
                guLogError( wxT( "Can't paste the data from the clipboard" ) );
            }

        }
        wxTheClipboard->Close();
    }
    else
    {
        guLogError( wxT( "Could not open the clipboard object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderEditTracks( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    m_FilesCtrl->GetAllSongs( &Tracks );

    if( Tracks.Count() )
    {
        guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics, &ChangedFlags );

        if( TrackEditor )
        {
            if( TrackEditor->ShowModal() == wxID_OK )
            {
                guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
                if( m_Db )
                    m_Db->UpdateSongs( &Tracks, ChangedFlags );
                else
                    m_DefaultDb->UpdateSongs( &Tracks, ChangedFlags );
                //guUpdateLyrics( Tracks, Lyrics, ChangedFlags );
                //guUpdateImages( Tracks, Images, ChangedFlags );

                // Update the track in database, playlist, etc
                ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &Tracks );
            }
            guImagePtrArrayClean( &Images );
            TrackEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderSaveToPlayList( wxCommandEvent &event )
{
    guTrackArray Tracks;

    m_FilesCtrl->GetAllSongs( &Tracks );
    wxArrayInt TrackIds;
    int Index;
    int Count = Tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        TrackIds.Add( Tracks[ Index ].m_SongId );
    }

    if( m_Db && TrackIds.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists,guPLAYLIST_TYPE_STATIC );
        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( wxTheApp->GetTopWindow(), m_Db, &TrackIds, &PlayLists );
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
                m_Db->CreateStaticPlayList( PLName, TrackIds );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, TrackIds );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                {
                    m_Db->AppendStaticPlayList( PLId, TrackIds );
                }
            }

            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }

        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_FilesCtrl->GetAllSongs( Tracks );

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
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsPlay( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles( true );

    int Index;
    int Count = Files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guLogMessage( wxT( "File%i: '%s'" ), Index, Files[ Index ].c_str() );
    }
    if( Files.Count() )
    {
        m_PlayerPanel->SetPlayList( Files );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsEnqueue( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles( true );
    if( Files.Count() )
    {
        m_PlayerPanel->AddToPlayList( Files, true, event.GetId() - ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsEditTracks( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    m_FilesCtrl->GetSelectedSongs( &Tracks );

    if( Tracks.Count() )
    {
        guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics, &ChangedFlags );

        if( TrackEditor )
        {
            if( TrackEditor->ShowModal() == wxID_OK )
            {
                guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
                if( m_Db )
                    m_Db->UpdateSongs( &Tracks, ChangedFlags );
                else
                    m_DefaultDb->UpdateSongs( &Tracks, ChangedFlags );
                //guUpdateLyrics( Tracks, Lyrics, ChangedFlags );
                //guUpdateImages( Tracks, Images, ChangedFlags );

                // Update the track in database, playlist, etc
                ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &Tracks );
            }
            guImagePtrArrayClean( &Images );
            TrackEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsSaveToPlayList( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_FilesCtrl->GetSelectedSongs( &Tracks );

    wxArrayInt TrackIds;
    int Index;
    int Count = Tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        TrackIds.Add( Tracks[ Index ].m_SongId );
    }

    if( m_Db && TrackIds.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );
        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( wxTheApp->GetTopWindow(), m_Db, &TrackIds, &PlayLists );
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
                m_Db->CreateStaticPlayList( PLName, TrackIds );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, TrackIds );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                {
                    m_Db->AppendStaticPlayList( PLId, TrackIds );
                }
            }

            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }

        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_FilesCtrl->GetSelectedSongs( Tracks );

    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index > guCOPYTO_DEVICE_BASE )
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
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsRename( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles( true );
    if( Files.Count() )
    {
        guFileRenamer * FileRenamer = new guFileRenamer( this, m_Db, Files );
        if( FileRenamer )
        {
            if( FileRenamer->ShowModal() == wxID_OK )
            {
                wxArrayString RenamedFiles = FileRenamer->GetRenamedNames();
                int Index;
                int Count = RenamedFiles.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    if( Files[ Index ] != RenamedFiles[ Index ] )
                    {
                        wxString NewDirName = wxPathOnly( RenamedFiles[ Index ] );
                        if( !wxDirExists( NewDirName ) )
                        {
                            wxFileName::Mkdir( NewDirName, 0770, wxPATH_MKDIR_FULL );
                        }

                        //if( wxFileExists( Files[ Index ] ) )
                        if( !guRenameFile( Files[ Index ], RenamedFiles[ Index ] ) )
                        {
                            guLogError( wxT( "Could no rename '%s' to '%s'" ),
                                Files[ Index ].c_str(),
                                RenamedFiles[ Index ].c_str() );
                        }

                        m_Db->UpdateTrackFileName( Files[ Index ], RenamedFiles[ Index ] );
                    }
                }

                //m_DirCtrl->ExpandPath( m_DirCtrl->GetPath() );
                m_FilesCtrl->SetPath( m_DirCtrl->GetPath(), m_MediaViewer );
            }
            FileRenamer->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsDelete( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles();

    int Index;
    int Count;
    bool Error = false;
    if( ( Count = Files.Count() ) )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected files ?" ),
                         _( "Confirm" ),
                         wxICON_QUESTION | wxYES_NO, this ) == wxYES )
        {
            wxArrayString DeleteFiles;
            for( Index = 0; Index < Count; Index++ )
            {
                if( wxDirExists( Files[ Index ] ) )
                {
                    Error = !RemoveDirItems( Files[ Index ], &DeleteFiles ) || !wxRmdir( Files[ Index ] );
                }
                else
                {
                    Error = !wxRemoveFile( Files[ Index ] );
                    DeleteFiles.Add( Files[ Index ] );
                }
                if( Error )
                {
                    if( wxMessageBox( _( "There was an error deleting " ) + wxFileNameFromPath( Files[ Index ] ) +
                                      _( "\nContinue deleting?" ),
                                     _( "Continue" ),
                                     wxICON_QUESTION | wxYES_NO, this ) == wxNO )
                    {
                        break;
                    }
                    //Error = false;
                }
            }
            wxString CurrentFolder = m_DirCtrl->GetPath();
            m_DirCtrl->CollapsePath( CurrentFolder );
            m_DirCtrl->ExpandPath( CurrentFolder );
            //
            //m_Db->DoCleanUp();
            if( m_Db )
                m_Db->CleanFiles( DeleteFiles );
            else
                m_DefaultDb->CleanFiles( DeleteFiles );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderCommand( wxCommandEvent &event )
{
    int Index;
    int Count;
    Index = event.GetId();

    guConfig * Config = ( guConfig * ) Config->Get();
    if( Config )
    {
        wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );

        Index -= ID_COMMANDS_BASE;
        wxString CurCmd = Commands[ Index ];
        if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
        {
            wxString DirPath = m_DirCtrl->GetPath();
            DirPath.Replace( wxT( " " ), wxT( "\\ " ) );
            CurCmd.Replace( wxT( "{bp}" ), DirPath );
        }

        if( CurCmd.Find( wxT( "{tp}" ) ) != wxNOT_FOUND )
        {
            wxString SongList;
            wxArrayString Files = m_FilesCtrl->GetAllFiles( true );
            Count = Files.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                SongList += wxT( " \"" ) + Files[ Index ] + wxT( "\"" );
            }
            CurCmd.Replace( wxT( "{tp}" ), SongList.Trim( false ) );
        }

        //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
        guExecute( CurCmd );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsCommand( wxCommandEvent &event )
{
    int Index;
    int Count;
    Index = event.GetId();

    guConfig * Config = ( guConfig * ) Config->Get();
    if( Config )
    {
        wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );

        Index -= ID_COMMANDS_BASE;
        wxString CurCmd = Commands[ Index ];
        if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
        {
            wxString DirPath = m_DirCtrl->GetPath();
            DirPath.Replace( wxT( " " ), wxT( "\\ " ) );
            CurCmd.Replace( wxT( "{bp}" ), DirPath );
        }

        if( CurCmd.Find( wxT( "{bc}" ) ) != wxNOT_FOUND )
        {
            wxString SongList;
            wxArrayString Files = m_FilesCtrl->GetSelectedFiles( false );
            Count = Files.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                SongList += wxT( " \"" ) + Files[ Index ] + wxT( "\"" );
            }
            CurCmd.Replace( wxT( "{bc}" ), SongList.Trim( false ) );
        }

        if( CurCmd.Find( wxT( "{tp}" ) ) != wxNOT_FOUND )
        {
            wxString SongList;
            wxArrayString Files = m_FilesCtrl->GetSelectedFiles( true );
            Count = Files.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                SongList += wxT( " \"" ) + Files[ Index ] + wxT( "\"" );
            }
            CurCmd.Replace( wxT( "{tp}" ), SongList.Trim( false ) );
        }

        //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
        guExecute( CurCmd );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsCopy( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles( false );
    if( Files.Count() )
    {
        wxTheClipboard->UsePrimarySelection( false );
        if( wxTheClipboard->Open() )
        {
            wxTheClipboard->Clear();
            wxFileDataObject * FileObject = new wxFileDataObject();
            wxTextDataObject * TextObject = new wxTextDataObject();
            wxDataObjectComposite * CompositeObject = new wxDataObjectComposite();

            wxString FilesText;
            int Index;
            int Count = Files.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                wxString CurFile = Files[ Index ];
                FilesText += ( FilesText.IsEmpty() ? wxT( "" ) : wxT( "\n" ) ) + CurFile;

                CurFile.Replace( wxT( "#" ), wxT( "%23" ) );
                FileObject->AddFile( CurFile );
            }

            TextObject->SetText( FilesText );

            CompositeObject->Add( FileObject );
            CompositeObject->Add( TextObject );

            if( !wxTheClipboard->AddData( CompositeObject ) )
            {
                delete FileObject;
                delete TextObject;
                delete CompositeObject;
                guLogError( wxT( "Can't copy the selected files to the clipboard" ) );
            }
            wxTheClipboard->Close();
        }
        else
        {
            guLogError( wxT( "Could not open the clipboard object" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsPaste( wxCommandEvent &event )
{
    wxString DestFolder;
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_FILENAME ) )
        {
            wxArrayString Selection = m_FilesCtrl->GetSelectedFiles( false );
            if( ( Selection.Count() == 1 ) && wxDirExists( Selection[ 0 ] ) )
            {
                DestFolder = Selection[ 0 ];
            }
            else
            {
                DestFolder = m_DirCtrl->GetPath();
            }

            wxFileDataObject FileObject;
            if( wxTheClipboard->GetData( FileObject ) )
            {
                wxArrayString Files = FileObject.GetFilenames();
                wxArrayString FromFiles;
                //guLogMessage( wxT( "Pasted: %s" ), Files[ 0 ].c_str() );
                int Index;
                int Count = Files.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    if( wxDirExists( Files[ Index ] ) )
                    {
                        guAddDirItems( Files[ Index ], FromFiles );

                        int FromIndex;
                        int FromCount = FromFiles.Count();
                        for( FromIndex = 0; FromIndex < FromCount; FromIndex++ )
                        {
                            wxString DestName = FromFiles[ FromIndex ];
                            DestName.Replace( wxPathOnly( Files[ Index ] ), DestFolder );
                            wxFileName::Mkdir( wxPathOnly( DestName ), 0770, wxPATH_MKDIR_FULL );
                            guLogMessage( wxT( "Copy file %s to %s" ), FromFiles[ FromIndex ].c_str(), DestName.c_str() );
                            wxCopyFile( FromFiles[ FromIndex ], DestName );
                        }
                    }
                    else
                    {
                        wxCopyFile( Files[ Index ], m_DirCtrl->GetPath() + wxT( "/" ) + wxFileNameFromPath( Files[ Index ] ) );
                    }
                }
                wxString CurPath = m_DirCtrl->GetPath();
                m_DirCtrl->CollapsePath( CurPath );
                m_DirCtrl->ExpandPath( CurPath );
            }
            else
            {
                guLogError( wxT( "Can't paste the data from the clipboard" ) );
            }
        }
        wxTheClipboard->Close();
    }
    else
    {
        guLogError( wxT( "Could not open the clipboard object" ) );
    }
}

// -------------------------------------------------------------------------------- //
