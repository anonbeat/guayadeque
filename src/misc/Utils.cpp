// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "Utils.h"

#include "Config.h"
#include "DbLibrary.h"
#include "FileRenamer.h"    // NormalizeField
#include "Http.h"
#include "MediaViewer.h"
#include "Settings.h"

#include <wx/process.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/uri.h>
#include <wx/protocol/http.h>
#include <wx/zstream.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
bool IsColorDark( const wxColour &color )
{
    double darkness = 1 - ( 0.299 * color.Red() + 0.587 * color.Green() + 0.114 * color.Blue() ) / 255;
    return darkness >= 0.5;
}

// -------------------------------------------------------------------------------- //
wxString LenToString( wxUint64 len )
{
    wxString LenStr;
    len /= 1000;
    unsigned int w = 0;
    unsigned int d = 0;
    unsigned int h = 0;
    unsigned int m = 0;
    if( len >= ( 7 * 24 * 60 * 60 ) )
    {
        w = len / ( 7 * 24 * 60 * 60 );
        len %= ( 7 * 24 * 60 * 60 );
    }
    if( len >= ( 24 * 60 * 60 ) )
    {
        d = len / ( 24 * 60 * 60 );
        len %= ( 24 * 60 * 60 );
    }
    if( len >= ( 60 * 60 ) )
    {
        h = len / ( 60 * 60 );
        len %= ( 60 * 60 );
    }
    if( len >= 60 )
    {
        m = len / 60;
        len %= 60;
    }
    if( w > 0 )
    {
        LenStr += LenStr.Format( wxT( "%uw " ), w );
    }
    if( d > 0 )
    {
        LenStr += LenStr.Format( wxT( "%ud " ), d );
    }
    if( h > 0 )
    {
        LenStr += LenStr.Format( wxT( "%02u:" ), h );
    }
    LenStr += LenStr.Format( wxT( "%02u:%02llu" ), m, len );
    //guLogMessage( wxT( "%lu -> %s" ), len, LenStr.c_str() );
    return LenStr;
}

// -------------------------------------------------------------------------------- //
wxString SizeToString( wxFileOffset size )
{
    double s = size;
    wxString Formats[] = { wxT( "%.0f Bytes" ),
                           wxT( "%.2f KB" ),
                           wxT( "%.2f MB" ),
                           wxT( "%.2f GB" ),
                           wxT( "%.2f TB" ),
                           wxT( "%.2f PB" ),
                           wxT( "%.2f EB" ),
                           wxT( "%.2f ZB" ),
                           wxT( "%.2f YB" ) };
    int i = 0;
    while( i < 9 && s >= 1024 )
    {
       s = ( 100 * s / 1024 ) / 100.0;
       i++;
    }
    return wxString::Format( Formats[ i ], s );
}

