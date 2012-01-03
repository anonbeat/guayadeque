// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
//	anonbeat@gmail.com
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
// guVumter
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
	m_OffBitmap     = NULL;
	m_OnBitmap      = NULL;
	m_LastHeight    = wxNOT_FOUND;
	m_LastWidth     = wxNOT_FOUND;

    Connect( wxEVT_SIZE, wxSizeEventHandler( guVumeter::OnChangedSize ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guVumeter::~guVumeter()
{
    wxMutexLocker Locker( m_BitmapMutex );
    if( m_OffBitmap )
    {
        delete m_OffBitmap;
        m_OffBitmap = NULL;
    }

    if( m_OnBitmap )
    {
        delete m_OnBitmap;
        m_OnBitmap = NULL;
    }

    Disconnect( wxEVT_SIZE, wxSizeEventHandler( guVumeter::OnChangedSize ), NULL, this );
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

//// -------------------------------------------------------------------------------- //
//double TestValues[] = {
//        -60.0,
//        -55.0,
//        -50.0,
//        -45.0,
//        -40.0,
//        -35.0,
//        -30.0,
//        -25.0,
//        -20.0,
//        -15.0,
//        -10.0,
//         -8.0,
//         -6.0,
//         -5.0,
//         -4.0,
//         -3.0,
//         -2.0,
//         -1.0,
//         -0.0
//};

// -------------------------------------------------------------------------------- //
void guVumeter::PaintHoriz( void )
{
	wxPaintDC dc( this );
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );
    //m_PeakLevel = TestValues[ wxGetUTCTime() % 19 ];
    //guLogMessage( wxT( "%0.2f -> %0.2f" ), m_PeakLevel, IEC_Scale( m_PeakLevel ) );
    int PeakLevel = IEC_Scale( m_PeakLevel ) * 100;
    int DecayLevel = IEC_Scale( m_DecayLevel ) * 100;
    //guLogMessage( wxT( "Peak: %i" ), PeakLevel );

    dc.SetPen( * wxTRANSPARENT_PEN );

    dc.DrawBitmap( * m_OffBitmap, 0, 0, false );

    wxRect ClipRect;
    ClipRect.y = 0;
    ClipRect.height = Height;
    ClipRect.x = 0;
    ClipRect.width = ( PeakLevel * Width ) / 100;
    dc.SetClippingRegion( ClipRect );
    dc.DrawBitmap( * m_OnBitmap, 0, 0, false );
    dc.DestroyClippingRegion();

    if( DecayLevel && ( DecayLevel > PeakLevel ) && ( DecayLevel < 100 ) )
    {
        ClipRect.x = ( DecayLevel * Width ) / 100;;
        ClipRect.width = 2;
        dc.SetClippingRegion( ClipRect );
        dc.DrawBitmap( * m_OnBitmap, 0, 0, false );
        dc.DestroyClippingRegion();
    }

    dc.SetPen( * wxBLACK_PEN );
    dc.DrawText( wxString::Format( wxT( "%02.1f" ), m_PeakLevel ), 2,  ( Height / 2 ) - 8 );
}

// -------------------------------------------------------------------------------- //
void guVumeter::PaintVert( void )
{
	wxPaintDC dc( this );
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );

    //guLogMessage( wxT( "%0.2f -> %0.2f" ), m_PeakLevel, IEC_Scale( m_PeakLevel ) );
    //m_PeakLevel = TestValues[ wxGetUTCTime() % 20 ];
    int PeakLevel = IEC_Scale( m_PeakLevel ) * 100;
    int DecayLevel = IEC_Scale( m_DecayLevel ) * 100;
    //guLogMessage( wxT( "Peak: %i" ), PeakLevel );

    dc.SetPen( * wxTRANSPARENT_PEN );

    dc.DrawBitmap( * m_OffBitmap, 0, 0, false );

    wxRect ClipRect;
    ClipRect.x = 0;
    ClipRect.width = Width;
    ClipRect.height = ( PeakLevel * Height ) / 100;
    ClipRect.y = Height - ClipRect.height;
    dc.SetClippingRegion( ClipRect );
    dc.DrawBitmap( * m_OnBitmap, 0, 0, false );
    dc.DestroyClippingRegion();

    if( DecayLevel && ( DecayLevel > PeakLevel ) && ( DecayLevel < 100 ) )
    {
        ClipRect.height = 2;
        ClipRect.y = Height - ( DecayLevel * Height ) / 100;
        dc.SetClippingRegion( ClipRect );
        dc.DrawBitmap( * m_OnBitmap, 0, 0, false );
        dc.DestroyClippingRegion();
    }

    dc.SetPen( * wxBLACK_PEN );
    dc.DrawRotatedText( wxString::Format( wxT( "%02.1f" ), m_PeakLevel ), ( Width / 2 ) - 7,  Height - 2, 90 );
}


// -------------------------------------------------------------------------------- //
void guVumeter::DrawHVumeter( wxBitmap * bitmap, int width, int height, wxColour &green, wxColour &orange, wxColour &red )
{
    if( bitmap && bitmap->IsOk() )
    {
        wxRect Rect;
        wxMemoryDC MemDC;
        MemDC.SelectObject( * bitmap );

        MemDC.SetPen( * wxTRANSPARENT_PEN );

        Rect.x = 0;
        Rect.y = 0;
        Rect.width = ( 84 * width ) / 100;
        Rect.height = height;

        MemDC.SetBrush( green );
        MemDC.DrawRectangle( 0, 0, Rect.width, height );

        Rect.x = Rect.width;
        Rect.width = ( 8 * width ) / 100;

        MemDC.GradientFillLinear( Rect, green, orange );

        Rect.x += Rect.width;
        Rect.width = width - Rect.x;

        MemDC.GradientFillLinear( Rect, orange, red );
    }
}

// -------------------------------------------------------------------------------- //
void guVumeter::DrawVVumeter( wxBitmap * bitmap, int width, int height, wxColour &green, wxColour &orange, wxColour &red )
{
    if( bitmap && bitmap->IsOk() )
    {
        wxRect Rect;
        wxMemoryDC MemDC;
        MemDC.SelectObject( * bitmap );

        MemDC.SetPen( * wxTRANSPARENT_PEN );

        Rect.x = 0;
        Rect.width = width;
        Rect.y = ( height * 16 ) / 100;
        Rect.height = height - Rect.y;

        MemDC.SetBrush( green );
        MemDC.DrawRectangle( 0, Rect.y, width, Rect.height );

        Rect.height = ( 8 * height ) / 100;
        Rect.y -= Rect.height;

        MemDC.GradientFillLinear( Rect, green, orange, wxNORTH );

        Rect.height = Rect.y;
        Rect.y = 0;

        MemDC.GradientFillLinear( Rect, orange, red, wxNORTH );
    }
}

// -------------------------------------------------------------------------------- //
void guVumeter::RefreshBitmaps( void )
{
    wxMutexLocker Locker( m_BitmapMutex );
    if( m_OnBitmap )
        delete m_OnBitmap;

    if( m_OffBitmap )
        delete m_OffBitmap;

	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );

	//guLogMessage( wxT( "RefreshBitmaps %i  %i " ), Width, Height );

    if( m_Style == guVU_HORIZONTAL )
    {
        m_OffBitmap = new wxBitmap( Width, Height, -1 );
        DrawHVumeter( m_OffBitmap, Width, Height, m_GreenOff, m_OrangeOff, m_RedOff );
        m_OnBitmap = new wxBitmap( Width, Height, -1 );
        DrawHVumeter( m_OnBitmap, Width, Height, m_GreenOn, m_OrangeOn, m_RedOn );
    }
    else
    {
        m_OffBitmap = new wxBitmap( Width, Height, -1 );
        DrawVVumeter( m_OffBitmap, Width, Height, m_GreenOff, m_OrangeOff, m_RedOff );
        m_OnBitmap = new wxBitmap( Width, Height, -1 );
        DrawVVumeter( m_OnBitmap, Width, Height, m_GreenOn, m_OrangeOn, m_RedOn );
    }
}

