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
#ifndef EQUALIZER_H
#define EQUALIZER_H

#include "MediaCtrl.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/combobox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/dialog.h>
#include <wx/dynarray.h>

// -------------------------------------------------------------------------------- //
class guEQPreset
{
  public :
    wxString    m_Name;
    wxArrayInt  m_Sets;

    guEQPreset() {}
    guEQPreset( const wxString &name, const wxArrayInt &prefs )
    {
        m_Name = name;
        m_Sets = prefs;
    }
};
WX_DECLARE_OBJARRAY(guEQPreset, guEQPresetArray);


// -------------------------------------------------------------------------------- //
// Class guEq10Band
// -------------------------------------------------------------------------------- //
class guEq10Band : public wxDialog
{
  private:

  protected:
    wxComboBox *        m_PresetComboBox;
    wxBitmapButton *    m_ResetButton;
    wxBitmapButton *    m_SaveButton;
    wxBitmapButton *    m_DelButton;
    wxSlider *          m_Band0;
    wxSlider *          m_Band1;
    wxSlider *          m_Band2;
    wxSlider *          m_Band3;
    wxSlider *          m_Band4;
    wxSlider *          m_Band5;
    wxSlider *          m_Band6;
    wxSlider *          m_Band7;
    wxSlider *          m_Band8;
    wxSlider *          m_Band9;

    guMediaCtrl *       m_MediaCtrl;
    guEQPresetArray     m_EQPresets;
    bool                m_BandChanged;

    void OnPresetSelected( wxCommandEvent& event );
    void OnPresetText( wxCommandEvent& event );
    void OnAddPreset( wxCommandEvent& event );
    void OnDelPreset( wxCommandEvent& event );
    void OnResetPreset( wxCommandEvent &event );
    void OnBandChanged( wxScrollEvent& event );

  public:
    guEq10Band( wxWindow* parent, guMediaCtrl * mediactrl );
    ~guEq10Band();

};

#endif
// -------------------------------------------------------------------------------- //
