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
#include "Utils.h"

#include "Config.h"
#include "DbLibrary.h"
#include "FileRenamer.h"    // NormalizeField

#include <wx/curl/http.h>
#include <wx/process.h>
#include <wx/regex.h>
#include <wx/uri.h>
#include <wx/zstream.h>

// -------------------------------------------------------------------------------- //
wxString LenToString( const unsigned int len )
{
    wxString LenStr;
    unsigned int Len = len;
    unsigned int w = 0;
    unsigned int d = 0;
    unsigned int h = 0;
    unsigned int m = 0;
    if( Len >= ( 7 * 24 * 60 * 60 ) )
    {
        w = Len / ( 7 * 24 * 60 * 60 );
        Len %= ( 7 * 24 * 60 * 60 );
    }
    if( Len >= ( 24 * 60 * 60 ) )
    {
        d = Len / ( 24 * 60 * 60 );
        Len %= ( 24 * 60 * 60 );
    }
    if( Len >= ( 60 * 60 ) )
    {
        h = Len / ( 60 * 60 );
        Len %= ( 60 * 60 );
    }
    if( Len >= 60 )
    {
        m = Len / 60;
        Len %= 60;
    }
    LenStr = wxEmptyString;
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
    LenStr += LenStr.Format( wxT( "%02u:%02u" ), m, Len );
//    guLogMessage( wxT( "%i -> %s" ), Len, LenStr.c_str() );
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
        Filters = Config->ReadAStr( wxT( "Filter" ), wxEmptyString, wxT( "SearchFilters" ) );
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
wxImage * guGetRemoteImage( const wxString &url, int &imgtype )
{
    wxImage *   Image = NULL;
    wxURI       Uri( url );

    wxString FileName = Uri.GetPath().Lower();

    //guLogMessage( wxT( "Downloading '%s' from '%s'" ), FileName.c_str(), url.c_str() );

    wxMemoryOutputStream Buffer;
    wxCurlHTTP http;
    http.AddHeader( wxT( "User-Agent: " ) guDEFAULT_BROWSER_USER_AGENT );
    //http.AddHeader( wxT( "Accept: */*" ) );
    //http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
    try {
        http.Get( Buffer, url );
        if( Buffer.IsOk() )
        {
            long ResCode = http.GetResponseCode();
            //guLogMessage( wxT( "ResCode: %lu" ), ResCode );
            if( ( ResCode < 200 ) || ( ResCode > 299 ) )
            {
                //guLogMessage( wxT( "Code   : %u\n%s" ), ResCode, http.GetResponseHeader().c_str() );
                if( ( ResCode == 301 ) || ( ResCode == 302 ) || ( ResCode == 307 ) )
                {
                    wxString Location = http.GetResponseHeader();
                    int Pos = Location.Lower().Find( wxT( "location: " ) );
                    if( Pos != wxNOT_FOUND )
                    {
                        Location = Location.Mid( Pos + 10 );
                        Location.Truncate( Location.Find( wxT( "\r\n" ) ) );
                        return guGetRemoteImage( Location, imgtype );
                    }
                }
            }

    //        if( ResCode != 200 )
    //        {
    //            guLogMessage( wxT( "Error %u getting remote image '%s'\n%s" ),
    //                http.GetResponseCode(),
    //                url.c_str(),
    //                http.GetResponseHeader().c_str() );
    //        }

            if( Buffer.IsOk() )
            {
                wxMemoryInputStream Ins( Buffer );
                if( Ins.IsOk() )
                {
                    if( FileName.EndsWith( wxT( ".jpg" ) ) ||
                        FileName.EndsWith( wxT( ".jpeg" ) ) )
                      imgtype = wxBITMAP_TYPE_JPEG;
                    else if( FileName.EndsWith( wxT( ".png" ) ) )
                      imgtype = wxBITMAP_TYPE_PNG;
                    else if( FileName.EndsWith( wxT( ".gif" ) ) )
                      imgtype = wxBITMAP_TYPE_GIF;
                    else if( FileName.EndsWith( wxT( ".bmp" ) ) )
                      imgtype = wxBITMAP_TYPE_BMP;
                    else
                      imgtype = wxBITMAP_TYPE_INVALID;

                    if( imgtype != wxBITMAP_TYPE_INVALID )
                    {
                        Image = new wxImage( Ins, imgtype );
                        if( Image )
                        {
                            if( Image->IsOk() )
                            {
                                return Image;
                            }
                            delete Image;
                        }
                    }
                }
            }
        }
    }
    catch( ... )
    {
        guLogError( wxT( "Exception downloading image '%s'" ), url.c_str() );
    }

    return NULL;
}

// -------------------------------------------------------------------------------- //
bool DownloadImage( const wxString &source, const wxString &target, const int imagetype, int maxwidth, int maxheight )
{
    bool        RetVal = false;
    int         ImageType;
    wxImage *   Image = guGetRemoteImage( source, ImageType );

    if( Image )
    {
        if( maxwidth != -1 )
        {
            if( maxheight != -1 )
                Image->Rescale( maxwidth, maxheight, wxIMAGE_QUALITY_HIGH );
            else
                Image->Rescale( maxwidth, maxwidth, wxIMAGE_QUALITY_HIGH );
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
    wxCurlHTTP http;
    http.AddHeader( wxT( "User-Agent: " ) guDEFAULT_BROWSER_USER_AGENT );
    //http.SetVerbose( true );
    http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
    http.Get( Target, Source );

    long ResCode = http.GetResponseCode();
    if( ( ResCode < 200 ) || ( ResCode > 299 ) )
    {
        //guLogMessage( wxT( "Code   : %u\n%s" ), ResCode, http.GetResponseHeader().c_str() );
        if( ( ResCode == 301 ) || ( ResCode == 302 ) || ( ResCode == 307 ) )
        {
            wxString Location = http.GetResponseHeader();
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
wxString guURLEncode( const wxString &Source )
{
  //wxString delims = wxT( ";/?:@&=+$," );
  // TODO : Check consecuencies of remove the delims
  wxString RetVal;
  wxString HexCode;
  size_t index;
  wxCharBuffer CharBuffer = Source.ToUTF8();

  for( index = 0; index < strlen( CharBuffer ); index++ )
  {
    unsigned char C = CharBuffer[ index ];
    if( C == wxT(' '))
    {
      RetVal += wxT( "+" );
    }
    else
    {
      static const wxChar marks[] = wxT( "-_.\"+!~*()'" );
      //static const wxChar marks[] = wxT( "-_.\"+!~*()" );

      //if( !wxIsalnum( C ) && !wxStrchr( marks, C ) /*&& !wxStrchr( delims, C )*/ )
      if( ( C >= 'a' && C <= 'z' ) ||
          ( C >= 'A' && C <= 'Z' ) ||
          ( C >= '0' && C <= '9' ) ||
          wxStrchr( marks, C ) )
      {
        RetVal += C;
      }
      else
      {
        HexCode.Printf( wxT( "%%%02X" ), C );
        RetVal += HexCode;
      }
    }
  }
//  guLogMessage( wxT( "guURLEncode: %s -> %s" ), Source.c_str(), RetVal.c_str() );
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
    unsigned char C = CharBuffer[ index ];
    {
      static const wxChar marks[] = wxT( " -_.\"/+!~*()'[]%" );

      if( ( C >= 'a' && C <= 'z' ) ||
          ( C >= 'A' && C <= 'Z' ) ||
          ( C >= '0' && C <= '9' ) ||
          wxStrchr( marks, C ) )
      {
        RetVal += C;
      }
      else
      {
        HexCode.Printf( wxT( "%%%02X" ), C );
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
        wxString BrowserCmd = Config->ReadStr( wxT( "BrowserCommand" ), wxT( "firefox --new-tab" ), wxT( "General" ) );
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
    wxCurlHTTP  http;
    //char *      Buffer;
    wxString RetVal = wxEmptyString;

    http.AddHeader( wxT( "User-Agent: " ) guDEFAULT_BROWSER_USER_AGENT );
    http.AddHeader( wxT( "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" ) );
    if( gzipped )
    {
        http.AddHeader( wxT( "Accept Encoding: gzip,deflate" ) );
    }
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    if( !referer.IsEmpty() )
    {
        http.AddHeader( wxT( "Referer: " ) + referer );
    }

    //guLogMessage( wxT( "Getting content for %s" ), url.c_str() );

    wxMemoryOutputStream Buffer;
    http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
    http.Get( Buffer, url );

    if( Buffer.IsOk() )
    {
        int ResponseCode = http.GetResponseCode();
        //guLogMessage( wxT( "ResponseCode: %i" ), ResponseCode );
        if( ResponseCode >= 300  && ResponseCode < 400 )
        {
            //guLogMessage( wxT( "Response %u:\n%s\n%s" ), http.GetResponseCode(), http.GetResponseHeader().c_str(), http.GetResponseBody().c_str() );
            wxString Location = http.GetResponseHeader();
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

        wxString ResponseHeaders = http.GetResponseHeader();
        //guLogMessage( wxT( "Response %u:\n%s\n%s" ), http.GetResponseCode(), http.GetResponseHeader().c_str(), http.GetResponseBody().c_str() );

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
wxString guGetNextXMLChunk( wxFile &xmlfile, wxFileOffset &CurPos, const char * startstr, const char * endstr )
{
    #define XMLREAD_BUFFER_SIZE     2048
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
                            RetVal = wxString( BufferString, wxConvUTF8 );
                            //guLogMessage( wxT( "%s" ), RetVal.c_str() );
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
wxString guExpandTrackMacros( const wxString &pattern, guTrack * track )
{
    wxString RetVal = pattern;

    if( RetVal.Find( wxT( "{a}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{a}" ), NormalizeField( track->m_ArtistName ) );
    if( RetVal.Find( wxT( "{al}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{al}" ), NormalizeField( track->m_ArtistName.Lower() ) );
    if( RetVal.Find( wxT( "{au}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{au}" ), NormalizeField( track->m_ArtistName.Upper() ) );
    if( RetVal.Find( wxT( "{a1}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{a1}" ), NormalizeField( track->m_ArtistName.Trim( false )[ 0 ] ) );
    if( RetVal.Find( wxT( "{aa}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{aa}" ), NormalizeField( track->m_AlbumArtist ) );
    if( RetVal.Find( wxT( "{aal}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{aal}" ), NormalizeField( track->m_AlbumArtist.Lower() ) );
    if( RetVal.Find( wxT( "{aau}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{aau}" ), NormalizeField( track->m_AlbumArtist.Upper() ) );
    if( RetVal.Find( wxT( "{aa1}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{aa1}" ), NormalizeField( track->m_AlbumArtist.Trim( false )[ 0 ] ) );
    if( RetVal.Find( wxT( "{A}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{A}" ), NormalizeField( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ) );
    if( RetVal.Find( wxT( "{Al}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{Al}" ), NormalizeField( ( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ).Lower() ) );
    if( RetVal.Find( wxT( "{Au}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{Au}" ), NormalizeField( ( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ).Upper() ) );
    if( RetVal.Find( wxT( "{A1}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{A1}" ), NormalizeField( ( track->m_AlbumArtist.IsEmpty() ? track->m_ArtistName : track->m_AlbumArtist ).Trim( false )[ 0 ] ) );
    if( RetVal.Find( wxT( "{b}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{b}" ), NormalizeField( track->m_AlbumName ) );
    if( RetVal.Find( wxT( "{bl}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{bl}" ), NormalizeField( track->m_AlbumName.Lower() ) );
    if( RetVal.Find( wxT( "{bu}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{bu}" ), NormalizeField( track->m_AlbumName.Upper() ) );
    if( RetVal.Find( wxT( "{b1}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{b1}" ), NormalizeField( track->m_AlbumName.Trim( false )[ 0 ] ) );
    if( RetVal.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{bp}" ), NormalizeField( track->m_FileName.BeforeLast( wxT( '/' ) ) ) );
    if( RetVal.Find( wxT( "{c}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{c}" ), NormalizeField( track->m_Composer ) );
    if( RetVal.Find( wxT( "{cl}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{cl}" ), NormalizeField( track->m_Composer.Lower() ) );
    if( RetVal.Find( wxT( "{cu}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{cu}" ), NormalizeField( track->m_Composer.Upper() ) );
    if( RetVal.Find( wxT( "{c1}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{c1}" ), NormalizeField( track->m_Composer.Trim( false )[ 0 ] ) );
    if( RetVal.Find( wxT( "{f}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{f}" ), wxFileNameFromPath( track->m_FileName.BeforeLast( wxT( '.' ) ) ) );
    if( RetVal.Find( wxT( "{g}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{g}" ), NormalizeField( track->m_GenreName ) );
    if( RetVal.Find( wxT( "{gl}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{gl}" ), NormalizeField( track->m_GenreName.Lower() ) );
    if( RetVal.Find( wxT( "{gu}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{gu}" ), NormalizeField( track->m_GenreName.Upper() ) );
    if( RetVal.Find( wxT( "{g1}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{g1}" ), NormalizeField( track->m_GenreName.Trim( false )[ 0 ] ) );
    if( RetVal.Find( wxT( "{n}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{n}" ), wxString::Format( wxT( "%02u" ), track->m_Number ) );
    if( RetVal.Find( wxT( "{t}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{t}" ), NormalizeField( track->m_SongName ) );
    if( RetVal.Find( wxT( "{tl}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{tl}" ), NormalizeField( track->m_SongName.Lower() ) );
    if( RetVal.Find( wxT( "{tu}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{tu}" ), NormalizeField( track->m_SongName.Upper() ) );
    if( RetVal.Find( wxT( "{y}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{y}" ), wxString::Format( wxT( "%u" ), track->m_Year ) );
    if( RetVal.Find( wxT( "{d}" ) ) != wxNOT_FOUND )
        RetVal.Replace( wxT( "{d}" ), NormalizeField( track->m_Disk ) );

    return RetVal;
}

// -------------------------------------------------------------------------------- //
