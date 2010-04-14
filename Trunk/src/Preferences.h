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
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/listbox.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/radiobox.h>
#include <wx/filepicker.h>
#include <wx/listbook.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/dialog.h>

// -------------------------------------------------------------------------------- //
class guPrefDialog : public wxDialog
{
  private:
    guDbLibrary *               m_Db;

  protected:
    wxListbook *                m_MainNotebook;
    wxImageList *               m_ImageList;

    wxPanel *                   m_GenPanel;
    wxCheckBox *                m_ShowSplashChkBox;
    wxCheckBox *                m_MinStartChkBox;
    wxCheckBox *                m_TaskIconChkBox;
    wxCheckBox *                m_IgnoreLayoutsChkBox;
    wxCheckBox *                m_DropFilesChkBox;
    wxCheckBox *                m_EnqueueChkBox;
    wxChoice *                  m_AlYearOrderChoice;
    wxCheckBox *                m_SavePlayListChkBox;
    wxCheckBox *                m_SavePosCheckBox;
	wxSpinCtrl *                m_MinLenSpinCtrl;
    wxCheckBox *                m_CloseTaskBarChkBox;
    wxCheckBox *                m_ExitConfirmChkBox;

    wxPanel *                   m_LibPanel;
    wxListBox *                 m_PathsListBox;
    wxBitmapButton *            m_AddPathButton;
    wxBitmapButton *            m_DelPathButton;
    wxListBox *                 m_CoversListBox;
    wxBitmapButton *            m_AddCoverButton;
    wxBitmapButton *            m_UpCoverButton;
    wxBitmapButton *            m_DownCoverButton;
    wxBitmapButton *            m_DelCoverButton;
    wxCheckBox *                m_UpdateLibChkBox;
    wxCheckBox *                m_SaveLyricsChkBox;

    wxPanel *                   m_PlayPanel;
    wxCheckBox *                m_RndPlayChkBox;
    wxChoice *                  m_RndModeChoice;
    wxCheckBox *                m_DelPlayChkBox;
    wxCheckBox *                m_PlayDelPlayedTrack;
    wxCheckBox *                m_PlayLevelEnabled;
    wxSlider *                  m_PlayLevelSlider;
    wxCheckBox *                m_PlayEndTimeCheckBox;
    wxSpinCtrl *                m_PlayEndTimeSpinCtrl;
    wxCheckBox *                m_NotifyChkBox;

    wxPanel *                   m_RecordPanel;
    wxCheckBox *                m_RecordChkBox;
    wxDirPickerCtrl *           m_RecSelDirPicker;
    wxChoice *                  m_RecFormatChoice;
    wxChoice *                  m_RecQualityChoice;
    wxCheckBox *                m_RecSplitChkBox;
    wxCheckBox *                m_RecDelTracks;
    wxSpinCtrl *                m_RecDelTime;

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
    wxSpinCtrl *                m_MinTracksSpinCtrl;
    wxSpinCtrl *                m_NumTracksSpinCtrl;
    wxSpinCtrl *                m_MaxTracksPlayed;

    wxPanel *                   m_LyricsPanel;
    wxRadioBox *                m_LyricsAlignSizer;
    wxCheckBox *                m_LyricsTracksSaveChkBox;
    wxCheckBox *                m_LyricsTracksSaveSelectedChkBox;
    wxCheckBox *                m_LyricsDirSaveChkBox;
    wxDirPickerCtrl *           m_LyricsDirSavePicker;
    wxCheckBox *                m_LyricsDirSaveSelectedChkBox;

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

    wxPanel *                   m_CmdPanel;
    wxListBox *                 m_CmdListBox;
    wxBitmapButton *            m_CmdAddBtn;
    wxBitmapButton *            m_CmdDelBtn;
    wxBitmapButton *            m_CmdMoveUpBtn;
    wxBitmapButton *            m_CmdMoveDownBtn;
    wxTextCtrl *                m_CmdTextCtrl;
    wxTextCtrl *                m_CmdNameTextCtrl;
    wxBitmapButton *            m_CmdAcceptBtn;
    wxArrayString               m_CmdNames;

