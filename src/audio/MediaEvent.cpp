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
#include "MediaEvent.h"

namespace Guayadeque {

wxIMPLEMENT_DYNAMIC_CLASS( guMediaEvent, wxEvent );

wxDEFINE_EVENT( guEVT_MEDIA_LOADED,           guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_FINISHED,         guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_CHANGED_STATE,    guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_BUFFERING,        guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_LEVELINFO,        guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_TAGINFO,          guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_CHANGED_BITRATE,  guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_CHANGED_CODEC,    guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_CHANGED_POSITION, guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_CHANGED_LENGTH,   guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_FADEOUT_FINISHED, guMediaEvent );
wxDEFINE_EVENT( guEVT_MEDIA_FADEIN_STARTED,   guMediaEvent );

wxDEFINE_EVENT( guEVT_PIPELINE_CHANGED,      guMediaEvent );

wxDEFINE_EVENT( guEVT_MEDIA_ERROR,            guMediaEvent );

}

// -------------------------------------------------------------------------------- //
