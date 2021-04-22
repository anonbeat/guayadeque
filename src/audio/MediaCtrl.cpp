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
#include "MediaCtrl.h"

#include "Config.h"
#include "FileRenamer.h" // NormalizeField
#include "LevelInfo.h"
#include "PlayerPanel.h"
#include "RadioTagInfo.h"
#include "Settings.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/uri.h>

namespace Guayadeque {

#define guFADERPLAYBIN_FAST_FADER_TIME          (1000)
#define guFADERPLAYBIN_SHORT_FADER_TIME         (200)

#define GST_TO_WXSTRING( str )  ( wxString( str, wxConvUTF8 ) )

#ifdef GU_DEBUG
#define guSHOW_DUMPFADERPLAYBINS     1
#endif

#ifdef guSHOW_DUMPFADERPLAYBINS
// -------------------------------------------------------------------------------- //
static void DumpFaderPlayBins( const guFaderPlayBinArray &playbins, guFaderPlaybin * current )
{
    guLogDebug( wxT( "CurrentPlayBin: %li" ), current ? current->GetId() : 0l );
    if( !playbins.Count() )
    {
        guLogMessage( wxT( "The faderplaybins list is empty" ) );
        return;
    }

    guLogDebug( wxT( " * * * * * * * * * * current stream list * * * * * * * * * *" ) );
    int Index;
    int Count = playbins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guFaderPlaybin * FaderPlayBin = playbins[ Index ];

        wxString StateName;

        switch( FaderPlayBin->GetState() )
        {
            case guFADERPLAYBIN_STATE_WAITING :	 	        StateName = wxT( "waiting" );		    break;
            case guFADERPLAYBIN_STATE_PLAYING :	 	        StateName = wxT( "playing" );		    break;
            case guFADERPLAYBIN_STATE_PAUSED :	 	        StateName = wxT( "paused" );		    break;
            case guFADERPLAYBIN_STATE_STOPPED :	 	        StateName = wxT( "stopped" );		    break;

            case guFADERPLAYBIN_STATE_FADEIN :   	        StateName = wxT( "fading in" ); 	    break;
            case guFADERPLAYBIN_STATE_WAITING_EOS : 	    StateName = wxT( "waiting for EOS" );   break;
            case guFADERPLAYBIN_STATE_FADEOUT : 	        StateName = wxT( "fading out" ); 	    break;
            case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :       StateName = wxT( "fading->paused" );    break;
            case guFADERPLAYBIN_STATE_FADEOUT_STOP :        StateName = wxT( "fading->stopped" );   break;

            case guFADERPLAYBIN_STATE_PENDING_REMOVE:	    StateName = wxT( "pending remove" );    break;
            case guFADERPLAYBIN_STATE_ERROR :	            StateName = wxT( "error" );             break;
            default :                                       StateName = wxT( "other" );             break;
        }

        //if( !FaderPlayBin->Uri().IsEmpty() )
        {
            guLogDebug( wxT( "[%li] '%s'" ), FaderPlayBin->GetId(), StateName.c_str() );
        }
    }
    guLogDebug( wxT( " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " ) );
}
#endif

// -------------------------------------------------------------------------------- //
extern "C" {

// -------------------------------------------------------------------------------- //
// idle handler used to clean up finished streams
static gboolean cleanup_mediactrl( guMediaCtrl * player )
{
    player->DoCleanUp();
	return false;
}

// -------------------------------------------------------------------------------- //
static bool tick_timeout( guMediaCtrl * mediactrl )
{
    mediactrl->UpdatePosition();
	return true;
}

}



// -------------------------------------------------------------------------------- //
guMediaCtrl::guMediaCtrl( guPlayerPanel * playerpanel )
{
    m_PlayerPanel = playerpanel;
    m_CurrentPlayBin = NULL;
    m_CleanUpId = 0;
    m_IsRecording = false;
    m_LastPosition = 0;

	// now that the sink is running, start polling for playing position.
	// might want to replace this with a complicated set of pad probes
	// to avoid polling, but duration queries on the sink are better
	// as they account for internal buffering etc.  maybe there's a way
	// to account for that in a pad probe callback on the sink's sink pad?
    gint ms_period = 1000 / 4;
    m_TickTimeoutId = g_timeout_add( ms_period, GSourceFunc( tick_timeout ), this );

    if( Init() )
    {
        guConfig * Config = guConfig::Get();
        m_ForceGapless = Config->ReadBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, false, CONFIG_PATH_CROSSFADER );
        UpdatedConfig();
    }
}

