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
#ifndef __RATINGCTRL_H__
#define __RATINGCTRL_H__

#include <wx/control.h>

namespace Guayadeque {

#define GURATING_STYLE_TINY     0
#define GURATING_STYLE_MID      1
#define GURATING_STYLE_BIG      2

#define GURATING_IMAGE_MINSIZE      12
#define GURATING_IMAGE_SEPARATION   1
#define GURATING_IMAGE_SIZE         GURATING_IMAGE_MINSIZE + GURATING_IMAGE_SEPARATION

class guRatingEvent : public wxNotifyEvent
{
public:
    guRatingEvent( wxEventType commandType = wxEVT_NULL, int winid = 0 )
        : wxNotifyEvent( commandType, winid )
    {
    }

    guRatingEvent( const guRatingEvent &clone )
            : wxNotifyEvent( clone )
    {
    }

    virtual wxEvent * Clone() const
    {
        return new guRatingEvent( * this );
    }

    wxDECLARE_DYNAMIC_CLASS( guRatingEvent );
};

typedef void (wxEvtHandler::*guRatingEventFunction)(guRatingEvent&);

#define guRatingEventHandler( func ) wxEVENT_HANDLER_CAST( guRatingEventFunction, func )

#define guRATING_CHANGED_ID     1170

wxDECLARE_EVENT( guEVT_RATING_CHANGED, guRatingEvent );
#define EVT_RATING_CHANGED( winid, fn ) wxDECLARE_EVENT_TABLE_ENTRY( guEVT_RATING_CHANGED, winid, wxID_ANY, guRatingEventHandler( fn ), NULL ),

// -------------------------------------------------------------------------------- //
class guRating : public wxControl
{
  private :
    int             m_Rating;
    int             m_Style;
    wxBitmap *      m_NormalStar;
    wxBitmap *      m_SelectStar;

    DECLARE_EVENT_TABLE()

  protected :
    virtual wxSize  DoGetBestSize() const;
    void            OnPaint( wxPaintEvent &event );
    void            OnMouseEvents( wxMouseEvent &event );

  public :
    guRating( wxWindow * parent, const int style );
    ~guRating();
    void            SetRating( const int rating );
    int             GetRating( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
