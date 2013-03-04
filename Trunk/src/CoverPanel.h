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
#ifndef COVERPANEL_H
#define COVERPANEL_H

#include "PlayerPanel.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>

// -------------------------------------------------------------------------------- //
class guCoverPanel : public wxPanel
{
  protected :
    guPlayerPanel *     m_PlayerPanel;
    int                 m_LastSize;
    wxBitmap            m_CoverImage;
    int                 m_CoverType;
    wxString            m_CoverPath;
    wxMutex             m_CoverImageMutex;
    wxTimer             m_ResizeTimer;

    virtual void        OnSize( wxSizeEvent &event );
    virtual void        OnPaint( wxPaintEvent &event );
    virtual void        OnResizeTimer( wxTimerEvent &event );

    void                UpdateImage( void );

  public :
    guCoverPanel( wxWindow * parent, guPlayerPanel * playerpanel );
    ~guCoverPanel();

    void                OnUpdatedTrack( wxCommandEvent &event );

};

#endif
// -------------------------------------------------------------------------------- //
