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
guVumeter::guVumeter( wxWindow * parent, wxWindowID id, const int style ) : wxControl( parent, id )
{
    m_Style         = style;
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
    if( m_Style == guVU_HORIZONTAL )
        return wxSize( 20, 4 );
    else
        return wxSize( 4, 20 );
}

// -------------------------------------------------------------------------------- //
void guVumeter::PaintHoriz( void )
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
    dc.DrawText( wxString::Format( wxT( "%02.0f" ), m_PeakLevel ), 2,  ( Height / 2 ) - 8 );
    //guLogMessage( wxT( "%f" ), m_PeakLevel );
}

// -------------------------------------------------------------------------------- //
void guVumeter::PaintVert( void )
{
	wxPaintDC dc( this );
    wxRect Rect;
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );

    int PeakLevel = pow( 10, m_PeakLevel / 20 ) * 100;
    int DecayLevel = pow( 10, m_DecayLevel / 20 ) * 100;

	int SizeG = ( Height * 40 ) / 100;
	//int SizeO = ( Width / 5 ) - 40;
	//int SizeR = Width - SizeG - SizeO;

    dc.SetPen( * wxTRANSPARENT_PEN );

    //guLogMessage( wxT( "Current Level: %i" ), PeakLevel );
    if( !PeakLevel )
    {
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( 0, Height - SizeG, Width, Height );
    }
    else if( PeakLevel > 40 )
    {
        dc.SetBrush( m_GreenOn );
        dc.DrawRectangle( 0, Height - SizeG, Width, Height );
    }
    else
    {
        int LevelHeight = ( Height * PeakLevel ) / 100;
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( 0, Height - SizeG, Width, Height - LevelHeight );
        dc.SetBrush( m_GreenOn );
        dc.DrawRectangle( 0, Height - LevelHeight, Width, Height );
    }

    Rect.x = 0;
    Rect.width = Width;
    Rect.y = SizeG;
    Rect.height = ( Height - SizeG ) - Rect.y;
    if( PeakLevel < 40 )
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff, wxNORTH );
    }
    else if( PeakLevel > 60 )
    {
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn, wxNORTH );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff, wxNORTH );
        wxRect ClipRect = Rect;
        ClipRect.height = ( Height * ( PeakLevel - 40 ) ) / 100;
        ClipRect.y = Rect.y + Rect.height - ClipRect.height;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn, wxNORTH );
        dc.DestroyClippingRegion();
    }

    Rect.y = Rect.y - ( SizeG / 2 );
    Rect.height = ( SizeG / 2 );
    if( PeakLevel < 60 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff, wxNORTH );
    }
    else if( PeakLevel > 80 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn, wxNORTH );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff, wxNORTH );
        wxRect ClipRect = Rect;
        ClipRect.height = ( Height * ( PeakLevel - 60 ) ) / 100;
        ClipRect.y = Rect.y + Rect.height - ClipRect.height;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn, wxNORTH );
        dc.DestroyClippingRegion();
    }

    if( PeakLevel < 80 )
    {
        dc.SetBrush( m_RedOff );
        dc.DrawRectangle( 0, 0, Width, Rect.y );
    }
    else if( PeakLevel == 100 )
    {
        dc.SetBrush( m_RedOn );
        dc.DrawRectangle( 0, 0, Width, Rect.y );
    }
    else
    {
        dc.SetBrush( m_RedOn );
        int LevelHeight = ( Height * ( PeakLevel - 80 ) ) / 100;
        dc.DrawRectangle( 0, Rect.y - LevelHeight, Width, LevelHeight );

        dc.SetBrush( m_RedOff );
        dc.DrawRectangle( 0, 0, Width, Rect.y - LevelHeight );
    }

    // Draw the decay level
    if( DecayLevel && ( DecayLevel > PeakLevel ) )
    {
        wxRect ClipRect = Rect;
        ClipRect.height = 2;
        ClipRect.y = Height - ( ( Height * DecayLevel ) / 100 );
        dc.SetClippingRegion( ClipRect );

        if( DecayLevel < 40 )       // at Solid green
        {
            dc.SetBrush( m_GreenOn );
            dc.DrawRectangle( 0, Height - SizeG, Width, Height );
        }
        else if( DecayLevel < 60 )  // at green to orange
        {
            Rect.y = SizeG;
            Rect.height = ( Height - SizeG ) - Rect.y;
            dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn, wxNORTH );
        }
        else if( DecayLevel < 80 )  // at orange to red
        {
            //Rect.y = SizeG + Rect.width;
            dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn, wxNORTH );
        }
        else                        // at solid red
        {
            dc.SetBrush( m_RedOn );
            dc.DrawRectangle( 0, 0, Width, Rect.y );
        }
        dc.DestroyClippingRegion();
    }

    dc.SetPen( * wxBLACK_PEN );
    dc.DrawRotatedText( wxString::Format( wxT( "%02.0f" ), m_PeakLevel ), ( Width / 2 ) - 7,  Height - 2, 90 );
    //guLogMessage( wxT( "%f" ), m_PeakLevel );
}


