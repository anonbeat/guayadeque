// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
//  Bassed on the QTimeLine class from QT
// -------------------------------------------------------------------------------- //
#include "TimeLine.h"

#include "Utils.h"

// -------------------------------------------------------------------------------- //
static const float pi = 3.14159265359;
static const float halfPi = pi / 2.0;

// -------------------------------------------------------------------------------- //
static float inline guSinProgress( float value )
{
    return ::sin( ( value * pi ) - halfPi ) / 2.0 + 0.5;
}

// -------------------------------------------------------------------------------- //
static float inline guSmoothBeginEndMixFactor( float value )
{
    return guMin( guMax( ( 1.0 - value * 2.0 + 0.3 ), 0.0 ), 1.0 );
}

// -------------------------------------------------------------------------------- //
guTimeLine::guTimeLine( int duration, wxEvtHandler * parent )
{
    m_Duration = duration;
    m_Parent = parent;
    m_StartTime = 0;
    m_StartFrame = 0;
    m_EndFrame = 0;
    m_UpdateInterval = ( 1000 / 25 );
    m_TotalLoopCount = 1;
    m_CurrentLoopCount = 0;
    m_TimerId = 0;
    m_Direction = guTimeLine::Forward;
    m_CurveShape = guTimeLine::EaseInOutCurve;
    m_State = guTimeLine::NotRunning;
    m_CurrentTime = wxNOT_FOUND;
    SetCurrentTime( 0 );
}

// -------------------------------------------------------------------------------- //
guTimeLine::~guTimeLine()
{
    if( m_TimerId )
        TimerDestroy(); //g_source_remove( m_TimerId );
    m_TimerId = 0;
}

// -------------------------------------------------------------------------------- //
void guTimeLine::ChangeCurrentTime( const int msecs )
{
    //guLogMessage( wxT( "///////////////////////// %i - %i - %i" ), m_Duration, msecs, m_CurrentTime );
    if( msecs == m_CurrentTime )
        return;

    float LastValue = CurrentValue();
    int LastFrame = CurrentFrame();

    m_CurrentTime = msecs;

    while( m_CurrentTime < 0 )
        m_CurrentTime += m_Duration;

    bool Looped = ( msecs < 0 || msecs > m_Duration );

    m_CurrentTime %= ( m_Duration + 1 );

    if( Looped )
        ++m_CurrentLoopCount;

    bool Ended = false;
    if( m_TotalLoopCount && Looped && m_CurrentLoopCount >= m_TotalLoopCount )
    {
        Ended = true;
        m_CurrentTime = ( m_Direction == guTimeLine::Backward ) ? 0 : m_Duration;
    }

    if( LastValue != CurrentValue() )
    {
        ValueChanged( CurrentValue() );
        //guLogMessage( wxT( "************** Value: %0.2f" ), CurrentValue() );
    }

    if( LastFrame != CurrentFrame() )
    {
        FrameChanged( CurrentFrame() );
        //guLogMessage( wxT( "************** Frame: %i" ), CurrentFrame() );
    }

    if( Ended )
    {
        Finished();
        Stop();
    }
}

// -------------------------------------------------------------------------------- //
void guTimeLine::SetDirection( guDirection direction )
{
    m_Direction = direction;
    m_StartTime = m_CurrentTime;

    //m_Timer.Start();
}

// -------------------------------------------------------------------------------- //
void guTimeLine::SetCurrentTime( const int msec )
{
    m_StartTime = 0;
    ChangeCurrentTime( msec );
}

