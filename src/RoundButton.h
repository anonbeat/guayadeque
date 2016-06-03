// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef ROUNDBUTTON_H
#define ROUNDBUTTON_H

#include <wx/control.h>
#include <wx/bitmap.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guRoundButton : public wxControl
{
  protected :
    wxBitmap            m_Bitmap;
    wxBitmap            m_HoverBitmap;
    wxBitmap            m_DisBitmap;
    wxRegion            m_Region;
    bool                m_MouseIsOver;
    bool                m_IsClicked;

    DECLARE_EVENT_TABLE()

  protected :
    virtual wxSize      DoGetBestSize() const;
    virtual void        OnPaint( wxPaintEvent &event );
    virtual void        OnMouseEvents( wxMouseEvent &event );

    void                CreateRegion( void );

public :
    guRoundButton( wxWindow * parent, const wxImage &image, const wxImage &selimage );
    virtual ~guRoundButton();

    virtual void        SetBitmapLabel( const wxImage &image );
    virtual void        SetBitmapHover( const wxImage &image );
    virtual void        SetBitmapDisabled( const wxImage &image );

};

}

#endif
// -------------------------------------------------------------------------------- //
