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
#ifndef SPLASHWIN_H
#define SPLASHWIN_H

#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/hyperlink.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/timer.h>

// -------------------------------------------------------------------------------- //
class guSplashFrame : public wxFrame
{
  private:

  protected:
	wxHyperlinkCtrl *   m_Email;
	wxStaticText *      m_Version;
    wxHyperlinkCtrl *   m_HomePage;
    wxHyperlinkCtrl *   m_Donate;
    wxBitmap *          m_Bitmap;
    wxTimer             m_Timer;


	// event handlers, overide them in your derived class
	void OnSplashClick( wxMouseEvent& event );
    void OnEraseBackground( wxEraseEvent &event );
    void DoPaint( wxDC &dc );
    void OnPaint( wxPaintEvent & );

  public:
	guSplashFrame( wxWindow * parent, int timeout = 4000 ); //, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,358 ), long style = 0|wxTAB_TRAVERSAL );
	~guSplashFrame();

    void OnCloseWindow( wxCloseEvent &event );
    void OnTimeout( wxTimerEvent &event );

};


#endif
// -------------------------------------------------------------------------------- //
