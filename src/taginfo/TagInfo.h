// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
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
#ifndef __TAGINFO_H__
#define __TAGINFO_H__

#include "DbLibrary.h"


#include <tag.h>
#include <attachedpictureframe.h>
#include <fileref.h>
#include <id3v2framefactory.h>
#include <textidentificationframe.h>
#include <unsynchronizedlyricsframe.h>
#include <asffile.h>
#include <mpegfile.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpcfile.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <wavpackfile.h>
#include <trueaudiofile.h>

#include <xiphcomment.h>

#ifdef TAGLIB_WITH_APE_SUPPORT
#include <apefile.h>
#endif
#include <mp4tag.h>
#include <apetag.h>
#include <id3v2tag.h>
#include <asftag.h>

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/image.h>

#include <gst/gst.h>

namespace Guayadeque {

using namespace TagLib;

#define wxStringToTString(s) TagLib::String(s.ToUTF8(), TagLib::String::UTF8)
#define TStringTowxString(s) wxString::FromUTF8( s.toCString(true) )

#define     guTRACK_CHANGED_DATA_NONE           0
#define     guTRACK_CHANGED_DATA_TAGS           ( 1 << 0 )
#define     guTRACK_CHANGED_DATA_IMAGES         ( 1 << 1 )
#define     guTRACK_CHANGED_DATA_LYRICS         ( 1 << 2 )
#define     guTRACK_CHANGED_DATA_LABELS         ( 1 << 3 )
#define     guTRACK_CHANGED_DATA_RATING         ( 1 << 4 )

// -------------------------------------------------------------------------------- //
class guTagInfo
{
  protected :
    FileRef *       m_TagFile;
    Tag *           m_Tag;

  protected :
    bool            ReadExtendedTags( ID3v2::Tag * tag );
    bool            WriteExtendedTags( ID3v2::Tag * tag, const int changedflag );
    bool            ReadExtendedTags( Ogg::XiphComment * tag );
    bool            WriteExtendedTags( Ogg::XiphComment * tag, const int changedflag );
    bool            ReadExtendedTags( MP4::Tag * tag );
    bool            WriteExtendedTags( MP4::Tag * tag, const int changedflag );
    bool            ReadExtendedTags( APE::Tag * tag );
    bool            WriteExtendedTags( APE::Tag * tag, const int changedflag );
    bool            ReadExtendedTags( ASF::Tag * tag );
    bool            WriteExtendedTags( ASF::Tag * tag, const int changedflag );

  public:
    wxString        m_FileName;
    wxString        m_TrackName;
    wxString        m_GenreName;
    wxString        m_ArtistName;
    wxString        m_AlbumArtist;
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
    bool            m_Compilation;

    guTagInfo( const wxString &filename = wxEmptyString );
    virtual ~guTagInfo();

    void                SetFileName( const wxString &filename );
    virtual bool        Read( void );
    virtual bool        Write( const int changedflag );

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
    virtual bool        Write( const int changedflag );
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
    virtual bool        Write( const int changedflag );

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
    virtual bool        Write( const int changedflag );

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
    virtual bool        Write( const int changedflag );

#ifdef TAGLIB_WITH_MP4_COVERS
    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );
#endif

    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};

#ifdef TAGLIB_WITH_APE_SUPPORT
// -------------------------------------------------------------------------------- //
class guApeTagInfo : public guTagInfo
{
  protected :
    ID3v1::Tag *        m_TagId3v1;
    APE::Tag *          m_ApeTag;

  public :
    guApeTagInfo( const wxString &filename = wxEmptyString );
    ~guApeTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( const int changedflag );
    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};
#endif

// -------------------------------------------------------------------------------- //
class guMpcTagInfo : public guTagInfo
{
  protected :
    TagLib::APE::Tag * m_ApeTag;

  public :
    guMpcTagInfo( const wxString &filename = wxEmptyString );
    ~guMpcTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( const int changedflag );

    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );
};

// -------------------------------------------------------------------------------- //
class guWavPackTagInfo : public guTagInfo
{
  protected :
    TagLib::APE::Tag * m_ApeTag;

  public :
    guWavPackTagInfo( const wxString &filename = wxEmptyString );
    ~guWavPackTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( const int changedflag );

    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );
    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};

// -------------------------------------------------------------------------------- //
class guTrueAudioTagInfo : public guTagInfo
{
  protected :
    ID3v2::Tag *        m_TagId3v2;

  public :
    guTrueAudioTagInfo( const wxString &filename = wxEmptyString );
    ~guTrueAudioTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( const int changedflag );

    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );
    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};

// -------------------------------------------------------------------------------- //
class guASFTagInfo : public guTagInfo
{
  protected :
    ASF::Tag *        m_ASFTag;

  public :
    guASFTagInfo( const wxString &filename = wxEmptyString );
    ~guASFTagInfo();

    virtual bool        Read( void );
    virtual bool        Write( const int changedflag );

    virtual bool        CanHandleImages( void );
    virtual wxImage *   GetImage( void );
    virtual bool        SetImage( const wxImage * image );
    virtual bool        CanHandleLyrics( void );
    virtual wxString    GetLyrics( void );
    virtual bool        SetLyrics( const wxString &lyrics );
};

// -------------------------------------------------------------------------------- //
class guGStreamerTagInfo : public guTagInfo
{

  protected :
    const GstTagList *        m_GstTagList = NULL;

  public :
    guGStreamerTagInfo( const wxString &filename = wxEmptyString );
    ~guGStreamerTagInfo();

    virtual bool        Read( void );
    virtual wxString    GetLyrics( void );

    virtual bool        ReadGStreamerTags( const wxString &filename = wxEmptyString );
    virtual wxString    GetGstStrTag( const gchar * tag );
    virtual int         GetGstIntTag( const gchar * tag );
    virtual bool        GetGstBoolTag( const gchar * tag );
    virtual GDateTime * GetGstTimeTag( const gchar * tag );

};

class guImagePtrArray;

// -------------------------------------------------------------------------------- //
wxImage *   guTagGetPicture( const wxString &filename );
bool        guTagSetPicture( const wxString &filename, wxImage * picture, const bool forcesave = false );
bool        guTagSetPicture( const wxString &filename, const wxString &imagefile, const bool forcesave = false );
wxString    guTagGetLyrics( const wxString &filename );
bool        guTagSetLyrics( const wxString &filename, const wxString &lyrics, const bool forcesave = false );
//void        guUpdateTrack( const guTrack &track, const wxImage * image, const wxString &lyrics, const int &changedflags );
void        guUpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                    const wxArrayString &lyrics, const wxArrayInt &changedflags, const bool forcesave = false );
void        guUpdateImages( const guTrackArray &songs, const guImagePtrArray &images, const wxArrayInt &changedflags );
void        guUpdateLyrics( const guTrackArray &songs, const wxArrayString &lyrics, const wxArrayInt &changedflags );
bool        guStrDiskToDiskNum( const wxString &diskstr, int &disknum, int &disktotal );

}

#endif
// -------------------------------------------------------------------------------- //
