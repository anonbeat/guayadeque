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
#include "StatusBar.h"
#include "Images.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guGauge::guGauge( wxWindow * parent ) :
         wxWindow( parent, wxID_ANY, wxDefaultPosition, wxSize( -1, -1 ) )
{
	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_StaticText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticText->Wrap( -1 );
	MainSizer->Add( m_StaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_Gauge = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxSize( 150, 15 ), wxGA_HORIZONTAL );
	MainSizer->Add( m_Gauge, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	SetSizer( MainSizer );
	Layout();
	MainSizer->Fit( this );

	//Gauge->Pulse();
}

// -------------------------------------------------------------------------------- //
guGauge::~guGauge()
{
}

// -------------------------------------------------------------------------------- //
void guGauge::SetValue( int value )
{
    if( m_Gauge )
    {
        m_Gauge->SetValue( value );
        if( m_StaticText )
            m_StaticText->SetLabel( wxString::Format( wxT( "%u/%u" ), m_Gauge->GetValue(), m_Gauge->GetRange() ) );
        Layout();
    }
}

// -------------------------------------------------------------------------------- //
void guGauge::SetTotal( int total )
{
    if( m_Gauge )
    {
        m_Gauge->SetRange( total );
        SetValue( 0 );
    }
}

// -------------------------------------------------------------------------------- //
guStatusBar::guStatusBar( wxWindow * parent ) : wxStatusBar( parent, wxID_ANY )
{
    int FieldWidths[] = { -1, 50 };
    SetFieldsCount( 2 );
    SetStatusWidths( 2, FieldWidths );
    m_ASBitmap = new wxStaticBitmap( this, wxID_ANY, wxBitmap( guImage_lastfm_as_off ) );
    m_ASBitmap->SetToolTip( _( "Shows the status of the LastFM connection." ) );
    Connect( wxEVT_SIZE, wxSizeEventHandler( guStatusBar::OnSize ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guStatusBar::~guStatusBar()
{
    if( m_ASBitmap )
        delete m_ASBitmap;

    Disconnect( wxEVT_SIZE, wxSizeEventHandler( guStatusBar::OnSize ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnSize( wxSizeEvent &event )
{
    wxRect rect;

    UpdateGauges();
    //
    if( m_ASBitmap )
    {
        //size = ASBitmap->GetSize();
        GetFieldRect( GetFieldsCount() - 1, rect );
        m_ASBitmap->Move( rect.x + 3,
                        rect.y + 3 );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guStatusBar::SetAudioScrobbleService( bool Enabled )
{
    if( m_ASBitmap )
    {
        m_ASBitmap->SetBitmap( wxBitmap( Enabled ? guImage_lastfm_as_on : guImage_lastfm_as_off ) );
        m_ASBitmap->Refresh();
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::SetSizes( int FieldCnt )
{
    int index;
    int * FieldWidths = ( int * ) malloc( sizeof( int ) * FieldCnt );
    if( FieldWidths )
    {
        for( index = 0; index < FieldCnt; index++ )
        {
            if( index == 0 )
                FieldWidths[ index ] = -1;
            else if( index == ( FieldCnt - 1 ) )
                FieldWidths[ index ] = 50;
            else
                FieldWidths[ index ] = 200;
            //printf( "Width: %i\n", FieldWidths[ index ] );
        }
        //SetStatusWidths( FieldCnt, FieldWidths );
        SetFieldsCount( FieldCnt, FieldWidths );
        free( FieldWidths );

        //Refresh();
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::UpdateGauges( void )
{
    int     index;
    int     count;
    wxRect  rect;
    wxSize  size;
    int     PanelIndex = 0;
    // Update the pos for all the gauges in the statusbar
    count = m_Gauges.Count();
    for( index = 0; index < count; index++ )
    {
        if( m_Gauges[ index ] )
        {
            // The first Panel is the info with index 0, the first gauge will be 1
            PanelIndex++;
            GetFieldRect( PanelIndex, rect );
            size.x = rect.width - 2;
            size.y = rect.height - 2;
            m_Gauges[ index ]->Move( rect.x + 1, rect.y + 1 );
            m_Gauges[ index ]->SetSize( size );
        }
    }
    Refresh();
}

// -------------------------------------------------------------------------------- //
int guStatusBar::AddGauge( void )
{
    m_Gauges.Add( new guGauge( this ) );
    SetSizes( GetFieldsCount() + 1 );

    UpdateGauges();

    return ( m_Gauges.Count() - 1 );
}

// -------------------------------------------------------------------------------- //
int guStatusBar::RemoveGauge( int gaugeid )
{

    SetSizes( GetFieldsCount() - 1 );
    RemoveChild( m_Gauges[ gaugeid ] );
    delete m_Gauges[ gaugeid ];
    m_Gauges[ gaugeid ] = NULL;
    //Gauges.RemoveAt( gaugeid );
//    if( gaugeid == ( Gauges.Count() - 1 )
//    {
//        Gauges.RemoveAt( gaugeid );
//    }

    // We remove all empty gauges from the array from the end
    // so the valid index dont get changed when remove a lower gauge index
    while( m_Gauges.Count() && ( m_Gauges[ m_Gauges.Count() - 1 ] == NULL ) )
    {
        m_Gauges.RemoveAt( m_Gauges.Count() - 1 );
    }

    UpdateGauges();

    return m_Gauges.Count();
}

// -------------------------------------------------------------------------------- //
