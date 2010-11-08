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
	m_GreenOff      = wxColour(   0, 128,   0 );
	m_GreenOn       = wxColour(   0, 255,   0 );
	m_OrangeOff     = wxColour( 191,  95,   0 );
	m_OrangeOn      = wxColour( 255, 128,   0 );
	m_RedOff        = wxColour( 128,   0,   0 );
	m_RedOn         = wxColour( 255,   0,   0 );
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
static inline float IEC_Scale( float db )
{
	float fScale = 1.0f;

	if( db < -70.0f )
		fScale = 0.0f;
	else if( db < -60.0f )
		fScale = ( db + 70.0f ) * 0.0025f;
	else if( db < -50.0f )
		fScale = ( db + 60.0f ) * 0.005f + 0.025f;
	else if( db < -40.0 )
		fScale = ( db + 50.0f ) * 0.0075f + 0.075f;
	else if( db < -30.0f )
		fScale = ( db + 40.0f ) * 0.015f + 0.15f;
	else if( db < -20.0f )
		fScale = ( db + 30.0f ) * 0.02f + 0.3f;
	else if( db < -0.001f || db > 0.001f )
		fScale = ( db + 20.0f ) * 0.025f + 0.5f;

	return fScale;
}

// -------------------------------------------------------------------------------- //
double TestValues[] = {
        -60.0,
        -55.0,
        -50.0,
        -45.0,
        -40.0,
        -35.0,
        -30.0,
        -25.0,
        -20.0,
        -15.0,
        -10.0,
         -8.0,
         -6.0,
         -5.0,
         -4.0,
         -3.0,
         -2.0,
         -1.0,
         -0.0
};

