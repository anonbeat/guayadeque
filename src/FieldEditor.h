// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#ifndef FIELDEDITOR_H
#define FIELDEDITOR_H

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

// -------------------------------------------------------------------------------- //
/// Class guFieldEditor
// -------------------------------------------------------------------------------- //
class guFieldEditor : public wxDialog
{
  protected:
	wxStaticText* EditLabel;
	wxComboBox* m_EditComboBox;
	wxStdDialogButtonSizer* ButtonFrame;
	wxButton* ButtonFrameOK;
	wxButton* ButtonFrameCancel;

  public:
    guFieldEditor( wxWindow * parent, const wxString &label, const wxString &defval, const wxArrayString &items );
	~guFieldEditor();
	wxString GetData( void );

};

#endif
// -------------------------------------------------------------------------------- //
