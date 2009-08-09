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

#include "TrackEdit.h"
#include "Base64.h"

#include <tag.h>
#include <attachedpictureframe.h>
#include <fileref.h>
#include <id3v2framefactory.h>
#include <textidentificationframe.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <flacfile.h>
#include <vorbisfile.h>

#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>

using namespace TagLib;

// -------------------------------------------------------------------------------- //
guTagInfo * guGetTagInfoHandler( const wxString &filename )
{
    if( filename.Lower().EndsWith( wxT( ".mp3" ) ) )
    {
        return new guMp3TagInfo();
    }
    else if( filename.Lower().EndsWith( wxT( ".flac" ) ) )
    {
        return new guFlacTagInfo();
    }
    else if( filename.Lower().EndsWith( wxT( ".ogg" ) ) ||
             filename.Lower().EndsWith( wxT( ".oga" ) ) )
    {
        return new guOggTagInfo();
    }
    else if( filename.Lower().EndsWith( wxT( ".mpc" ) ) )
    {
        return new guMpcTagInfo();
    }
//    else if( filename.Lower().EndsWith( wxT( ".ape" ) ) )
//    {
//        return new guApeTagInfo();
//    }
    else
    {
        return NULL;
    }
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Read( const wxString &filename )
{
    FileRef fileref = TagLib::FileRef( filename.ToUTF8(), true, TagLib::AudioProperties::Fast );
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
            guLogWarning( wxT( "Cant get tag object from %s" ), filename.c_str() );
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
            guLogWarning( wxT( "Cant read audio properties from %s" ), filename.c_str() );
        }
    }
    else
    {
      guLogError( wxT( "Could not read tags from file %s" ), filename.c_str() );
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Write( const wxString &filename )
{
    FileRef fileref = TagLib::FileRef( filename.ToUTF8() );
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

        if( !fileref.save() )
        {
          guLogWarning( wxT( "Tags Save failed for file %s" ), filename.c_str() );
          return false;
        }
    }
    else
    {
      guLogError( wxT( "Invalid file references writing tags for file %s" ), filename.c_str() );
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::CanHandleImages( void )
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxImage * guTagInfo::GetImage( const wxString &filename )
{
	return NULL;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::SetImage( const wxString &filename, const wxImage * image )
{
    return false;
}




// -------------------------------------------------------------------------------- //
// guMp3TagInfo
// -------------------------------------------------------------------------------- //
guMp3TagInfo::guMp3TagInfo() : guTagInfo()
{
}

// -------------------------------------------------------------------------------- //
guMp3TagInfo::~guMp3TagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::Read( const wxString &filename )
{
    wxASSERT( !filename.Lower().EndsWith( wxT( ".mp3" ) ) )

    FileRef fileref = TagLib::FileRef( filename.ToUTF8(), true, TagLib::AudioProperties::Fast );
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
            guLogWarning( wxT( "Cant read audio properties from %s\n" ), filename.c_str() );
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
      guLogError( wxT( "Could not read tags from file '%s'" ), filename.c_str() );
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
bool guMp3TagInfo::Write( const wxString &filename )
{
    FileRef fileref = TagLib::FileRef( filename.ToUTF8() );
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
      guLogError( wxT( "Could not read tags from file %s\n" ), filename.c_str() );
    }
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMp3TagInfo::GetImage( const wxString &filename )
{
    TagLib::MPEG::File tagfile( filename.ToUTF8() );
    ID3v2::Tag * tagv2 = tagfile.ID3v2Tag( false );
    if( !tagv2 )
        return NULL;

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
bool guMp3TagInfo::SetImage( const wxString &filename, const wxImage * image )
{
    TagLib::MPEG::File tagfile( filename.ToUTF8() );
    ID3v2::Tag * tagv2 = tagfile.ID3v2Tag( true );

	if( !tagv2 )
        return false;

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
    return tagfile.save();
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::CanHandleImages( void )
{
    return true;
}



// -------------------------------------------------------------------------------- //
// guFlacTagInfo
// -------------------------------------------------------------------------------- //
guFlacTagInfo::guFlacTagInfo() : guTagInfo()
{
}

// -------------------------------------------------------------------------------- //
guFlacTagInfo::~guFlacTagInfo()
{
}




// -------------------------------------------------------------------------------- //
// guOggTagInfo
// -------------------------------------------------------------------------------- //
guOggTagInfo::guOggTagInfo() : guTagInfo()
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
wxImage * GetXiphCommentCoverArt( Ogg::XiphComment * xiphcomment )
{
    if( xiphcomment->contains( "COVERART" ) )
    {
        wxString CoverMime = TStringTowxString( xiphcomment->fieldListMap()[ "COVERARTMIME" ].front() );

        wxString CoverEncData =  wxString( xiphcomment->fieldListMap()[ "COVERART" ].front().data( TagLib::String::Latin1 ).data(), wxConvISO8859_1 );
        wxMemoryBuffer CoverDecData = guBase64Decode( CoverEncData );

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
wxImage * guOggTagInfo::GetImage( const wxString &filename )
{
    TagLib::Ogg::Vorbis::File tagfile( filename.ToUTF8() );

    wxImage * RetVal = GetXiphCommentCoverArt( tagfile.tag() );
    if( !RetVal )
    {
        guLogMessage( wxT( "No cover found in ogg file" ) );
    }
    return RetVal;


#if 0
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
#endif
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::SetImage( const wxString &filename, const wxImage * image )
{
    TagLib::MPEG::File tagfile( filename.ToUTF8() );
    ID3v2::Tag * tagv2 = tagfile.ID3v2Tag( true );

	if( !tagv2 )
        return false;

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
    return tagfile.save();
}




// -------------------------------------------------------------------------------- //
// guMpcTagInfo
// -------------------------------------------------------------------------------- //
guMpcTagInfo::guMpcTagInfo() : guTagInfo()
{
}

// -------------------------------------------------------------------------------- //
guMpcTagInfo::~guMpcTagInfo()
{
}




//// -------------------------------------------------------------------------------- //
//// guApeTagInfo
//// -------------------------------------------------------------------------------- //
//guApeTagInfo::guApeTagInfo() : guTagInfo()
//{
//}
//
//// -------------------------------------------------------------------------------- //
//guApeTagInfo::~guApeTagInfo()
//{
//}




// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
wxImage * ID3TagGetPicture( const wxString &filename )
{
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        wxImage * RetVal = TagInfo->GetImage( filename );
        delete TagInfo;
        return RetVal;
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool ID3TagSetPicture( const wxString &filename, wxImage * picture )
{
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        bool RetVal = TagInfo->CanHandleImages() && TagInfo->SetImage( filename, picture );
        delete TagInfo;
        return RetVal;
    }
    return false;
}


// -------------------------------------------------------------------------------- //
void UpdateImages( const guTrackArray &Songs, const guImagePtrArray &Images )
{
    int index;
    int count = Images.Count();
    for( index = 0; index < count; index++ )
    {
        ID3TagSetPicture( Songs[ index ].m_FileName, Images[ index ] );
    }
}

// -------------------------------------------------------------------------------- //
