// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
//    http://www.gnu.org/copyleft/gpl.h"tml
//
// -------------------------------------------------------------------------------- //
#ifndef __FILERENAMER_H__
#define __FILERENAMER_H__

#include "DbLibrary.h"

#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/regex.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
wxString inline NormalizeField( const wxString &name )
{
    // Special chars: < > : " / \ | ? *
    wxString RetVal = name;
    wxRegEx RepEx( wxT( "[<>:\"/\\?*]" ) );
    RepEx.ReplaceAll( &RetVal, wxT( "_" ) );

    if( RetVal[ 0 ] == wxT( '.' ) )
        RetVal[ 0 ] = wxT( '_' );

    return RetVal.Trim( true ).Trim( false );
}

// -------------------------------------------------------------------------------- //
class guFileRenamer : public wxDialog
{
  protected:
    wxListBox *         m_FilesListBox;
    wxTextCtrl *        m_NameTextCtrl;
    wxTextCtrl *        m_PatTextCtrl;
    wxBitmapButton *    m_PatApplyBtn;
    wxBitmapButton *    m_PatRevertBtn;

    guDbLibrary *       m_Db;
    int                 m_CurFile;
    wxArrayString       m_Files;

    void OnFileSelected( wxCommandEvent& event );
    void OnPatternChanged( wxCommandEvent& event );
    void OnPatternApply( wxCommandEvent& event );
    void OnPattternRevert( wxCommandEvent& event );

    void OnOKButton( wxCommandEvent& event );

  public:
    guFileRenamer( wxWindow * parent, guDbLibrary * db, const wxArrayString &files );
    ~guFileRenamer();

    wxArrayString GetRenamedNames( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
