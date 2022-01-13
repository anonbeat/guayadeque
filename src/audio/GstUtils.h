// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
#ifndef __GSTUTILS_H__
#define __GSTUTILS_H__

#include <gst/gst.h>

#include "Utils.h"

namespace Guayadeque {

// debugging routines

#ifdef GU_DEBUG

// log pad info
void guLogGstPadData( const char * msg, GstPad *pad );

#else

#define guLogGstPadData(...)

#endif
// GU_DEBUG

// get actual peer of the pad avoiding proxy pads
GstPad * guGetPeerPad( GstPad * pad );

// check if any of element pads is linked to another element
bool guIsGstElementLinked( GstElement *element );

// set element state to NULL if unlinked
bool guGstStateToNullIfUnlinked( GstElement *element );

// set element state to NULL and unref
bool guGstStateToNullAndUnref( GstElement *element );


}

#endif
// -------------------------------------------------------------------------------- //

