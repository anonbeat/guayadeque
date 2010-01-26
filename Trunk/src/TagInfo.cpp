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

#include "ApeTag.h"
#include "Base64.h"
#include "TrackEdit.h"

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


#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>

using namespace TagLib;


// -------------------------------------------------------------------------------- //
bool guIsValidAudioFile( const wxString &filename )
{
    wxString FileName = filename.Lower();
    if( FileName.EndsWith( wxT( ".mp3"  ) ) ||
        FileName.EndsWith( wxT( ".flac" ) ) ||
        FileName.EndsWith( wxT( ".ogg"  ) ) ||
        FileName.EndsWith( wxT( ".oga"  ) ) ||
        FileName.EndsWith( wxT( ".mp4"  ) ) ||  // MP4 files
        FileName.EndsWith( wxT( ".m4a"  ) ) ||
        FileName.EndsWith( wxT( ".m4b"  ) ) ||
        FileName.EndsWith( wxT( ".m4p"  ) ) ||
        FileName.EndsWith( wxT( ".wma"  ) ) ||
        FileName.EndsWith( wxT( ".aac"  ) ) ||
        FileName.EndsWith( wxT( ".ape"  ) ) ||
        FileName.EndsWith( wxT( ".wav"  ) ) ||
        FileName.EndsWith( wxT( ".aif"  ) ) ||
        FileName.EndsWith( wxT( ".mpc"  ) ) )
    {
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guTagInfo * guGetTagInfoHandler( const wxString &filename )
{
    if( filename.Lower().EndsWith( wxT( ".mp3" ) ) )
    {
        return new guMp3TagInfo( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".flac" ) ) )
    {
        return new guFlacTagInfo( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".ogg" ) ) ||
             filename.Lower().EndsWith( wxT( ".oga" ) ) )
    {
        return new guOggTagInfo( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".mpc" ) ) )
    {
        return new guMpcTagInfo( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".ape" ) ) )
    {
        return new guApeTagInfo( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".mp4" ) ) ||
            filename.Lower().EndsWith( wxT( ".m4a" ) ) ||
            filename.Lower().EndsWith( wxT( ".m4b" ) ) ||
            filename.Lower().EndsWith( wxT( ".m4p" ) ) ||
            filename.Lower().EndsWith( wxT( ".aac" ) ) )
    {
        return new guMp4TagInfo( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".wma" ) ) ||
             filename.Lower().EndsWith( wxT( ".asf" ) ) ||
             filename.Lower().EndsWith( wxT( ".wav" ) ) )
    {
        return new guTagInfo( filename );
    }
    else
    {
        return NULL;
    }
}

// -------------------------------------------------------------------------------- //
wxImage * GetID3v2Image( ID3v2::Tag * tagv2 )
{
	TagLib::ID3v2::FrameList frameList = tagv2->frameList( "APIC" );
	if( !frameList.isEmpty() )
	{
		TagLib::ID3v2::AttachedPictureFrame * PicFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame * >( frameList.front() );
        int ImgDataSize = PicFrame->picture().size();

		if( ImgDataSize > 0 )
		{
			//guLogMessage( wxT( "ID3v2 header contains APIC frame with %u bytes." ), ImgDataSize );
            wxMemoryOutputStream ImgOutStream;
            ImgOutStream.Write( PicFrame->picture().data(), ImgDataSize );
            wxMemoryInputStream ImgInputStream( ImgOutStream );
            wxImage * CoverImage = new wxImage( ImgInputStream, wxString( PicFrame->mimeType().toCString( true ), wxConvUTF8 ) );
            if( CoverImage )
            {
                if( CoverImage->IsOk() )
                {
                    return CoverImage;
                }
                else
                {
                    delete CoverImage;
                }
            }
//		    wxFileOutputStream FOut( wxT( "~/test.jpg" ) );
//		    FOut.Write( PicFrame->picture().data(), ImgDataSize );
//		    FOut.Close();
		}
	}
	return NULL;
}

