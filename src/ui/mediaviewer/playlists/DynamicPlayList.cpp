// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#include "DynamicPlayList.h"

#include "Images.h"
#include "Config.h"
#include "Utils.h"

namespace Guayadeque {

WX_DEFINE_OBJARRAY( guFilterItemArray )
WX_DEFINE_OBJARRAY( guDynPlayListArray )

wxArrayString m_FilterFieldChoices;
wxArrayString m_FilterTextOptionChoices;
wxArrayString m_FilterLabelOptionChoices;
wxArrayString m_FilterNumberOptionChoices;
wxArrayString m_FilterYearOptionChoices;
wxArrayString m_FilterDateOptionChoices;
wxArrayString m_FilterBoolOptionChoices;
wxArrayString m_LimitChoices;
wxArrayString m_SortChoices;
wxArrayString m_FilterDateOption2Choices;

// -------------------------------------------------------------------------------- //
void inline InitArrayStrings( void )
{
    if( !m_FilterFieldChoices.Count() )
    {
        m_FilterFieldChoices.Add( _( "Title" ) );
        m_FilterFieldChoices.Add( _( "Artist" ) );
        m_FilterFieldChoices.Add( _( "Album Artist" ) );
        m_FilterFieldChoices.Add( _( "Album" ) );
        m_FilterFieldChoices.Add( _( "Genre" ) );
        m_FilterFieldChoices.Add( _( "Label" ) );
        m_FilterFieldChoices.Add( _( "Composer" ) );
        m_FilterFieldChoices.Add( _( "Comment" ) );
        m_FilterFieldChoices.Add( _( "Path" ) );
        m_FilterFieldChoices.Add( _( "Year" ) );
        m_FilterFieldChoices.Add( _( "Rating" ) );
        m_FilterFieldChoices.Add( _( "Length" ) );
        m_FilterFieldChoices.Add( _( "Plays" ) );
        m_FilterFieldChoices.Add( _( "Last Played" ) );
        m_FilterFieldChoices.Add( _( "Added" ) );
        m_FilterFieldChoices.Add( _( "Track Number" ) );
        m_FilterFieldChoices.Add( _( "Bit Rate" ) );
        m_FilterFieldChoices.Add( _( "Size" ) );
        m_FilterFieldChoices.Add( _( "Disc" ) );
        m_FilterFieldChoices.Add( _( "Has Cover" ) );
//    }
//
//    if( !m_FilterTextOptionChoices.Count() )
//    {
        m_FilterTextOptionChoices.Add( _("contains") );
        m_FilterTextOptionChoices.Add( _("does not contain") );
        m_FilterTextOptionChoices.Add( _("is") );
        m_FilterTextOptionChoices.Add( _("is not") );
        m_FilterTextOptionChoices.Add( _("begins with") );
        m_FilterTextOptionChoices.Add( _("ends with") );
//    }
//
//    if( !m_FilterLabelOptionChoices.Count() )
//    {
        m_FilterLabelOptionChoices.Add( _("contains") );
        m_FilterLabelOptionChoices.Add( _("does not contain") );
        m_FilterLabelOptionChoices.Add( _("is") );
        m_FilterLabelOptionChoices.Add( _("is not") );
        m_FilterLabelOptionChoices.Add( _("begins with") );
        m_FilterLabelOptionChoices.Add( _("ends with") );
        m_FilterLabelOptionChoices.Add( _("not set") );
//    }
//
//    if( !m_FilterNumberOptionChoices.Count() )
//    {
        m_FilterNumberOptionChoices.Add( _( "is" ) );
        m_FilterNumberOptionChoices.Add( _( "is not" ) );
        m_FilterNumberOptionChoices.Add( _( "at least" ) );
        m_FilterNumberOptionChoices.Add( _( "at most" ) );
//    }
//
//    if( !m_FilterYearOptionChoices.Count() )
//    {
        m_FilterYearOptionChoices.Add( _( "is" ) );
        m_FilterYearOptionChoices.Add( _( "is not" ) );
        m_FilterYearOptionChoices.Add( _( "after" ) );
        m_FilterYearOptionChoices.Add( _( "before" ) );
//    }
//
//    if( !m_FilterDateOptionChoices.Count() )
//    {
        m_FilterDateOptionChoices.Add( _( "in the last" ) );
        m_FilterDateOptionChoices.Add( _( "before the last" ) );
//    }
//
//    if( !m_FilterBoolOptionChoices.Count() )
//    {
        m_FilterBoolOptionChoices.Add( _( "false" ) );
        m_FilterBoolOptionChoices.Add( _( "true" ) );
//    }
//    if( !m_LimitChoices.Count() )
//    {
        m_LimitChoices.Add( _( "Tracks" ) );
        m_LimitChoices.Add( _( "Minutes" ) );
        m_LimitChoices.Add( wxT( "MB" ) );
        m_LimitChoices.Add( wxT( "GB" ) );
//    }
//
//    if( !m_SortChoices.Count() )
//    {
        m_SortChoices.Add( _( "Title" ) );
        m_SortChoices.Add( _( "Artist" ) );
        m_SortChoices.Add( _( "Album Artist" ) );
        m_SortChoices.Add( _( "Album" ) );
        m_SortChoices.Add( _( "Genre" ) );
        m_SortChoices.Add( _( "Label" ) );
        m_SortChoices.Add( _( "Composer" ) );
        m_SortChoices.Add( _( "Year" ) );
        m_SortChoices.Add( _( "Rating" ) );
        m_SortChoices.Add( _( "Length" ) );
        m_SortChoices.Add( _( "Plays" ) );
        m_SortChoices.Add( _( "Last Played" ) );
        m_SortChoices.Add( _( "Added" ) );
        m_SortChoices.Add( _( "Random" ) );
//    }
//
//    if( !m_FilterDateOption2Choices.Count() )
//    {
        m_FilterDateOption2Choices.Add( _( "minutes" ) );
        m_FilterDateOption2Choices.Add( _( "hours" ) );
        m_FilterDateOption2Choices.Add( _( "days" ) );
        m_FilterDateOption2Choices.Add( _( "weeks" ) );
        m_FilterDateOption2Choices.Add( _( "months" ) );
    }
}

// -------------------------------------------------------------------------------- //
// guFilterItem
// -------------------------------------------------------------------------------- //
void guFilterItem::Set( int type, int option, const wxString &text )
{
    m_Type = type;
    m_Option = option;
    m_Text = text;
    m_Number = 0;
    SetFilterLabel();
}

// -------------------------------------------------------------------------------- //
void guFilterItem::Set( int type, int option, int number, int option2 )
{
    m_Type = type;
    m_Option = option;
    m_Number = number;
    m_Option2 = option2;
    m_Text = wxEmptyString;
    SetFilterLabel();
}

// -------------------------------------------------------------------------------- //
void guFilterItem::SetFilterLabel( void )
{
    InitArrayStrings();
    m_Label = m_FilterFieldChoices[ m_Type ] + wxT( " " );
    switch( m_Type )
    {
        case guDYNAMIC_FILTER_TYPE_TITLE : // String
        case guDYNAMIC_FILTER_TYPE_ARTIST :
        case guDYNAMIC_FILTER_TYPE_ALBUMARTIST :
        case guDYNAMIC_FILTER_TYPE_ALBUM :
        case guDYNAMIC_FILTER_TYPE_GENRE :
        case guDYNAMIC_FILTER_TYPE_COMPOSER :
        case guDYNAMIC_FILTER_TYPE_COMMENT :
        case guDYNAMIC_FILTER_TYPE_PATH :
        case guDYNAMIC_FILTER_TYPE_DISK :
        {
            m_Label += m_FilterTextOptionChoices[ m_Option ] + wxT( " " );
            m_Label += m_Text;
            break;
        }

        case guDYNAMIC_FILTER_TYPE_LABEL :
        {
            m_Label += m_FilterLabelOptionChoices[ m_Option ] + wxT( " " );
            m_Label += m_Text;
            break;
        }

        case guDYNAMIC_FILTER_TYPE_YEAR : // Year
        {
            m_Label += m_FilterYearOptionChoices[ m_Option ];
            m_Label += wxString::Format( wxT( " %u" ), m_Number );
            break;
        }

        case guDYNAMIC_FILTER_TYPE_RATING : // Numbers
        case guDYNAMIC_FILTER_TYPE_PLAYCOUNT :
        case guDYNAMIC_FILTER_TYPE_TRACKNUMBER :
        case guDYNAMIC_FILTER_TYPE_BITRATE :
        case guDYNAMIC_FILTER_TYPE_SIZE :
        {
            m_Label += m_FilterNumberOptionChoices[ m_Option ];
            m_Label += wxString::Format( wxT( " %u" ), m_Number );
            break;
        }

        case guDYNAMIC_FILTER_TYPE_LENGTH : // Time
        {
            m_Label += m_FilterNumberOptionChoices[ m_Option ] + wxT( " " );
            m_Label += LenToString( m_Number * 1000 );
            break;
        }

        case guDYNAMIC_FILTER_TYPE_LASTPLAY :
        case guDYNAMIC_FILTER_TYPE_ADDEDDATE :
        {
            m_Label += m_FilterDateOptionChoices[ m_Option ];
            m_Label += wxString::Format( wxT( " %u " ), m_Number );
            m_Label += m_FilterDateOption2Choices[ m_Option2 ];
            break;
        }

        case guDYNAMIC_FILTER_TYPE_HASARTWORK :
        {
            m_Label += m_FilterBoolOptionChoices[ m_Option ];
        }
    }
}

// -------------------------------------------------------------------------------- //
// guDynPlayList
// -------------------------------------------------------------------------------- //
wxString guDynPlayList::ToString( void )
{
    wxString RetVal = wxT( "DynPlayList0:" );

    RetVal += wxString::Format( wxT( "%i:%s:%i:%i:%i:%i:%i:%i:%i:" ),
        m_Id,
        escape_configlist_str( m_Name ).c_str(), // Need to escape the ':'
        m_Limited,
        m_LimitValue,
        m_LimitType,
        m_Sorted,
        m_SortType,
        m_SortDesc,
        m_AnyOption );

    int Index;
    int Count = m_Filters.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guFilterItem FilterItem = m_Filters[ Index ];
        RetVal += wxString::Format( wxT( "{%i;%i;%s;%i;%i;%s}:" ),
            FilterItem.m_Type,
            FilterItem.m_Option,
            escape_configlist_str( FilterItem.m_Text ).c_str(),
            FilterItem.m_Number,
            FilterItem.m_Option2,
            escape_configlist_str( FilterItem.m_Label ).c_str() );
    }

