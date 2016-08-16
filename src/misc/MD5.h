// -------------------------------------------------------------------------------- //
//
// Name:        md5.h
// Author:      Kai Krahn
// Created:     11.11.07 12:35

// Description: wxWidgets md5 header
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

#ifndef __MD5_H__
#define __MD5_H__

#ifdef __BORLANDC__
        #pragma hdrstop
#endif

#ifndef WX_PRECOMP
        #include <wx/wx.h>
#else
        #include <wx/wxprec.h>
#endif

typedef unsigned int uint32;

// -------------------------------------------------------------------------------- //
class guMD5CTX
{
  private:

    uint32 m_State[ 4 ];
    uint32 m_Count[ 2 ];
    unsigned char m_Buffer[ 64 ];

    void Transform( uint32 * in );

  public:

    guMD5CTX();
    void Init();
    void Update( const unsigned char * InBuffer, unsigned Len );
    void Final( unsigned char Digest[ 16 ] );

};

// -------------------------------------------------------------------------------- //
class guMD5
{
  private :
    guMD5CTX m_Context;

  public :
    guMD5(){}

    wxString inline MD5( const wxString &Input )
    {
        return MD5( Input.char_str(), Input.Length() );
    };

    wxString inline MD5( const wxChar * Data, unsigned int Len )
    {
        return MD5( ( unsigned char * ) Data, Len );
    };

    wxString MD5( const unsigned char * Data, unsigned int Len );

    wxString inline MD5( const void * Data, unsigned int Len )
    {
        return MD5( ( unsigned char * ) Data, Len );
    };

    wxString MD5File( const wxString &FileName );

};

#endif
// -------------------------------------------------------------------------------- //
