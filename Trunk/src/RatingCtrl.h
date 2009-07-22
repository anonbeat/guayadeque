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
#ifndef RATINGCTRL_H
#define RATINGCTRL_H

#include <wx/control.h>

#define GURATING_STYLE_TINY     0
#define GURATING_STYLE_MID      1
#define GURATING_STYLE_BIG      2

#define GURATING_IMAGE_MINSIZE      8
#define GURATING_IMAGE_SEPARATION   1
#define GURATING_IMAGE_SIZE         GURATING_IMAGE_MINSIZE + GURATING_IMAGE_SEPARATION

class guRatingEvent : public wxNotifyEvent
{
public:
    guRatingEvent( wxEventType commandType = wxEVT_NULL, int winid = 0 )
        : wxNotifyEvent( commandType, winid )
    {
    };

    guRatingEvent( const guRatingEvent &clone )
            : wxNotifyEvent( clone )
    {
    };

    virtual wxEvent *Clone() const
    {
        return new guRatingEvent(*this);
    };
};

typedef void (wxEvtHandler::*guRatingEventFunction)(guRatingEvent&);

#define guRatingEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(guRatingEventFunction, &func)


#define guRATING_CHANGED_ID     1170
DECLARE_EVENT_TYPE( guEVT_RATING_CHANGED, guRATING_CHANGED_ID )
#define EVT_RATING_CHANGED(winid, fn) DECLARE_EVENT_TABLE_ENTRY( guEVT_RATING_CHANGED, winid, wxID_ANY, wxRatingEventHandler(fn), (wxObject *) NULL ),

// -------------------------------------------------------------------------------- //
class guRating : public wxControl
{
  private :
    int             m_Rating;
    int             m_Style;
    wxBitmap *      m_GreyStar;
    wxBitmap *      m_YellowStar;

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


#endif
// -------------------------------------------------------------------------------- //
