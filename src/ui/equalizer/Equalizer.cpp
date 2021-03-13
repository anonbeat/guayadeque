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
#include "Equalizer.h"

#include "Config.h"
#include "Images.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/fileconf.h>

namespace Guayadeque {

WX_DEFINE_OBJARRAY( guEQPresetArray )

// -------------------------------------------------------------------------------- //
bool ReadEQPresets( const wxString &value, wxArrayInt &preset )
{
    long CurVal;
    int count;
    wxArrayString Values = wxStringTokenize( value, wxT( "," ), wxTOKEN_RET_EMPTY_ALL );
    if( ( count = Values.Count() ) == guEQUALIZER_BAND_COUNT )
    {
        for( int index = 0; index < count; index++ )
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
guEq10Band::guEq10Band( wxWindow * parent, guMediaCtrl * mediactrl ) //wxDialog( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 400,250 ), wxDEFAULT_DIALOG_STYLE )
{
    m_MediaCtrl = mediactrl;
    m_BandChanged = false;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( CONFIG_KEY_EQUALIZER_POS_X, -1, CONFIG_PATH_EQUALIZER );
    WindowPos.y = Config->ReadNum( CONFIG_KEY_EQUALIZER_POS_Y, -1, CONFIG_PATH_EQUALIZER );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( CONFIG_KEY_EQUALIZER_WIDTH, 400, CONFIG_PATH_EQUALIZER );
    WindowSize.y = Config->ReadNum( CONFIG_KEY_EQUALIZER_HEIGHT, 250, CONFIG_PATH_EQUALIZER );

    Create( parent, wxID_ANY, _( "Equalizer" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX );

    wxFileConfig * EqConfig = new wxFileConfig( wxEmptyString, wxEmptyString, guPATH_EQUALIZERS_FILENAME );
    if( EqConfig )
    {
        EqConfig->SetPath( wxT( "Equalizers" ) );

        wxString    EntryName;
        wxString    EntryValue;
        wxArrayInt  Presets;
        long index;
        if( EqConfig->GetFirstEntry( EntryName, index ) )
        {
            do {
                EqConfig->Read( EntryName, &EntryValue, wxEmptyString );
                if( !EntryValue.IsEmpty() )
                {
                    //guLogMessage( wxT( "Entry%02u ) %s=%s" ), index, EntryName.c_str(), EntryValue.c_str() );
                    Presets.Empty();
                    if( ReadEQPresets( EntryValue, Presets ) && Presets.Count() == guEQUALIZER_BAND_COUNT )
                    {
                        m_EQPresets.Add( new guEQPreset( EntryName, Presets ) );
                    }
                }
            } while( EqConfig->GetNextEntry( EntryName, index ) );
        }

        delete EqConfig;
    }

    //
    SetSizeHints( wxSize( 450,260 ), wxDefaultSize );

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
    wxString LastPreset = Config->ReadStr( CONFIG_KEY_EQUALIZER_LAST_PRESET, wxEmptyString, CONFIG_PATH_EQUALIZER );
    int LastPresetIndex = wxNOT_FOUND;
    int index;
    int count = m_EQPresets.Count();
    for( index = 0; index < count; index++ )
    {
        m_PresetComboBox->Append( m_EQPresets[ index ].m_Name );
        if( m_EQPresets[ index ].m_Name == LastPreset )
            LastPresetIndex = index;
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

    wxBoxSizer * LabelsSizer;
    LabelsSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticText * Label = new wxStaticText( this, wxID_ANY, wxT("dBs"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxTOP|wxALIGN_RIGHT, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxT("12"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxALIGN_RIGHT, 5 );

    LabelsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxT("6"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxALIGN_RIGHT, 5 );

    LabelsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxT("3"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxALIGN_RIGHT, 5 );

    LabelsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxALIGN_RIGHT, 5 );

    LabelsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxALIGN_RIGHT, 5 );

    LabelsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxALIGN_RIGHT, 5 );

    LabelsSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxT("-12"), wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxALIGN_RIGHT, 5 );

    Label = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    Label->Wrap( -1 );
    LabelsSizer->Add( Label, 0, wxRIGHT|wxLEFT|wxALIGN_RIGHT, 5 );

    BandsSizer->Add( LabelsSizer, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer00;
    BandSizer00 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 0 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 0 ]->Wrap( -1 );
    BandSizer00->Add( m_Values[ 0 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 0 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 0 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 0 ]->SetLabel( wxT( "0" ) );
    BandSizer00->Add( m_Bands[ 0 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label0 = new wxStaticText( this, wxID_ANY, wxT("30"), wxDefaultPosition, wxDefaultSize, 0 );
    Label0->Wrap( -1 );
    BandSizer00->Add( Label0, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer00, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer01;
    BandSizer01 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 1 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 1 ]->Wrap( -1 );
    BandSizer01->Add( m_Values[ 1 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 1 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 1 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 1 ]->SetLabel( wxT( "1" ) );
    BandSizer01->Add( m_Bands[ 1 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label1 = new wxStaticText( this, wxID_ANY, wxT("60"), wxDefaultPosition, wxDefaultSize, 0 );
    Label1->Wrap( -1 );
    BandSizer01->Add( Label1, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer01, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer02;
    BandSizer02 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 2 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 2 ]->Wrap( -1 );
    BandSizer02->Add( m_Values[ 2 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 2 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 2 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 2 ]->SetLabel( wxT( "2" ) );
    BandSizer02->Add( m_Bands[ 2 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label2 = new wxStaticText( this, wxID_ANY, wxT("120"), wxDefaultPosition, wxDefaultSize, 0 );
    Label2->Wrap( -1 );
    BandSizer02->Add( Label2, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer02, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer03;
    BandSizer03 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 3 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 3 ]->Wrap( -1 );
    BandSizer03->Add( m_Values[ 3 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 3 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 3 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 3 ]->SetLabel( wxT( "3" ) );
    BandSizer03->Add( m_Bands[ 3 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label3 = new wxStaticText( this, wxID_ANY, wxT("250"), wxDefaultPosition, wxDefaultSize, 0 );
    Label3->Wrap( -1 );
    BandSizer03->Add( Label3, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer03, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer04;
    BandSizer04 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 4 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 4 ]->Wrap( -1 );
    BandSizer04->Add( m_Values[ 4 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 4 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 4 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 4 ]->SetLabel( wxT( "4" ) );
    BandSizer04->Add( m_Bands[ 4 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label4 = new wxStaticText( this, wxID_ANY, wxT("500"), wxDefaultPosition, wxDefaultSize, 0 );
    Label4->Wrap( -1 );
    BandSizer04->Add( Label4, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer04, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer05;
    BandSizer05 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 5 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 5 ]->Wrap( -1 );
    BandSizer05->Add( m_Values[ 5 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 5 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 5 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 5 ]->SetLabel( wxT( "5" ) );
    BandSizer05->Add( m_Bands[ 5 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label5 = new wxStaticText( this, wxID_ANY, wxT("1K"), wxDefaultPosition, wxDefaultSize, 0 );
    Label5->Wrap( -1 );
    BandSizer05->Add( Label5, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer05, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer06;
    BandSizer06 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 6 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 6 ]->Wrap( -1 );
    BandSizer06->Add( m_Values[ 6 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 6 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 6 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 6 ]->SetLabel( wxT( "6" ) );
    BandSizer06->Add( m_Bands[ 6 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label6 = new wxStaticText( this, wxID_ANY, wxT("2K"), wxDefaultPosition, wxDefaultSize, 0 );
    Label6->Wrap( -1 );
    BandSizer06->Add( Label6, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer06, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer07;
    BandSizer07 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 7 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 7 ]->Wrap( -1 );
    BandSizer07->Add( m_Values[ 7 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 7 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 7 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 7 ]->SetLabel( wxT( "7" ) );
    BandSizer07->Add( m_Bands[ 7 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label7 = new wxStaticText( this, wxID_ANY, wxT("4K"), wxDefaultPosition, wxDefaultSize, 0 );
    Label7->Wrap( -1 );
    BandSizer07->Add( Label7, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer07, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer08;
    BandSizer08 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 8 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 8 ]->Wrap( -1 );
    BandSizer08->Add( m_Values[ 8 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 8 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 8 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 8 ]->SetLabel( wxT( "8" ) );
    BandSizer08->Add( m_Bands[ 8 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    wxStaticText * Label8 = new wxStaticText( this, wxID_ANY, wxT("8K"), wxDefaultPosition, wxDefaultSize, 0 );
    Label8->Wrap( -1 );
    BandSizer08->Add( Label8, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    BandsSizer->Add( BandSizer08, 1, wxEXPAND, 5 );

    wxBoxSizer* BandSizer09;
    BandSizer09 = new wxBoxSizer( wxVERTICAL );

    m_Values[ 9 ] = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Values[ 9 ]->Wrap( -1 );
    BandSizer09->Add( m_Values[ 9 ], 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_Bands[ 9 ] = new wxSlider( this, wxID_ANY, m_MediaCtrl->GetEqualizerBand( 9 ), -120, 120, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
    m_Bands[ 9 ]->SetLabel( wxT( "9" ) );
    BandSizer09->Add( m_Bands[ 9 ], 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

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
    EQBtnSizer->SetAffirmativeButton( EQBtnOK );
    EQBtnSizer->Realize();
    MainSizer->Add( EQBtnSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    int Index;
    for( Index = 0; Index < guEQUALIZER_BAND_COUNT; Index++ )
    {
        m_Values[ Index ]->SetLabel( wxString::Format( wxT( "%.1f" ), double( m_Bands[ Index ]->GetValue() ) / 10.0 ) );

        m_Bands[ Index ]->Bind( wxEVT_SCROLL_CHANGED, &guEq10Band::OnBandChanged, this );
        m_Bands[ Index ]->Bind( wxEVT_SCROLL_THUMBTRACK, &guEq10Band::OnUpdateLabel, this );
    }

    this->SetSizer( MainSizer );
    this->Layout();

    EQBtnOK->SetDefault();

    // Bind Events
    m_PresetComboBox->Bind( wxEVT_COMBOBOX, &guEq10Band::OnPresetSelected, this );
    m_PresetComboBox->Bind( wxEVT_TEXT, &guEq10Band::OnPresetText, this );
    m_ResetButton->Bind( wxEVT_BUTTON, &guEq10Band::OnResetPreset, this );
    m_SaveButton->Bind( wxEVT_BUTTON, &guEq10Band::OnAddPreset, this );
    m_DelButton->Bind( wxEVT_BUTTON, &guEq10Band::OnDelPreset, this );

    if( LastPresetIndex != wxNOT_FOUND )
    {
        m_PresetComboBox->SetSelection( LastPresetIndex );
    }

    m_PresetComboBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
guEq10Band::~guEq10Band()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( CONFIG_KEY_EQUALIZER_POS_X, WindowPos.x, CONFIG_PATH_EQUALIZER );
    Config->WriteNum( CONFIG_KEY_EQUALIZER_POS_Y, WindowPos.y, CONFIG_PATH_EQUALIZER );
    wxSize WindowSize = GetSize();
    Config->WriteNum( CONFIG_KEY_EQUALIZER_WIDTH, WindowSize.x, CONFIG_PATH_EQUALIZER );
    Config->WriteNum( CONFIG_KEY_EQUALIZER_HEIGHT, WindowSize.y, CONFIG_PATH_EQUALIZER );

    Config->WriteStr( CONFIG_KEY_EQUALIZER_LAST_PRESET, m_BandChanged ? wxT( "" ) : m_PresetComboBox->GetValue(), CONFIG_PATH_EQUALIZER );

    wxFileConfig * EqConfig = new wxFileConfig( wxEmptyString, wxEmptyString, wxGetHomeDir() + wxT( "/.guayadeque/equalizers.conf" ) );
    if( EqConfig )
    {
        EqConfig->DeleteGroup( wxT( "Equalizers" ) );
        EqConfig->SetPath( wxT( "Equalizers" ) );
        int count = m_EQPresets.Count();
        for( int index = 0; index < count; index++ )
        {
            if( !EqConfig->Write( m_EQPresets[ index ].m_Name, wxString::Format( wxT( "%i,%i,%i,%i,%i,%i,%i,%i,%i,%i" ),
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
        delete EqConfig;
    }

    // Unbind Events
    m_PresetComboBox->Unbind( wxEVT_COMBOBOX, &guEq10Band::OnPresetSelected, this );
    m_PresetComboBox->Unbind( wxEVT_TEXT, &guEq10Band::OnPresetText, this );
    m_ResetButton->Unbind( wxEVT_BUTTON, &guEq10Band::OnResetPreset, this );
    m_SaveButton->Unbind( wxEVT_BUTTON, &guEq10Band::OnAddPreset, this );
    m_DelButton->Unbind( wxEVT_BUTTON, &guEq10Band::OnDelPreset, this );

    int Index;
    for( Index = 0; Index < guEQUALIZER_BAND_COUNT; Index++ )
    {
        m_Bands[ Index ]->Unbind( wxEVT_SCROLL_CHANGED, &guEq10Band::OnBandChanged, this );
        m_Bands[ Index ]->Unbind( wxEVT_SCROLL_THUMBTRACK, &guEq10Band::OnUpdateLabel, this );
    }
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
void guEq10Band::OnUpdateLabel( wxScrollEvent &event )
{
    wxSlider * Band = ( wxSlider * ) event.GetEventObject();
    if( Band )
    {
        long BandIndex;
        Band->GetLabel().ToLong( &BandIndex );

        m_Values[ BandIndex ]->SetLabel( wxString::Format( wxT( "%.1f" ), double( event.GetPosition() ) / 10.0 ) );
        m_Values[ BandIndex ]->GetContainingSizer()->Layout();
    }
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnBandChanged( wxScrollEvent &event )
{
    wxSlider * Band = ( wxSlider * ) event.GetEventObject();
    if( Band )
    {
        long BandIndex;
        Band->GetLabel().ToLong( &BandIndex );
        //guLogMessage( wxT( "Band%u = %i (%i)" ), BandIndex, event.GetPosition(), m_PresetComboBox->GetSelection() );
        m_MediaCtrl->SetEqualizerBand( BandIndex, event.GetPosition() );
        if( m_PresetComboBox->GetSelection() != wxNOT_FOUND )
            m_BandChanged = true;

        OnPresetText( event );

        m_Values[ BandIndex ]->SetLabel( wxString::Format( wxT( "%.1f" ), double( event.GetPosition() ) / 10.0 ) );
        m_Values[ BandIndex ]->GetContainingSizer()->Layout();
    }
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnPresetSelected( wxCommandEvent& event )
{
    int Preset = event.GetInt();
    if( Preset >= 0 )
    {
        int Index;
        for( Index = 0; Index < guEQUALIZER_BAND_COUNT; Index++ )
        {
            m_Bands[ Index ]->SetValue( m_EQPresets[ Preset ].m_Sets[ Index ] );
            m_Values[ Index ]->SetLabel( wxString::Format( wxT( "%.1f" ), double( m_EQPresets[ Preset ].m_Sets[ Index ] ) / 10.0 ) );
            m_Values[ Index ]->GetContainingSizer()->Layout();
        }

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

    m_SaveButton->Enable( m_PresetComboBox->GetCount() != 0 &&
                          ( m_BandChanged || !FindPresetName( m_PresetComboBox->GetValue(), m_EQPresets ) ) );

    m_DelButton->Enable( Sel != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guEq10Band::OnAddPreset( wxCommandEvent& event )
{
    wxArrayInt EQSet;
    int Index;
    for( Index = 0; Index < guEQUALIZER_BAND_COUNT; Index++ )
    {
        EQSet.Add( m_Bands[ Index ]->GetValue() );
    }

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
    int Index;
    for( Index = 0; Index < guEQUALIZER_BAND_COUNT; Index++ )
    {
        m_Bands[ Index ]->SetValue( Index );
    }

    m_MediaCtrl->ResetEqualizer();

    m_PresetComboBox->SetSelection( wxNOT_FOUND );
    m_PresetComboBox->SetValue( wxEmptyString );
    m_SaveButton->Enable( false );
    m_DelButton->Enable( false );
}

}

// -------------------------------------------------------------------------------- //
