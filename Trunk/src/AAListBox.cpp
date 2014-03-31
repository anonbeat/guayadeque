// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2013 J.Rios
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
#include "AAListBox.h"

#include "Accelerators.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "LibPanel.h"
#include "OnlineLinks.h"
#include "MainApp.h"
#include "MediaViewer.h"
#include "Settings.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guAAListBox::guAAListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label ) :
             guAccelListBox( parent, db, label )
{
    m_LibPanel = libpanel;

    Connect( ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAAListBox::OnSearchLinkClicked ) );
    Connect( ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAAListBox::OnCommandClicked ) );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guAAListBox::~guAAListBox()
{
    Disconnect( ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAAListBox::OnSearchLinkClicked ) );
    Disconnect( ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAAListBox::OnCommandClicked ) );
}

// -------------------------------------------------------------------------------- //
void guAAListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_ALBUMARTIST_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_ALBUMARTIST_EDITTRACKS );
    RealAccelCmds.Add( ID_ALBUMARTIST_PLAY );
    RealAccelCmds.Add( ID_ALBUMARTIST_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_ALBUMARTIST_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_ALBUMARTIST_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_ALBUMARTIST_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_LIBRARY_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guAAListBox::GetItemsList( void )
{
    m_Db->GetAlbumArtists( m_Items );
}

// -------------------------------------------------------------------------------- //
int guAAListBox::GetSelectedSongs( guTrackArray * songs, const bool isdrag ) const
{
    int Count = m_Db->GetAlbumArtistsSongs( GetSelectedItems(), songs );
    m_LibPanel->NormalizeTracks( songs, isdrag );
    return Count;
}

// -------------------------------------------------------------------------------- //
void AddAlbumArtistCommands( wxMenu * Menu, int SelCount )
{
    wxMenu * SubMenu;
    int index;
    int count;
    wxMenuItem * MenuItem;
    if( Menu )
    {
        SubMenu = new wxMenu();

        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );
        wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "commands/names" ) );
        if( ( count = Commands.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                if( ( Commands[ index ].Find( wxT( "{bc}" ) ) == wxNOT_FOUND ) || ( SelCount == 1 ) )
                {
                    MenuItem = new wxMenuItem( Menu, ID_COMMANDS_BASE + index, Names[ index ], Commands[ index ] );
                    SubMenu->Append( MenuItem );
                }
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
void guAAListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    int SelCount = GetSelectedCount();
    int ContextMenuFlags = m_LibPanel->GetContextMenuFlags();

    MenuItem = new wxMenuItem( Menu, ID_ALBUMARTIST_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                            _( "Play current selected album artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( Menu, ID_ALBUMARTIST_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                            _( "Add current selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMARTIST_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMARTIST_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMARTIST_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

    if( SelCount )
    {
        if( ContextMenuFlags & guCONTEXTMENU_EDIT_TRACKS )
        {
            Menu->AppendSeparator();

            MenuItem = new wxMenuItem( Menu, ID_ALBUMARTIST_EDITTRACKS,
                                wxString( _( "Edit Songs" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                                _( "Edit the selected tracks" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu->Append( MenuItem );
        }

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_ALBUMARTIST_SAVETOPLAYLIST,
                                wxString( _( "Save to Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                                _( "Save the selected tracks to Playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        if( ( ContextMenuFlags & guCONTEXTMENU_COPY_TO ) ||
            ( ContextMenuFlags & guCONTEXTMENU_LINKS ) ||
            ( ContextMenuFlags & guCONTEXTMENU_COMMANDS ) )
        {
            Menu->AppendSeparator();

            if( ContextMenuFlags & guCONTEXTMENU_COPY_TO )
            {
                m_LibPanel->CreateCopyToMenu( Menu );
            }

            if( SelCount == 1 && ( ContextMenuFlags & guCONTEXTMENU_LINKS ) )
            {
                AddOnlineLinksMenu( Menu );
            }

            if( ContextMenuFlags & guCONTEXTMENU_COMMANDS )
                AddAlbumArtistCommands( Menu, SelCount );
        }
    }

    m_LibPanel->CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
wxString guAAListBox::GetSearchText( int item ) const
{
    return GetItemName( item );
}

// -------------------------------------------------------------------------------- //
int guAAListBox::FindAlbumArtist( const wxString &albumartist )
{
    int Index;
    int Count = m_Items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_Items->Item( Index ).m_Name == albumartist )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guAAListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    unsigned long cookie;
    int Item = GetFirstSelected( cookie );
    if( Item != wxNOT_FOUND )
    {
        ExecuteOnlineLink( event.GetId(), GetSearchText( Item ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAAListBox::OnCommandClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        index = event.GetId();

        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );

            //guLogMessage( wxT( "CommandId: %u" ), index );
            index -= ID_COMMANDS_BASE;
            wxString CurCmd = Commands[ index ];

            if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
            {
                wxArrayInt AlbumList;
                m_Db->GetAlbumArtistsAlbums( Selection, &AlbumList );
                wxArrayString AlbumPaths = m_Db->GetAlbumsPaths( AlbumList );
                count = AlbumPaths.Count();
                wxString Paths = wxEmptyString;
                for( index = 0; index < count; index++ )
                {
                    AlbumPaths[ index ].Replace( wxT( " " ), wxT( "\\ " ) );
                    Paths += wxT( " " ) + AlbumPaths[ index ];
                }
                CurCmd.Replace( wxT( "{bp}" ), Paths.Trim( false ) );
            }

            if( CurCmd.Find( wxT( "{bc}" ) ) != wxNOT_FOUND )
            {
                wxArrayInt AlbumList;
                m_Db->GetAlbumArtistsAlbums( Selection, &AlbumList );
                int CoverId = m_Db->GetAlbumCoverId( AlbumList[ 0 ] );
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                {
                    CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
                }
                CurCmd.Replace( wxT( "{bc}" ), CoverPath );
            }

            if( CurCmd.Find( wxT( "{tp}" ) ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                wxString SongList = wxEmptyString;
                if( m_Db->GetAlbumArtistsSongs( Selection, &Songs ) )
                {
                    count = Songs.Count();
                    for( index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + Songs[ index ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( wxT( "{tp}" ), SongList.Trim( false ) );
                }
            }

            //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
            guExecute( CurCmd );
        }
    }
}

// -------------------------------------------------------------------------------- //
