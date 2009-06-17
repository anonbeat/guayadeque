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
    ReloadItems();
};

// -------------------------------------------------------------------------------- //
guArListBox::~guArListBox()
{
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArListBox::OnSearchLinkClicked ) );
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

    if( GetSelection().Count() == 1 )
    {
        Menu->AppendSeparator();

        AddOnlineLinksMenu( Menu );
    }
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
