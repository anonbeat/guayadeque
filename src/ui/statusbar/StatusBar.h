// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <wx/wx.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guGauge : public wxControl
{
  protected :
    int         m_Value;
    int         m_LastValue;
    int         m_PaintWidth;
    int         m_Range;
    float       m_Factor;
    bool        m_ShowPorcent;
    wxString    m_Label;
    wxFont      m_Font;
    wxColour    m_GradStart;
    wxColour    m_GradEnd;
    wxColour    m_FgColor1;
    wxColour    m_FgColor2;

    void OnPaint( wxPaintEvent &event );


  public :
    guGauge() : wxControl() { m_LastValue = wxNOT_FOUND; m_PaintWidth = 0; }
    guGauge( wxWindow * parent, const wxString &label = wxEmptyString, bool showporcent = true, wxWindowID id = wxID_ANY, unsigned int max = 100,
               const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxGA_HORIZONTAL | wxBORDER_NONE );
    ~guGauge( void );

    void    SetRange( int range );
    int     GetValue( void ) { return m_Value; }
    bool    SetValue( int value );

  DECLARE_EVENT_TABLE()
};
WX_DEFINE_ARRAY_PTR( guGauge *, guGaugeArray );

// -------------------------------------------------------------------------------- //
class guStatusBar : public wxStatusBar
{
  private:
    guGaugeArray        m_Gauges;
    wxStaticBitmap *    m_ASBitmap;
    wxStaticText *      m_SelInfo;

    int                 m_LastClickAction;
    wxTimer             m_ClickTimer;

    void                OnSize( wxSizeEvent &event );
    void                SetSizes( int fieldcnt );
    void                UpdateGauges( void );

    void                OnAudioScrobbleClicked( void );
    void                OnAudioScrobbleDClicked( void );

    void                OnButtonClick( wxMouseEvent &event );
    void                OnButtonDClick( wxMouseEvent &event );
    void                OnTimerEvent( wxTimerEvent &event );

  public:
                        guStatusBar( wxWindow * parent );
    virtual             ~guStatusBar();

    void                UpdateAudioScrobbleIcon( bool Enabled = false );
    int                 AddGauge( const wxString &text = wxEmptyString, bool showporcent = true );
    int                 RemoveGauge( int gaugeid );
    void                Pulse( int id ) { /*m_Gauges[ id ]->Pulse(); */ }
    void                SetTotal( int id, int total ) { m_Gauges[ id ]->SetRange( total ); }
    void                SetValue( int id, int value ) { m_Gauges[ id ]->SetValue( value ); }

    void                SetSelInfo( const wxString &label );

    void                SetAudioScrobble( const bool audioscrobble );

    virtual void        DrawField( wxDC &dc, int i, int textHeight );

};

}

#endif
// -------------------------------------------------------------------------------- //
