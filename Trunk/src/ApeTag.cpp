// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "ApeTag.h"

#include <wx/arrimpl.cpp>
#include <wx/defs.h>
#include <wx/string.h>

#define APE_HEADER_MAGIC                0x2043414D  //"MAC "
#define APE_MAGIC_0                     0x54455041  //"APET"
#define APE_MAGIC_1                     0x58454741  //"AGEX"
#define APE_VERSION_1                   1000
#define APE_VERSION_2                   2000

#define COMPRESSION_LEVEL_EXTRA_HIGH    4000

// -------------------------------------------------------------------------------- //
struct APE_COMMON_HEADER
{
    wxUint32 ID;                              // should equal 'MAC '
    wxUint16 nVersion;                        // version number * 1000 (3.81 = 3810)
};

// -------------------------------------------------------------------------------- //
struct APE_DESCRIPTOR
{
    wxUint32 ID;                                 // should equal 'MAC '
    wxUint16 nVersion;                           // version number * 1000 (3.81 = 3810) (remember that 4-byte alignment causes this to take 4-bytes)

    wxUint32 nDescriptorBytes;                   // the number of descriptor bytes (allows later expansion of this header)
    wxUint32 nHeaderBytes;                       // the number of header APE_HEADER bytes
    wxUint32 nSeekTableBytes;                    // the number of bytes of the seek table
    wxUint32 nHeaderDataBytes;                   // the number of header data bytes (from original file)
    wxUint32 nAPEFrameDataBytes;                 // the number of bytes of APE frame data
    wxUint32 nAPEFrameDataBytesHigh;             // the high order number of APE frame data bytes
    wxUint32 nTerminatingDataBytes;              // the terminating data of the file (not including tag data)

    wxUint8  cFileMD5[16];                       // the MD5 hash of the file (see notes for usage... it's a littly tricky)
};

// -------------------------------------------------------------------------------- //
struct APE_HEADER
{
    wxUint16 nCompressionLevel;                 // the compression level (see defines I.E. COMPRESSION_LEVEL_FAST)
    wxUint16 nFormatFlags;                      // any format flags (for future use)

    wxUint32 nBlocksPerFrame;                   // the number of audio blocks in one frame
    wxUint32 nFinalFrameBlocks;                 // the number of audio blocks in the final frame
    wxUint32 nTotalFrames;                      // the total number of frames

    wxUint16 nBitsPerSample;                    // the bits per sample (typically 16)
    wxUint16 nChannels;                         // the number of channels (1 or 2)
    wxUint32 nSampleRate;                       // the sample rate (typically 44100)
};

// -------------------------------------------------------------------------------- //
struct APE_HEADER_OLD
{
    wxUint32 ID;                            // should equal 'MAC '
    wxUint16 nVersion;                        // version number * 1000 (3.81 = 3810)
    wxUint16 nCompressionLevel;               // the compression level
    wxUint16 nFormatFlags;                    // any format flags (for future use)
    wxUint16 nChannels;                       // the number of channels (1 or 2)
    wxUint32 nSampleRate;                     // the sample rate (typically 44100)
    wxUint32 nHeaderBytes;                    // the bytes after the MAC header that compose the WAV header
    wxUint32 nTerminatingBytes;               // the bytes after that raw data (for extended info)
    wxUint32 nTotalFrames;                    // the number of frames in the file
    wxUint32 nFinalFrameBlocks;               // the number of samples in the final frame
};


// -------------------------------------------------------------------------------- //
typedef struct {
    wxUint32    m_Magic[ 2 ];
    wxUint32    m_Version;
    wxUint32    m_Length;
    wxUint32    m_Items;
    wxUint32    m_Flags;
    wxUint32    m_Reserved[ 2 ];
} guAPE_HEADER_FOOTER;


