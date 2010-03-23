// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#include "Equalizer.h"

#include "Config.h"
#include "Images.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/tokenzr.h>

WX_DEFINE_OBJARRAY(guEQPresetArray);

// -------------------------------------------------------------------------------- //
bool ReadEQPresets( const wxString &value, wxArrayInt &preset )
{
    long CurVal;
    int index;
    int count;
    wxArrayString Values = wxStringTokenize( value, wxT( "," ), wxTOKEN_RET_EMPTY_ALL );
    if( ( count = Values.Count() ) == guEQUALIZER_BAND_COUNT )
    {
        for( index = 0; index < count; index++ )
        {
            if( Values[ index ].ToLong( &CurVal ) )
            {
                preset.Add( CurVal );
            }
            else
              break;
        }
        return ( preset.Count() == guEQUALIZER_BAND_COUNT );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guEq10Band::guEq10Band( wxWindow * parent, guMediaCtrl * mediactrl ) :
                wxDialog( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 400,250 ), wxDEFAULT_DIALOG_STYLE )
{
    m_MediaCtrl = mediactrl;
    m_BandChanged = false;

    guConfig * Config = new guConfig( wxT( ".guayadeque/equalizers.conf" ) );
    if( Config )
    {
        Config->SetPath( wxT( "Equalizers" ) );

        wxString    EntryName;
        wxString    EntryValue;
        wxArrayInt  Presets;
        long index;
        if( Config->GetFirstEntry( EntryName, index ) )
        {
            do {
                Config->Read( EntryName, &EntryValue, wxEmptyString );
                if( !EntryValue.IsEmpty() )
                {
                    //guLogMessage( wxT( "Entry%02u ) %s=%s" ), index, EntryName.c_str(), EntryValue.c_str() );
                    Presets.Empty();
                    if( ReadEQPresets( EntryValue, Presets ) && Presets.Count() == guEQUALIZER_BAND_COUNT )
                    {
                        m_EQPresets.Add( new guEQPreset( EntryName, Presets ) );
                    }
                }
            } while( Config->GetNextEntry( EntryName, index ) );
        }

        delete Config;
    }

    //
	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* TopSizer;
	TopSizer = new wxBoxSizer( wxHORIZONTAL );

	TopSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText * PresetLabel;
	PresetLabel = new wxStaticText( this, wxID_ANY, _( "Preset:" ), wxDefaultPosition, wxDefaultSize, 0 );
	PresetLabel->Wrap( -1 );
	TopSizer->Add( PresetLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_PresetComboBox = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 );
	int index;
	int count = m_EQPresets.Count();
	for( index = 0; index < count; index++ )
	{
        m_PresetComboBox->Append( m_EQPresets[ index ].m_Name );
	}
	TopSizer->Add( m_PresetComboBox, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_SaveButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_doc_save ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_SaveButton->Enable( false );

	TopSizer->Add( m_SaveButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_DelButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit_clear ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelButton->Enable( false );

	TopSizer->Add( m_DelButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_ResetButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_reload ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	TopSizer->Add( m_ResetButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	MainSizer->Add( TopSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* BandsSizer;
	BandsSizer = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* BandSizer00;
	BandSizer00 = new wxBoxSizer( wxVERTICAL );

	m_Band0 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 0 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band0->SetLabel( wxT( "0" ) );
	BandSizer00->Add( m_Band0, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label0 = new wxStaticText( this, wxID_ANY, wxT("30"), wxDefaultPosition, wxDefaultSize, 0 );
	Label0->Wrap( -1 );
	BandSizer00->Add( Label0, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer00, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer01;
	BandSizer01 = new wxBoxSizer( wxVERTICAL );

	m_Band1 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 1 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band1->SetLabel( wxT( "1" ) );
	BandSizer01->Add( m_Band1, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label1 = new wxStaticText( this, wxID_ANY, wxT("60"), wxDefaultPosition, wxDefaultSize, 0 );
	Label1->Wrap( -1 );
	BandSizer01->Add( Label1, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer01, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer02;
	BandSizer02 = new wxBoxSizer( wxVERTICAL );

	m_Band2 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 2 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band2->SetLabel( wxT( "2" ) );
	BandSizer02->Add( m_Band2, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label2 = new wxStaticText( this, wxID_ANY, wxT("120"), wxDefaultPosition, wxDefaultSize, 0 );
	Label2->Wrap( -1 );
	BandSizer02->Add( Label2, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer02, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer03;
	BandSizer03 = new wxBoxSizer( wxVERTICAL );

	m_Band3 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 3 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band3->SetLabel( wxT( "3" ) );
	BandSizer03->Add( m_Band3, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label3 = new wxStaticText( this, wxID_ANY, wxT("250"), wxDefaultPosition, wxDefaultSize, 0 );
	Label3->Wrap( -1 );
	BandSizer03->Add( Label3, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer03, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer04;
	BandSizer04 = new wxBoxSizer( wxVERTICAL );

	m_Band4 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 4 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band4->SetLabel( wxT( "4" ) );
	BandSizer04->Add( m_Band4, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label4 = new wxStaticText( this, wxID_ANY, wxT("500"), wxDefaultPosition, wxDefaultSize, 0 );
	Label4->Wrap( -1 );
	BandSizer04->Add( Label4, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer04, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer05;
	BandSizer05 = new wxBoxSizer( wxVERTICAL );

	m_Band5 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 5 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band5->SetLabel( wxT( "5" ) );
	BandSizer05->Add( m_Band5, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label5 = new wxStaticText( this, wxID_ANY, wxT("1K"), wxDefaultPosition, wxDefaultSize, 0 );
	Label5->Wrap( -1 );
	BandSizer05->Add( Label5, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer05, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer06;
	BandSizer06 = new wxBoxSizer( wxVERTICAL );

	m_Band6 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 6 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band6->SetLabel( wxT( "6" ) );
	BandSizer06->Add( m_Band6, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label6 = new wxStaticText( this, wxID_ANY, wxT("2K"), wxDefaultPosition, wxDefaultSize, 0 );
	Label6->Wrap( -1 );
	BandSizer06->Add( Label6, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer06, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer07;
	BandSizer07 = new wxBoxSizer( wxVERTICAL );

	m_Band7 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 7 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band7->SetLabel( wxT( "7" ) );
	BandSizer07->Add( m_Band7, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label7 = new wxStaticText( this, wxID_ANY, wxT("4K"), wxDefaultPosition, wxDefaultSize, 0 );
	Label7->Wrap( -1 );
	BandSizer07->Add( Label7, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer07, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer08;
	BandSizer08 = new wxBoxSizer( wxVERTICAL );

	m_Band8 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 8 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band8->SetLabel( wxT( "8" ) );
	BandSizer08->Add( m_Band8, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label8 = new wxStaticText( this, wxID_ANY, wxT("8K"), wxDefaultPosition, wxDefaultSize, 0 );
	Label8->Wrap( -1 );
	BandSizer08->Add( Label8, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer08, 1, wxEXPAND, 5 );

	wxBoxSizer* BandSizer09;
	BandSizer09 = new wxBoxSizer( wxVERTICAL );

	m_Band9 = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 9 ), -24, 12, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_LABELS|wxSL_VERTICAL );
	m_Band9->SetLabel( wxT( "9" ) );
	BandSizer09->Add( m_Band9, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * Label9 = new wxStaticText( this, wxID_ANY, wxT("16K"), wxDefaultPosition, wxDefaultSize, 0 );
	Label9->Wrap( -1 );
	BandSizer09->Add( Label9, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	BandsSizer->Add( BandSizer09, 1, wxEXPAND, 5 );

	MainSizer->Add( BandsSizer, 1, wxEXPAND|wxBOTTOM, 5 );

	wxStdDialogButtonSizer * EQBtnSizer;
	EQBtnSizer = new wxStdDialogButtonSizer();
    wxButton * EQBtnOK;
	EQBtnOK = new wxButton( this, wxID_OK );
	EQBtnSizer->AddButton( EQBtnOK );
	EQBtnSizer->Realize();
	MainSizer->Add( EQBtnSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	// Connect Events
	m_PresetComboBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( guEq10Band::OnPresetSelected ), NULL, this );
	m_PresetComboBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guEq10Band::OnPresetText ), NULL, this );
	m_ResetButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guEq10Band::OnResetPreset ), NULL, this );
	m_SaveButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guEq10Band::OnAddPreset ), NULL, this );
	m_DelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guEq10Band::OnDelPreset ), NULL, this );
	m_Band0->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band1->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band2->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band3->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band4->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band5->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band6->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band7->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band8->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band9->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guEq10Band::~guEq10Band()
{
    guConfig * Config = new guConfig( wxT( ".guayadeque/equalizers.conf" ) );
    if( Config )
    {
        Config->DeleteGroup( wxT( "Equalizers" ) );
        Config->SetPath( wxT( "Equalizers" ) );
        int index;
        int count = m_EQPresets.Count();
        for( index = 0; index < count; index++ )
        {
            if( !Config->Write( m_EQPresets[ index ].m_Name, wxString::Format( wxT( "%i,%i,%i,%i,%i,%i,%i,%i,%i,%i" ),
              m_EQPresets[ index ].m_Sets[ 0 ],
              m_EQPresets[ index ].m_Sets[ 1 ],
              m_EQPresets[ index ].m_Sets[ 2 ],
              m_EQPresets[ index ].m_Sets[ 3 ],
              m_EQPresets[ index ].m_Sets[ 4 ],
              m_EQPresets[ index ].m_Sets[ 5 ],
              m_EQPresets[ index ].m_Sets[ 6 ],
              m_EQPresets[ index ].m_Sets[ 7 ],
              m_EQPresets[ index ].m_Sets[ 8 ],
              m_EQPresets[ index ].m_Sets[ 9 ] ) ) )
              guLogError( wxT( "Error writing key %s" ), m_EQPresets[ index ].m_Name.c_str() );
        }
        delete Config;
    }
    //
	m_PresetComboBox->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( guEq10Band::OnPresetSelected ), NULL, this );
	m_PresetComboBox->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guEq10Band::OnPresetText ), NULL, this );
	m_ResetButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guEq10Band::OnResetPreset ), NULL, this );
	m_SaveButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guEq10Band::OnAddPreset ), NULL, this );
	m_DelButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guEq10Band::OnDelPreset ), NULL, this );
	m_Band0->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band1->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band2->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band3->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band4->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band5->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band6->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band7->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band8->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
	m_Band9->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guEq10Band::OnBandChanged ), NULL, this );
}

// -------------------------------------------------------------------------------- //
bool FindPresetName( const wxString &name, guEQPresetArray &presets )
{
    int index;
    int count = presets.Count();
    for( index = 0; index < count; index++ )
    {
        if( presets[ index ].m_Name == name )
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnBandChanged( wxScrollEvent &event )
{
    wxSlider * Band = ( wxSlider * ) event.GetEventObject();
    if( Band )
    {
        long value;
        Band->GetLabel().ToLong( &value );
        //guLogMessage( wxT( "Band%u = %i (%i)" ), value, event.GetPosition(), m_PresetComboBox->GetSelection() );
        m_MediaCtrl->SetEqualizerBand( value, event.GetPosition() );
        if( m_PresetComboBox->GetSelection() != wxNOT_FOUND )
            m_BandChanged = true;

        OnPresetText( event );
    }
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnPresetSelected( wxCommandEvent& event )
{
    int Preset = event.GetInt();
    if( Preset >= 0 )
    {
        m_Band0->SetValue( m_EQPresets[ Preset ].m_Sets[ 0 ] );
        m_Band1->SetValue( m_EQPresets[ Preset ].m_Sets[ 1 ] );
        m_Band2->SetValue( m_EQPresets[ Preset ].m_Sets[ 2 ] );
        m_Band3->SetValue( m_EQPresets[ Preset ].m_Sets[ 3 ] );
        m_Band4->SetValue( m_EQPresets[ Preset ].m_Sets[ 4 ] );
        m_Band5->SetValue( m_EQPresets[ Preset ].m_Sets[ 5 ] );
        m_Band6->SetValue( m_EQPresets[ Preset ].m_Sets[ 6 ] );
        m_Band7->SetValue( m_EQPresets[ Preset ].m_Sets[ 7 ] );
        m_Band8->SetValue( m_EQPresets[ Preset ].m_Sets[ 8 ] );
        m_Band9->SetValue( m_EQPresets[ Preset ].m_Sets[ 9 ] );

        m_MediaCtrl->SetEqualizer( m_EQPresets[ Preset ].m_Sets );
        m_DelButton->Enable( true );
        m_SaveButton->Enable( false );
        m_BandChanged = false;
    }
    else
    {
        m_DelButton->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnPresetText( wxCommandEvent& event )
{
    int Sel = m_PresetComboBox->GetSelection();

    m_SaveButton->Enable( !m_PresetComboBox->GetValue().IsEmpty() &&
                          ( m_BandChanged || !FindPresetName( m_PresetComboBox->GetValue(), m_EQPresets ) ) );

    m_DelButton->Enable( Sel != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnAddPreset( wxCommandEvent& event )
{
    wxArrayInt EQSet;
    EQSet.Add( m_Band0->GetValue() );
    EQSet.Add( m_Band1->GetValue() );
    EQSet.Add( m_Band2->GetValue() );
    EQSet.Add( m_Band3->GetValue() );
    EQSet.Add( m_Band4->GetValue() );
    EQSet.Add( m_Band5->GetValue() );
    EQSet.Add( m_Band6->GetValue() );
    EQSet.Add( m_Band7->GetValue() );
    EQSet.Add( m_Band8->GetValue() );
    EQSet.Add( m_Band9->GetValue() );

    if( m_BandChanged && ( m_PresetComboBox->GetSelection() != wxNOT_FOUND ) )
    {
        m_EQPresets[ m_PresetComboBox->GetSelection() ].m_Sets = EQSet;
    }
    else
    {
        m_EQPresets.Add( new guEQPreset( m_PresetComboBox->GetValue(), EQSet ) );
        m_PresetComboBox->Append( m_PresetComboBox->GetValue() );
        m_PresetComboBox->SetSelection( m_EQPresets.Count() - 1 );
    }

    m_BandChanged = false;
    OnPresetText( event );
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnDelPreset( wxCommandEvent& event )
{
    int Sel = m_PresetComboBox->GetSelection();
    if( Sel >= 0 )
    {
        m_PresetComboBox->Delete( Sel );
        m_PresetComboBox->SetSelection( wxNOT_FOUND );
        m_PresetComboBox->SetValue( wxEmptyString );

        m_EQPresets.RemoveAt( Sel );

        m_DelButton->Enable( false );
    }
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnResetPreset( wxCommandEvent &event )
{
    m_Band0->SetValue( 0 );
    m_Band1->SetValue( 0 );
    m_Band2->SetValue( 0 );
    m_Band3->SetValue( 0 );
    m_Band4->SetValue( 0 );
    m_Band5->SetValue( 0 );
    m_Band6->SetValue( 0 );
    m_Band7->SetValue( 0 );
    m_Band8->SetValue( 0 );
    m_Band9->SetValue( 0 );
    m_MediaCtrl->ResetEqualizer();

    m_PresetComboBox->SetSelection( wxNOT_FOUND );
    m_PresetComboBox->SetValue( wxEmptyString );
    m_SaveButton->Enable( false );
    m_DelButton->Enable( false );
}

// -------------------------------------------------------------------------------- //
