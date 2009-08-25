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
#include "Base64.h"

#include "Utils.h"

// -------------------------------------------------------------------------------- //
static const wxString guBase64_Chars = wxT( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );

// -------------------------------------------------------------------------------- //
#define IsBase64Char( c ) ( ( c >= 'A' && c <= 'Z' ) ||\
                            ( c >= 'a' && c <= 'z' ) ||\
                            ( c >= '0' && c <= '9' ) ||\
                            ( c == '+' ) || ( c == '/' ) )

// -------------------------------------------------------------------------------- //
wxString guBase64Encode( const char * src, unsigned int len )
{
    wxString RetVal = wxEmptyString;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[ 3 ];
    unsigned char char_array_4[ 4 ];

    while( len-- )
    {
        char_array_3[ i++ ] = * ( src++ );
        if( i == 3 )
        {
            char_array_4[ 0 ] =   ( char_array_3[ 0 ] & 0xfc ) >> 2;
            char_array_4[ 1 ] = ( ( char_array_3[ 0 ] & 0x03 ) << 4 ) + ( ( char_array_3[ 1 ] & 0xf0 ) >> 4 );
            char_array_4[ 2 ] = ( ( char_array_3[ 1 ] & 0x0f ) << 2 ) + ( ( char_array_3[ 2 ] & 0xc0 ) >> 6 );
            char_array_4[ 3 ] = char_array_3[ 2 ] & 0x3f;

            for( i = 0; ( i < 4 ) ; i++ )
            {
                RetVal += guBase64_Chars[ char_array_4[ i ] ];
            }
            i = 0;
        }
    }

    if( i )
    {
        for( j = i; j < 3; j++ )
        {
            char_array_3[ j ] = 0;
        }

        char_array_4[ 0 ] = ( char_array_3[ 0 ] & 0xfc ) >> 2;
        char_array_4[ 1 ] = ( ( char_array_3[ 0 ] & 0x03 ) << 4 ) + ( ( char_array_3[ 1 ] & 0xf0 ) >> 4 );
        char_array_4[ 2 ] = ( ( char_array_3[ 1 ] & 0x0f ) << 2 ) + ( ( char_array_3[ 2 ] & 0xc0 ) >> 6 );
        char_array_4[ 3 ] = char_array_3[ 2 ] & 0x3f;

        for( j = 0; ( j < i + 1 ); j++ )
        {
            RetVal += guBase64_Chars[ char_array_4[ j ] ];
        }

        while( ( i++ < 3 ) )
            RetVal += '=';

    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxMemoryBuffer guBase64Decode( const wxString  &src )
{
    unsigned int len = src.Length();
    int i = 0;
    int j = 0;
    int index = 0;
    unsigned char char_array_3[ 3 ];
    unsigned char char_array_4[ 4 ];
    wxMemoryBuffer RetVal( src.Length() * 5 );
    char CurChar;
    //RetVal.SetDataLen( 0 );
//    guLogMessage( wxT( "Src. Len %i " ), src.Length() );
//    guLogMessage( wxT( "Buf. Len %i %i" ), RetVal.GetBufSize(), RetVal.GetDataLen() );

    while( len-- && ( ( CurChar = src[ index ] ) != '=' ) && IsBase64Char( CurChar ) )
    {
        char_array_4[ i++ ] = CurChar;
        index++;
        if( i == 4 )
        {
            for( i = 0; i < 4; i++ )
            {
                char_array_4[ i ] = guBase64_Chars.Find( char_array_4[ i ] );
            }

            char_array_3[ 0 ] = ( char_array_4[ 0 ] << 2 ) + ( ( char_array_4[ 1 ] & 0x30 ) >> 4 );
            char_array_3[ 1 ] = ( ( char_array_4[ 1 ] & 0xf ) << 4 ) + ( ( char_array_4[ 2 ] & 0x3c ) >> 2 );
            char_array_3[ 2 ] = ( ( char_array_4[ 2 ] & 0x3 ) << 6 ) + char_array_4[ 3 ];

            for( i = 0; ( i < 3 ); i++ )
            {
                RetVal.AppendByte( char_array_3[ i ] );
            }
            i = 0;
        }
    }
    //guLogMessage( wxT( "%i %i %c %i" ), index, len, CurChar, IsBase64Char( CurChar ) );

    if( i )
    {
        for( j = i; j < 4; j++ )
            char_array_4[ j ] = 0;

        for( j = 0; j < 4; j++ )
        {
            char_array_4[ j ] = guBase64_Chars.Find( char_array_4[ j ] );
        }

        char_array_3[ 0 ] = ( char_array_4[ 0 ] << 2 ) + ( ( char_array_4[ 1 ] & 0x30 ) >> 4 );
        char_array_3[ 1 ] = ( ( char_array_4[ 1 ] & 0xf ) << 4 ) + ( ( char_array_4[ 2 ] & 0x3c ) >> 2 );
        char_array_3[ 2 ] = ( ( char_array_4[ 2 ] & 0x3 ) << 6 ) + char_array_4[ 3 ];

        for( j = 0; ( j < i - 1 ); j++ )
        {
            RetVal.AppendByte( char_array_3[ j ] );
        }
    }
    printf( "\n" );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
//int main( void )
//{
//    wxString TestStr = wxT( "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure." );
//    wxString Encoded = guBase64Encode( TestStr.ToAscii(), TestStr.Length() );
//    wprintf( Encoded.c_str() ); printf( "\n\n" );
//    wxString Decoded = guBase64Decode( Encoded );
//    wprintf( Decoded.c_str() ); printf( "\n\n" );
//    return 0;
//}
// -------------------------------------------------------------------------------- //