// -------------------------------------------------------------------------------- //
static wxUint32 ReadLittleEndianUint32( const char * cp )
{
    wxUint32 result = cp[ 3 ] & 0xff;
    result <<= 8;
    result |= cp[ 2 ] & 0xff;
    result <<= 8;
    result |= cp[ 1 ] & 0xff;
    result <<= 8;
    result |= cp[ 0 ] & 0xff;
    return result;
}

// -------------------------------------------------------------------------------- //
static void WriteLittleEndianUint32( char * cp, wxUint32 i )
{
    cp[ 0 ] = i & 0xff;
    i >>= 8;
    cp[ 1 ] = i & 0xff;
    i >>= 8;
    cp[ 2 ] = i & 0xff;
    i >>= 8;
    cp[ 3 ] = i & 0xff;
}

// -------------------------------------------------------------------------------- //
int guCompareApeItems( guApeItem * item1, guApeItem * item2 )
{
    return item1->Key() > item2->Key();
}




// -------------------------------------------------------------------------------- //
// guApeTag
// -------------------------------------------------------------------------------- //
guApeTag::guApeTag( wxUint32 length, wxUint32 offset, wxUint32 items )
{
    m_FileLength = length;
    m_TagOffset = offset;
    m_ItemCount = items;
    m_Items = new guApeItemArray( guCompareApeItems );
}

// -------------------------------------------------------------------------------- //
guApeTag::~guApeTag()
{
    if( m_Items )
        delete m_Items;
}

// -------------------------------------------------------------------------------- //
void guApeTag::DelAllItems()
{
    m_Items->Clear();
}

