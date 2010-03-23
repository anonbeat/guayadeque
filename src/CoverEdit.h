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
#ifndef COVEREDIT_H
#define COVEREDIT_H

#include "ArrayStringArray.h"
#include "AutoPulseGauge.h"
#include "ThreadArray.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/image.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/choice.h>

#define GUCOVERINFO_LINK    0
#define GUCOVERINFO_SIZE    1


// -------------------------------------------------------------------------------- //
class guCoverImage
{
  public:
    wxString    m_Link;
    wxString    m_SizeStr;
    wxImage *   m_Image;

    guCoverImage()
    {
        m_Link = wxEmptyString;
        m_Image = NULL;
    };

    guCoverImage( const wxString &link, const wxString size, wxImage * image )
    {
        m_Link = link;
        m_SizeStr = size;
        m_Image = image;
    };

    ~guCoverImage()
    {
        if( m_Image )
            delete m_Image;
    };
};
WX_DECLARE_OBJARRAY(guCoverImage, guCoverImageArray);

class guCoverEditor;
class guCoverFetcher;

// -------------------------------------------------------------------------------- //
class guFetchCoverLinksThread : public wxThread
{
  private:
    guCoverEditor *     m_CoverEditor;
    guCoverFetcher *    m_CoverFetcher;
	guArrayStringArray  m_CoverLinks;
	int                 m_CurrentPage;
    int                 m_LastDownload;
    wxString            m_Artist;
    wxString            m_Album;
    int                 m_EngineIndex;

  public:
    guFetchCoverLinksThread( guCoverEditor * owner, const wxChar * artist, const wxChar * album, int engineindex );
    ~guFetchCoverLinksThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guDownloadCoverThread : public wxThread
{
  private:
    guCoverEditor * m_CoverEditor;
    wxString        m_UrlStr;
    wxString        m_SizeStr;

  public:
    guDownloadCoverThread( guCoverEditor * Owner, const wxArrayString * CoverInfo );
    ~guDownloadCoverThread();

    virtual ExitCode Entry();
};

class guCoverFetcher;

// -------------------------------------------------------------------------------- //
// Class guCoverEditor
// -------------------------------------------------------------------------------- //
class guCoverEditor : public wxDialog
{
  private:
    wxTextCtrl *                m_ArtistTextCtrl;
    wxTextCtrl *                m_AlbumTextCtrl;
    wxChoice *                  m_EngineChoice;

	wxStaticBitmap *            m_CoverBitmap;
	wxBitmapButton *            m_PrevButton;
	wxBitmapButton *            m_NextButton;
	wxButton *                  m_ButtonsSizerOK;
	wxButton *                  m_ButtonsSizerCancel;
	wxBoxSizer *                m_SizeSizer;
	wxStaticText *              m_SizeStaticText;

	wxMutex                     m_DownloadThreadMutex;
	wxMutex                     m_DownloadEventsMutex;

    guAutoPulseGauge *          m_Gauge;
    wxStaticText *              m_InfoTextCtrl;

	guCoverImageArray           m_AlbumCovers;
	guFetchCoverLinksThread *   m_DownloadCoversThread;
	guThreadArray               m_DownloadThreads;
	int                         m_CurrentImage;
    int                         m_EngineIndex;


	void OnInitDialog( wxInitDialogEvent& event );
	void OnTextCtrlEnter( wxCommandEvent& event );
    void OnEngineChanged( wxCommandEvent& event );
	void OnCoverLeftDClick( wxMouseEvent& event );
	void OnPrevButtonClick( wxCommandEvent& event );
	void OnNextButtonClick( wxCommandEvent& event );
	void OnAddCoverImage( wxCommandEvent &event );
	void UpdateCoverBitmap();
	void EndDownloadLinksThread();
	void EndDownloadCoverThread( guDownloadCoverThread * DownloadCoverThread );
	void OnDownloadedLinks( wxCommandEvent &event );

  public:
	guCoverEditor( wxWindow * parent, const wxString &Artist, const wxString &Album );// wxWindowID id = wxID_ANY, const wxString& title = wxT("Cover Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE );
	~guCoverEditor();
    wxString    GetSelectedCoverUrl( void );
    wxImage *   GetSelectedCoverImage( void );

  friend class guFetchCoverLinksThread;
  friend class guDownloadCoverThread;
};

#endif
// -------------------------------------------------------------------------------- //
