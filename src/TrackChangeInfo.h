// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2013 J.Rios
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
#ifndef TRACKCHANGEINFO_H
#define TRACKCHANGEINFO_H


#include <wx/dynarray.h>
#include <wx/string.h>

class guMediaViewer;

// -------------------------------------------------------------------------------- //
class guTrackChangeInfo
{
  public :
    wxString            m_ArtistName;
    wxString            m_TrackName;
    guMediaViewer *     m_MediaViewer;

    guTrackChangeInfo() {};
    guTrackChangeInfo( const wxString &artist, const wxString &track, guMediaViewer * mediaviewer )
    {
        m_ArtistName = artist;
        m_TrackName = track;
        m_MediaViewer = mediaviewer;
    };

    ~guTrackChangeInfo() {};
};
WX_DECLARE_OBJARRAY(guTrackChangeInfo, guTrackChangeInfoArray);


#define guTRACKCHANGEINFO_MAXCOUNT  15

#endif
// -------------------------------------------------------------------------------- //
