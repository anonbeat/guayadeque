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
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <wx/wx.h>

// -------------------------------------------------------------------------------- //
class guGauge : public wxWindow
{
    private :

      wxGauge *         m_Gauge;
      wxStaticText *    m_StaticText;
    public :
      guGauge( wxWindow * parent );
      ~guGauge( void );
      void SetText( const wxString text );
      void SetValue( int value );
      void SetTotal( int total );
      void Pulse( void ) { m_Gauge->Pulse(); };

};
WX_DEFINE_ARRAY_PTR( guGauge *, guGaugeArray );

// -------------------------------------------------------------------------------- //
class guStatusBar : public wxStatusBar
{
  private:
    guGaugeArray        m_Gauges;
    wxStaticBitmap *    m_ASBitmap;

    void                OnSize( wxSizeEvent &event );
    void                SetSizes( int fieldcnt );
    void                UpdateGauges( void );

  public:
                        guStatusBar( wxWindow * parent );
    virtual             ~guStatusBar();

    void                SetAudioScrobbleService( bool Enabled = false );
    int                 AddGauge( void );
    int                 RemoveGauge( int gaugeid );
    void                Pulse( int id ) { m_Gauges[ id ]->Pulse(); };
    void                SetTotal( int id, int total ) { m_Gauges[ id ]->SetTotal( total ); };
    void                SetValue( int id, int value ) { m_Gauges[ id ]->SetValue( value ); };

};


#endif
// -------------------------------------------------------------------------------- //
