// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2013 J.Rios
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
#include "TagInfo.h"
#include "Utils.h"

#include "ApeTag.h"
#include "Base64.h"
#include "MainFrame.h"
#include "TrackEdit.h"

#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>

#include <asfattribute.h>
#include <popularimeterframe.h>
#include <id3v1tag.h>

wxArrayString guSupportedFormats;
wxMutex       guSupportedFormatsMutex;

// -------------------------------------------------------------------------------- //
bool guIsValidAudioFile( const wxString &filename )
{
    guSupportedFormatsMutex.Lock();
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

    int Position = guSupportedFormats.Index( filename.Lower().AfterLast( wxT( '.' ) ) );

    guSupportedFormatsMutex.Unlock();

    return ( Position != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
guTagInfo * guGetTagInfoHandler( const wxString &filename )
{
    guSupportedFormatsMutex.Lock();
    int FormatIndex = guSupportedFormats.Index( filename.Lower().AfterLast( wxT( '.' ) ) );
    guSupportedFormatsMutex.Unlock();
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

#ifdef TAGLIB_WITH_APE_SUPPORT
        case 11 : return new guApeTagInfo( filename );
#endif

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
TagLib::ID3v2::PopularimeterFrame * GetPopM( TagLib::ID3v2::Tag * tag, const TagLib::String &email )
{
    TagLib::ID3v2::FrameList PopMList = tag->frameList( "POPM" );
    for( TagLib::ID3v2::FrameList::Iterator it = PopMList.begin(); it != PopMList.end(); ++it )
    {
        TagLib::ID3v2::PopularimeterFrame * PopMFrame = static_cast<TagLib::ID3v2::PopularimeterFrame *>( * it );
        //guLogMessage( wxT( "PopM e: '%s'  r: %i  c: %i" ), TStringTowxString( PopMFrame->email() ).c_str(), PopMFrame->rating(), PopMFrame->counter() );
        if( email.isEmpty() || ( PopMFrame->email() == email ) )
        {
            return PopMFrame;
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
int inline guPopMToRating( const int rating )
{
    if( rating < 0 )
        return 0;
    if( rating == 0 )
        return 0;
    if( rating < 64 )
        return 1;
    if( rating < 128 )
        return 2;
    if( rating < 192 )
        return 3;
    if( rating < 255 )
        return 4;
    return 5;
}

// -------------------------------------------------------------------------------- //
int inline guWMRatingToRating( const int rating )
{
    if( rating <= 0 )
        return 0;
    if( rating < 25 )
        return 1;
    if( rating < 50 )
        return 2;
    if( rating < 75 )
        return 3;
    if( rating < 99 )
        return 4;
    return 5;
}

// -------------------------------------------------------------------------------- //
int inline guRatingToPopM( const int rating )
{
    int Ratings[] = { 0, 0, 1, 64, 128, 192, 255 };
    guLogMessage( wxT( "Rating: %i => %i" ), rating, Ratings[ rating + 1 ] );
    return Ratings[ rating + 1 ];
}


// -------------------------------------------------------------------------------- //
wxImage * GetID3v2ImageType( TagLib::ID3v2::FrameList &framelist,
            int frametype  = TagLib::ID3v2::AttachedPictureFrame::FrontCover );

wxImage * GetID3v2ImageType( TagLib::ID3v2::FrameList &framelist,
            int frametype )
{
    TagLib::ID3v2::AttachedPictureFrame * PicFrame;
    for( std::list<TagLib::ID3v2::Frame*>::iterator iter = framelist.begin(); iter != framelist.end(); iter++ )
    {
        PicFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>( *iter );

        if( ( frametype == wxNOT_FOUND ) || ( PicFrame->type() == frametype ) )
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
        if( !CoverImage )
        {
            CoverImage = GetID3v2ImageType( FrameList, wxNOT_FOUND );
        }
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
    m_TagFile = NULL;
    m_Tag = NULL;

    SetFileName( filename );

    m_Track = 0;
    m_Year = 0;
    m_Length = 0;
    m_Bitrate = 0;
    m_Rating = wxNOT_FOUND;
    m_PlayCount = 0;
    m_Compilation = false;
}

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
    }

    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_Tag = m_TagFile->tag();
        if( !m_Tag )
        {
            guLogWarning( wxT( "Cant get tag object from '%s'" ), filename.c_str() );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Read( void )
{
    AudioProperties * apro;
    if( m_Tag )
    {
        m_TrackName = TStringTowxString( m_Tag->title() );
        m_ArtistName = TStringTowxString( m_Tag->artist() );
        m_AlbumName = TStringTowxString( m_Tag->album() );
        m_GenreName = TStringTowxString( m_Tag->genre() );
        m_Comments = TStringTowxString( m_Tag->comment() );
        m_Track = m_Tag->track();
        m_Year = m_Tag->year();
    }

    if( m_TagFile && m_Tag && ( apro = m_TagFile->audioProperties() ) )
    {
        m_Length = apro->length() * 1000;
        m_Bitrate = apro->bitrate();
        //m_Samplerate = apro->sampleRate();
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Write( const int changedflag )
{
    if( m_Tag && ( changedflag & guTRACK_CHANGED_DATA_TAGS ) )
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
void ID3v2_CheckLabelFrame( ID3v2::Tag * tagv2, const char * description, const wxString &value )
{
    ID3v2::UserTextIdentificationFrame * frame;
    //guLogMessage( wxT( "USERTEXT[ '%s' ] = '%s'" ), wxString( description, wxConvUTF8 ).c_str(), value.c_str() );
    frame = ID3v2::UserTextIdentificationFrame::find( tagv2, description );
    if( !frame )
    {
        frame = new ID3v2::UserTextIdentificationFrame( TagLib::String::UTF8 );
        tagv2->addFrame( frame );
        //frame->setDescription( TagLib::String( description, TagLib::String::UTF8 ) );
        frame->setDescription( description );
    }

    if( frame )
    {
        frame->setText( wxStringToTString( value ) );
    }
}

// -------------------------------------------------------------------------------- //
void Xiph_CheckLabelFrame( Ogg::XiphComment * xiphcomment, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if( xiphcomment->fieldListMap().contains( description ) )
    {
        if( !value.IsEmpty() )
        {
            xiphcomment->addField( description, wxStringToTString( value ) );
        }
        else
        {
            xiphcomment->removeField( description );
        }
    }
    else
    {
        if( !value.IsEmpty() )
        {
            xiphcomment->addField( description, wxStringToTString( value ) );
        }
    }
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
void Mp4_CheckLabelFrame( TagLib::MP4::Tag * mp4tag, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if( mp4tag->itemListMap().contains( description ) )
    {
        if( !value.IsEmpty() )
        {
            mp4tag->itemListMap()[ description ] = TagLib::MP4::Item( TagLib::StringList( wxStringToTString( value ) ) );
        }
        else
        {
            mp4tag->itemListMap().erase( description );
        }
    }
    else
    {
        if( !value.IsEmpty() )
        {
            mp4tag->itemListMap().insert( description, TagLib::MP4::Item( TagLib::StringList( wxStringToTString( value ) ) ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void Ape_CheckLabelFrame( TagLib::APE::Tag * apetag, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if( apetag->itemListMap().contains( description ) )
        apetag->removeItem( description );
    if( !value.IsEmpty() )
    {
        apetag->addValue( description, wxStringToTString( value ) );
    }
}

// -------------------------------------------------------------------------------- //
void ASF_CheckLabelFrame( ASF::Tag * asftag, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if( asftag->attributeListMap().contains( description ) )
        asftag->removeItem( description );
    if( !value.IsEmpty() )
    {
        asftag->setAttribute( description, wxStringToTString( value ) );
    }
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( ID3v2::Tag * tag )
{
    if( tag )
    {
        if( tag->frameListMap().contains( "TPOS" ) )
        {
            m_Disk = TStringTowxString( tag->frameListMap()[ "TPOS" ].front()->toString() );
        }

        if( tag->frameListMap().contains( "TCOM" ) )
        {
            m_Composer = TStringTowxString( tag->frameListMap()[ "TCOM" ].front()->toString() );
        }

        if( tag->frameListMap().contains( "TPE2" ) )
        {
            m_AlbumArtist = TStringTowxString( tag->frameListMap()[ "TPE2" ].front()->toString() );
        }

        if( tag->frameListMap().contains( "TCMP" ) )
        {
            m_Compilation = TStringTowxString( tag->frameListMap()[ "TCMP" ].front()->toString() ) == wxT( "1" );
        }

        TagLib::ID3v2::PopularimeterFrame * PopMFrame = NULL;

        PopMFrame = GetPopM( tag, "Guayadeque" );
        if( !PopMFrame )
            PopMFrame = GetPopM( tag, "" );

        if( PopMFrame )
        {
            m_Rating = guPopMToRating( PopMFrame->rating() );
            m_PlayCount = PopMFrame->counter();
        }


        if( m_TrackLabels.Count() == 0 )
        {
            ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tag, "TRACK_LABELS" );
            if( !Frame )
                Frame = ID3v2::UserTextIdentificationFrame::find( tag, "guTRLABELS" );
            if( Frame )
            {
                //guLogMessage( wxT( "*Track Label: '%s'" ), TStringTowxString( Frame->fieldList()[ 1 ] ).c_str() );
                // [guTRLABELS] guTRLABELS labels
                StringList TrLabelsList = Frame->fieldList();
                if( TrLabelsList.size() )
                {
                    m_TrackLabelsStr = TStringTowxString( TrLabelsList[ 1 ] );
                    m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tag, "ARTIST_LABELS" );
            if( !Frame )
                Frame = ID3v2::UserTextIdentificationFrame::find( tag, "guARLABELS" );
            if( Frame )
            {
                //guLogMessage( wxT( "*Artist Label: '%s'" ), TStringTowxString( Frame->fieldList()[ 1 ] ).c_str() );
                StringList ArLabelsList = Frame->fieldList();
                if( ArLabelsList.size() )
                {
                    m_ArtistLabelsStr = TStringTowxString( ArLabelsList[ 1 ] );
                    m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
                }
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tag, "ALBUM_LABELS" );
            if( !Frame )
                Frame = ID3v2::UserTextIdentificationFrame::find( tag, "guALLABELS" );
            if( Frame )
            {
                //guLogMessage( wxT( "*Album Label: '%s'" ), TStringTowxString( Frame->fieldList()[ 1 ] ).c_str() );
                StringList AlLabelsList = Frame->fieldList();
                if( AlLabelsList.size() )
                {
                    m_AlbumLabelsStr = TStringTowxString( AlLabelsList[ 1 ] );
                    m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
                }
            }
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( ID3v2::Tag * tag, const int changedflag )
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            TagLib::ID3v2::TextIdentificationFrame * frame;
            tag->removeFrames( "TPOS" );
            frame = new TagLib::ID3v2::TextIdentificationFrame( "TPOS" );
            frame->setText( wxStringToTString( m_Disk ) );
            tag->addFrame( frame );

            tag->removeFrames( "TCOM" );
            frame = new TagLib::ID3v2::TextIdentificationFrame( "TCOM" );
            frame->setText( wxStringToTString( m_Composer ) );
            tag->addFrame( frame );

            tag->removeFrames( "TPE2" );
            frame = new TagLib::ID3v2::TextIdentificationFrame( "TPE2" );
            frame->setText( wxStringToTString( m_AlbumArtist ) );
            tag->addFrame( frame );

            //tag->removeFrames( "TCMP" );
            //frame = new TagLib::ID3v2::TextIdentificationFrame( "TCMP" );
            //frame->setText( wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
            //tag->addFrame( frame );

            // I have found several TRCK fields in the mp3s
            tag->removeFrames( "TRCK" );
            tag->setTrack( m_Track );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            guLogMessage( wxT( "Writing ratings and playcount..." ) );
            TagLib::ID3v2::PopularimeterFrame * PopMFrame = GetPopM( tag, "Guayadeque" );
            if( !PopMFrame )
            {
                PopMFrame = new TagLib::ID3v2::PopularimeterFrame();
                tag->addFrame( PopMFrame );
                PopMFrame->setEmail( "Guayadeque" );
            }
            PopMFrame->setRating( guRatingToPopM( m_Rating ) );
            PopMFrame->setCounter( m_PlayCount );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            ID3v2_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            ID3v2_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            ID3v2_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( Ogg::XiphComment * tag )
{
    if( tag )
    {
        if( tag->fieldListMap().contains( "COMPOSER" ) )
        {
            m_Composer = TStringTowxString( tag->fieldListMap()["COMPOSER"].front() );
        }

        if( tag->fieldListMap().contains( "DISCNUMBER" ) )
        {
            m_Disk = TStringTowxString( tag->fieldListMap()["DISCNUMBER"].front() );
        }

        if( tag->fieldListMap().contains( "COMPILATION" ) )
        {
            m_Compilation = TStringTowxString( tag->fieldListMap()["COMPILATION"].front() ) == wxT( "1" );
        }

        if( tag->fieldListMap().contains( "ALBUMARTIST" ) )
        {
            m_AlbumArtist = TStringTowxString( tag->fieldListMap()["ALBUMARTIST"].front() );
        }
        else if( tag->fieldListMap().contains( "ALBUM ARTIST" ) )
        {
            m_AlbumArtist = TStringTowxString( tag->fieldListMap()["ALBUM ARTIST"].front() );
        }

        // Rating
        if( tag->fieldListMap().contains( "RATING" ) )
        {
            long Rating = 0;
            if( TStringTowxString( tag->fieldListMap()["RATING"].front() ).ToLong( &Rating ) )
            {
                if( Rating )
                {
                    if( Rating > 5 )
                    {
                        m_Rating = guPopMToRating( Rating );
                    }
                    else
                    {
                        m_Rating = Rating;
                    }
                }
            }
        }

        if( tag->fieldListMap().contains( "PLAY_COUNTER" ) )
        {
            long PlayCount = 0;
            if( TStringTowxString( tag->fieldListMap()["PLAY_COUNTER"].front() ).ToLong( &PlayCount ) )
            {
                m_PlayCount = PlayCount;
            }
        }

        // Labels
        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->fieldListMap().contains( "TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->fieldListMap()["TRACK_LABELS"].front() );
                //guLogMessage( wxT( "*Track Label: '%s'\n" ), m_TrackLabelsStr.c_str() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->fieldListMap().contains( "ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->fieldListMap()["ARTIST_LABELS"].front() );
                //guLogMessage( wxT( "*Artist Label: '%s'\n" ), m_ArtistLabelsStr.c_str() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->fieldListMap().contains( "ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->fieldListMap()["ALBUM_LABELS"].front() );
                //guLogMessage( wxT( "*Album Label: '%s'\n" ), m_AlbumLabelsStr.c_str() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }

        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( Ogg::XiphComment * tag, const int changedflag )
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->addField( "DISCNUMBER", wxStringToTString( m_Disk ) );
            tag->addField( "COMPOSER", wxStringToTString( m_Composer ) );
            tag->addField( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
            tag->addField( "ALBUMARTIST", wxStringToTString(  m_AlbumArtist ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            tag->addField( "RATING", wxStringToTString( wxString::Format( wxT( "%u" ), guRatingToPopM( m_Rating ) ) ) );
            tag->addField( "PLAY_COUNTER", wxStringToTString( wxString::Format( wxT( "%u" ), m_PlayCount ) ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            Xiph_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            Xiph_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            Xiph_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( MP4::Tag * tag )
{
    if( tag )
    {
        if( tag->itemListMap().contains( "aART" ) )
        {
            m_AlbumArtist = TStringTowxString( tag->itemListMap()["aART"].toStringList().front() );
        }

        if( tag->itemListMap().contains( "\xA9wrt" ) )
        {
            m_Composer = TStringTowxString( tag->itemListMap()["\xa9wrt"].toStringList().front() );
        }

        if( tag->itemListMap().contains( "disk" ) )
        {
            m_Disk = wxString::Format( wxT( "%i/%i" ),
                tag->itemListMap()["disk"].toIntPair().first,
                tag->itemListMap()["disk"].toIntPair().second );

        }

        if( tag->itemListMap().contains( "cpil" ) )
        {
            m_Compilation = tag->itemListMap()["cpil"].toBool();
        }

        // Rating
        if( tag->itemListMap().contains( "----:com.apple.iTunes:RATING" ) )
        {
            long Rating = 0;
            if( TStringTowxString( tag->itemListMap()["----:com.apple.iTunes:RATING"].toStringList().front() ).ToLong( &Rating ) )
            {
                if( Rating )
                {
                    if( Rating > 5 )
                    {
                        m_Rating = guPopMToRating( Rating );
                    }
                    else
                    {
                        m_Rating = Rating;
                    }
                }
            }
        }

        if( tag->itemListMap().contains( "----:com.apple.iTunes:PLAY_COUNTER" ) )
        {
            long PlayCount = 0;
            if( TStringTowxString( tag->itemListMap()["----:com.apple.iTunes:PLAY_COUNTER"].toStringList().front()  ).ToLong( &PlayCount ) )
            {
                m_PlayCount = PlayCount;
            }
        }

        // Labels
        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "----:com.apple.iTunes:TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->itemListMap()["----:com.apple.iTunes:TRACK_LABELS"].toStringList().front() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "----:com.apple.iTunes:ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->itemListMap()["----:com.apple.iTunes:ARTIST_LABELS"].toStringList().front() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "----:com.apple.iTunes:ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->itemListMap()["----:com.apple.iTunes:ALBUM_LABELS"].toStringList().front() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }

        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( MP4::Tag * tag, const int changedflag )
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->itemListMap()["aART"] = TagLib::StringList( wxStringToTString( m_AlbumArtist ) );
            tag->itemListMap()["\xA9wrt"] = TagLib::StringList( wxStringToTString( m_Composer ) );
            int first;
            int second;
            guStrDiskToDiskNum( m_Disk, first, second );
            tag->itemListMap()["disk"] = TagLib::MP4::Item( first, second );
            tag->itemListMap()["cpil"] = TagLib::MP4::Item( m_Compilation );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            tag->itemListMap()["----:com.apple.iTunes:RATING" ] = TagLib::MP4::Item( wxStringToTString( wxString::Format( wxT( "%u" ), guRatingToPopM( m_Rating ) ) ) );
            tag->itemListMap()[ "----:com.apple.iTunes:PLAY_COUNTER" ] = TagLib::MP4::Item( wxStringToTString( wxString::Format( wxT( "%u" ), m_PlayCount ) ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            Mp4_CheckLabelFrame( tag, "----:com.apple.iTunes:ARTIST_LABELS", m_ArtistLabelsStr );
            Mp4_CheckLabelFrame( tag, "----:com.apple.iTunes:ALBUM_LABELS", m_AlbumLabelsStr );
            Mp4_CheckLabelFrame( tag, "----:com.apple.iTunes:TRACK_LABELS", m_TrackLabelsStr );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( APE::Tag * tag )
{
    if( tag )
    {
        if( tag->itemListMap().contains( "COMPOSER" ) )
        {
            m_Composer = TStringTowxString( tag->itemListMap()["COMPOSER"].toStringList().front() );
        }

        if( tag->itemListMap().contains( "DISCNUMBER" ) )
        {
            m_Disk = TStringTowxString( tag->itemListMap()["DISCNUMBER"].toStringList().front() );
        }

        if( tag->itemListMap().contains( "COMPILATION" ) )
        {
            m_Compilation = TStringTowxString( tag->itemListMap()["COMPILATION"].toStringList().front() ) == wxT( "1" );
        }

        if( tag->itemListMap().contains( "ALBUM ARTIST" ) )
        {
            m_AlbumArtist = TStringTowxString( tag->itemListMap()["ALBUM ARTIST"].toStringList().front() );
        }
        else if( tag->itemListMap().contains( "ALBUMARTIST" ) )
        {
            m_AlbumArtist = TStringTowxString( tag->itemListMap()["ALBUMARTIST"].toStringList().front() );
        }

        // Rating
        if( tag->itemListMap().contains( "RATING" ) )
        {
            long Rating = 0;
            if( TStringTowxString( tag->itemListMap()["RATING"].toStringList().front() ).ToLong( &Rating ) )
            {
                if( Rating )
                {
                    if( Rating > 5 )
                    {
                        m_Rating = guPopMToRating( Rating );
                    }
                    else
                    {
                        m_Rating = Rating;
                    }
                }
            }
        }

        if( tag->itemListMap().contains( "PLAY_COUNTER" ) )
        {
            long PlayCount = 0;
            if( TStringTowxString( tag->itemListMap()["PLAY_COUNTER"].toStringList().front() ).ToLong( &PlayCount ) )
            {
                m_PlayCount = PlayCount;
            }
        }

        // Labels
        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->itemListMap()["TRACK_LABELS"].toStringList().front() );
                //guLogMessage( wxT( "*Track Label: '%s'\n" ), m_TrackLabelsStr.c_str() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->itemListMap()["ARTIST_LABELS"].toStringList().front() );
                //guLogMessage( wxT( "*Artist Label: '%s'\n" ), m_ArtistLabelsStr.c_str() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->itemListMap()["ALBUM_LABELS"].toStringList().front() );
                //guLogMessage( wxT( "*Album Label: '%s'\n" ), m_AlbumLabelsStr.c_str() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }

        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( APE::Tag * tag, const int changedflag )
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->addValue( "COMPOSER", wxStringToTString( m_Composer ) );
            tag->addValue( "DISCNUMBER", wxStringToTString( m_Disk ) );
            tag->addValue( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
            tag->addValue( "ALBUM ARTIST", wxStringToTString( m_AlbumArtist ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            tag->addValue( "RATING", wxStringToTString( wxString::Format( wxT( "%u" ), guRatingToPopM( m_Rating ) ) ) );
            tag->addValue( "PLAY_COUNTER", wxStringToTString( wxString::Format( wxT( "%u" ), m_PlayCount ) ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            Ape_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            Ape_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            Ape_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }

        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( ASF::Tag * tag )
{
    if( tag )
    {
        if( tag->attributeListMap().contains( "WM/PartOfSet" ) )
        {
            m_Disk = TStringTowxString( tag->attributeListMap()[ "WM/PartOfSet" ].front().toString() );
        }

        if( tag->attributeListMap().contains( "WM/Composer" ) )
        {
            m_Composer = TStringTowxString( tag->attributeListMap()[ "WM/Composer" ].front().toString() );
        }

        if( tag->attributeListMap().contains( "WM/AlbumArtist" ) )
        {
            m_AlbumArtist = TStringTowxString( tag->attributeListMap()[ "WM/AlbumArtist" ].front().toString() );
        }

        long Rating = 0;
        if( tag->attributeListMap().contains( "WM/SharedUserRating" ) )
        {
            TStringTowxString( tag->attributeListMap()[ "WM/SharedUserRating" ].front().toString() ).ToLong( &Rating );
        }

        if( !Rating && tag->attributeListMap().contains( "Rating" ) )
        {
            TStringTowxString( tag->attributeListMap()[ "Rating" ].front().toString() ).ToLong( &Rating );
        }

        if( Rating )
        {
            if( Rating > 5 )
            {
                m_Rating = guWMRatingToRating( Rating );
            }
            else
            {
                m_Rating = Rating;
            }
        }


        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->attributeListMap().contains( "TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->attributeListMap()[ "TRACK_LABELS" ].front().toString() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->attributeListMap().contains( "ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->attributeListMap()[ "ARTIST_LABELS" ].front().toString() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->attributeListMap().contains( "ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->attributeListMap()[ "ALBUM_LABELS" ].front().toString() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }

        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( ASF::Tag * tag, const int changedflag )
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->removeItem( "WM/PartOfSet" );
            tag->setAttribute( "WM/PartOfSet", wxStringToTString( m_Disk ) );

            tag->removeItem( "WM/Composer" );
            tag->setAttribute( "WM/Composer", wxStringToTString( m_Composer ) );

            tag->removeItem( "WM/AlbumArtist" );
            tag->setAttribute( "WM/AlbumArtist", wxStringToTString( m_AlbumArtist ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
             tag->removeItem( "WM/SharedUserRating" );
             int WMRatings[] = { 0, 0, 1, 25, 50, 75, 99 };
             tag->setAttribute( "WM/SharedUserRating", wxStringToTString( wxString::Format( wxT( "%i" ), WMRatings[ m_Rating + 1 ] ) ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            ASF_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            ASF_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            ASF_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }
        return true;
    }
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
            ReadExtendedTags( m_TagId3v2 );
        }
    }
    else
    {
      guLogError( wxT( "Could not read tags from file '%s'" ), m_FileName.c_str() );
    }

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::Write( const int changedflag )
{
    if( m_TagId3v2 )
    {
        WriteExtendedTags( m_TagId3v2, changedflag );
    }

    return guTagInfo::Write( changedflag );
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

    return true;
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
        return true;
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
        ReadExtendedTags( m_XiphComment );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_XiphComment, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::CanHandleImages( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * GetFlacImage( const wxString &filename, int imagetype )
{
    wxImage * CoverImage = NULL;

    FLAC__Metadata_SimpleIterator * iter = FLAC__metadata_simple_iterator_new();
    if( iter )
    {
        if( FLAC__metadata_simple_iterator_init( iter, filename.mb_str( wxConvFile ), true, false ) )
        {
            while( !CoverImage && FLAC__metadata_simple_iterator_next( iter ) )
            {
                if( FLAC__metadata_simple_iterator_get_block_type( iter ) == FLAC__METADATA_TYPE_PICTURE )
                {
                    FLAC__StreamMetadata * block = FLAC__metadata_simple_iterator_get_block( iter );

                    if( block->data.picture.type == imagetype )
                    {
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
wxImage * guFlacTagInfo::GetImage( void )
{
    wxImage * CoverImage = NULL;

    CoverImage = GetFlacImage( m_FileName, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER );
    if( !CoverImage )
    {
        CoverImage = GetFlacImage( m_FileName, FLAC__STREAM_METADATA_PICTURE_TYPE_OTHER );
    }

    return CoverImage;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::SetImage( const wxImage * image )
{
    bool RetVal = false;
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
                        }
                        else
                        {
                            FLAC__metadata_object_delete( Picture );
                        }
                    }
                }

                FLAC__metadata_chain_sort_padding( Chain );
                if( !FLAC__metadata_chain_write( Chain, TRUE, TRUE ) )
                {
                    guLogError( wxT( "Could not save the FLAC file" ) );
                }
                else
                {
                    RetVal = true;
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

        FLAC__metadata_chain_delete( Chain );
    }
    else
    {
        guLogError( wxT( "Could not create a FLAC chain." ) );
    }
    return RetVal;
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
    return SetXiphCommentLyrics( m_XiphComment, lyrics );
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
        ReadExtendedTags( m_XiphComment );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_XiphComment, changedflag );
    return guTagInfo::Write( changedflag );
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
    return SetXiphCommentCoverArt( m_XiphComment, image );
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
    return SetXiphCommentLyrics( m_XiphComment, lyrics );
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
        ReadExtendedTags( m_Mp4Tag );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_Mp4Tag, changedflag );
    return guTagInfo::Write( changedflag );
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
    return SetMp4Image( m_Mp4Tag, image );
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
    return SetMp4Lyrics( ( ( TagLib::MP4::File * ) m_TagFile->file() )->tag(), lyrics );
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
        ReadExtendedTags( m_ApeTag );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ApeTag, changedflag );
    return guTagInfo::Write( changedflag );
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
    //return m_ApeTag && SetApeImage( m_ApeTag, image ) && Write();
    return m_ApeTag && SetApeImage( m_ApeTag, image );
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
        ReadExtendedTags( m_ApeTag );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ApeTag, changedflag );
    return guTagInfo::Write( changedflag );
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
    return m_ApeTag && SetApeImage( m_ApeTag, image );
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
    return SetApeLyrics( m_ApeTag, lyrics );
}


#ifdef TAGLIB_WITH_APE_SUPPORT
// -------------------------------------------------------------------------------- //
// guApeTagInfo
// -------------------------------------------------------------------------------- //
guApeTagInfo::guApeTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    m_TagId3v1 = NULL;
    m_ApeTag = NULL;
    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_TagId3v1 = ( ( TagLib::APE::File * ) m_TagFile->file() )->ID3v1Tag();

        m_ApeTag = ( ( TagLib::APE::File * ) m_TagFile->file() )->APETag();
    }
}

// -------------------------------------------------------------------------------- //
guApeTagInfo::~guApeTagInfo()
{
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Read( void )
{
    if( guTagInfo::Read() )
    {
        ReadExtendedTags( m_ApeTag );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ApeTag, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::CanHandleLyrics( void )
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guApeTagInfo::GetLyrics( void )
{
    return GetApeLyrics( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetApeLyrics( m_ApeTag, lyrics );
}
#endif


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
        ReadExtendedTags( m_TagId3v2 );
        return true;
    }
    else
    {
      guLogError( wxT( "Could not read tags from file '%s'" ), m_FileName.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_TagId3v2, changedflag );
    return guTagInfo::Write( changedflag );
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

    return true;
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
        return true;
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
        ReadExtendedTags( m_ASFTag );
        return true;
    }
    else
    {
      guLogError( wxT( "Could not read tags from file '%s'" ), m_FileName.c_str() );
    }

    return false;
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ASFTag, changedflag );
    return guTagInfo::Write( changedflag );
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

    return true;
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
    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();

    const guCurrentTrack * CurrentTrack = MainFrame->GetCurrentTrack();
    if( CurrentTrack && CurrentTrack->m_Loaded )
    {
        if( CurrentTrack->m_FileName == filename )
        {
            // Add the pending track change to MainFrame
            MainFrame->AddPendingUpdateTrack( filename, picture, wxEmptyString, guTRACK_CHANGED_DATA_IMAGES );
            return true;
        }
    }

    bool RetVal = false;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleImages() )
        {
            RetVal = TagInfo->SetImage( picture ) && TagInfo->Write( guTRACK_CHANGED_DATA_IMAGES );
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
    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();

    const guCurrentTrack * CurrentTrack = MainFrame->GetCurrentTrack();
    if( CurrentTrack && CurrentTrack->m_Loaded )
    {
        if( CurrentTrack->m_FileName == filename )
        {
            // Add the pending track change to MainFrame
            MainFrame->AddPendingUpdateTrack( filename, NULL, lyrics, guTRACK_CHANGED_DATA_LYRICS );
            return true;
        }
    }

    bool RetVal = false;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleLyrics() )
        {
            RetVal = TagInfo->SetLyrics( lyrics ) && TagInfo->Write( guTRACK_CHANGED_DATA_LYRICS );
        }
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guUpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                    const wxArrayString &lyrics, const wxArrayInt &changedflags )
{
    int Index;
    int Count = tracks.Count();

    guMainFrame * MainFrame = guMainFrame::GetMainFrame();

    // Process each Track
    for( Index = 0; Index < Count; Index++ )
    {
        // If there is nothign to change continue with next one
        int ChangedFlag = changedflags[ Index ];
        if( !ChangedFlag )
            continue;

        const guTrack &Track = tracks[ Index ];

        // Dont allow to edit tags from Cue files tracks
        if( Track.m_Offset )
            continue;

        if( wxFileExists( Track.m_FileName ) )
        {
            // Prevent write to the current playing file in order to avoid segfaults specially with flac and wma files
            const guCurrentTrack * CurrentTrack = MainFrame->GetCurrentTrack();
            if( CurrentTrack && CurrentTrack->m_Loaded )
            {
                if( CurrentTrack->m_FileName == Track.m_FileName )
                {
                    // Add the pending track change to MainFrame
                    MainFrame->AddPendingUpdateTrack( Track,
                                                       Index < (int) images.Count() ? images[ Index ] : NULL,
                                                       Index < (int) lyrics.Count() ? lyrics[ Index ] : wxT( "" ),
                                                       changedflags[ Index ] );
                    continue;
                }
            }

            guTagInfo * TagInfo = guGetTagInfoHandler( Track.m_FileName );

            if( !TagInfo )
            {
                guLogError( wxT( "There is no handler for the file '%s'" ), Track.m_FileName.c_str() );
                return;
            }

            if( ChangedFlag & guTRACK_CHANGED_DATA_TAGS )
            {
                TagInfo->m_TrackName = Track.m_SongName;
                TagInfo->m_AlbumArtist = Track.m_AlbumArtist;
                TagInfo->m_ArtistName = Track.m_ArtistName;
                TagInfo->m_AlbumName = Track.m_AlbumName;
                TagInfo->m_GenreName = Track.m_GenreName;
                TagInfo->m_Track = Track.m_Number;
                TagInfo->m_Year = Track.m_Year;
                TagInfo->m_Composer = Track.m_Composer;
                TagInfo->m_Comments = Track.m_Comments;
                TagInfo->m_Disk = Track.m_Disk;
            }

            if( ChangedFlag & guTRACK_CHANGED_DATA_RATING )
            {
                TagInfo->m_Rating = Track.m_Rating;
                TagInfo->m_PlayCount = Track.m_PlayCount;
            }

            if( ( ChangedFlag & guTRACK_CHANGED_DATA_LYRICS ) && TagInfo->CanHandleLyrics() )
            {
                TagInfo->SetLyrics( lyrics[ Index ] );
            }

            if( ( ChangedFlag & guTRACK_CHANGED_DATA_IMAGES ) && TagInfo->CanHandleImages() )
            {
                TagInfo->SetImage( images[ Index ] );
            }

            TagInfo->Write( ChangedFlag );

            delete TagInfo;
        }
        else
        {
            guLogMessage( wxT( "File not found for edition: '%s'" ), Track.m_FileName.c_str() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guUpdateImages( const guTrackArray &songs, const guImagePtrArray &images, const wxArrayInt &changedflags )
{
    int Index;
    int Count = images.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( !songs[ Index ].m_Offset && ( changedflags[ Index ] & guTRACK_CHANGED_DATA_IMAGES ) )
            guTagSetPicture( songs[ Index ].m_FileName, images[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guUpdateLyrics( const guTrackArray &songs, const wxArrayString &lyrics, const wxArrayInt &changedflags )
{
    int Index;
    int Count = lyrics.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( !songs[ Index ].m_Offset && ( changedflags[ Index ] & guTRACK_CHANGED_DATA_LYRICS ) )
            guTagSetLyrics( songs[ Index ].m_FileName, lyrics[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
