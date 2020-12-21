// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#include "PlayerFilters.h"

#include "EventCommandIds.h"
#include "Config.h"
#include "Utils.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guPlayerFilters::guPlayerFilters( wxWindow * parent, guDbLibrary * db ) :
	wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;

	wxBoxSizer * FiltersMainSizer;
	FiltersMainSizer = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* FiltersFlexSizer;
    FiltersFlexSizer = new wxFlexGridSizer( 2, 0, 0 );
	FiltersFlexSizer->AddGrowableCol( 1 );
	FiltersFlexSizer->SetFlexibleDirection( wxBOTH );
	FiltersFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * FiltersAllowLabel = new wxStaticText( this, wxID_ANY, _( "Allow:" ), wxDefaultPosition, wxDefaultSize, 0 );
	FiltersAllowLabel->Wrap( -1 );
	FiltersFlexSizer->Add( FiltersAllowLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	wxArrayString m_FilterAllowChoiceChoices;
	m_FilterAllowChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FilterAllowChoiceChoices, 0 );
	m_FilterAllowChoice->SetSelection( 0 );
	FiltersFlexSizer->Add( m_FilterAllowChoice, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * FiltersDenyLabel = new wxStaticText( this, wxID_ANY, _( "Deny:" ), wxDefaultPosition, wxDefaultSize, 0 );
	FiltersDenyLabel->Wrap( -1 );
	FiltersFlexSizer->Add( FiltersDenyLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	wxArrayString m_FilterDenyChoiceChoices;
	m_FilterDenyChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FilterDenyChoiceChoices, 0 );
	m_FilterDenyChoice->SetSelection( 0 );
	FiltersFlexSizer->Add( m_FilterDenyChoice, 1, wxEXPAND|wxRIGHT, 5 );

	FiltersMainSizer->Add( FiltersFlexSizer, 1, wxEXPAND, 5 );

	SetSizer( FiltersMainSizer );
	Layout();
	FiltersMainSizer->Fit( this );

    UpdateFilters();
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_FilterDenyChoice->SetSelection( Config->ReadNum( CONFIG_KEY_PLAYBACK_PLAYLIST_DENY_FILTER, 0, CONFIG_PATH_PLAYBACK ) );
    m_FilterAllowChoice->SetSelection( Config->ReadNum( CONFIG_KEY_PLAYBACK_PLAYLIST_ALLOW_FILTER, 0, CONFIG_PATH_PLAYBACK ) );

}

// -------------------------------------------------------------------------------- //
guPlayerFilters::~guPlayerFilters()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( CONFIG_KEY_PLAYBACK_PLAYLIST_ALLOW_FILTER, m_FilterAllowChoice->GetSelection(), CONFIG_PATH_PLAYBACK );
    Config->WriteNum( CONFIG_KEY_PLAYBACK_PLAYLIST_DENY_FILTER, m_FilterDenyChoice->GetSelection(), CONFIG_PATH_PLAYBACK );
}

// -------------------------------------------------------------------------------- //
int GetListItemsIdIndex( const guListItems &listitems, const int id )
{
    int Index;
    int Count = listitems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( listitems[ Index ].m_Id == id )
            return Index;
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guPlayerFilters::UpdateFilters( void )
{
    int CurAllowFilter = wxNOT_FOUND;
    int CurDenyFilter = wxNOT_FOUND;
    if( m_FilterPlayLists.Count() )
    {
        CurAllowFilter = m_FilterPlayLists[ m_FilterAllowChoice->GetSelection() ].m_Id;
        CurDenyFilter = m_FilterPlayLists[ m_FilterDenyChoice->GetSelection() ].m_Id;
    }

    m_FilterPlayLists.Empty();

    m_FilterPlayLists.Add( new guListItem( wxNOT_FOUND, _( "All" ) ) );
    m_Db->GetPlayLists( &m_FilterPlayLists, guPLAYLIST_TYPE_STATIC );
    m_Db->GetPlayLists( &m_FilterPlayLists, guPLAYLIST_TYPE_DYNAMIC );

    wxArrayString ChoiceItems;
    int Index;
    int Count = m_FilterPlayLists.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        ChoiceItems.Add( m_FilterPlayLists[ Index ].m_Name );
    }

    m_FilterAllowChoice->Clear();
    m_FilterAllowChoice->Append( ChoiceItems );
    m_FilterAllowChoice->SetSelection( GetListItemsIdIndex( m_FilterPlayLists, CurAllowFilter ) );

    ChoiceItems[ 0 ] = _( "None" );
    m_FilterDenyChoice->Clear();
    m_FilterDenyChoice->Append( ChoiceItems );
    m_FilterDenyChoice->SetSelection( GetListItemsIdIndex( m_FilterPlayLists, CurDenyFilter ) );
}

// -------------------------------------------------------------------------------- //
void guPlayerFilters::EnableFilters( const bool enable )
{
    m_FilterAllowChoice->Enable( enable );
    m_FilterDenyChoice->Enable( enable );
}

// -------------------------------------------------------------------------------- //
void guPlayerFilters::SetAllowFilterId( const int id )
{
    m_FilterAllowChoice->SetSelection( GetListItemsIdIndex( m_FilterPlayLists, id ) );
}

// -------------------------------------------------------------------------------- //
void guPlayerFilters::SetDenyFilterId( const int id )
{
    m_FilterDenyChoice->SetSelection( GetListItemsIdIndex( m_FilterPlayLists, id ) );
}

// -------------------------------------------------------------------------------- //
int guPlayerFilters::GetAllowSelection( void )
{
    return m_FilterAllowChoice->GetSelection();
}

// -------------------------------------------------------------------------------- //
int guPlayerFilters::GetDenySelection( void )
{
    return m_FilterDenyChoice->GetSelection();
}

// -------------------------------------------------------------------------------- //
int guPlayerFilters::GetAllowFilterId( void )
{
    int Index = m_FilterAllowChoice->GetSelection();
    if( Index != wxNOT_FOUND )
    {
        return m_FilterPlayLists[ Index ].m_Id;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guPlayerFilters::GetDenyFilterId( void )
{
    int Index = m_FilterDenyChoice->GetSelection();
    if( Index != wxNOT_FOUND )
    {
        return m_FilterPlayLists[ Index ].m_Id;
    }
    return wxNOT_FOUND;
}

}

// -------------------------------------------------------------------------------- //
