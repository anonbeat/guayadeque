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

#include <wx/html/htmlwin.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

class guFetchLyricThread;

// -------------------------------------------------------------------------------- //
class guLyricsPanel : public wxScrolledWindow
{
  protected :
    wxStaticText *          m_LyricTitle;
    wxHtmlWindow *          m_LyricText;
    guFetchLyricThread *    m_LyricThread;

    void    SetTitle( const wxString &title );
    void    SetText( const wxString &text );
    void    OnDownloadedLyric( wxCommandEvent &event );

  public :
    guLyricsPanel( wxWindow * parent );
    ~guLyricsPanel();

    void    OnUpdatedTrack( wxCommandEvent &event );
    void    SetTrack( const wxString &artistname, const wxString &trackname );
    void    ClearLyricThread( void );

};

// -------------------------------------------------------------------------------- //
class guFetchLyricThread : public wxThread
{
  protected:
    guLyricsPanel *         m_LyricsPanel;
    wxString                m_ArtistName;
    wxString                m_TrackName;

  public:
    guFetchLyricThread( guLyricsPanel * lyricpanel, const wxChar * artistname, const wxChar * trackname );
    ~guFetchLyricThread();

    virtual ExitCode Entry();

};

#endif
// -------------------------------------------------------------------------------- //
