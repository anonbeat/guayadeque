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
#ifndef TAGINFO_H
#define TAGINFO_H

#include <id3/tag.h>
#include <id3/misc_support.h>

#include <wx/string.h>
#include <wx/arrstr.h>

// -------------------------------------------------------------------------------- //
class TagInfo
{
  public:
    wxString        m_TrackName;
    wxString        m_GenreName;
    wxString        m_ArtistName;
    wxString        m_AlbumName;
    long            m_Track;
    long            m_Year;
    long            m_Length;
    wxArrayString   m_TrackLabels;
    wxString        m_TrackLabelsStr;
    wxArrayString   m_ArtistLabels;
    wxString        m_ArtistLabelsStr;
    wxArrayString   m_AlbumLabels;
    wxString        m_AlbumLabelsStr;

    TagInfo()
    {
        m_TrackName = wxEmptyString;
        m_GenreName = wxEmptyString;
        m_ArtistName = wxEmptyString;
        m_AlbumName = wxEmptyString;
        m_Track = 0;
        m_Year = 0;
        m_Length = 0;
    };

    int         RemoveLabelFrame( ID3_Tag * tag, const char * Name );
    ID3_Frame * Find( ID3_Tag * tag, const char * Name );
    bool        ReadID3Tags( const ID3_Tag * tag );
    bool        WriteID3Tags( ID3_Tag * tag );

};


#endif
// -------------------------------------------------------------------------------- //
