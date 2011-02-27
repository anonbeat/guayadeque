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
#include "VolumeFrame.h"
//#include "Utils.h"

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

	// Connect Events
	Connect( wxEVT_ACTIVATE, wxActivateEventHandler( guVolumeFrame::VolFrameActivate ), NULL, this );
	Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guVolumeFrame::OnMouseWheel ), NULL, this );
	m_VolSlider->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guVolumeFrame::OnMouseWheel ), NULL, this );
	m_IncVolButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guVolumeFrame::IncVolButtonClick ), NULL, this );
	m_VolSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guVolumeFrame::VolSliderChanged ), NULL, this );
	m_VolSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guVolumeFrame::VolSliderChanged ), NULL, this );

	m_DecVolButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guVolumeFrame::DecVolButtonClick ), NULL, this );

    Connect( wxEVT_MOTION, wxMouseEventHandler( guVolumeFrame::OnMouse ), NULL, this );
    m_IncVolButton->Connect( wxEVT_MOTION, wxMouseEventHandler( guVolumeFrame::OnMouse ), NULL, this );
    m_VolSlider->Connect( wxEVT_MOTION, wxMouseEventHandler( guVolumeFrame::OnMouse ), NULL, this );
    m_DecVolButton->Connect( wxEVT_MOTION, wxMouseEventHandler( guVolumeFrame::OnMouse ), NULL, this );

    Connect( wxEVT_TIMER, wxTimerEventHandler( guVolumeFrame::OnTimer ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guVolumeFrame::~guVolumeFrame()
{
	// Disconnect Events
	Disconnect( wxEVT_ACTIVATE, wxActivateEventHandler( guVolumeFrame::VolFrameActivate ) );
	Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guVolumeFrame::OnMouseWheel ) );
	m_IncVolButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guVolumeFrame::IncVolButtonClick ), NULL, this );
	m_VolSlider->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guVolumeFrame::VolSliderChanged ), NULL, this );
	m_DecVolButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guVolumeFrame::DecVolButtonClick ), NULL, this );

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

// -------------------------------------------------------------------------------- //
