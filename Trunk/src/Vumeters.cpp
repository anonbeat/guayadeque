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
#include "Vumeters.h"

#include "Utils.h"

// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE( guVumeter, wxControl )
	EVT_PAINT( guVumeter::OnPaint )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guVumeter::guVumeter( wxWindow * parent, wxWindowID id ) : wxControl( parent, id )
{
    m_PeakLevel     = -INFINITY;
    m_DecayLevel    = -INFINITY;
	m_GreenOff      = wxColour( 0, 128, 0 );
	m_OrangeOff     = wxColour( 191, 95, 0 );
	m_RedOff        = wxColour( 128, 0, 0 );
	m_GreenOn       = wxColour( 0, 255, 0 );
	m_OrangeOn      = wxColour( 255, 128, 0 );
	m_RedOn         = wxColour( 255, 0, 0 );
}

// -------------------------------------------------------------------------------- //
wxSize guVumeter::DoGetBestSize() const
{
	return wxSize( 20, 2 );
}

// -------------------------------------------------------------------------------- //
void guVumeter::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
	wxPaintDC dc( this );
    wxRect Rect;
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );
    int PeakLevel = pow( 10, m_PeakLevel / 20 ) * 100;
    int DecayLevel = pow( 10, m_DecayLevel / 20 ) * 100;

	int SizeG = ( Width * 40 ) / 100;
	//int SizeO = ( Width / 5 ) - 40;
	//int SizeR = Width - SizeG - SizeO;

    dc.SetPen( * wxTRANSPARENT_PEN );

    //guLogMessage( wxT( "Current Level: %i" ), PeakLevel );
    if( !PeakLevel )
    {
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( 0, 0, SizeG, Height );
    }
    else if( PeakLevel > 40 )
    {
        dc.SetBrush( m_GreenOn );
        dc.DrawRectangle( 0, 0, SizeG, Height );
    }
    else
    {
        dc.SetBrush( m_GreenOn );
        int LevelWidth = ( Width * PeakLevel ) / 100;
        dc.DrawRectangle( 0, 0, LevelWidth, Height );
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( LevelWidth, 0, SizeG, Height );
    }

    Rect.x = SizeG;
    Rect.width = ( Width / 5 );
    Rect.height = Height;
    Rect.y = 0;
    if( PeakLevel < 40 )
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff );
    }
    else if( PeakLevel > 60 )
    {
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff );
        wxRect ClipRect = Rect;
        ClipRect.width = ( Width * ( PeakLevel - 40 ) ) / 100;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn );
        dc.DestroyClippingRegion();
    }

    Rect.x = SizeG + Rect.width;
    if( PeakLevel < 60 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff );
    }
    else if( PeakLevel > 80 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff );
        wxRect ClipRect = Rect;
        ClipRect.width = ( Width * ( PeakLevel - 60 ) ) / 100;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn );
        dc.DestroyClippingRegion();
    }

    if( PeakLevel < 80 )
    {
        dc.SetBrush( m_RedOff );
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ), 0, Width, Height );
    }
    else if( PeakLevel == 100 )
    {
        dc.SetBrush( m_RedOn );
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ), 0, Width, Height );
    }
    else
    {
        dc.SetBrush( m_RedOn );
        int LevelWidth = ( Width * ( PeakLevel - 80 ) ) / 100;
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ), 0, LevelWidth, Height );

        dc.SetBrush( m_RedOff );
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ) + LevelWidth, 0, Width, Height );
    }

    // Draw the decay level
    if( DecayLevel && ( DecayLevel > PeakLevel ) )
    {
        wxRect ClipRect = Rect;
        ClipRect.width = 2;
        ClipRect.x = ( Width * DecayLevel ) / 100;
        dc.SetClippingRegion( ClipRect );

        if( DecayLevel < 40 )       // at Solid green
        {
            dc.SetBrush( m_GreenOn );
            dc.DrawRectangle( 0, 0, SizeG, Height );
        }
        else if( DecayLevel < 60 )  // at green to orange
        {
            Rect.x = SizeG;
            dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn );
        }
        else if( DecayLevel < 80 )  // at orange to red
        {
            Rect.x = SizeG + Rect.width;
            dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn );
        }
        else                        // at solid red
        {
            dc.SetBrush( m_RedOn );
            dc.DrawRectangle( SizeG + ( Rect.width * 2 ), 0, Width, Height );
        }
        dc.DestroyClippingRegion();
    }

    dc.SetPen( * wxBLACK_PEN );
    dc.DrawText( wxString::Format( wxT( "%02.0f" ), m_PeakLevel ), 2,  ( Height / 2 ) - 9 );
    //guLogMessage( wxT( "%f" ), m_PeakLevel );
}


