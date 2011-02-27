// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "TaListBox.h"

#include "Accelerators.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "Utils.h"
#include "LibPanel.h"

// -------------------------------------------------------------------------------- //
guTaListBox::guTaListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label ) :
     guAccelListBox( parent, db, label )
{
    m_LibPanel = libpanel;

    Connect( ID_LABEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::AddLabel ) );
    Connect( ID_LABEL_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::DelLabel ) );
    Connect( ID_LABEL_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::EditLabel ) );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guTaListBox::~guTaListBox()
{
    Disconnect( ID_LABEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::AddLabel ) );
    Disconnect( ID_LABEL_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::DelLabel ) );
    Disconnect( ID_LABEL_EDIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTaListBox::EditLabel ) );
}

// -------------------------------------------------------------------------------- //
void guTaListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_SONG_PLAY );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_ASNEXT );

    RealAccelCmds.Add( ID_LABEL_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_LABEL_PLAY );
    RealAccelCmds.Add( ID_LABEL_ENQUEUE );
    RealAccelCmds.Add( ID_LABEL_ENQUEUE_ASNEXT );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guTaListBox::GetItemsList( void )
{
    m_Db->GetLabels( m_Items );
}

// -------------------------------------------------------------------------------- //
int guTaListBox::GetSelectedSongs( guTrackArray * songs ) const
{
    int Count = m_Db->GetLabelsSongs( GetSelectedItems(), songs );
    m_LibPanel->NormalizeTracks( songs );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guTaListBox::CreateContextMenu( wxMenu * Menu ) const
{
    //    menu->Append(Menu_Dummy_First, _T("&First item\tCtrl-F1"));
    //    menu->AppendSeparator();
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ADD, _( "Add Label" ), _( "Create a new label" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu->Append( MenuItem );

    if( GetSelectedCount() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LABEL_EDIT, _( "Edit Label" ), _( "Change selected label" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LABEL_DELETE, _( "Delete label" ), _( "Delete selected labels" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_LABEL_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_PLAY ),
                            _( "Play current selected labels" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ENQUEUE,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE ),
                            _( "Add current selected labels to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ENQUEUE_ASNEXT,
                            wxString( _( "Enqueue Next" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_ASNEXT ),
                            _( "Add current selected labels to playlist as Next Tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_LABEL_SAVETOPLAYLIST,
                            wxString( _( "Save to PlayList" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                            _( "Save the selected tracks to PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    Menu->Append( MenuItem );

    if( m_LibPanel->GetContextMenuFlags() & guLIBRARY_CONTEXTMENU_COPY_TO )
    {
        Menu->AppendSeparator();
        m_LibPanel->CreateCopyToMenu( Menu, ID_LABEL_COPYTO );
    }

    m_LibPanel->CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guTaListBox::AddLabel( wxCommandEvent &event )
{
    //wxMessageBox( wxT( "AddLabel" ), wxT( "Information" ) );
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ), _( "Please enter the label name" ) );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        //wxMessageBox( EntryDialog->GetValue(), wxT( "Entered..." ) );
        m_Db->AddLabel( EntryDialog->GetValue() );
        ReloadItems();
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guTaListBox::DelLabel( wxCommandEvent &event )
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
                m_Db->DelLabel( Selection[ Index ] );
            }
            ReloadItems();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guTaListBox::EditLabel( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        // Get the Index of the First Selected Item
        if( Selection[ 0 ] )
        {
            unsigned long cookie;
            int Item = GetFirstSelected( cookie );
            wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Label Name: " ),
                                                    _( "Enter the new label name" ), ( * m_Items )[ Item ] .m_Name );
            if( EntryDialog->ShowModal() == wxID_OK &&
                ( * m_Items )[ Selection[ Item ] ].m_Name != EntryDialog->GetValue() )
            {
                m_Db->SetLabelName( Selection[ 0 ], ( * m_Items )[ Item ].m_Name, EntryDialog->GetValue() );
                ReloadItems();
            }
            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
int guTaListBox::GetDragFiles( wxFileDataObject * files )
{
    guTrackArray Songs;
    int index;
    int count = GetSelectedSongs( &Songs );
    m_LibPanel->NormalizeTracks( &Songs, true );
    for( index = 0; index < count; index++ )
    {
       wxString FileName = guFileDnDEncode( Songs[ index ].m_FileName );
       //FileName.Replace( wxT( "#" ), wxT( "%23" ) );
       //FileName.Replace( wxT( "%" ), wxT( "%25" ) );
       //guLogMessage( wxT( "Adding song '%s'" ), Songs[ index ].m_FileName.c_str() );
       files->AddFile( FileName );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
