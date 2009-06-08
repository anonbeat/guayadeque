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
#ifndef TRACKEDIT_H
#define TRACKEDIT_H

#include <wx/wx.h>
#include <wx/statline.h>
#include <wx/splitter.h>

#include "DbLibrary.h"

// -------------------------------------------------------------------------------- //
// Class guTrackEditor
// -------------------------------------------------------------------------------- //
class guTrackEditor : public wxDialog
{
	private:
	  guTrackArray *        m_Items;
	  int                   m_CurItem;
	  DbLibrary *           m_Db;

	protected:
		wxSplitterWindow *  m_SongListSplitter;
		wxPanel *           m_SongListPanel;
		wxListBox *         m_SongListBox;
		wxPanel *           m_DetailPanel;
		wxBitmapButton *    m_ArCopyButton;
		wxStaticText *      m_ArStaticText;
		wxTextCtrl *        m_ArtistTextCtrl;
		wxBitmapButton *    m_AlCopyButton;
		wxStaticText *      m_AlStaticText;
		wxTextCtrl *        m_AlbumTextCtrl;
		wxBitmapButton *    m_TiCopyButton;
		wxStaticText *      m_TiStaticText;
		wxTextCtrl *        m_TitleTextCtrl;
		wxBitmapButton *    m_NuCopyButton;
		wxStaticText *      m_NuStaticText;
		wxTextCtrl *        m_NumberTextCtrl;
		wxBitmapButton *    m_GeCopyButton;
		wxStaticText *      m_GeStaticText;
		wxTextCtrl *        m_GenreTextCtrl;
		wxBitmapButton *    m_YeCopyButton;
		wxBitmapButton *    m_GetYearButton;
		wxStaticText *      m_YeStaticText;
		wxTextCtrl *        m_YearTextCtrl;

		// Event handlers, overide them in your derived class
		void OnSongListBoxSelected( wxCommandEvent &event );
		void OnArCopyButtonClicked( wxCommandEvent &event );
		void OnAlCopyButtonClicked( wxCommandEvent &event );
		void OnTiCopyButtonClicked( wxCommandEvent &event );
		void OnNuCopyButtonClicked( wxCommandEvent &event );
		void OnGeCopyButtonClicked( wxCommandEvent &event );
		void OnYeCopyButtonClicked( wxCommandEvent &event );
//		void OnGetYearButtonClicked( wxCommandEvent &event );
		void ReadItemData( void );
		void WriteItemData( void );

		void OnButton( wxCommandEvent& event );
		void SongListSplitterOnIdle( wxIdleEvent& );

	public:
		guTrackEditor( wxWindow * parent, DbLibrary * Db, guTrackArray * Songs );
		~guTrackEditor();

};

#endif
// -------------------------------------------------------------------------------- //
