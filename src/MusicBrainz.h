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
#ifndef MUSICBRAINZ_H
#define MUSICBRAINZ_H

#include "DbLibrary.h"
#include "AcousticId.h"

#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/thread.h>
#include <wx/xml/xml.h>

#define GetTrackLengthDiff( time1, time2 )      abs( ( int ) time1 - time2 )
#define guMBRAINZ_MAX_TIME_DIFF                 3000

// -------------------------------------------------------------------------------- //
class guMBRecording
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

    guMBRecording() { m_Length = 0; m_Number = 0; }

};
WX_DECLARE_OBJARRAY( guMBRecording, guMBRecordingArray );

// -------------------------------------------------------------------------------- //
class guMBRelease
{
  public :
    wxString        m_Id;
    wxString        m_Title;
    wxString        m_ArtistId;
    wxString        m_ArtistName;
    int             m_Year;
    guMBRecordingArray  m_Recordings;
};
WX_DECLARE_OBJARRAY( guMBRelease, guMBReleaseArray );

int FindMBReleaseId( guMBReleaseArray * releases, const wxString &releaseid );

// -------------------------------------------------------------------------------- //
class guMusicBrainz
{
  protected :

  public :
    guMusicBrainz();
    virtual ~guMusicBrainz();

    int             GetReleases( const wxString &artist, const wxString &title, guMBReleaseArray * releases );
    int             GetReleases( const wxString &recordid, guMBReleaseArray * releases );

    int             GetRecordings( const wxString &releaseid, guMBRecordingArray * recordings );
    int             GetRecordings( guMBRelease &mbrelease );

    void            GetRecordingInfo( const wxString &recordingid, guMBRecording * mbrecording );
};


#endif
// -------------------------------------------------------------------------------- //
