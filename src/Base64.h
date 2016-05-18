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
#ifndef GUBASE64_H
#define GUBASE64_H

#include <wx/mstream.h>
#include <wx/string.h>

// -------------------------------------------------------------------------------- //
int guBase64Encode( const char * src, const size_t srclen, char * dst, const size_t dstlen );
wxString        guBase64Encode( const wxMemoryInputStream &ins );
wxString        guBase64Encode( const char * src, const size_t srclen );

wxMemoryBuffer  guBase64Decode( const wxString  &src );

#endif
