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
#ifndef MEDIACTRL_H
#define MEDIACTRL_H

#include <wx/wx.h>
#include <wx/uri.h>
#include <wx/mediactrl.h>

#undef ATTRIBUTE_PRINTF // there are warnings about redefined ATTRIBUTE_PRINTF in Fedora
#include <gst/gst.h>

DECLARE_EVENT_TYPE( wxEVT_MEDIA_TAG, wxID_ANY )
DECLARE_EVENT_TYPE( wxEVT_MEDIA_BUFFERING, wxID_ANY )
#define EVT_MEDIA_TAG(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_TAG, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),
#define EVT_MEDIA_BUFFERING(winid, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_BUFFERING, winid, wxID_ANY, wxMediaEventHandler(fn), (wxObject *) NULL ),

// -------------------------------------------------------------------------------- //
// guMediaCtrl : Interface class for gstreamer
// -------------------------------------------------------------------------------- //
class guMediaCtrl : public wxEvtHandler
{
  private :
    wxLongLong   m_llPausedPos;

  public :
    GstElement * m_Playbin;
    bool         m_Buffering;
    bool         m_WasPlaying;

    guMediaCtrl();
    ~guMediaCtrl();

    static bool Init();

    //bool Load( const wxURI &uri );
    bool Load( const wxString &uri );
    bool Stop();
    bool Play();
    bool Pause();

    bool Seek( wxLongLong where );
    wxFileOffset Tell();

    wxMediaState GetState();

    double GetVolume();
    bool SetVolume( double volume );

};

#endif
// -------------------------------------------------------------------------------- //