// -------------------------------------------------------------------------------- //
wxArrayString guSplitWords( const wxString &InputStr )
{
    wxArrayString RetVal;
    wxString TempStr = InputStr;
    wxString ResStr;
    size_t index, len;
    wxRegEx RegEx( wxT( " *([^ ]*|\" *[^\"]* *\") *" ) );
    while( TempStr.Length() && RegEx.Matches( TempStr ) )
    {
        RegEx.GetMatch( &index, &len );
        ResStr = RegEx.GetMatch( TempStr, 1 );
        ResStr.Replace( wxT( "\"" ), wxT( " " ), true );
        RetVal.Add( ResStr.Trim( true ).Trim( false ) );
        //guLogMessage( wxT( "%i  %s" ), RetVal.Count(), ResStr.Trim( true ).Trim( false ).c_str() );
        TempStr = TempStr.Mid( len );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString RemoveSearchFilters( const wxString &SearchStr )
{
    wxString RetVal = SearchStr;
    wxString TempStr;
    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString Filters;
    if( Config )
    {
        Filters = Config->ReadAStr( CONFIG_KEY_SEARCH_FILTERS_FILTER, wxEmptyString, CONFIG_PATH_SEARCH_FILTERS );
    }
    int Count = Filters.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        TempStr = RetVal.Lower();
        int Pos;
        while( ( Pos = TempStr.Find( Filters[ Index ] ) ) != wxNOT_FOUND )
        {
            //RetVal = RetVal.Remove( Pos, Filters[ Index ].Length() );
            RetVal = RetVal.Mid( 0, Pos ) + wxT( " " ) + RetVal.Mid( Pos + Filters[ Index ].Length() );
            TempStr = RetVal.Lower();
        }
    }

    // Get the Info from the album...
    RetVal.Trim( true );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool SearchCoverWords( const wxString &FileName, const wxArrayString &Strings )
{
    int index;
    int count = Strings.Count();
    for( index = 0; index < count; index++ )
    {
        if( FileName.Find( Strings[ index ] ) != wxNOT_FOUND )
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
wxImage * guGetRemoteImage( const wxString &url, wxBitmapType &imgtype )
{
    wxImage *   Image = NULL;

    //guLogMessage( wxT( "Downloading '%s'" ), url.c_str() );
    wxMemoryOutputStream Buffer;
    guHttp Http;
    Http.AddHeader( wxT( "User-Agent" ), guDEFAULT_BROWSER_USER_AGENT );
    //http.AddHeader( wxT( "Accept: * / *" ) );
    //http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
    try {
        if( Http.Get( Buffer, url ) && Buffer.IsOk() && Buffer.GetSize() )
        {
            long ResCode = Http.GetResCode();
            //guLogMessage( wxT( "ResCode: %lu" ), ResCode );
            if( ( ResCode < 200 ) || ( ResCode > 299 ) )
            {
                //guLogMessage( wxT( "Code   : %u\n%s" ), ResCode, http.GetResponseHeader().c_str() );
                if( ( ResCode == 301 ) || ( ResCode == 302 ) || ( ResCode == 307 ) )
                {
                    wxString Location = Http.GetResHeader();
                    int Pos = Location.Lower().Find( wxT( "location: " ) );
                    if( Pos != wxNOT_FOUND )
                    {
                        Location = Location.Mid( Pos + 10 );
                        Location.Truncate( Location.Find( wxT( "\r\n" ) ) );
                        return guGetRemoteImage( Location, imgtype );
                    }
                }
            }

            if( ResCode != 200 )
            {
                guLogMessage( wxT( "Error %lu getting remote image '%s'\n%s" ),
                    Http.GetResCode(),
                    url.c_str(),
                    Http.GetResHeader().c_str() );
            }

            wxMemoryInputStream Ins( Buffer );
            if( Ins.IsOk() )
            {
                Image = new wxImage( Ins );
                if( Image )
                {
                    if( Image->IsOk() )
                    {
                        imgtype = Image->GetType();
                        return Image;
                    }
                    delete Image;
                }
            }
        }
        else
        {
            guLogError( wxT( "Could not get the image from '%s'" ), url.c_str() );
        }
    }
    catch( ... )
    {
        guLogError( wxT( "Exception downloading image '%s'" ), url.c_str() );
    }

    return NULL;
}

// -------------------------------------------------------------------------------- //
bool DownloadImage( const wxString &source, const wxString &target, const wxBitmapType imagetype, int maxwidth, int maxheight )
{
    bool RetVal = false;
    wxBitmapType ImageType;
    wxImage *   Image = guGetRemoteImage( source, ImageType );

    if( Image )
    {
        if( maxwidth != -1 )
        {
            guImageResize( Image, maxwidth, ( maxheight != -1 ) ? maxheight : maxwidth );
        }
        RetVal = Image->SaveFile( target, imagetype );

        delete Image;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool DownloadImage( const wxString &source, const wxString &target, int maxwidth, int maxheight )
{
    return DownloadImage( source, target, wxBITMAP_TYPE_JPEG, maxwidth, maxheight );
}

// -------------------------------------------------------------------------------- //
int DownloadFile( const wxString &Source, const wxString &Target )
{
    //guLogMessage( wxT( "Downloading %s" ), Source.c_str() );
    guHttp http;
    http.AddHeader( wxT( "User-Agent" ), guDEFAULT_BROWSER_USER_AGENT );
    http.SetOption( CURLOPT_FOLLOWLOCATION, 1L );
    http.Get( Target, Source );

    long ResCode = http.GetResCode();
    if( ( ResCode < 200 ) || ( ResCode > 299 ) )
    {
        //guLogMessage( wxT( "Code   : %u\n%s" ), ResCode, http.GetResponseHeader().c_str() );
        if( ( ResCode == 301 ) || ( ResCode == 302 ) || ( ResCode == 307 ) )
        {
            wxString Location = http.GetResHeader();
            int Pos = Location.Lower().Find( wxT( "location: " ) );
            if( Pos != wxNOT_FOUND )
            {
                Location = Location.Mid( Pos + 10 );
                Location.Truncate( Location.Find( wxT( "\r\n" ) ) );
                return DownloadFile( Location, Target );
            }
            return 0;
        }
    }
    else
        return 1;
    return 0;
}

// -------------------------------------------------------------------------------- //
wxString guURLEncode( const wxString &url, bool encodespace )
{
    static const wxChar marks[] = wxT( "-_.\"!~*()'" );

	wxString RetVal;
    wxChar CurChar;

    wxCharBuffer CharBuffer = url.ToUTF8();
	int Index;
	int Count = strlen( CharBuffer );

	for( Index = 0; Index < Count; ++Index )
	{
		CurChar = CharBuffer[ Index ];

        if( ( CurChar >= 'a' && CurChar <= 'z' ) || ( CurChar >= 'A' && CurChar <= 'Z' ) ||
            ( CurChar >= '0' && CurChar <= '9' ) || wxStrchr( marks, CurChar ) )
		{
	        RetVal += CurChar;
	    }
        else if( encodespace && ( CurChar == wxT( ' ' ) ) )
	    {
		    RetVal += wxT( "+" );
		}
		else
		{
            RetVal += wxString::Format( wxT( "%%%02X" ), CurChar & 0xFF );
		}
	}

    //guLogMessage( wxT( "URLEncode: '%s' => '%s'" ), url.c_str(), RetVal.c_str() );

	return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guFileDnDEncode( const wxString &file )
{
  wxString RetVal;
  wxString HexCode;
  size_t index;
  wxCharBuffer CharBuffer = file.ToUTF8();
  size_t StrLen = strlen( CharBuffer );

  for( index = 0; index < StrLen; index++ )
  {
    wxChar C = CharBuffer[ index ];
    {
      static const wxChar marks[] = wxT( " -_.\"/+!~*()'[]%" ); //~!@#$&*()=:/,;?+'

      if( ( C >= 'a' && C <= 'z' ) ||
          ( C >= 'A' && C <= 'Z' ) ||
          ( C >= '0' && C <= '9' ) ||
          wxStrchr( marks, C ) )
      {
        RetVal += C;
      }
      else
      {
        HexCode.Printf( wxT( "%%%02X" ), C & 0xFF );
        RetVal += HexCode;
      }
    }
  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guWebExecute( const wxString &Url )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        wxString BrowserCmd = Config->ReadStr( CONFIG_KEY_GENERAL_BROWSER_COMMAND, wxT( "firefox --new-tab" ), CONFIG_PATH_GENERAL );
        wxString Cmd = Url;
        Cmd.Replace( wxT( "(" ), wxT( "%28" ) );
        Cmd.Replace( wxT( ")" ), wxT( "%29" ) );
        //guLogMessage( wxString::Format( wxT( "%s %s" ), BrowserCmd.c_str(), Cmd.c_str() ).c_str() );
        //return wxShell( wxString::Format( wxT( "%s \"%s\"" ), BrowserCmd.c_str(), Cmd.c_str() ) );
        return wxExecute( BrowserCmd + wxT( " " ) + Cmd );
    }
    return -1;
}

// -------------------------------------------------------------------------------- //
int guExecute( const wxString &Command )
{
    guLogMessage( wxT( "Running command %s" ), Command.c_str() );
    return wxExecute( Command );
}

// -------------------------------------------------------------------------------- //
wxFileOffset guGetFileSize( const wxString &FileName )
{
    wxStructStat St;
    wxStat( FileName, &St );
    return St.st_size;
}

// -------------------------------------------------------------------------------- //
wxString GetUrlContent( const wxString &url, const wxString &referer, bool gzipped )
{
    guHttp  Http;
    //char *      Buffer;
    wxString RetVal = wxEmptyString;

    Http.AddHeader( wxT( "User-Agent" ), guDEFAULT_BROWSER_USER_AGENT );
    Http.AddHeader( wxT( "Accept" ), wxT( "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" ) );
    if( gzipped )
    {
        Http.AddHeader( wxT( "Accept-Encoding" ), wxT( "gzip, deflate" ) );
    }
    Http.AddHeader( wxT( "Accept-Charset" ), wxT( "utf-8" ) );
    if( !referer.IsEmpty() )
    {
        Http.AddHeader( wxT( "Referer" ), referer );
    }

    //guLogMessage( wxT( "Getting content for %s" ), url.c_str() );

    wxMemoryOutputStream Buffer;
    Http.SetOption( CURLOPT_FOLLOWLOCATION, 1L );
    Http.Get( Buffer, url );

    if( Buffer.IsOk() )
    {
        int ResponseCode = Http.GetResCode();
        //guLogMessage( wxT( "ResponseCode: %i" ), ResponseCode );
        if( ResponseCode >= 300  && ResponseCode < 400 )
        {
            //guLogMessage( wxT( "Response %u:\n%s\n%s" ), http.GetResponseCode(), http.GetResponseHeader().c_str(), http.GetResponseBody().c_str() );
            wxString Location = Http.GetResHeader();
            int Pos = Location.Lower().Find( wxT( "location: " ) );
            if( Pos != wxNOT_FOUND )
            {
                Location = Location.Mid( Pos + 10 );
                Location.Truncate( Location.Find( wxT( "\r\n" ) ) );
                if( Location.StartsWith( wxT( "/" ) ) )
                {
                    wxURI Uri( url );
                    wxString NewURL;
                    if( Uri.HasScheme() )
                        NewURL = Uri.GetScheme() + wxT( "://" );
                    NewURL += Uri.GetServer();
                    NewURL += Location;
                    Location = NewURL;
                }
                return GetUrlContent( Location, referer, gzipped );
            }
        }
        else if( ResponseCode >= 400 )
            return wxEmptyString;

        wxString ResponseHeaders = Http.GetResHeader();
        //guLogMessage( wxT( "Response %u:\n%s\n%s" ), Http.GetResponseCode(), Http.GetResponseHeader().c_str(), Http.GetResponseBody().c_str() );

        if( ResponseHeaders.Lower().Find( wxT( "content-encoding: gzip" ) ) != wxNOT_FOUND )
        {
            //guLogMessage( wxT( "Response Headers:\n%s" ), ResponseHeaders.c_str() );
            wxMemoryInputStream Ins( Buffer );
            wxZlibInputStream ZIn( Ins );
            wxStringOutputStream Outs( &RetVal );
            ZIn.Read( Outs );
        }
        else
        {
            //RetVal = wxString( Buffer, wxConvUTF8 );
//            wxStringOutputStream Outs( &RetVal );
//            wxMemoryInputStream Ins( Buffer );
//            Ins.Read( Outs );
            if( Buffer.GetLength() )
            {
                size_t Count = Buffer.GetLength();
                const char * pData = ( const char * ) Buffer.GetOutputStreamBuffer()->GetBufferStart();
                RetVal = wxString( pData, wxConvUTF8, Count );
                if( RetVal.IsEmpty() )
                {
                    RetVal = wxString( pData, wxConvISO8859_1, Count );
                    if( RetVal.IsEmpty() )
                    {
                        RetVal = wxString( pData, wxConvLibc, Count );
                    }
                }
            }
        }
        //free( Buffer );
    }
    else
    {
        guLogError( wxT( "Could not get '%s'" ), url.c_str() );
    }
    //guLogMessage( wxT( "Response:\n%s\n###############" ), RetVal.c_str() );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void CheckSymLinks( wxArrayString &libpaths )
{
    char TmpBuf[ 4096 ];
    int Result;

    int Index;
    int Count = libpaths.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        Result = readlink( libpaths[ Index ].char_str(), TmpBuf, WXSIZEOF( TmpBuf ) - sizeof( char ) );
        if( Result != -1 )
        {
            TmpBuf[ Result ] = 0;
            libpaths[ Index ] = wxString::FromUTF8( TmpBuf );
            guLogMessage( wxT( "Detected symbolic link pointing to %s" ), libpaths[ Index ].c_str() );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool CheckFileLibPath( const wxArrayString &LibPaths, const wxString &filename )
{
    int index;
    int count = LibPaths.Count();
    for( index = 0; index < count; index++ )
    {
        if( filename.StartsWith( LibPaths[ index ] ) )
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
int guGetFileMode( const wxString &filepath )
{
    mode_t mode;
    wxStructStat st;
    if( !filepath.IsEmpty() && stat( ( const char * ) filepath.fn_str(), &st ) == 0 )
    {
        mode = st.st_mode;
    }
    else
    {
        mode_t mask = umask(0777);
        mode = 0666 & ~mask;
        umask(mask);
    }
    return mode;
}

// -------------------------------------------------------------------------------- //
bool guSetFileMode( const wxString &filepath, int mode, bool adding )
{
    int m = mode;
    if( adding )
    {
        m |= guGetFileMode( filepath );
    }

    if( chmod( ( const char * ) filepath.fn_str(), mode ) == -1 )
    {
        guLogError( wxT( "Failed to set file permission for '%s'"), filepath.c_str() );
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guRenameFile( const wxString &oldname, const wxString &newname, bool overwrite )
{
    int Mode = guGetFileMode( oldname );
    if( !wxRenameFile( oldname, newname, overwrite ) )
    {
        return false;
    }
    return guSetFileMode( newname, Mode );;
}

// -------------------------------------------------------------------------------- //
wxString guGetNextXMLChunk( wxFile &xmlfile, wxFileOffset &CurPos, const char * startstr, const char * endstr, const wxMBConv &conv )
{
    #define XMLREAD_BUFFER_SIZE         10240
    wxString RetVal;
    //wxFileOffset CurPos = xmlfile.Tell();
    CurPos = xmlfile.Tell();
    wxFileOffset StartPos = wxNOT_FOUND;
    wxFileOffset EndPos = wxNOT_FOUND;
    int endstrlen = strlen( endstr );
    char * Buffer = ( char * ) malloc( XMLREAD_BUFFER_SIZE + 1 );
    if( Buffer )
    {
        while( StartPos == wxNOT_FOUND )
        {
            int ReadCount = xmlfile.Read( Buffer, XMLREAD_BUFFER_SIZE );
            if( ReadCount != wxInvalidOffset && ReadCount > 0 )
            {
                Buffer[ ReadCount ] = 0;
            }
            else
            {
                break;
            }
            char * StartString = strstr( Buffer, startstr );
            if( StartString )
            {
                StartPos = CurPos + ( StartString - Buffer );
                break;
            }
            else
            {
                CurPos += ReadCount;
            }
        }

        if( StartPos != wxNOT_FOUND )
        {
            xmlfile.Seek( StartPos );
            CurPos = StartPos;
            while( EndPos == wxNOT_FOUND )
            {
                int ReadCount = xmlfile.Read( Buffer, XMLREAD_BUFFER_SIZE );
                if( ReadCount != wxInvalidOffset && ReadCount > 0 )
                {
                    Buffer[ ReadCount ] = 0;
                }
                else
                {
                    break;
                }
                char * EndString = strstr( Buffer, endstr );
                if( EndString )
                {
                    EndPos = CurPos + ( EndString - Buffer ) + endstrlen;
                    break;
                }
                else
                {
                    // Prevent that </artist was partially included
                    CurPos += ReadCount - endstrlen;
                    xmlfile.Seek( -endstrlen, wxFromCurrent );
                }
            }

            if( EndPos != wxNOT_FOUND )
            {
                //guLogMessage( wxT( "Found From %lli => %lli  (%lli)" ), StartPos, EndPos, EndPos - StartPos );
                int BufferSize = EndPos - StartPos;
                if( BufferSize )
                {
                    char * BufferString = ( char * ) malloc( BufferSize + 1 );
                    if( BufferString )
                    {
                        xmlfile.Seek( StartPos );
                        int ReadCount = xmlfile.Read( BufferString, BufferSize );
                        if( ReadCount != wxInvalidOffset )
                        {
                            BufferString[ ReadCount ] = 0;
                            RetVal = wxString( BufferString, conv );
                            //guLogMessage( wxT( "%i %i" ), BufferSize, ReadCount );
                        }

                        free( BufferString );
                    }
                }
            }
        }

        free( Buffer );
    }


    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guExpandTrackMacros( const wxString &pattern, guTrack * track, const int indexpos )
{
    wxString RetVal = pattern;

    if( RetVal.Find( guCOPYTO_ARTIST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ARTIST, NormalizeField( track->m_ArtistName ) );
    if( RetVal.Find( guCOPYTO_ARTIST_LOWER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ARTIST_LOWER, NormalizeField( track->m_ArtistName.Lower() ) );
    if( RetVal.Find( guCOPYTO_ARTIST_UPPER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ARTIST_UPPER, NormalizeField( track->m_ArtistName.Upper() ) );
    if( RetVal.Find( guCOPYTO_ARTIST_FIRST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ARTIST_FIRST, NormalizeField( track->m_ArtistName.Trim( false )[ 0 ] ) );
    if( RetVal.Find( guCOPYTO_ALBUMARTIST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUMARTIST, NormalizeField( track->m_AlbumArtist ) );
    if( RetVal.Find( guCOPYTO_ALBUMARTIST_LOWER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUMARTIST_LOWER, NormalizeField( track->m_AlbumArtist.Lower() ) );
    if( RetVal.Find( guCOPYTO_ALBUMARTIST_UPPER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUMARTIST_UPPER, NormalizeField( track->m_AlbumArtist.Upper() ) );
    if( RetVal.Find( guCOPYTO_ALBUMARTIST_FIRST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUMARTIST_FIRST, NormalizeField( track->m_AlbumArtist.Trim( false )[ 0 ] ) );
    if( RetVal.Find( guCOPYTO_ANYARTIST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ANYARTIST, NormalizeField( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ) );
    if( RetVal.Find( guCOPYTO_ANYARTIST_LOWER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ANYARTIST_LOWER, NormalizeField( ( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ).Lower() ) );
    if( RetVal.Find( guCOPYTO_ANYARTIST_UPPER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ANYARTIST_UPPER, NormalizeField( ( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ).Upper() ) );
    if( RetVal.Find( guCOPYTO_ANYARTIST_FIRST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ANYARTIST_FIRST, NormalizeField( ( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ).Trim( false )[ 0 ] ) );
    if( RetVal.Find( guCOPYTO_ALBUM ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUM, NormalizeField( track->m_AlbumName ) );
    if( RetVal.Find( guCOPYTO_ALBUM_LOWER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUM_LOWER, NormalizeField( track->m_AlbumName.Lower() ) );
    if( RetVal.Find( guCOPYTO_ALBUM_UPPER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUM_UPPER, NormalizeField( track->m_AlbumName.Upper() ) );
    if( RetVal.Find( guCOPYTO_ALBUM_FIRST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUM_FIRST, NormalizeField( track->m_AlbumName.Trim( false )[ 0 ] ) );
    if( RetVal.Find( guCOPYTO_ALBUM_PATH ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_ALBUM_PATH, NormalizeField( track->m_FileName.BeforeLast( wxT( '/' ) ) ) );
    if( RetVal.Find( guCOPYTO_COMPOSER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_COMPOSER, NormalizeField( track->m_Composer ) );
    if( RetVal.Find( guCOPYTO_COMPOSER_LOWER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_COMPOSER_LOWER, NormalizeField( track->m_Composer.Lower() ) );
    if( RetVal.Find( guCOPYTO_COMPOSER_UPPER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_COMPOSER_UPPER, NormalizeField( track->m_Composer.Upper() ) );
    if( RetVal.Find( guCOPYTO_COMPOSER_FIRST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_COMPOSER_FIRST, NormalizeField( track->m_Composer.Trim( false )[ 0 ] ) );
    if( RetVal.Find( guCOPYTO_FILENAME ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_FILENAME, wxFileNameFromPath( track->m_FileName.BeforeLast( wxT( '.' ) ) ) );
    if( RetVal.Find( guCOPYTO_GENRE ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_GENRE, NormalizeField( track->m_GenreName ) );
    if( RetVal.Find( guCOPYTO_GENRE_LOWER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_GENRE_LOWER, NormalizeField( track->m_GenreName.Lower() ) );
    if( RetVal.Find( guCOPYTO_GENRE_UPPER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_GENRE_UPPER, NormalizeField( track->m_GenreName.Upper() ) );
    if( RetVal.Find( guCOPYTO_GENRE_FIRST ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_GENRE_FIRST, NormalizeField( track->m_GenreName.Trim( false )[ 0 ] ) );
    if( RetVal.Find( guCOPYTO_NUMBER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_NUMBER, wxString::Format( wxT( "%02u" ), track->m_Number ) );
    if( RetVal.Find( guCOPYTO_TITLE ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_TITLE, NormalizeField( track->m_SongName ) );
    if( RetVal.Find( guCOPYTO_TITLE_LOWER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_TITLE_LOWER, NormalizeField( track->m_SongName.Lower() ) );
    if( RetVal.Find( guCOPYTO_TITLE_UPPER ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_TITLE_UPPER, NormalizeField( track->m_SongName.Upper() ) );
    if( RetVal.Find( guCOPYTO_YEAR ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_YEAR, wxString::Format( wxT( "%u" ), track->m_Year ) );
    if( RetVal.Find( guCOPYTO_DISC ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_DISC, NormalizeField( track->m_Disk ) );
    if( RetVal.Find( guCOPYTO_INDEX ) != wxNOT_FOUND )
        RetVal.Replace( guCOPYTO_INDEX, wxString::Format( wxT( "%04u" ), indexpos ) );

    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guIsValidImageFile( const wxString &filename )
{
    return filename.EndsWith( wxT( ".jpg" ) ) ||
           filename.EndsWith( wxT( ".jpeg" ) ) ||
           filename.EndsWith( wxT( ".png" ) ) ||
           filename.EndsWith( wxT( ".bmp" ) ) ||
           filename.EndsWith( wxT( ".gif" ) );
}

// -------------------------------------------------------------------------------- //
bool guRemoveDir( const wxString &path )
{
    wxString CurPath = path;
    if( !CurPath.EndsWith( wxT( "/" ) ) )
        CurPath += wxT( "/" );

    wxDir         Dir;
    wxString      FileName;

    Dir.Open( CurPath );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
        {
            do {
                if( FileName == wxT( "." ) )
                    continue;

                if( FileName == wxT( ".." ) )
                    continue;

                if( Dir.Exists( CurPath + FileName ) )
                {
                    if( !guRemoveDir( CurPath + FileName ) )
                    {
                        guLogMessage( wxT( "Could not remove dir '%s'" ), wxString( CurPath + FileName ).c_str() );
                        return false;
                    }
                }
                else
                {
                    if( !wxRemoveFile( CurPath + FileName ) )
                    {
                        guLogMessage( wxT( "Could not remove file '%s'" ), wxString( CurPath + FileName ).c_str() );
                        return false;
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }

        return wxRmdir( CurPath );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void GetMediaViewerTracks( const guTrackArray &sourcetracks, const wxArrayInt &sourceflags,
                                 const guMediaViewer * mediaviewer, guTrackArray &tracks, wxArrayInt &changedflags )
{
    int Index;
    int Count = sourcetracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = sourcetracks[ Index ];
        if( Track.m_MediaViewer == mediaviewer )
        {
            tracks.Add( new guTrack( Track ) );
            changedflags.Add( sourceflags[ Index ] );
        }
    }
}

// -------------------------------------------------------------------------------- //
void GetMediaViewerTracks( const guTrackArray &sourcetracks, const int flags,
                                 const guMediaViewer * mediaviewer, guTrackArray &tracks, wxArrayInt &changedflags )
{
    int Index;
    int Count = sourcetracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = sourcetracks[ Index ];
        if( Track.m_MediaViewer == mediaviewer )
        {
            tracks.Add( new guTrack( Track ) );
            changedflags.Add( flags );
        }
    }
}

// -------------------------------------------------------------------------------- //
void GetMediaViewerTracks( const guTrackArray &sourcetracks, const guMediaViewer * mediaviewer, guTrackArray &tracks )
{
    int Index;
    int Count = sourcetracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = sourcetracks[ Index ];
        if( Track.m_MediaViewer == mediaviewer )
        {
            tracks.Add( new guTrack( Track ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void GetMediaViewersList( const guTrackArray &tracks, wxArrayPtrVoid &MediaViewerPtrs )
{
    int Index;
    int Count = tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = tracks[ Index ];
        guMediaViewer * MediaViewer = Track.m_MediaViewer;
        if( MediaViewer )
        {
            if( MediaViewerPtrs.Index( MediaViewer ) == wxNOT_FOUND )
            {
                MediaViewerPtrs.Add( MediaViewer );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString ExtractString( const wxString &source, const wxString &start, const wxString &end )
{
    int StartPos = source.Find( start );
    int EndPos;
    if( StartPos != wxNOT_FOUND )
    {
        wxString SearchStr = source.Mid( StartPos + start.Length() );
        EndPos = SearchStr.Find( end );
        if( EndPos != wxNOT_FOUND )
        {
            return SearchStr.Mid( 0, EndPos );
        }
    }
    return wxEmptyString;
}

}

// -------------------------------------------------------------------------------- //
