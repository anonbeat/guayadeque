// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "RadioGenreEditor.h"

#include "Shoutcast.h"

// -------------------------------------------------------------------------------- //
guRadioGenreEditor::guRadioGenreEditor( wxWindow * parent, guDbRadios * db ) :
    wxDialog( parent, wxID_ANY, _("Radio Genre Editor"), wxDefaultPosition, wxSize( 280,360 ), wxDEFAULT_DIALOG_STYLE )
{
    m_Db = db;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* ListBoxSizer;
	ListBoxSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _(" Genres ") ), wxVERTICAL );

    guShoutCast ShoutCast;
    m_RadioGenres = ShoutCast.GetGenres();

	m_CheckListBox = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_RadioGenres, wxLB_MULTIPLE|wxLB_NEEDED_SB|wxNO_BORDER );
	ListBoxSizer->Add( m_CheckListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* InputSizer;
	InputSizer = new wxBoxSizer( wxHORIZONTAL );

	m_InputStaticText = new wxStaticText( this, wxID_ANY, _("Other:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_InputStaticText->Wrap( -1 );
	InputSizer->Add( m_InputStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_InputTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	InputSizer->Add( m_InputTextCtrl, 1, wxALL, 5 );

	ListBoxSizer->Add( InputSizer, 0, wxEXPAND, 5 );

	MainSizer->Add( ListBoxSizer, 1, wxEXPAND|wxALL, 5 );

    wxStdDialogButtonSizer* TagEditorBtnSizer;
    wxButton*               TagEditorBtnSizerOK;
    wxButton*               TagEditorBtnSizerCancel;

	TagEditorBtnSizer = new wxStdDialogButtonSizer();
	TagEditorBtnSizerOK = new wxButton( this, wxID_OK );
	TagEditorBtnSizer->AddButton( TagEditorBtnSizerOK );
	TagEditorBtnSizerCancel = new wxButton( this, wxID_CANCEL );
	TagEditorBtnSizer->AddButton( TagEditorBtnSizerCancel );
	TagEditorBtnSizer->Realize();
	MainSizer->Add( TagEditorBtnSizer, 0, wxRIGHT|wxBOTTOM|wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    // By default enable already added items
    m_Db->GetRadioGenres( guRADIO_SOURCE_GENRE, &m_AddedGenres, false );
    int index;
    int count = m_AddedGenres.Count();
    int item;
    for( index = 0; index < count; index ++ )
    {
        item = m_RadioGenres.Index( m_AddedGenres[ index ].m_Name );
        if( item != wxNOT_FOUND )
            m_CheckListBox->Check( item );
    }


}

// -------------------------------------------------------------------------------- //
wxArrayString guRadioGenreEditor::GetGenres( void )
{
    wxArrayString RetVal;
    int index;
    int count = m_CheckListBox->GetCount();
    for( index = 0; index < count; index++ )
    {
        if( m_CheckListBox->IsChecked( index ) )
        {
            RetVal.Add( m_RadioGenres[ index ] );
        }
    }
    if( !m_InputTextCtrl->IsEmpty() )
    {
        RetVal.Add( m_InputTextCtrl->GetValue() );
    }

    // we add all and then remove the existans as guListBox dont have the Index function
    // so we check the other direction
    count = m_AddedGenres.Count();
    int item;
    for( index = 0; index < count; index++ )
    {
        item = RetVal.Index( m_AddedGenres[ index ].m_Name );
        if( item != wxNOT_FOUND )
        {
            RetVal.RemoveAt( item );
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guRadioGenreEditor::~guRadioGenreEditor()
{
}

// -------------------------------------------------------------------------------- //

