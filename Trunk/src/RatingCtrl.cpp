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
#include "RatingCtrl.h"

#include "Images.h"
#include "Utils.h"

#include <wx/dcclient.h>

DEFINE_EVENT_TYPE( guEVT_RATING_CHANGED )

BEGIN_EVENT_TABLE( guRating, wxControl )
    EVT_PAINT( guRating::OnPaint)
    EVT_MOUSE_EVENTS( guRating::OnMouseEvents )
//    EVT_MOUSEWHEEL( guPlayerPanel::OnRatingMouseEvents )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guRating::guRating( wxWindow * parent, const int style ) : wxControl( parent, wxID_ANY )
{
    m_Rating = wxNOT_FOUND;
    m_Style = style;

    m_GreyStar   = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_grey_star_big + style ) ) );
    m_YellowStar = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_yellow_star_big + style ) ) );
}

// -------------------------------------------------------------------------------- //
guRating::~guRating()
{
    if( m_GreyStar )
        delete m_GreyStar;
    if( m_YellowStar )
        delete m_YellowStar;
}

// -------------------------------------------------------------------------------- //
void guRating::SetRating( const int rating )
{
    m_Rating = rating;
    guLogMessage( wxT( "Rating Set to %i" ), rating );
    Refresh();
    Update();
}

// -------------------------------------------------------------------------------- //
int guRating::GetRating( void )
{
    return m_Rating;
}

// -------------------------------------------------------------------------------- //
wxSize guRating::DoGetBestSize( void ) const
{
    wxSize RetVal;
    RetVal.x = 5 * ( ( m_Style * 2 ) + 12 );
    RetVal.y = ( m_Style * 2 ) + 12;
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guRating::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    dc.SetBackgroundMode( wxTRANSPARENT );
    int x;
    //int y = 2;
    for( x = 0; x < 5; x++ )
    {
       dc.DrawBitmap( ( x >= m_Rating ) ? * m_GreyStar : * m_YellowStar,
                      2 + ( 10 * x ), 2, true );
    }
}

// -------------------------------------------------------------------------------- //
void guRating::OnMouseEvents( wxMouseEvent &event )
{
    int SavedRating = m_Rating;
    if( event.RightDown() )
    {
        m_Rating = 0;
    }
    else if( event.LeftIsDown() )
    {
        m_Rating = ( wxMax( 0, event.m_x - 2 ) / 10 ) + 1;
        //guLogMessage( wxT( "Clicked %i %i" ), event.m_x, m_Rating );
    }
    if( SavedRating != m_Rating )
    {
        Refresh();
        Update();
        guRatingEvent evt( guEVT_RATING_CHANGED );
        //evt.SetClientObject( this );
        evt.SetInt( m_Rating );
        wxPostEvent( this, evt );
    }
}

// -------------------------------------------------------------------------------- //
