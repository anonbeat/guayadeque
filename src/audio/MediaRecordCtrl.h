// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#ifndef __MEDIARECORDCTRL_H__
#define __MEDIARECORDCTRL_H__

#include "DbLibrary.h"

#include <wx/wx.h>

namespace Guayadeque {

class guPlayerPanel;
class guMediaCtrl;

// -------------------------------------------------------------------------------- //
class guMediaRecordCtrl
{
  protected :
    guPlayerPanel * m_PlayerPanel;
    guMediaCtrl *   m_MediaCtrl;
    guTrack         m_TrackInfo;
    guTrack         m_PrevTrack;
    wxString        m_PrevFileName;

    wxString        m_MainPath;
    int             m_Format;
    int             m_Quality;
    bool            m_DeleteTracks;
    int             m_DeleteTime;
    wxString        m_Ext;
    wxString        m_FileName;


    bool            m_Recording;
    bool            m_SplitTracks;
    bool            m_FirstChange;

    wxString        GenerateRecordFileName( void );

  public :
    guMediaRecordCtrl( guPlayerPanel * playerpanel, guMediaCtrl * mediactrl );
    virtual ~guMediaRecordCtrl();

    void            SetTrack( const guTrack &track );
    void            SetTrackName( const wxString &artistname, const wxString &trackname );

    void            SetStation( const wxString &station );

    void            SetGenre( const wxString &genre ) { m_TrackInfo.m_GenreName = genre; }

    bool            SaveTagInfo( const wxString &filename, const guTrack * track );

    bool            IsRecording( void ) { return m_Recording; }

    bool            Start( const guTrack * track );
    bool            Stop( void );

    void            SplitTrack( void );

    void            UpdatedConfig( void );

    wxString        GetRecordFileName( void ) { return m_FileName; }

};

}

#endif
// -------------------------------------------------------------------------------- //
