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
#include "FileBrowser.h"

#include "AuiDockArt.h"
#include "Config.h"
#include "FileRenamer.h"
#include "Images.h"
#include "LibUpdate.h"
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
guGenericDirCtrl::guGenericDirCtrl( wxWindow * parent, const bool showlibpath  ) :
              wxGenericDirCtrl( parent, wxID_ANY, wxDirDialogDefaultFolderStr,
                wxDefaultPosition, wxDefaultSize, wxDIRCTRL_DIR_ONLY|wxDIRCTRL_3D_INTERNAL|wxSUNKEN_BORDER,
                wxEmptyString, 0, wxTreeCtrlNameStr )
{
    m_ShowLibPaths = showlibpath;
    m_FileBrowserDirCtrl = ( guFileBrowserDirCtrl * ) parent;

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
    if( m_ShowLibPaths )
    {
        wxString CurPath = GetPath();
        ReCreateTree();
        SetPath( CurPath );
    }
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::SetupSections()
{
    if( m_ShowLibPaths )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );
        int Index;
        int Count = LibPaths.Count();
        if( Count )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                wxString LibName = LibPaths[ Index ];
                if( LibName.EndsWith( wxT( "/" ) ) )
                    LibName.RemoveLast();
                AddSection( LibPaths[ Index ], wxFileNameFromPath( LibName ), 1 );
            }
        }
        else
        {
            AddSection( wxT( "/" ), wxT( "/" ), 1 );
        }
    }
    else
    {
        AddSection( wxT( "/" ), wxT( "/" ), 1 );
    }
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnBeginRenameDir( wxTreeEvent &event )
{
    m_RenameName = GetPath();
    OnBeginEditItem( event );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnEndRenameDir( wxTreeEvent &event )
{
    OnEndEditItem( event );
    m_FileBrowserDirCtrl->RenamedDir( m_RenameName, GetPath() );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::FolderRename( void )
{
    wxTreeCtrl * TreeCtrl = GetTreeCtrl();
    TreeCtrl->EditLabel( TreeCtrl->GetSelection() );
}

// -------------------------------------------------------------------------------- //
// guFileBrowserDirCtrl
// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::guFileBrowserDirCtrl( wxWindow * parent, guDbLibrary * db, const wxString &dirpath ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_AddingFolder = false;

    guConfig * Config = ( guConfig * ) guConfig::Get();

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

    bool ShowLibPath = Config->ReadBool( wxT( "ShowLibPaths" ), true, wxT( "FileBrowser" ) );
	m_DirCtrl = new guGenericDirCtrl( this, ShowLibPath );
	m_DirCtrl->SetPath( dirpath );

	m_DirCtrl->ShowHidden( false );
	MainSizer->Add( m_DirCtrl, 1, wxEXPAND, 5 );

	wxBoxSizer* DirBtnSizer;
	DirBtnSizer = new wxBoxSizer( wxHORIZONTAL );


	DirBtnSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_ShowLibPathsBtn = new wxToggleBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_library ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_ShowLibPathsBtn->SetToolTip( _( "Switch between see all filesystem and only library locations" ) );
	m_ShowLibPathsBtn->SetValue( ShowLibPath );
	DirBtnSizer->Add( m_ShowLibPathsBtn, 0, wxALL, 5 );

	MainSizer->Add( DirBtnSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    m_DirCtrl->Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guFileBrowserDirCtrl::OnContextMenu ), NULL, this );
	m_ShowLibPathsBtn->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guFileBrowserDirCtrl::OnShowLibPathsClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::~guFileBrowserDirCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( wxT( "ShowLibPaths" ), m_ShowLibPathsBtn->GetValue(), wxT( "FileBrowser" ) );
    m_DirCtrl->Disconnect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( guFileBrowserDirCtrl::OnContextMenu ), NULL, this );
	m_ShowLibPathsBtn->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( guFileBrowserDirCtrl::OnShowLibPathsClick ), NULL, this );
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
    wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
    wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "Commands" ) );
    if( ( Count = Commands.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( ( Commands[ Index ].Find( wxT( "{bc}" ) ) == wxNOT_FOUND ) )
            {
                MenuItem = new wxMenuItem( menu, ID_FILESYSTEM_FOLDER_COMMANDS + Index, Names[ Index ], Commands[ Index ] );
                SubMenu->Append( MenuItem );
            }
        }
    }
    else
    {
        MenuItem = new wxMenuItem( menu, -1, _( "No commands defined" ), _( "Add commands in preferences" ) );
        SubMenu->Append( MenuItem );
    }
    menu->AppendSeparator();
    menu->AppendSubMenu( SubMenu, _( "Commands" ) );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPoint();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_PLAY, _( "Play" ), _( "Play the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_ENQUEUE, _( "Enqueue" ), _( "Add the selected folder to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_EDITTRACKS, _( "Edit Tracks" ), _( "Edit the tracks in the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST, _( "Save to Playlist" ), _( "Add the tracks in the selected folder to a playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_COPYTO, _( "Copy to..." ), _( "Copy the selected folder to a folder or device" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_COPY, _( "Copy" ), _( "Copy the selected folder to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    Menu.Append( MenuItem );
    //MenuItem->Enable( false );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_PASTE, _( "Paste" ), _( "Paste to the selected folder" ) );
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
    Menu.Append( MenuItem );

    AppendFolderCommands( &Menu );

    PopupMenu( &Menu, Point );
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::RenamedDir( const wxString &oldname, const wxString &newname )
{
    //guLogMessage( wxT( "'%s' -> '%s'" ), oldname.c_str(), newname.c_str() );

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
            m_Db->UpdatePaths( oldname, newname );
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

        wxString NewDirName = m_DirCtrl->GetPath() + wxT( "/" ) + _( "New Folder" );
        int Index = 1;
        while( wxDirExists( NewDirName ) )
        {
            NewDirName = m_DirCtrl->GetPath() + wxT( "/" ) + _( "New Folder" );
            NewDirName += wxString::Format( wxT( "%i" ), Index++ );
        }

        if( wxMkdir( NewDirName, 0770 ) )
        {
            TreeCtrl->Collapse( FolderParent );
            //TreeCtrl->Expand( m_AddFolderParent );
            m_DirCtrl->ExpandPath( NewDirName );
            wxTextCtrl * TextCtrl = TreeCtrl->EditLabel( TreeCtrl->GetSelection() );
            TextCtrl->SetSelection( -1, -1 );
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
        m_Db->CleanFiles( DeleteFiles );
    }
}


// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnShowLibPathsClick( wxCommandEvent& event )
{
    wxString CurPath = m_DirCtrl->GetPath();
    m_DirCtrl->SetShowLibPaths( event.IsChecked() );
    m_DirCtrl->ReCreateTree();
    m_DirCtrl->SetPath( CurPath );
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

    m_Order = Config->ReadNum( wxT( "Order" ), 0, wxT( "FileBrowser" ) );
    m_OrderDesc = Config->ReadNum( wxT( "OrderDesc" ), false, wxT( "FileBrowser" ) );

    int ColId;
    wxString ColName;
    wxArrayString ColumnNames = GetColumnNames();
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "FileBrowserCol%u" ), index ), index, wxT( "FileBrowserColumns" ) );

        ColName = ColumnNames[ ColId ];

        ColName += ( ( ColId == m_Order ) ? ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "FileBrowserColWidth%u" ), index ), 80, wxT( "FileBrowserColumns" ) ),
            Config->ReadBool( wxString::Format( wxT( "FileBrowserColShow%u" ), index ), true, wxT( "FileBrowserColumns" ) )
            );
        InsertColumn( Column );
    }

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guFilesListBox::~guFilesListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString ColumnNames = GetColumnNames();
    int index;
    int count = ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "FileBrowserCol%u" ), index ),
                          ( * m_Columns )[ index ].m_Id, wxT( "FileBrowserColumns" ) );
        Config->WriteNum( wxString::Format( wxT( "FileBrowserColWidth%u" ), index ),
                          ( * m_Columns )[ index ].m_Width, wxT( "FileBrowserColumns" ) );
        Config->WriteBool( wxString::Format( wxT( "FileBrowserColShow%u" ), index ),
                           ( * m_Columns )[ index ].m_Enabled, wxT( "FileBrowserColumns" ) );
    }

    Config->WriteNum( wxT( "Order" ), m_Order, wxT( "FileBrowser" ) );
    Config->WriteBool( wxT( "OrderDesc" ), m_OrderDesc, wxT( "FileBrowser" ) );
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
        int ImageIndex = guFILEITEM_IMAGE_INDEX_OTHER;
        if( FileItem->m_Type == guFILEITEM_TYPE_FOLDER )
        {
            ImageIndex = guFILEITEM_IMAGE_INDEX_FOLDER;
        }
        else if( FileItem->m_Type == guFILEITEM_TYPE_AUDIO )
        {
            ImageIndex = guFILEITEM_IMAGE_INDEX_AUDIO;
        }
        else if( FileItem->m_Type == guFILEITEM_TYPE_IMAGE )
        {
            ImageIndex = guFILEITEM_IMAGE_INDEX_IMAGE;
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
    wxASSERT( m_Db );

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
    wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
    wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "Commands" ) );
    if( ( Count = Commands.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            if( ( Commands[ Index ].Find( wxT( "{bc}" ) ) == wxNOT_FOUND ) ||
                ( ( selcount == 1 ) && ( seltype == guFILEITEM_TYPE_IMAGE ) ) )
            {
                MenuItem = new wxMenuItem( menu, ID_FILESYSTEM_ITEMS_COMMANDS + Index, Names[ Index ], Commands[ Index ] );
                SubMenu->Append( MenuItem );
            }
        }
    }
    else
    {
        MenuItem = new wxMenuItem( menu, -1, _( "No commands defined" ), _( "Add commands in preferences" ) );
        SubMenu->Append( MenuItem );
    }
    menu->AppendSeparator();
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
        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_PLAY, _( "Play" ), _( "Play current selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_ENQUEUE, _( "Enqueue" ), _( "Add current selected files to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_COPY, _( "Copy" ), _( "Copy the selected folder to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    Menu->Append( MenuItem );
    //MenuItem->Enable( false );

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

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_EDITTRACKS, _( "Edit tracks" ), _( "Edit the current selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_SAVEPLAYLIST, _( "Save to Playlist" ), _( "Add the current selected tracks to a playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_COPYTO, _( "Copy to..." ), _( "Copy the selected files to a folder or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_RENAME, _( "Rename files" ), _( "Rename the current selected file" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_ ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_DELETE, _( "Delete" ), _( "Delete the selected files" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_ ) );
        Menu->Append( MenuItem );

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

                    if( !m_Db->FindTrackFile( FileName, Track ) )
                    {
                        guPodcastItem PodcastItem;
                        if( m_Db->GetPodcastItemFile( FileName, &PodcastItem ) )
                        {
                            delete Track;
                            continue;
                        }
                        else
                        {
                            //guLogMessage( wxT( "Reading tags from the file..." ) );
                            guTagInfo * TagInfo = guGetTagInfoHandler( FileName );
                            if( TagInfo )
                            {
                                Track->m_Type = guTRACK_TYPE_NOTDB;

                                TagInfo->Read();

                                Track->m_ArtistName  = TagInfo->m_ArtistName;
                                Track->m_AlbumName   = TagInfo->m_AlbumName;
                                Track->m_SongName    = TagInfo->m_TrackName;
                                Track->m_Number      = TagInfo->m_Track;
                                Track->m_GenreName   = TagInfo->m_GenreName;
                                Track->m_Length      = TagInfo->m_Length;
                                Track->m_Year        = TagInfo->m_Year;
                                Track->m_Rating      = wxNOT_FOUND;
                                Track->m_CoverId     = 0;

                                delete TagInfo;
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
       files->AddFile( SelectFiles[ index ] );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::SetPath( const wxString &path )
{
    guLogMessage( wxT( "SetPath: %s" ), path.c_str() );
    m_CurDir = path;
    if( !m_CurDir.EndsWith( wxT( "/" ) ) )
        m_CurDir += wxT( "/" );
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
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;
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
// guFileBrowser
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE( guFileBrowser, wxPanel )
    EVT_TREE_BEGIN_DRAG( wxID_ANY, guFileBrowser::OnDirBeginDrag)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guFileBrowser::guFileBrowser( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_PlayerPanel = playerpanel;

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

    m_VisiblePanels = Config->ReadNum( wxT( "FBVisiblePanels" ), guPANEL_FILEBROWSER_VISIBLE_DEFAULT, wxT( "Positions" ) );


    wxArrayString LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );
    if( !LibPaths.Count() )
        LibPaths.Add( wxGetHomeDir() );

    m_DirCtrl = new guFileBrowserDirCtrl( this, m_Db,
        Config->ReadStr( wxT( "Path" ), LibPaths[ 0 ], wxT( "FileBrowser" ) ) );

    m_AuiManager.AddPane( m_DirCtrl,
            wxAuiPaneInfo().Name( wxT( "FileBrowserDirCtrl" ) ).Caption( _( "Directories" ) ).
            MinSize( 60, 28 ).Row( 0 ).Layer( 0 ).Position( 0 ).
            CloseButton( false ).
            Dockable( true ).Left() );

    m_FilesCtrl = new guFileBrowserFileCtrl( this, db, m_DirCtrl );
    m_FilesCtrl->SetPath( m_DirCtrl->GetPath() );

    m_AuiManager.AddPane( m_FilesCtrl,
            wxAuiPaneInfo().Name( wxT( "FileBrowserFilesCtrl" ) ).
            Dockable( true ).CenterPane() );

    wxString FileBrowserLayout = Config->ReadStr( wxT( "FileBrowser" ), wxEmptyString, wxT( "Positions" ) );
    if( Config->GetIgnoreLayouts() || FileBrowserLayout.IsEmpty() )
    {
        m_AuiManager.Update();
        m_VisiblePanels = guPANEL_FILEBROWSER_VISIBLE_DEFAULT;
    }
    else
    {
        m_AuiManager.LoadPerspective( FileBrowserLayout, true );
    }

	m_DirCtrl->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guFileBrowser::OnDirItemChanged ), NULL, this );
    m_FilesCtrl->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,  wxListEventHandler( guFileBrowser::OnFileItemActivated ), NULL, this );
	m_FilesCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guFileBrowser::OnFilesColClick ), NULL, this );

    Connect( ID_FILESYSTEM_FOLDER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPlay ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEnqueue ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_NEW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderNew ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderRename ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderDelete ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopy ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPaste ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEditTracks ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderSaveToPlayList ), NULL, this );
    Connect( ID_FILESYSTEM_FOLDER_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopyTo ), NULL, this );

    Connect( ID_FILESYSTEM_FOLDER_COMMANDS, ID_FILESYSTEM_FOLDER_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCommand ) );

    Connect( ID_FILESYSTEM_ITEMS_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsPlay ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEnqueue ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEditTracks ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsSaveToPlayList ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCopyTo ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsRename ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsDelete ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCopy ), NULL, this );
    Connect( ID_FILESYSTEM_ITEMS_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsPaste ), NULL, this );

    Connect( ID_FILESYSTEM_ITEMS_COMMANDS, ID_FILESYSTEM_ITEMS_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCommand ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guFileBrowser::~guFileBrowser()
{
    guConfig *  Config = ( guConfig * ) guConfig::Get();

    Config->WriteNum( wxT( "FBVisiblePanels" ), m_VisiblePanels, wxT( "Positions" ) );
    Config->WriteStr( wxT( "FileBrowser" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
    Config->WriteStr( wxT( "Path" ), m_DirCtrl->GetPath(), wxT( "FileBrowser" ) );

	m_DirCtrl->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guFileBrowser::OnDirItemChanged ), NULL, this );
    m_FilesCtrl->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,  wxListEventHandler( guFileBrowser::OnFileItemActivated ), NULL, this );
	m_FilesCtrl->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guFileBrowser::OnFilesColClick ), NULL, this );

    Disconnect( ID_FILESYSTEM_FOLDER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPlay ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEnqueue ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_NEW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderNew ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderRename ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderDelete ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopy ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderPaste ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderEditTracks ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderSaveToPlayList ), NULL, this );
    Disconnect( ID_FILESYSTEM_FOLDER_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCopyTo ), NULL, this );

    Disconnect( ID_FILESYSTEM_FOLDER_COMMANDS, ID_FILESYSTEM_FOLDER_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnFolderCommand ) );

    Disconnect( ID_FILESYSTEM_ITEMS_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsPlay ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEnqueue ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsEditTracks ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_SAVEPLAYLIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsSaveToPlayList ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCopyTo ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsRename ), NULL, this );
    Disconnect( ID_FILESYSTEM_ITEMS_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsDelete ), NULL, this );

    Disconnect( ID_FILESYSTEM_ITEMS_COMMANDS, ID_FILESYSTEM_ITEMS_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guFileBrowser::OnItemsCommand ), NULL, this );

    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnDirItemChanged( wxTreeEvent &event )
{
    guLogMessage( wxT( "The current selected directory is '%s'" ), m_DirCtrl->GetPath().c_str() );
    m_FilesCtrl->SetPath( m_DirCtrl->GetPath() );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFileItemActivated( wxListEvent &Event )
{
    wxArrayInt Selection = m_FilesCtrl->GetSelectedItems( false );
    if( Selection.Count() )
    {
        if( m_FilesCtrl->GetType( Selection[ 0 ] ) == guFILEITEM_TYPE_FOLDER )
        {
            m_DirCtrl->SetPath( m_FilesCtrl->GetPath( Selection[ 0 ] ) );
        }
        else
        {
            wxArrayString Files;
            Files.Add( m_FilesCtrl->GetPath( Selection[ 0 ] ) );

            guConfig * Config = ( guConfig * ) guConfig::Get();
            if( Config )
            {
                if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false , wxT( "General" )) )
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
    m_PlayerPanel->AddToPlayList( Files );
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

    m_FilesCtrl->GetAllSongs( &Tracks );

    if( Tracks.Count() )
    {
        guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics );

        if( TrackEditor )
        {
            if( TrackEditor->ShowModal() == wxID_OK )
            {
                m_Db->UpdateSongs( &Tracks );
                UpdateImages( Tracks, Images );
                UpdateLyrics( Tracks, Lyrics );

                // Update the track in database, playlist, etc
                ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &Tracks );
            }
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

    if( Tracks.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists,GUPLAYLIST_STATIC );
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

    event.SetId( ID_MAINFRAME_COPYTO );
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
        m_PlayerPanel->AddToPlayList( Files );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsEditTracks( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;

    m_FilesCtrl->GetSelectedSongs( &Tracks );

    if( Tracks.Count() )
    {
        guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics );

        if( TrackEditor )
        {
            if( TrackEditor->ShowModal() == wxID_OK )
            {
                m_Db->UpdateSongs( &Tracks );
                UpdateImages( Tracks, Images );
                UpdateLyrics( Tracks, Lyrics );

                // Update the track in database, playlist, etc
                ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &Tracks );
            }
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

    if( Tracks.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists,GUPLAYLIST_STATIC );
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

    event.SetId( ID_MAINFRAME_COPYTO );
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
                        if( !wxDirExists( wxPathOnly( RenamedFiles[ Index ] ) ) )
                        {
                            wxFileName::Mkdir( wxPathOnly( RenamedFiles[ Index ] ), 0770, wxPATH_MKDIR_FULL );
                        }

                        //if( wxFileExists( Files[ Index ] ) )
                        if( !wxRenameFile( Files[ Index ], RenamedFiles[ Index ] ) )
                        {
                            guLogError( wxT( "Could no rename '%s' to '%s'" ),
                                Files[ Index ].c_str(),
                                RenamedFiles[ Index ].c_str() );
                        }
                    }
                }
                //m_DirCtrl->ExpandPath( m_DirCtrl->GetPath() );
                m_FilesCtrl->SetPath( m_DirCtrl->GetPath() );
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
            m_Db->CleanFiles( DeleteFiles );
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
        wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );

        Index -= ID_FILESYSTEM_FOLDER_COMMANDS;
        wxString CurCmd = Commands[ Index ];
        if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
        {
            CurCmd.Replace( wxT( "{bp}" ), wxT( "\"" ) + m_DirCtrl->GetPath() + wxT( "\"" ) );
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
        wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );

        Index -= ID_FILESYSTEM_ITEMS_COMMANDS;
        wxString CurCmd = Commands[ Index ];
        if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
        {
            CurCmd.Replace( wxT( "{bp}" ), wxT( "\"" ) + m_DirCtrl->GetPath() + wxT( "\"" ) );
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
