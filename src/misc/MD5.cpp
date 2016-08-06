// -------------------------------------------------------------------------------- //
//
// Name:        md5.cpp
// Author:      Kai Krahn
// Created:     11.11.07 12:35

// Description: wxWidgets md5 implementation
//
// This code implements the MD5 message-digest algorithm.
// The algorithm is due to Ron Rivest. This code is based on the code
// written by Colin Plumb in 1993, no copyright is claimed.
// This code is in the public domain; do with it what you wish.
//
// This version implements the MD5 algorithm for the free GUI toolkit wxWidgets.
// Basic functionality, like MD5 hash creation out of strings and files, or
// to verify strings and files against a given MD5 hash, was added
// by Kai Krahn.
//
// This code is provided "as is" and comes without any warranty!
// -------------------------------------------------------------------------------- //

#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/filename.h>

#include <memory.h>		 /* for memcpy() */
#include "MD5.h"

#ifndef HIGHFIRST
#ifdef wxBYTE_ORDER
    #if wxBYTE_ORDER == wxBIG_ENDIAN
        #define HIGHFIRST
    #endif
#endif
#endif

//---------------------------------------------------------------------------
#ifndef HIGHFIRST
  #define byteReverse( buf, len )	/* Nothing */
#else
// -------------------------------------------------------------------------------- //
void byteReverse( unsigned char * buf, unsigned longs )
{
    uint32 t;
    do {
        t = ( uint32 ) ( ( unsigned ) buf[ 3 ] << 8 | buf[ 2 ] ) << 16 |
                       ( ( unsigned ) buf[ 1 ] << 8 | buf[ 0 ] );
	    *( uint32 * ) buf = t;
	    buf += 4;
    } while ( --longs );
}
#endif

// -------------------------------------------------------------------------------- //
guMD5CTX::guMD5CTX()
{
    Init();
}

// -------------------------------------------------------------------------------- //
void guMD5CTX::Init()
{
    m_State[ 0 ] = 0x67452301;
    m_State[ 1 ] = 0xefcdab89;
    m_State[ 2 ] = 0x98badcfe;
    m_State[ 3 ] = 0x10325476;

    m_Count[ 0 ] = 0;
    m_Count[ 1 ] = 0;
}


