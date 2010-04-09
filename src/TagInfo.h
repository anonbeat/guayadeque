// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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


#include <tag.h>
#include <attachedpictureframe.h>
#include <fileref.h>
#include <id3v2framefactory.h>
#include <textidentificationframe.h>
#include <unsynchronizedlyricsframe.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <flacfile.h>
#include <mpcfile.h>
#include <oggfile.h>
#include <vorbisfile.h>

#include <xiphcomment.h>

#include <mp4tag.h>
#include <mp4file.h>

// FLAC Dev files
#include <FLAC/metadata.h>
#include <FLAC/format.h>


#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/image.h>

using namespace TagLib;

#define wxStringToTString(s) TagLib::String(s.ToUTF8(), TagLib::String::UTF8)
#define TStringTowxString(s) wxString::FromUTF8( s.toCString(true) )

// -------------------------------------------------------------------------------- //
class guTagInfo
{
  protected :
    FileRef *       m_TagFile;
    Tag *           m_Tag;

  public:
    wxString        m_FileName;
    wxString        m_TrackName;
    wxString        m_GenreName;
    wxString        m_ArtistName;
    wxString        m_AlbumName;
    wxString        m_Composer;
    wxString        m_Comments;
    int             m_Track;
    int             m_Year;
    int             m_Length;
    int             m_Bitrate;
    int             m_PlayCount;
    int             m_Rating;
    wxString        m_Disk;
    wxArrayString   m_TrackLabels;
    wxString        m_TrackLabelsStr;
    wxArrayString   m_ArtistLabels;
    wxString        m_ArtistLabelsStr;
    wxArrayString   m_AlbumLabels;
    wxString        m_AlbumLabelsStr;

    guTagInfo( const wxString &filename = wxEmptyString );
    ~guTagInfo();

    void                SetFileName( const wxString &filename );
    virtual bool        Read( void );
    virtual bool        Write( void );

    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );

    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );

};

bool guIsValidAudioFile( const wxString &filename );
guTagInfo * guGetTagInfoHandler( const wxString &filename );

// -------------------------------------------------------------------------------- //
class guMp3TagInfo : public guTagInfo
{
  protected :
    ID3v2::Tag *        m_TagId3v2;

  public :
    guMp3TagInfo( const wxString &filename = wxEmptyString );
    ~guMp3TagInfo();

    virtual bool        Read( void );
    virtual bool        Write( void );
    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );

    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );

};

// -------------------------------------------------------------------------------- //
class guFlacTagInfo : public guTagInfo
{
  protected :
    Ogg::XiphComment * m_XiphComment;

  public :
    guFlacTagInfo( const wxString &filename = wxEmptyString );
    ~guFlacTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( void );

    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage();
    virtual bool        SetImage( const wxImage * image );

    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};

// -------------------------------------------------------------------------------- //
class guOggTagInfo : public guTagInfo
{
  protected :
    Ogg::XiphComment * m_XiphComment;

  public :
    guOggTagInfo( const wxString &filename = wxEmptyString );
    ~guOggTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( void );

    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );

    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};

// -------------------------------------------------------------------------------- //
class guMp4TagInfo : public guTagInfo
{
  protected :
    TagLib::MP4::Tag *  m_Mp4Tag;

  public :
    guMp4TagInfo( const wxString &filename = wxEmptyString );
    ~guMp4TagInfo();

    virtual bool        Read( void );
    virtual bool        Write( void );

//    virtual bool        CanHandleImages( void );  // Need taglib 1.6.1 to implement it as I dont want to depend on libmp4
//    virtual wxImage *   GetImage( void );
//    virtual bool        SetImage( const wxImage * image );

    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};

// -------------------------------------------------------------------------------- //
class guMpcTagInfo : public guTagInfo
{
  public :
    guMpcTagInfo( const wxString &filename = wxEmptyString );
    ~guMpcTagInfo();

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

// -------------------------------------------------------------------------------- //
wxImage *   guTagGetPicture( const wxString &filename );
bool        guTagSetPicture( const wxString &filename, wxImage * picture );
wxString    guTagGetLyrics( const wxString &filename );
bool        guTagSetPicture( const wxString &filename, wxString &lyrics );
void        UpdateImages( const guTrackArray &songs, const guImagePtrArray &images );
void        UpdateLyrics( const guTrackArray &songs, const wxArrayString &lyrics );

#endif
// -------------------------------------------------------------------------------- //
