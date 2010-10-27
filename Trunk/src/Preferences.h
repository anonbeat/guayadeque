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
#include <wx/dynarray.h>

#define  guPREFERENCE_PAGE_FLAG_GENERAL          ( 1 << 0 )
#define  guPREFERENCE_PAGE_FLAG_LIBRARY          ( 1 << 1 )
#define  guPREFERENCE_PAGE_FLAG_PLAYBACK         ( 1 << 2 )
#define  guPREFERENCE_PAGE_FLAG_CROSSFADER       ( 1 << 3 )
#define  guPREFERENCE_PAGE_FLAG_RECORD           ( 1 << 4 )
#define  guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE    ( 1 << 5 )
#define  guPREFERENCE_PAGE_FLAG_LYRICS           ( 1 << 6 )
#define  guPREFERENCE_PAGE_FLAG_ONLINE           ( 1 << 7 )
#define  guPREFERENCE_PAGE_FLAG_PODCASTS         ( 1 << 8 )
#define  guPREFERENCE_PAGE_FLAG_JAMENDO          ( 1 << 9 )
#define  guPREFERENCE_PAGE_FLAG_MAGNATUNE        ( 1 << 10 )
#define  guPREFERENCE_PAGE_FLAG_LINKS            ( 1 << 11 )
#define  guPREFERENCE_PAGE_FLAG_COMMANDS         ( 1 << 12 )
#define  guPREFERENCE_PAGE_FLAG_COPYTO           ( 1 << 13 )

enum guPreference_Page {
    guPREFERENCE_PAGE_GENERAL,
    guPREFERENCE_PAGE_LIBRARY,
    guPREFERENCE_PAGE_PLAYBACK,
    guPREFERENCE_PAGE_CROSSFADER,
    guPREFERENCE_PAGE_RECORD,
    guPREFERENCE_PAGE_AUDIOSCROBBLE,
    guPREFERENCE_PAGE_LYRICS,
    guPREFERENCE_PAGE_ONLINE,
    guPREFERENCE_PAGE_PODCASTS,
    guPREFERENCE_PAGE_JAMENDO,
    guPREFERENCE_PAGE_MAGNATUNE,
    guPREFERENCE_PAGE_LINKS,
    guPREFERENCE_PAGE_COMMANDS,
    guPREFERENCE_PAGE_COPYTO,
    guPREFERENCE_PAGE_LASTUSED = -1
};

// -------------------------------------------------------------------------------- //
class guCopyToPattern
{
  public :
    wxString    m_Name;
    wxString    m_Pattern;
    int         m_Format;
    int         m_Quality;
    bool        m_MoveFiles;

    guCopyToPattern();
    guCopyToPattern( const wxString &pattern );

    ~guCopyToPattern();

