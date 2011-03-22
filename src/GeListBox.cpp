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
#include "GeListBox.h"

#include "Accelerators.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "Utils.h"
#include "LibPanel.h"

// -------------------------------------------------------------------------------- //
guGeListBox::guGeListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label ) :
     guAccelListBox( parent, db, label )
{
    m_LibPanel = libpanel;

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guGeListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_SONG_PLAY );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE );
    AliasAccelCmds.Add( ID_SONG_ENQUEUE_ASNEXT );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_GENRE_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_GENRE_PLAY );
    RealAccelCmds.Add( ID_GENRE_ENQUEUE );
    RealAccelCmds.Add( ID_GENRE_ENQUEUE_ASNEXT );
    RealAccelCmds.Add( ID_LIBRARY_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guGeListBox::GetItemsList( void )
{
    m_Db->GetGenres( m_Items );
}

// -------------------------------------------------------------------------------- //
int guGeListBox::GetSelectedSongs( guTrackArray * songs ) const
{
    int Count = m_Db->GetGenresSongs( GetSelectedItems(), songs );
    m_LibPanel->NormalizeTracks( songs );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guGeListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedCount();

    MenuItem = new wxMenuItem( Menu, ID_GENRE_PLAY,
                        wxString( _( "Play" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_PLAY ),
                        _( "Play current selected genres" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_GENRE_ENQUEUE,
                        wxString( _( "Enqueue" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE ),
                        _( "Add current selected genres to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_GENRE_ENQUEUE_ASNEXT,
                        wxString( _( "Enqueue Next" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_ASNEXT ),
                        _( "Add current selected genres to playlist as Next Tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_GENRE_SAVETOPLAYLIST,
                        wxString( _( "Save to PlayList" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                        _( "Save the selected tracks to PlayList" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
    Menu->Append( MenuItem );

    if( SelCount && ( m_LibPanel->GetContextMenuFlags() & guLIBRARY_CONTEXTMENU_COPY_TO ) )
    {
        Menu->AppendSeparator();
        m_LibPanel->CreateCopyToMenu( Menu, ID_GENRE_COPYTO );
    }

    m_LibPanel->CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
int guGeListBox::GetDragFiles( wxFileDataObject * files )
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
