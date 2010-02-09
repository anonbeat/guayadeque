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
#ifndef VUMETERS_H
#define VUMETERS_H

#include "MediaCtrl.h"
#include "Utils.h"

#include <wx/control.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/panel.h>

// -------------------------------------------------------------------------------- //
// Class guVumeters
// -------------------------------------------------------------------------------- //
class guVumeter : public wxControl
{
  protected :
    int     m_CurrentLevel;

  public:
	guVumeter() {}
	guVumeter( wxWindow * parent, wxWindowID id )
		: wxControl( parent, id ) { m_CurrentLevel = 0; }

	wxSize DoGetBestSize() const;
	void OnPaint(wxPaintEvent& event);
    void SetLevel( const int level )
    {
        //guLogMessage( wxT( "Level: %i" ), level );
        if( m_CurrentLevel != level )
        {
            m_CurrentLevel = level;
            Refresh(); Update();
        }
    }

	DECLARE_EVENT_TABLE();
};


// -------------------------------------------------------------------------------- //
class guPlayerVumeters : public wxPanel
{
  protected:
	guVumeter * m_VumLeft;
	guVumeter * m_VumRight;

  public:
	guPlayerVumeters( wxWindow * parent );
	~guPlayerVumeters();

	void SetLevels( const guLevelInfo &level );

};

#endif
// -------------------------------------------------------------------------------- //
