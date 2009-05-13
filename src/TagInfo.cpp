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
#include "TagInfo.h"
#include "Utils.h"

#include <tag.h>
#include <fileref.h>
#include <textidentificationframe.h>
#include <id3v2framefactory.h>
#include <id3v2tag.h>
#include <mpegfile.h>

#include <wx/tokenzr.h>

using namespace TagLib;

// -------------------------------------------------------------------------------- //
bool TagInfo::ReadID3Tags( const wxString &FileName )
{
    FileRef fileref = TagLib::FileRef( FileName.ToUTF8(), true, TagLib::AudioProperties::Fast );
    Tag * tag;
    AudioProperties * apro;

    if( !fileref.isNull() )
    {
        if( ( tag = fileref.tag() ) )
        {
            m_TrackName = wxString( tag->title().toCString( true ), wxConvUTF8 );
            m_ArtistName = wxString( tag->artist().toCString( true ), wxConvUTF8 );
            m_AlbumName = wxString( tag->album().toCString( true ), wxConvUTF8 );
            m_GenreName = wxString( tag->genre().toCString( true ), wxConvUTF8 );
            m_Track = tag->track();
            m_Year = tag->year();

        }

        apro = fileref.audioProperties();
        if( apro )
        {
            m_Length = apro->length();
        }
        else
        {
            guLogWarning( wxT( "Cant read audio properties from %s\n" ), FileName.c_str() );
        }


        // If its a ID3v2 Tag try to load the labels
        ID3v2::Tag * tagv2 = ( ( TagLib::MPEG::File * ) fileref.file() )->ID3v2Tag();
        if( tagv2 )
        {
            if( m_TrackLabels.Count() == 0 )
            {
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tagv2, "guTRLABELS" );
                if( Frame )
                {
                    m_TrackLabelsStr = wxString( Frame->toString().toCString( true ), wxConvUTF8 );
                    guLogMessage( wxT( "Track Label: '%s'\n" ), m_TrackLabelsStr.c_str() );
                    m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }

            if( m_ArtistLabels.Count() == 0 )
            {
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tagv2, "guARLABELS" );
                if( Frame )
                {
                    m_ArtistLabelsStr = wxString( Frame->toString().toCString( true ), wxConvUTF8 );
                    guLogMessage( wxT( "Artist Label: '%s'\n" ), m_ArtistLabelsStr.c_str() );
                    m_ArtistLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }

            if( m_AlbumLabels.Count() == 0 )
            {
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tagv2, "guALLABELS" );
                if( Frame )
                {
                    m_AlbumLabelsStr = wxString( Frame->toString().toCString( true ), wxConvUTF8 );
                    guLogMessage( wxT( "Album Label: '%s'\n" ), m_AlbumLabelsStr.c_str() );
                    m_AlbumLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }
        }
    }
    else
    {
      guLogError( wxT( "Could not read tags from file %s\n" ), FileName.c_str() );
    }

    return true;
}