// -------------------------------------------------------------------------------- //
guPlayerVumeters::guPlayerVumeters( wxWindow * parent ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 300, 60 ), wxTAB_TRAVERSAL )
{
	this->SetMinSize( wxSize( -1, 40 ) );

	wxBoxSizer * VumMainSizer;
	VumMainSizer = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer * VumFlexSizer;
	VumFlexSizer = new wxFlexGridSizer( 3, 2, 0, 0 );
	VumFlexSizer->AddGrowableCol( 1 );
	VumFlexSizer->AddGrowableRow( 1 );
	VumFlexSizer->AddGrowableRow( 2 );
	VumFlexSizer->SetFlexibleDirection( wxBOTH );
	VumFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	VumFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer* LabelsSizer;
	LabelsSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * DbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
	DbLabel->Wrap( -1 );
	DbLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	LabelsSizer->Add( DbLabel, 5, wxALIGN_BOTTOM, 5 );

	wxStaticText * OrangeLabel = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
	OrangeLabel->Wrap( -1 );
	OrangeLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	LabelsSizer->Add( OrangeLabel, 2, wxALIGN_BOTTOM, 5 );

	wxStaticText * RedLabel = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
	RedLabel->Wrap( -1 );
	RedLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	LabelsSizer->Add( RedLabel, 3, wxALIGN_BOTTOM, 5 );

	wxStaticText * ClipLabel = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	ClipLabel->Wrap( -1 );
	ClipLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	LabelsSizer->Add( ClipLabel, 0, wxRIGHT|wxALIGN_BOTTOM, 5 );

	VumFlexSizer->Add( LabelsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * VumLeftLabel = new wxStaticText( this, wxID_ANY, wxT("L:"), wxDefaultPosition, wxDefaultSize, 0 );
	VumLeftLabel->Wrap( -1 );
	VumFlexSizer->Add( VumLeftLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT, 5 );

	m_VumLeft = new guVumeter( this, wxID_ANY ); //, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
	//m_VumLeft->SetValue( 0 );
	m_VumLeft->SetMinSize( wxSize( -1, 2 ) );

	VumFlexSizer->Add( m_VumLeft, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * VumRightLabel = new wxStaticText( this, wxID_ANY, wxT("R:"), wxDefaultPosition, wxDefaultSize, 0 );
	VumRightLabel->Wrap( -1 );
	VumFlexSizer->Add( VumRightLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT, 5 );

	m_VumRight = new guVumeter( this, wxID_ANY ); //, 100, wxDefaultPosition, wxSize( -1,-1 ), wxGA_HORIZONTAL );
	//m_VumRight->SetValue( 0 );
	m_VumRight->SetMinSize( wxSize( -1, 2 ) );

	VumFlexSizer->Add( m_VumRight, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	VumMainSizer->Add( VumFlexSizer, 1, wxEXPAND, 5 );

	this->SetSizer( VumMainSizer );
	this->Layout();
}

// -------------------------------------------------------------------------------- //
guPlayerVumeters::~guPlayerVumeters()
{
}

// -------------------------------------------------------------------------------- //
void guPlayerVumeters::SetLevels( const guLevelInfo &levels )
{
    m_VumLeft->SetLevel( levels.m_Peak_L, levels.m_Decay_L );
    m_VumRight->SetLevel( levels.m_Peak_R, levels.m_Decay_R );
    //m_VumLeft->SetValue( left );
    //m_VumRight->SetValue( right );
}

// -------------------------------------------------------------------------------- //
