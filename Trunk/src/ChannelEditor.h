// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
//	anonbeat@gmail.com
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
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef CHANNELEDITOR_H
#define CHANNELEDITOR_H

#include "DbLibrary.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

extern const wxEventType guChannelEditorEvent;
#define guCHANNELEDITOR_EVENT_UPDATE_IMAGE          1000

// -------------------------------------------------------------------------------- //
class guChannelEditor : public wxDialog
{
  protected:
    guPodcastChannel *  m_PodcastChannel;
	wxStaticBitmap  *   m_Image;
	wxStaticText    *   m_Title;
	wxStaticText    *   m_DescText;
	wxStaticText    *   m_AuthorText;
	wxStaticText    *   m_OwnerText;
	wxChoice        *   m_DownloadChoice;
	wxTextCtrl      *   m_DownloadText;
	wxCheckBox      *   m_DeleteCheckBox;

	void OnDownloadChoice( wxCommandEvent& event );
	void OnChannelImageUpdated( wxCommandEvent &event );

  public:
	guChannelEditor( wxWindow * parent, guPodcastChannel * channel );
	~guChannelEditor();
	void GetEditData( void );

};

class guChannelUpdateImageThread  : public wxThread
{
  protected :
    wxString            m_ImageUrl;
    guChannelEditor *   m_ChannelEditor;

  public :
    guChannelUpdateImageThread( guChannelEditor * channeleditor, const wxChar * imageurl );
    ~guChannelUpdateImageThread();

    ExitCode Entry();

};

#endif
// -------------------------------------------------------------------------------- //
