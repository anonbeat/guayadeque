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
#ifndef __MEDIAEVENT_H__
#define __MEDIAEVENT_H__

#include <wx/event.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guMediaEvent : public wxNotifyEvent
{
    wxDECLARE_DYNAMIC_CLASS( guMediaEvent );

  public:
    guMediaEvent( wxEventType commandType = wxEVT_NULL, int winid = 0 ) : wxNotifyEvent( commandType, winid ) { }
    guMediaEvent( const guMediaEvent &clone ) : wxNotifyEvent( clone ) { }

    virtual wxEvent * Clone() const
    {
        return new guMediaEvent( * this );
    }
};


//Function type(s) our events need
typedef void (wxEvtHandler::*guMediaEventFunction)(guMediaEvent&);

#define guMediaEventHandler(func)   wxEVENT_HANDLER_CAST( guMediaEventFunction, func )

wxDECLARE_EVENT( guEVT_MEDIA_LOADED,           guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_FINISHED,         guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_CHANGED_STATE,    guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_BUFFERING,        guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_LEVELINFO,        guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_TAGINFO,          guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_CHANGED_BITRATE,  guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_CHANGED_CODEC,    guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_CHANGED_POSITION, guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_CHANGED_LENGTH,   guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_FADEOUT_FINISHED, guMediaEvent );
wxDECLARE_EVENT( guEVT_MEDIA_FADEIN_STARTED,   guMediaEvent );
wxDECLARE_EVENT( guEVT_PIPELINE_CHANGED,      guMediaEvent );

wxDECLARE_EVENT( guEVT_MEDIA_ERROR,            guMediaEvent );

}

#endif