// -------------------------------------------------------------------------------- //
void guVumeter::PaintHoriz( void )
{
	wxPaintDC dc( this );
    wxRect Rect;
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );
    //m_PeakLevel = TestValues[ wxGetUTCTime() % 19 ];
    //guLogMessage( wxT( "%0.2f -> %0.2f" ), m_PeakLevel, IEC_Scale( m_PeakLevel ) );
    int PeakLevel = IEC_Scale( m_PeakLevel ) * 100;
    int DecayLevel = IEC_Scale( m_DecayLevel ) * 100;
    //guLogMessage( wxT( "Peak: %i" ), PeakLevel );

    dc.SetPen( * wxTRANSPARENT_PEN );

    Rect.x = 0;
    Rect.y = 0;
    Rect.width = ( 84 * Width ) / 100;
    Rect.height = Height;

    if( !PeakLevel )
    {
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( 0, 0, Rect.width, Height );
    }
    else if( PeakLevel > 84 )
    {
        dc.SetBrush( m_GreenOn );
        dc.DrawRectangle( 0, 0, Rect.width, Height );
    }
    else
    {
        dc.SetBrush( m_GreenOn );
        int LevelWidth = PeakLevel * Width / 100;
        dc.DrawRectangle( 0, 0, LevelWidth, Height );
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( LevelWidth, 0, Rect.width - LevelWidth, Height );
    }

    Rect.x = Rect.width;
    Rect.width = ( 8 * Width ) / 100;

    if( PeakLevel < 84 )
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff );
    }
    else if( PeakLevel > 92 )
    {
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff );
        wxRect ClipRect = Rect;
        ClipRect.width = ( PeakLevel - 84 ) * Rect.width / 7;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn );
        dc.DestroyClippingRegion();
    }

    Rect.x += Rect.width;
    Rect.width = Width - Rect.x;

    if( PeakLevel < 92 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff );
    }
    else if( PeakLevel > 99 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff );
        wxRect ClipRect = Rect;
        ClipRect.width = ( PeakLevel - 92 ) * Rect.width / 8;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn );
        dc.DestroyClippingRegion();
    }

    //
    // Draw the decay level
    if( DecayLevel && ( DecayLevel > PeakLevel ) && ( DecayLevel < 100 ) )
    {
        wxRect ClipRect = Rect;
        ClipRect.width = 2;
        ClipRect.x = ( DecayLevel * Width ) / 100;
        dc.SetClippingRegion( ClipRect );
        Rect.width = ( 84 * Width ) / 100;
        Rect.x = 0;

        if( DecayLevel < 84 )       // at Solid green
        {
            dc.SetBrush( m_GreenOn );
            dc.DrawRectangle( 0, 0, Rect.width, Height );
        }
        else if( DecayLevel < 92 )  // at green to orange
        {
            Rect.x += Rect.width;
            Rect.width = ( 8 * Width ) / 100;
            dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn );
        }
        else                        // at solid red
        {
            Rect.x += ( 92 * Width ) / 100;
            Rect.width = Width - Rect.x;
            dc.SetBrush( m_RedOn );
            dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn );
        }
        dc.DestroyClippingRegion();
    }

    dc.SetPen( * wxBLACK_PEN );
    dc.DrawText( wxString::Format( wxT( "%02.1f" ), m_PeakLevel ), 2,  ( Height / 2 ) - 8 );
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

    //guLogMessage( wxT( "%0.2f -> %0.2f" ), m_PeakLevel, IEC_Scale( m_PeakLevel ) );
    //m_PeakLevel = TestValues[ wxGetUTCTime() % 20 ];
    int PeakLevel = IEC_Scale( m_PeakLevel ) * 100;
    int DecayLevel = IEC_Scale( m_DecayLevel ) * 100;
    //guLogMessage( wxT( "Peak: %i" ), PeakLevel );

    dc.SetPen( * wxTRANSPARENT_PEN );

    Rect.x = 0;
    Rect.width = Width;
    Rect.y = ( Height * 16 ) / 100;
    Rect.height = Height - Rect.y;

    if( !PeakLevel )
    {
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( 0, Rect.y, Width, Rect.height );
    }
    else if( PeakLevel > 84 )
    {
        dc.SetBrush( m_GreenOn );
        dc.DrawRectangle( 0, Rect.y, Width, Rect.height );
    }
    else
    {
        int LevelHeight = ( PeakLevel * Height ) / 100;
        dc.SetBrush( m_GreenOff );
        dc.DrawRectangle( 0, Rect.y, Width, Rect.height - LevelHeight );
        dc.SetBrush( m_GreenOn );
        dc.DrawRectangle( 0, Height - LevelHeight, Width, LevelHeight );
    }

    Rect.height = ( 8 * Height ) / 100;
    Rect.y -= Rect.height;

    if( PeakLevel < 84 )
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff, wxNORTH );
    }
    else if( PeakLevel > 92 )
    {
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn, wxNORTH );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_GreenOff, m_OrangeOff, wxNORTH );
        wxRect ClipRect = Rect;
        int Increment = ( ( 92 - PeakLevel ) * Height ) / 100;
        ClipRect.y += Increment;
        ClipRect.height = ( Rect.y + Rect.height ) - Increment;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn, wxNORTH );
        dc.DestroyClippingRegion();
    }


    Rect.height = Rect.y;
    Rect.y = 0;

    if( PeakLevel < 92 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff, wxNORTH );
    }
    else if( PeakLevel > 99 )
    {
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn, wxNORTH );
    }
    else
    {
        dc.GradientFillLinear( Rect, m_OrangeOff, m_RedOff, wxNORTH );
        wxRect ClipRect = Rect;
        int Increment = ( ( 100 - PeakLevel ) * Height ) / 100;
        ClipRect.y += Increment;
        ClipRect.height -= Increment;
        dc.SetClippingRegion( ClipRect );
        dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn, wxNORTH );
        dc.DestroyClippingRegion();
    }

    // Draw the decay level
    if( DecayLevel && ( DecayLevel > PeakLevel ) && ( DecayLevel < 99 ) )
    {
        Rect.y = ( Height * 16 ) / 100;
        Rect.height = Height - Rect.y;

        wxRect ClipRect = Rect;
        ClipRect.height = 2;
        ClipRect.y = Height - ( ( DecayLevel * Height ) / 100 );
        dc.SetClippingRegion( ClipRect );

        if( DecayLevel < 84 )       // at Solid green
        {
            dc.SetBrush( m_GreenOn );
            dc.DrawRectangle( 0, Rect.y, Width, Rect.height );
        }
        else if( DecayLevel < 92 )  // at green to orange
        {
            Rect.height = ( 8 * Height ) / 100;
            Rect.y -= Rect.height;
            dc.GradientFillLinear( Rect, m_GreenOn, m_OrangeOn, wxNORTH );
        }
        else                       // at orange to red
        {
            Rect.height = Rect.y - ( 8 * Height ) / 100;
            Rect.y = 0;
            dc.GradientFillLinear( Rect, m_OrangeOn, m_RedOn, wxNORTH );
        }
        dc.DestroyClippingRegion();
    }

    dc.SetPen( * wxBLACK_PEN );
    dc.DrawRotatedText( wxString::Format( wxT( "%02.1f" ), m_PeakLevel ), ( Width / 2 ) - 7,  Height - 2, 90 );
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
	wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

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

	CurrentFont.SetPointSize( 8 );

	wxBoxSizer * HLabelsSizer;
	HLabelsSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * HDbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
	HDbLabel->Wrap( -1 );
	HDbLabel->SetFont( CurrentFont );
	m_HVumFlexSizer->Add( HDbLabel, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );
	//m_HVumFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );


	//CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );

