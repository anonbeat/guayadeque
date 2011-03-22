// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#ifndef MUSICBRAINZ_H
#define MUSICBRAINZ_H

#include "DbLibrary.h"
#include "MusicDns.h"

#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/thread.h>
#include <wx/xml/xml.h>

#define GetTrackLengthDiff( time1, time2 )      abs( ( int ) time1 - time2 )
#define guMBRAINZ_MAX_TIME_DIFF                 3000

// -------------------------------------------------------------------------------- //
class guMBTrack
{
  public :
    wxString        m_Id;
    wxString        m_Title;
    int             m_Length;
    wxString        m_ArtistId;
    wxString        m_ArtistName;
    wxString        m_ReleaseId;
    wxString        m_ReleaseName;
    int             m_Number;

    guMBTrack() { m_Length = 0; m_Number = 0; }

};
WX_DECLARE_OBJARRAY( guMBTrack, guMBTrackArray );

int FindMBTracksReleaseId( guMBTrackArray * mbtracks, const wxString &releaseid );

// -------------------------------------------------------------------------------- //
class guMBRelease
{
  public :
    wxString        m_Id;
    wxString        m_Title;
    wxString        m_ArtistId;
    wxString        m_ArtistName;
    wxArrayString   m_Events;
    guMBTrackArray  m_Tracks;
};
WX_DECLARE_OBJARRAY( guMBRelease, guMBReleaseArray );


// -------------------------------------------------------------------------------- //
class guMusicBrainz
{
  protected :
    bool m_ReceivedPUId;

  public :
    guMusicBrainz();
    ~guMusicBrainz();

    void GetTracks( guMBTrackArray * mbtracks, const wxString &trackpuid, int tracklength = wxNOT_FOUND );
    void GetTracks( guMBTrackArray * mbtracks, const guTrack * track, int tracklength = wxNOT_FOUND );

    void GetRelease( guMBRelease * mbrelease, const wxString &releaseid );
    void GetReleases( guMBReleaseArray * mbreleases, const wxString &artist, const wxString &title );
    //void GetTracksMetadata( guTrackArray * tracks );

    void FoundPUID( const wxString &puid ); // Method called from MusicDns
};


#endif
// -------------------------------------------------------------------------------- //
