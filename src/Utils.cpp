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
#include "Utils.h"
#include "Config.h"

#include <wx/curl/http.h>
#include <wx/regex.h>

// -------------------------------------------------------------------------------- //
wxString LenToString( int Len )
{
    wxString LenStr;
    int d = 0;
    int h = 0;
    int m = 0;
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
    if( d > 0 )
    {
        LenStr += LenStr.Format( wxT( "%ud " ), d );
    }
    if( h > 0 )
    {
        LenStr += LenStr.Format( wxT( "%02u:" ), h );
    }
    LenStr += LenStr.Format( wxT( "%02u:%02u" ), m, Len );
    return LenStr;
}

// -------------------------------------------------------------------------------- //
wxArrayString guSplitWords( const wxString &InputStr )
{
    wxArrayString RetVal;
    wxString TempStr = InputStr;
    wxString ResStr;
    size_t index, len;
    wxRegEx RegEx( wxT( " *([^ ]*|\" *[^\"]* *\") *" ) );
    while( TempStr.Length() && RegEx.Matches( InputStr ) )
    {
        RegEx.GetMatch( &index, &len );
        ResStr = RegEx.GetMatch( TempStr, 1 );
        ResStr.Replace( wxT( "\"" ), wxT( " " ), true );
        RetVal.Add( ResStr.Trim( true ).Trim( false ) );
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
            RetVal = RetVal.Remove( Pos, Filters[ Index ].Length() );
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
        if( FileName.Find( Strings[ index ] ) )
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool DownloadImage( const wxString &Source, const wxString &Target )
{
    bool        RetVal = false;
    long        ImageType;
    wxImage *   Image = NULL;

    if( Source.Lower().EndsWith( wxT( ".jpg" ) ) )
      ImageType = wxBITMAP_TYPE_JPEG;
    else if( Source.Lower().EndsWith( wxT( ".png" ) ) )
      ImageType = wxBITMAP_TYPE_PNG;
//    else if( UrlStr.Lower().EndsWith( wxT( ".gif" ) ) ) // Removed because of some random segfaults
//      ImageType = wxBITMAP_TYPE_GIF;                    // in gifs handler functions
    else if( Source.Lower().EndsWith( wxT( ".bmp" ) ) )
      ImageType = wxBITMAP_TYPE_BMP;
    else
      ImageType = wxBITMAP_TYPE_INVALID;

    if( ImageType > wxBITMAP_TYPE_INVALID )
    {
        wxMemoryOutputStream Buffer;
        wxCurlHTTP http;
        if( http.Get( Buffer, Source ) )
        {
            if( Buffer.IsOk() )
            {
                wxMemoryInputStream Ins( Buffer );
                if( Ins.IsOk() )
                {
                    Image = new wxImage( Ins, ImageType );
                    if( Image )
                    {
                        if( Image->IsOk() )
                        {
                            RetVal = Image->SaveFile( Target, wxBITMAP_TYPE_JPEG );
                        }
                        delete Image;
                    }
                }
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int DownloadFile( const wxString &Source, const wxString &Target )
{
    //guLogMessage( wxT( "Downloading %s" ), Source.c_str() );
    wxCurlHTTP http;
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    //http.SetVerbose( true );
    http.Get( Target, Source );

    long ResCode = http.GetResponseCode();
    guLogMessage( wxT( "Code   : %u" ), ResCode );
    if( ResCode < 200 || ResCode > 299 )
    {
        if( ResCode == 301 || ResCode == 307 )
        {
            wxString Location = http.GetResponseHeader();
            int Pos = Location.Find( wxT( "Location: " ) );
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

  for( index = 0; index < Source.Length(); index++ )
  {
    wxChar C = Source.GetChar( index );
    if( C == wxT(' '))
    {
      RetVal += wxT( "+" );
    }
    else
    {
      static const wxChar marks[] = wxT( "-_.\"+!~*()'" );

      if( !wxIsalnum( C ) && !wxStrchr( marks, C ) /*&& !wxStrchr( delims, C )*/ )
      {
        HexCode.Printf( wxT( "%%%02X" ), C );
        RetVal += HexCode;
      }
      else
      {
        RetVal += C;
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
        return wxShell( wxString::Format( wxT( "%s \"%s\"" ), BrowserCmd.c_str(), Url.c_str() ) );
    }
    return -1;
}

// -------------------------------------------------------------------------------- //
