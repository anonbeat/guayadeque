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

#define guVU_HORIZONTAL     0
#define guVU_VERTICAL       1

// -------------------------------------------------------------------------------- //
// Class guVumeters
// -------------------------------------------------------------------------------- //
class guVumeter : public wxControl
{
  protected :
    int                 m_Style;
    double              m_PeakLevel;
    double              m_DecayLevel;
    wxColour            m_RedOn;
    wxColour            m_RedOff;
    wxColour            m_GreenOn;
    wxColour            m_GreenOff;
    wxColour            m_OrangeOn;
    wxColour            m_OrangeOff;

    wxBitmap *          m_OnBitmap;
    wxBitmap *          m_OffBitmap;
    wxMutex             m_BitmapMutex;

	int                 m_LastWidth;
	int                 m_LastHeight;

    void                PaintHoriz( void );
    void                PaintVert( void );

    void                DrawHVumeter( wxBitmap * bitmap, int width, int height, wxColour &green, wxColour &orange, wxColour &red );
    void                DrawVVumeter( wxBitmap * bitmap, int width, int height, wxColour &green, wxColour &orange, wxColour &red );
	void                RefreshBitmaps( void );

	void                OnChangedSize( wxSizeEvent &event );

  public:
	guVumeter() {}
	guVumeter( wxWindow * parent, wxWindowID id, const int style = guVU_HORIZONTAL );
	~guVumeter();

	wxSize              DoGetBestSize() const;
	void                OnPaint( wxPaintEvent& event );

    void                SetLevel( const double peak, const double decay )
    {
        if( ( m_PeakLevel != peak ) || ( m_DecayLevel != decay ) )
        {
            m_PeakLevel = peak;
            m_DecayLevel = decay;
            Refresh(); Update();
        }
    }

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

	int                 m_LastWidth;
	int                 m_LastHeight;

	void                OnChangedSize( wxSizeEvent &event );

  public:
	guPlayerVumeters( wxWindow * parent );
	~guPlayerVumeters();

	void                SetLevels( const guLevelInfo &level );

};

#endif
// -------------------------------------------------------------------------------- //