// -------------------------------------------------------------------------------- //
void guVumeter::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxMutexLocker Locker( m_BitmapMutex );
    if( m_Style == guVU_HORIZONTAL )
        PaintHoriz();
    else
        PaintVert();
}

// -------------------------------------------------------------------------------- //
void guVumeter::OnChangedSize( wxSizeEvent &event )
{
    wxSize Size = event.GetSize();

    if( ( Size.GetWidth() != m_LastWidth ) || ( Size.GetHeight() != m_LastHeight ) )
    {
        m_LastWidth = Size.GetWidth();
        m_LastHeight = Size.GetHeight();

        RefreshBitmaps();
    }
    event.Skip();
}





// -------------------------------------------------------------------------------- //
// guPlayerVumeters
// -------------------------------------------------------------------------------- //
guPlayerVumeters::guPlayerVumeters( wxWindow * parent ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_LastWidth = wxNOT_FOUND;
    m_LastHeight = wxNOT_FOUND;
	wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

	this->SetMinSize( wxSize( -1, 40 ) );

	m_VumMainSizer = new wxBoxSizer( wxVERTICAL );

    //
    // Create the Horizontal vumeter layout
    //
	m_HVumFlexSizer = new wxFlexGridSizer( 3, 2, 0, 0 );
	m_HVumFlexSizer->AddGrowableCol( 1 );
	m_HVumFlexSizer->AddGrowableRow( 0 );
	m_HVumFlexSizer->AddGrowableRow( 2 );
	m_HVumFlexSizer->SetFlexibleDirection( wxBOTH );
	m_HVumFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	CurrentFont.SetPointSize( 7 );

	wxStaticText * HVumLeftLabel = new wxStaticText( this, wxID_ANY, wxT("L:"), wxDefaultPosition, wxDefaultSize, 0 );
	HVumLeftLabel->Wrap( -1 );
	HVumLeftLabel->SetFont( CurrentFont );
	m_HVumFlexSizer->Add( HVumLeftLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

	m_HVumLeft = new guVumeter( this, wxID_ANY ); //, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
	m_HVumLeft->SetMinSize( wxSize( -1, 2 ) );
	m_HVumFlexSizer->Add( m_HVumLeft, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );

	wxBoxSizer * HLabelsSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * HDbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
	HDbLabel->Wrap( -1 );
	HDbLabel->SetFont( CurrentFont );
	m_HVumFlexSizer->Add( HDbLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxStaticText * HSixtyLevel = new wxStaticText( this, wxID_ANY, wxT("-60"), wxDefaultPosition, wxDefaultSize, 0 );
	HSixtyLevel->Wrap( -1 );
	HSixtyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HSixtyLevel, 13, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HFortyLevel = new wxStaticText( this, wxID_ANY, wxT("-40"), wxDefaultPosition, wxDefaultSize, 0 );
	HFortyLevel->Wrap( -1 );
	HFortyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HFortyLevel, 17, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HThirtyLevel = new wxStaticText( this, wxID_ANY, wxT("-30"), wxDefaultPosition, wxDefaultSize, 0 );
	HThirtyLevel->Wrap( -1 );
	HThirtyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HThirtyLevel, 23, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HTwentyLevel = new wxStaticText( this, wxID_ANY, wxT("-20"), wxDefaultPosition, wxDefaultSize, 0 );
	HTwentyLevel->Wrap( -1 );
	HTwentyLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HTwentyLevel, 27, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HTenLevel = new wxStaticText( this, wxID_ANY, wxT("-10"), wxDefaultPosition, wxDefaultSize, 0 );
	HTenLevel->Wrap( -1 );
	HTenLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HTenLevel, 13, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HOrangeLevel = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
	HOrangeLevel->Wrap( -1 );
	HOrangeLevel->SetFont( CurrentFont );
	HLabelsSizer->Add( HOrangeLevel, 7, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HRedLabel = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
	HRedLabel->Wrap( -1 );
	HRedLabel->SetFont( CurrentFont );
	HLabelsSizer->Add( HRedLabel, 7, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HClipLabel = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	HClipLabel->Wrap( -1 );
	HClipLabel->SetFont( CurrentFont );
	HLabelsSizer->Add( HClipLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	m_HVumFlexSizer->Add( HLabelsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * HVumRightLabel = new wxStaticText( this, wxID_ANY, wxT("R:"), wxDefaultPosition, wxDefaultSize, 0 );
	HVumRightLabel->Wrap( -1 );
	HVumRightLabel->SetFont( CurrentFont );
	m_HVumFlexSizer->Add( HVumRightLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5 );

	m_HVumRight = new guVumeter( this, wxID_ANY ); //, 100, wxDefaultPosition, wxSize( -1,-1 ), wxGA_HORIZONTAL );
	//m_HVumRight->SetValue( 0 );
	m_HVumRight->SetMinSize( wxSize( -1, 2 ) );

	m_HVumFlexSizer->Add( m_HVumRight, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );
	m_VumMainSizer->Add( m_HVumFlexSizer, 1, wxEXPAND, 5 );


    //
    // Create the Vertical vumeter layout
    //
	m_VVumFlexSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
	m_VVumFlexSizer->AddGrowableCol( 0 );
	m_VVumFlexSizer->AddGrowableCol( 2 );
	m_VVumFlexSizer->AddGrowableRow( 0 );
	m_VVumFlexSizer->SetFlexibleDirection( wxBOTH );
	m_VVumFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_VVumLeft = new guVumeter( this, wxID_ANY, guVU_VERTICAL );
	m_VVumFlexSizer->Add( m_VVumLeft, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxLEFT, 5 );


	wxBoxSizer * VLabelsSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticText * VClipLabel = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	VClipLabel->Wrap( -1 );
	VClipLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VClipLabel, 8, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	wxStaticText * VRedLabel = new wxStaticText( this, wxID_ANY, wxT("-3"), wxDefaultPosition, wxDefaultSize, 0 );
	VRedLabel->Wrap( -1 );
	VRedLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VRedLabel, 8, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxStaticText * VOrangeLabel = new wxStaticText( this, wxID_ANY, wxT("-6"), wxDefaultPosition, wxDefaultSize, 0 );
	VOrangeLabel->Wrap( -1 );
	VOrangeLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VOrangeLabel, 12, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxStaticText * VTenLabel = new wxStaticText( this, wxID_ANY, wxT("-10"), wxDefaultPosition, wxDefaultSize, 0 );
	VTenLabel->Wrap( -1 );
	VTenLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VTenLabel, 27, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxStaticText * VTwentyLabel = new wxStaticText( this, wxID_ANY, wxT("-20"), wxDefaultPosition, wxDefaultSize, 0 );
	VTwentyLabel->Wrap( -1 );
	VTwentyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VTwentyLabel, 23, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxStaticText * VThirtyLabel = new wxStaticText( this, wxID_ANY, wxT("-30"), wxDefaultPosition, wxDefaultSize, 0 );
	VThirtyLabel->Wrap( -1 );
	VThirtyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VThirtyLabel, 17, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxStaticText * VFortyLabel = new wxStaticText( this, wxID_ANY, wxT("-40"), wxDefaultPosition, wxDefaultSize, 0 );
	VFortyLabel->Wrap( -1 );
	VFortyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VFortyLabel, 15, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	wxStaticText * VSixtyLabel = new wxStaticText( this, wxID_ANY, wxT("-60"), wxDefaultPosition, wxDefaultSize, 0 );
	VSixtyLabel->Wrap( -1 );
	VSixtyLabel->SetFont( CurrentFont );
	VLabelsSizer->Add( VSixtyLabel, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_VVumFlexSizer->Add( VLabelsSizer, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );


	m_VVumRight = new guVumeter( this, wxID_ANY, guVU_VERTICAL );
	m_VVumFlexSizer->Add( m_VVumRight, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT, 5 );


	wxStaticText * VVumLeftLabel = new wxStaticText( this, wxID_ANY, wxT("L:"), wxDefaultPosition, wxDefaultSize, 0 );
	VVumLeftLabel->Wrap( -1 );
	VVumLeftLabel->SetFont( CurrentFont );
	m_VVumFlexSizer->Add( VVumLeftLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxLEFT, 5 );

	wxStaticText * VDbLabel = new wxStaticText( this, wxID_ANY, wxT("db"), wxDefaultPosition, wxDefaultSize, 0 );
	VDbLabel->Wrap( -1 );
	VDbLabel->SetFont( CurrentFont );
	m_VVumFlexSizer->Add( VDbLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * VVumRightLabel = new wxStaticText( this, wxID_ANY, wxT("R:"), wxDefaultPosition, wxDefaultSize, 0 );
	VVumRightLabel->Wrap( -1 );
	VVumRightLabel->SetFont( CurrentFont );
	m_VVumFlexSizer->Add( VVumRightLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 5 );

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
    wxSize Size = event.GetSize();

    if( ( Size.GetWidth() != m_LastWidth ) || ( Size.GetHeight() != m_LastHeight ) )
    {
        bool ChangedOrientation = false;

        m_LastWidth = Size.GetWidth();
        m_LastHeight = Size.GetHeight();

        //guLogMessage( wxT( "%i, %i" ), Size.GetWidth(), Size.GetHeight() );
        if( m_LastWidth >= m_LastHeight )
        {
            if( m_VumMainSizer->IsShown( m_VVumFlexSizer ) )
            {
                m_VumMainSizer->Hide( m_VVumFlexSizer );
                m_VumMainSizer->Show( m_HVumFlexSizer );
                m_VumLeft = m_HVumLeft;
                m_VumRight = m_HVumRight;
                ChangedOrientation = true;
            }
        }
        else if( m_VumMainSizer->IsShown( m_HVumFlexSizer ) )
        {
            m_VumMainSizer->Hide( m_HVumFlexSizer );
            m_VumMainSizer->Show( m_VVumFlexSizer );
            m_VumLeft = m_VVumLeft;
            m_VumRight = m_VVumRight;
            ChangedOrientation = true;
        }

        if( ChangedOrientation )
        {
            m_VumMainSizer->Layout();
        }
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