    return RetVal;
}

// -------------------------------------------------------------------------------- //
void ReadFilterFromString( guFilterItem * filteritem, wxString &filterstr )
{
    filterstr = filterstr.AfterFirst( wxT( '{' ) ).BeforeLast( wxT( '}' ) );
    int Field = 0;
    while( filterstr.Length() )
    {
        wxString Val = filterstr.BeforeFirst( wxT( ';' ) );
        filterstr = filterstr.AfterFirst( wxT( ';' ) );
        //guLogMessage( wxT( "(%i) %s" ), Field, Val.c_str() );

        switch( Field )
        {
            case 0 :
            {
                filteritem->m_Type = wxAtoi( Val );
                break;
            }

            case 1 :
            {
                filteritem->m_Option = wxAtoi( Val );
                break;
            }

            case 2 :
            {
                filteritem->m_Text = unescape_configlist_str( Val );
                break;
            }

            case 3 :
            {
                filteritem->m_Number = wxAtoi( Val );
                break;
            }

            case 4 :
            {
                filteritem->m_Option2 = wxAtoi( Val );
                break;
            }

            case 5 :
            {
                filteritem->m_Label = unescape_configlist_str( Val );
                break;
            }
        }
        Field++;
    }
}

// -------------------------------------------------------------------------------- //
void ReadFiltersFromString( guFilterItemArray * filterarray, wxString &filterstr )
{
    while( filterstr.Length() )
    {
        wxString Filter = filterstr.BeforeFirst( wxT( ':' ) );
        filterstr = filterstr.AfterFirst( wxT( ':' ) );
        if( Filter.Length() )
        {
            guFilterItem * FilterItem = new guFilterItem();
            ReadFilterFromString( FilterItem, Filter );
            filterarray->Add( FilterItem );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayList::FromString( const wxString &playlist )
{
    wxString Fields = playlist;
    int Field = 0;
    m_Filters.Empty();

    while( Fields.Length() )
    {
        wxString Val = Fields.BeforeFirst( wxT( ':' ) );
        Fields = Fields.AfterFirst( wxT( ':' ) );
        switch( Field )
        {
            case 0 :
            {
                if( Val != wxT( "DynPlayList0" ) )
                {
                    return;
                }
                break;
            }

            case 1 :
            {
                m_Id = wxAtoi( Val );
                break;
            }

            case 2 :
            {
                m_Name = unescape_configlist_str( Val );
                break;
            }

            case 3 :
            {
                m_Limited = wxAtoi( Val );
                break;
            }

            case 4 :
            {
                m_LimitValue = wxAtoi( Val );
                break;
            }

            case 5 :
            {
                m_LimitType = wxAtoi( Val );
                break;
            }

            case 6 :
            {
                m_Sorted = wxAtoi( Val );
                break;
            }

            case 7 :
            {
                m_SortType = wxAtoi( Val );
                break;
            }

            case 8 :
            {
                m_SortDesc = wxAtoi( Val );
                break;
            }

            case 9 :
            {
                m_AnyOption = wxAtoi( Val );
                break;
            }
        }

        Field++;
        if( Field > 9 )
            break;
    }

    ReadFiltersFromString( &m_Filters, Fields );
}

// -------------------------------------------------------------------------------- //
// guDynPlayLIstEditor
// -------------------------------------------------------------------------------- //
guDynPlayListEditor::guDynPlayListEditor( wxWindow * parent, guDynPlayList * playlist,
                                          const bool albumfilter ) :
  wxDialog( parent, wxID_ANY, albumfilter ? _( "Filter Album Browser" ) :
    _( "Dynamic Playlist Editor" ), wxDefaultPosition, wxSize( 600,400 ), wxDEFAULT_DIALOG_STYLE )
{
	int index;
	int count;
	m_PlayList = playlist;
	m_Filters = &m_PlayList->m_Filters;
	m_CurFilter = wxNOT_FOUND;
	m_HasChanged = false;
	m_AlbumFilter = albumfilter;

    InitArrayStrings();

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * NameSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * NameStaticText = new wxStaticText( this, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
	NameStaticText->Wrap( -1 );
	NameSizer->Add( NameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_NameTextCtrl = new wxTextCtrl( this, wxID_ANY, playlist->m_Name, wxDefaultPosition, wxDefaultSize, 0 );
	NameSizer->Add( m_NameTextCtrl, 1, wxALL|wxEXPAND, 5 );

	MainSizer->Add( NameSizer, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticBoxSizer * CurFiltersSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( " Current Filters " ) ), wxVERTICAL );

	m_FiltersListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_FiltersListBox->SetMinSize( wxSize( -1,80 ) );

	count = m_Filters->Count();
	//guLogMessage( wxT( "Found %u filters" ), count );
	for( index = 0; index < count; index++ )
	{
	    m_FiltersListBox->Append( ( * m_Filters )[ index ].GetLabel() );
	    //guLogMessage( wxT( "Filter %u : %s" ), index, ( * m_Filters )[ index ].GetLabel().c_str() );
	}

	CurFiltersSizer->Add( m_FiltersListBox, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_FilterEditSizer = new wxBoxSizer( wxHORIZONTAL );

	m_FilterFieldChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FilterFieldChoices, 0 );
	m_FilterFieldChoice->SetSelection( 0 );
	m_FilterEditSizer->Add( m_FilterFieldChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_FilterTextOptionChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FilterTextOptionChoices, 0 );
	m_FilterTextOptionChoice->SetSelection( 0 );
	m_FilterEditSizer->Add( m_FilterTextOptionChoice, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_FilterLabelOptionChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FilterLabelOptionChoices, 0 );
	m_FilterLabelOptionChoice->SetSelection( 0 );
    m_FilterLabelOptionChoice->Show( false );
	m_FilterEditSizer->Add( m_FilterLabelOptionChoice, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_FilterText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_FilterEditSizer->Add( m_FilterText, 1, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_FilterRating = new guRating( this, GURATING_STYLE_BIG );
	m_FilterRating->Show( false );
	m_FilterEditSizer->Add( m_FilterRating, 1, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_FilterDateOption2Choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FilterDateOption2Choices, 0 );
	m_FilterDateOption2Choice->SetSelection( 2 );
	m_FilterDateOption2Choice->Show( false );
	m_FilterEditSizer->Add( m_FilterDateOption2Choice, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_LengthHours = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 9999 );
    m_LengthHours->Show( false );
	m_FilterEditSizer->Add( m_LengthHours, 1, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_LengthSeparator1 = new wxStaticText( this, wxID_ANY, wxT( ":" ) );
    m_LengthSeparator1->Show( false );
	m_FilterEditSizer->Add( m_LengthSeparator1, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_LengthMinutes = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 59 );
    m_LengthMinutes->Show( false );
	m_FilterEditSizer->Add( m_LengthMinutes, 1, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_LengthSeparator2 = new wxStaticText( this, wxID_ANY, wxT( ":" ) );
    m_LengthSeparator2->Show( false );
	m_FilterEditSizer->Add( m_LengthSeparator2, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_LengthSeconds = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 59 );
    m_LengthSeconds->Show( false );
	m_FilterEditSizer->Add( m_LengthSeconds, 1, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_FilterAdd = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_FilterAdd->Enable( false );
	m_FilterEditSizer->Add( m_FilterAdd, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5 );

	m_FilterDel = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_FilterDel->Enable( false );
	m_FilterEditSizer->Add( m_FilterDel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT, 5 );

	m_FilterAccept = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_FilterAccept->Enable( false );
	m_FilterEditSizer->Add( m_FilterAccept, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	CurFiltersSizer->Add( m_FilterEditSizer, 0, wxEXPAND, 5 );

	MainSizer->Add( CurFiltersSizer, 1, wxEXPAND|wxALL, 5 );

    if( !albumfilter )
    {
        wxStaticBoxSizer* ResultSizer;
        ResultSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _(" Result ") ), wxVERTICAL );

        wxBoxSizer* LimitSizer;
        LimitSizer = new wxBoxSizer( wxHORIZONTAL );

        m_LimitCheckBox = new wxCheckBox( this, wxID_ANY, _("Limit to"), wxDefaultPosition, wxDefaultSize, 0 );
        m_LimitCheckBox->SetValue( m_PlayList->m_Limited );

        LimitSizer->Add( m_LimitCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

        m_LimitSpinCtrl = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 2147483647, 0 );
        m_LimitSpinCtrl->SetValue( m_PlayList->m_LimitValue );
        m_LimitSpinCtrl->Enable( m_LimitCheckBox->IsChecked() );

        LimitSizer->Add( m_LimitSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

        m_LimitChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_LimitChoices, 0 );
        m_LimitChoice->SetSelection( m_PlayList->m_LimitType );
        m_LimitChoice->Enable( m_LimitCheckBox->IsChecked() );

        LimitSizer->Add( m_LimitChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );


        LimitSizer->Add( 0, 0, 1, wxEXPAND, 5 );

        m_AddOnAnyCheckBox = new wxCheckBox( this, wxID_ANY, _("Any filter selects tracks"), wxDefaultPosition, wxDefaultSize, 0 );
        m_AddOnAnyCheckBox->SetValue( m_PlayList->m_AnyOption );
        m_AddOnAnyCheckBox->Enable( m_FiltersListBox->GetCount() > 1 );

        LimitSizer->Add( m_AddOnAnyCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

        ResultSizer->Add( LimitSizer, 1, wxEXPAND, 5 );

        wxBoxSizer* SortSizer;
        SortSizer = new wxBoxSizer( wxHORIZONTAL );

        m_SortCheckBox = new wxCheckBox( this, wxID_ANY, _("Sort by"), wxDefaultPosition, wxDefaultSize, 0 );
        m_SortCheckBox->SetValue( m_PlayList->m_Sorted );
        SortSizer->Add( m_SortCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

        m_SortChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_SortChoices, 0 );
        m_SortChoice->SetSelection( m_PlayList->m_SortType );
        m_SortChoice->Enable( m_SortCheckBox->IsChecked() );

        SortSizer->Add( m_SortChoice, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

        m_DescCheckBox = new wxCheckBox( this, wxID_ANY, _("Descending"), wxDefaultPosition, wxDefaultSize, 0 );
        m_DescCheckBox->SetValue( m_PlayList->m_SortDesc );
        m_DescCheckBox->Enable( m_SortCheckBox->IsChecked() );

        SortSizer->Add( m_DescCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

        ResultSizer->Add( SortSizer, 1, wxEXPAND, 5 );

        MainSizer->Add( ResultSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    }
    else
    {
        wxBoxSizer* LimitSizer;
        LimitSizer = new wxBoxSizer( wxHORIZONTAL );

        LimitSizer->Add( 0, 0, 1, wxEXPAND, 5 );

        m_AddOnAnyCheckBox = new wxCheckBox( this, wxID_ANY, _("Add tracks on any criteria"), wxDefaultPosition, wxDefaultSize, 0 );
        m_AddOnAnyCheckBox->SetValue( m_PlayList->m_AnyOption );
        m_AddOnAnyCheckBox->Enable( m_FiltersListBox->GetCount() > 1 );
        LimitSizer->Add( m_AddOnAnyCheckBox, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
        MainSizer->Add( LimitSizer, 0, wxEXPAND, 5 );
    }

    wxStdDialogButtonSizer * ButtonsSizer;
    wxButton *  BtnCancel;

	ButtonsSizer = new wxStdDialogButtonSizer();
	m_BtnOk = new wxButton( this, wxID_OK );
    m_BtnOk->Enable( m_Filters->Count() && !m_NameTextCtrl->IsEmpty() );
	ButtonsSizer->AddButton( m_BtnOk );
	BtnCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( BtnCancel );
	ButtonsSizer->SetAffirmativeButton( m_BtnOk );
	ButtonsSizer->SetCancelButton( BtnCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	m_BtnOk->SetDefault();

    // Bind Events
    m_FiltersListBox->Bind( wxEVT_LISTBOX, &guDynPlayListEditor::OnFiltersSelected, this );

    m_FilterFieldChoice->Bind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterFieldSelected, this );
    m_FilterTextOptionChoice->Bind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterOptionSelected, this );
    m_FilterLabelOptionChoice->Bind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterOptionSelected, this );
    m_FilterText->Bind( wxEVT_TEXT, &guDynPlayListEditor::OnFilterTextChanged, this );
    m_FilterRating->Bind( guEVT_RATING_CHANGED, &guDynPlayListEditor::OnRatingChanged, this );
    m_FilterDateOption2Choice->Bind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterDateOption2Selected, this );
    m_FilterAdd->Bind( wxEVT_BUTTON, &guDynPlayListEditor::OnFilterAddClicked, this );
    m_FilterDel->Bind( wxEVT_BUTTON, &guDynPlayListEditor::OnFilterDelClicked, this );
    m_FilterAccept->Bind( wxEVT_BUTTON, &guDynPlayListEditor::OnFilterUpdateClicked, this );
    if( !m_AlbumFilter )
	{
        m_LimitCheckBox->Bind( wxEVT_CHECKBOX, &guDynPlayListEditor::OnLimitChecked, this );
        m_SortCheckBox->Bind( wxEVT_CHECKBOX, &guDynPlayListEditor::OnSortChecked, this );
	}
    m_NameTextCtrl->Bind( wxEVT_TEXT, &guDynPlayListEditor::OnNameChanged, this );
    m_LengthHours->Bind( wxEVT_SPINCTRL, &guDynPlayListEditor::OnHoursChanged, this );
    m_LengthMinutes->Bind( wxEVT_SPINCTRL, &guDynPlayListEditor::OnMinutesChanged, this );
    m_LengthSeconds->Bind( wxEVT_SPINCTRL, &guDynPlayListEditor::OnSecondsChanged, this );

	m_FiltersListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
guDynPlayListEditor::~guDynPlayListEditor()
{
    m_FiltersListBox->Unbind( wxEVT_LISTBOX, &guDynPlayListEditor::OnFiltersSelected, this );

    m_FilterFieldChoice->Unbind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterFieldSelected, this );
    m_FilterTextOptionChoice->Unbind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterOptionSelected, this );
    m_FilterLabelOptionChoice->Unbind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterOptionSelected, this );
    m_FilterText->Unbind( wxEVT_TEXT, &guDynPlayListEditor::OnFilterTextChanged, this );
    m_FilterRating->Unbind( guEVT_RATING_CHANGED, &guDynPlayListEditor::OnRatingChanged, this );
    m_FilterDateOption2Choice->Unbind( wxEVT_CHOICE, &guDynPlayListEditor::OnFilterDateOption2Selected, this );
    m_FilterAdd->Unbind( wxEVT_BUTTON, &guDynPlayListEditor::OnFilterAddClicked, this );
    m_FilterDel->Unbind( wxEVT_BUTTON, &guDynPlayListEditor::OnFilterDelClicked, this );
    m_FilterAccept->Unbind( wxEVT_BUTTON, &guDynPlayListEditor::OnFilterUpdateClicked, this );
    if( !m_AlbumFilter )
    {
        m_LimitCheckBox->Unbind( wxEVT_CHECKBOX, &guDynPlayListEditor::OnLimitChecked, this );
        m_SortCheckBox->Unbind( wxEVT_CHECKBOX, &guDynPlayListEditor::OnSortChecked, this );
    }
    m_NameTextCtrl->Unbind( wxEVT_TEXT, &guDynPlayListEditor::OnNameChanged, this );
    m_LengthHours->Unbind( wxEVT_SPINCTRL, &guDynPlayListEditor::OnHoursChanged, this );
    m_LengthMinutes->Unbind( wxEVT_SPINCTRL, &guDynPlayListEditor::OnMinutesChanged, this );
    m_LengthSeconds->Unbind( wxEVT_SPINCTRL, &guDynPlayListEditor::OnSecondsChanged, this );
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::FillPlayListEditData( void )
{
    m_PlayList->m_Name = m_NameTextCtrl->GetValue();
    if( !m_AlbumFilter )
    {
        m_PlayList->m_Limited = m_LimitCheckBox->IsChecked();
        m_PlayList->m_LimitValue = m_LimitSpinCtrl->GetValue();
        m_PlayList->m_LimitType = m_LimitChoice->GetSelection();
        m_PlayList->m_Sorted = m_SortCheckBox->IsChecked();
        m_PlayList->m_SortType = m_SortChoice->GetSelection();
        m_PlayList->m_SortDesc = m_DescCheckBox->IsChecked();
    }
    else
    {
        m_PlayList->m_Limited = false;
        m_PlayList->m_LimitValue = 0;
        m_PlayList->m_LimitType = 0;
        m_PlayList->m_Sorted = false;
        m_PlayList->m_SortType = 0;
        m_PlayList->m_SortDesc = false;
    }
    m_PlayList->m_AnyOption = m_AddOnAnyCheckBox->IsChecked();
}

// -------------------------------------------------------------------------------- //
guFilterItem guDynPlayListEditor::GetFilterItem( void )
{
    guFilterItem FilterItem;
    int FilterType = m_FilterFieldChoice->GetSelection();
    if( FilterType == guDYNAMIC_FILTER_TYPE_LABEL )
    {
        FilterItem.Set( FilterType,
                    m_FilterLabelOptionChoice->GetSelection(),
                    FilterItem.m_Text = m_FilterText->GetValue() );
    }
    else if( ( FilterType < guDYNAMIC_FILTER_TYPE_YEAR ) ||
             ( FilterType == guDYNAMIC_FILTER_TYPE_DISK ) )
    {
        FilterItem.Set( FilterType,
                    m_FilterTextOptionChoice->GetSelection(),
                    FilterItem.m_Text = m_FilterText->GetValue() );
    }
    else if( FilterType == guDYNAMIC_FILTER_TYPE_RATING )
    {
        FilterItem.Set( FilterType,
                        m_FilterTextOptionChoice->GetSelection(),
                        m_FilterRating->GetRating(),
                        0 );
    }
    else if( FilterType == guDYNAMIC_FILTER_TYPE_LENGTH )
    {
        unsigned long hour = m_LengthHours->GetValue();
        unsigned long min = m_LengthMinutes->GetValue();
        unsigned long sec = m_LengthSeconds->GetValue();
        FilterItem.Set( FilterType,
                        m_FilterTextOptionChoice->GetSelection(),
                        ( hour * 3600 ) + ( min * 60 ) + sec,
                        0 );

    }
    else if( ( FilterType == guDYNAMIC_FILTER_TYPE_LASTPLAY ) ||
             ( FilterType == guDYNAMIC_FILTER_TYPE_ADDEDDATE ) )
    {
        unsigned long value = 0;
        m_FilterText->GetValue().ToULong( &value );
        FilterItem.Set( FilterType,
                        m_FilterTextOptionChoice->GetSelection(),
                        value,
                        m_FilterDateOption2Choice->GetSelection() );
    }
    else // FilterType == guDYNAMIC_FILTER_TYPE_YEAR
    {
        unsigned long value = 0;
        m_FilterText->GetValue().ToULong( &value );
        FilterItem.Set( FilterType,
                        m_FilterTextOptionChoice->GetSelection(),
                        value,
                        0 );
    }
    return FilterItem;
}

// -------------------------------------------------------------------------------- //
bool guDynPlayListEditor::FilterHasChanged( void )
{
    return m_HasChanged;
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::UpdateEditor( int FilterType )
{
    switch( FilterType )
    {
        case guDYNAMIC_FILTER_TYPE_RATING :
        case guDYNAMIC_FILTER_TYPE_LENGTH :
        case guDYNAMIC_FILTER_TYPE_HASARTWORK :
            m_FilterAdd->Enable( true );
            break;

        default :
            m_FilterAdd->Enable( false );
            break;
    }

    m_FilterText->Enable( FilterType != guDYNAMIC_FILTER_TYPE_HASARTWORK );

    if( FilterType == guDYNAMIC_FILTER_TYPE_RATING )
    {
        m_FilterText->Show( false );
        m_FilterTextOptionChoice->Show( true );
        m_FilterLabelOptionChoice->Show( false );
        m_FilterRating->Show( true );
        m_FilterRating->SetRating( 0 );
        m_FilterDateOption2Choice->Show( false );
        m_LengthHours->Show( false );
        m_LengthSeparator1->Show( false );
        m_LengthMinutes->Show( false );
        m_LengthSeparator2->Show( false );
        m_LengthSeconds->Show( false );
    }
    else if( FilterType == guDYNAMIC_FILTER_TYPE_LENGTH )
    {
        m_FilterText->Show( false );
        m_FilterTextOptionChoice->Show( true );
        m_FilterLabelOptionChoice->Show( false );
        m_FilterRating->Show( false );
        m_FilterDateOption2Choice->Show( false );
        m_LengthHours->Show( true );
        m_LengthSeparator1->Show( true );
        m_LengthMinutes->Show( true );
        m_LengthSeparator2->Show( true );
        m_LengthSeconds->Show( true );
    }
    else if( ( FilterType == guDYNAMIC_FILTER_TYPE_LASTPLAY ) ||
             ( FilterType == guDYNAMIC_FILTER_TYPE_ADDEDDATE ) )
    {
        m_FilterText->Show( true );
        m_FilterTextOptionChoice->Show( true );
        m_FilterLabelOptionChoice->Show( false );
        m_FilterRating->Show( false );
        m_FilterDateOption2Choice->Show( true );
        m_LengthHours->Show( false );
        m_LengthSeparator1->Show( false );
        m_LengthMinutes->Show( false );
        m_LengthSeparator2->Show( false );
        m_LengthSeconds->Show( false );
    }
    else if( FilterType == guDYNAMIC_FILTER_TYPE_LABEL )
    {
        m_FilterText->Show( true );
        m_FilterTextOptionChoice->Show( false );
        m_FilterLabelOptionChoice->Show( true );
        m_FilterRating->Show( false );
        m_FilterDateOption2Choice->Show( false );
        m_LengthHours->Show( false );
        m_LengthSeparator1->Show( false );
        m_LengthMinutes->Show( false );
        m_LengthSeparator2->Show( false );
        m_LengthSeconds->Show( false );
    }
    else if( FilterType == guDYNAMIC_FILTER_TYPE_HASARTWORK )
    {
        m_FilterText->Show( true );
        m_FilterTextOptionChoice->Show( true );
        m_FilterLabelOptionChoice->Show( false );
        m_FilterRating->Show( false );
        m_FilterDateOption2Choice->Show( false );
        m_LengthHours->Show( false );
        m_LengthSeparator1->Show( false );
        m_LengthMinutes->Show( false );
        m_LengthSeparator2->Show( false );
        m_LengthSeconds->Show( false );
    }
    else
    {
        m_FilterText->Show( true );
        m_FilterTextOptionChoice->Show( true );
        m_FilterLabelOptionChoice->Show( false );
        m_FilterRating->Show( false );
        m_FilterDateOption2Choice->Show( false );
        m_LengthHours->Show( false );
        m_LengthSeparator1->Show( false );
        m_LengthMinutes->Show( false );
        m_LengthSeparator2->Show( false );
        m_LengthSeconds->Show( false );
    }

    m_FilterTextOptionChoice->Clear();

    if( ( FilterType < guDYNAMIC_FILTER_TYPE_YEAR ) ||
        ( FilterType == guDYNAMIC_FILTER_TYPE_DISK ) )
    {
          m_FilterTextOptionChoice->Append( m_FilterTextOptionChoices );
    }
    else if( FilterType == guDYNAMIC_FILTER_TYPE_YEAR )
    {
        m_FilterTextOptionChoice->Append( m_FilterYearOptionChoices );
    }
    else if( ( FilterType == guDYNAMIC_FILTER_TYPE_LASTPLAY ) ||
             ( FilterType == guDYNAMIC_FILTER_TYPE_ADDEDDATE ) )
    {
        m_FilterTextOptionChoice->Append( m_FilterDateOptionChoices );
    }
    else if( FilterType == guDYNAMIC_FILTER_TYPE_HASARTWORK )
    {
        m_FilterTextOptionChoice->Append( m_FilterBoolOptionChoices );
    }
    else //if( FilterType < guDYNAMIC_FILTER_TYPE_LASTPLAY )
    {
        m_FilterTextOptionChoice->Append( m_FilterNumberOptionChoices );
    }
    m_FilterTextOptionChoice->SetSelection( 0 );
    m_FilterLabelOptionChoice->SetSelection( 0 );

    m_FilterEditSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFiltersSelected( wxCommandEvent &event )
{
    m_CurFilter = event.GetInt();
    if( m_CurFilter != wxNOT_FOUND )
    {
        guFilterItem * FilterItem = &( * m_Filters )[ m_CurFilter ];
        UpdateEditor( FilterItem->m_Type );
        m_FilterFieldChoice->SetSelection( FilterItem->m_Type );
        //guLogMessage( wxT( "Type : %u   Option: %u" ), FilterItem->m_Type, FilterItem->m_Option );
        if( FilterItem->m_Type == guDYNAMIC_FILTER_TYPE_LABEL )
            m_FilterLabelOptionChoice->SetSelection( FilterItem->m_Option );
        else
            m_FilterTextOptionChoice->SetSelection( FilterItem->m_Option );

        if( ( FilterItem->m_Type < guDYNAMIC_FILTER_TYPE_YEAR ) ||
            ( FilterItem->m_Type == guDYNAMIC_FILTER_TYPE_DISK ) )
        {
            m_FilterText->SetValue( FilterItem->m_Text );
        }
        else if( FilterItem->m_Type == guDYNAMIC_FILTER_TYPE_RATING )
        {
            m_FilterRating->SetRating( FilterItem->m_Number );
        }
        else if( FilterItem->m_Type == guDYNAMIC_FILTER_TYPE_LENGTH )
        {
            unsigned long value = FilterItem->m_Number;
            m_LengthHours->SetValue( int( value / 3600 ) );
            value = value % 3600;
            m_LengthMinutes->SetValue( int( value / 60 ) );
            value = value % 60;
            m_LengthSeconds->SetValue( value );
        }
        else
        {
            m_FilterText->SetValue( wxString::Format( wxT( "%u" ), FilterItem->m_Number ) );
        }

        m_FilterDateOption2Choice->SetSelection( FilterItem->m_Option2 );
        //m_Filter
        m_FilterDel->Enable( true );
        m_FilterAccept->Enable( false );
        m_HasChanged = false;

    }
    else
    {
        m_FilterDel->Enable( false );
        m_FilterAccept->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFilterFieldSelected( wxCommandEvent &event )
{
    int FilterType = event.GetInt();

    m_FilterText->SetValue( wxEmptyString );
    m_LengthHours->SetValue( 0 );
    m_LengthMinutes->SetValue( 0 );
    m_LengthSeconds->SetValue( 0 );
    m_FilterRating->SetRating( 0 );

    UpdateEditor( FilterType );

    if( m_CurFilter != wxNOT_FOUND )
    {
        m_HasChanged = true;
        m_FilterAccept->Enable( true );
    }

}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFilterOptionSelected( wxCommandEvent &event )
{
    bool IsNotSetLabel = ( m_FilterFieldChoice->GetSelection() == guDYNAMIC_FILTER_TYPE_LENGTH ) ||
                         ( m_FilterFieldChoice->GetSelection() == guDYNAMIC_FILTER_TYPE_HASARTWORK ) ||
                          ( ( m_FilterFieldChoice->GetSelection() == guDYNAMIC_FILTER_TYPE_LABEL ) &&
                          ( m_FilterLabelOptionChoice->GetSelection() == guDYNAMIC_FILTER_OPTION_LABELS_NOTSET ) );
    //guLogMessage( wxT( "IsNotSetLabel: %i" ), IsNotSetLabel );
    m_FilterText->Enable( !IsNotSetLabel );
    if( IsNotSetLabel )
        m_FilterText->SetValue( wxEmptyString );
    m_FilterAdd->Enable( IsNotSetLabel || !m_FilterText->IsEmpty() );

    if( m_CurFilter != wxNOT_FOUND )
    {
        m_HasChanged = true;
        m_FilterAccept->Enable( true );
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFilterDateOption2Selected( wxCommandEvent &event )
{
    if( m_CurFilter != wxNOT_FOUND )
    {
        m_HasChanged = true;
        m_FilterAccept->Enable( true );
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFilterTextChanged( wxCommandEvent& event )
{
    wxString FilterText = event.GetString();
    int FilterType = m_FilterFieldChoice->GetSelection();
    if( ( FilterType > guDYNAMIC_FILTER_TYPE_PATH ) &&
        ( FilterType != guDYNAMIC_FILTER_TYPE_DISK ) )
    {
        unsigned long value = 0;

        if( !FilterText.IsEmpty() && !FilterText.ToULong( &value ) )
        {
            m_FilterText->ChangeValue( wxEmptyString );
            //m_FilterText->SetValue( wxEmptyString );
        }
    }


    bool Enable = !FilterText.IsEmpty() ||
                     ( ( m_FilterFieldChoice->GetSelection() == guDYNAMIC_FILTER_TYPE_LABEL ) &&
                     ( m_FilterLabelOptionChoice->GetSelection() == guDYNAMIC_FILTER_OPTION_LABELS_NOTSET ) );

    m_FilterAdd->Enable( Enable );

    if( m_CurFilter != wxNOT_FOUND )
    {
        m_HasChanged = true;
        m_FilterAccept->Enable( Enable );
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnRatingChanged( guRatingEvent &event )
{
    m_FilterAdd->Enable( m_FilterRating->GetRating() >= 0 );
    if( m_CurFilter != wxNOT_FOUND )
    {
        m_HasChanged = true;
        m_FilterAccept->Enable( true );
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFilterAddClicked( wxCommandEvent& event )
{
    guFilterItem FilterItem = GetFilterItem();
    m_Filters->Add( FilterItem );
    m_FiltersListBox->Append( FilterItem.GetLabel() );
    m_FiltersListBox->Refresh();

    m_BtnOk->Enable( m_Filters->Count() && !m_NameTextCtrl->IsEmpty() );

    m_AddOnAnyCheckBox->Enable( m_FiltersListBox->GetCount() > 1 );
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFilterDelClicked( wxCommandEvent& event )
{
    if( m_CurFilter != wxNOT_FOUND )
    {
        m_Filters->RemoveAt( m_CurFilter );
        m_FiltersListBox->Delete( m_CurFilter );
        m_CurFilter = wxNOT_FOUND;

        m_BtnOk->Enable( m_Filters->Count() );

        m_AddOnAnyCheckBox->Enable( m_FiltersListBox->GetCount() > 1 );
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnFilterUpdateClicked( wxCommandEvent& event )
{
    if( m_CurFilter != wxNOT_FOUND )
    {
        guFilterItem FilterItem = GetFilterItem();
        ( * m_Filters )[ m_CurFilter ] = FilterItem;
        m_FiltersListBox->SetString( m_CurFilter, FilterItem.GetLabel() );
        m_FilterAccept->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnLimitChecked( wxCommandEvent &event )
{
	m_LimitChoice->Enable( m_LimitCheckBox->IsChecked() );
	m_LimitSpinCtrl->Enable( m_LimitCheckBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnSortChecked( wxCommandEvent &event )
{
	m_DescCheckBox->Enable( m_SortCheckBox->IsChecked() );
	m_SortChoice->Enable( m_SortCheckBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnNameChanged( wxCommandEvent &event )
{
    m_BtnOk->Enable( m_Filters->Count() && !event.GetString().IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnHoursChanged( wxSpinEvent &event )
{
    m_FilterAccept->Enable( m_CurFilter != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnMinutesChanged( wxSpinEvent &event )
{
    m_FilterAccept->Enable( m_CurFilter != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guDynPlayListEditor::OnSecondsChanged( wxSpinEvent &event )
{
    m_FilterAccept->Enable( m_CurFilter != wxNOT_FOUND );
}

}

// -------------------------------------------------------------------------------- //