// -------------------------------------------------------------------------------- //
guMediaCtrl::~guMediaCtrl()
{
    if( m_TickTimeoutId )
    {
        g_source_remove( m_TickTimeoutId );
        m_TickTimeoutId = 0;
    }

    if( m_CleanUpId )
    {
        g_source_remove( m_CleanUpId );
        m_CleanUpId = 0;
    }

    CleanUp();

    gst_deinit();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::CleanUp( void )
{
    Lock();
    int Index;
    int Count = m_FaderPlayBins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        delete m_FaderPlayBins[ Index ];
    }

    Unlock();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::SendEvent( guMediaEvent &event )
{
    wxPostEvent( m_PlayerPanel, event );
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::UpdatePosition( void )
{
	gint64 Pos = -1;
	gint64 Duration = -1;

	Lock();
	if( m_CurrentPlayBin )
	{
	    m_CurrentPlayBin->Lock();

        Pos = m_CurrentPlayBin->Position();

        Pos /= GST_MSECOND;

		Duration = -1;
        gst_element_query_duration( m_CurrentPlayBin->m_OutputSink, GST_FORMAT_TIME, &Duration );

        if( Pos != m_LastPosition )
        {
            if( !m_CurrentPlayBin->AboutToFinishPending() || Pos < m_LastPosition )
            {
                //guLogMessage( wxT( "Sent UpdatePositon event for %i" ), m_CurrentPlayBin->GetId() );
                guMediaEvent event( guEVT_MEDIA_CHANGED_POSITION );
                event.SetInt( Pos );
                event.SetExtraLong( Duration / GST_MSECOND );
                event.SetClientData( ( void * ) m_CurrentPlayBin->GetId() );
                SendEvent( event );
                if( m_CurrentPlayBin->AboutToFinishPending() )
                    m_CurrentPlayBin->ResetAboutToFinishPending();
            }
            m_LastPosition = Pos;
        }
        //guLogDebug( wxT( "Current fade volume: %0.2f" ), m_CurrentPlayBin->GetFaderVolume() );
        m_CurrentPlayBin->Unlock();
	}
	Unlock();
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::RemovePlayBin( guFaderPlaybin * playbin )
{
    guLogDebug( wxT( "guMediaCtrl::RemovePlayBin (%li)" ), playbin->m_Id );
    bool RetVal = false;

    Lock();
    if( m_CurrentPlayBin == playbin )
        m_CurrentPlayBin = NULL;

    if( m_FaderPlayBins.Index( playbin ) != wxNOT_FOUND )
    {
        m_FaderPlayBins.Remove( playbin );
        RetVal = true;
    }
    Unlock();
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxFileOffset guMediaCtrl::Position( void )
{
    wxFileOffset Pos = 0;
    Lock();
    if( m_CurrentPlayBin )
    {
        Pos = m_CurrentPlayBin->Position();
    }
    Unlock();
    return Pos;
}

// -------------------------------------------------------------------------------- //
wxFileOffset guMediaCtrl::Length( void )
{
    Lock();
    wxFileOffset Len = 0;
    if( m_CurrentPlayBin )
    {
        Len = m_CurrentPlayBin->Length();
    }
    Unlock();
    return Len;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetVolume( double volume )
{
    guLogDebug( wxT( "MediaCtrl::SetVolume( %0.5f )" ), volume );
    m_Volume = volume;
    Lock();
    int Index;
    int Count = m_FaderPlayBins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_FaderPlayBins[ Index ]->SetVolume( volume );
    }
    Unlock();
    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::SetEqualizerBand( const int band, const int value )
{
    m_EqBands[ band ] = value;
    Lock();
    int Index;
    int Count = m_FaderPlayBins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_FaderPlayBins[ Index ]->SetEqualizerBand( band, value );
    }
    Unlock();
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetEqualizer( const wxArrayInt &eqbands )
{
    m_EqBands = eqbands;
    Lock();
    int Index;
    int Count = m_FaderPlayBins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_FaderPlayBins[ Index ]->SetEqualizer( eqbands );
    }
    Unlock();
    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ResetEqualizer( void )
{
    int Index;
    int Count = m_EqBands.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_EqBands[ Index ] = 0;
    }

    Lock();
    Count = m_FaderPlayBins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_FaderPlayBins[ Index ]->SetEqualizer( m_EqBands );
    }
    Unlock();
}

// -------------------------------------------------------------------------------- //
guMediaState guMediaCtrl::GetState( void )
{
    guMediaState State = guMEDIASTATE_STOPPED;
    Lock();
    if( m_CurrentPlayBin )
    {
        guLogDebug( wxT( "guMediaCtrl::GetState %i" ), m_CurrentPlayBin->GetState() );

        switch( m_CurrentPlayBin->GetState() )
        {
            case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
            case guFADERPLAYBIN_STATE_PAUSED :
                State = guMEDIASTATE_PAUSED;
                break;

            case guFADERPLAYBIN_STATE_FADEOUT :
            case guFADERPLAYBIN_STATE_FADEOUT_STOP :
            case guFADERPLAYBIN_STATE_FADEIN :
            case guFADERPLAYBIN_STATE_PLAYING :
                State = guMEDIASTATE_PLAYING;
                break;

            case guFADERPLAYBIN_STATE_ERROR :
                State = guMEDIASTATE_ERROR;



            default :
                break;
        }
    }
    Unlock();
    return State;
}

// -------------------------------------------------------------------------------- //
long guMediaCtrl::Load( const wxString &uri, guFADERPLAYBIN_PLAYTYPE playtype, const int startpos )
{
    guLogDebug( wxT( "guMediaCtrl::Load %i" ), playtype );

    guFaderPlaybin * FaderPlaybin;
    long Result = 0;

#ifdef guSHOW_DUMPFADERPLAYBINS
    Lock();
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
    Unlock();
#endif

    if( IsBuffering() )
    {
        playtype = guFADERPLAYBIN_PLAYTYPE_REPLACE;
    }

    switch( playtype )
    {
        case guFADERPLAYBIN_PLAYTYPE_AFTER_EOS :
        {
            guLogDebug( wxT( "guMediaCtrl::Load guFADERPLAYBIN_PLAYTYPE_AFTER_EOS..." ) );
            Lock();
            if( m_CurrentPlayBin )
            {
                m_CurrentPlayBin->SetNextUri( uri );
                m_CurrentPlayBin->SetNextId( wxGetLocalTime() );
                Result = m_CurrentPlayBin->NextId();
                Unlock();
                return Result;
            }
            Unlock();
            break;
        }

        case guFADERPLAYBIN_PLAYTYPE_REPLACE :
        {
            guLogDebug( wxT( "guMediaCtrl::Load Replacing the current track in the current playbin..." ) );
            Lock();
            if( m_CurrentPlayBin )
            {
                guLogDebug( wxT( "guMediaCtrl::Load Id: %li  State: %i  Error: %i" ), m_CurrentPlayBin->GetId(), m_CurrentPlayBin->GetState(), m_CurrentPlayBin->ErrorCode() );
                //if( m_CurrentPlayBin->m_State == guFADERPLAYBIN_STATE_ERROR )
                if( !m_CurrentPlayBin->IsOk() || m_IsRecording )
                {
                    guLogDebug( wxT( "guMediaCtrl::Load The current playbin has error or recording...Removing it" ) );
                    m_CurrentPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                    m_CurrentPlayBin->DisableRecordAndStop();
                    ScheduleCleanUp();
                }
                else
                {
                    guLogDebug( wxT( "guMediaCtrl::Load The current playbin has no error...Replacing it" ) );
                    m_CurrentPlayBin->m_PlayType = guFADERPLAYBIN_PLAYTYPE_REPLACE;
                    m_CurrentPlayBin->m_State = guFADERPLAYBIN_STATE_WAITING;
                    m_CurrentPlayBin->Load( uri, true, startpos );
                    m_CurrentPlayBin->SetBuffering( false );
                    m_CurrentPlayBin->SetFaderVolume( 1.0 );
                    Result = m_CurrentPlayBin->GetId();
                    Unlock();
                    return Result;
                }
            }
            Unlock();
            break;
        }

        default :
            break;
    }

    FaderPlaybin = new guFaderPlaybin( this, uri, playtype, startpos );
    if( FaderPlaybin )
    {
        if( FaderPlaybin->IsOk() )
        {
            if( gst_element_set_state( FaderPlaybin->Playbin(), GST_STATE_PAUSED ) != GST_STATE_CHANGE_FAILURE )
            {
                Lock();
                m_FaderPlayBins.Insert( FaderPlaybin, 0 );
                Unlock();

                guMediaEvent event( guEVT_MEDIA_LOADED );
                event.SetInt( true );
                SendEvent( event );

                return FaderPlaybin->GetId();
            }
//            else
//            {
//                RemovePlayBin( FaderPlaybin );
//            }
        }
        delete FaderPlaybin;
    }
    return Result;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Play( void )
{
    guLogDebug( wxT( "wxMediaCtrl::Play" ) );
    Lock();

    if( !m_FaderPlayBins.Count() )
    {
        Unlock();
        return false;
    }

#ifdef guSHOW_DUMPFADERPLAYBINS
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
#endif
    guFaderPlaybin * FaderPlaybin = m_FaderPlayBins[ 0 ];
    guLogDebug( wxT( "CurrentFaderPlayBin State %i" ), FaderPlaybin->GetState() );

    Unlock();

    switch( FaderPlaybin->GetState() )
    {
        case guFADERPLAYBIN_STATE_FADEIN :
        case guFADERPLAYBIN_STATE_PLAYING :
        case guFADERPLAYBIN_STATE_FADEOUT :
        case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
        case guFADERPLAYBIN_STATE_FADEOUT_STOP :
        {
            FaderPlaybin->m_State = guFADERPLAYBIN_STATE_PLAYING;
            // Send event of change state to Playing
            guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
            event.SetInt( GST_STATE_PLAYING );
            SendEvent( event );
            break;
        }

        case guFADERPLAYBIN_STATE_PAUSED :
        {
            Lock();
            if( m_CurrentPlayBin != FaderPlaybin )
            {
                m_CurrentPlayBin = FaderPlaybin;
            }
            Unlock();
            FaderPlaybin->StartFade( 0.0, 1.0, ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_FAST_FADER_TIME : guFADERPLAYBIN_SHORT_FADER_TIME );
            FaderPlaybin->m_State = guFADERPLAYBIN_STATE_FADEIN;
            FaderPlaybin->Play();

            guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
            event.SetInt( GST_STATE_PLAYING );
            SendEvent( event );
            break;
        }

        case guFADERPLAYBIN_STATE_WAITING :
        case guFADERPLAYBIN_STATE_WAITING_EOS :
            return FaderPlaybin->StartPlay();

        default :
            break;
    }

    return false;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Pause( void )
{
    guLogDebug( wxT( "***************************************************************************** guMediaCtrl::Pause" ) );

    guFaderPlaybin * FaderPlayBin = NULL;
	wxArrayPtrVoid  ToFade;

	bool            Done = FALSE;
	double          FadeOutStart = 1.0f;
	gint64          FadeOutTime;

	Lock();
	int Index;
	int Count = m_FaderPlayBins.Count();
	if( !Count )
	{
	    Unlock();
        return true;
	}
#ifdef guSHOW_DUMPFADERPLAYBINS
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
#endif

	for( Index = 0; Index < Count; Index++ )
	{
        FaderPlayBin = m_FaderPlayBins[ Index ];

		switch( FaderPlayBin->m_State )
		{
            case guFADERPLAYBIN_STATE_WAITING :
            case guFADERPLAYBIN_STATE_WAITING_EOS :
                FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PAUSED;
                //guLogDebug( wxT( "stream %s is not yet playing, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;

            case guFADERPLAYBIN_STATE_PAUSED :
            case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
            case guFADERPLAYBIN_STATE_FADEOUT_STOP :
                //guLogDebug( wxT( "stream %s is already paused" ), FaderPlayBin->m_Uri.c_str() );
                Done = true;
                break;

            case guFADERPLAYBIN_STATE_FADEIN :
            case guFADERPLAYBIN_STATE_FADEOUT :
            case guFADERPLAYBIN_STATE_PLAYING :
                //guLogDebug( wxT( "pausing stream %s -> FADING_OUT_PAUSED" ), FaderPlayBin->m_Uri.c_str() );
                ToFade.Insert( FaderPlayBin, 0 );
                Done = true;
                break;

            case guFADERPLAYBIN_STATE_PENDING_REMOVE:
                //guLogDebug( wxT( "stream %s is done, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;
		}

//		if( Done )
//			break;
	}

    Unlock();

	Count = ToFade.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        FaderPlayBin = ( guFaderPlaybin * ) ToFade[ Index ];
        FadeOutTime = ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_FAST_FADER_TIME : guFADERPLAYBIN_SHORT_FADER_TIME;
		switch( FaderPlayBin->m_State )
		{
            case guFADERPLAYBIN_STATE_FADEOUT :
            case guFADERPLAYBIN_STATE_FADEIN :
                FadeOutStart = FaderPlayBin->GetFaderVolume();
                FadeOutTime = ( gint64 ) ( ( ( double ) guFADERPLAYBIN_FAST_FADER_TIME ) * FadeOutStart );
                //guLogDebug( wxT( "============== Fading Out a Fading In playbin =================" ) );

            case guFADERPLAYBIN_STATE_PLAYING:
            {
                FaderPlayBin->m_State = guFADERPLAYBIN_STATE_FADEOUT_PAUSE;
                FaderPlayBin->StartFade( FadeOutStart, 0.0f, FadeOutTime );
            }

            default:
                // shouldn't happen, but ignore it if it does
                break;
		}
	}

	if( !Done )
		guLogMessage( wxT( "couldn't find a stream to pause" ) );

#ifdef guSHOW_DUMPFADERPLAYBINS
    Lock();
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
    Unlock();
#endif

    return Done;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Stop( void )
{
    guLogDebug( wxT( "***************************************************************************** guMediaCtrl::Stop" ) );

    guFaderPlaybin * FaderPlayBin = NULL;
	wxArrayPtrVoid  ToFade;

	bool            Done = FALSE;
	double          FadeOutStart = 1.0f;
	gint64          FadeOutTime;

	Lock();
	int Index;
	int Count = m_FaderPlayBins.Count();
	if( !Count )
	{
	    Unlock();
        return true;
	}

#ifdef guSHOW_DUMPFADERPLAYBINS
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
#endif

	for( Index = 0; Index < Count; Index++ )
	{
        FaderPlayBin = m_FaderPlayBins[ Index ];

		switch( FaderPlayBin->m_State )
		{
            case guFADERPLAYBIN_STATE_WAITING :
            case guFADERPLAYBIN_STATE_WAITING_EOS :
                FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                Unlock();
                ScheduleCleanUp();
                Lock();
                //guLogDebug( wxT( "stream %s is not yet playing, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;

            case guFADERPLAYBIN_STATE_PAUSED :
            case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
            case guFADERPLAYBIN_STATE_FADEOUT_STOP :
                //guLogDebug( wxT( "stream %s is already paused" ), FaderPlayBin->m_Uri.c_str() );
                FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                Unlock();
                ScheduleCleanUp();
                Lock();
                break;

            case guFADERPLAYBIN_STATE_FADEIN :
            case guFADERPLAYBIN_STATE_FADEOUT :
            case guFADERPLAYBIN_STATE_PLAYING :
                //guLogDebug( wxT( "pausing stream %s -> FADING_OUT_PAUSED" ), FaderPlayBin->m_Uri.c_str() );
                ToFade.Insert( FaderPlayBin, 0 );
                Done = true;
                break;

            case guFADERPLAYBIN_STATE_PENDING_REMOVE:
                //guLogDebug( wxT( "stream %s is done, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;
		}

//		if( Done )
//			break;
	}

    Unlock();

	Count = ToFade.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        FaderPlayBin = ( guFaderPlaybin * ) ToFade[ Index ];
        FadeOutTime = ( !m_ForceGapless && m_FadeOutTime ) ? guFADERPLAYBIN_FAST_FADER_TIME : guFADERPLAYBIN_SHORT_FADER_TIME;
		switch( FaderPlayBin->m_State )
		{
            case guFADERPLAYBIN_STATE_FADEOUT :
            case guFADERPLAYBIN_STATE_FADEIN :
                FadeOutStart = FaderPlayBin->GetFaderVolume();
                FadeOutTime = ( gint64 ) ( ( ( double ) guFADERPLAYBIN_FAST_FADER_TIME ) * FadeOutStart );
                //guLogDebug( wxT( "============== Fading Out a Fading In playbin =================" ) );

            case guFADERPLAYBIN_STATE_PLAYING:
            {
                if( IsBuffering() )
                {
                    FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                    ScheduleCleanUp();

                    guMediaEvent BufEvent( guEVT_MEDIA_BUFFERING );
                    BufEvent.SetInt( 100 );
                    SendEvent( BufEvent );

                    guMediaEvent event( guEVT_MEDIA_FADEOUT_FINISHED );
                    SendEvent( event );
                    FaderPlayBin->SetBuffering( false );
                }
                else
                {
                    FaderPlayBin->m_State = guFADERPLAYBIN_STATE_FADEOUT_STOP;
                    FaderPlayBin->StartFade( FadeOutStart, 0.0f, FadeOutTime );
                }
            }

            default:
                // shouldn't happen, but ignore it if it does
                break;
		}
	}

	if( !Done )
	{
        guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
        event.SetInt( GST_STATE_READY );
        SendEvent( event );

		guLogMessage( wxT( "couldn't find a stream to pause" ) );
	}

#ifdef guSHOW_DUMPFADERPLAYBINS
    Lock();
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
    Unlock();
#endif

    return Done;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Seek( wxFileOffset where, const bool accurate )
{
    guLogDebug( wxT( "guMediaCtrl::Seek( %lli )" ), where );
    bool Result = false;
    Lock();
    if( m_CurrentPlayBin )
    {
        Result = m_CurrentPlayBin->Seek( where, accurate );
    }
    Unlock();
    return Result;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::UpdatedConfig( void )
{
    guConfig * Config  = ( guConfig * ) guConfig::Get();
    //m_ForceGapless          = Config->ReadBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, false, CONFIG_PATH_CROSSFADER );
    m_FadeOutTime           = Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEOUT_TIME, 50, CONFIG_PATH_CROSSFADER ) * 100;
    m_FadeInTime            = Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEIN_TIME, 10, CONFIG_PATH_CROSSFADER ) * 100;
    m_FadeInVolStart        = double( Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEIN_VOL_START, 80, CONFIG_PATH_CROSSFADER ) ) / 100.0;
    m_FadeInVolTriger       = double( Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEIN_VOL_TRIGER, 50, CONFIG_PATH_CROSSFADER ) ) / 100.0;
    m_BufferSize            = Config->ReadNum( CONFIG_KEY_GENERAL_BUFFER_SIZE, 64, CONFIG_PATH_GENERAL );
    m_EnableEq              = Config->ReadBool( CONFIG_KEY_GENERAL_EQ_ENABLED, true, CONFIG_PATH_GENERAL );
    m_EnableVolCtls         = Config->ReadBool( CONFIG_KEY_GENERAL_VOLUME_ENABLED, true, CONFIG_PATH_GENERAL );
    m_ReplayGainMode        = Config->ReadNum( CONFIG_KEY_GENERAL_REPLAY_GAIN_MODE, 0, CONFIG_PATH_GENERAL );
    m_ReplayGainPreAmp      = double( Config->ReadNum( CONFIG_KEY_GENERAL_REPLAY_GAIN_PREAMP, 6, CONFIG_PATH_GENERAL ) );
    m_OutputDevice          = Config->ReadNum( CONFIG_KEY_PLAYBACK_OUTPUT_DEVICE, guOUTPUT_DEVICE_AUTOMATIC, CONFIG_PATH_PLAYBACK );
    m_OutputDeviceName      = Config->ReadStr( CONFIG_KEY_PLAYBACK_OUTPUT_DEVICE_NAME, wxEmptyString, CONFIG_PATH_PLAYBACK );
    m_ProxyEnabled          = Config->ReadBool( CONFIG_KEY_PROXY_ENABLED, false, CONFIG_PATH_PROXY );
    m_ProxyHost             = Config->ReadStr( CONFIG_KEY_PROXY_HOSTNAME, wxEmptyString, CONFIG_PATH_PROXY );
    m_ProxyPort             = Config->ReadNum( CONFIG_KEY_PROXY_PORT, 8080, CONFIG_PATH_PROXY );
    m_ProxyUser             = Config->ReadStr( CONFIG_KEY_PROXY_USERNAME, wxEmptyString, CONFIG_PATH_PROXY );
    m_ProxyPass             = Config->ReadStr( CONFIG_KEY_PROXY_PASSWORD, wxEmptyString, CONFIG_PATH_PROXY );

    m_ProxyServer = wxString::Format( wxT( "%s:%d" ), m_ProxyHost, m_ProxyPort );

    ReconfigureRG();
    guMediaEvent e( guEVT_PIPELINE_CHANGED );
    e.SetClientData( NULL );
    SendEvent( e ); // returns in ::RefreshPlaybackItems


}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ScheduleCleanUp( void )
{
    //Lock();
    if( !m_CleanUpId )
    {
        m_CleanUpId = g_timeout_add( 4000, GSourceFunc( cleanup_mediactrl ), this );
    }
    //Unlock();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::FadeInStart( void )
{
    guLogDebug( wxT( "guMediaCtrl::FadeInStart" ) );

    Lock();
#ifdef guSHOW_DUMPFADERPLAYBINS
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
#endif
    int Index;
    int Count = m_FaderPlayBins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guFaderPlaybin * NextPlayBin = m_FaderPlayBins[ Index ];
        if( NextPlayBin->m_State == guFADERPLAYBIN_STATE_WAITING )
        {
            guLogDebug( wxT( "got fade-in-start for stream %s -> FADE_IN" ), NextPlayBin->m_Uri.c_str() );
            NextPlayBin->StartFade( m_FadeInVolStart, 1.0, m_FadeInTime );
            NextPlayBin->m_State = guFADERPLAYBIN_STATE_FADEIN;
            NextPlayBin->Play();

            guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
            event.SetInt( GST_STATE_PLAYING );
            SendEvent( event );

            guMediaEvent event2( guEVT_MEDIA_FADEIN_STARTED );
            event2.SetExtraLong( NextPlayBin->GetId() );
            SendEvent( event2 );

            m_CurrentPlayBin = NextPlayBin;
            break;
        }
    }
    Unlock();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::FadeOutDone( guFaderPlaybin * faderplaybin )
{
    guLogDebug( wxT( "guMediaCtrl:FadeOutDone" ) );
    switch( faderplaybin->m_State )
    {
        case guFADERPLAYBIN_STATE_FADEOUT :
        {
            faderplaybin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
            ScheduleCleanUp();

            guMediaEvent event( guEVT_MEDIA_FADEOUT_FINISHED );
            event.SetExtraLong( faderplaybin->GetId() );
            SendEvent( event );
            break;
        }

        case guFADERPLAYBIN_STATE_FADEOUT_PAUSE:
        {
            // try to seek back a bit to account for the fade
            gint64 Pos = Position();

            if( Pos != -1 )
            {
                faderplaybin->m_PausePosition = Pos > guFADERPLAYBIN_FAST_FADER_TIME ? Pos - guFADERPLAYBIN_FAST_FADER_TIME : guFADERPLAYBIN_SHORT_FADER_TIME;
                guLogDebug( wxT( "got fade-out-done for stream %s -> SEEKING_PAUSED [%" G_GINT64_FORMAT "]" ), faderplaybin->m_Uri.c_str(), faderplaybin->m_Id );
            }
            else
            {
                faderplaybin->m_PausePosition = wxNOT_FOUND;
                guLogDebug( wxT( "got fade-out-done for stream %s -> PAUSED (position query failed)" ), faderplaybin->m_Uri.c_str() );
            }
            faderplaybin->m_State = guFADERPLAYBIN_STATE_PAUSED;
            faderplaybin->Pause();

            guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
            event.SetInt( GST_STATE_PAUSED );
            SendEvent( event );
            break;
        }

        case guFADERPLAYBIN_STATE_FADEOUT_STOP :
        {
            faderplaybin->m_PausePosition = 0;
            faderplaybin->m_State = guFADERPLAYBIN_STATE_STOPPED;
            faderplaybin->Stop();

            guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
            event.SetInt( GST_STATE_READY );
            SendEvent( event );
            break;
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::EnableRecord( const wxString &recfile, const int format, const int quality )
{
    guLogDebug( wxT( "guMediaCtrl::EnableRecord" ) );
    Lock();
    m_IsRecording = ( m_CurrentPlayBin && m_CurrentPlayBin->EnableRecord( recfile, format, quality ) );
    Unlock();
    return  m_IsRecording;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::DisableRecord( void )
{
    guLogDebug( wxT( "guMediaCtrl::DisableRecord" ) );
    m_IsRecording = false;
    Lock();
    if( m_CurrentPlayBin )
    {
        m_CurrentPlayBin->DisableRecord();
    }
    Unlock();
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetRecordFileName( const wxString &filename )
{
    guLogDebug( "guMediaCtrl::SetRecordFileName  '%s'", filename );

    Lock();
    bool Result = m_CurrentPlayBin && m_CurrentPlayBin->SetRecordFileName( filename );
    Unlock();
    return Result;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::DoCleanUp( void )
{
    guLogDebug( wxT( "guMediaCtrl::DoCleanUp" ) );
	wxArrayPtrVoid ToDelete;

    Lock();
#ifdef guSHOW_DUMPFADERPLAYBINS
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
#endif


	m_CleanUpId = 0;

	int Index;
	int Count = m_FaderPlayBins.Count();
	for( Index = Count - 1; Index >= 0; Index-- )
	{
        guFaderPlaybin * FaderPlaybin = m_FaderPlayBins[ Index ];
		if( ( FaderPlaybin->m_State == guFADERPLAYBIN_STATE_PENDING_REMOVE ) ||
		    ( FaderPlaybin->m_State == guFADERPLAYBIN_STATE_ERROR ) )
		{
			ToDelete.Add( FaderPlaybin );
			m_FaderPlayBins.RemoveAt( Index );
			if( FaderPlaybin == m_CurrentPlayBin )
			{
			    m_CurrentPlayBin = NULL;
			}
		}
	}
	if( !m_FaderPlayBins.Count() )
	{
        guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
        event.SetInt( GST_STATE_READY );
        SendEvent( event );
	}

#ifdef guSHOW_DUMPFADERPLAYBINS
    DumpFaderPlayBins( m_FaderPlayBins, m_CurrentPlayBin );
#endif
	Unlock();

	Count = ToDelete.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        guLogDebug( wxT( "Free stream %li" ), ( ( guFaderPlaybin * ) ToDelete[ Index ] )->GetId() );
        delete ( ( guFaderPlaybin * ) ToDelete[ Index ] );
	}
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Init()
{
    // Code taken from wxMediaCtrl class
    //Convert arguments to unicode if enabled
#if wxUSE_UNICODE
    int i;
    char * * argvGST = new char * [ wxTheApp->argc + 1 ];
    for( i = 0; i < wxTheApp->argc; i++ )
    {
        argvGST[ i ] = wxStrdup( wxTheApp->argv[ i ].char_str() );
    }

    argvGST[ wxTheApp->argc ] = NULL;

    int argcGST = wxTheApp->argc;
#else
#define argcGST wxTheApp->argc
#define argvGST wxTheApp->argv
#endif

    //Really init gstreamer
    gboolean bInited;
    GError * error = NULL;
    bInited = gst_init_check( &argcGST, &argvGST, &error );

    // Cleanup arguments for unicode case
#if wxUSE_UNICODE
    for( i = 0; i < argcGST; i++ )
    {
        free( argvGST[ i ] );
    }

    delete [] argvGST;
#endif

    return bInited;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::ProxyEnabled() const
{
    return m_ProxyEnabled;
}

// -------------------------------------------------------------------------------- //
wxString guMediaCtrl::ProxyHost() const
{
    return m_ProxyHost;
}

// -------------------------------------------------------------------------------- //
int guMediaCtrl::ProxyPort() const
{
    return m_ProxyPort;
}

// -------------------------------------------------------------------------------- //
wxString guMediaCtrl::ProxyUser() const
{
    return m_ProxyUser;
}

// -------------------------------------------------------------------------------- //
wxString guMediaCtrl::ProxyPass() const
{
    return m_ProxyPass;
}

// -------------------------------------------------------------------------------- //
wxString guMediaCtrl::ProxyServer() const
{
    return m_ProxyServer;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ToggleEqualizer()
{
    guLogDebug("guMediaCtrl::ToggleEqualizer <<" );
    Lock();
    int Count = m_FaderPlayBins.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guFaderPlaybin * FaderPlaybin = m_FaderPlayBins[ Index ];
        if( FaderPlaybin->IsOk() )
            FaderPlaybin->ToggleEqualizer();
    }
    m_EnableEq = !m_EnableEq;
    Unlock();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ToggleVolCtl()
{
    guLogDebug("guMediaCtrl::ToggleVolCtl <<" );
    Lock();
    int Count = m_FaderPlayBins.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guFaderPlaybin * FaderPlaybin = m_FaderPlayBins[ Index ];
        if( FaderPlaybin->IsOk() )
            FaderPlaybin->ToggleVolCtl();
    }
    m_EnableVolCtls = !m_EnableVolCtls;
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_ForceGapless = m_EnableVolCtls ? Config->ReadBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, false, CONFIG_PATH_CROSSFADER ) : true;
    Unlock();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ReconfigureRG()
{
    guLogDebug("guMediaCtrl::ReconfigureRG <<" );
    Lock();
    int Count = m_FaderPlayBins.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guFaderPlaybin * FaderPlaybin = m_FaderPlayBins[ Index ];
        if( FaderPlaybin->IsOk() )
            FaderPlaybin->ReconfigureRG();
    }
    Unlock();
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::IsRecording( void )
{
    if( m_CurrentPlayBin )
        m_IsRecording = m_CurrentPlayBin->IsRecording();
    return m_IsRecording;
}

}

// -------------------------------------------------------------------------------- //
