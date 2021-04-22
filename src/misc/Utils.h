// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#ifndef _UTILS_H
#define _UTILS_H

#include <wx/wx.h>
#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

#ifdef NDEBUG
    #define guLogDebug(...)
    #define guLogTrace(...) guLogMsgIfDebug( __VA_ARGS__ )
    #define guLogMessage    wxLogMessage
    #define guLogWarning    wxLogWarning
    #define guLogError      wxLogError
#else
    #define GU_DEBUG
    #define guLogDebug(...) guLogMsgIfDebug( __VA_ARGS__ )
    #define guLogTrace      wxLogMessage
    #define guLogMessage    wxLogMessage
    #define guLogWarning    wxLogWarning
    #define guLogError      wxLogError
#endif

#define guRandomInit() (srand( time( NULL ) ))
#define guRandom(x) (rand() % x)

#define guDEFAULT_BROWSER_USER_AGENT    wxT( "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/55.0.2883.87 Chrome/55.0.2883.87 Safari/537.36" )

class guTrackArray;
class guMediaViewer;

static bool guDebugMode     = std::getenv( "GU_DEBUG" ) != NULL;
static bool guGatherStats   = std::getenv( "GU_STATS" ) != NULL;

template<typename... Args>
static void guLogMsgIfDebug( Args... what )
{
    if( guDebugMode )
        guLogMessage( what... );
}

template<typename... Args>
static void guLogStats( Args... what )
{
    if( guGatherStats )
        guLogMessage( what... );
}

// -------------------------------------------------------------------------------- //
void inline         guImageResize( wxImage * image, int maxsize, bool forceresize = false )
{
    int w = image->GetWidth();
    int h = image->GetHeight();

    double ratio = wxMin( static_cast<double>( maxsize ) / h,
                          static_cast<double>( maxsize ) / w );

    if( forceresize || ( ratio < 1 ) )
    {
        image->Rescale( ( w * ratio ) + .5, ( h * ratio ) + .5, wxIMAGE_QUALITY_HIGH );
    }
}

// -------------------------------------------------------------------------------- //
time_t inline         GetFileLastChangeTime( const wxString &filename )
{
    wxStructStat St;
    if( wxStat( filename, &St ) )
        return -1;
    return St.st_ctime;
}

// -------------------------------------------------------------------------------- //
bool inline         IsFileSymbolicLink( const wxString &filename )
{
    wxStructStat St;
    wxLstat( filename, &St );
    return S_ISLNK( St.st_mode );
}


class guTrack;

// -------------------------------------------------------------------------------- //
bool                IsColorDark( const wxColour &color );
wxString            LenToString( wxUint64 len );
wxString            SizeToString( wxFileOffset size );
wxArrayString       guSplitWords( const wxString &InputStr );
wxImage *           guGetRemoteImage( const wxString &url, wxBitmapType &imgtype );
bool                DownloadImage( const wxString &source, const wxString &target, const wxBitmapType imagetype, int maxwidth, int maxheight );
bool                DownloadImage( const wxString &source, const wxString &taget, int maxwidth = -1, int maxheight = -1 );
int                 DownloadFile( const wxString &Source, const wxString &Target );
wxString            RemoveSearchFilters( const wxString &Album );
bool                SearchCoverWords( const wxString &filename, const wxArrayString &Strings );
wxString            guURLEncode( const wxString &url, bool encodespace = true );
wxString            guFileDnDEncode( const wxString &file );
int                 guWebExecute( const wxString &Url );
int                 guExecute( const wxString &Command );
wxFileOffset        guGetFileSize( const wxString &filename );
wxString            GetUrlContent( const wxString &url, const wxString &referer = wxEmptyString, bool encoding = false );
void                CheckSymLinks( wxArrayString &libpaths );
bool                CheckFileLibPath( const wxArrayString &LibPaths, const wxString &filename );
int                 guGetFileMode( const wxString &filepath );
bool                guSetFileMode( const wxString &filepath, int mode, bool adding = false );
bool                guRenameFile( const wxString &oldname, const wxString &newname, bool overwrite = true );
wxString            guGetNextXMLChunk( wxFile &xmlfile, wxFileOffset &CurPos, const char * startstr, const char * endstr, const wxMBConv &conv = wxConvUTF8 );
wxString            guExpandTrackMacros( const wxString &pattern, guTrack * track, const int indexpos = 0 );
bool                guIsValidImageFile( const wxString &filename );
bool                guRemoveDir( const wxString &path );
void                GetMediaViewerTracks( const guTrackArray &sourcetracks, const wxArrayInt &sourceflags,
                                 const guMediaViewer * mediaviewer, guTrackArray &tracks, wxArrayInt &changedflags );
void                GetMediaViewerTracks( const guTrackArray &sourcetracks, const int flags,
                                 const guMediaViewer * mediaviewer, guTrackArray &tracks, wxArrayInt &changedflags );
void                GetMediaViewerTracks( const guTrackArray &sourcetracks, const guMediaViewer * mediaviewer, guTrackArray &tracks );
void                GetMediaViewersList( const guTrackArray &tracks, wxArrayPtrVoid &MediaViewerPtrs );
wxString            ExtractString( const wxString &source, const wxString &start, const wxString &end );

}

// -------------------------------------------------------------------------------- //
#endif
