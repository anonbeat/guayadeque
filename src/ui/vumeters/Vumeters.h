// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#ifndef __VUMETERS_H__
#define __VUMETERS_H__

#include "LevelInfo.h"
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

namespace Guayadeque {

#define guVU_HORIZONTAL     0
#define guVU_VERTICAL       1

// -------------------------------------------------------------------------------- //
class guVumeterColour
{
  public :
    wxColour    m_Off;
    wxColour    m_Peak;
    wxColour    m_Rms;

    guVumeterColour() {}
    guVumeterColour( wxColour off, wxColour peak, wxColour rms ) {
        m_Off = off; m_Peak = peak; m_Rms = rms;
    }
    virtual ~guVumeterColour() {}

};

// -------------------------------------------------------------------------------- //
// Class guVumeters
// -------------------------------------------------------------------------------- //
class guVumeter : public wxControl
{
  protected :
    int                 m_Style;
    double              m_PeakLevel;
    double              m_RmsLevel;
    double              m_DecayLevel;
    guVumeterColour     m_Green;
    guVumeterColour     m_Orange;
    guVumeterColour     m_Red;
    wxBitmap *          m_OffBitmap;
    wxBitmap *          m_PeakBitmap;
    wxBitmap *          m_RmsBitmap;
    wxMutex             m_BitmapMutex;

	int                 m_LastWidth;
	int                 m_LastHeight;

    void                PaintHoriz( void );
    void                PaintVert( void );

    void                DrawHVumeter( wxBitmap * bitmap, int width, int height, wxColour &green, wxColour &orange, wxColour &red );
    void                DrawVVumeter( wxBitmap * bitmap, int width, int height, wxColour &green, wxColour &orange, wxColour &red );
	void                RefreshBitmaps( void );

	void                OnChangedSize( wxSizeEvent &event );

    void                OnLevelTimeout( wxTimerEvent &event );

  public:
	guVumeter() {}
	guVumeter( wxWindow * parent, wxWindowID id, const int style = guVU_HORIZONTAL );
	~guVumeter();

	wxSize              DoGetBestSize() const;
	void                OnPaint( wxPaintEvent& event );

    void                SetLevel( const double peak, const double rms, const double decay );

    double              DecayLevel( void ) { return m_DecayLevel; }
    double              RmsLevel( void ) { return m_RmsLevel; }
    double              PeakLevel( void ) { return m_PeakLevel; }

	DECLARE_EVENT_TABLE();

    friend class guPlayerVumeters;
};


// -------------------------------------------------------------------------------- //
class guPlayerVumeters : public wxPanel
{
  protected:
	guVumeter *         m_VumLeft;
	guVumeter *         m_VumRight;

    wxBoxSizer *        m_VumMainSizer;
    wxFlexGridSizer *   m_HVumFlexSizer;
    wxFlexGridSizer *   m_VVumFlexSizer;
	guVumeter *         m_HVumLeft;
	guVumeter *         m_HVumRight;
	guVumeter *         m_VVumLeft;
	guVumeter *         m_VVumRight;

    wxTimer             m_LevelsTimer;

	int                 m_LastWidth;
	int                 m_LastHeight;

	void                OnChangedSize( wxSizeEvent &event );
    void                OnLevelsTimeout( wxTimerEvent &event );

  public:
	guPlayerVumeters( wxWindow * parent );
	~guPlayerVumeters();

	void                SetLevels( const guLevelInfo &level );

};

}

#endif
// -------------------------------------------------------------------------------- //
