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
#ifndef __TRACKCHANGEINFO_H__
#define __TRACKCHANGEINFO_H__


#include <wx/dynarray.h>
#include <wx/string.h>

namespace Guayadeque {

#define guTRACKCHANGEINFO_MAXCOUNT  15

class guMediaViewer;

// -------------------------------------------------------------------------------- //
class guTrackChangeInfo
{
  public :
    wxString            m_ArtistName;
    wxString            m_TrackName;
    guMediaViewer *     m_MediaViewer;

    guTrackChangeInfo() {}
    guTrackChangeInfo( const wxString &artist, const wxString &track, guMediaViewer * mediaviewer )
    {
        m_ArtistName = artist;
        m_TrackName = track;
        m_MediaViewer = mediaviewer;
    };

    ~guTrackChangeInfo() {}
};
WX_DECLARE_OBJARRAY(guTrackChangeInfo, guTrackChangeInfoArray);

}

#endif
// -------------------------------------------------------------------------------- //