// -------------------------------------------------------------------------------- //
void guMD5CTX::Update( const unsigned char * buf, unsigned len )
{
    uint32 t;

    /* Update bitcount */
    t = m_Count[ 0 ];
    if( ( m_Count[ 0 ] = t + ( ( uint32 ) len << 3 ) ) < t )
        m_Count[ 1 ]++; 	/* Carry from low to high */
    m_Count[ 1 ] += len >> 29;

    t = ( t >> 3 ) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */
    if( t )
    {
        unsigned char * p = ( unsigned char * ) m_Buffer + t;

        t = 64 - t;
        if( len < t )
        {
            memcpy( p, buf, len );
            return;
        }
        memcpy( p, buf, t );
        byteReverse( m_Buffer, 16 );
        Transform( ( uint32 * ) m_Buffer );
        buf += t;
        len -= t;
    }
    /* Process data in 64-byte chunks */

    while( len >= 64 )
    {
        memcpy( m_Buffer, buf, 64 );
        byteReverse( m_Buffer, 16 );
        Transform( ( uint32 * ) m_Buffer );
        buf += 64;
        len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy( m_Buffer, buf, len );
}


// -------------------------------------------------------------------------------- //
void guMD5CTX::Final( unsigned char digest[ 16 ] )
{
    unsigned Cnt;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    Cnt = ( m_Count[ 0 ] >> 3 ) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = m_Buffer + Cnt;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    Cnt = 64 - 1 - Cnt;

    /* Pad out to 56 mod 64 */
    if( Cnt < 8 )
    {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset( p, 0, Cnt );
        byteReverse( m_Buffer, 16 );
        Transform( ( uint32 * ) m_Buffer );

        /* Now fill the next block with 56 bytes */
        memset( m_Buffer, 0, 56 );
    }
    else
    {
        /* Pad block to 56 bytes */
        memset( p, 0, Cnt - 8 );
    }
    byteReverse( m_Buffer, 14 );

    /* Append length in bits and transform */
    ( ( uint32 * ) m_Buffer )[ 14 ] = m_Count[ 0 ];
    ( ( uint32 * ) m_Buffer )[ 15 ] = m_Count[ 1 ];

    Transform( ( uint32 * ) m_Buffer );
    byteReverse( ( unsigned char * ) m_State, 4 );
    memcpy( digest, m_State, 16 );
    //memset( ctx, 0, sizeof( ctx ) );        /* In case it's sensitive */
}


// -------------------------------------------------------------------------------- //
/** The four core functions - F1 is optimized somewhat */

/* #define F1( x, y, z ) ( x & y | ~x & z ) */
#define F1( x, y, z ) ( z ^ ( x & ( y ^ z ) ) )
#define F2( x, y, z ) F1( z, x, y )
#define F3( x, y, z ) ( x ^ y ^ z )
#define F4( x, y, z ) ( y ^ ( x | ~z ) )

/** This is the central step in the MD5 algorithm. */
#define MD5STEP( f, w, x, y, z, data, s ) \
	( w += f( x, y, z ) + data,  w = w<<s | w>>( 32-s ),  w += x )


// -------------------------------------------------------------------------------- //
void guMD5CTX::Transform( uint32 in[ 16 ] )
{
    register uint32 a, b, c, d;

    a = m_State[ 0 ];
    b = m_State[ 1 ];
    c = m_State[ 2 ];
    d = m_State[ 3 ];

    MD5STEP( F1, a, b, c, d, in[  0 ] + 0xd76aa478,  7 );
    MD5STEP( F1, d, a, b, c, in[  1 ] + 0xe8c7b756, 12 );
    MD5STEP( F1, c, d, a, b, in[  2 ] + 0x242070db, 17 );
    MD5STEP( F1, b, c, d, a, in[  3 ] + 0xc1bdceee, 22 );
    MD5STEP( F1, a, b, c, d, in[  4 ] + 0xf57c0faf,  7 );
    MD5STEP( F1, d, a, b, c, in[  5 ] + 0x4787c62a, 12 );
    MD5STEP( F1, c, d, a, b, in[  6 ] + 0xa8304613, 17 );
    MD5STEP( F1, b, c, d, a, in[  7 ] + 0xfd469501, 22 );
    MD5STEP( F1, a, b, c, d, in[  8 ] + 0x698098d8,  7 );
    MD5STEP( F1, d, a, b, c, in[  9 ] + 0x8b44f7af, 12 );
    MD5STEP( F1, c, d, a, b, in[ 10 ] + 0xffff5bb1, 17 );
    MD5STEP( F1, b, c, d, a, in[ 11 ] + 0x895cd7be, 22 );
    MD5STEP( F1, a, b, c, d, in[ 12 ] + 0x6b901122,  7 );
    MD5STEP( F1, d, a, b, c, in[ 13 ] + 0xfd987193, 12 );
    MD5STEP( F1, c, d, a, b, in[ 14 ] + 0xa679438e, 17 );
    MD5STEP( F1, b, c, d, a, in[ 15 ] + 0x49b40821, 22 );

    MD5STEP( F2, a, b, c, d, in[  1 ] + 0xf61e2562,  5 );
    MD5STEP( F2, d, a, b, c, in[  6 ] + 0xc040b340,  9 );
    MD5STEP( F2, c, d, a, b, in[ 11 ] + 0x265e5a51, 14 );
    MD5STEP( F2, b, c, d, a, in[  0 ] + 0xe9b6c7aa, 20 );
    MD5STEP( F2, a, b, c, d, in[  5 ] + 0xd62f105d,  5 );
    MD5STEP( F2, d, a, b, c, in[ 10 ] + 0x02441453,  9 );
    MD5STEP( F2, c, d, a, b, in[ 15 ] + 0xd8a1e681, 14 );
    MD5STEP( F2, b, c, d, a, in[  4 ] + 0xe7d3fbc8, 20 );
    MD5STEP( F2, a, b, c, d, in[  9 ] + 0x21e1cde6,  5 );
    MD5STEP( F2, d, a, b, c, in[ 14 ] + 0xc33707d6,  9 );
    MD5STEP( F2, c, d, a, b, in[  3 ] + 0xf4d50d87, 14 );
    MD5STEP( F2, b, c, d, a, in[  8 ] + 0x455a14ed, 20 );
    MD5STEP( F2, a, b, c, d, in[ 13 ] + 0xa9e3e905,  5 );
    MD5STEP( F2, d, a, b, c, in[  2 ] + 0xfcefa3f8,  9 );
    MD5STEP( F2, c, d, a, b, in[  7 ] + 0x676f02d9, 14 );
    MD5STEP( F2, b, c, d, a, in[ 12 ] + 0x8d2a4c8a, 20 );

    MD5STEP( F3, a, b, c, d, in[  5 ] + 0xfffa3942,  4 );
    MD5STEP( F3, d, a, b, c, in[  8 ] + 0x8771f681, 11 );
    MD5STEP( F3, c, d, a, b, in[ 11 ] + 0x6d9d6122, 16 );
    MD5STEP( F3, b, c, d, a, in[ 14 ] + 0xfde5380c, 23 );
    MD5STEP( F3, a, b, c, d, in[  1 ] + 0xa4beea44,  4 );
    MD5STEP( F3, d, a, b, c, in[  4 ] + 0x4bdecfa9, 11 );
    MD5STEP( F3, c, d, a, b, in[  7 ] + 0xf6bb4b60, 16 );
    MD5STEP( F3, b, c, d, a, in[ 10 ] + 0xbebfbc70, 23 );
    MD5STEP( F3, a, b, c, d, in[ 13 ] + 0x289b7ec6,  4 );
    MD5STEP( F3, d, a, b, c, in[  0 ] + 0xeaa127fa, 11 );
    MD5STEP( F3, c, d, a, b, in[  3 ] + 0xd4ef3085, 16 );
    MD5STEP( F3, b, c, d, a, in[  6 ] + 0x04881d05, 23 );
    MD5STEP( F3, a, b, c, d, in[  9 ] + 0xd9d4d039,  4 );
    MD5STEP( F3, d, a, b, c, in[ 12 ] + 0xe6db99e5, 11 );
    MD5STEP( F3, c, d, a, b, in[ 15 ] + 0x1fa27cf8, 16 );
    MD5STEP( F3, b, c, d, a, in[  2 ] + 0xc4ac5665, 23 );

    MD5STEP( F4, a, b, c, d, in[  0 ] + 0xf4292244,  6 );
    MD5STEP( F4, d, a, b, c, in[  7 ] + 0x432aff97, 10 );
    MD5STEP( F4, c, d, a, b, in[ 14 ] + 0xab9423a7, 15 );
    MD5STEP( F4, b, c, d, a, in[  5 ] + 0xfc93a039, 21 );
    MD5STEP( F4, a, b, c, d, in[ 12 ] + 0x655b59c3,  6 );
    MD5STEP( F4, d, a, b, c, in[  3 ] + 0x8f0ccc92, 10 );
    MD5STEP( F4, c, d, a, b, in[ 10 ] + 0xffeff47d, 15 );
    MD5STEP( F4, b, c, d, a, in[  1 ] + 0x85845dd1, 21 );
    MD5STEP( F4, a, b, c, d, in[  8 ] + 0x6fa87e4f,  6 );
    MD5STEP( F4, d, a, b, c, in[ 15 ] + 0xfe2ce6e0, 10 );
    MD5STEP( F4, c, d, a, b, in[  6 ] + 0xa3014314, 15 );
    MD5STEP( F4, b, c, d, a, in[ 13 ] + 0x4e0811a1, 21 );
    MD5STEP( F4, a, b, c, d, in[  4 ] + 0xf7537e82,  6 );
    MD5STEP( F4, d, a, b, c, in[ 11 ] + 0xbd3af235, 10 );
    MD5STEP( F4, c, d, a, b, in[  2 ] + 0x2ad7d2bb, 15 );
    MD5STEP( F4, b, c, d, a, in[  9 ] + 0xeb86d391, 21 );

    m_State[ 0 ] += a;
    m_State[ 1 ] += b;
    m_State[ 2 ] += c;
    m_State[ 3 ] += d;
}

// -------------------------------------------------------------------------------- //
wxString guMD5::MD5( const unsigned char * Data, unsigned int Len )
{
    unsigned char MD5Hash[ 16 ];

    m_Context.Init();
    m_Context.Update( Data, Len );
    m_Context.Final( MD5Hash );

    wxString RetVal = wxEmptyString;
    for( int i = 0; i < 16; i++ )
         RetVal += wxString::Format( wxT( "%02x" ), MD5Hash[ i ] );

    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guMD5::MD5File( const wxString &FileName )
{
    unsigned char MD5Hash[ 16 ];
#define ChunkSize 1024

    m_Context.Init();

    wxString RetVal = wxEmptyString;

    if( wxFileExists( FileName ) )
    {
        wxFileInputStream * InStream = new wxFileInputStream( FileName );
        size_t FileSize = InStream->GetSize();

        unsigned char * m_Buffer = new unsigned char[ ChunkSize ];
        size_t BytesRead = 0;
        while( BytesRead < FileSize )
        {
            size_t BytesLeft = FileSize - BytesRead;
            size_t Cnt = wxMin( ChunkSize, BytesLeft );

            InStream->Read( m_Buffer, Cnt );
            BytesRead += Cnt;

            m_Context.Update( m_Buffer, Cnt );
        }
        delete [] m_Buffer;

        delete InStream;
    }

    m_Context.Final( MD5Hash );

    for( int i = 0; i < 16; i++ )
        RetVal += wxString::Format( wxT( "%02x" ), MD5Hash[ i ] );

    return RetVal;
}

// -------------------------------------------------------------------------------- //
//#define MD5TESTING
#ifdef MD5TESTING
int main( void )
{
    guMD5 MD5;

    printf( MD5.MD5( wxT( "The quick brown fox jumps over the lazy dog" ) ).char_str(), true ); printf( "\n" );
    printf( "9e107d9d372bb6826bd81d3542a419d6 <<\n" );
    printf( MD5.MD5( wxT( "The quick brown fox jumps over the lazy eog" ) ).char_str(), true ); printf( "\n" );
    printf( "ffd93f16876049265fbaef4da268dd0e <<\n" );
}

#endif

// -------------------------------------------------------------------------------- //
