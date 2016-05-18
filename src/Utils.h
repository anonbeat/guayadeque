// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
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
#ifndef _UTILS_H
#define _UTILS_H

#include <wx/wx.h>
#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/xml/xml.h>

#if 0
#define guLogMessage    //wxLogMessage
#define guLogWarning    //wxLogWarning
#define guLogError      //wxLogError
#else
#define guLogMessage    wxLogMessage
#define guLogWarning    wxLogWarning
#define guLogError      wxLogError
#endif

#define guRandomInit() (srand( time( NULL ) ))
#define guRandom(x) (rand() % x)

//template <typename T>
//inline const T &guMin( const T &a, const T &b ) { if( a < b ) return a; return b; }
//template <typename T>
//inline const T &guMax( const T &a, const T &b ) { if( a < b ) return b; return a; }
//template <typename T>
//inline const T &guBound( const T &min, const T &val, const T &max ) { return guMax( min, guMin( max, val ) ); }


#define guDEFAULT_BROWSER_USER_AGENT    wxT( "Mozilla/5.0 (X11; U; Linux x86_64; es-AR; rv:1.9.2.13) Gecko/20101206 Ubuntu/10.10 (maverick) Firefox/3.6.13" )

class guTrackArray;
class guMediaViewer;

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
wxString            LenToString( wxUint64 len );
wxString            SizeToString( wxFileOffset size );
wxArrayString       guSplitWords( const wxString &InputStr );
wxImage *           guGetRemoteImage( const wxString &url, wxBitmapType &imgtype );
bool                DownloadImage( const wxString &source, const wxString &target, const wxBitmapType imagetype, int maxwidth, int maxheight );
bool                DownloadImage( const wxString &source, const wxString &taget, int maxwidth = -1, int maxheight = -1 );
int                 DownloadFile( const wxString &Source, const wxString &Target );
wxString            RemoveSearchFilters( const wxString &Album );
bool                SearchCoverWords( const wxString &filename, const wxArrayString &Strings );
wxString            guURLEncode( const wxString &url );
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

// -------------------------------------------------------------------------------- //
#endif