// -------------------------------------------------------------------------------- //
void guVumeter::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    if( m_Style == guVU_HORIZONTAL )
        PaintHoriz();
    else
        PaintVert();
}


// -------------------------------------------------------------------------------- //
guPlayerVumeters::guPlayerVumeters( wxWindow * parent ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
	this->SetMinSize( wxSize( -1, 40 ) );

	m_VumMainSizer = new wxBoxSizer( wxVERTICAL );

    //
    // Create the Horizontal vumeter layout
    //
	m_HVumFlexSizer = new wxFlexGridSizer( 3, 2, 0, 0 );
	m_HVumFlexSizer->AddGrowableCol( 1 );
	m_HVumFlexSizer->AddGrowableRow( 1 );
	m_HVumFlexSizer->AddGrowableRow( 2 );
	m_HVumFlexSizer->SetFlexibleDirection( wxBOTH );
	m_HVumFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_HVumFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer * HLabelsSizer;
	HLabelsSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * HDbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
	HDbLabel->Wrap( -1 );
	HDbLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	HLabelsSizer->Add( HDbLabel, 5, wxALIGN_BOTTOM, 5 );

	wxStaticText * HOrangeLevel = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
	HOrangeLevel->Wrap( -1 );
	HOrangeLevel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	HLabelsSizer->Add( HOrangeLevel, 2, wxALIGN_BOTTOM, 5 );

	wxStaticText * HRedLabel = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
	HRedLabel->Wrap( -1 );
	HRedLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	HLabelsSizer->Add( HRedLabel, 3, wxALIGN_BOTTOM, 5 );

	wxStaticText * HClipLabel = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	HClipLabel->Wrap( -1 );
	HClipLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	HLabelsSizer->Add( HClipLabel, 0, wxRIGHT|wxALIGN_BOTTOM, 5 );

	m_HVumFlexSizer->Add( HLabelsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HVumLeftLabel = new wxStaticText( this, wxID_ANY, wxT("L:"), wxDefaultPosition, wxDefaultSize, 0 );
	HVumLeftLabel->Wrap( -1 );
	m_HVumFlexSizer->Add( HVumLeftLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT, 5 );

	m_HVumLeft = new guVumeter( this, wxID_ANY ); //, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
	//m_VumLeft->SetValue( 0 );
	m_HVumLeft->SetMinSize( wxSize( -1, 2 ) );

	m_HVumFlexSizer->Add( m_HVumLeft, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * HVumRightLabel = new wxStaticText( this, wxID_ANY, wxT("R:"), wxDefaultPosition, wxDefaultSize, 0 );
	HVumRightLabel->Wrap( -1 );
	m_HVumFlexSizer->Add( HVumRightLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT, 5 );

	m_HVumRight = new guVumeter( this, wxID_ANY ); //, 100, wxDefaultPosition, wxSize( -1,-1 ), wxGA_HORIZONTAL );
	//m_HVumRight->SetValue( 0 );
	m_HVumRight->SetMinSize( wxSize( -1, 2 ) );

	m_HVumFlexSizer->Add( m_HVumRight, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	m_VumMainSizer->Add( m_HVumFlexSizer, 1, wxEXPAND, 5 );


    //
    // Create the Vertical vumeter layout
    //
	m_VVumFlexSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
	m_VVumFlexSizer->AddGrowableCol( 1 );
	m_VVumFlexSizer->AddGrowableCol( 2 );
	m_VVumFlexSizer->AddGrowableRow( 0 );
	m_VVumFlexSizer->SetFlexibleDirection( wxBOTH );
	m_VVumFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxBoxSizer* VLabelsSizer;
	VLabelsSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticText * VClipLabel = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	VClipLabel->Wrap( -1 );
	VClipLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	VLabelsSizer->Add( VClipLabel, 3, wxALIGN_BOTTOM|wxALIGN_RIGHT|wxTOP, 5 );

	wxStaticText * VRedLabel = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
	VRedLabel->Wrap( -1 );
	VRedLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	VLabelsSizer->Add( VRedLabel, 2, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VOrangeLabel = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
	VOrangeLabel->Wrap( -1 );
	VOrangeLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	VLabelsSizer->Add( VOrangeLabel, 5, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VDbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
	VDbLabel->Wrap( -1 );
	VDbLabel->SetFont( wxFont( 8, 74, 90, 90, false, wxT("Sans") ) );

	VLabelsSizer->Add( VDbLabel, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_VVumFlexSizer->Add( VLabelsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	m_VVumLeft = new guVumeter( this, wxID_ANY, guVU_VERTICAL );
	//m_VVumLeft->SetValue( 72 );
	//m_VVumLeft->SetMinSize( wxSize( 2, -1 ) );

	m_VVumFlexSizer->Add( m_VVumLeft, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	m_VVumRight = new guVumeter( this, wxID_ANY, guVU_VERTICAL );
	//m_VVumRight->SetValue( 76 );
	//m_VVumRight->SetMinSize( wxSize( 2, -1 ) );

	m_VVumFlexSizer->Add( m_VVumRight, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );


	m_VVumFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText * VVumLeftLabel = new wxStaticText( this, wxID_ANY, wxT("L:"), wxDefaultPosition, wxDefaultSize, 0 );
	VVumLeftLabel->Wrap( -1 );
	m_VVumFlexSizer->Add( VVumLeftLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxLEFT, 5 );

	wxStaticText * VVumRightLabel = new wxStaticText( this, wxID_ANY, wxT("R:"), wxDefaultPosition, wxDefaultSize, 0 );
	VVumRightLabel->Wrap( -1 );
	m_VVumFlexSizer->Add( VVumRightLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxLEFT, 5 );

	m_VumMainSizer->Add( m_VVumFlexSizer, 1, wxEXPAND, 5 );


    m_VumMainSizer->Hide( m_VVumFlexSizer, false );

	this->SetSizer( m_VumMainSizer );
	this->Layout();


    Connect( wxEVT_SIZE, wxSizeEventHandler( guPlayerVumeters::OnChangedSize ), NULL, this );

    m_VumLeft = m_HVumLeft;
    m_VumRight = m_HVumRight;

}

// -------------------------------------------------------------------------------- //
guPlayerVumeters::~guPlayerVumeters()
{
    Disconnect( wxEVT_SIZE, wxSizeEventHandler( guPlayerVumeters::OnChangedSize ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPlayerVumeters::OnChangedSize( wxSizeEvent &event )
{
    bool Changed = false;
    wxSize Size = event.GetSize();
    //guLogMessage( wxT( "%i, %i" ), Size.GetWidth(), Size.GetHeight() );
    if( Size.GetWidth() >= Size.GetHeight() )
    {
        if( m_VumMainSizer->IsShown( m_VVumFlexSizer ) )
        {
            m_VumMainSizer->Hide( m_VVumFlexSizer );
            m_VumMainSizer->Show( m_HVumFlexSizer );
            m_VumLeft = m_HVumLeft;
            m_VumRight = m_HVumRight;
            Changed = true;
        }
    }
    else if( m_VumMainSizer->IsShown( m_HVumFlexSizer ) )
    {
        m_VumMainSizer->Hide( m_HVumFlexSizer );
        m_VumMainSizer->Show( m_VVumFlexSizer );
        m_VumLeft = m_VVumLeft;
        m_VumRight = m_VVumRight;
        Changed = true;
    }

    if( Changed )
    {
        m_VumMainSizer->Layout();
        //m_VumMainSizer->FitInside( this );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayerVumeters::SetLevels( const guLevelInfo &levels )
{
    m_VumLeft->SetLevel( levels.m_Peak_L, levels.m_Decay_L );
    m_VumRight->SetLevel( levels.m_Peak_R, levels.m_Decay_R );
}

// -------------------------------------------------------------------------------- //
