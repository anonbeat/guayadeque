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

#include "DbLibrary.h"

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/image.h>

#define wxStringToTString(s) TagLib::String(s.ToUTF8(), TagLib::String::UTF8)
#define TStringTowxString(s) wxString::FromUTF8( s.toCString(true))

// -------------------------------------------------------------------------------- //
class guTagInfo
{
  public:
    wxString        m_FileName;
    wxString        m_TrackName;
    wxString        m_GenreName;
    wxString        m_ArtistName;
    wxString        m_AlbumName;
    int             m_Track;
    int             m_Year;
    int             m_Length;
    int             m_Bitrate;
    int             m_PlayCount;
    int             m_Rating;
    wxArrayString   m_TrackLabels;
    wxString        m_TrackLabelsStr;
    wxArrayString   m_ArtistLabels;
    wxString        m_ArtistLabelsStr;
    wxArrayString   m_AlbumLabels;
    wxString        m_AlbumLabelsStr;

    guTagInfo( const wxString &filename = wxEmptyString )
    {
        m_FileName = filename;
        m_TrackName = wxEmptyString;
        m_GenreName = wxEmptyString;
        m_ArtistName = wxEmptyString;
        m_AlbumName = wxEmptyString;
        m_Track = 0;
        m_Year = 0;
        m_Length = 0;
        m_Bitrate = 0;
        m_TrackLabels.Empty();
        m_TrackLabelsStr = wxEmptyString;
        m_ArtistLabels.Empty();
        m_ArtistLabelsStr = wxEmptyString;
        m_AlbumLabels.Empty();
        m_AlbumLabelsStr = wxEmptyString;
    };

    ~guTagInfo(){};

    void                SetFileName( const wxString &filename ) { m_FileName = filename; };
    virtual bool        Read( void );
    virtual bool        Write( void );
    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );

};

bool guIsValidAudioFile( const wxString &filename );
guTagInfo * guGetTagInfoHandler( const wxString &filename );

// -------------------------------------------------------------------------------- //
class guMp3TagInfo : public guTagInfo
{
  public :
    guMp3TagInfo( const wxString &filename = wxEmptyString );
    ~guMp3TagInfo();

    virtual bool        Read( void );
    virtual bool        Write( void );
    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );

};

// -------------------------------------------------------------------------------- //
class guFlacTagInfo : public guTagInfo
{
  public :
    guFlacTagInfo( const wxString &filename = wxEmptyString );
    ~guFlacTagInfo();

//    virtual bool        CanHandleImages( void );
//    virtual wxImage *   GetImage();
//    virtual bool        SetImage( const wxImage * image );
};

// -------------------------------------------------------------------------------- //
class guOggTagInfo : public guTagInfo
{
  public :
    guOggTagInfo( const wxString &filename = wxEmptyString );
    ~guOggTagInfo();

//    virtual bool        CanHandleImages( void );
//    virtual wxImage *   GetImage( const wxString &filename );
//    virtual bool        SetImage( const wxString &filename, const wxImage * image );
};

// -------------------------------------------------------------------------------- //
class guMpcTagInfo : public guTagInfo
{
  public :
    guMpcTagInfo( const wxString &filename = wxEmptyString );
    ~guMpcTagInfo();

};

// -------------------------------------------------------------------------------- //
class guMp4TagInfo : public guTagInfo
{
  public :
    guMp4TagInfo( const wxString &filename = wxEmptyString );
    ~guMp4TagInfo();

    virtual bool        Read( void );
    virtual bool        Write( void );
    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );
};

// -------------------------------------------------------------------------------- //
class guApeTagInfo : public guTagInfo
{
  public :
    guApeTagInfo( const wxString &filename = wxEmptyString );
    ~guApeTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( void );
};


class guImagePtrArray;

wxImage *   ID3TagGetPicture( const wxString &filename );
bool        ID3TagSetPicture( const wxString &filename, wxImage * picture );
void        UpdateImages( const guTrackArray &Songs, const guImagePtrArray &Images );


#endif
// -------------------------------------------------------------------------------- //