// -------------------------------------------------------------------------------- //
void guApeTag::DelItem( guApeItem * item )
{
    int Pos = m_Items->Index( item );
    if( Pos != wxNOT_FOUND )
    {
        m_Items->Remove( item );
        delete item;
    }
    else
    {
        guLogError( wxT( "Could not find the item in the ape tags" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guApeTag::AddItem( guApeItem * item )
{
    m_Items->Add( item );
}

// -------------------------------------------------------------------------------- //
guApeItem * guApeTag::GetItem( const int pos ) const
{
    return m_Items->Item( pos );
}

// -------------------------------------------------------------------------------- //
guApeItem * guApeTag::GetItem( const wxString &key ) const
{
    guApeItem * ApeItem;
    int index;
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        ApeItem = m_Items->Item( index );
        if( !ApeItem->m_Key.CmpNoCase( key ) )
            return ApeItem;
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
wxString guApeTag::GetItemValue( const wxString &key ) const
{
    wxString RetVal = wxEmptyString;
    guApeItem * ApeItem = GetItem( key );
    if( ApeItem )
    {
        RetVal = ApeItem->m_Value;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetItem( const wxString &key, const wxString &value, wxUint32 flags )
{
    guApeItem * ApeItem = GetItem( key );
    if( ApeItem )
    {
        ApeItem->m_Value = value;
    }
    else
    {
        ApeItem = new guApeItem( key, value, flags );
        m_Items->Add( ApeItem );
    }
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetItem( const wxString &key, char * data, wxUint32 len )
{
    guApeItem * ApeItem = GetItem( key );
    if( ApeItem )
    {
        ApeItem->m_Value = wxString::From8BitData( data, len );
    }
    else
    {
        ApeItem = new guApeItem( key, wxString::From8BitData( data, len ), APE_FLAG_CONTENT_BINARY );
        m_Items->Add( ApeItem );
    }
}

// -------------------------------------------------------------------------------- //
wxUint32 guApeTag::FileLength( void ) const
{
    return m_FileLength;
}

// -------------------------------------------------------------------------------- //
wxUint32 guApeTag::TagOffset( void ) const
{
    return m_TagOffset;
}

// -------------------------------------------------------------------------------- //
wxUint32 guApeTag::ItemLength( void ) const
{
    wxUint32 RetVal = 0;
    int index;
    int count = m_Items->Count();
    for( index = 0; index < count; index++ )
    {
        guApeItem * item = m_Items->Item( index );

        if( !item->Value().IsEmpty() )
        {
            RetVal += 8;
            RetVal += 1;
            RetVal += item->Key().Length();
            RetVal += strlen( item->Value().mb_str() );
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxUint32 guApeTag::ItemCount( void ) const
{
    wxUint32 RetVal = 0;
    int Index;
    int Count = m_Items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guApeItem * ApeItem = m_Items->Item( Index );

        //guLogMessage( wxT( "'%s' => '%s'" ), ApeItem->m_Key.c_str(), ApeItem->m_Value.c_str() );
        if( !ApeItem->m_Value.IsEmpty() )
        {
            const wxWX2MBbuf ValueBuf = ApeItem->m_Value.mb_str( wxConvUTF8 );
            if( ValueBuf )
            {
                RetVal++;
            }
        }
    }
    guLogMessage( wxT( "ItemCount() -> %u" ), RetVal );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guApeTag::GetTitle( void ) const
{
    return GetItemValue( APE_TAG_KEY_TITLE );
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetTitle( const wxString &title )
{
    SetItem( APE_TAG_KEY_TITLE, title );
}

// -------------------------------------------------------------------------------- //
wxString guApeTag::GetArtist( void ) const
{
    return GetItemValue( APE_TAG_KEY_ARTIST );
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetArtist( const wxString &artist )
{
    SetItem( APE_TAG_KEY_ARTIST, artist );
}

// -------------------------------------------------------------------------------- //
wxString guApeTag::GetAlbum( void ) const
{
    return GetItemValue( APE_TAG_KEY_ALBUM );
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetAlbum( const wxString &album )
{
    SetItem( APE_TAG_KEY_ALBUM, album );
}

// -------------------------------------------------------------------------------- //
wxString guApeTag::GetGenre( void ) const
{
    return GetItemValue( APE_TAG_KEY_GENRE );
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetGenre( const wxString &genre )
{
    SetItem( APE_TAG_KEY_GENRE, genre );
}

// -------------------------------------------------------------------------------- //
wxUint32 guApeTag::GetTrack( void ) const
{
    unsigned long Track;
    GetItemValue( APE_TAG_KEY_TRACK ).ToULong( &Track );
    return Track;
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetTrack( const wxUint32 track )
{
    SetItem( APE_TAG_KEY_TRACK, wxString::Format( wxT( "%u" ), track ) );
}

// -------------------------------------------------------------------------------- //
wxUint32 guApeTag::GetYear( void ) const
{
    unsigned long Year;
    GetItemValue( APE_TAG_KEY_YEAR ).ToULong( &Year );
    return Year;
}

// -------------------------------------------------------------------------------- //
void guApeTag::SetYear( const wxUint32 year )
{
    SetItem( APE_TAG_KEY_YEAR, wxString::Format( wxT( "%u" ), year ) );
}




// -------------------------------------------------------------------------------- //
// guApeFile
// -------------------------------------------------------------------------------- //
guApeFile::guApeFile( const wxString &filename )
{
    m_Tag = NULL;
    m_TrackLength = 0;
    m_BitRate = 0;
    m_File = new wxFile( filename, wxFile::read_write );
    if( m_File->IsOpened() )
    {
        ReadAndProcessApeHeader();
    }
    else
    {
        guLogWarning( wxT( "Could not open the ape file %s" ), filename.c_str() );
    }
}

// -------------------------------------------------------------------------------- //
guApeFile::~guApeFile()
{
    if( m_File )
        delete m_File;
    if( m_Tag )
        delete m_Tag;
}

// -------------------------------------------------------------------------------- //
void inline guApeFile::WriteInt( const int value )
{
    m_File->Write( &value, sizeof( value ) );
}

// -------------------------------------------------------------------------------- //
void guApeFile::WriteApeHeaderFooter( const wxUint32 flags )
{
    //guLogMessage( wxT( "Writing header/footer at %08X" ), m_File->Tell() );

    WriteInt( APE_MAGIC_0 );
    WriteInt( APE_MAGIC_1 );

    WriteInt( APE_VERSION_2 );

    WriteInt( m_Tag->ItemLength() + sizeof( guAPE_HEADER_FOOTER ) );

    WriteInt( m_Tag->ItemCount() );

    WriteInt( flags );

    WriteInt( 0 );
    WriteInt( 0 );
}


// -------------------------------------------------------------------------------- //
void guApeFile::WriteApeItems( void )
{
    int index;
    int count = m_Tag->m_Items->Count();
    char pad = 0;
    for( index = 0; index < count; index++ )
    {
        guApeItem * ApeItem = m_Tag->GetItem( index );

        //guLogMessage( wxT( "Writing item %i   '%s' => '%s' at %08X" ), index, ApeItem->m_Key.c_str(), ApeItem->m_Value.c_str(), m_File->Tell() );

        if( ApeItem->m_Value.IsEmpty() )
            continue;

        const wxWX2MBbuf ValueBuf = ApeItem->m_Value.mb_str( wxConvUTF8 );
        if( !ValueBuf )
            continue;

        //guLogMessage( wxT( "'%s' => '%s'" ), ApeItem->m_Key.c_str(), ApeItem->m_Value.c_str() );
        //int KeyLen = ApeItem->m_Key.size();


        int ValueLen = strlen( ValueBuf );
        WriteInt( ValueLen );
        WriteInt( ApeItem->m_Flags );
        m_File->Write( ApeItem->m_Key.ToUTF8(), ApeItem->m_Key.size() );
        m_File->Write( &pad, sizeof( pad ) );

        if( ( ApeItem->m_Flags & APE_FLAG_CONTENT_TYPE ) == APE_FLAG_CONTENT_BINARY )
        {
            m_File->Write( ApeItem->m_Value.To8BitData(), ApeItem->m_Value.Length() );
        }
        else
        {
            m_File->Write( ValueBuf, ValueLen );

            //guLogMessage( wxT( "'%s' Size: %u   Length: %u" ),
            //    ApeItem->m_Value.c_str(), ApeItem->m_Value.size(), ApeItem->m_Value.Length() );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guApeFile::WriteApeTag( void )
{
    guLogMessage( wxT( "file length %u, %08x" ), m_Tag->FileLength(), m_Tag->TagOffset() );

    const wxUint32 TagOffset = !m_Tag->TagOffset() ?  m_Tag->FileLength() : m_Tag->TagOffset();

    m_File->Seek( TagOffset );

    // write header
    if( m_File->Tell() != TagOffset )
    {
        guLogWarning( wxT( "Seek for header failed %u target pos %u" ), m_File->Tell(), TagOffset );
    }

    WriteApeHeaderFooter( APE_FLAG_IS_HEADER | APE_FLAG_HAVE_HEADER );
    WriteApeItems();
    WriteApeHeaderFooter( APE_FLAG_HAVE_HEADER );

    wxUint32 CurPos = m_File->Tell();

    if( CurPos < m_Tag->FileLength() )
    {
        int result = ftruncate( m_File->fd(), CurPos );
        if( result )
        {
            guLogWarning( wxT( "FAILED Truncating file %s" ), m_FileName.c_str() );
        }
    }
    m_File->Flush();
    return true;
}


// -------------------------------------------------------------------------------- //
void guApeFile::ReadAndProcessApeHeader( void )
{
    const wxUint32 FileLength = m_File->Length();
    //guLogMessage( wxT( "file length is %u" ), FileLength );

    APE_COMMON_HEADER CHeader;
    m_File->Seek( 0 );
    m_File->Read( &CHeader, sizeof( APE_COMMON_HEADER ) );
    if( CHeader.ID != APE_HEADER_MAGIC )
    {
        guLogWarning( wxT( "This is not a valid ape file %08x" ), CHeader.ID );
        return;
    }

//    printf( "Version          : %u\n", CHeader.nVersion );
    // New header format
    if( CHeader.nVersion >= 3980 )
    {
        APE_DESCRIPTOR  Descriptor;
        APE_HEADER      Header;
        m_File->Seek( 0 );

        int ReadCnt = m_File->Read( &Descriptor, sizeof( APE_DESCRIPTOR ) );
        if( ( ReadCnt - Descriptor.nDescriptorBytes ) > 0 )
            m_File->Seek( Descriptor.nDescriptorBytes - ReadCnt, wxFromCurrent );

        ReadCnt = m_File->Read( &Header, sizeof( APE_HEADER ) );
        if( ( ReadCnt - Descriptor.nHeaderBytes ) > 0 )
            m_File->Seek( Descriptor.nHeaderBytes - ReadCnt, wxFromCurrent );

//        printf( "TotalFrames      : %u\n", Header.nTotalFrames );
//        printf( "BlocksPerFrame   : %u\n", Header.nBlocksPerFrame );
//        printf( "FinalFrameBlocks : %u\n", Header.nFinalFrameBlocks );
//        printf( "SampleRate       : %u\n", Header.nSampleRate );

        m_TrackLength = int( double( ( ( Header.nTotalFrames - 1 ) * Header.nBlocksPerFrame ) + Header.nFinalFrameBlocks )
                             / double( Header.nSampleRate ) );

    }
    else    // Old header format
    {
        APE_HEADER_OLD Header;
        m_File->Seek( 0 );

        m_File->Read( &Header, sizeof( APE_HEADER_OLD ) );

        wxUint32 BlocksPerFrame = ( ( Header.nVersion >= 3900 ) || ( ( Header.nVersion >= 3800 ) && ( Header.nCompressionLevel == COMPRESSION_LEVEL_EXTRA_HIGH ) ) ) ? 73728 : 9216;
        if( ( Header.nVersion >= 3950 ) )
            BlocksPerFrame = 73728 * 4;

//        printf( "TotalFrames      : %u\n", Header.nTotalFrames );
//        printf( "BlocksPerFrame   : %u\n", BlocksPerFrame );
//        printf( "FinalFrameBlocks : %u\n", Header.nFinalFrameBlocks );
//        printf( "SampleRate       : %u\n", Header.nSampleRate );

        m_TrackLength = int( double( ( ( Header.nTotalFrames - 1 ) * BlocksPerFrame ) + Header.nFinalFrameBlocks )
                             / double( Header.nSampleRate ) );

    }

    m_BitRate = m_TrackLength ? int( ( double( FileLength ) * double( 8 ) ) / ( double( m_TrackLength ) * double( 1000 ) ) ) : 0;
//    guLogMessage( wxT( "Track size %u  length %s  bitrate %u" ), FileLength, LenToString( m_TrackLength ).c_str(), m_BitRate );

    if( FileLength < sizeof( guAPE_HEADER_FOOTER ) )
    {
        guLogError( wxT( "file too short to contain an ape tag" ) );
        return; // new TAG(FileLength,0,0);
    }

    // read footer
    guAPE_HEADER_FOOTER ApeFooter;
    m_File->Seek( FileLength - sizeof( guAPE_HEADER_FOOTER ) );
    m_File->Read( &ApeFooter, sizeof( guAPE_HEADER_FOOTER ) );

    if( ApeFooter.m_Magic[ 0 ] != APE_MAGIC_0 || ApeFooter.m_Magic[ 1 ] != APE_MAGIC_1 )
    {
        guLogWarning( wxT( "file does not contain ApeFooter tag" ) );

        m_Tag = new guApeTag( FileLength, FileLength, 0 );
        return;
    }

    if( ApeFooter.m_Version != APE_VERSION_2 )
    {
        guLogWarning( wxT( "Unsupported ApeFooter tag version %i" ), ApeFooter.m_Version );
        return;
    }

    //guLogMessage( wxT( "Found ApeFooter tag footer version: %i  length: %i  items: %i  flags: %08x" ),
    //              ApeFooter.m_Version, ApeFooter.m_Length, ApeFooter.m_Items, ApeFooter.m_Flags );


    if( FileLength < ApeFooter.m_Length )
    {
        guLogWarning( wxT( "ApeTag bigger than file" ) );
        return;
    }

    // read header if any
    bool have_header = false;

    if( FileLength >= ApeFooter.m_Length + sizeof( guAPE_HEADER_FOOTER ) )
    {
        m_File->Seek( FileLength - ( ApeFooter.m_Length + sizeof( guAPE_HEADER_FOOTER ) ) );
        guAPE_HEADER_FOOTER ApeHeader;

        m_File->Read( ( void * ) &ApeHeader, sizeof( guAPE_HEADER_FOOTER ) );

        if( ApeHeader.m_Magic[ 0 ] == APE_MAGIC_0 &&
            ApeHeader.m_Magic[ 1 ] == APE_MAGIC_1 )
        {
            have_header = true;

            if( ApeFooter.m_Version != ApeHeader.m_Version || ApeFooter.m_Length != ApeHeader.m_Length || ApeFooter.m_Items != ApeHeader.m_Items )
            {
                guLogWarning( wxT( "ApeFooter header/footer data mismatch" ) );
            }

            //guLogMessage( wxT( "Found ApeFooter tag header version: %i  length: %i  items: %i  flags: %08x" ),
            //      ApeHeader.m_Version, ApeHeader.m_Length, ApeHeader.m_Items, ApeHeader.m_Flags );
        }
    }

    m_Tag = new guApeTag( FileLength,
                          FileLength - ApeFooter.m_Length - have_header * sizeof( guAPE_HEADER_FOOTER ),
                          ApeFooter.m_Items );

    // read and process tag data
    m_File->Seek( - ( int ) ApeFooter.m_Length, wxFromEnd );

    char * const ItemsBuf = new char[ ApeFooter.m_Length ];

    m_File->Read( ( void * ) ItemsBuf, ApeFooter.m_Length );

    char * CurBufPos = ItemsBuf;

    //guLogMessage( wxT( "Found a valid ape footer with %i items and %i bytes length" ), ApeFooter.m_Items, ApeFooter.m_Length );
    wxString Value;
    wxString Key;
    int index;
    for( index = 0; index < ( int ) ApeFooter.m_Items; index++ )
    {
        const wxUint32 ValueLen = ReadLittleEndianUint32( CurBufPos );
        CurBufPos += sizeof( ValueLen );
        if( ValueLen > ( ApeFooter.m_Length - ( ItemsBuf - CurBufPos ) ) )
        {
            guLogWarning( wxT( "Aborting reading of corrupt ape tag %i > %i" ), ValueLen, ( ApeFooter.m_Length - ( ItemsBuf - CurBufPos ) ) );
            m_Tag->DelAllItems();
            break;
        }

        const wxUint32 ItemFlags = ReadLittleEndianUint32( CurBufPos );
        CurBufPos += sizeof( ItemFlags );

        Key = wxString( CurBufPos, wxConvUTF8 );

        CurBufPos += 1 + Key.Length();

        if( ( ItemFlags & APE_FLAG_CONTENT_TYPE ) == APE_FLAG_CONTENT_BINARY )
        {
            Value = wxString::From8BitData( CurBufPos, ValueLen );
        }
        else
        {
            Value = wxString::FromUTF8( CurBufPos, ValueLen );
        }

        CurBufPos += ValueLen;

        //guLogMessage( wxT( "Tag%i => Len: %i  Flags: %08x  Key: '%s'  Value: '%s'" ),
        //                                         index, ValueLen, ItemFlags, Key.c_str(), Value.c_str() );
        m_Tag->AddItem( new guApeItem( Key, Value, ItemFlags ) );
    }

    delete ItemsBuf;
}

// -------------------------------------------------------------------------------- //
