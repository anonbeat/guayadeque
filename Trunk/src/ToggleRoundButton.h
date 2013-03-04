// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2013 J.Rios
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
#ifndef TOGGLEROUNDBUTTON_H
#define TOGGLEROUNDBUTTON_H

#include <wx/control.h>
#include <wx/bitmap.h>
#include <wx/tglbtn.h>

// -------------------------------------------------------------------------------- //
class guToggleRoundButton : public wxControl
{
  private :
    wxBitmap    m_Bitmap;
    wxBitmap    m_HoverBitmap;
    wxBitmap    m_DisBitmap;
    wxRegion    m_Region;
    bool        m_MouseIsOver;
    bool        m_IsClicked;
    bool        m_Value;

    DECLARE_EVENT_TABLE()

  protected :
    virtual wxSize  DoGetBestSize() const;
    void            OnPaint( wxPaintEvent &event );
    void            OnMouseEvents( wxMouseEvent &event );

  public :
    guToggleRoundButton( wxWindow * parent, const wxImage &image, const wxImage &selimage, const wxImage &hoverimage );
    ~guToggleRoundButton();

    void            SetBitmapLabel( const wxImage &image );
    void            SetBitmapHover( const wxImage &image );
    void            SetBitmapDisabled( const wxImage &image );

    bool            GetValue( void ) { return m_Value; };
    void            SetValue( bool value )
    {
        if( value != m_Value )
        {
            m_Value = value;
            Refresh();
        }
    }

};


#endif
// -------------------------------------------------------------------------------- //