	wxPanel *                   m_CopyPanel;
	wxTextCtrl *                m_CopyToFileName;
	wxTextCtrl *                m_CopyToExampleTextCtrl;

    wxTextCtrl *                m_BrowserCmdTextCtrl;

    wxRadioBox *                m_RadioMinBitRateRadBox;
    wxArrayString               m_RadioMinBitRateRadBoxChoices;

    wxDirPickerCtrl *           m_PodcastPath;
    wxCheckBox *                m_PodcastUpdate;
    wxChoice *                  m_PodcastUpdatePeriod;
    wxCheckBox *                m_PodcastDelete;
    wxSpinCtrl *                m_PodcastDeleteTime;
    wxChoice *                  m_PodcastDeletePeriod;

    wxCheckBox* m_PodcastDeletePlayed;

    wxChoice *                  m_LyricsChoice;

    guConfig *                  m_Config;
    int                         m_PathSelected;
    int                         m_CoverSelected;
    int                         m_FilterSelected;
    int                         m_LinkSelected;
    int                         m_CmdSelected;
    bool                        m_LibPathsChanged;

    // Event Handlers
    void OnActivateTaskBarIcon( wxCommandEvent& event );
    void OnRndPlayClicked( wxCommandEvent& event );
    void OnDelPlayedTracksChecked( wxCommandEvent& event );
    void OnPathsListBoxSelected( wxCommandEvent& event );
    void OnAddPathBtnClick( wxCommandEvent& event );
	void OnDelPathBtnClick( wxCommandEvent& event );
	void OnPathsListBoxDClicked( wxCommandEvent &event );
	void OnCoverListBoxDClicked( wxCommandEvent &event );
	void OnLyricsSaveTracksClicked( wxCommandEvent &event );
	void OnLyricsSaveDirClicked( wxCommandEvent &event );
    void OnCoversListBoxSelected( wxCommandEvent& event );
    void OnAddCoverBtnClick( wxCommandEvent& event );
    void OnUpCoverBtnClick( wxCommandEvent& event );
    void OnDownCoverBtnClick( wxCommandEvent& event );
	void OnDelCoverBtnClick( wxCommandEvent& event );
    void OnPlayLevelEnabled( wxCommandEvent& event );
    void OnPlayEndTimeEnabled( wxCommandEvent& event );
    void OnRecEnableClicked( wxCommandEvent& event );
    void OnRecDelTracksClicked( wxCommandEvent& event );
	void OnFiltersListBoxSelected( wxCommandEvent &event );
	void OnASUserNameChanged( wxCommandEvent &event );
    void OnOnlineAddBtnClick( wxCommandEvent& event );
	void OnOnlineDelBtnClick( wxCommandEvent& event );
	void OnOnlineListBoxDClicked( wxCommandEvent &event );

    void OnLinksListBoxSelected( wxCommandEvent &event );
    void OnLinksAddBtnClick( wxCommandEvent& event );
	void OnLinksDelBtnClick( wxCommandEvent& event );
	void OnLinkMoveUpBtnClick( wxCommandEvent &event );
	void OnLinkMoveDownBtnClick( wxCommandEvent &event );
	void OnLinksTextChanged( wxCommandEvent &event );
	void OnLinksSaveBtnClick( wxCommandEvent &event );

    void OnCmdListBoxSelected( wxCommandEvent &event );
    void OnCmdAddBtnClick( wxCommandEvent& event );
	void OnCmdDelBtnClick( wxCommandEvent& event );
	void OnCmdMoveUpBtnClick( wxCommandEvent &event );
	void OnCmdMoveDownBtnClick( wxCommandEvent &event );
	void OnCmdTextChanged( wxCommandEvent &event );
	void OnCmdSaveBtnClick( wxCommandEvent &event );


	void OnCopyToFileNameUpdated( wxCommandEvent &event );

  public:
    guPrefDialog( wxWindow * parent, guDbLibrary * db );
    ~guPrefDialog();

    void SaveSettings( void );

};

#endif
// -------------------------------------------------------------------------------- //
