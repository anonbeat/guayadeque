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
#include "AlbumBrowser.h"

#include "Images.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY( guAlbumBrowserItemArray );

// -------------------------------------------------------------------------------- //
// guAlbumBrowserItemPanel
// -------------------------------------------------------------------------------- //
guAlbumBrowserItemPanel::guAlbumBrowserItemPanel( wxWindow * parent, const int index,
    guAlbumBrowserItem * albumitem ) : wxPanel( parent, wxID_ANY )
{
    m_AlbumBrowserItem = albumitem;

    // GUI
	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 150,150 ), 0 );
	MainSizer->Add( m_Bitmap, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_ArtistLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_ArtistLabel->Wrap( 140 );
	MainSizer->Add( m_ArtistLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxLEFT, 5 );

	m_AlbumLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_AlbumLabel->Wrap( 140 );
	MainSizer->Add( m_AlbumLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxLEFT, 5 );

	m_TracksLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_TracksLabel->Wrap( 140 );
	MainSizer->Add( m_TracksLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	SetSizer( MainSizer );
	Layout();
	MainSizer->Fit( this );
}

// -------------------------------------------------------------------------------- //
guAlbumBrowserItemPanel::~guAlbumBrowserItemPanel()
{
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::SetAlbumItem( const int index, guAlbumBrowserItem * albumitem )
{
    m_Index = index;
    m_AlbumBrowserItem = albumitem;
    if( m_AlbumBrowserItem )
    {
        m_Bitmap->SetBitmap( * m_AlbumBrowserItem->m_Cover );
        m_ArtistLabel->SetLabel( m_AlbumBrowserItem->m_ArtistName );
        wxString AlbumLabel = m_AlbumBrowserItem->m_AlbumName;
        if( m_AlbumBrowserItem->m_Year )
        {
            AlbumLabel += wxString::Format( wxT( " (%u)" ), m_AlbumBrowserItem->m_Year );
        }
        m_AlbumLabel->SetLabel( AlbumLabel );
        m_TracksLabel->SetLabel( wxString::Format( wxT( "%u " ), m_AlbumBrowserItem->m_TrackCount ) + _( "Tracks" ) );
    }
    else
    {
        m_Bitmap->SetBitmap( wxNullBitmap );
        m_ArtistLabel->SetLabel( wxEmptyString );
        m_AlbumLabel->SetLabel( wxEmptyString );
        m_TracksLabel->SetLabel( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
// guAlbumBrowser
// -------------------------------------------------------------------------------- //
guAlbumBrowser::guAlbumBrowser( wxWindow * parent, guDbLibrary * db ) :
    wxPanel( parent, wxID_ANY )
{
    m_Db = db;
    m_StartItem = 0;

    // GUI
	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_AlbumsSizer = new wxGridSizer( 1, 1, 0, 0 );

	MainSizer->Add( m_AlbumsSizer, 1, wxEXPAND, 5 );

	wxBoxSizer* NavigatorSizer;
	NavigatorSizer = new wxBoxSizer( wxVERTICAL );

	m_NavLabel = new wxStaticText( this, wxID_ANY, wxT("A"), wxDefaultPosition, wxDefaultSize, 0 );
	m_NavLabel->Wrap( -1 );
	NavigatorSizer->Add( m_NavLabel, 0, wxRIGHT|wxLEFT|wxEXPAND, 5 );

	m_NavSlider = new wxSlider( this, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	NavigatorSizer->Add( m_NavSlider, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	MainSizer->Add( NavigatorSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();
}

// -------------------------------------------------------------------------------- //
guAlbumBrowser::~guAlbumBrowser()
{
}

// -------------------------------------------------------------------------------- //
