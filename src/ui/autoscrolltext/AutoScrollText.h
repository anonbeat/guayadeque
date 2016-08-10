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
#ifndef __AUTOSCROLLTEXT_H__
#define __AUTOSCROLLTEXT_H__

#include <wx/control.h>
#include <wx/string.h>
#include <wx/timer.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guAutoScrollText : public wxControl
{
  protected :
    wxString        m_Label;
    wxSize          m_LabelExtent;
    int             m_VisWidth;
    bool            m_AllowScroll;
    wxTimer         m_StartTimer;
    wxTimer         m_ScrollTimer;
    int             m_ScrollPos;
    int             m_ScrollQuantum;
    wxSize          m_DefaultSize;


    virtual wxSize  DoGetBestSize() const;
    void            OnPaint( wxPaintEvent &event );
    void            OnMouseEvents( wxMouseEvent &event );
    void            CalcTextExtent( void );
    void            OnSize( wxSizeEvent &event );
    void            OnScrollTimer( wxTimerEvent &event );
    void            OnStartTimer( wxTimerEvent &event );

  public :
    guAutoScrollText( wxWindow * parent, const wxString &label, const wxSize &size = wxDefaultSize );
    ~guAutoScrollText();

    void SetLabel( const wxString &label );

  private :

  DECLARE_EVENT_TABLE()

};

}

#endif
