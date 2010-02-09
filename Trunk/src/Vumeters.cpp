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
wxSize guVumeter::DoGetBestSize() const
{
	return wxSize( 35, 10 );
}

// -------------------------------------------------------------------------------- //
void guVumeter::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
	wxPaintDC dc( this );
    wxRect Rect;
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );
	wxColour GOff = wxColour( 0, 128, 0 );
	wxColour OOff = wxColour( 191, 95, 0 );
	wxColour ROff = wxColour( 128, 0, 0 );
	wxColour GOn  = wxColour( 0, 255, 0 );
	wxColour OOn  = wxColour( 255, 128, 0 );
	wxColour ROn  = wxColour( 255, 0, 0 );

	int SizeG = ( Width * 40 ) / 100;
	//int SizeO = ( Width / 5 ) - 40;
	//int SizeR = Width - SizeG - SizeO;

    dc.SetPen( * wxTRANSPARENT_PEN );

    //guLogMessage( wxT( "Current Level: %i" ), m_CurrentLevel );
    if( !m_CurrentLevel )
    {
        dc.SetBrush( GOff );
        dc.DrawRectangle( 0, 0, SizeG, Height );
    }
    else if( m_CurrentLevel > 40 )
    {
        dc.SetBrush( GOn );
        dc.DrawRectangle( 0, 0, SizeG, Height );
    }
    else
    {
        dc.SetBrush( GOn );
        int LevelWidth = ( Width * m_CurrentLevel ) / 100;
        dc.DrawRectangle( 0, 0, LevelWidth, Height );
        dc.SetBrush( GOff );
        dc.DrawRectangle( LevelWidth, 0, SizeG, Height );
    }

    Rect.x = SizeG;
    Rect.width = ( Width / 5 );
    Rect.height = Height;
    Rect.y = 0;
    if( m_CurrentLevel < 40 )
    {
        dc.GradientFillLinear( Rect, GOff, OOff );
    }
    else if( m_CurrentLevel > 60 )
    {
        dc.GradientFillLinear( Rect, GOn, OOn );
    }
    else
    {
        dc.GradientFillLinear( Rect, GOff, OOff );
        wxRect ClipRect = Rect;
        ClipRect.width = ( Width * ( m_CurrentLevel - 40 ) ) / 100;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, GOn, OOn );
        dc.DestroyClippingRegion();
    }

    Rect.x = SizeG + Rect.width;
    if( m_CurrentLevel < 60 )
    {
        dc.GradientFillLinear( Rect, OOff, ROff );
    }
    else if( m_CurrentLevel > 80 )
    {
        dc.GradientFillLinear( Rect, OOn, ROn );
    }
    else
    {
        dc.GradientFillLinear( Rect, OOff, ROff );
        wxRect ClipRect = Rect;
        ClipRect.width = ( Width * ( m_CurrentLevel - 60 ) ) / 100;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, OOn, ROn );
        dc.DestroyClippingRegion();
    }

    if( m_CurrentLevel < 80 )
    {
        dc.SetBrush( ROff );
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ), 0, Width, Height );
    }
    else if( m_CurrentLevel == 100 )
    {
        dc.SetBrush( ROn );
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ), 0, Width, Height );
    }
    else
    {
        dc.SetBrush( ROn );
        int LevelWidth = ( Width * ( m_CurrentLevel - 80 ) ) / 100;
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ), 0, LevelWidth, Height );

        dc.SetBrush( ROff );
        dc.DrawRectangle( SizeG + ( Rect.width * 2 ) + LevelWidth, 0, Width, Height );
    }
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
    m_VumLeft->SetLevel( pow( 10, levels.m_Peak_L / 20 ) * 100 );
    m_VumRight->SetLevel( pow( 10, levels.m_Peak_R / 20 ) * 100 );
    //m_VumLeft->SetValue( left );
    //m_VumRight->SetValue( right );
}

// -------------------------------------------------------------------------------- //
