// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#ifndef __DISCOGS_H__
#define __DISCOGS_H__

#include "CoverFetcher.h"

#include <wx/dynarray.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guDiscogsImage
{
  public :
    wxString    m_Url;
    int         m_Width;
    int         m_Height;

    guDiscogsImage() {}
    ~guDiscogsImage() {}
    guDiscogsImage( const wxString &url, int width = -1, int height = -1 )
    {
        m_Url = url;
        m_Width = width;
        m_Height = height;
    }

};
WX_DECLARE_OBJARRAY( guDiscogsImage, guDiscogsImageArray );

// -------------------------------------------------------------------------------- //
class guDiscogsTrack
{
  public :
    wxString m_Title;
    int      m_Length;
    int      m_Position;

    guDiscogsTrack() {}
    ~guDiscogsTrack() {}
};
WX_DECLARE_OBJARRAY( guDiscogsTrack, guDiscogsTrackArray );

// -------------------------------------------------------------------------------- //
class guDiscogsRelease
{
  public:
    int                     m_Id;
    wxString                m_Type;
    wxString                m_Format;
    wxString                m_Title;
    //wxString                m_Label;
    int                     m_Year;
    guDiscogsImageArray     m_Images;
    guDiscogsTrackArray     m_Tracks;

    guDiscogsRelease() {}
    ~guDiscogsRelease() {}
};
WX_DECLARE_OBJARRAY( guDiscogsRelease, guDiscogsReleaseArray );

// -------------------------------------------------------------------------------- //
class guDiscogsArtist
{
  public :
    wxString                m_Name;
    wxString                m_RealName;
    guDiscogsImageArray     m_Images;
    wxArrayString           m_Urls;
    //wxArrayString           m_Aliases;
    guDiscogsReleaseArray   m_Releases;

    guDiscogsArtist() {}
    ~guDiscogsArtist() {}
};

// -------------------------------------------------------------------------------- //
class guDiscogs
{
  protected :

  public :
    guDiscogs();
    ~guDiscogs();

    bool GetArtist( const wxString &name, guDiscogsArtist * artist );
    bool GetReleaseImages( const int id, guDiscogsRelease * release );
    bool GetRelease( const int id, guDiscogsRelease * release );
    guDiscogsReleaseArray   GetArtistReleases( const wxString &artist, const wxString &type = wxEmptyString, const wxString &format = wxEmptyString );
    //Search( const wxString &query, const int pagenum = 1 );

};

// -------------------------------------------------------------------------------- //
class guDiscogsCoverFetcher : public guCoverFetcher
{
  public :
    guDiscogsCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                    const wxChar * artist, const wxChar * album );
    ~guDiscogsCoverFetcher();

    virtual int   AddCoverLinks( int pagenum );
};

}

#endif
// -------------------------------------------------------------------------------- //
