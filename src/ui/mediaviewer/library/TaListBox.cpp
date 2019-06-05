// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#include "TaListBox.h"

#include "Accelerators.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "Images.h"
#include "Utils.h"
#include "LibPanel.h"
#include "MediaViewer.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guTaListBox::guTaListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label ) :
     guAccelListBox( parent, db, label )
{
    m_LibPanel = libpanel;

    Bind( wxEVT_MENU, &guTaListBox::AddLabel, this, ID_LABEL_ADD );
    Bind( wxEVT_MENU, &guTaListBox::DelLabel, this, ID_LABEL_DELETE );
    Bind( wxEVT_MENU, &guTaListBox::EditLabel, this, ID_LABEL_EDIT );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guTaListBox::~guTaListBox()
{
    Unbind( wxEVT_MENU, &guTaListBox::AddLabel, this, ID_LABEL_ADD );
    Unbind( wxEVT_MENU, &guTaListBox::DelLabel, this, ID_LABEL_DELETE );
    Unbind( wxEVT_MENU, &guTaListBox::EditLabel, this, ID_LABEL_EDIT );
}

// -------------------------------------------------------------------------------- //
void guTaListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_LABEL_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_LABEL_PLAY );
    RealAccelCmds.Add( ID_LABEL_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_LABEL_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_LABEL_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_LABEL_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_LIBRARY_SEARCH );

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
int guTaListBox::GetSelectedSongs( guTrackArray * songs, const bool isdrag ) const
{
    int Count = m_Db->GetLabelsSongs( GetSelectedItems(), songs );
    m_LibPanel->NormalizeTracks( songs, isdrag );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guTaListBox::CreateContextMenu( wxMenu * Menu ) const
{
    int SelCount = GetSelectedCount();

    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ADD, _( "Create" ), _( "Create a new label" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu->Append( MenuItem );


    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_LABEL_EDIT, _( "Rename" ), _( "Change selected label" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LABEL_DELETE, _( "Delete" ), _( "Delete selected labels" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_LABEL_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                            _( "Play current selected labels" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( Menu, ID_LABEL_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                            _( "Add current selected labels to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_LABEL_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_LABEL_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_LABEL_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_LABEL_SAVETOPLAYLIST,
                                wxString( _( "Save to Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                                _( "Save the selected tracks to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        if( m_LibPanel->GetContextMenuFlags() & guCONTEXTMENU_COPY_TO )
        {
            Menu->AppendSeparator();
            m_LibPanel->CreateCopyToMenu( Menu );
        }
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
                          wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT, this ) == wxYES )
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
                ( * m_Items )[ Item ].m_Name != EntryDialog->GetValue() )
            {
                m_Db->SetLabelName( Selection[ 0 ], ( * m_Items )[ Item ].m_Name, EntryDialog->GetValue() );
                ReloadItems();
            }
            EntryDialog->Destroy();
        }
    }
}

}

// -------------------------------------------------------------------------------- //
