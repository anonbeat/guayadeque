// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
//	anonbeat@gmail.com
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
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef APETAG_H
#define APETAG_H

#include "Utils.h"

#include <wx/dynarray.h>
#include <wx/file.h>
#include <wx/string.h>
#include <wx/defs.h>

#define APE_TAG_KEY_TITLE               wxT( "Title" )
#define APE_TAG_KEY_SUBTITLE            wxT( "Subtitle" )
#define APE_TAG_KEY_ARTIST              wxT( "Artist" )
#define APE_TAG_KEY_ALBUM               wxT( "Album" )
#define APE_TAG_KEY_DEBUTALBUM          wxT( "Debut Album" )
#define APE_TAG_KEY_PUBLISHER           wxT( "Publisher" )
#define APE_TAG_KEY_CONDUCTOR           wxT( "Conductor" )
#define APE_TAG_KEY_TRACK               wxT( "Track" )
#define APE_TAG_KEY_COMPOSER            wxT( "Composer" )
#define APE_TAG_KEY_COMMENT             wxT( "Comment" )
#define APE_TAG_KEY_COPYRIGHT           wxT( "Copyright" )
#define APE_TAG_KEY_PUBLICATIONRIGHT    wxT( "Publicationright" )
#define APE_TAG_KEY_FILE                wxT( "File" )
#define APE_TAG_KEY_EANUPC              wxT( "EAN/UPC" )
#define APE_TAG_KEY_ISBN                wxT( "ISBN" )
#define APE_TAG_KEY_CATALOG             wxT( "Catalog" )
#define APE_TAG_KEY_LC                  wxT( "LC" )
#define APE_TAG_KEY_YEAR                wxT( "Year" )
#define APE_TAG_KEY_RECORDDATE          wxT( "Record Date" )
#define APE_TAG_KEY_RECORDLOCATION      wxT( "Record Location" )
#define APE_TAG_KEY_GENRE               wxT( "Genre" )
#define APE_TAG_KEY_MEDIA               wxT( "Media" )
#define APE_TAG_KEY_INDEX               wxT( "Index" )
#define APE_TAG_KEY_RELATED_URL         wxT( "Related" )
#define APE_TAG_KEY_ISRC                wxT( "ISRC" )
#define APE_TAG_KEY_ABSTRACT_URL        wxT( "Abstract" )
#define APE_TAG_KEY_LANGUAGE            wxT( "Language" )
#define APE_TAG_KEY_BIBLIOGRAPHY_URL    wxT( "Bibliography" )
#define APE_TAG_KEY_INTROPLAY           wxT( "Introplay" )
#define APE_TAG_KEY_DUMMY               wxT( "Dummy" )

#define APE_TAG_KEY_COVER_ART_FRONT     wxT( "Cover Art (front)" )
#define APE_TAG_KEY_COVER_ART_OTHER     wxT( "Cover Art (other)" )
#define APE_TAG_KEY_NOTES               wxT( "Notes" )
#define APE_TAG_KEY_LYRICS              wxT( "Lyrics" )
#define APE_TAG_KEY_BUY_URL             wxT( "Buy URL" )
#define APE_TAG_KEY_ARTIST_URL          wxT( "Artist URL" )
#define APE_TAG_KEY_PUBLISHER_URL       wxT( "Publisher URL" )
#define APE_TAG_KEY_FILE_URL            wxT( "File URL" )
#define APE_TAG_KEY_COPYRIGHT_URL       wxT( "Copyright URL" )
#define APE_TAG_KEY_MJ_METADATA         wxT( "Media Jukebox Metadata" )

#define APE_TAG_KEY_ALBUMARTIST         wxT( "Album Artist" )

#define APE_FLAG_HAVE_HEADER            0x80000000
#define APE_FLAG_HAVE_FOOTER            0x40000000
#define APE_FLAG_IS_HEADER              0x20000000

#define APE_FLAG_CONTENT_TYPE           0x00000006
#define APE_FLAG_CONTENT_TEXT           0x00000000
#define APE_FLAG_CONTENT_BINARY         0x00000002
#define APE_FLAG_CONTENT_EXTERNAL       0x00000004

#define APE_FLAG_IS_READONLY            0x00000001

// -------------------------------------------------------------------------------- //
class guApeItem
{
  public:
    wxString    m_Key;
    wxString    m_Value;
    wxUint32    m_Flags;

    guApeItem() {};

    guApeItem( const wxString &key, const wxString &value, wxUint32 flags )
    {
        m_Key = key;
        m_Value = value;
        m_Flags = flags;
    }

    const wxString & Key( void ) const
    {
        return m_Key;
    }

    const wxString & Value( void ) const
    {
        return m_Value;
    }

    const wxUint32 Flags( void ) const
    {
        return m_Flags;
    }
};

// -------------------------------------------------------------------------------- //
WX_DEFINE_SORTED_ARRAY( guApeItem *, guApeItemArray );

int guCompareApeItems( guApeItem * item1, guApeItem * item2 );

// -------------------------------------------------------------------------------- //
class guApeTag
{
  protected :
    wxUint32            m_FileLength;
    wxUint32            m_TagOffset;
    wxUint32            m_ItemCount;
    guApeItemArray *    m_Items;

  public:

    guApeTag( wxUint32 length, wxUint32 offset, wxUint32 items );
    ~guApeTag();

    void            DelAllItems( void );
    void            DelItem( guApeItem * item );
    void            AddItem( guApeItem * item );
    guApeItem *     GetItem( const int pos ) const;
    guApeItem *     GetItem( const wxString &key ) const;
    wxString        GetItemValue( const wxString &key ) const;
    void            SetItem( const wxString &key, const wxString &value, wxUint32 flags = APE_FLAG_CONTENT_TEXT );
    void            SetItem( const wxString &key, char * data, wxUint32 len );
    wxUint32        FileLength( void ) const;
    wxUint32        TagOffset( void ) const;
    wxUint32        ItemLength( void ) const;
    wxUint32        ItemCount( void ) const;

    wxString        GetTitle( void ) const;
    void            SetTitle( const wxString &title );
    wxString        GetArtist( void ) const;
    void            SetArtist( const wxString &artist );
    wxString        GetAlbum( void ) const;
    void            SetAlbum( const wxString &album );
    wxString        GetGenre( void ) const;
    void            SetGenre( const wxString &genre );
    wxUint32        GetTrack( void ) const;
    void            SetTrack( const wxUint32 track );
    wxUint32        GetYear( void ) const;
    void            SetYear( const wxUint32 year );

    friend class   guApeFile;
};

// -------------------------------------------------------------------------------- //
class guApeFile
{
  private :
    wxString    m_FileName;
    wxUint32    m_TrackLength;
    wxUint32    m_BitRate;
    wxFile *    m_File;
    guApeTag *  m_Tag;

    void        ReadAndProcessApeHeader( void );
    void        WriteApeHeaderFooter( const wxUint32 flags );
    void        WriteApeItems( void );
    void inline WriteInt( const int value );

  public :
    guApeFile( const wxString &filename );
    ~guApeFile();

    bool        WriteApeTag( void );

    guApeTag * GetApeTag()
    {
        return m_Tag;
    };

    wxUint32 GetBitRate( void )
    {
        return m_BitRate;
    }

    wxUint32 GetTrackLength( void )
    {
        return m_TrackLength;
    }

};

#endif
// -------------------------------------------------------------------------------- //
