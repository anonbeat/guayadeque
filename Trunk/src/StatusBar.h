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
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <wx/wx.h>

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
    guGauge() : wxControl() { m_LastValue = wxNOT_FOUND; m_PaintWidth = 0; };
    guGauge( wxWindow * parent, const wxString &label = wxEmptyString, bool showporcent = true, wxWindowID id = wxID_ANY, unsigned int max = 100,
               const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxGA_HORIZONTAL );
    ~guGauge( void );

    void    SetRange( int range );
    int     GetValue( void ) { return m_Value; };
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
    wxStaticBitmap *    m_PlayMode;
    wxStaticText *      m_SelInfo;
    bool                m_ForceGapless;

    int                 m_LastClickAction;
    wxTimer             m_ClickTimer;

    void                OnSize( wxSizeEvent &event );
    void                SetSizes( int fieldcnt );
    void                UpdateGauges( void );

    void                OnAudioScrobbleClicked( void );
    void                OnAudioScrobbleDClicked( void );
    void                OnPlayModeClicked( void );
    void                OnPlayModeDClicked( void );

    void                OnButtonClick( wxMouseEvent &event );
    void                OnButtonDClick( wxMouseEvent &event );
    void                OnTimerEvent( wxTimerEvent &event );

    void                OnConfigUpdated( wxCommandEvent &event );

  public:
                        guStatusBar( wxWindow * parent );
    virtual             ~guStatusBar();

    void                UpdateAudioScrobbleIcon( bool Enabled = false );
    int                 AddGauge( const wxString &text = wxEmptyString, bool showporcent = true );
    int                 RemoveGauge( int gaugeid );
    void                Pulse( int id ) { /*m_Gauges[ id ]->Pulse(); */ };
    void                SetTotal( int id, int total ) { m_Gauges[ id ]->SetRange( total ); };
    void                SetValue( int id, int value ) { m_Gauges[ id ]->SetValue( value ); };

    void                SetSelInfo( const wxString &label );

    void                SetPlayMode( const bool forcegapless );
    void                SetAudioScrobble( const bool audioscrobble );

};


#endif
// -------------------------------------------------------------------------------- //
