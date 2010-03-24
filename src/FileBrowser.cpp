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
#include "Utils.h"

#include <wx/aui/aui.h>
#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY(guFileItemArray);

// -------------------------------------------------------------------------------- //
// guFileBrowserDirCtrl
// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::guFileBrowserDirCtrl( wxWindow * parent, const wxString &dirpath ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_DirCtrl = new wxGenericDirCtrl( this, wxID_ANY, dirpath, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_DIR_ONLY|wxSUNKEN_BORDER, wxEmptyString, 0 );

	m_DirCtrl->ShowHidden( false );
	MainSizer->Add( m_DirCtrl, 1, wxEXPAND | wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();
}

// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::~guFileBrowserDirCtrl()
{
}




// -------------------------------------------------------------------------------- //
// guFilesListBox
// -------------------------------------------------------------------------------- //
wxString guFILES_COLUMN_NAMES[] = {
    _( "Name" ),
    _( "Size" ),
    _( "Modified" )
};

// -------------------------------------------------------------------------------- //
guFilesListBox::guFilesListBox( wxWindow * parent, guDbLibrary * db ) :
    guListView( parent, wxLB_MULTIPLE | guLISTVIEW_COLUMN_SELECT | guLISTVIEW_COLUMN_SORTING | guLISTVIEW_ALLOWDRAG )
{
    m_Db = db;
    //m_CurDir = wxEmptyString;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_Order = Config->ReadNum( wxT( "Order" ), 0, wxT( "FileBrowser" ) );
    m_OrderDesc = Config->ReadNum( wxT( "OrderDesc" ), false, wxT( "FileBrowser" ) );

    int ColId;
    wxString ColName;
    int index;
    int count = sizeof( guFILES_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "FileBrowserCol%u" ), index ), index, wxT( "FileBrowserColumns" ) );

        ColName = guFILES_COLUMN_NAMES[ ColId ];

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
    int index;
    int count = sizeof( guFILES_COLUMN_NAMES ) / sizeof( wxString );
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
wxString guFilesListBox::OnGetItemText( const int row, const int col ) const
{
    guFileItem * FileItem;
    FileItem = &m_Files[ row ];
    switch( ( * m_Columns )[ col ].m_Id )
    {
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
void inline GetFileDetails( const wxString &filename, guFileItem * fileitem )
{
    wxStructStat St;
    wxStat( filename, &St );
    fileitem->m_Size = St.st_size;
    fileitem->m_Time = St.st_ctime;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::GetItemsList( void )
{
    if( !m_CurDir.IsEmpty() && wxDirExists( m_CurDir ) )
    {
        wxDir Dir( m_CurDir );
        if( Dir.IsOpened() )
        {
            wxString FileName;
            if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES ) )
            {
                do {
                    guFileItem * FileItem = new guFileItem();
                    FileItem->m_Name = FileName;
                    GetFileDetails( m_CurDir + FileName, FileItem );
                    m_Files.Add( FileItem );
                } while( Dir.GetNext( &FileName ) );
            }
        }
    }

//    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_UPDATE_SELINFO );
//    AddPendingEvent( event );
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
void guFilesListBox::CreateContextMenu( wxMenu * Menu ) const
{
//    wxArrayInt Selection = GetSelectedItems();
//    if( Selection.Count() )
//    {
//        wxMenuItem * MenuItem;
//        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_PLAY, _( "Play" ), _( "Play current selected songs" ) );
//        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
//        Menu->Append( MenuItem );
//
//        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
//        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
//        Menu->Append( MenuItem );
//
//        Menu->AppendSeparator();
//
//        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_DEL, _( "Delete" ), _( "Delete the current selected podcasts" ) );
//        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_del ) );
//        Menu->Append( MenuItem );
//
//        Menu->AppendSeparator();
//
//        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_DOWNLOAD, _( "Download" ), _( "Download the current selected podcasts" ) );
//        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
//        Menu->Append( MenuItem );
//
//        Menu->AppendSeparator();
//        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_COPYTO, _( "Copy to..." ), _( "Copy the current selected podcasts to a directory or device" ) );
//        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
//        Menu->Append( MenuItem );
//    }
//    else
//    {
//        wxMenuItem * MenuItem;
//        MenuItem = new wxMenuItem( Menu, wxID_ANY, _( "No selected items..." ), _( "Copy the current selected podcasts to a directory or device" ) );
//        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_status_error ) );
//        Menu->Append( MenuItem );
//    }
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

////    m_Db->SetPodcastOrder( m_Order );
//
//    int CurColId;
//    int index;
//    int count = sizeof( guPODCASTS_COLUMN_NAMES ) / sizeof( wxString );
//    for( index = 0; index < count; index++ )
//    {
//        CurColId = GetColumnId( index );
//        SetColumnLabel( index,
//            guPODCASTS_COLUMN_NAMES[ CurColId ]  + ( ( CurColId == m_Order ) ?
//                ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
//    }
//
//    ReloadItems();
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetSelectedSongs( guTrackArray * tracks ) const
{
//    wxArrayInt Selection = GetSelectedItems();
//    int Index;
//    int Count = Selection.Count();
//    for( Index = 0; Index < Count; Index++ )
//    {
//        guPodcastItem PodcastItem;
//        if( ( m_Db->GetPodcastItemId( Selection[ Index ], &PodcastItem ) != wxNOT_FOUND ) &&
//            ( PodcastItem.m_Status == guPODCAST_STATUS_READY ) &&
//            ( wxFileExists( PodcastItem.m_FileName ) ) )
//        {
//            guTrack * Track = new guTrack();
//            if( Track )
//            {
//                Track->m_Type = guTRACK_TYPE_PODCAST;
//                Track->m_SongId = PodcastItem.m_Id;
//                Track->m_FileName = PodcastItem.m_FileName;
//                Track->m_SongName = PodcastItem.m_Title;
//                Track->m_ArtistName = PodcastItem.m_Author;
//                Track->m_AlbumId = PodcastItem.m_ChId;
//                Track->m_AlbumName = PodcastItem.m_Channel;
//                Track->m_Length = PodcastItem.m_Length;
//                Track->m_PlayCount = PodcastItem.m_PlayCount;
//                Track->m_GenreName = wxT( "Podcasts" );
//                Track->m_Number = Index;
//                Track->m_Rating = -1;
//                Track->m_CoverId = 0;
//                Track->m_Year = 0; // Get year from item date
//                tracks->Add( Track );
//            }
//        }
//    }
//    return tracks->Count();
    return 0;
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetDragFiles( wxFileDataObject * files )
{
//    guTrackArray Songs;
//    int index;
//    int count = GetSelectedSongs( &Songs );
//    for( index = 0; index < count; index++ )
//    {
//       files->AddFile( Songs[ index ].m_FileName );
//    }
//    return count;
    return 0;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::SetPath( const wxString &path )
{
    m_CurDir = path;
    if( !m_CurDir.EndsWith( wxT( "/" ) ) )
        m_CurDir += wxT( "/" );
    ReloadItems();
}



// -------------------------------------------------------------------------------- //
// guFileBrowserFileCtrl
// -------------------------------------------------------------------------------- //
guFileBrowserFileCtrl::guFileBrowserFileCtrl( wxWindow * parent, guDbLibrary * db ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_FilesListBox = new guFilesListBox( this, db );
	MainSizer->Add( m_FilesListBox, 1, wxEXPAND | wxALL, 5 );

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

    m_VisiblePanels = Config->ReadNum( wxT( "RadVisiblePanels" ), guPANEL_FILEBROWSER_VISIBLE_DEFAULT, wxT( "Positions" ) );


    m_DirCtrl = new guFileBrowserDirCtrl( this, wxT( "/Datos/Music" ) );

    m_AuiManager.AddPane( m_DirCtrl,
            wxAuiPaneInfo().Name( wxT( "FileBrowserDirCtrl" ) ).Caption( _( "Directories" ) ).
            MinSize( 60, 28 ).Row( 0 ).Layer( 0 ).Position( 0 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Left() );

    m_FilesCtrl = new guFileBrowserFileCtrl( this, db );
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

}

// -------------------------------------------------------------------------------- //
guFileBrowser::~guFileBrowser()
{
	m_DirCtrl->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( guFileBrowser::OnDirItemChanged ), NULL, this );

    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnDirItemChanged( wxTreeEvent &event )
{
    guLogMessage( wxT( "The current selected directory is '%s'" ), m_DirCtrl->GetPath().c_str() );

    m_FilesCtrl->SetPath( m_DirCtrl->GetPath() );
}

// -------------------------------------------------------------------------------- //
