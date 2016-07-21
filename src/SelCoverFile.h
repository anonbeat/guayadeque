// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef __SELCOVERFILE_H__
#define __SELCOVERFILE_H__

#include "DbLibrary.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>


namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// Class guSelCoverFile
// -------------------------------------------------------------------------------- //
class guSelCoverFile : public wxDialog
{
  protected:
    guDbLibrary *   m_Db;
    wxString        m_AlbumPath;

    // GUI
    wxTextCtrl *    m_FileLink;
    wxButton *      m_SelFileBtn;
    wxCheckBox *    m_EmbedToFilesChkBox;
    wxButton *      m_StdBtnOk;

    void            OnSelFileClicked( wxCommandEvent& event );
    void            OnPathChanged( wxCommandEvent &event );

    void            OnCoverFinish( wxCommandEvent &event );

  public:
    guSelCoverFile( wxWindow * parent, guDbLibrary * db, const int albumid = wxNOT_FOUND ); //, wxWindowID id = wxID_ANY, const wxString& title = wxT("Select Cover File"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,125 ), long style = wxDEFAULT_DIALOG_STYLE );
	~guSelCoverFile();

    wxString GetSelFile( void );
    wxString GetAlbumPath( void ) { return m_AlbumPath; }

    bool     EmbedToFiles( void ) { return m_EmbedToFilesChkBox->IsChecked(); }
};

}

#endif
// -------------------------------------------------------------------------------- //
