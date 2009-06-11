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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "Config.h"
#include "DbLibrary.h"

#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/listbox.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/dialog.h>

// -------------------------------------------------------------------------------- //
class guPrefDialog : public wxDialog
{
  private:
    DbLibrary *                 m_Db;

  protected:
    wxNotebook *                m_MainNotebook;
    wxPanel *                   m_GenPanel;
    wxCheckBox *                m_ShowSplashChkBox;
    wxCheckBox *                m_MinStartChkBox;
    wxCheckBox *                m_TaskIconChkBox;
    wxCheckBox *                m_DropFilesChkBox;
    wxCheckBox *                m_EnqueueChkBox;
    wxCheckBox *                m_RndPlayChkBox;
    wxCheckBox *                m_AlYearOrderChkBox;
    wxCheckBox *                m_SavePlayListChkBox;
    wxCheckBox *                m_CloseTaskBarChkBox;
    wxCheckBox *                m_ExitConfirmChkBox;
    wxPanel *                   m_LibPanel;
    wxListBox *                 m_PathsListBox;
    wxBitmapButton *            m_AddPathButton;
    wxBitmapButton *            m_DelPathButton;
    wxListBox *                 m_CoversListBox;
    wxBitmapButton *            m_AddCoverButton;
    wxBitmapButton *            m_DelCoverButton;
    wxCheckBox *                m_UpdateLibChkBox;
    wxCheckBox *                m_CoverSearchChkBox;
    wxPanel *                   m_LastFMPanel;
    wxCheckBox *                m_ASEnableChkBox;
    wxStaticText *              m_UserNameStaticText;
    wxTextCtrl *                m_UserNameTextCtrl;
    wxStaticText *              m_PasswdStaticText;
    wxTextCtrl *                m_PasswdTextCtrl;
    wxStaticText *              m_LangStaticText;
    wxChoice *                  m_LangChoice;
    wxArrayString               m_LangNames;
    wxArrayString               m_LangIds;
    wxSpinCtrl *                m_SmartPlayListMinTracksSpinCtrl;
    wxStaticText *              m_SmartPlayListMinTracksStaticText;
    wxSpinCtrl *                m_SmartPlayListAddTracksSpinCtrl;
    wxStaticText *              m_SmartPlayListAddTracksStaticText;
    wxSpinCtrl *                m_SmartPlayListMaxTracksSpinCtrl;
    wxStaticText *              m_SmartPlayListMaxTracksStaticText;

    wxPanel *                   m_OnlinePanel;
    wxListBox *                 m_OnlineFiltersListBox;
    wxBitmapButton *            m_OnlineAddBtn;
    wxBitmapButton *            m_OnlineDelBtn;

    wxPanel *                   m_LinksPanel;
    wxListBox *                 m_LinksListBox;
    wxBitmapButton *            m_LinksAddBtn;
    wxBitmapButton *            m_LinksDelBtn;
    wxBitmapButton *            m_LinksMoveUpBtn;
    wxBitmapButton *            m_LinksMoveDownBtn;
    wxTextCtrl *                m_LinksUrlTextCtrl;
    wxTextCtrl *                m_LinksNameTextCtrl;
    wxBitmapButton *            m_LinksAcceptBtn;
    wxArrayString               m_LinksNames;

	wxPanel *                   m_CopyPanel;
	wxTextCtrl *                m_CopyToFileName;
	wxTextCtrl *                m_CopyToExampleTextCtrl;

    wxTextCtrl *                m_BrowserCmdTextCtrl;

    wxRadioBox *                m_RadioMinBitRateRadBox;
    wxArrayString               m_RadioMinBitRateRadBoxChoices;

    guConfig *                  m_Config;
    int                         m_PathSelected;
    int                         m_CoverSelected;
    int                         m_FilterSelected;
    int                         m_LinkSelected;

    // Event Handlers
    void OnPathsListBoxSelected( wxCommandEvent& event );
    void OnAddPathBtnClick( wxCommandEvent& event );
	void OnDelPathBtnClick( wxCommandEvent& event );
	void OnPathsListBoxDClicked( wxCommandEvent &event );
	void OnCoverListBoxDClicked( wxCommandEvent &event );
    void OnCoversListBoxSelected( wxCommandEvent& event );
    void OnAddCoverBtnClick( wxCommandEvent& event );
	void OnDelCoverBtnClick( wxCommandEvent& event );
	void OnFiltersListBoxSelected( wxCommandEvent &event );
	void OnASUserNameChanged( wxCommandEvent &event );
    void OnOnlineAddBtnClick( wxCommandEvent& event );
	void OnOnlineDelBtnClick( wxCommandEvent& event );
	void OnOnlineListBoxDClicked( wxCommandEvent &event );
	void OnLinksListBoxSelected( wxCommandEvent &event );
    //void OnLinkListBoxDClicked( wxCommandEvent &event );
    void OnLinksAddBtnClick( wxCommandEvent& event );
	void OnLinksDelBtnClick( wxCommandEvent& event );
	void OnLinkMoveUpBtnClick( wxCommandEvent &event );
	void OnLinkMoveDownBtnClick( wxCommandEvent &event );
	void OnLinksTextChanged( wxCommandEvent &event );
	void OnLinksSaveBtnClick( wxCommandEvent &event );

	void OnCopyToFileNameUpdated( wxCommandEvent &event );

  public:
    guPrefDialog( wxWindow * parent, DbLibrary * db );
    ~guPrefDialog();

};

#endif
// -------------------------------------------------------------------------------- //
