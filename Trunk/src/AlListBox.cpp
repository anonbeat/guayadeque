// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
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
#include "AlListBox.h"

#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "Utils.h"

#include <wx/renderer.h>

#define ALLISTBOX_ITEM_SIZE  40

// -------------------------------------------------------------------------------- //
// guAlListBox
// -------------------------------------------------------------------------------- //
guAlListBox::guAlListBox( wxWindow * parent, guDbLibrary * db, const wxString &label ) :
    guListView( parent, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG | guLISTVIEW_HIDE_HEADER )
{
    m_Db = db;
    m_Items = new guAlbumItems();

    guListViewColumn * Column = new guListViewColumn( label, 0 );
    InsertColumn( Column );

    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlListBox::OnSearchLinkClicked ) );
    Connect( ID_ALBUM_COMMANDS, ID_ALBUM_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlListBox::OnCommandClicked ) );
    Connect( ID_ALBUM_ORDER_NAME, ID_ALBUM_ORDER_ARTIST_YEAR_REVERSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlListBox::OnOrderSelected ) );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guAlListBox::~guAlListBox()
{
    if( m_Items )
        delete m_Items;

    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlListBox::OnSearchLinkClicked ) );
    Disconnect( ID_ALBUM_COMMANDS, ID_ALBUM_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlListBox::OnCommandClicked ) );
}

// -------------------------------------------------------------------------------- //
void guAlListBox::GetItemsList( void )
{
    m_Db->GetAlbums( ( guAlbumItems * ) m_Items );
}

