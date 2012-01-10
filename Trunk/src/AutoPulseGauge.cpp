// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
#include "AutoPulseGauge.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guAutoPulseGauge::guAutoPulseGauge( wxWindow * parent, wxWindowID id, int range, const wxPoint &pos,
             const wxSize &size, long style, const wxValidator& validator, const wxString &name ) :
    wxGauge( parent, id, range, pos, size, style, validator, name )
{
    m_Timer = new guGaugeTimer( this );
    m_Timer->Start( 300 );
}

// -------------------------------------------------------------------------------- //
guAutoPulseGauge::~guAutoPulseGauge( void )
{
    if( m_Timer )
        delete m_Timer;
}

// -------------------------------------------------------------------------------- //
void guAutoPulseGauge::StopPulse( int range, int value )
{
    m_Timer->Stop();
    SetRange( range );
    SetValue( value );
}

// -------------------------------------------------------------------------------- //
void guAutoPulseGauge::StartPulse( void )
{
    m_Timer->Start( 300 );
}

// -------------------------------------------------------------------------------- //
bool guAutoPulseGauge::IsPulsing( void )
{
    return m_Timer->IsRunning();
};
