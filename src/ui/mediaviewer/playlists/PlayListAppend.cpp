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
#include "PlayListAppend.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guPlayListAppend::guPlayListAppend( wxWindow * parent, guDbLibrary * db, const wxArrayInt * tracks, guListItems * plitems ) :
    wxDialog( parent, wxID_ANY, _( "Save to Playlist" ), wxDefaultPosition, wxSize( 387,198 ), wxDEFAULT_DIALOG_STYLE )
{
    wxStaticText *              PosLabel;
    wxStaticText *              PlayListLabel;
    wxStaticText *              TracksLabel;
    wxStdDialogButtonSizer *    StdBtnSizer;
    wxButton *                  StdBtnSizerOK;
    wxButton *                  StdBtnSizerCancel;

    m_Db = db;
    m_Tracks = tracks;

    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    MainSizer->Add( 0, 20, 0, wxEXPAND, 5 );

    wxFlexGridSizer * FieldsSizer = new wxFlexGridSizer( 2, 0, 0 );
    FieldsSizer->AddGrowableCol( 1 );
    FieldsSizer->AddGrowableRow( 2 );
    FieldsSizer->SetFlexibleDirection( wxBOTH );
    FieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    PlayListLabel = new wxStaticText( this, wxID_ANY, _("Playlist:"), wxDefaultPosition, wxDefaultSize, 0 );
    PlayListLabel->Wrap( -1 );
    FieldsSizer->Add( PlayListLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    m_PlayListComboBox = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    m_PlayListItems = plitems;
    int index;
    int count = plitems->Count();
    for( index = 0; index < count; index++ )
    {
        m_PlayListComboBox->Append( plitems->Item( index ).m_Name );
    }
    m_PlayListComboBox->SetValue( _( "New playlist" ) );

    FieldsSizer->Add( m_PlayListComboBox, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    PosLabel = new wxStaticText( this, wxID_ANY, _( "Where:" ), wxDefaultPosition, wxDefaultSize, 0 );
    PosLabel->Wrap( -1 );
    FieldsSizer->Add( PosLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    wxString m_PosChoiceChoices[] = { _("Beginning"), _("End") };
    int m_PosChoiceNChoices = sizeof( m_PosChoiceChoices ) / sizeof( wxString );
    m_PosChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PosChoiceNChoices, m_PosChoiceChoices, 0 );
    m_PosChoice->SetSelection( 1 );
    FieldsSizer->Add( m_PosChoice, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    TracksLabel = new wxStaticText( this, wxID_ANY, _("Tracks:"), wxDefaultPosition, wxDefaultSize, 0 );
    TracksLabel->Wrap( -1 );
    FieldsSizer->Add( TracksLabel, 0, wxALL|wxALIGN_RIGHT, 5 );

    m_TracksStaticText = new wxStaticText( this, wxID_ANY, wxString::Format( wxT( "%lu" ), tracks->Count() ), wxDefaultPosition, wxDefaultSize, 0 );
    m_TracksStaticText->Wrap( -1 );
    FieldsSizer->Add( m_TracksStaticText, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    MainSizer->Add( FieldsSizer, 1, wxEXPAND, 5 );

    StdBtnSizer = new wxStdDialogButtonSizer();
    StdBtnSizerOK = new wxButton( this, wxID_OK );
    StdBtnSizer->AddButton( StdBtnSizerOK );
    StdBtnSizerCancel = new wxButton( this, wxID_CANCEL );
    StdBtnSizer->AddButton( StdBtnSizerCancel );
    StdBtnSizer->SetAffirmativeButton( StdBtnSizerOK );
    StdBtnSizer->SetCancelButton( StdBtnSizerCancel );
    StdBtnSizer->Realize();
    MainSizer->Add( StdBtnSizer, 0, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    this->SetSizer( MainSizer );
    this->Layout();

    StdBtnSizerOK->SetDefault();

    m_PlayListComboBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
guPlayListAppend::~guPlayListAppend()
{
}

// -------------------------------------------------------------------------------- //
int guPlayListAppend::GetSelectedPosition( void )
{
    return m_PosChoice->GetSelection();
}

// -------------------------------------------------------------------------------- //
int FindPlayListItem( guListItems * items, const wxString &playlistname )
{
    int Index;
    int Count = items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( items->Item( Index ).m_Name.Lower() == playlistname )
            return Index;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guPlayListAppend::GetSelectedPlayList( void )
{
    int Selection = m_PlayListComboBox->GetSelection();
    if( Selection == wxNOT_FOUND && m_PlayListComboBox->GetCount() != 0 )
    {
        Selection = FindPlayListItem( m_PlayListItems, m_PlayListComboBox->GetValue().Lower().Trim().Trim( false ) );
    }
    return Selection;
}

// -------------------------------------------------------------------------------- //
wxString guPlayListAppend::GetPlaylistName( void )
{
    return m_PlayListComboBox->GetValue();
}

}

// -------------------------------------------------------------------------------- //
