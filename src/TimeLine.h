// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
//  Bassed on the QTimeLine class from QT
// -------------------------------------------------------------------------------- //
#ifndef guTIMELINE_H
#define guTIMELINE_H

#include <wx/event.h>
#include <gst/gst.h>

// -------------------------------------------------------------------------------- //
class guTimeLine
{
  public :
    enum guState {
        NotRunning,
        Paused,
        Running
    };

    enum guDirection {
        Forward,
        Backward
    };

    enum guCurveShape {
        EaseInCurve,
        EaseOutCurve,
        EaseInOutCurve,
        LinearCurve,
        SineCurve
    };

  private :
    int                 m_StartTime;
    int                 m_StartFrame;
    int                 m_EndFrame;
    int                 m_TotalLoopCount;
    int                 m_CurrentLoopCount;

    int                 m_TimerId;

    guState             m_State;

    void                ChangeCurrentTime( const int msec );

  protected :
    int                 m_Duration;
    int                 m_UpdateInterval;
    int                 m_CurrentTime;
    guDirection         m_Direction;
    int                 m_LoopCount;
    guCurveShape        m_CurveShape;

    wxEvtHandler *      m_Parent;

  public :
    guTimeLine( int duration = 1000, wxEvtHandler * parent = NULL );
    ~guTimeLine();

    int             Duration( void ) const { return m_Duration; }
    void            SetDuration( const int duration ) { m_Duration = duration; }
    int             UpdateInterval( void ) const { return m_UpdateInterval; }
    void            SetUpdateInterval( const int interval ) { m_UpdateInterval = interval; }
    int             CurrentTime( void ) const { return m_CurrentTime; }
    void            SetCurrentTime( const int time );
    guDirection     Direction( void ) const { return m_Direction; }
    void            SetDirection( const guDirection direction );
    int             LoopCount( void ) const { return m_LoopCount; }
    void            SetLoopCount( const int count ) { m_LoopCount = count; }
    guCurveShape    CurveShape( void ) const { return m_CurveShape; }
    void            SetCurveShape( const guCurveShape shape ) { m_CurveShape = shape; }

    guState         State( void ) const { return m_State; }
    void            SetState( guState state ) { if( m_State != state ) StateChanged( state ); }

    int             StartFrame( void ) const { return m_StartFrame; }
    void            SetStartFrame( const int frame ) { m_StartFrame = frame; }
    int             EndFrame( void ) const { return m_EndFrame; }
    void            SetEndFrame( const int frame ) { m_EndFrame = frame; }
    void            SetFrameRange( const int start, const int end ) { m_StartFrame = start; m_EndFrame = end; }

    int             CurrentFrame( void ) { return FrameForTime( m_CurrentTime ); }
    float           CurrentValue( void ) { return ValueForTime( m_CurrentTime ); }

    int             FrameForTime( int msec ) { return m_StartFrame + int( ( m_EndFrame - m_StartFrame ) * ValueForTime( msec ) ); }
    virtual float   ValueForTime( int msec );

    void            Start( void );
    void            Stop( void );
    void            SetPaused( const bool paused );
    void            ToggleDirection( void ) { SetDirection( m_Direction == guTimeLine::Forward ? guTimeLine::Backward : guTimeLine::Forward ); }

    virtual void    ValueChanged( float value );
    virtual void    FrameChanged( int frame );
    virtual void    StateChanged( guState state );
    virtual void    Finished( void );

    virtual void    TimerEvent( void );
    virtual int     TimerCreate( void );
    virtual void    TimerDestroy( void ) { g_source_remove( m_TimerId ); }

};

#endif
// -------------------------------------------------------------------------------- //

