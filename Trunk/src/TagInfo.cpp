// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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

#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>

#include <asfattribute.h>

wxArrayString guSupportedFormats;


// -------------------------------------------------------------------------------- //
bool guIsValidAudioFile( const wxString &filename )
{
    if( !guSupportedFormats.Count() )
    {
        guSupportedFormats.Add( wxT( "mp3"  ) );
        //
        guSupportedFormats.Add( wxT( "flac" ) );
        //
        guSupportedFormats.Add( wxT( "ogg"  ) );
        guSupportedFormats.Add( wxT( "oga"  ) );
        //
        guSupportedFormats.Add( wxT( "mp4"  ) );  // MP4 files
        guSupportedFormats.Add( wxT( "m4a"  ) );
        guSupportedFormats.Add( wxT( "m4b"  ) );
        guSupportedFormats.Add( wxT( "m4p"  ) );
        guSupportedFormats.Add( wxT( "aac"  ) );
        //
        guSupportedFormats.Add( wxT( "wma"  ) );
        guSupportedFormats.Add( wxT( "asf"  ) );
        //
        guSupportedFormats.Add( wxT( "ape"  ) );
        //
        guSupportedFormats.Add( wxT( "wav"  ) );
        guSupportedFormats.Add( wxT( "aif"  ) );
        //
        guSupportedFormats.Add( wxT( "wv"   ) );
        //
        guSupportedFormats.Add( wxT( "tta"  ) );
        //
        guSupportedFormats.Add( wxT( "mpc"  ) );
        //
        //guSupportedFormats.Add( wxT( "rmj"  ) );
    }

    return ( guSupportedFormats.Index( filename.Lower().AfterLast( wxT( '.' ) ) ) != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
guTagInfo * guGetTagInfoHandler( const wxString &filename )
{
    int FormatIndex = guSupportedFormats.Index( filename.Lower().AfterLast( wxT( '.' ) ) );
    switch( FormatIndex )
    {
        case  0 : return new guMp3TagInfo( filename );

        case  1 : return new guFlacTagInfo( filename );

        case  2 :
        case  3 : return new guOggTagInfo( filename );

        case  4 :
        case  5 :
        case  6 :
        case  7 :
        case  8 : return new guMp4TagInfo( filename );

        case  9 :
        case 10 : return new guASFTagInfo( filename );

        case 11 : return new guApeTagInfo( filename );

        case 12 :
        case 13 : return new guTagInfo( filename );

        case 14 : return new guWavPackTagInfo( filename );

        case 15 : return new guTrueAudioTagInfo( filename );

        case 16 : return new guMpcTagInfo( filename );

        //case 17 :

        default :
            break;
    }

    return NULL;
}


// -------------------------------------------------------------------------------- //
wxImage * GetID3v2ImageType( TagLib::ID3v2::FrameList &framelist,
            TagLib::ID3v2::AttachedPictureFrame::Type frametype  = TagLib::ID3v2::AttachedPictureFrame::FrontCover );

wxImage * GetID3v2ImageType( TagLib::ID3v2::FrameList &framelist,
            TagLib::ID3v2::AttachedPictureFrame::Type frametype )
{
    TagLib::ID3v2::AttachedPictureFrame * PicFrame;
    for( std::list<TagLib::ID3v2::Frame*>::iterator iter = framelist.begin(); iter != framelist.end(); iter++ )
    {
        PicFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>( *iter );

        if( PicFrame->type() == frametype )
        {
            int ImgDataSize = PicFrame->picture().size();

            if( ImgDataSize > 0 )
            {
                //guLogMessage( wxT( "ID3v2 header contains APIC frame with %u bytes." ), ImgDataSize );
                wxMemoryOutputStream ImgOutStream;
                ImgOutStream.Write( PicFrame->picture().data(), ImgDataSize );
                wxMemoryInputStream ImgInputStream( ImgOutStream );
                wxString ImgHandler = wxString( PicFrame->mimeType().toCString( true ), wxConvUTF8 );
                ImgHandler.Replace( wxT( "/jpg" ), wxT( "/jpeg" ) );
                wxImage * CoverImage = new wxImage( ImgInputStream, ImgHandler );
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
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
wxImage * GetID3v2Image( ID3v2::Tag * tagv2 )
{
    TagLib::ID3v2::FrameList FrameList = tagv2->frameListMap()["APIC"];

    wxImage * CoverImage = GetID3v2ImageType( FrameList, TagLib::ID3v2::AttachedPictureFrame::FrontCover );

    if( !CoverImage )
    {
        CoverImage = GetID3v2ImageType( FrameList, TagLib::ID3v2::AttachedPictureFrame::Other );
    }

	return CoverImage;
}

// -------------------------------------------------------------------------------- //
void SetID3v2Image( ID3v2::Tag * tagv2, const wxImage * image )
{
    TagLib::ID3v2::AttachedPictureFrame * PicFrame;

    TagLib::ID3v2::FrameList FrameList = tagv2->frameListMap()["APIC"];
    for( std::list<TagLib::ID3v2::Frame*>::iterator iter = FrameList.begin(); iter != FrameList.end(); iter++ )
    {
        PicFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>( *iter );
        // TODO : Ppl should be able to select which image types want guayadeque to remove from the audio files
        if( ( PicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover ) ||
            ( PicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::Other ) )
            tagv2->removeFrame( PicFrame, TRUE );
    }

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

#ifdef TAGLIB_WITH_MP4_COVERS
// -------------------------------------------------------------------------------- //
wxImage * GetMp4Image( TagLib::MP4::Tag * mp4tag )
{
    if( mp4tag && mp4tag->itemListMap().contains( "covr" ) )
    {
        TagLib::MP4::CoverArtList Covers = mp4tag->itemListMap()[ "covr" ].toCoverArtList();

        for( TagLib::MP4::CoverArtList::Iterator it = Covers.begin(); it != Covers.end(); it++ )
        {
            int ImgType = wxBITMAP_TYPE_INVALID;
            if( it->format() == TagLib::MP4::CoverArt::PNG )
            {
                ImgType = wxBITMAP_TYPE_PNG;
            }
            else if( it->format() == TagLib::MP4::CoverArt::JPEG )
            {
                ImgType = wxBITMAP_TYPE_JPEG;
            }

            wxMemoryOutputStream ImgOutStream;
            ImgOutStream.Write( it->data().data(), it->data().size() );
            wxMemoryInputStream ImgInputStream( ImgOutStream );
            wxImage * CoverImage = new wxImage( ImgInputStream, ImgType );
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
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool SetMp4Image( TagLib::MP4::Tag * mp4tag, const wxImage * image )
{
    if( mp4tag )
    {
        if( mp4tag->itemListMap().contains( "covr" ) )
        {
            mp4tag->itemListMap().erase( "covr" );
        }

        if( image )
        {
            wxMemoryOutputStream ImgOutputStream;
            if( image && image->SaveFile( ImgOutputStream, wxBITMAP_TYPE_JPEG ) )
            {
                ByteVector ImgData( ( TagLib::uint ) ImgOutputStream.GetSize() );
                ImgOutputStream.CopyTo( ImgData.data(), ImgOutputStream.GetSize() );

                TagLib::MP4::CoverArtList CoverList;
                TagLib::MP4::CoverArt Cover( TagLib::MP4::CoverArt::JPEG, ImgData );
                CoverList.append( Cover );
                mp4tag->itemListMap()[ "covr" ] = CoverList;

                return true;
            }
            return false;
        }
        return true;
    }
    return false;
}
#endif

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
wxImage * GetApeItemImage( const TagLib::APE::Item &item )
{
    if( item.type() == TagLib::APE::Item::Binary )
    {
        TagLib::ByteVector CoverData = item.value();

        if( CoverData.size() )
        {
            wxMemoryOutputStream ImgOutStream;
            ImgOutStream.Write( CoverData.data(), CoverData.size() );
            wxMemoryInputStream ImgInputStream( ImgOutStream );
            wxImage * CoverImage = new wxImage( ImgInputStream, wxBITMAP_TYPE_JPEG );
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
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
wxImage * GetApeImage( TagLib::APE::Tag * apetag )
{
    if( apetag )
    {
        if( apetag->itemListMap().contains( "Cover Art (front)" ) )
        {
            return GetApeItemImage( apetag->itemListMap()[ "Cover Art (front)" ] );
        }
        else if( apetag->itemListMap().contains( "Cover Art (other)" ) )
        {
            return GetApeItemImage( apetag->itemListMap()[ "Cover Art (other)" ] );
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool SetApeImage( TagLib::APE::Tag * apetag, const wxImage * image )
{
    return false;
}


// -------------------------------------------------------------------------------- //
wxString GetApeLyrics( APE::Tag * apetag )
{
    if( apetag )
    {
        if( apetag->itemListMap().contains( "LYRICS" ) )
        {
            return TStringTowxString( apetag->itemListMap()[ "LYRICS" ].toStringList().front() );
        }
        else if( apetag->itemListMap().contains( "UNSYNCED LYRICS" ) )
        {
            return TStringTowxString( apetag->itemListMap()[ "UNSYNCED LYRICS" ].toStringList().front() );
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool SetApeLyrics( APE::Tag * apetag, const wxString &lyrics )
{
    if( apetag )
    {
        if( apetag->itemListMap().contains( "LYRICS" ) )
        {
            apetag->removeItem( "LYRICS" );
        }
        if( apetag->itemListMap().contains( "UNSYNCED LYRICS" ) )
        {
            apetag->removeItem( "UNSYNCED LYRICS" );
        }
        if( !lyrics.IsEmpty() )
        {
            const TagLib::String Lyrics = wxStringToTString( lyrics );
            apetag->addValue( "Lyrics", Lyrics );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
wxImage * GetASFImage( ASF::Tag * asftag )
{
    if( asftag )
    {
        if( asftag->attributeListMap().contains( "WM/Picture" ) )
        {
            ByteVector PictureData = asftag->attributeListMap()[ "WM/Picture" ].front().toByteVector();

            TagLib::ID3v2::AttachedPictureFrame * PicFrame;

            PicFrame = ( TagLib::ID3v2::AttachedPictureFrame * ) PictureData.data();

            int ImgDataSize = PicFrame->picture().size();

            if( ImgDataSize > 0 )
            {
                //guLogMessage( wxT( "ASF header contains APIC frame with %u bytes." ), ImgDataSize );
                wxMemoryOutputStream ImgOutStream;
                ImgOutStream.Write( PicFrame->picture().data(), ImgDataSize );
                wxMemoryInputStream ImgInputStream( ImgOutStream );
                wxString ImgHandler = wxString( PicFrame->mimeType().toCString( true ), wxConvUTF8 );
                ImgHandler.Replace( wxT( "/jpg" ), wxT( "/jpeg" ) );
                wxImage * CoverImage = new wxImage( ImgInputStream, ImgHandler );
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
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool SetASFImage( ASF::Tag * asftag, const wxImage * image )
{
    return NULL;
}

// -------------------------------------------------------------------------------- //
// guTagInfo
// -------------------------------------------------------------------------------- //
guTagInfo::guTagInfo( const wxString &filename )
{
    SetFileName( filename );

    m_Track = 0;
    m_Year = 0;
    m_Length = 0;
    m_Bitrate = 0;
    m_Compilation = false;
};

// -------------------------------------------------------------------------------- //
guTagInfo::~guTagInfo()
{
    if( m_TagFile )
        delete m_TagFile;
}

// -------------------------------------------------------------------------------- //
void guTagInfo::SetFileName( const wxString &filename )
{
    m_FileName = filename;
    if( !filename.IsEmpty() )
    {
        m_TagFile = new TagLib::FileRef( filename.mb_str( wxConvFile ), true, TagLib::AudioProperties::Fast );
        //m_TagFile = new TagLib::FileRef( filename.ToUTF8(), true, TagLib::AudioProperties::Fast );
    }
    else
    {
        m_TagFile = NULL;
    }

    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_Tag = m_TagFile->tag();
        if( !m_Tag )
        {
            guLogWarning( wxT( "Cant get tag object from '%s'" ), filename.c_str() );
        }
    }
    else
    {
        m_Tag = NULL;
    }
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Read( void )
{
    AudioProperties * apro;
    if( m_Tag )
    {
        m_TrackName = TStringTowxString( m_Tag->title() );
        if( m_TrackName.IsEmpty() )
            m_TrackName = m_FileName.AfterLast( wxT( '/' ) );
        m_ArtistName = TStringTowxString( m_Tag->artist() );
        if( m_ArtistName.IsEmpty() )
            m_ArtistName = _( "Unknown" );
        m_AlbumName = TStringTowxString( m_Tag->album() );
        if( m_AlbumName.IsEmpty() )
            m_AlbumName = m_FileName.BeforeLast( wxT( '/' ) ).AfterLast( wxT( '/' ) );
        m_GenreName = TStringTowxString( m_Tag->genre() );
        if( m_GenreName.IsEmpty() )
            m_GenreName = _( "Unknown" );
        m_Comments = TStringTowxString( m_Tag->comment() );
        m_Track = m_Tag->track();
        m_Year = m_Tag->year();
    }

    if( m_TagFile && m_Tag && ( apro = m_TagFile->audioProperties() ) )
    {
        m_Length = apro->length();
        m_Bitrate = apro->bitrate();
        //m_Samplerate = apro->sampleRate();
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Write( void )
{
    if( m_Tag )
    {
        m_Tag->setTitle( wxStringToTString( m_TrackName ) );
        m_Tag->setArtist( wxStringToTString( m_ArtistName ) );
        m_Tag->setAlbum( wxStringToTString( m_AlbumName ) );
        m_Tag->setGenre( wxStringToTString( m_GenreName ) );
        m_Tag->setComment( wxStringToTString( m_Comments ) );
        m_Tag->setTrack( m_Track ); // set the id3v1 track
        m_Tag->setYear( m_Year );
    }

    if( !m_TagFile->save() )
    {
      guLogWarning( wxT( "Tags Save failed for file '%s'" ), m_FileName.c_str() );
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
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_TagId3v2 = ( ( TagLib::MPEG::File * ) m_TagFile->file() )->ID3v2Tag();
    }
    else
    {
        m_TagId3v2 = NULL;
    }
}

// -------------------------------------------------------------------------------- //
guMp3TagInfo::~guMp3TagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        // If its a ID3v2 Tag try to load the labels
        if( m_TagId3v2 )
        {
            if( m_TagId3v2->frameListMap().contains( "TPOS" ) )
            {
                m_Disk = TStringTowxString( m_TagId3v2->frameListMap()[ "TPOS" ].front()->toString() );
            }

            if( m_TagId3v2->frameListMap().contains( "TCOM" ) )
            {
                m_Composer = TStringTowxString( m_TagId3v2->frameListMap()[ "TCOM" ].front()->toString() );
            }

            if( m_TagId3v2->frameListMap().contains( "TPE2" ) )
            {
                m_AlbumArtist = TStringTowxString( m_TagId3v2->frameListMap()[ "TPE2" ].front()->toString() );
            }

            if( m_TagId3v2->frameListMap().contains( "TCMP" ) )
            {
                m_Compilation = TStringTowxString( m_TagId3v2->frameListMap()[ "TCMP" ].front()->toString() ) == wxT( "1" );
            }

            if( m_TrackLabels.Count() == 0 )
            {
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( m_TagId3v2, "guTRLABELS" );
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
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( m_TagId3v2, "guARLABELS" );
                if( Frame )
                {
                    m_ArtistLabelsStr = TStringTowxString( Frame->toString() ).Mid( 24 );
                    //guLogMessage( wxT( "*Artist Label: '%s'\n" ), m_ArtistLabelsStr.c_str() );
                    m_ArtistLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }

            if( m_AlbumLabels.Count() == 0 )
            {
                ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( m_TagId3v2, "guALLABELS" );
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
    if( m_TagId3v2 )
    {
        TagLib::ID3v2::TextIdentificationFrame * frame;
        m_TagId3v2->removeFrames( "TPOS" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TPOS" );
        frame->setText( wxStringToTString( m_Disk ) );
        m_TagId3v2->addFrame( frame );

        m_TagId3v2->removeFrames( "TCOM" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TCOM" );
        frame->setText( wxStringToTString( m_Composer ) );
        m_TagId3v2->addFrame( frame );

        m_TagId3v2->removeFrames( "TPE2" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TPE2" );
        frame->setText( wxStringToTString( m_AlbumArtist ) );
        m_TagId3v2->addFrame( frame );

        m_TagId3v2->removeFrames( "TCMP" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TCMP" );
        frame->setText( wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
        m_TagId3v2->addFrame( frame );

        // I have found several TRCK fields in the mp3s
        m_TagId3v2->removeFrames( "TRCK" );
        m_TagId3v2->setTrack( m_Track );

        // The Labels
        ID3v2_CheckLabelFrame( m_TagId3v2, "guARLABELS", m_ArtistLabelsStr );
        ID3v2_CheckLabelFrame( m_TagId3v2, "guALLABELS", m_AlbumLabelsStr );
        ID3v2_CheckLabelFrame( m_TagId3v2, "guTRLABELS", m_TrackLabelsStr );
    }

    return guTagInfo::Write();
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMp3TagInfo::GetImage( void )
{
    if( m_TagId3v2 )
    {
        return GetID3v2Image( m_TagId3v2 );
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::SetImage( const wxImage * image )
{
    if( m_TagId3v2 )
    {
        SetID3v2Image( m_TagId3v2, image );
    }
    else
        return false;

    return m_TagFile->save();
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMp3TagInfo::GetLyrics( void )
{
    if( m_TagId3v2 )
    {
        return GetID3v2Lyrics( m_TagId3v2  );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::SetLyrics( const wxString &lyrics )
{
	if( m_TagId3v2 )
    {
        SetID3v2Lyrics( m_TagId3v2, lyrics );
        return m_TagFile->save();
    }
    return false;
}




// -------------------------------------------------------------------------------- //
// guFlacTagInfo
// -------------------------------------------------------------------------------- //
guFlacTagInfo::guFlacTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_XiphComment = ( ( TagLib::FLAC::File * ) m_TagFile->file() )->xiphComment();
    }
    else
        m_XiphComment = NULL;
}

// -------------------------------------------------------------------------------- //
guFlacTagInfo::~guFlacTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        if( m_XiphComment )
        {
            if( m_XiphComment->fieldListMap().contains( "COMPOSER" ) )
            {
                m_Composer = TStringTowxString( m_XiphComment->fieldListMap()["COMPOSER"].front() );
            }

            if( m_XiphComment->fieldListMap().contains( "DISCNUMBER" ) )
            {
                m_Disk = TStringTowxString( m_XiphComment->fieldListMap()["DISCNUMBER"].front() );
            }

            if( m_XiphComment->fieldListMap().contains( "COMPILATION" ) )
            {
                m_Compilation = TStringTowxString( m_XiphComment->fieldListMap()["COMPILATION"].front() ) == wxT( "1" );
            }

            if( m_XiphComment->fieldListMap().contains( "ALBUMARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_XiphComment->fieldListMap()["ALBUMARTIST"].front() );
            }
            else if( m_XiphComment->fieldListMap().contains( "ALBUM ARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_XiphComment->fieldListMap()["ALBUM ARTIST"].front() );
            }

            return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::Write( void )
{
    if( m_XiphComment )
    {
        m_XiphComment->addField( "DISCNUMBER", wxStringToTString( m_Disk ) );
        m_XiphComment->addField( "COMPOSER", wxStringToTString( m_Composer ) );
        m_XiphComment->addField( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
        m_XiphComment->addField( "ALBUMARTIST", wxStringToTString(  m_AlbumArtist ) );
    }
    return guTagInfo::Write();
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
        if( FLAC__metadata_simple_iterator_init( iter, m_FileName.mb_str( wxConvFile ), true, false ) )
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
        if( FLAC__metadata_chain_read( Chain, m_FileName.mb_str( wxConvFile ) ) )
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
    return GetXiphCommentLyrics( m_XiphComment );
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetXiphCommentLyrics( m_XiphComment, lyrics ) && m_TagFile->save();
}




// -------------------------------------------------------------------------------- //
// guOggTagInfo
// -------------------------------------------------------------------------------- //
guOggTagInfo::guOggTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_XiphComment = ( ( TagLib::Ogg::Vorbis::File * ) m_TagFile->file() )->tag();
    }
    else
        m_XiphComment = NULL;
}

// -------------------------------------------------------------------------------- //
guOggTagInfo::~guOggTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        if( m_XiphComment )
        {
            if( m_XiphComment->fieldListMap().contains( "COMPOSER" ) )
            {
                m_Composer = TStringTowxString( m_XiphComment->fieldListMap()["COMPOSER"].front() );
            }

            if( m_XiphComment->fieldListMap().contains( "DISCNUMBER" ) )
            {
                m_Disk = TStringTowxString( m_XiphComment->fieldListMap()["DISCNUMBER"].front() );
            }

            if( m_XiphComment->fieldListMap().contains( "COMPILATION" ) )
            {
                m_Compilation = TStringTowxString( m_XiphComment->fieldListMap()["COMPILATION"].front() ) == wxT( "1" );
            }

            if( m_XiphComment->fieldListMap().contains( "ALBUMARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_XiphComment->fieldListMap()["ALBUMARTIST"].front() );
            }
            else if( m_XiphComment->fieldListMap().contains( "ALBUM ARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_XiphComment->fieldListMap()["ALBUM ARTIST"].front() );
            }

            return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::Write( void )
{
    if( m_XiphComment )
    {
        m_XiphComment->addField( "DISCNUMBER", wxStringToTString( m_Disk ) );
        m_XiphComment->addField( "COMPOSER", wxStringToTString( m_Composer ) );
        m_XiphComment->addField( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
        m_XiphComment->addField( "ALBUMARTIST", wxStringToTString(  m_AlbumArtist ) );
    }
    return guTagInfo::Write();
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guOggTagInfo::GetImage( void )
{
    return GetXiphCommentCoverArt( m_XiphComment );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::SetImage( const wxImage * image )
{
    return SetXiphCommentCoverArt( m_XiphComment, image ) && m_TagFile->save();
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guOggTagInfo::GetLyrics( void )
{
    return GetXiphCommentLyrics( m_XiphComment );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetXiphCommentLyrics( m_XiphComment, lyrics ) && m_TagFile->save();
}




// -------------------------------------------------------------------------------- //
// guMp4TagInfo
// -------------------------------------------------------------------------------- //
guMp4TagInfo::guMp4TagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_Mp4Tag = ( ( TagLib::MP4::File * ) m_TagFile->file() )->tag();
    }
    else
        m_Mp4Tag = NULL;
}

// -------------------------------------------------------------------------------- //
guMp4TagInfo::~guMp4TagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        if( m_Mp4Tag )
        {
            if( m_Mp4Tag->itemListMap().contains( "aART" ) )
            {
                m_AlbumArtist = TStringTowxString( m_Mp4Tag->itemListMap()["aART"].toStringList().front() );
            }

            if( m_Mp4Tag->itemListMap().contains( "\xA9wrt" ) )
            {
                m_Composer = TStringTowxString( m_Mp4Tag->itemListMap()["\xa9wrt"].toStringList().front() );
            }

            if( m_Mp4Tag->itemListMap().contains( "disk" ) )
            {
                m_Disk = wxString::Format( wxT( "%i/%i" ),
                    m_Mp4Tag->itemListMap()["disk"].toIntPair().first,
                    m_Mp4Tag->itemListMap()["disk"].toIntPair().second );

            }

            if( m_Mp4Tag->itemListMap().contains( "cpil" ) )
            {
                m_Compilation = m_Mp4Tag->itemListMap()["cpil"].toBool();
            }

        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guStrDiskToDiskNum( const wxString &diskstr, int &disknum, int &disktotal )
{
    unsigned long Number;
    disknum = 0;
    disktotal = 0;
    wxString DiskNum = diskstr.BeforeFirst( wxT( '/' ) );
    if( !DiskNum.IsEmpty() )
    {
        if( DiskNum.ToULong( &Number ) )
        {
            disknum = Number;
            if( diskstr.Find( wxT( "/" ) ) )
            {
                DiskNum = diskstr.AfterFirst( wxT( '/' ) );
                if( DiskNum.ToULong( &Number ) )
                {
                    disktotal = Number;
                }
            }
            return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::Write( void )
{
    if( m_Mp4Tag )
    {
        m_Mp4Tag->itemListMap()["aART"] = TagLib::StringList( wxStringToTString( m_AlbumArtist ) );
        m_Mp4Tag->itemListMap()["\xA9wrt"] = TagLib::StringList( wxStringToTString( m_Composer ) );
        int first;
        int second;
        guStrDiskToDiskNum( m_Disk, first, second );
        m_Mp4Tag->itemListMap()["disk"] = TagLib::MP4::Item( first, second );
        m_Mp4Tag->itemListMap()["cpil"] = TagLib::MP4::Item( m_Compilation );
    }
    return guTagInfo::Write();
}

#ifdef TAGLIB_WITH_MP4_COVERS
// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMp4TagInfo::GetImage( void )
{
    return GetMp4Image( m_Mp4Tag );
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::SetImage( const wxImage * image )
{
    return SetMp4Image( m_Mp4Tag, image ) && m_Mp4Tag->save();
}
#endif

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMp4TagInfo::GetLyrics( void )
{
    //TagLib::MP4::File tagfile( m_FileName.mb_str( wxConvFile ) );
    return GetMp4Lyrics( ( ( TagLib::MP4::File * ) m_TagFile->file() )->tag() );
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::SetLyrics( const wxString &lyrics )
{
    return SetMp4Lyrics( ( ( TagLib::MP4::File * ) m_TagFile->file() )->tag(), lyrics ) && m_TagFile->save();
}




// -------------------------------------------------------------------------------- //
// guMpcTagInfo
// -------------------------------------------------------------------------------- //
guMpcTagInfo::guMpcTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_ApeTag = ( ( TagLib::MPC::File * ) m_TagFile->file() )->APETag();
    }
    else
        m_ApeTag = NULL;
}

// -------------------------------------------------------------------------------- //
guMpcTagInfo::~guMpcTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        if( m_ApeTag )
        {
            if( m_ApeTag->itemListMap().contains( "COMPOSER" ) )
            {
                m_Composer = TStringTowxString( m_ApeTag->itemListMap()["COMPOSER"].toStringList().front() );
            }

            if( m_ApeTag->itemListMap().contains( "DISCNUMBER" ) )
            {
                m_Disk = TStringTowxString( m_ApeTag->itemListMap()["DISCNUMBER"].toStringList().front() );
            }

            if( m_ApeTag->itemListMap().contains( "COMPILATION" ) )
            {
                m_Compilation = TStringTowxString( m_ApeTag->itemListMap()["COMPILATION"].toStringList().front() ) == wxT( "1" );
            }

            if( m_ApeTag->itemListMap().contains( "ALBUM ARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_ApeTag->itemListMap()["ALBUM ARTIST"].toStringList().front() );
            }
            else if( m_ApeTag->itemListMap().contains( "ALBUMARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_ApeTag->itemListMap()["ALBUMARTIST"].toStringList().front() );
            }
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::Write( void )
{
    if( m_ApeTag )
    {
        m_ApeTag->addValue( "COMPOSER", wxStringToTString( m_Composer ) );
        m_ApeTag->addValue( "DISCNUMBER", wxStringToTString( m_Disk ) );
        m_ApeTag->addValue( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
        m_ApeTag->addValue( "ALBUM ARTIST", wxStringToTString( m_AlbumArtist ) );
    }
    return guTagInfo::Write();
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMpcTagInfo::GetImage( void )
{
    return GetApeImage( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::SetImage( const wxImage * image )
{
    return m_ApeTag && SetApeImage( m_ApeTag, image ) && Write();
}




// -------------------------------------------------------------------------------- //
// guWavPackTagInfo
// -------------------------------------------------------------------------------- //
guWavPackTagInfo::guWavPackTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_ApeTag = ( ( TagLib::WavPack::File * ) m_TagFile->file() )->APETag();
    }
    else
        m_ApeTag = NULL;
}

// -------------------------------------------------------------------------------- //
guWavPackTagInfo::~guWavPackTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        if( m_ApeTag )
        {
            if( m_ApeTag->itemListMap().contains( "COMPOSER" ) )
            {
                m_Composer = TStringTowxString( m_ApeTag->itemListMap()["COMPOSER"].toStringList().front() );
            }

            if( m_ApeTag->itemListMap().contains( "DISCNUMBER" ) )
            {
                m_Disk = TStringTowxString( m_ApeTag->itemListMap()["DISCNUMBER"].toStringList().front() );
            }

            if( m_ApeTag->itemListMap().contains( "COMPILATION" ) )
            {
                m_Compilation = TStringTowxString( m_ApeTag->itemListMap()["COMPILATION"].toStringList().front() ) == wxT( "1" );
            }

            if( m_ApeTag->itemListMap().contains( "ALBUM ARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_ApeTag->itemListMap()["ALBUM ARTIST"].toStringList().front() );
            }
            else if( m_ApeTag->itemListMap().contains( "ALBUMARTIST" ) )
            {
                m_AlbumArtist = TStringTowxString( m_ApeTag->itemListMap()["ALBUMARTIST"].toStringList().front() );
            }
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::Write( void )
{
    if( m_ApeTag )
    {
        m_ApeTag->addValue( "COMPOSER", wxStringToTString( m_Composer ) );
        m_ApeTag->addValue( "DISCNUMBER", wxStringToTString( m_Disk ) );
        m_ApeTag->addValue( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
        m_ApeTag->addValue( "ALBUM ARTIST", wxStringToTString( m_AlbumArtist ) );
    }
    return guTagInfo::Write();
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guWavPackTagInfo::GetImage( void )
{
    return GetApeImage( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::SetImage( const wxImage * image )
{
    return m_ApeTag && SetApeImage( m_ApeTag, image ) && Write();
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guWavPackTagInfo::GetLyrics( void )
{
    return GetApeLyrics( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetApeLyrics( m_ApeTag, lyrics ) && m_TagFile->save();
}



// -------------------------------------------------------------------------------- //
// guApeTagInfo
// -------------------------------------------------------------------------------- //
guApeTagInfo::guApeTagInfo( const wxString &filename ) : guTagInfo(), m_ApeFile( filename )
{
    //wxASSERT( !m_FileName.Lower().EndsWith( wxT( ".ape" ) ) );
}

// -------------------------------------------------------------------------------- //
guApeTagInfo::~guApeTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Read( void )
{
    guApeTag * Tag = m_ApeFile.GetApeTag();
    if( Tag )
    {
        m_TrackName = Tag->GetTitle();
        if( m_TrackName.IsEmpty() )
            m_TrackName = m_FileName.AfterLast( wxT( '/' ) );
        m_ArtistName = Tag->GetArtist();
        if( m_ArtistName.IsEmpty() )
            m_ArtistName = _( "Unknown" );
        m_AlbumName = Tag->GetAlbum();
        if( m_AlbumName.IsEmpty() )
            m_AlbumName = m_FileName.BeforeLast( wxT( '/' ) ).AfterLast( wxT( '/' ) );
        m_GenreName = Tag->GetGenre();
        if( m_GenreName.IsEmpty() )
            m_GenreName = _( "Unknown" );
        m_Track = Tag->GetTrack();
        m_Year = Tag->GetYear();
        m_Length = m_ApeFile.GetTrackLength();
        m_Bitrate = m_ApeFile.GetBitRate();

        m_Comments = Tag->GetItemValue( APE_TAG_KEY_COMMENT );
        m_Composer = Tag->GetItemValue( APE_TAG_KEY_COMPOSER );
        m_Disk = Tag->GetItemValue( APE_TAG_KEY_MEDIA );
        m_AlbumArtist = Tag->GetItemValue( APE_TAG_KEY_ALBUMARTIST );
        if( m_AlbumArtist.IsEmpty() )
            m_AlbumArtist = Tag->GetItemValue( wxT( "AlbumArtist" ) );

        return true;
    }
    else
    {
        guLogError( wxT( "Ape file with no tags found" ) );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Write( void )
{
    guApeTag * Tag = m_ApeFile.GetApeTag();
    if( Tag )
    {
        Tag->SetTitle( m_TrackName );
        Tag->SetArtist( m_ArtistName );
        Tag->SetAlbum( m_AlbumName );
        Tag->SetGenre( m_GenreName );
        Tag->SetTrack( m_Track );
        Tag->SetYear( m_Year );
        Tag->SetItem( APE_TAG_KEY_COMMENT, m_Comments );
        Tag->SetItem( APE_TAG_KEY_COMPOSER, m_Composer );
        Tag->SetItem( APE_TAG_KEY_MEDIA, m_Disk );
        Tag->SetItem( APE_TAG_KEY_ALBUMARTIST, m_AlbumArtist );

        m_ApeFile.WriteApeTag();
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guApeTagInfo::GetLyrics( void )
{
    guApeTag * Tag = m_ApeFile.GetApeTag();
    if( Tag )
        return Tag->GetItemValue( APE_TAG_KEY_LYRICS );
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::SetLyrics( const wxString &lyrics )
{
    guApeTag * Tag = m_ApeFile.GetApeTag();
    if( Tag )
    {
        Tag->SetItem( APE_TAG_KEY_LYRICS, lyrics );
        return m_ApeFile.WriteApeTag();
    }
    return false;
}



// -------------------------------------------------------------------------------- //
// guTrueAudioTagInfo
// -------------------------------------------------------------------------------- //
guTrueAudioTagInfo::guTrueAudioTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_TagId3v2 = ( ( TagLib::TrueAudio::File * ) m_TagFile->file() )->ID3v2Tag();
    }
    else
    {
        m_TagId3v2 = NULL;
    }
}

// -------------------------------------------------------------------------------- //
guTrueAudioTagInfo::~guTrueAudioTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        // If its a ID3v2 Tag try to load the labels
        if( m_TagId3v2 )
        {
            if( m_TagId3v2->frameListMap().contains( "TPOS" ) )
            {
                m_Disk = TStringTowxString( m_TagId3v2->frameListMap()[ "TPOS" ].front()->toString() );
            }

            if( m_TagId3v2->frameListMap().contains( "TCOM" ) )
            {
                m_Composer = TStringTowxString( m_TagId3v2->frameListMap()[ "TCOM" ].front()->toString() );
            }

            if( m_TagId3v2->frameListMap().contains( "TPE2" ) )
            {
                m_AlbumArtist = TStringTowxString( m_TagId3v2->frameListMap()[ "TPE2" ].front()->toString() );
            }

            if( m_TagId3v2->frameListMap().contains( "TCMP" ) )
            {
                m_Compilation = TStringTowxString( m_TagId3v2->frameListMap()[ "TCMP" ].front()->toString() ) == wxT( "1" );
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
bool guTrueAudioTagInfo::Write( void )
{
    if( m_TagId3v2 )
    {
        TagLib::ID3v2::TextIdentificationFrame * frame;
        m_TagId3v2->removeFrames( "TPOS" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TPOS" );
        frame->setText( wxStringToTString( m_Disk ) );
        m_TagId3v2->addFrame( frame );

        m_TagId3v2->removeFrames( "TCOM" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TCOM" );
        frame->setText( wxStringToTString( m_Composer ) );
        m_TagId3v2->addFrame( frame );

        m_TagId3v2->removeFrames( "TPE2" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TPE2" );
        frame->setText( wxStringToTString( m_AlbumArtist ) );
        m_TagId3v2->addFrame( frame );

        m_TagId3v2->removeFrames( "TCMP" );
        frame = new TagLib::ID3v2::TextIdentificationFrame( "TCMP" );
        frame->setText( wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
        m_TagId3v2->addFrame( frame );

        // I have found several TRCK fields in the mp3s
        m_TagId3v2->removeFrames( "TRCK" );
        m_TagId3v2->setTrack( m_Track );

    }

    return guTagInfo::Write();
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guTrueAudioTagInfo::GetImage( void )
{
    if( m_TagId3v2 )
    {
        return GetID3v2Image( m_TagId3v2 );
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::SetImage( const wxImage * image )
{
    if( m_TagId3v2 )
    {
        SetID3v2Image( m_TagId3v2, image );
    }
    else
        return false;

    return m_TagFile->save();
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guTrueAudioTagInfo::GetLyrics( void )
{
    if( m_TagId3v2 )
    {
        return GetID3v2Lyrics( m_TagId3v2  );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::SetLyrics( const wxString &lyrics )
{
	if( m_TagId3v2 )
    {
        SetID3v2Lyrics( m_TagId3v2, lyrics );
        return m_TagFile->save();
    }
    return false;
}




// -------------------------------------------------------------------------------- //
// guASFTagInfo
// -------------------------------------------------------------------------------- //
guASFTagInfo::guASFTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_ASFTag = ( ( TagLib::ASF::File * ) m_TagFile->file() )->tag();
    }
    else
    {
        m_ASFTag = NULL;
    }
}

// -------------------------------------------------------------------------------- //
guASFTagInfo::~guASFTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        if( m_ASFTag )
        {
            if( m_ASFTag->attributeListMap().contains( "WM/PartOfSet" ) )
            {
                m_Disk = TStringTowxString( m_ASFTag->attributeListMap()[ "WM/PartOfSet" ].front().toString() );
            }

            if( m_ASFTag->attributeListMap().contains( "WM/Composer" ) )
            {
                m_Composer = TStringTowxString( m_ASFTag->attributeListMap()[ "WM/Composer" ].front().toString() );
            }

            if( m_ASFTag->attributeListMap().contains( "WM/AlbumArtist" ) )
            {
                m_AlbumArtist = TStringTowxString( m_ASFTag->attributeListMap()[ "WM/AlbumArtist" ].front().toString() );
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
bool guASFTagInfo::Write( void )
{
    if( m_ASFTag )
    {
        m_ASFTag->removeItem( "WM/PartOfSet" );
        m_ASFTag->setAttribute( "WM/PartOfSet", wxStringToTString( m_Disk ) );

        m_ASFTag->removeItem( "WM/Composer" );
        m_ASFTag->setAttribute( "WM/Composer", wxStringToTString( m_Composer ) );

        m_ASFTag->removeItem( "WM/AlbumArtist" );
        m_ASFTag->setAttribute( "WM/AlbumArtist", wxStringToTString( m_AlbumArtist ) );
    }

    return guTagInfo::Write();
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::CanHandleImages( void )
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxImage * guASFTagInfo::GetImage( void )
{
    if( m_ASFTag )
    {
        return GetASFImage( m_ASFTag );
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::SetImage( const wxImage * image )
{
    if( m_ASFTag )
    {
        SetASFImage( m_ASFTag, image );
    }
    else
        return false;

    return m_TagFile->save();
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guASFTagInfo::GetLyrics( void )
{
    if( m_ASFTag )
    {
        if( m_ASFTag->attributeListMap().contains( "WM/Lyrics" ) )
        {
            return TStringTowxString( m_ASFTag->attributeListMap()[ "WM/Lyrics" ].front().toString() );
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::SetLyrics( const wxString &lyrics )
{
	if( m_ASFTag )
    {
        m_ASFTag->removeItem( "WM/Lyrics" );
        m_ASFTag->setAttribute( "WM/Lyrics", wxStringToTString( lyrics ) );
        return m_TagFile->save();
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
bool guTagSetPicture( const wxString &filename, const wxString &imagefile )
{
    wxImage Image( imagefile );
    if( Image.IsOk() )
    {
        return guTagSetPicture( filename, &Image );
    }
    return false;
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
void guUpdateImages( const guTrackArray &songs, const guImagePtrArray &images )
{
    int Index;
    int Count = images.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTagSetPicture( songs[ Index ].m_FileName, images[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guUpdateLyrics( const guTrackArray &songs, const wxArrayString &lyrics )
{
    int Index;
    int Count = lyrics.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTagSetLyrics( songs[ Index ].m_FileName, lyrics[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
