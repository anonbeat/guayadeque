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
#ifndef AUTOSCROLLTEXT_H
#define AUTOSCROLLTEXT_H

#include <wx/control.h>
#include <wx/string.h>
#include <wx/timer.h>

// -------------------------------------------------------------------------------- //
class guAutoScrollText : public wxControl
{
  protected :
    wxString        m_Label;
    wxSize          m_LabelExtent;
    int             m_VisWidth;
    bool            m_AllowScroll;
    wxTimer         m_ScrollTimer;
    wxTimer         m_StartTimer;
    int             m_ScrollPos;
    int             m_ScrollQuantum;


    virtual wxSize  DoGetBestSize() const;
    void            OnPaint( wxPaintEvent &event );
    void            OnMouseEvents( wxMouseEvent &event );
    void            CalcTextExtent( void );
    void            OnSize( wxSizeEvent &event );
    void            OnTimer( wxTimerEvent &event );

  public :
    guAutoScrollText( wxWindow * parent, const wxString &label );
    ~guAutoScrollText();

    void SetLabel( const wxString &label );

  private :

  DECLARE_EVENT_TABLE()

};

#endif
