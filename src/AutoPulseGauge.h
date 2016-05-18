// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
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
#ifndef GAUGE_H
#define GAUGE_H

#include <wx/gauge.h>
#include <wx/timer.h>

class guGaugeTimer;

// -------------------------------------------------------------------------------- //
class guAutoPulseGauge : public wxGauge
{
  private :
    guGaugeTimer * m_Timer;

  public :
    guAutoPulseGauge( wxWindow * parent, wxWindowID id, int range, const wxPoint &pos = wxDefaultPosition,
             const wxSize &size = wxDefaultSize, long style = wxGA_HORIZONTAL,
             const wxValidator& validator = wxDefaultValidator, const wxString &name = wxT( "gauge" ) );

    ~guAutoPulseGauge();

    void StartPulse( void );
    void StopPulse( int range, int value );
    bool IsPulsing( void );
};

// -------------------------------------------------------------------------------- //
class guGaugeTimer : public wxTimer
{
  private :
    guAutoPulseGauge * m_Gauge;

  public:
    guGaugeTimer( guAutoPulseGauge * gauge ) { m_Gauge = gauge; };

    void Notify() { m_Gauge->Pulse(); };
};


#endif
// -------------------------------------------------------------------------------- //

