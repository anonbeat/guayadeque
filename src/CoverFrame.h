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
#ifndef COVERFRAME_H
#define COVERFRAME_H

#include <wx/wx.h>
#include "PlayerPanel.h"

// -------------------------------------------------------------------------------- //
class guCoverFrame : public wxFrame
{
  protected:
    wxStaticBitmap * m_CoverBitmap;
    bool             m_CapturedMouse;
    wxTimer *        m_AutoCloseTimer;

    void CoverFrameActivate( wxActivateEvent &event );
    void OnClick( wxMouseEvent &event );
    void OnCaptureLost( wxMouseCaptureLostEvent &event );
    void OnMouse( wxMouseEvent &event );
    void OnTimer( wxTimerEvent &event );

  public:
    guCoverFrame( wxWindow * parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 293, 258 ), long style = 0|wxTAB_TRAVERSAL );
    ~guCoverFrame();
    void SetBitmap( const guSongCoverType CoverType, const wxString &CoverPath = wxEmptyString );

};

#endif
// -------------------------------------------------------------------------------- //


