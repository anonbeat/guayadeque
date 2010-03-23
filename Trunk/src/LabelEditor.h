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
#ifndef LABELEDITOR_H
#define LABELEDITOR_H

#include "DbLibrary.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/checklst.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

// -------------------------------------------------------------------------------- //
// Class guLabelEditor
// -------------------------------------------------------------------------------- //
class guLabelEditor : public wxDialog
{
  protected:
    guDbLibrary *       m_Db;
    wxCheckListBox *    m_CheckListBox;
    wxBitmapButton *    m_AddLabelBtn;
	wxBitmapButton *    m_DelLabelBtn;

    wxArrayInt          m_LabelIds;
    int                 m_SelectedItem;

    bool                m_IsRadioLabel;

	void SetCheckedItems( const wxArrayInt &Checked );
    void OnAddLabelBtnClick( wxCommandEvent &event );
	void OnDelLabelBtnClick( wxCommandEvent &event );
	void OnCheckListBoxSelected( wxCommandEvent& event );

  public:
	guLabelEditor( wxWindow * parent, guDbLibrary * db, const wxString &title, const bool isradiolabel, const guListItems &labels, const guArrayListItems &enabelditems );
	~guLabelEditor();
    wxArrayInt GetCheckedIds( void );

};

#endif
// -------------------------------------------------------------------------------- //