// -------------------------------------------------------------------------------- //
void SetID3v2Image( ID3v2::Tag * tagv2, const wxImage * image )
{
    TagLib::ID3v2::AttachedPictureFrame * PicFrame;
    if( image )
    {
        PicFrame = new TagLib::ID3v2::AttachedPictureFrame;
        PicFrame->setMimeType( "image/jpeg" );
        PicFrame->setType( TagLib::ID3v2::AttachedPictureFrame::FrontCover );
        wxMemoryOutputStream ImgOutputStream;
        if( image->SaveFile( ImgOutputStream, wxBITMAP_TYPE_JPEG ) )
        {
            ByteVector ImgData( ( TagLib::uint ) ImgOutputStream.GetSize() );
            ImgOutputStream.CopyTo( ImgData.data(), ImgOutputStream.GetSize() );
            PicFrame->setPicture( ImgData );
            tagv2->addFrame( PicFrame );
        }
    }
    else
    {
        TagLib::ID3v2::FrameList FrameList = tagv2->frameListMap()["APIC"];
        for( std::list<TagLib::ID3v2::Frame*>::iterator iter = FrameList.begin(); iter != FrameList.end(); iter++ )
        {
            PicFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>( *iter );
            tagv2->removeFrame( PicFrame, TRUE );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString GetID3v2Lyrics( ID3v2::Tag * tagv2 )
{
	TagLib::ID3v2::FrameList frameList = tagv2->frameList( "USLT" );
	if( !frameList.isEmpty() )
	{
		TagLib::ID3v2::UnsynchronizedLyricsFrame * LyricsFrame = static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame * >( frameList.front() );
        if( LyricsFrame )
        {
            //guLogMessage( wxT( "Found lyrics" ) );
            return TStringTowxString( LyricsFrame->text() );
        }
	}
	return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void SetID3v2Lyrics( ID3v2::Tag * tagv2, const wxString &lyrics )
{
    //guLogMessage( wxT( "Saving lyrics..." ) );
    TagLib::ID3v2::UnsynchronizedLyricsFrame * LyricsFrame;

    TagLib::ID3v2::FrameList FrameList = tagv2->frameListMap()["USLT"];
    for( std::list<TagLib::ID3v2::Frame*>::iterator iter = FrameList.begin(); iter != FrameList.end(); iter++ )
    {
        LyricsFrame = static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>( *iter );
        tagv2->removeFrame( LyricsFrame, TRUE );
    }

    if( !lyrics.IsEmpty() )
    {
        LyricsFrame = new TagLib::ID3v2::UnsynchronizedLyricsFrame( TagLib::String::UTF8 );
        LyricsFrame->setText( wxStringToTString( lyrics ) );
        tagv2->addFrame( LyricsFrame );
    }
}

// -------------------------------------------------------------------------------- //
wxImage * GetXiphCommentCoverArt( Ogg::XiphComment * xiphcomment )
{
    if( xiphcomment && xiphcomment->contains( "COVERART" ) )
    {
        wxString CoverMime = TStringTowxString( xiphcomment->fieldListMap()[ "COVERARTMIME" ].front() );

        wxString CoverEncData = TStringTowxString( xiphcomment->fieldListMap()[ "COVERART" ].front() );

        //guLogMessage( wxT( "Image:\n%s\n" ), CoverEncData.c_str() );

        wxMemoryBuffer CoverDecData = guBase64Decode( CoverEncData );

        //guLogMessage( wxT( "Image Decoded Data : (%i) %i bytes" ), CoverDecData.GetBufSize(), CoverDecData.GetDataLen() );

        //wxFileOutputStream FOut( wxT( "/home/jrios/test.jpg" ) );
        //FOut.Write( CoverDecData.GetData(), CoverDecData.GetDataLen() );
        //FOut.Close();

        wxMemoryInputStream ImgInputStream( CoverDecData.GetData(), CoverDecData.GetDataLen() );

        wxImage * CoverImage = new wxImage( ImgInputStream, CoverMime );
        if( CoverImage )
        {
            if( CoverImage->IsOk() )
            {
                return CoverImage;
            }
            else
            {
                delete CoverImage;
            }
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool SetXiphCommentCoverArt( Ogg::XiphComment * xiphcomment, const wxImage * image )
{
    if( xiphcomment )
    {
        if( xiphcomment->contains( "COVERART" ) )
        {
            xiphcomment->removeField( "COVERARTMIME" );
            xiphcomment->removeField( "COVERART" );
        }
        if( image )
        {
            wxMemoryOutputStream ImgOutputStream;
            if( image->SaveFile( ImgOutputStream, wxBITMAP_TYPE_JPEG ) )
            {
                //ByteVector ImgData( ( TagLib::uint ) ImgOutputStream.GetSize() );
                //ImgOutputStream.CopyTo( ImgData.data(), ImgOutputStream.GetSize() );
                char * ImgData = ( char * ) malloc( ImgOutputStream.GetSize() );
                if( ImgData )
                {
                    ImgOutputStream.CopyTo( ImgData, ImgOutputStream.GetSize() );
                    xiphcomment->addField( "COVERARTMIME", "image/jpeg" );
                    xiphcomment->addField( "COVERART", wxStringToTString( guBase64Encode( ImgData, ImgOutputStream.GetSize() ) ) );
                    free( ImgData );
                    return true;
                }
                else
                {
                    guLogMessage( wxT( "Couldnt allocate memory saving the image to ogg" ) );
                }
            }
            return false;
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
wxString GetXiphCommentLyrics( Ogg::XiphComment * xiphcomment )
{
    if( xiphcomment && xiphcomment->contains( "LYRICS" ) )
    {
        return TStringTowxString( xiphcomment->fieldListMap()[ "LYRICS" ].front() );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool SetXiphCommentLyrics( Ogg::XiphComment * xiphcomment, const wxString &lyrics )
{
    if( xiphcomment )
    {
        while( xiphcomment->contains( "LYRICS" ) )
        {
            xiphcomment->removeField( "LYRICS" );
        }

        if( !lyrics.IsEmpty() )
        {
            xiphcomment->addField( "LYRICS", wxStringToTString( lyrics ) );
        }
        return true;
    }
    return false;
}

//// -------------------------------------------------------------------------------- //
//wxImage * GetMp4Image( TagLib::MP4::Tag * mp4tag )
//{
//    return NULL;
//}
//
//// -------------------------------------------------------------------------------- //
//bool SetMp4Image( TagLib::MP4::Tag * mp4tag, const wxImage * image )
//{
//    return false;
//}

// -------------------------------------------------------------------------------- //
wxString GetMp4Lyrics( TagLib::MP4::Tag * mp4tag )
{
    if( mp4tag )
    {
        if( mp4tag->itemListMap().contains( "\xa9lyr" ) )
            return TStringTowxString( mp4tag->itemListMap()[ "\xa9lyr" ].toStringList().front() );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool SetMp4Lyrics( TagLib::MP4::Tag * mp4tag, const wxString &lyrics )
{
    if( mp4tag )
    {
        if( mp4tag->itemListMap().contains( "\xa9lyr" ) )
        {
            mp4tag->itemListMap().erase( "\xa9lyr" );
        }
        if( !lyrics.IsEmpty() )
        {
            const TagLib::String Lyrics = wxStringToTString( lyrics );
            mp4tag->itemListMap()[ "\xa9lyr" ] = TagLib::StringList( Lyrics );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Read( void )
{
    FileRef fileref( m_FileName.ToUTF8(), true, TagLib::AudioProperties::Fast );
    Tag * tag;
    AudioProperties * apro;

    if( !fileref.isNull() )
    {
        if( ( tag = fileref.tag() ) )
        {
            m_TrackName = TStringTowxString( tag->title() );
            m_ArtistName = TStringTowxString( tag->artist() );
            m_AlbumName = TStringTowxString( tag->album() );
            m_GenreName = TStringTowxString( tag->genre() );
            m_Track = tag->track();
            m_Year = tag->year();
        }
        else
        {
            guLogWarning( wxT( "Cant get tag object from %s" ), m_FileName.c_str() );
            return false;
        }

        apro = fileref.audioProperties();
        if( apro )
        {
            m_Length = apro->length();
            m_Bitrate = apro->bitrate();
            //m_Samplerate = apro->sampleRate();
        }
        else
        {
            guLogWarning( wxT( "Cant read audio properties from %s" ), m_FileName.c_str() );
        }
    }
    else
    {
      guLogError( wxT( "Could not read tags from file %s" ), m_FileName.c_str() );
      return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Write( void )
{
    FileRef fileref( m_FileName.ToUTF8() );
    Tag * tag;

    if( !fileref.isNull() )
    {
        if( ( tag = fileref.tag() ) )
        {
            tag->setTitle( wxStringToTString( m_TrackName ) );
            tag->setArtist( wxStringToTString( m_ArtistName ) );
            tag->setAlbum( wxStringToTString( m_AlbumName ) );
            tag->setGenre( wxStringToTString( m_GenreName ) );
            tag->setTrack( m_Track ); // set the id3v1 track
            tag->setYear( m_Year );
        }
        else
        {
            guLogWarning( wxT( "Cant get tag object from %s" ), m_FileName.c_str() );
            return false;
        }

        if( !fileref.save() )
        {
          guLogWarning( wxT( "Tags Save failed for file %s" ), m_FileName.c_str() );
          return false;
        }
    }
    else
    {
      guLogError( wxT( "Invalid file references writing tags for file %s" ), m_FileName.c_str() );
      return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::CanHandleImages( void )
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxImage * guTagInfo::GetImage( void )
{
	return NULL;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::SetImage( const wxImage * image )
{
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::CanHandleLyrics( void )
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxString guTagInfo::GetLyrics( void )
{
	return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::SetLyrics( const wxString &lyrics )
{
    return false;
}




// -------------------------------------------------------------------------------- //
// guMp3TagInfo
// -------------------------------------------------------------------------------- //
guMp3TagInfo::guMp3TagInfo( const wxString &filename ) : guTagInfo( filename )
{
}

// -------------------------------------------------------------------------------- //
guMp3TagInfo::~guMp3TagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::Read( void )
{
    wxASSERT( !m_FileName.Lower().EndsWith( wxT( ".mp3" ) ) )

    FileRef fileref( m_FileName.ToUTF8(), true, TagLib::AudioProperties::Fast );
    Tag * tag;
    AudioProperties * apro;

    if( !fileref.isNull() )
    {
        if( ( tag = fileref.tag() ) )
        {
            m_TrackName = TStringTowxString( tag->title() );
            m_ArtistName = TStringTowxString( tag->artist() );
            m_AlbumName = TStringTowxString( tag->album() );
            m_GenreName = TStringTowxString( tag->genre() );
            m_Track = tag->track();
            m_Year = tag->year();
        }

        apro = fileref.audioProperties();
        if( apro )
        {
            m_Length = apro->length();
            m_Bitrate = apro->bitrate();
            //m_Samplerate = apro->sampleRate();
        }
        else
        {
            guLogWarning( wxT( "Cant read audio properties from %s\n" ), m_FileName.c_str() );
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
                    // [guTRLABELS] guTRLABELS labels
                    m_TrackLabelsStr = TStringTowxString( Frame->toString() ).Mid( 24 );
                    //guLogMessage( wxT( "*Track Label: '%s'\n" ), m_TrackLabelsStr.c_str() );
                    m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }

            if( m_ArtistLabels.Count() == 0 )
            {
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tagv2, "guARLABELS" );
                if( Frame )
                {
                    m_ArtistLabelsStr = TStringTowxString( Frame->toString() ).Mid( 24 );
                    //guLogMessage( wxT( "*Artist Label: '%s'\n" ), m_ArtistLabelsStr.c_str() );
                    m_ArtistLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }

            if( m_AlbumLabels.Count() == 0 )
            {
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tagv2, "guALLABELS" );
                if( Frame )
                {
                    m_AlbumLabelsStr = TStringTowxString( Frame->toString() ).Mid( 24 );
                    //guLogMessage( wxT( "*Album Label: '%s'\n" ), m_AlbumLabelsStr.c_str() );
                    m_AlbumLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }
        }
    }
    else
    {
      guLogError( wxT( "Could not read tags from file '%s'" ), m_FileName.c_str() );
    }

    return true;
}

// -------------------------------------------------------------------------------- //
void ID3v2_CheckLabelFrame( ID3v2::Tag * tagv2, const char * description, const wxString &value )
{
    ID3v2::UserTextIdentificationFrame * frame;

    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );

    frame = ID3v2::UserTextIdentificationFrame::find( tagv2, description );
    if( frame )
    {
        frame->setText( wxStringToTString( value ) );
    }
    else
    {
        if( !value.IsEmpty() )
        {
            frame = new ID3v2::UserTextIdentificationFrame( TagLib::String::UTF8 );
            frame->setDescription( TagLib::String( description, TagLib::String::UTF8 ) );
            frame->setText( wxStringToTString( value ) );
            tagv2->addFrame( frame );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::Write( void )
{
    FileRef fileref = TagLib::FileRef( m_FileName.ToUTF8() );
    Tag * tag;

    if( !fileref.isNull() )
    {
        if( ( tag = fileref.tag() ) )
        {
            tag->setTitle( wxStringToTString( m_TrackName ) );
            tag->setArtist( wxStringToTString( m_ArtistName ) );
            tag->setAlbum( wxStringToTString( m_AlbumName ) );
            tag->setGenre( wxStringToTString( m_GenreName ) );
            tag->setTrack( m_Track ); // set the id3v1 track
            tag->setYear( m_Year );
        }

        // Check if we have a id3v2 Tag
        ID3v2::Tag * tagv2 = ( ( TagLib::MPEG::File * ) fileref.file() )->ID3v2Tag();
        if( tagv2 )
        {
            // I have found several TRCK fields in the mp3s
            tagv2->removeFrames( "TRCK" );
            tagv2->setTrack( m_Track );

            // The Labels
            ID3v2_CheckLabelFrame( tagv2, "guARLABELS", m_ArtistLabelsStr );
            ID3v2_CheckLabelFrame( tagv2, "guALLABELS", m_AlbumLabelsStr );
            ID3v2_CheckLabelFrame( tagv2, "guTRLABELS", m_TrackLabelsStr );

        }

        if( !fileref.save() )
        {
          guLogWarning( _( "iD3Tags Save failed" ) );
          return false;
        }
    }
    else
    {
      guLogError( wxT( "Could not read tags from file %s\n" ), m_FileName.c_str() );
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMp3TagInfo::GetImage( void )
{
    TagLib::MPEG::File tagfile( m_FileName.ToUTF8() );
    ID3v2::Tag * tagv2 = tagfile.ID3v2Tag( true );

	if( !tagv2 )
        return NULL;

    return GetID3v2Image( tagv2  );
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::SetImage( const wxImage * image )
{
    TagLib::MPEG::File tagfile( m_FileName.ToUTF8() );
    ID3v2::Tag * tagv2 = tagfile.ID3v2Tag( true );

	if( !tagv2 )
        return false;

    SetID3v2Image( tagv2, image );

    return tagfile.save();
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMp3TagInfo::GetLyrics( void )
{
    TagLib::MPEG::File tagfile( m_FileName.ToUTF8() );
    ID3v2::Tag * tagv2 = tagfile.ID3v2Tag( true );

	if( !tagv2 )
        return wxEmptyString;

    return GetID3v2Lyrics( tagv2  );
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::SetLyrics( const wxString &lyrics )
{
    TagLib::MPEG::File tagfile( m_FileName.ToUTF8() );
    ID3v2::Tag * tagv2 = tagfile.ID3v2Tag( true );

	if( !tagv2 )
        return false;

    SetID3v2Lyrics( tagv2, lyrics );

    return tagfile.save();
}




// -------------------------------------------------------------------------------- //
// guFlacTagInfo
// -------------------------------------------------------------------------------- //
guFlacTagInfo::guFlacTagInfo( const wxString &filename ) : guTagInfo( filename )
{
}

// -------------------------------------------------------------------------------- //
guFlacTagInfo::~guFlacTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guFlacTagInfo::GetImage( void )
{
    wxImage * CoverImage = NULL;

    FLAC__Metadata_SimpleIterator * iter = FLAC__metadata_simple_iterator_new();
    if( iter )
    {
        if( FLAC__metadata_simple_iterator_init( iter, m_FileName.ToUTF8(), true, false ) )
        {
            while( FLAC__metadata_simple_iterator_next( iter ) )
            {
                if( FLAC__metadata_simple_iterator_get_block_type( iter ) == FLAC__METADATA_TYPE_PICTURE )
                {
                    FLAC__StreamMetadata * block = FLAC__metadata_simple_iterator_get_block( iter );

                    wxMemoryOutputStream ImgOutStream;

                    FLAC__StreamMetadata_Picture * PicInfo = &block->data.picture;

                    ImgOutStream.Write( PicInfo->data, PicInfo->data_length );
                    wxMemoryInputStream ImgInputStream( ImgOutStream );
                    CoverImage = new wxImage( ImgInputStream, wxString( PicInfo->mime_type, wxConvUTF8 ) );

                    if( CoverImage )
                    {
                        if( !CoverImage->IsOk() )
                        {
                            delete CoverImage;
                            CoverImage = NULL;
                        }
                    }

                    FLAC__metadata_object_delete( block );
                }
            }
        }

        FLAC__metadata_simple_iterator_delete( iter );
    }

    return CoverImage;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::SetImage( const wxImage * image )
{
    FLAC__Metadata_Chain * Chain;
    FLAC__Metadata_Iterator * Iter;

    Chain = FLAC__metadata_chain_new();
    if( Chain )
    {
        if( FLAC__metadata_chain_read( Chain, m_FileName.ToUTF8() ) )
        {
            Iter = FLAC__metadata_iterator_new();
            if( Iter )
            {
                FLAC__metadata_iterator_init( Iter, Chain );

                while( FLAC__metadata_iterator_next( Iter ) )
                {
                    if( FLAC__metadata_iterator_get_block_type( Iter ) == FLAC__METADATA_TYPE_PICTURE )
                    {
                        FLAC__StreamMetadata * Picture = FLAC__metadata_iterator_get_block( Iter );
                        if( Picture->data.picture.type ==  FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER )
                        {
                            //
                            FLAC__metadata_iterator_delete_block( Iter, true );
                        }
                    }
                }

                wxMemoryOutputStream ImgOutputStream;
                if( image && image->SaveFile( ImgOutputStream, wxBITMAP_TYPE_JPEG ) )
                {
                    FLAC__byte * CoverData = ( FLAC__byte * ) malloc( ImgOutputStream.GetSize() );
                    if( CoverData )
                    {
                        const char * PicErrStr;

                        ImgOutputStream.CopyTo( CoverData, ImgOutputStream.GetSize() );

                        //
                        FLAC__StreamMetadata * Picture;
                        Picture = FLAC__metadata_object_new( FLAC__METADATA_TYPE_PICTURE );
                        Picture->data.picture.type = FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER;
                        FLAC__metadata_object_picture_set_mime_type( Picture,  ( char * ) "image/jpeg", TRUE );

                        //FLAC__metadata_object_picture_set_description( Picture, ( char * ) "", TRUE );
                        Picture->data.picture.width  = image->GetWidth();
                        Picture->data.picture.height = image->GetHeight();
                        Picture->data.picture.depth  = 0;

                        FLAC__metadata_object_picture_set_data( Picture, CoverData, ( FLAC__uint32 ) ImgOutputStream.GetSize(), FALSE );

                        if( FLAC__metadata_object_picture_is_legal( Picture, &PicErrStr ) )
                        {
                            FLAC__metadata_iterator_insert_block_after( Iter, Picture );
                            FLAC__metadata_chain_sort_padding( Chain );

                            if( !FLAC__metadata_chain_write( Chain, TRUE, TRUE ) )
                            {
                                guLogError( wxT( "Could not save the FLAC file" ) );
                            }

                            FLAC__metadata_chain_delete( Chain );
                        }
                        else
                        {
                            guLogError( wxT( "The FLAC picture is invalid: %s" ), PicErrStr );
                            FLAC__metadata_object_delete( Picture );
                        }
                    }
                }
            }
            else
            {
                guLogError( wxT( "Could not create the FLAC Iterator." ) );
            }
        }
        else
        {
            guLogError( wxT( "Could not read the FLAC metadata." ) );
        }
    }
    else
    {
        guLogError( wxT( "Could not create a FLAC chain." ) );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guFlacTagInfo::GetLyrics( void )
{
    TagLib::FLAC::File tagfile( m_FileName.ToUTF8() );

    return GetXiphCommentLyrics( tagfile.xiphComment() );
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::SetLyrics( const wxString &lyrics )
{
    TagLib::FLAC::File tagfile( m_FileName.ToUTF8() );

    return SetXiphCommentLyrics( tagfile.xiphComment(), lyrics ) && tagfile.save();
}




// -------------------------------------------------------------------------------- //
// guOggTagInfo
// -------------------------------------------------------------------------------- //
guOggTagInfo::guOggTagInfo( const wxString &filename ) : guTagInfo( filename )
{
}

// -------------------------------------------------------------------------------- //
guOggTagInfo::~guOggTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guOggTagInfo::GetImage( void )
{
    TagLib::Ogg::Vorbis::File tagfile( m_FileName.ToUTF8() );

    return GetXiphCommentCoverArt( tagfile.tag() );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::SetImage( const wxImage * image )
{
    TagLib::Ogg::Vorbis::File tagfile( m_FileName.ToUTF8() );

    return SetXiphCommentCoverArt( tagfile.tag(), image ) && tagfile.save();
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guOggTagInfo::GetLyrics( void )
{
    TagLib::Ogg::Vorbis::File tagfile( m_FileName.ToUTF8() );
    return GetXiphCommentLyrics( tagfile.tag() );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::SetLyrics( const wxString &lyrics )
{
    TagLib::Ogg::Vorbis::File tagfile( m_FileName.ToUTF8() );

    return SetXiphCommentLyrics( tagfile.tag(), lyrics ) && tagfile.save();
}




// -------------------------------------------------------------------------------- //
// guMp4TagInfo
// -------------------------------------------------------------------------------- //
guMp4TagInfo::guMp4TagInfo( const wxString &filename ) : guTagInfo( filename )
{
}

// -------------------------------------------------------------------------------- //
guMp4TagInfo::~guMp4TagInfo()
{
}

//// -------------------------------------------------------------------------------- //
//bool guMp4TagInfo::CanHandleImages( void )
//{
//    return false;
//}

//// -------------------------------------------------------------------------------- //
//wxImage * guMp4TagInfo::GetImage( void )
//{
//    MP4FileHandle mp4_file = MP4Read( m_FileName.ToUTF8() );
//    if( mp4_file != MP4_INVALID_FILE_HANDLE )
//    {
//        if( MP4GetMetadataCoverArtCount( mp4_file ) )
//        {
//            uint8_t *   CoverData;
//            uint32_t    CoverSize;
//            if( MP4GetMetadataCoverArt( mp4_file, &CoverData, &CoverSize ) )
//            {
//                wxMemoryOutputStream ImgOutStream;
//                ImgOutStream.Write( CoverData, CoverSize );
//                wxMemoryInputStream ImgInputStream( ImgOutStream );
//                // TODO : Determine image type from data stream
//                wxImage * CoverImage = new wxImage( ImgInputStream, wxBITMAP_TYPE_JPEG );
//                if( !CoverImage || !CoverImage->IsOk() )
//                {
//                    if( CoverImage )
//                        delete CoverImage;
//                    CoverImage = new wxImage( ImgInputStream, wxBITMAP_TYPE_PNG );
//                }
//
//                if( CoverImage )
//                {
//                    if( CoverImage->IsOk() )
//                    {
//                        return CoverImage;
//                    }
//                    else
//                    {
//                        delete CoverImage;
//                    }
//                }
//            }
//        }
//        MP4Close( mp4_file );
//    }
//    else
//    {
//        guLogError( wxT( "GetImage : could not open the file %s" ), m_FileName.c_str() );
//    }
//    return NULL;
//}
//
//// -------------------------------------------------------------------------------- //
//bool guMp4TagInfo::SetImage( const wxImage * image )
//{
//    bool RetVal = false;
//    MP4FileHandle mp4_file = MP4Modify( m_FileName.ToUTF8() );
//    if( mp4_file != MP4_INVALID_FILE_HANDLE )
//    {
//        if( image )
//        {
//            wxMemoryOutputStream ImgOutputStream;
//            if( image->SaveFile( ImgOutputStream, wxBITMAP_TYPE_JPEG ) )
//            {
//                uint8_t * CoverData = ( uint8_t * ) malloc( ImgOutputStream.GetSize() );
//                if( CoverData )
//                {
//                    ImgOutputStream.CopyTo( CoverData, ImgOutputStream.GetSize() );
//                    RetVal = MP4SetMetadataCoverArt( mp4_file, CoverData, ImgOutputStream.GetSize() );
//                    free( CoverData );
//                }
//                else
//                {
//                    guLogError( wxT( "could not allocate memory for image processing %s" ), m_FileName.c_str() );
//                }
//            }
//        }
//        else
//        {
//            RetVal = MP4DeleteMetadataCoverArt( mp4_file );
//        }
//        MP4Close( mp4_file );
//    }
//    else
//    {
//        guLogError( wxT( "SetImage : could not open the file %s" ), m_FileName.c_str() );
//    }
//    return RetVal;
//}

//// -------------------------------------------------------------------------------- //
//bool guMp4TagInfo::CanHandleImages( void )
//{
//    return true;
//}
//
//// -------------------------------------------------------------------------------- //
//wxImage * guMp4TagInfo::GetImage( void )
//{
//    TagLib::MP4::File tagfile( m_FileName.ToUTF8() );
//    return GetMp4Image( tagfile.tag() );
//}
//
//// -------------------------------------------------------------------------------- //
//bool guMp4TagInfo::SetImage( const wxImage * image )
//{
//    TagLib::MP4::File tagfile( m_FileName.ToUTF8() );
//    return SetMp4Image( tagfile.tag(), image ) && tagfile.save();
//}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMp4TagInfo::GetLyrics( void )
{
    TagLib::MP4::File tagfile( m_FileName.ToUTF8() );
    return GetMp4Lyrics( tagfile.tag() );
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::SetLyrics( const wxString &lyrics )
{
    TagLib::MP4::File tagfile( m_FileName.ToUTF8() );

    return SetMp4Lyrics( tagfile.tag(), lyrics ) && tagfile.save();
}




// -------------------------------------------------------------------------------- //
// guMpcTagInfo
// -------------------------------------------------------------------------------- //
guMpcTagInfo::guMpcTagInfo( const wxString &filename ) : guTagInfo( filename )
{
}

// -------------------------------------------------------------------------------- //
guMpcTagInfo::~guMpcTagInfo()
{
}

// -------------------------------------------------------------------------------- //
// guApeTagInfo
// -------------------------------------------------------------------------------- //
guApeTagInfo::guApeTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    wxASSERT( !m_FileName.Lower().EndsWith( wxT( ".ape" ) ) )
}

// -------------------------------------------------------------------------------- //
guApeTagInfo::~guApeTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Read( void )
{
    guApeFile File( m_FileName );
    guApeTag * Tag = File.GetApeTag();
    if( Tag )
    {
        m_TrackName = Tag->GetTitle();
        m_ArtistName = Tag->GetArtist();
        m_AlbumName = Tag->GetAlbum();
        m_GenreName = Tag->GetGenre();
        m_Track = Tag->GetTrack();
        m_Year = Tag->GetYear();
        m_Length = File.GetTrackLength();
        m_Bitrate = File.GetBitRate();
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Write( void )
{
    guApeFile File( m_FileName );
    guApeTag * Tag = File.GetApeTag();
    if( Tag )
    {
        Tag->SetTitle( m_TrackName );
        Tag->SetArtist( m_ArtistName );
        Tag->SetAlbum( m_AlbumName );
        Tag->SetGenre( m_GenreName );
        Tag->SetTrack( m_Track );
        Tag->SetYear( m_Year );
        File.WriteApeTag();
        return true;
    }
    return false;
}




// -------------------------------------------------------------------------------- //
// Other functions
// -------------------------------------------------------------------------------- //
wxImage * guTagGetPicture( const wxString &filename )
{
    wxImage * RetVal = NULL;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleImages() )
        {
            RetVal = TagInfo->GetImage();
        }
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guTagSetPicture( const wxString &filename, wxImage * picture )
{
    bool RetVal = false;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleImages() )
        {
            RetVal = TagInfo->SetImage( picture );
        }
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guTagGetLyrics( const wxString &filename )
{
    wxString RetVal = wxEmptyString;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleLyrics() )
        {
            RetVal = TagInfo->GetLyrics();
        }
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guTagSetLyrics( const wxString &filename, wxString &lyrics )
{
    bool RetVal = false;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleLyrics() )
        {
            RetVal = TagInfo->SetLyrics( lyrics );
        }
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void UpdateImages( const guTrackArray &songs, const guImagePtrArray &images )
{
    int Index;
    int Count = images.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTagSetPicture( songs[ Index ].m_FileName, images[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
void UpdateLyrics( const guTrackArray &songs, const wxArrayString &lyrics )
{
    int Index;
    int Count = lyrics.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTagSetLyrics( songs[ Index ].m_FileName, lyrics[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