//	wxStaticText * HDbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
//	HDbLabel->Wrap( -1 );
//	HDbLabel->SetFont( CurrentFont );
//	HLabelsSizer->Add( HDbLabel, 5, wxALIGN_BOTTOM, 5 );

	wxStaticText * HSixtyLevel = new wxStaticText( this, wxID_ANY, wxT("-60"), wxDefaultPosition, wxDefaultSize, 0 );
	HSixtyLevel->Wrap( -1 );
	HSixtyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HSixtyLevel, 13, wxALIGN_BOTTOM, 5 );

	wxStaticText * HFortyLevel = new wxStaticText( this, wxID_ANY, wxT("-40"), wxDefaultPosition, wxDefaultSize, 0 );
	HFortyLevel->Wrap( -1 );
	HFortyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HFortyLevel, 17, wxALIGN_BOTTOM, 5 );

	wxStaticText * HThirtyLevel = new wxStaticText( this, wxID_ANY, wxT("-30"), wxDefaultPosition, wxDefaultSize, 0 );
	HThirtyLevel->Wrap( -1 );
	HThirtyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HThirtyLevel, 23, wxALIGN_BOTTOM, 5 );

	wxStaticText * HTwentyLevel = new wxStaticText( this, wxID_ANY, wxT("-20"), wxDefaultPosition, wxDefaultSize, 0 );
	HTwentyLevel->Wrap( -1 );
	HTwentyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HTwentyLevel, 27, wxALIGN_BOTTOM, 5 );

	wxStaticText * HTenLevel = new wxStaticText( this, wxID_ANY, wxT("-10"), wxDefaultPosition, wxDefaultSize, 0 );
	HTenLevel->Wrap( -1 );
	HTenLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HTenLevel, 13, wxALIGN_BOTTOM, 5 );

	wxStaticText * HOrangeLevel = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
	HOrangeLevel->Wrap( -1 );
	HOrangeLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HOrangeLevel, 7, wxALIGN_BOTTOM, 5 );

	wxStaticText * HRedLabel = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
	HRedLabel->Wrap( -1 );
	HRedLabel->SetFont( CurrentFont );
	HLabelsSizer->Add( HRedLabel, 7, wxALIGN_BOTTOM, 5 );

	wxStaticText * HClipLabel = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	HClipLabel->Wrap( -1 );
	HClipLabel->SetFont( CurrentFont );
	HLabelsSizer->Add( HClipLabel, 0, wxRIGHT|wxALIGN_BOTTOM, 5 );

	m_HVumFlexSizer->Add( HLabelsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HVumLeftLabel = new wxStaticText( this, wxID_ANY, wxT("L:"), wxDefaultPosition, wxDefaultSize, 0 );
	HVumLeftLabel->Wrap( -1 );
	HVumLeftLabel->SetFont( CurrentFont );
	m_HVumFlexSizer->Add( HVumLeftLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT, 5 );

	m_HVumLeft = new guVumeter( this, wxID_ANY ); //, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
	//m_VumLeft->SetValue( 0 );
	m_HVumLeft->SetMinSize( wxSize( -1, 2 ) );

	m_HVumFlexSizer->Add( m_HVumLeft, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * HVumRightLabel = new wxStaticText( this, wxID_ANY, wxT("R:"), wxDefaultPosition, wxDefaultSize, 0 );
	HVumRightLabel->Wrap( -1 );
	HVumRightLabel->SetFont( CurrentFont );
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
	VClipLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VClipLabel, 8, wxALIGN_BOTTOM|wxALIGN_RIGHT|wxTOP, 5 );

	wxStaticText * VRedLabel = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
	VRedLabel->Wrap( -1 );
	VRedLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VRedLabel, 8, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VOrangeLabel = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
	VOrangeLabel->Wrap( -1 );
	VOrangeLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VOrangeLabel, 12, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VTenLabel = new wxStaticText( this, wxID_ANY, wxT("-10"), wxDefaultPosition, wxDefaultSize, 0 );
	VTenLabel->Wrap( -1 );
	VTenLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VTenLabel, 27, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VTwentyLabel = new wxStaticText( this, wxID_ANY, wxT("-20"), wxDefaultPosition, wxDefaultSize, 0 );
	VTwentyLabel->Wrap( -1 );
	VTwentyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VTwentyLabel, 23, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VThirtyLabel = new wxStaticText( this, wxID_ANY, wxT("-30"), wxDefaultPosition, wxDefaultSize, 0 );
	VThirtyLabel->Wrap( -1 );
	VThirtyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VThirtyLabel, 17, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VFortyLabel = new wxStaticText( this, wxID_ANY, wxT("-40"), wxDefaultPosition, wxDefaultSize, 0 );
	VFortyLabel->Wrap( -1 );
	VFortyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VFortyLabel, 15, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	wxStaticText * VSixtyLabel = new wxStaticText( this, wxID_ANY, wxT("-60"), wxDefaultPosition, wxDefaultSize, 0 );
	VSixtyLabel->Wrap( -1 );
	VSixtyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VSixtyLabel, 0, wxALIGN_BOTTOM|wxALIGN_RIGHT, 5 );

	m_VVumFlexSizer->Add( VLabelsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	m_VVumLeft = new guVumeter( this, wxID_ANY, guVU_VERTICAL );
	//m_VVumLeft->SetValue( 72 );
	//m_VVumLeft->SetMinSize( wxSize( 2, -1 ) );

	m_VVumFlexSizer->Add( m_VVumLeft, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	m_VVumRight = new guVumeter( this, wxID_ANY, guVU_VERTICAL );
	//m_VVumRight->SetValue( 76 );
	//m_VVumRight->SetMinSize( wxSize( 2, -1 ) );

	m_VVumFlexSizer->Add( m_VVumRight, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );


	wxStaticText * VDbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
	VDbLabel->Wrap( -1 );
	VDbLabel->SetFont( CurrentFont );
	m_VVumFlexSizer->Add( VDbLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * VVumLeftLabel = new wxStaticText( this, wxID_ANY, wxT("L:"), wxDefaultPosition, wxDefaultSize, 0 );
	VVumLeftLabel->Wrap( -1 );
	VVumLeftLabel->SetFont( CurrentFont );
	m_VVumFlexSizer->Add( VVumLeftLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxLEFT, 5 );

	wxStaticText * VVumRightLabel = new wxStaticText( this, wxID_ANY, wxT("R:"), wxDefaultPosition, wxDefaultSize, 0 );
	VVumRightLabel->Wrap( -1 );
	VVumRightLabel->SetFont( CurrentFont );
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
    if( levels.m_Channels > 1 )
        m_VumRight->SetLevel( levels.m_Peak_R, levels.m_Decay_R );
    else
        m_VumRight->SetLevel( levels.m_Peak_L, levels.m_Decay_L );
}

// -------------------------------------------------------------------------------- //
