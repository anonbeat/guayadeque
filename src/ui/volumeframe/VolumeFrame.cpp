// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
#include "VolumeFrame.h"
//#include "Utils.h"

namespace Guayadeque {

#define guVOLUMEN_AUTOCLOSE_TIMEOUT 3000

// -------------------------------------------------------------------------------- //
guVolumeFrame::guVolumeFrame( guPlayerPanel * Player, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) :
                 wxFrame( parent, id, title, pos, size, style|wxFRAME_NO_TASKBAR )
{
    m_PlayerPanel = Player;
    m_MouseTimer = new wxTimer( this );
    m_MouseTimer->Start( guVOLUMEN_AUTOCLOSE_TIMEOUT, wxTIMER_ONE_SHOT );

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* SetVolSizer;
	SetVolSizer = new wxBoxSizer( wxVERTICAL );

	m_IncVolButton = new wxButton( this, wxID_ANY, wxT("+"), wxDefaultPosition, wxSize( 24,28 ), wxNO_BORDER );
	SetVolSizer->Add( m_IncVolButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );

	m_VolSlider = new wxSlider( this, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE|wxSL_VERTICAL );
	SetVolSizer->Add( m_VolSlider, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 2 );
	if( m_PlayerPanel )
	{
        m_VolSlider->SetValue( m_PlayerPanel->GetVolume() );
        //VolSlider->Refresh();
	}

	m_DecVolButton = new wxButton( this, wxID_ANY, wxT("-"), wxDefaultPosition, wxSize( 24,28 ), wxNO_BORDER );
	SetVolSizer->Add( m_DecVolButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );

	this->SetSizer( SetVolSizer );
	this->Layout();

	// Bind Events
	Bind( wxEVT_ACTIVATE, &guVolumeFrame::VolFrameActivate, this );
	Bind( wxEVT_MOUSEWHEEL, &guVolumeFrame::OnMouseWheel, this );
	m_VolSlider->Bind( wxEVT_MOUSEWHEEL, &guVolumeFrame::OnMouseWheel, this );
    m_IncVolButton->Bind( wxEVT_BUTTON, &guVolumeFrame::IncVolButtonClick, this );
	m_VolSlider->Bind( wxEVT_SCROLL_CHANGED, &guVolumeFrame::VolSliderChanged, this );
	m_VolSlider->Bind( wxEVT_SCROLL_THUMBTRACK, &guVolumeFrame::VolSliderChanged, this );

    m_DecVolButton->Bind( wxEVT_BUTTON, &guVolumeFrame::DecVolButtonClick, this );

    Bind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );
    m_IncVolButton->Bind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );
    m_VolSlider->Bind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );
    m_DecVolButton->Bind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );

    Bind( wxEVT_TIMER, &guVolumeFrame::OnTimer, this );

}

// -------------------------------------------------------------------------------- //
guVolumeFrame::~guVolumeFrame()
{
    // Unbind Events
    Unbind( wxEVT_ACTIVATE, &guVolumeFrame::VolFrameActivate, this );
    Unbind( wxEVT_MOUSEWHEEL, &guVolumeFrame::OnMouseWheel, this );
    m_VolSlider->Unbind( wxEVT_MOUSEWHEEL, &guVolumeFrame::OnMouseWheel, this );
    m_IncVolButton->Unbind( wxEVT_BUTTON, &guVolumeFrame::IncVolButtonClick, this );
    m_VolSlider->Unbind( wxEVT_SCROLL_CHANGED, &guVolumeFrame::VolSliderChanged, this );
    m_VolSlider->Unbind( wxEVT_SCROLL_THUMBTRACK, &guVolumeFrame::VolSliderChanged, this );

    m_DecVolButton->Unbind( wxEVT_BUTTON, &guVolumeFrame::DecVolButtonClick, this );

    Unbind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );
    m_IncVolButton->Unbind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );
    m_VolSlider->Unbind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );
    m_DecVolButton->Unbind( wxEVT_MOTION, &guVolumeFrame::OnMouse, this );

    Unbind( wxEVT_TIMER, &guVolumeFrame::OnTimer, this );

    if( m_MouseTimer )
        delete m_MouseTimer;
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::SetVolume( void )
{
    if( m_PlayerPanel )
        m_PlayerPanel->SetVolume( m_VolSlider->GetValue() );
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::VolFrameActivate( wxActivateEvent& event )
{
    if( !event.GetActive() )
      Close();
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::IncVolButtonClick( wxCommandEvent& event )
{
    m_VolSlider->SetValue( m_VolSlider->GetValue() + 5 );
    SetVolume();
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::VolSliderChanged( wxScrollEvent& event )
{
    SetVolume();
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::DecVolButtonClick( wxCommandEvent& event )
{
    m_VolSlider->SetValue( m_VolSlider->GetValue() - 5 );
    SetVolume();
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::OnMouseWheel( wxMouseEvent &event )
{
    if( m_MouseTimer->IsRunning() )
        m_MouseTimer->Stop();
    m_MouseTimer->Start( guVOLUMEN_AUTOCLOSE_TIMEOUT, wxTIMER_ONE_SHOT );

    int Rotation = event.GetWheelRotation() / event.GetWheelDelta();
    m_VolSlider->SetValue( m_VolSlider->GetValue() + ( Rotation * 2 ) );
    SetVolume();
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::OnMouse( wxMouseEvent &event )
{
    if( m_MouseTimer->IsRunning() )
        m_MouseTimer->Stop();
    m_MouseTimer->Start( guVOLUMEN_AUTOCLOSE_TIMEOUT, wxTIMER_ONE_SHOT );

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guVolumeFrame::OnTimer( wxTimerEvent &event )
{
    Close();
}

}

// -------------------------------------------------------------------------------- //