    wxString    ToString( void );
};
WX_DECLARE_OBJARRAY( guCopyToPattern, guCopyToPatternArray );

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
    wxCheckBox *                m_LibScanPlayListChkBox;
    wxCheckBox *                m_LibScanSymlinksChkBox;
    wxCheckBox *                m_LibScanEmbCoversChkBox;
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

    wxPanel *                   m_XFadePanel;
    wxStaticText *              m_XFadeOutLenVal;
	wxStaticText *              m_XFadeInLenVal;
	wxStaticText *              m_XFadeInStartVal;
	wxStaticText *              m_XFadeTrigerVal;
    wxSlider *                  m_XFadeOutLenSlider;
    wxSlider *                  m_XFadeInLenSlider;
    wxSlider *                  m_XFadeInStartSlider;
    wxSlider *                  m_XFadeInTrigerSlider;
    wxStaticBitmap *            m_FadeBitmap;

    wxPanel *                   m_RecordPanel;
    wxCheckBox *                m_RecordChkBox;
    wxDirPickerCtrl *           m_RecSelDirPicker;
    wxChoice *                  m_RecFormatChoice;
    wxChoice *                  m_RecQualityChoice;
    wxCheckBox *                m_RecSplitChkBox;
    wxCheckBox *                m_RecDelTracks;
    wxSpinCtrl *                m_RecDelTime;

    wxPanel *                   m_LastFMPanel;
    wxCheckBox *                m_LastFMASEnableChkBox;
    wxTextCtrl *                m_LastFMUserNameTextCtrl;
    wxTextCtrl *                m_LastFMPasswdTextCtrl;
    wxCheckBox *                m_LibreFMASEnableChkBox;
    wxTextCtrl *                m_LibreFMUserNameTextCtrl;
    wxTextCtrl *                m_LibreFMPasswdTextCtrl;

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
    wxListBox *                 m_CopyToListBox;
    wxBitmapButton *            m_CopyToAddBtn;
    wxBitmapButton *            m_CopyToUpBtn;
    wxBitmapButton *            m_CopyToDownBtn;
    wxBitmapButton *            m_CopyToDelBtn;
    wxTextCtrl *                m_CopyToPatternTextCtrl;
    wxTextCtrl *                m_CopyToNameTextCtrl;
    wxChoice *                  m_CopyToFormatChoice;
    wxChoice *                  m_CopyToQualityChoice;
    wxCheckBox *                m_CopyToMoveFilesChkBox;
    wxBitmapButton *            m_CopyToAcceptBtn;

    wxTextCtrl *                m_BrowserCmdTextCtrl;

    wxRadioBox *                m_RadioMinBitRateRadBox;
    wxArrayString               m_RadioMinBitRateRadBoxChoices;
    wxSlider *                  m_BufferSizeSlider;

    wxPanel *                   m_PodcastPanel;
    wxDirPickerCtrl *           m_PodcastPath;
    wxCheckBox *                m_PodcastUpdate;
    wxChoice *                  m_PodcastUpdatePeriod;
    wxCheckBox *                m_PodcastDelete;
    wxSpinCtrl *                m_PodcastDeleteTime;
    wxChoice *                  m_PodcastDeletePeriod;

    wxCheckBox *                m_PodcastDeletePlayed;

    wxPanel *                   m_JamendoPanel;
    wxCheckListBox *            m_JamGenresListBox;
    wxButton *                  m_JamSelAllBtn;
    wxButton *                  m_JamSelNoneBtn;
    wxButton *                  m_JamInvertBtn;
    wxChoice *                  m_JamFormatChoice;
    wxTextCtrl *                m_JamBTCmd;
    wxArrayInt                  m_LastJamendoGenres;

    wxPanel *                   m_MagnatunePanel;
    wxCheckListBox *            m_MagGenresListBox;
    wxButton *                  m_MagSelAllBtn;
    wxButton *                  m_MagSelNoneBtn;
    wxButton *                  m_MagInvertBtn;
    wxRadioButton *             m_MagNoRadioItem;
    wxRadioButton *             m_MagStRadioItem;
    wxRadioButton *             m_MagDlRadioItem;
    wxTextCtrl *                m_MagUserTextCtrl;
    wxTextCtrl *                m_MagPassTextCtrl;
    wxChoice *                  m_MagFormatChoice;
    wxChoice *                  m_MagDownFormatChoice;
    wxArrayString               m_LastMagnatuneGenres;


    wxChoice *                  m_LyricsChoice;

    guConfig *                  m_Config;
    int                         m_PathSelected;
    int                         m_CoverSelected;
    int                         m_FilterSelected;
    int                         m_LinkSelected;
    int                         m_CmdSelected;
    int                         m_CopyToSelected;
    guCopyToPatternArray *      m_CopyToOptions;
    bool                        m_LibPathsChanged;
    int                         m_VisiblePanels;

    void                        BuildGeneralPage( void );
    void                        BuildLibraryPage( void );
    void                        BuildPlaybackPage( void );
    void                        BuildCrossfaderPage( void );
    void                        BuildRecordPage( void );
    void                        BuildAudioScrobblePage( void );
    void                        BuildLyricsPage( void );
    void                        BuildOnlinePage( void );
    void                        BuildPodcastsPage( void );
    void                        BuildJamendoPage( void );
    void                        BuildMagnatunePage( void );
    void                        BuildLinksPage( void );
    void                        BuildCommandsPage( void );
    void                        BuildCopyToPage( void );

    // Event Handlers
    void OnPageChanged( wxCommandEvent &event );
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
    void OnCrossFadeChanged( wxScrollEvent& event );
    void OnRecEnableClicked( wxCommandEvent& event );
    void OnRecDelTracksClicked( wxCommandEvent& event );
	void OnFiltersListBoxSelected( wxCommandEvent &event );
	void OnLastFMASUserNameChanged( wxCommandEvent &event );
	void OnLibreFMASUserNameChanged( wxCommandEvent &event );
    void OnOnlineAddBtnClick( wxCommandEvent& event );
	void OnOnlineDelBtnClick( wxCommandEvent& event );
	void OnOnlineListBoxDClicked( wxCommandEvent &event );

    void OnJamendoSelectAll( wxCommandEvent& event );
    void OnJamendoSelectNone( wxCommandEvent& event );
    void OnJamendoInvertSelection( wxCommandEvent& event );

    void OnMagnatuneSelectAll( wxCommandEvent& event );
    void OnMagnatuneSelectNone( wxCommandEvent& event );
    void OnMagnatuneInvertSelection( wxCommandEvent& event );
    void OnMagNoRadioItemChanged( wxCommandEvent& event );

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

    void OnCopyToListBoxSelected( wxCommandEvent &event );
    void OnCopyToAddBtnClick( wxCommandEvent& event );
	void OnCopyToDelBtnClick( wxCommandEvent& event );
	void OnCopyToMoveUpBtnClick( wxCommandEvent &event );
	void OnCopyToMoveDownBtnClick( wxCommandEvent &event );
	void OnCopyToTextChanged( wxCommandEvent &event );
	void OnCopyToFormatChanged( wxCommandEvent &event );
	void OnCopyToQualityChanged( wxCommandEvent &event );
	void OnCopyToMoveFilesChanged( wxCommandEvent &event );
	void OnCopyToSaveBtnClick( wxCommandEvent &event );

  public:
    guPrefDialog( wxWindow * parent, guDbLibrary * db, int pagenum = guPREFERENCE_PAGE_LASTUSED );
    ~guPrefDialog();

    void SaveSettings( void );
    int  GetVisiblePanels( void ) { return m_VisiblePanels; }

};

#endif
// -------------------------------------------------------------------------------- //
