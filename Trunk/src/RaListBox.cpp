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
#include "RaListBox.h"

#include "Accelerators.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "OnlineLinks.h"
#include "RatingCtrl.h"
#include "MainApp.h"
#include "Utils.h"
#include "LibPanel.h"

// -------------------------------------------------------------------------------- //
guRaListBox::guRaListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label ) :
             guAccelListBox( parent, db, label )
{
    m_LibPanel = libpanel;

    m_NormalStar   = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_normal_tiny + GURATING_STYLE_MID ) ) );
    m_SelectStar = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_highlight_tiny + GURATING_STYLE_MID ) ) );

    CreateAcceleratorTable();

    ReloadItems();
};

// -------------------------------------------------------------------------------- //
guRaListBox::~guRaListBox()
{
    if( m_NormalStar )
        delete m_NormalStar;
    if( m_SelectStar )
        delete m_SelectStar;
}

// -------------------------------------------------------------------------------- //
void guRaListBox::CreateAcceleratorTable( void )
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
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_RATING_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_RATING_EDITTRACKS );
    RealAccelCmds.Add( ID_RATING_PLAY );
    RealAccelCmds.Add( ID_RATING_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_RATING_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_RATING_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_RATING_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_LIBRARY_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }

}

// -------------------------------------------------------------------------------- //
void guRaListBox::GetItemsList( void )
{
    m_Db->GetRatings( m_Items );
}

// -------------------------------------------------------------------------------- //
int guRaListBox::GetSelectedSongs( guTrackArray * songs ) const
{
    wxArrayInt Items = GetSelectedItems();
    int Index;
    int Count = Items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( Items[ Index ] )
            Items[ Index ]--;
    }
    Count = m_Db->GetRatingsSongs( Items, songs );
    m_LibPanel->NormalizeTracks( songs );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guRaListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( row == 0 )
    {
        guListBox::DrawItem( dc, rect, row, col );
    }
    else
    {
        int Rating = GetItemId( row ) - 1;

        dc.SetBackgroundMode( wxTRANSPARENT );
        int x;
        int w = ( ( GURATING_STYLE_MID * 2 ) + GURATING_IMAGE_SIZE );

        for( x = 0; x < 5; x++ )
        {
           dc.DrawBitmap( ( x >= Rating ) ? * m_NormalStar : * m_SelectStar,
                          rect.x + 3 + ( w * x ), rect.y + 3, true );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guRaListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    int SelCount = GetSelectedCount();
    int ContextMenuFlags = m_LibPanel->GetContextMenuFlags();

    MenuItem = new wxMenuItem( Menu, ID_RATING_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_PLAY ),
                            _( "Play the selected tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( Menu, ID_RATING_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ALL ),
                            _( "Add current selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_RATING_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_RATING_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_RATING_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    Menu->Append( wxID_ANY, _( "Enqueue after" ), EnqueueMenu );

    if( SelCount )
    {
        if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_EDIT_TRACKS )
        {
            Menu->AppendSeparator();

            MenuItem = new wxMenuItem( Menu, ID_RATING_EDITTRACKS,
                                wxString( _( "Edit Songs" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                                _( "Edit the selected tracks" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu->Append( MenuItem );
        }

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_RATING_SAVETOPLAYLIST,
                                wxString( _( "Save to PlayList" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                                _( "Save the selected tracks to PlayList" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_COPY_TO )
        {
            Menu->AppendSeparator();
            m_LibPanel->CreateCopyToMenu( Menu, ID_RATING_COPYTO );
        }
    }
    m_LibPanel->CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
wxString guRaListBox::GetSearchText( int item ) const
{
    return GetItemName( item );
}

// -------------------------------------------------------------------------------- //
int guRaListBox::GetDragFiles( wxFileDataObject * files )
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
