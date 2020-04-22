// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef __TRANSCODE_H__
#define __TRANSCODE_H__

#include "DbLibrary.h"

#include <wx/event.h>
#include <wx/wx.h>

#include <gst/gst.h>

namespace Guayadeque {

enum guTranscodeFormat {
    guTRANSCODE_FORMAT_KEEP,
    guTRANSCODE_FORMAT_MP3,
    guTRANSCODE_FORMAT_OGG,
    guTRANSCODE_FORMAT_FLAC,
    guTRANSCODE_FORMAT_AAC,
    guTRANSCODE_FORMAT_WMA
};

enum guPortableMediaTranscodeQuality {      // mp3  ogg flac aac wma
    guTRANSCODE_QUALITY_KEEP,
    guTRANSCODE_QUALITY_VERY_HIGH,          // 320  240
    guTRANSCODE_QUALITY_HIGH,               // 256  160
    guTRANSCODE_QUALITY_VERY_GOOD,          // 192  140
    guTRANSCODE_QUALITY_GOOD,               // 160  120
    guTRANSCODE_QUALITY_NORMAL,             // 128  110
    guTRANSCODE_QUALITY_LOW,                // 96   96
    guTRANSCODE_QUALITY_VERY_LOW            // 64   70
};

wxArrayString       guTranscodeFormatStrings( const bool isipod = false );
wxString            guTranscodeFormatString( const int format );
wxArrayString       guTranscodeQualityStrings( void );
wxString            guTranscodeQualityString( const int quality );
int                 guGetTranscodeFileFormat( const wxString &filetype );

// -------------------------------------------------------------------------------- //
class guTranscodeThread : public wxThread
{
  protected :
    const guTrack *     m_Track;
    wxString            m_Target;
    wxString            m_TempName;
    int                 m_Format;
    int                 m_Quality;
    int                 m_StartPos;
    int                 m_Length;
    bool                m_Running;
    GstElement *        m_Pipeline;
    bool                m_HasError;
    int                 m_SeekTimerId;

    void                BuildPipeline( void );
    bool                BuildEncoder( GstElement ** enc, GstElement ** mux );
    void                WriteTags();

  public :
    guTranscodeThread( const guTrack * track, const wxChar * target, const int format, const int quality );
    ~guTranscodeThread();

    virtual ExitCode    Entry();
    void                Stop();
    bool                IsTranscoding( void ) { return m_Running; }
    bool                IsOk() { return !m_HasError; }
    void                SetError( bool error ) { m_HasError = error; }
    bool                CheckTrackEnd();
    GstElement *        GetPipeline() { return m_Pipeline; }
    bool                DoStartSeek();

};

}

#endif
// -------------------------------------------------------------------------------- //