// -------------------------------------------------------------------------------- //
bool guAlListBox::SelectAlbumName( const wxString &AlbumName )
{
    long item = FindItem( 0, AlbumName, false );
    if( item != wxNOT_FOUND )
    {
        wxArrayInt * Albums = new wxArrayInt();
        Albums->Add( ( * m_Items )[ item ].m_Id );

        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUM_SETSELECTION );
        event.SetClientData( ( void * ) Albums );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guAlListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    m_Attr.m_Font->SetPointSize( 10 );

    guAlbumItem * Item = &( * ( guAlbumItems * ) m_Items )[ row ];

    dc.SetFont( * m_Attr.m_Font );
    dc.SetBackgroundMode( wxTRANSPARENT );

    dc.SetTextForeground( IsSelected( row ) ? m_Attr.m_SelFgColor : m_Attr.m_TextFgColor );

    dc.DrawText( Item->m_Name, rect.x + 45, rect.y + 4 );

    if( Item->m_Year > 0 )
    {
        m_Attr.m_Font->SetPointSize( 7 );
        dc.SetFont( * m_Attr.m_Font );
        dc.DrawText( wxString::Format( wxT( "%4u" ), Item->m_Year ), rect.x + 45, rect.y + 22 );
    }

    if( Item->m_Thumb && Item->m_Thumb->IsOk() )
    {
        dc.DrawBitmap( * Item->m_Thumb, rect.x + 1, rect.y + 1, false );
    }
    else if( Item->m_Thumb )
    {
        guLogError( wxT( "Thumb image corrupt or not correctly loaded" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnOrderSelected( wxCommandEvent &event )
{
    m_Db->SetAlbumsOrder( event.GetId() - ID_ALBUM_ORDER_NAME );
    ReloadItems( false );
}

// -------------------------------------------------------------------------------- //
wxCoord guAlListBox::OnMeasureItem( size_t n ) const
{
    // Code taken from the generic/listctrl.cpp file
    guAlListBox * self = wxConstCast( this, guAlListBox );

    self->SetItemHeight( ALLISTBOX_ITEM_SIZE );
    return wxCoord( ALLISTBOX_ITEM_SIZE );
}

// -------------------------------------------------------------------------------- //
int guAlListBox::GetSelectedSongs( guTrackArray * tracks ) const
{
    return m_Db->GetAlbumsSongs( GetSelectedItems(), tracks );
}

// -------------------------------------------------------------------------------- //
void AddAlbumCommands( wxMenu * Menu, int SelCount )
{
    wxMenu * SubMenu;
    int index;
    int count;
    wxMenuItem * MenuItem;
    if( Menu )
    {
        SubMenu = new wxMenu();
        wxASSERT( SubMenu );

        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
        wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "Commands" ) );
        if( ( count = Commands.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                if( ( Commands[ index ].Find( wxT( "{bc}" ) ) == wxNOT_FOUND ) || ( SelCount == 1 ) )
                {
                    MenuItem = new wxMenuItem( Menu, ID_ALBUM_COMMANDS + index, Names[ index ], Commands[ index ] );
                    SubMenu->Append( MenuItem );
                }
            }
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, -1, _( "No commands defined" ), _( "Add commands in preferences" ) );
            SubMenu->Append( MenuItem );
        }
        Menu->AppendSubMenu( SubMenu, _( "Commands" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_ALBUM_PLAY, _( "Play" ), _( "Play current selected albums" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_ALBUM_ENQUEUE, _( "Enqueue" ), _( "Add current selected albums to the Playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    int SelCount = GetSelectedItems().Count();
    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected albums" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_EDITTRACKS, _( "Edit Album songs" ), _( "Edit the selected albums songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
        Menu->Append( MenuItem );

        if( SelCount == 1 )
        {
            Menu->AppendSeparator();

            MenuItem = new wxMenuItem( Menu, ID_ALBUM_MANUALCOVER, _( "Download Album cover" ), _( "Download cover for the current selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu->Append( MenuItem );

            MenuItem = new wxMenuItem( Menu, ID_ALBUM_SELECT_COVER, _( "Select cover location" ), _( "Select the cover image file from disk" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu->Append( MenuItem );

            MenuItem = new wxMenuItem( Menu, ID_ALBUM_COVER_DELETE, _( "Delete Album cover" ), _( "Delete the cover for the selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
            Menu->Append( MenuItem );
        }
    }

    Menu->AppendSeparator();

    wxMenu * SubMenu = new wxMenu();

//    MenuItem = new wxMenuItem( Menu, ID_ALBUM_ORDER_NAME, _( "Name" ), _( "Albums are sorted by name" ) );
//    SubMenu->Append( MenuItem );
//    MenuItem = new wxMenuItem( Menu, ID_ALBUM_ORDER_YEAR, _( "Year" ), _( "Albums are sorted by year" ) );
//    SubMenu->Append( MenuItem );
//    MenuItem = new wxMenuItem( Menu, ID_ALBUM_ORDER_YEAR_REVERSE, _( "Year descending" ), _( "Albums are sorted by year descending" ) );
//    SubMenu->Append( MenuItem );
//    MenuItem = new wxMenuItem( Menu, ID_ALBUM_ORDER_ARTIST_NAME, _( "Artist, Name" ), _( "Albums are sorted by artist and album name" ) );
//    SubMenu->Append( MenuItem );
//    MenuItem = new wxMenuItem( Menu, ID_ALBUM_ORDER_ARTIST_YEAR, _( "Artist, Year" ), _( "Albums are sorted by artist and year" ) );
//    SubMenu->Append( MenuItem );
//    MenuItem = new wxMenuItem( Menu, ID_ALBUM_ORDER_ARTIST_YEAR_REVERSE, _( "Artist, Year descending" ), _( "Albums are sorted by artist and year descending" ) );
//    SubMenu->Append( MenuItem );

    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_NAME, _( "Name" ), _( "Albums are sorted by name" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_YEAR, _( "Year" ), _( "Albums are sorted by year" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_YEAR_REVERSE, _( "Year descending" ), _( "Albums are sorted by year descending" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_ARTIST_NAME, _( "Artist, Name" ), _( "Albums are sorted by artist and album name" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_ARTIST_YEAR, _( "Artist, Year" ), _( "Albums are sorted by artist and year" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_ARTIST_YEAR_REVERSE, _( "Artist, Year descending" ), _( "Albums are sorted by artist and year descending" ) );

    MenuItem = SubMenu->FindItemByPosition( m_Db->GetAlbumsOrder() );
    MenuItem->Check( true );

    Menu->Append( wxID_ANY, _( "Ordered by" ), SubMenu, _( "Sets the albums order" ) );

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
        if( SelCount == 1 )
        {
            AddOnlineLinksMenu( Menu );
        }

        AddAlbumCommands( Menu, SelCount );
    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    int Item;
    unsigned long cookie;
    Item = GetFirstSelected( cookie );
    if( Item != wxNOT_FOUND )
    {
        int index = event.GetId();

        guConfig * Config = ( guConfig * ) Config->Get();
        if( Config )
        {
            wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );
            wxASSERT( Links.Count() > 0 );

            index -= ID_LASTFM_SEARCH_LINK;
            wxString SearchLink = Links[ index ];
            wxString Lang = Config->ReadStr( wxT( "Language" ), wxT( "en" ), wxT( "LastFM" ) );
            if( Lang.IsEmpty() )
            {
                Lang = ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().Mid( 0, 2 );
                //guLogMessage( wxT( "Locale: %s" ), ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().c_str() );
            }
            SearchLink.Replace( wxT( "{lang}" ), Lang );
            SearchLink.Replace( wxT( "{text}" ), guURLEncode( GetSearchText( Item ) ) );
            guWebExecute( SearchLink );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnCommandClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        index = event.GetId();

        guConfig * Config = ( guConfig * ) Config->Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
            wxASSERT( Commands.Count() > 0 );

            index -= ID_ALBUM_COMMANDS;
            wxString CurCmd = Commands[ index ];
            if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
            {
                wxArrayString AlbumPaths = m_Db->GetAlbumsPaths( Selection );
                wxString Paths = wxEmptyString;
                count = AlbumPaths.Count();
                for( index = 0; index < count; index++ )
                {
                    Paths += wxT( " \"" ) + AlbumPaths[ index ] + wxT( "\"" );
                }
                CurCmd.Replace( wxT( "{bp}" ), Paths.Trim( false ) );
            }

            if( CurCmd.Find( wxT( "{bc}" ) ) != wxNOT_FOUND )
            {
                int CoverId = m_Db->GetAlbumCoverId( Selection[ 0 ] );
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
                if( m_Db->GetAlbumsSongs( Selection, &Songs ) )
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
wxString guAlListBox::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Db->GetArtistName( ( * m_Items )[ item ].m_ArtistId ).c_str(),
        ( * m_Items )[ item ].m_Name.c_str() );
}


// -------------------------------------------------------------------------------- //
wxString inline guAlListBox::GetItemName( const int row ) const
{
    return ( * m_Items )[ row ].m_Name;
}

// -------------------------------------------------------------------------------- //
int inline guAlListBox::GetItemId( const int row ) const
{
    return ( * m_Items )[ row ].m_Id;
}

// -------------------------------------------------------------------------------- //
void guAlListBox::ReloadItems( bool reset )
{
    wxArrayInt Selection;
    int FirstVisible = GetFirstVisibleLine();

    if( reset )
        SetSelection( -1 );
    else
        Selection = GetSelectedItems( false );

    m_Items->Empty();

    GetItemsList();
    m_Items->Insert( new guAlbumItem( 0, wxString::Format( wxT( "%s (%u)" ), _( "All" ), m_Items->Count() ) ), 0 );
    SetItemCount( m_Items->Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToLine( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void  guAlListBox::SetSelectedItems( const wxArrayInt &selection )
{
    guListView::SetSelectedItems( selection );

//    wxCommandEvent event( wxEVT_COMMAND_LISTBOX_SELECTED, GetId() );
//    event.SetEventObject( this );
//    event.SetInt( -1 );
//    (void) GetEventHandler()->ProcessEvent( event );
}

// -------------------------------------------------------------------------------- //
int guAlListBox::GetDragFiles( wxFileDataObject * files )
{
    guTrackArray Songs;
    int index;
    int count = GetSelectedSongs( &Songs );
    for( index = 0; index < count; index++ )
    {
       wxString FileName = Songs[ index ].m_FileName;
       FileName.Replace( wxT( "#" ), wxT( "%23" ) );
       //FileName.Replace( wxT( "%" ), wxT( "%25" ) );
       //guLogMessage( wxT( "Adding song '%s'" ), Songs[ index ].m_FileName.c_str() );
       files->AddFile( FileName );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
int guAlListBox::FindAlbum( const int albumid )
{
    int Index;
    int Count = m_Items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_Items->Item( Index ).m_Id == albumid )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
