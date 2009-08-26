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

#include <wx/sstream.h>

// -------------------------------------------------------------------------------- //
static const wxChar guBase64_Chars[] = wxT( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );
static const wxChar guBase64_Pad = wxT( '=' );

// -------------------------------------------------------------------------------- //
#define IsBase64Char( c ) ( ( c >= 'A' && c <= 'Z' ) ||\
                            ( c >= 'a' && c <= 'z' ) ||\
                            ( c >= '0' && c <= '9' ) ||\
                            ( c == '+' ) || ( c == '/' ) )

// -------------------------------------------------------------------------------- //
wxString guBase64Encode( const char * src, const unsigned int len )
{
    wxMemoryInputStream Ins( src, len );
    return guBase64Encode( Ins );
}

// -------------------------------------------------------------------------------- //
wxString guBase64Encode( const wxMemoryInputStream &ins )
{
    wxString EncodedString;
    int Index;
    int Count = ins.GetLength();
    EncodedString.Alloc( ( ( Count / 3 ) + ( ( Count % 3 ) > 0 ) ) * 4 );
    wxUint32 Temp;
    char * pData = ( char * ) ins.GetInputStreamBuffer()->GetBufferStart();
    for( Index = 0; Index < Count / 3; Index++ )
    {
        Temp  = ( * pData++ ) << 16;
        Temp += ( * pData++ ) <<  8;
        Temp += ( * pData++ );
        EncodedString.Append( guBase64_Chars[ ( Temp & 0x00FC0000 ) >> 18 ] );
        EncodedString.Append( guBase64_Chars[ ( Temp & 0x0003F000 ) >> 12 ] );
        EncodedString.Append( guBase64_Chars[ ( Temp & 0x00000FC0 ) >>  6 ] );
        EncodedString.Append( guBase64_Chars[ ( Temp & 0x0000003F )       ] );
    }
    switch( Count % 3 )
    {
        case 1 :
            Temp = ( * pData++ ) << 16;
            EncodedString.Append( guBase64_Chars[ ( Temp & 0x00FC0000 ) >> 18 ] );
            EncodedString.Append( guBase64_Chars[ ( Temp & 0x0003F000 ) >> 12 ] );
            EncodedString.Append( guBase64_Pad, 2 );
            break;

        case 2 :
            Temp  = ( * pData++ ) << 16;
            Temp += ( * pData++ ) <<  8;
            EncodedString.Append( guBase64_Chars[ ( Temp & 0x00FC0000 ) >> 18 ] );
            EncodedString.Append( guBase64_Chars[ ( Temp & 0x0003F000 ) >> 12 ] );
            EncodedString.Append( guBase64_Chars[ ( Temp & 0x00000FC0 ) >>  6 ] );
            EncodedString.Append( guBase64_Pad, 1 );
            break;
    }
    return EncodedString;
}

// -------------------------------------------------------------------------------- //
wxMemoryBuffer guBase64Decode( const wxString &ins )
{
    wxMemoryBuffer DecodedBytes;
    int Index;
    int Count = ins.Length();
    if( !( Count % 4 ) )
    {
        DecodedBytes.SetBufSize( ( Count / 4 ) * 3 );
        wxUint32 Temp = 0;
        //guLogMessage( wxT( "Decoding %s" ), ins.c_str() );
        const wxChar * pData = ins.c_str();
        for( Index = 0; Index < Count / 4; Index++ )
        {
            int iPos;
            for( iPos = 0; iPos < 4; iPos++ )
            {
                Temp <<= 6;
                if( * pData >= 0x41 && * pData <= 0x5A ) // A .. Z
                {
                    Temp |= * pData - 0x41;
                }
                else if( * pData >= 0x61 && * pData <= 0x7A ) // a .. z
                {
                    Temp |= * pData - 0x47;
                }
                else if( * pData >= 0x30 && * pData <= 0x39 )
                {
                    Temp |= * pData + 0x04;
                }
                else if( * pData == '+' )
                {
                    Temp |= 0x3E;
                }
                else if( * pData == '/' )
                {
                    Temp |= 0x3F;
                }
                else if( * pData == '=' )   // Pad Character
                {
                    switch(  Count - ( ( Index * 4 ) + iPos ) )
                    {
                        case 1 :
                            DecodedBytes.AppendByte( ( Temp >> 16 ) & 0x000000FF );
                            DecodedBytes.AppendByte( ( Temp >>  8 ) & 0x000000FF );
                            break;
                        case 2 :
                            DecodedBytes.AppendByte( ( Temp >> 10 ) & 0x000000FF );
                            break;
                        default :
                            guLogError( wxT( "Invalid pad character in Base64" ) );
                    }
                    return DecodedBytes;
                }
                else
                {
                    guLogError( wxT( "Invalid Base64 character %02x at pos %u" ), * pData, ( Index * 4 ) + iPos );
                    return DecodedBytes;
                }
                pData++;
            }
            DecodedBytes.AppendByte( ( Temp >> 16 ) & 0x000000FF );
            DecodedBytes.AppendByte( ( Temp >>  8 ) & 0x000000FF );
            DecodedBytes.AppendByte( ( Temp       ) & 0x000000FF );
        }
    }
    else
    {
        guLogError( wxT( "Wrong Base64 data length" ) );
    }
    return DecodedBytes;
}

// -------------------------------------------------------------------------------- //
//int main( void )
//{
//    wxString TestStr = wxT( "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure." );
//    wprintf( TestStr.c_str() ); wprintf( wxT( " (%u)\n\n" ), TestStr.Length() );
//    wxString Encoded = guBase64Encode( TestStr.ToAscii(), TestStr.Length() );
//    wprintf( Encoded.c_str() ); wprintf( wxT( "\n\n" ) );
//    wxMemoryBuffer Decoded = guBase64Decode( Encoded );
//    wxString DStr;
//    wprintf( wxT( "String Length %u" ), Decoded.GetDataLen() );
//    DStr = wxString::From8BitData( ( char * ) Decoded.GetData(), Decoded.GetDataLen() );
//    wprintf( DStr.c_str() );  wprintf( wxT( "\n\n" ) );
//    return 0;
//}
// -------------------------------------------------------------------------------- //
