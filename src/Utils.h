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
#ifndef _UTILS_H
#define _UTILS_H
#include <wx/wx.h>

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

// -------------------------------------------------------------------------------- //
wxString LenToString( int Len );
wxArrayString guSplitWords( const wxString &InputStr );
wxImage * guGetRemoteImage( const wxString &url, int &imgtype );
bool DownloadImage( const wxString &source, const wxString &taget, int maxwidth = -1, int maxheight = -1 );
int DownloadFile( const wxString &Source, const wxString &Target );
wxString RemoveSearchFilters( const wxString &Album );
bool SearchCoverWords( const wxString &FileName, const wxArrayString &Strings );
wxString guURLEncode( const wxString &Source );
int guWebExecute( const wxString &Url );
int guExecute( const wxString &Command );
unsigned int guGetFileSize( const wxString &FileName );
wxString GetUrlContent( const wxString &url, const wxString &referer = wxEmptyString, bool encoding = false );

// -------------------------------------------------------------------------------- //
#endif
