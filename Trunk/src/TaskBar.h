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
#ifndef TASKBAR_H
#define TASKBAR_H

#include <wx/taskbar.h>
#include "MainFrame.h"
//#include "PlayerPanel.h"

// ---------------------------------------------------------------------- //
class guTaskBarIcon : public wxTaskBarIcon
{
  protected :
    guMainFrame *   m_MainFrame;
    guPlayerPanel * m_PlayerPanel;

    virtual wxMenu * CreatePopupMenu();

    void SendEventToMainFrame( wxCommandEvent &event );
    void OnClick( wxTaskBarIconEvent &event );
    void SetRatingEvent( wxCommandEvent &event );

  public :
    guTaskBarIcon( guMainFrame * NewMainFrame, guPlayerPanel * NewPlayerPanel );
    ~guTaskBarIcon();

};

#endif
// ---------------------------------------------------------------------- //