// -------------------------------------------------------------------------------- //
float guTimeLine::ValueForTime( int msec )
{
    msec = guMin( guMax( msec, 0 ), m_Duration );

    // Simple linear interpolation
    float Value = msec / float( m_Duration );

    switch( m_CurveShape )
    {
        case EaseInOutCurve :
            Value = guSinProgress( Value );
            break;
            // SmoothBegin blends Smooth and Linear Interpolation.
            // Progress 0 - 0.3      : Smooth only
            // Progress 0.3 - ~ 0.5  : Mix of Smooth and Linear
            // Progress ~ 0.5  - 1   : Linear only
        case EaseInCurve :
        {
            const float SinProgress = guSinProgress( Value );
            const float LinearProgress = Value;
            const float Mix = guSmoothBeginEndMixFactor( Value );
            Value = SinProgress * Mix + LinearProgress * ( 1.0 - Mix );
            break;
        }

        case EaseOutCurve:
        {
            const float SinProgress = guSinProgress( Value );
            const float LinearProgress = Value;
            const float Mix = guSmoothBeginEndMixFactor( 1.0 - Value );
            Value = SinProgress * Mix + LinearProgress * ( 1.0 - Mix );
            break;
        }

        case SineCurve:
            Value = ( ::sin( ( ( msec * pi * 2 ) / m_Duration ) - pi / 2.0 ) + 1.0 ) / 2.0;
            break;

        default:
            break;
    }

    return Value;
}

// -------------------------------------------------------------------------------- //
static bool TimerUpdated( guTimeLine * timeline )
{
    timeline->TimerEvent();
    return true;
}

// -------------------------------------------------------------------------------- //
void guTimeLine::TimerEvent( void )
{
    ChangeCurrentTime( m_Direction == Forward ? m_CurrentTime + m_UpdateInterval : m_CurrentTime - m_UpdateInterval );
}

// -------------------------------------------------------------------------------- //
int guTimeLine::TimerCreate( void )
{
    return g_timeout_add( m_UpdateInterval, GSourceFunc( TimerUpdated ), this );
}

// -------------------------------------------------------------------------------- //
void guTimeLine::Start( void )
{
    if( m_TimerId )
    {
        guLogWarning( wxT( "guTimeLine::Start: already running" ) );
        return;
    }

    if( m_CurrentTime == m_Duration && m_Direction == guTimeLine::Forward )
    {
        m_CurrentTime = 0;
    }
    else if( m_CurrentTime == 0 && m_Direction == guTimeLine::Backward )
    {
        m_CurrentTime = m_Duration;
    }

    //m_TimerId = g_timeout_add( m_UpdateInterval, GSourceFunc( TimerUpdated ), this );
    m_TimerId = TimerCreate();
    m_StartTime = m_CurrentTime;
    SetState( guTimeLine::Running );

}

// -------------------------------------------------------------------------------- //
void guTimeLine::Stop( void )
{
    if( m_TimerId )
        TimerDestroy(); //g_source_remove( m_TimerId );
    m_TimerId = 0;
    SetState( guTimeLine::NotRunning );
}

// -------------------------------------------------------------------------------- //
void guTimeLine::SetPaused( const bool paused )
{
    if( m_State == NotRunning )
    {
        guLogWarning( wxT( "guTimeLine::SetPaused: Not running" ) );
        return;
    }

    if( paused && m_State != Paused )
    {
        m_StartTime = m_CurrentTime;
        TimerDestroy(); //g_source_remove( m_TimerId );
        m_TimerId = 0;
        SetState( guTimeLine::Paused );
    }
    else if( !paused && m_State == Paused )
    {
        //m_TimerId = g_timeout_add( m_UpdateInterval, GSourceFunc( TimerUpdated ), this );
        m_TimerId = TimerCreate();
        SetState( guTimeLine::Running );
    }
}

// -------------------------------------------------------------------------------- //
void guTimeLine::ValueChanged( float value )
{
    guLogMessage( wxT( "guTimeLine::ValueChanged to %0.2f" ), value );
}


// -------------------------------------------------------------------------------- //
void guTimeLine::FrameChanged( int frame )
{
}

// -------------------------------------------------------------------------------- //
void guTimeLine::StateChanged( guState state )
{
}

// -------------------------------------------------------------------------------- //
void guTimeLine::Finished( void )
{
}

// -------------------------------------------------------------------------------- //