// -------------------------------------------------------------------------------- //
void ID3v2_CheckLabelFrame( ID3v2::Tag * tagv2, const char * description, const wxString &value )
{
    ID3v2::UserTextIdentificationFrame * frame;
    frame = ID3v2::UserTextIdentificationFrame::find( tagv2, description );
    if( frame )
    {
        if( value.IsEmpty() )
        {
            while( frame )
            {
                // remove the frame
                tagv2->removeFrame( frame );
                frame = ID3v2::UserTextIdentificationFrame::find( tagv2, description );
            }
        }
        else
        {
            frame->setText( ( char * ) value.char_str( wxConvISO8859_1 ) );
        }
    }
    else
    {
        if( !value.IsEmpty() )
        {
            frame = new ID3v2::UserTextIdentificationFrame( TagLib::String::UTF8 );
            frame->setDescription( TagLib::String( description, TagLib::String::UTF8 ) );
            frame->setText( ( char * ) value.char_str( wxConvUTF8 ) );
            tagv2->addFrame( frame );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool TagInfo::WriteID3Tags( const wxString &FileName )
{
    FileRef fileref = TagLib::FileRef( FileName.ToUTF8() );
    Tag * tag;

    if( !fileref.isNull() )
    {
        if( ( tag = fileref.tag() ) )
        {
            tag->setTitle( ( char * ) m_TrackName.char_str( wxConvISO8859_1 ) );
            tag->setArtist( ( char * ) m_ArtistName.char_str( wxConvISO8859_1 ) );
            tag->setAlbum( ( char * ) m_AlbumName.char_str( wxConvISO8859_1 ) );
            tag->setGenre( ( char * ) m_GenreName.char_str( wxConvISO8859_1 ) );
            tag->setTrack( m_Track );
            tag->setYear( m_Year );
        }

        // Check if we have a id3v2 Tag
        ID3v2::Tag * tagv2 = ( ( TagLib::MPEG::File * ) fileref.file() )->ID3v2Tag();
        if( tagv2 )
        {

            // The Labels
            if( m_ArtistLabelsStr.IsEmpty() )
            {
                if( m_ArtistLabels.Count() )
                {
                    int index;
                    int count = m_ArtistLabels.Count();
                    for( index = 0; index < count; index++ )
                    {
                        m_ArtistLabelsStr += m_ArtistLabels[ index ];
                        m_ArtistLabelsStr += wxT( "|" );
                    }
                    m_ArtistLabelsStr.RemoveLast( 1 );
                }
            }


            if( m_AlbumLabelsStr.IsEmpty() )
            {
                if( m_AlbumLabels.Count() )
                {
                    int index;
                    int count = m_AlbumLabels.Count();
                    for( index = 0; index < count; index++ )
                    {
                        m_AlbumLabelsStr += m_AlbumLabels[ index ];
                        m_AlbumLabelsStr += wxT( "|" );
                    }
                    m_AlbumLabelsStr.RemoveLast( 1 );
                }
            }


            if( m_TrackLabelsStr.IsEmpty() )
            {
                if( m_TrackLabels.Count() )
                {
                    int index;
                    int count = m_TrackLabels.Count();
                    for( index = 0; index < count; index++ )
                    {
                        m_TrackLabelsStr += m_TrackLabels[ index ];
                        m_TrackLabelsStr += wxT( "|" );
                    }
                    m_TrackLabelsStr.RemoveLast( 1 );
                }
            }

            ID3v2_CheckLabelFrame( tagv2, "guARLABELS", m_ArtistLabelsStr );
            ID3v2_CheckLabelFrame( tagv2, "guALLABELS", m_AlbumLabelsStr );
            ID3v2_CheckLabelFrame( tagv2, "guTRLABELS", m_TrackLabelsStr );

        }

        if( !fileref.save() )
          guLogWarning( _( "iD3Tags Save failed" ) );

    }
    else
    {
      guLogError( wxT( "Could not read tags from file %s\n" ), FileName.c_str() );
    }



//////  // The Labels
//////  if( m_ArtistLabelsStr.IsEmpty() )
//////  {
//////    if( m_ArtistLabels.Count() )
//////    {
//////      int index;
//////      int count = m_ArtistLabels.Count();
//////      for( index = 0; index < count; index++ )
//////      {
//////        m_ArtistLabelsStr += m_ArtistLabels[ index ];
//////        m_ArtistLabelsStr += wxT( "|" );
//////      }
//////      m_ArtistLabelsStr.RemoveLast( 1 );
//////    }
//////  }
//////
//////
////////  if( !ArtistLabelsStr.IsEmpty() )
//////  {
//////    frame = Find( tag, "guARLABELS" );
//////    if( !frame )
//////    {
//////      frame = new ID3_Frame( ID3FID_USERTEXT );
//////      frame->Field( ID3FN_TEXTENC ) = ID3TE_ASCII;
//////      frame->Field( ID3FN_DESCRIPTION ) = "guARLABELS";
//////      tag->AttachFrame( frame );
//////    }
//////    frame->Field( ID3FN_TEXT ) = m_ArtistLabelsStr.char_str();
//////  }
//////
//////
//////  if( m_AlbumLabelsStr.IsEmpty() )
//////  {
//////    if( m_AlbumLabels.Count() )
//////    {
//////      int index;
//////      int count = m_AlbumLabels.Count();
//////      for( index = 0; index < count; index++ )
//////      {
//////        m_AlbumLabelsStr += m_AlbumLabels[ index ];
//////        m_AlbumLabelsStr += wxT( "|" );
//////      }
//////      m_AlbumLabelsStr.RemoveLast( 1 );
//////    }
//////  }
//////
//////
////////  if( !AlbumLabelsStr.IsEmpty() )
//////  {
//////    frame = Find( tag, "guALLABELS" );
//////    if( !frame )
//////    {
//////      //guLogMessage( wxT( "Creating guALLABELS Frame..." ) );
//////      frame = new ID3_Frame( ID3FID_USERTEXT );
//////      frame->Field( ID3FN_TEXTENC ) = ID3TE_ASCII;
//////      frame->Field( ID3FN_DESCRIPTION ) = "guALLABELS";
//////      tag->AttachFrame( frame );
//////    }
//////    frame->Field( ID3FN_TEXT ) = m_AlbumLabelsStr.char_str();
//////  }
//////
//////
//////  if( m_TrackLabelsStr.IsEmpty() )
//////  {
//////    if( m_TrackLabels.Count() )
//////    {
//////      int index;
//////      int count = m_TrackLabels.Count();
//////      for( index = 0; index < count; index++ )
//////      {
//////        m_TrackLabelsStr += m_TrackLabels[ index ];
//////        m_TrackLabelsStr += wxT( "|" );
//////      }
//////      m_TrackLabelsStr.RemoveLast( 1 );
//////    }
//////  }
//////
//////
////////  if( !TrackLabelsStr.IsEmpty() )
//////  {
//////    frame = Find( tag, "guTRLABELS" );
//////    if( !frame )
//////    {
//////      frame = new ID3_Frame( ID3FID_USERTEXT );
//////      frame->Field( ID3FN_TEXTENC ) = ID3TE_ASCII;
//////      frame->Field( ID3FN_DESCRIPTION ) = "guTRLABELS";
//////      tag->AttachFrame( frame );
//////    }
//////    frame->Field( ID3FN_TEXT ) = m_TrackLabelsStr.char_str();
//////  }
//////
//////  tag->SetPadding( false );
//////  tag->Update();
//////
  return true;
}

// -------------------------------------------------------------------------------- //

//char* taglib_id3v2_get_user_string(TagLib_File *file,char* description){
//  ID3v2::Tag* t = get_id3v2_tag(file);
//
//  if (t){
//    ID3v2::UserTextIdentificationFrame* f=ID3v2::UserTextIdentificationFrame::find(t,String(description, unicodeStrings ? String::UTF8 : String::Latin1));
//    if (f) {
//      char *s=NULL;
//      s = ::strdup(f->toString().toCString(unicodeStrings));
//      if(stringManagementEnabled)
//        strings.append(s);
//      return s;
//    }
//  }
//  return 0;
//}
//
//BOOL taglib_id3v2_set_user_string(TagLib_File *file,const char* string,char* description){
//  ID3v2::Tag* t = get_id3v2_tag(file);
//
//  if (t){
//    ID3v2::UserTextIdentificationFrame* f=ID3v2::UserTextIdentificationFrame::find(t,String(description, unicodeStrings ? String::UTF8 : String::Latin1));
//    if (f) {
//      f->setText(String(string, unicodeStrings ? String::UTF8 : String::Latin1));
//    } else {
//      f=new ID3v2::UserTextIdentificationFrame(unicodeStrings ? String::UTF8 : String::Latin1);
//      f->setDescription(String(description, unicodeStrings ? String::UTF8 : String::Latin1));
//      f->setText(String(string, unicodeStrings ? String::UTF8 : String::Latin1));
//      t->addFrame(f);
//    }
//    return 1;
//  }
//  return 0;
//}
