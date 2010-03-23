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
#ifndef STATICBITMAP_H
#define STATICBITMAP_H

#include <wx/wx.h>

extern const wxEventType guStaticBitmapMouseOverEvent;
#define guEVT_STATICBITMAP_MOUSE_OVER           1000

class guStaticBitmapTimer;

// -------------------------------------------------------------------------------- //
class guStaticBitmap : public wxStaticBitmap
{
  public :
    guStaticBitmap( wxWindow * parent, wxWindowID id, const wxBitmap &label, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0 );
    ~guStaticBitmap();

  protected :
    guStaticBitmapTimer *   m_MouseOverTimer;

    void OnMouse( wxMouseEvent &event );

};

// -------------------------------------------------------------------------------- //
class guStaticBitmapTimer : public wxTimer
{
  protected :
    guStaticBitmap * m_Bitmap;

  public :
    guStaticBitmapTimer( guStaticBitmap * bitmap ) { m_Bitmap = bitmap; };

    void Notify();
};

#endif
// -------------------------------------------------------------------------------- //
