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
#include "ArListBox.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "OnlineLinks.h"
#include "MainApp.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guArListBox::guArListBox( wxWindow * parent, DbLibrary * db, wxString label ) :
             guListBox( parent, db, label )
{
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArListBox::OnSearchLinkClicked ) );
    Connect( ID_ARTIST_COMMANDS, ID_ARTIST_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArListBox::OnCommandClicked ) );
    ReloadItems();
};

// -------------------------------------------------------------------------------- //
guArListBox::~guArListBox()
{
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArListBox::OnSearchLinkClicked ) );
    Disconnect( ID_ARTIST_COMMANDS, ID_ARTIST_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArListBox::OnCommandClicked ) );
}

// -------------------------------------------------------------------------------- //
void guArListBox::GetItemsList( void )
{
    m_Items.Add( new guListItem( 0, _( "All" ) ) );
    m_Db->GetArtists( &m_Items );
}

// -------------------------------------------------------------------------------- //
int guArListBox::GetSelectedSongs( guTrackArray * songs ) const
{
    return m_Db->GetArtistsSongs( GetSelection(), songs );
}

// -------------------------------------------------------------------------------- //
void AddArtistCommands( wxMenu * Menu, int SelCount )
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
                    MenuItem = new wxMenuItem( Menu, ID_ARTIST_COMMANDS + index, Names[ index ], Commands[ index ] );
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
void guArListBox::GetContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_PLAY, _( "Play" ), _( "Play current selected artists" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_media_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_ENQUEUE, _( "Enqueue" ), _( "Add current selected artists to playlist" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected artists" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_tags ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_EDITTRACKS, _( "Edit Songs" ), _( "Edit the songs from the selected artists" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_gtk_edit ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_ARTIST_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
    MenuItem->SetBitmap( wxBitmap( guImage_edit_copy ) );
    Menu->Append( MenuItem );

    int SelCount = GetSelection().Count();
    if( SelCount == 1 )
    {
        Menu->AppendSeparator();

        AddOnlineLinksMenu( Menu );
    }

    AddArtistCommands( Menu, SelCount );
}

// -------------------------------------------------------------------------------- //
void guArListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    int Item = wxNOT_FOUND;
    Item = GetNextItem( Item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
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
void guArListBox::OnCommandClicked( wxCommandEvent &event )
{
    int index;
    int count;
    wxArrayInt Selection = GetSelection();
    if( Selection.Count() )
    {
        index = event.GetId();

        guConfig * Config = ( guConfig * ) Config->Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );
            wxASSERT( Commands.Count() > 0 );

            guLogMessage( wxT( "CommandId: %u" ), index );
            index -= ID_ARTIST_COMMANDS;
            wxString CurCmd = Commands[ index ];

            if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
            {
                wxArrayInt AlbumList;
                m_Db->GetArtistsAlbums( Selection, &AlbumList );
                wxArrayString AlbumPaths = m_Db->GetAlbumsPaths( AlbumList );
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
                wxArrayInt AlbumList;
                m_Db->GetArtistsAlbums( Selection, &AlbumList );
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
                if( m_Db->GetArtistsSongs( Selection, &Songs ) )
                {
                    count = Songs.Count();
                    for( index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + Songs[ index ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( wxT( "{tp}" ), SongList.Trim( false ) );
                }
            }

            guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
            wxExecute( CurCmd );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guArListBox::GetSearchText( int item )
{
    return GetItemText( item );
}

// -------------------------------------------------------------------------------- //
bool guArListBox::SelectArtistName( const wxString &ArtistName )
{
    long item = FindItem( 0, ArtistName, false );
    if( item != wxNOT_FOUND )
    {
        wxArrayInt Select;
        Select.Add( m_Items[ item ].m_Id );
        SetSelection( Select );
        EnsureVisible( item );
    }
}

// -------------------------------------------------------------------------------- //
