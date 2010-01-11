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
#ifndef LYRICSPANEL_H
#define LYRICSPANEL_H

#include "DbLibrary.h"
#include "TrackChangeInfo.h"

#include <wx/bitmap.h>
#include <wx/checkbox.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/image.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/html/htmlwin.h>
#include <wx/panel.h>

class guSearchLyricEngine;

enum guLYRIC_ENGINE_ID {
    guLYRIC_ENGINE_LYRICWIKI = 0,
    guLYRIC_ENGINE_LEOSLYRICS,
    guLYRIC_ENGINE_LYRC_COM_AR,
    guLYRIC_ENGINE_CDUNIVERSE
};

// -------------------------------------------------------------------------------- //
class guLyricsPanel : public wxPanel
{
  protected :
    wxStaticText *          m_LyricTitle;
    wxHtmlWindow *          m_LyricText;
    guSearchLyricEngine *   m_LyricThread;
    wxCheckBox *            m_UpdateCheckBox;
	wxBitmapButton *        m_ReloadButton;
	wxChoice *              m_ServerChoice;
	wxTextCtrl *            m_ArtistTextCtrl;
	wxTextCtrl *            m_TrackTextCtrl;
	wxBitmapButton *        m_SearchButton;

	bool                    m_UpdateEnabled;
    guTrackChangeInfo       m_CurrentTrackInfo;
	wxString                m_CurrentFileName;

    void    SetTitle( const wxString &title );
    void    SetText( const wxString &text );
    void    OnDownloadedLyric( wxCommandEvent &event );
	void    OnReloadBtnClick( wxCommandEvent& event );
	void    OnUpdateChkBoxClicked( wxCommandEvent& event );
    void    OnTextUpdated( wxCommandEvent& event );
//	void    OnSearchBtnClick( wxCommandEvent& event );


  public :
    guLyricsPanel( wxWindow * parent );
    ~guLyricsPanel();

    void    OnUpdatedTrack( wxCommandEvent &event );
    void    SetTrack( const guTrackChangeInfo * trackchangeinfo );
    void    ClearLyricThread( void );

};

// -------------------------------------------------------------------------------- //
class guSearchLyricEngine : public wxThread
{
  private :
    guLyricsPanel *         m_LyricsPanel;
  protected :
    wxString                m_ArtistName;
    wxString                m_TrackName;

  public:
    guSearchLyricEngine( guLyricsPanel * lyricspanel, const wxChar * artistname, const wxChar * trackname );
    ~guSearchLyricEngine();

    virtual ExitCode Entry();
    virtual void SearchLyric( void ) = 0;
    virtual void SetLyric( wxString * lyrictext );
};

// -------------------------------------------------------------------------------- //
class guLyricWikiEngine : public guSearchLyricEngine
{
  public:
    guLyricWikiEngine( guLyricsPanel * lyricspanel, const wxChar * artistname, const wxChar * trackname );
    ~guLyricWikiEngine();

    virtual void SearchLyric( void );
};

// -------------------------------------------------------------------------------- //
class guLeosLyricsEngine : public guSearchLyricEngine
{
  private:
    wxString GetLyricId( void );
    wxString GetLyricText( const wxString &lyricid );

  public:
    guLeosLyricsEngine( guLyricsPanel * lyricspanel, const wxChar * artistname, const wxChar * trackname );
    ~guLeosLyricsEngine();

    virtual void SearchLyric( void );
};

// -------------------------------------------------------------------------------- //
class guLyrcComArEngine : public guSearchLyricEngine
{
  public:
    guLyrcComArEngine( guLyricsPanel * lyricspanel, const wxChar * artistname, const wxChar * trackname );
    ~guLyrcComArEngine();

    virtual void SearchLyric( void );
};

// -------------------------------------------------------------------------------- //
class guCDUEngine : public guSearchLyricEngine
{
  public:
    guCDUEngine( guLyricsPanel * lyricspanel, const wxChar * artistname, const wxChar * trackname );
    ~guCDUEngine();

    virtual void SearchLyric( void );
};

#endif
// -------------------------------------------------------------------------------- //
