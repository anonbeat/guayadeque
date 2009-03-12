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
#ifndef RADIOPANEL_H
#define RADIOPANEL_H

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/frame.h>

#include "PlayerPanel.h"
#include "ItemListBox.h"

class guRadioGenreListBox;
class guRadioStationListBox;

// -------------------------------------------------------------------------------- //
// Class guRadioPanel
// -------------------------------------------------------------------------------- //
class guRadioPanel : public wxPanel
{
	private:
	  DbLibrary * m_Db;
	  guPlayerPanel * m_PlayerPanel;

      void OnRadioUpdateEnd( wxCommandEvent &event );
	  void OnRadioUpdate( wxCommandEvent &Event );
	  void OnRadioUpdated( wxCommandEvent &Event );
	  void OnRadioGenreListSelected( wxListEvent &Event );
      void OnStationListActivated( wxListEvent &event );
      void OnStationListBoxColClick( wxListEvent &event );
      void ListsSplitterOnIdle( wxIdleEvent& );
      void OnSearchActivated( wxCommandEvent& event );
      void OnSearchCancelled( wxMouseEvent &event );

	protected:
		wxStaticText*           m_SearchStaticText;
		wxPanel*                m_InputTextPanel;
		wxStaticBitmap*         m_InputTextLeftBitmap;
		wxTextCtrl*             m_InputTextCtrl;
		wxStaticBitmap*         m_InputTextClearBitmap;
		wxStaticLine*           m_SearchStaticline;
		wxSplitterWindow*       m_ListsSplitter;
		wxPanel*                m_GenresPanel;
		guRadioGenreListBox *   m_GenresListBox;
		wxPanel*                m_RadiosPanel;
		guRadioStationListBox * m_StationsListBox;

	public:
		guRadioPanel( wxWindow* parent, DbLibrary * Db, guPlayerPanel * NewPlayerPanel ); //wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 579,465 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~guRadioPanel();

};

#endif
// -------------------------------------------------------------------------------- //

