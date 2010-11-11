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
// -------------------------------------------------------------------------------- //
#include "MediaCtrl.h"

#include "Config.h"
#include "FileRenamer.h" // NormalizeField
#include "PlayerPanel.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/uri.h>

DEFINE_EVENT_TYPE( guEVT_MEDIA_LOADED )
//DEFINE_EVENT_TYPE( guEVT_MEDIA_SET_NEXT_MEDIA )
DEFINE_EVENT_TYPE( guEVT_MEDIA_FINISHED )
DEFINE_EVENT_TYPE( guEVT_MEDIA_CHANGED_STATE )
DEFINE_EVENT_TYPE( guEVT_MEDIA_BUFFERING )
DEFINE_EVENT_TYPE( guEVT_MEDIA_LEVELINFO )
DEFINE_EVENT_TYPE( guEVT_MEDIA_TAGINFO )
DEFINE_EVENT_TYPE( guEVT_MEDIA_CHANGED_BITRATE )
DEFINE_EVENT_TYPE( guEVT_MEDIA_CHANGED_POSITION )
DEFINE_EVENT_TYPE( guEVT_MEDIA_CHANGED_LENGTH )
DEFINE_EVENT_TYPE( guEVT_MEDIA_ERROR )
DEFINE_EVENT_TYPE( guEVT_MEDIA_FADEOUT_FINISHED )
DEFINE_EVENT_TYPE( guEVT_MEDIA_FADEIN_STARTED )

#define guFADERPLAYBIN_FAST_FADER_TIME          (1000)

#define GST_AUDIO_TEST_SRC_WAVE_SILENCE         4

#define GST_TO_WXSTRING( str )  ( wxString( str, wxConvUTF8 ) )

#define guLogDebug(...)  guLogMessage(__VA_ARGS__)
//#define guLogDebug(...)
#define guSHOW_DUMPFADERPLAYBINS     1

#ifdef guSHOW_DUMPFADERPLAYBINS
// -------------------------------------------------------------------------------- //
static void DumpFaderPlayBins( const guFaderPlayBinArray &playbins, guFaderPlayBin * current )
{
    guLogMessage( wxT( "CurrentPlayBin: %i" ), current ? current->GetId() : 0 );
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
        guFaderPlayBin * FaderPlayBin = playbins[ Index ];

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
            default :                                       StateName = wxT( "other" ); break;
        }

        //if( !FaderPlayBin->Uri().IsEmpty() )
        {
            guLogDebug( wxT( "[%i] '%s'" ), FaderPlayBin->GetId(), StateName.c_str() );
        }
    }
    guLogDebug( wxT( " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " ) );
}
#endif

// -------------------------------------------------------------------------------- //
//static guFaderPlayBin * FindFaderPlayBin( const guFaderPlayBinArray &playbins, GstElement * element )
//{
//    int Index;
//    int Count = playbins.Count();
//    for( Index = 0; Index < Count; Index++ )
//    {
//        GstElement * e = element;
//        while( e )
//        {
//            if( e == GST_ELEMENT( playbins[ Index ]->m_Playbin ) )
//            {
//                return playbins[ Index ];
//            }
//			e = GST_ELEMENT_PARENT( e );
//        }
//    }
//    return NULL;
//}
//
// -------------------------------------------------------------------------------- //
//static guFaderPlayBin * FindFaderPlayBin( const guFaderPlayBinArray &playbins, int statemask )
//{
//    int Index;
//    int Count = playbins.Count();
//    for( Index = 0; Index < Count; Index++ )
//    {
//        if( playbins[ Index ]->m_State & statemask )
//            return playbins[ Index ];
//    }
//    return NULL;
//}


// -------------------------------------------------------------------------------- //
extern "C" {

// -------------------------------------------------------------------------------- //
static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guFaderPlayBin * ctrl )
{
    switch( GST_MESSAGE_TYPE( message ) )
    {
        case GST_MESSAGE_ERROR :
        {
            GError * err;
            //gchar * debug;
            gst_message_parse_error( message, &err, NULL );

            ctrl->SetState( guFADERPLAYBIN_STATE_ERROR );

            guMediaCtrl * MediaCtrl = ctrl->GetPlayer();
            if( MediaCtrl && ctrl->IsOk() )
            {
                MediaCtrl->SetLastError( err->code );
                ctrl->SetErrorCode( err->code );
                ctrl->SetState( guFADERPLAYBIN_STATE_ERROR );

                wxString * ErrorStr = new wxString( err->message, wxConvUTF8 );

                guLogError( wxT( "Gstreamer error '%s'" ), ErrorStr->c_str() );

                guMediaEvent event( guEVT_MEDIA_ERROR );
                event.SetClientData( ( void * ) ErrorStr );
                MediaCtrl->SendEvent( event );
            }

            g_error_free( err );
            //g_free( debug );
            break;
        }

//        case GST_MESSAGE_STATE_CHANGED:
//        {
////            GstState oldstate, newstate, pendingstate;
////            gst_message_parse_state_changed( message, &oldstate, &newstate, &pendingstate );
//
////            //guLogMessage( wxT( "State changed %u -> %u (%u)" ), oldstate, newstate, pendingstate );
////            if( pendingstate == GST_STATE_VOID_PENDING )
////            {
////                wxMediaEvent event( wxEVT_MEDIA_STATECHANGED );
////                ctrl->AddPendingEvent( event );
////            }
//            break;
//        }

        case GST_MESSAGE_BUFFERING :
        {
            gint        Percent;
            gst_message_parse_buffering( message, &Percent );

            guLogDebug( wxT( "Buffering (%i): %i%%" ), ctrl->GetId(), Percent );

            if( Percent != 100 )
            {
                if( !ctrl->IsBuffering() )
                    ctrl->Pause();
            }
            else
            {
                ctrl->Play();
            }
            ctrl->SetBuffering( Percent != 100 );

            guMediaEvent event( guEVT_MEDIA_BUFFERING );
            event.SetInt( Percent );
            ctrl->SendEvent( event );
            break;
        }

        case GST_MESSAGE_EOS :
        {
            guMediaEvent event( guEVT_MEDIA_FINISHED );
            event.SetExtraLong( ctrl->GetId() );
            ctrl->SendEvent( event );

            ctrl->SetState( guFADERPLAYBIN_STATE_PENDING_REMOVE );

            ctrl->GetPlayer()->ScheduleCleanUp();

            guLogDebug( wxT( "***** EOS received..." ) );
          break;
        }

        case GST_MESSAGE_TAG :
        {
            // The stream discovered new tags.
            GstTagList * tags;
            //gchar * title = NULL;
            unsigned int bitrate = 0;
            // Extract from the message the GstTagList.
            //This generates a copy, so we must remember to free it.
            gst_message_parse_tag( message, &tags );

            guRadioTagInfo * RadioTagInfo = new guRadioTagInfo();

            gst_tag_list_get_string( tags, GST_TAG_ORGANIZATION, &RadioTagInfo->m_Organization );
            gst_tag_list_get_string( tags, GST_TAG_LOCATION, &RadioTagInfo->m_Location );
            gst_tag_list_get_string( tags, GST_TAG_TITLE, &RadioTagInfo->m_Title );
            gst_tag_list_get_string( tags, GST_TAG_GENRE, &RadioTagInfo->m_Genre );

            ////guLogDebug( wxT( "New Tag Found:\n'%s'\n'%s'\n'%s'\n'%s'" ),
            //    wxString( RadioTagInfo->m_Organization, wxConvUTF8 ).c_str(),
            //    wxString( RadioTagInfo->m_Location, wxConvUTF8 ).c_str(),
            //    wxString( RadioTagInfo->m_Title, wxConvUTF8 ).c_str(),
            //    wxString( RadioTagInfo->m_Genre, wxConvUTF8 ).c_str() );

            if( RadioTagInfo->m_Organization || RadioTagInfo->m_Location ||
                RadioTagInfo->m_Title || RadioTagInfo->m_Genre )
            {
                guMediaEvent event( guEVT_MEDIA_TAGINFO );
                event.SetClientData( RadioTagInfo );
                ctrl->SendEvent( event );
            }
            else
            {
                delete RadioTagInfo;
            }

            gst_tag_list_get_uint( tags, GST_TAG_BITRATE, &bitrate );
            if( bitrate )
            {
                guMediaEvent event( guEVT_MEDIA_CHANGED_BITRATE );
                event.SetInt( bitrate );
                event.SetExtraLong( ctrl->GetId() );
                ctrl->SendEvent( event );
            }

            // Free the tag list
            gst_tag_list_free( tags );
            break;
        }

        case GST_MESSAGE_ELEMENT :
        {
            if( ctrl == ctrl->GetPlayer()->CurrentPlayBin() )
            {
                const GstStructure * s = gst_message_get_structure( message );
                const gchar * name = gst_structure_get_name( s );

                ////guLogDebug( wxT( "MESSAGE_ELEMENT %s" ), wxString( element ).c_str() );
                if( !strcmp( name, "level" ) )
                {
                    guLevelInfo * LevelInfo = new guLevelInfo();
                    guMediaEvent event( guEVT_MEDIA_LEVELINFO );
                    gint channels;
                    const GValue * list;
                    const GValue * value;
//                    if( !gst_structure_get_clock_time( s, "endtime", &LevelInfo->m_EndTime ) )
//                        guLogWarning( wxT( "Could not parse endtime" ) );
//
//                    LevelInfo->m_EndTime /= GST_MSECOND;
//
//                    GstFormat format = GST_FORMAT_TIME;
//                    if( gst_element_query_position( ctrl->OutputSink(), &format, ( gint64 * ) &LevelInfo->m_OutTime ) )
//                    {
//                        LevelInfo->m_OutTime /= GST_MSECOND;
//                    }

                    ////guLogDebug( wxT( "endtime: %" GST_TIME_FORMAT ", channels: %d" ), GST_TIME_ARGS( endtime ), channels );

                    // we can get the number of channels as the length of any of the value lists
    //                list = gst_structure_get_value( s, "rms" );
    //                channels = LevelInfo->m_Channels = gst_value_list_get_size( list );
    //                value = gst_value_list_get_value( list, 0 );
    //                LevelInfo->m_RMS_L = g_value_get_double( value );
    //                if( channels > 1 )
    //                {
    //                    value = gst_value_list_get_value( list, 1 );
    //                    LevelInfo->m_RMS_R = g_value_get_double( value );
    //                }

                    list = gst_structure_get_value( s, "peak" );
                    channels = LevelInfo->m_Channels = gst_value_list_get_size( list );
                    value = gst_value_list_get_value( list, 0 );
                    LevelInfo->m_Peak_L = g_value_get_double( value );
                    if( channels > 1 )
                    {
                        value = gst_value_list_get_value( list, 1 );
                        LevelInfo->m_Peak_R = g_value_get_double( value );
                    }

                    list = gst_structure_get_value( s, "decay" );
                    value = gst_value_list_get_value( list, 0 );
                    LevelInfo->m_Decay_L = g_value_get_double( value );
                    if( channels > 1 )
                    {
                        value = gst_value_list_get_value( list, 1 );
                        LevelInfo->m_Decay_R = g_value_get_double( value );
                    }


    //                //guLogDebug( wxT( "    RMS: %f dB, peak: %f dB, decay: %f dB" ),
    //                    event.m_LevelInfo.m_RMS_L,
    //                    event.m_LevelInfo.m_Peak_L,
    //                    event.m_LevelInfo.m_Decay_L );

    //                // converting from dB to normal gives us a value between 0.0 and 1.0 */
    //                rms = pow( 10, rms_dB / 20 );
    //                  //guLogDebug( wxT( "    normalized rms value: %f" ), rms );
                    event.SetClientObject( ( wxClientData * ) LevelInfo );
                    event.SetExtraLong( ctrl->GetId() );
                    ctrl->SendEvent( event );
                }
            }
            break;
        }

        //
        case GST_MESSAGE_APPLICATION :
        {
            const GstStructure * Struct;
            const char * Name;

            Struct = gst_message_get_structure( message );
            Name = gst_structure_get_name( Struct );
            guLogDebug( wxT( "Got Application Message %s" ), GST_TO_WXSTRING( Name ).c_str() );

            if( !strcmp( Name, guFADERPLAYBIN_MESSAGE_FADEIN_START ) )
            {
                if( !ctrl->EmittedStartFadeIn() )
                {
                    ctrl->FadeInStart();
                }
            }
            else if( !strcmp( Name, guFADERPLAYBIN_MESSAGE_FADEOUT_DONE ) )
            {
                if( !ctrl->EmittedStartFadeIn() )
                {
                    ctrl->FadeInStart();
                }
                ctrl->FadeOutDone();
            }

            break;
        }

        default:
            break;
    }

    return TRUE;
}

// -------------------------------------------------------------------------------- //
static void gst_about_to_finish( GstElement * playbin, guFaderPlayBin * ctrl )
{
    if( !ctrl->NextUri().IsEmpty() )
    {
        ctrl->AboutToFinish();
    }
    else if( !ctrl->EmittedStartFadeIn() )
    {
        ctrl->FadeInStart();
    }
}

// -------------------------------------------------------------------------------- //
static void gst_audio_changed( GstElement * playbin, guFaderPlayBin * ctrl )
{
    ctrl->AudioChanged();
}

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

// -------------------------------------------------------------------------------- //
static void record_unlocked( GstPad * pad, bool isblocked, GstPad * new_pad )
{
    guLogDebug( wxT( "Record_Unlocked" ) );
	GstEvent * segment;
	if( new_pad == NULL )
		return;

	// send a very unimaginative new segment through the new pad
	segment = gst_event_new_new_segment( true, 1.0, GST_FORMAT_DEFAULT, 0, GST_CLOCK_TIME_NONE, 0 );
	gst_pad_send_event( new_pad, segment );
	gst_object_unref( new_pad );
}

// -------------------------------------------------------------------------------- //
static void add_record_element( GstPad * pad, bool isblocked, guFaderPlayBin * ctrl )
{
    ctrl->AddRecordElement( pad, isblocked );
}

// -------------------------------------------------------------------------------- //
static void remove_record_element( GstPad * pad, bool isblocked, guFaderPlayBin * ctrl )
{
    ctrl->RemoveRecordElement( pad, isblocked );
}

// -------------------------------------------------------------------------------- //
bool IsValidElement( GstElement * element )
{
    if( !GST_IS_ELEMENT( element ) )
    {
        if( G_IS_OBJECT( element ) )
            g_object_unref( element );
        return false;
    }
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
    gint ms_period = 1000 / 5;
    m_TickTimeoutId = g_timeout_add( ms_period, GSourceFunc( tick_timeout ), this );

    if( Init() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_FadeOutTime       = Config->ReadNum( wxT( "FadeOutTime" ), 50, wxT( "Crossfader" ) ) * 100;
        m_FadeInTime        = Config->ReadNum( wxT( "FadeInTime" ), 10, wxT( "Crossfader" ) ) * 100;
        m_FadeInVolStart    = double( Config->ReadNum( wxT( "FadeInVolStart" ), 80, wxT( "Crossfader" ) ) ) / 100.0;
        m_FadeInVolTriger   = double( Config->ReadNum( wxT( "FadeInVolTriger" ), 50, wxT( "Crossfader" ) ) ) / 100.0;
        m_BufferSize        = Config->ReadNum( wxT( "BufferSize" ), 64, wxT( "General" ) );
        m_ReplayGainMode    = Config->ReadNum( wxT( "ReplayGainMode" ), 0, wxT( "General" ) );
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

    gst_deinit();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::SendEvent( guMediaEvent &event )
{
    wxPostEvent( m_PlayerPanel, event );
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::UpdatePosition( void )
{
    GstFormat Format;
	gint64 Pos = -1;
	gint64 Duration = -1;

	Lock();
	if( m_CurrentPlayBin )
	{
	    m_CurrentPlayBin->Lock();
        Format = GST_FORMAT_TIME;
		Pos = -1;
		gst_element_query_position( m_CurrentPlayBin->m_OutputSink, &Format, &Pos );

        Format = GST_FORMAT_TIME;
		Duration = -1;
        gst_element_query_duration( m_CurrentPlayBin->m_OutputSink, &Format, &Duration );

        Pos = Pos / GST_MSECOND;
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
bool guMediaCtrl::RemovePlayBin( guFaderPlayBin * playbin )
{
    guLogDebug( wxT( "guMediaCtrl::RemovePlayBin (%i)" ), playbin->m_Id );
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
    //guLogDebug( wxT( "Volume: %0.5f" ), volume );
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
            case guFADERPLAYBIN_STATE_PAUSED :
                State = guMEDIASTATE_PAUSED;
                break;

            case guFADERPLAYBIN_STATE_FADEIN :
            case guFADERPLAYBIN_STATE_FADEOUT :
            case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
            case guFADERPLAYBIN_STATE_FADEOUT_STOP :
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
long guMediaCtrl::Load( const wxString &uri, guFADERPLAYBIN_PLAYTYPE playtype )
{
    guLogDebug( wxT( "guMediaCtrl::Load %i" ), playtype );

    guFaderPlayBin * FaderPlaybin;
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
            guLogDebug( wxT( "Replacing the current track in the current playbin..." ) );
            Lock();
            if( m_CurrentPlayBin )
            {
                if( m_CurrentPlayBin->m_State == guFADERPLAYBIN_STATE_ERROR )
                {
                    m_CurrentPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                    ScheduleCleanUp();
                }
                else
                {
                    m_CurrentPlayBin->m_State = guFADERPLAYBIN_STATE_WAITING;
                    m_CurrentPlayBin->Load( uri, true );
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

    FaderPlaybin = new guFaderPlayBin( this, uri, playtype );
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
    guFaderPlayBin * FaderPlaybin = m_FaderPlayBins[ 0 ];
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
            FaderPlaybin->StartFade( 0.0, 1.0, m_FadeOutTime ? guFADERPLAYBIN_FAST_FADER_TIME : 200 );
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

    guFaderPlayBin * FaderPlayBin = NULL;
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
        FaderPlayBin = ( guFaderPlayBin * ) ToFade[ Index ];
        FadeOutTime = m_FadeOutTime ? guFADERPLAYBIN_FAST_FADER_TIME : 200;
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

    guFaderPlayBin * FaderPlayBin = NULL;
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
        FaderPlayBin = ( guFaderPlayBin * ) ToFade[ Index ];
        FadeOutTime = m_FadeOutTime ? guFADERPLAYBIN_FAST_FADER_TIME : 200;
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
bool guMediaCtrl::Seek( wxFileOffset where )
{
    guLogDebug( wxT( "guMediaCtrl::Seek( %lli )" ), where );
    bool Result = false;
    Lock();
    if( m_CurrentPlayBin )
    {
        Result = m_CurrentPlayBin->Seek( where );
    }
    Unlock();
    return Result;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::UpdatedConfig( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_FadeOutTime       = Config->ReadNum( wxT( "FadeOutTime" ), 50, wxT( "Crossfader" ) ) * 100;
    m_FadeInTime        = Config->ReadNum( wxT( "FadeInTime" ), 10, wxT( "Crossfader" ) ) * 100;
    m_FadeInVolStart    = double( Config->ReadNum( wxT( "FadeInVolStart" ), 80, wxT( "Crossfader" ) ) ) / 100.0;
    m_FadeInVolTriger   = double( Config->ReadNum( wxT( "FadeInVolTriger" ), 50, wxT( "Crossfader" ) ) ) / 100.0;
    m_BufferSize        = Config->ReadNum( wxT( "BufferSize" ), 64, wxT( "General" ) );
    m_ReplayGainMode    = Config->ReadNum( wxT( "ReplayGainMode" ), 0, wxT( "General" ) );
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
        guFaderPlayBin * NextPlayBin = m_FaderPlayBins[ Index ];
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
void guMediaCtrl::FadeOutDone( guFaderPlayBin * faderplaybin )
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
            GstFormat Format = GST_FORMAT_TIME;
            gint64 Pos = -1;
            gst_element_query_position( faderplaybin->OutputSink(), &Format, &Pos );
            if( Pos != -1 )
            {
                faderplaybin->m_PausePosition = Pos > guFADERPLAYBIN_FAST_FADER_TIME ? Pos - guFADERPLAYBIN_FAST_FADER_TIME : 200;
                //guLogDebug( wxT( "got fade-out-done for stream %s -> SEEKING_PAUSED [%" G_GINT64_FORMAT "]" ), FaderPlayBin->m_Uri.c_str(), FaderPlayBin->m_SeekTarget );
            }
            else
            {
                faderplaybin->m_PausePosition = wxNOT_FOUND;
                //guLogDebug( wxT( "got fade-out-done for stream %s -> PAUSED (position query failed)" ), FaderPlayBin->m_Uri.c_str() );
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
    bool Result = ( m_IsRecording = ( m_CurrentPlayBin && m_CurrentPlayBin->EnableRecord( recfile, format, quality ) ) );
    Unlock();
    return  Result;
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
    guLogDebug( wxT( "guMediaCtrl::SetRecordFileName  '%s'" ), filename.c_str() );
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

	m_CleanUpId = 0;

	int Index;
	int Count = m_FaderPlayBins.Count();
	for( Index = Count - 1; Index >= 0; Index-- )
	{
	    guFaderPlayBin * FaderPlaybin = m_FaderPlayBins[ Index ];
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

	Unlock();

	Count = ToDelete.Count();
	for( Index = 0; Index < Count; Index++ )
	{
		//guLogDebug( wxT( "reaping stream %s" ), ( ( guFaderPlayBin * ) ToDelete[ Index ] )->m_Uri.c_str() );
		delete ( ( guFaderPlayBin * ) ToDelete[ Index ] );
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
        argvGST[ i ] = wxStrdupA( wxConvUTF8.cWX2MB( wxTheApp->argv[ i ] ) );
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
// guFaderPlayBin
// -------------------------------------------------------------------------------- //
guFaderPlayBin::guFaderPlayBin( guMediaCtrl * mediactrl, const wxString &uri, const int playtype )
{
	//guLogDebug( wxT( "creating new stream for %s" ), uri.c_str() );
    m_Player = mediactrl;
    m_Uri = uri;
    m_Id = wxGetLocalTime();
    m_State = guFADERPLAYBIN_STATE_WAITING;
    m_ErrorCode = 0;
    m_IsFading = false;
    m_IsBuffering = false;
    m_EmittedStartFadeIn = false;
    m_PlayType = playtype;
    m_FaderTimeLine = NULL;
    m_AboutToFinishPending = false;
    m_LastFadeVolume = -1;

    guLogDebug( wxT( "guFaderPlayBin::guFaderPlayBin (%i)  %i" ), m_Id, playtype );

    if( BuildOutputBin() && BuildPlaybackBin() )
    {
        //Load( uri, false );
        SetVolume( m_Player->GetVolume() );
        SetEqualizer( m_Player->GetEqualizer() );
    }
}

// -------------------------------------------------------------------------------- //
guFaderPlayBin::~guFaderPlayBin()
{
    guLogDebug( wxT( "guFaderPlayBin::~guFaderPlayBin (%i)" ), m_Id );
    //m_Player->RemovePlayBin( this );

    if( m_Playbin )
    {
        gst_element_set_state( m_Playbin, GST_STATE_NULL );
        gst_object_unref( GST_OBJECT( m_Playbin ) );
    }

    if( m_FaderTimeLine )
    {
        EndFade();
    }
    guLogDebug( wxT( "Finished destroying the playbin %i" ), m_Id );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::BuildOutputBin( void )
{
    GstElement * outputsink;

    const char * ElementNames[] = {
        "gconfaudiosink",
        "autoaudiosink",
        "alsasink",
        "pulsesink",
        "osssink",
        NULL
    };

    int Index = 0;
    while( ElementNames[ Index ] )
    {
        outputsink = gst_element_factory_make( ElementNames[ Index ], "OutputSink" );
        if( IsValidElement( outputsink ) )
        {
            m_OutputSink = outputsink;
            return true;
        }
        Index++;
    }

    guLogError( wxT( "Could not find a valid audiosink" ) );
    m_OutputSink = NULL;
    return false;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::BuildPlaybackBin( void )
{
  m_Playbin = gst_element_factory_make( "playbin2", "play" );
  if( IsValidElement( m_Playbin ) )
  {
    g_object_set( m_Playbin, "uri", ( const char * ) m_Uri.mb_str( wxConvFile ), NULL );
    g_object_set( m_Playbin, "buffer-size", m_Player->BufferSize() * 1024, NULL );
    //g_object_set( m_Playbin, "volume", 1.0, NULL );

    m_Playbackbin = gst_bin_new( "playbackbin" );
    if( IsValidElement( m_Playbackbin ) )
    {
      GstElement * converter = gst_element_factory_make( "audioconvert", "pb_audioconvert" );
      if( IsValidElement( converter ) )
      {
        m_ReplayGain = gst_element_factory_make( "rgvolume", "pb_rgvolume" );
        if( IsValidElement( m_ReplayGain ) )
        {
          g_object_set( G_OBJECT( m_ReplayGain ), "album-mode", m_Player->m_ReplayGainMode, NULL );
          //g_object_set( G_OBJECT( m_ReplayGain ), "pre-amp", gdouble( 6 ), NULL );

          m_FaderVolume = gst_element_factory_make( "volume", "fader_volume" );
          if( IsValidElement( m_FaderVolume ) )
          {
            if( m_PlayType == guFADERPLAYBIN_PLAYTYPE_CROSSFADE )
            {
                g_object_set( m_FaderVolume, "volume", 0.0, NULL );
            }

            GstElement * level = gst_element_factory_make( "level", "pb_level" );
            if( IsValidElement( level ) )
            {
                g_object_set( level, "message", TRUE, NULL );
                g_object_set( level, "interval", gint64( 100000000 ), NULL );

                m_Volume = gst_element_factory_make( "volume", "pb_volume" );
                if( IsValidElement( m_Volume ) )
                {
                    m_Equalizer = gst_element_factory_make( "equalizer-10bands", "pb_equalizer" );
                    if( IsValidElement( m_Equalizer ) )
                    {
                        GstElement * limiter = gst_element_factory_make( "rglimiter", "pb_rglimiter" );
                        if( IsValidElement( limiter ) )
                        {
                            //g_object_set( G_OBJECT( limiter ), "enabled", TRUE, NULL );

                            GstElement * outconverter = gst_element_factory_make( "audioconvert", "pb_audioconvert2" );
                            if( IsValidElement( outconverter ) )
                            {

                                m_Tee = gst_element_factory_make( "tee", "pb_tee" );
                                if( IsValidElement( m_Tee ) )
                                {
                                    GstElement * queue = gst_element_factory_make( "queue", "pb_queue" );
                                    if( IsValidElement( queue ) )
                                    {
                                        //g_object_set( queue, "max-size-time", guint64( 250000000 ), NULL );
                                        //g_object_set( queue, "max-size-time", 5 * GST_SECOND, "max-size-buffers", 0, "max-size-bytes", 0, NULL );

                                        gst_bin_add_many( GST_BIN( m_Playbackbin ), m_Tee, queue, converter, m_FaderVolume, m_ReplayGain, level, m_Equalizer, limiter, m_Volume, outconverter, m_OutputSink, NULL );
                                        gst_element_link_many( m_Tee, queue, converter, m_FaderVolume, m_ReplayGain, level, m_Equalizer, limiter, m_Volume, outconverter, m_OutputSink, NULL );

                                        GstPad * pad = gst_element_get_pad( m_Tee, "sink" );
                                        if( GST_IS_PAD( pad ) )
                                        {
                                            GstPad * ghostpad = gst_ghost_pad_new( "sink", pad );
                                            gst_element_add_pad( m_Playbackbin, ghostpad );
                                            gst_object_unref( pad );

                                            g_object_set( G_OBJECT( m_Playbin ), "audio-sink", m_Playbackbin, NULL );

                                            g_object_set( G_OBJECT( m_Playbin ), "flags", 0x02 | 0x10, NULL );

                                            g_signal_connect( G_OBJECT( m_Playbin ), "about-to-finish",
                                                G_CALLBACK( gst_about_to_finish ), ( void * ) this );
                                            //
                                            g_signal_connect( G_OBJECT( m_Playbin ), "audio-changed",
                                                G_CALLBACK( gst_audio_changed ), ( void * ) this );

                                            gst_bus_add_watch( gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) ),
                                                GstBusFunc( gst_bus_async_callback ), this );

                                            return true;
                                        }
                                        else
                                        {
                                            if( G_IS_OBJECT( pad ) )
                                                gst_object_unref( pad );
                                            guLogError( wxT( "Could not create the pad element" ) );
                                        }

                                        g_object_unref( queue );
                                    }
                                    else
                                    {
                                        guLogError( wxT( "Could not create the playback queue object" ) );
                                    }

                                    g_object_unref( m_Tee );
                                    m_Tee = NULL;
                                }
                                else
                                {
                                    guLogError( wxT( "Could not create the tee object" ) );
                                }

                                g_object_unref( outconverter );
                            }
                            else
                            {
                                guLogError( wxT( "Could not create the output audioconvert object" ) );
                            }

                            g_object_unref( limiter );
                        }
                        else
                        {
                            guLogError( wxT( "Could not create the limiter object" ) );
                        }

                        g_object_unref( m_Equalizer );
                        m_Equalizer = NULL;
                    }
                    else
                    {
                        guLogError( wxT( "Could not create the equalizer object" ) );
                    }

                    g_object_unref( m_Volume );
                    m_Volume = NULL;
                }
                else
                {
                    guLogError( wxT( "Could not create the volume object" ) );
                }

                g_object_unref( level );
            }
            else
            {
                guLogError( wxT( "Could not create the level object" ) );
            }

            g_object_unref( m_FaderVolume );
          }
          else
          {
            guLogError( wxT( "Could not create the fader volume object" ) );
          }
          m_FaderVolume = NULL;

          g_object_unref( m_ReplayGain );
        }
        else
        {
            guLogError( wxT( "Could not create the replay gain object" ) );
        }
        m_ReplayGain = NULL;

        g_object_unref( converter );
      }
      else
      {
        guLogError( wxT( "Could not create the audioconvert object" ) );
      }

      g_object_unref( m_Playbackbin );
    }
    else
    {
      guLogError( wxT( "Could not create the gstreamer playbackbin." ) );
    }
    m_Playbackbin = NULL;

    gst_object_unref( m_Playbin );
  }
  else
  {
    guLogError( wxT( "Could not create the gstreamer playbin." ) );
  }
  m_Playbin = NULL;

  return false;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::BuildRecordBin( const wxString &path, GstElement * encoder, GstElement * muxer )
{
    m_RecordBin = gst_bin_new( "recordbin" );
    if( IsValidElement( m_RecordBin ) )
    {
        GstElement * converter = gst_element_factory_make( "audioconvert", "rb_audioconvert" );
        if( IsValidElement( converter ) )
        {
            m_FileSink = gst_element_factory_make( "filesink", "rb_filesink" );
            if( IsValidElement( m_FileSink ) )
            {
                g_object_set( m_FileSink, "location", ( const char * ) path.mb_str( wxConvFile ), NULL );

                GstElement * queue = gst_element_factory_make( "queue", "rb_queue" );
                if( IsValidElement( queue ) )
                {
                    // The bin contains elements that change state asynchronously and not as part of a state change in the entire pipeline.
                    g_object_set( m_RecordBin, "async-handling", true, NULL );

                    g_object_set( queue, "max-size-buffers", 3, NULL );
                    //g_object_set( queue, "max-size-time", 10 * GST_SECOND, "max-size-buffers", 0, "max-size-bytes", 0, NULL );

                    if( muxer )
                    {
                        gst_bin_add_many( GST_BIN( m_RecordBin ), queue, converter, encoder, muxer, m_FileSink, NULL );
                        gst_element_link_many( queue, converter, encoder, muxer, m_FileSink, NULL );
                    }
                    else
                    {
                        gst_bin_add_many( GST_BIN( m_RecordBin ), queue, converter, encoder, m_FileSink, NULL );
                        gst_element_link_many( queue, converter, encoder, m_FileSink, NULL );
                    }

                    GstPad * pad = gst_element_get_pad( queue, "sink" );
                    if( GST_IS_PAD( pad ) )
                    {
                        m_RecordGhostPad = gst_ghost_pad_new( "sink", pad );
                        gst_element_add_pad( m_RecordBin, m_RecordGhostPad );
                        gst_object_unref( pad );

                        return true;
                    }
                    else
                    {
                        if( G_IS_OBJECT( pad ) )
                            gst_object_unref( pad );
                        guLogError( wxT( "Could not create the pad element" ) );
                    }

                    g_object_unref( queue );
                }
                else
                {
                    guLogError( wxT( "Could not create the playback queue object" ) );
                }

                g_object_unref( m_FileSink );
                m_FileSink = NULL;
            }
            else
            {
                guLogError( wxT( "Could not create the lame encoder object" ) );
            }
            g_object_unref( converter );
        }
        else
        {
            guLogError( wxT( "Could not create the record convert object" ) );
        }

        g_object_unref( m_RecordBin );
    }
    else
    {
        guLogError( wxT( "Could not create the recordbin object" ) );
    }
    m_RecordBin = NULL;

    return false;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::SendEvent( guMediaEvent &event )
{
    m_Player->SendEvent( event );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::SetVolume( double volume )
{
    guLogDebug( wxT( "guFaderPlayBin::SetVolume (%i)  %0.2f" ), m_Id, volume );
    g_object_set( m_Volume, "volume", volume, NULL );
    return true;
}

// -------------------------------------------------------------------------------- //
double guFaderPlayBin::GetFaderVolume( void )
{
    double RetVal;
    g_object_get( m_FaderVolume, "volume", &RetVal, NULL );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::FadeInStart( void )
{
    m_EmittedStartFadeIn = true;
    m_Player->FadeInStart();
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::FadeOutDone( void )
{
     m_Player->FadeOutDone( this );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::SetFaderVolume( double volume )
{
	const char * Message = NULL;


    if( volume != m_LastFadeVolume )
    {
        //guLogDebug( wxT( "guFaderPlayBin::SetFaderVolume (%i)   %0.2f  %i" ), m_Id, volume, m_State == guFADERPLAYBIN_STATE_FADEIN );
        Lock();
        m_LastFadeVolume = volume;
        g_object_set( m_FaderVolume, "volume", volume, NULL );

        switch( m_State )
        {
            case guFADERPLAYBIN_STATE_FADEIN :
            {
                if( ( volume > 0.99 ) && m_IsFading )
                {
                    guLogDebug( wxT( "stream fully faded in (at %f) -> PLAYING state" ), volume );
                    m_IsFading = false;
                    m_State = guFADERPLAYBIN_STATE_PLAYING;
                }
                break;
            }

            case guFADERPLAYBIN_STATE_FADEOUT :
            case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
            case guFADERPLAYBIN_STATE_FADEOUT_STOP :
            {
                if( volume < 0.001 )
                {
                    //guLogDebug( wxT( "stream %s fully faded out (at %f)" ), faderplaybin->m_Uri.c_str(), Vol );
                    if( m_IsFading )
                    {
                        //Message = guFADERPLAYBIN_MESSAGE_FADEOUT_DONE;
    //                    m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
    //                    guMediaEvent event( guEVT_MEDIA_FADEOUT_FINISHED );
    //                    event.SetExtraLong( GetId() );
    //                    SendEvent( event );
    //
    //                    m_Player->ScheduleCleanUp();
                        //m_Player->FadeOutDone( this );
                        Message = guFADERPLAYBIN_MESSAGE_FADEOUT_DONE;
                        //m_IsFading = false;
                    }
                }
                else if( !m_EmittedStartFadeIn && volume < ( m_Player->m_FadeInVolTriger + 0.001 ) )
                {
                    if( m_IsFading && m_State == guFADERPLAYBIN_STATE_FADEOUT )
                    {
                        //m_EmittedStartFadeIn = true;
                        //m_Player->FadeInStart();
                        Message = guFADERPLAYBIN_MESSAGE_FADEIN_START;
                    }
                }
                break;
            }
        }

        Unlock();
    }

	if( Message )
	{
		GstMessage * Msg;
		GstStructure * Struct;
		//guLogDebug( wxT( "posting %s message for stream %s" ), GST_TO_WXSTRING( Message ).c_str(), faderplaybin->m_Uri.c_str() );
		Struct = gst_structure_new( Message, NULL );
		Msg = gst_message_new_application( GST_OBJECT( m_Playbin ), Struct );
		gst_element_post_message( GST_ELEMENT( m_Playbin ), Msg );
	}

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::SetEqualizer( const wxArrayInt &eqbands )
{
    if( m_Equalizer && ( eqbands.Count() == guEQUALIZER_BAND_COUNT ) )
    {
        int index;
        for( index = 0; index < guEQUALIZER_BAND_COUNT; index++ )
        {
            SetEqualizerBand( index, eqbands[ index ] );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::SetEqualizerBand( const int band, const int value )
{
    g_object_set( G_OBJECT( m_Equalizer ), wxString::Format( wxT( "band%u" ),
            band ).char_str(), gdouble( value ), NULL );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::Load( const wxString &uri, const bool restart )
{
    guLogDebug( wxT( "guFaderPlayBin::Load (%i) %i" ), m_Id, restart );

    if( restart )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
        {
            return false;
        }
        gst_element_set_state( m_Playbin, GST_STATE_NULL );
    }

    if( !gst_uri_is_valid( ( const char * ) uri.mb_str( wxConvFile ) ) )
        return false;

    g_object_set( G_OBJECT( m_Playbin ), "uri", ( const char * ) uri.mb_str( wxConvFile ), NULL );

    if( restart )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        {
            return false;
        }
    }

    guMediaEvent event( guEVT_MEDIA_LOADED );
    event.SetInt( restart );
    SendEvent( event );
    guLogDebug( wxT( "Sent the loaded event..." ) );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::Play( void )
{
    guLogDebug( wxT( "guFaderPlayBin::Play (%i)" ), m_Id );
    return ( gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) != GST_STATE_CHANGE_FAILURE );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::StartPlay( void )
{
    guLogDebug( wxT( "guFaderPlayBin::StartPlay (%i)" ), m_Id );
    bool                Ret = true;
    bool                NeedReap = false;
    bool                Playing = false;
    guFaderPlayBinArray ToFade;

#ifdef guSHOW_DUMPFADERPLAYBINS
    m_Player->Lock();
    DumpFaderPlayBins( m_Player->m_FaderPlayBins, m_Player->m_CurrentPlayBin );
    m_Player->Unlock();
#endif


    switch( m_PlayType )
    {
        case guFADERPLAYBIN_PLAYTYPE_CROSSFADE :
        {
            guLogDebug( wxT( "About to start the faderplaybin in crossfade type" ) );
            m_Player->Lock();
            int Index;
            int Count = m_Player->m_FaderPlayBins.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                guFaderPlayBin * FaderPlaybin = m_Player->m_FaderPlayBins[ Index ];

                if( FaderPlaybin == this )
                    continue;

                switch( FaderPlaybin->m_State )
                {
                    case guFADERPLAYBIN_STATE_FADEIN :
                    case guFADERPLAYBIN_STATE_PLAYING :
                        ToFade.Add( FaderPlaybin );
                        break;

                    case guFADERPLAYBIN_STATE_ERROR :
                    case guFADERPLAYBIN_STATE_PAUSED :
                    case guFADERPLAYBIN_STATE_STOPPED :
                    case guFADERPLAYBIN_STATE_WAITING :
                    case guFADERPLAYBIN_STATE_WAITING_EOS :
                        FaderPlaybin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;

                    case guFADERPLAYBIN_STATE_PENDING_REMOVE :
                        NeedReap = true;

                    default :
                        break;
                }

            }
            m_Player->Unlock();

            Count = ToFade.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                double FadeOutStart = 1.0;
                int    FadeOutTime = m_Player->m_FadeOutTime;

                guFaderPlayBin * FaderPlaybin = ToFade[ Index ];

                switch( FaderPlaybin->m_State )
                {
                    case guFADERPLAYBIN_STATE_FADEIN :
                        //g_object_get( FaderPlaybin->m_Playbin, "volume", &FadeOutStart, NULL );
                        FadeOutStart = FaderPlaybin->GetFaderVolume();
                        FadeOutTime = ( int ) ( ( ( double ) FadeOutTime ) * FadeOutStart );

                    case guFADERPLAYBIN_STATE_PLAYING :
                        FaderPlaybin->StartFade( FadeOutStart, 0.0, FadeOutTime );
                        FaderPlaybin->m_State = guFADERPLAYBIN_STATE_FADEOUT;
                        m_IsFading = true;
                        break;

                    default :
                        break;
                }
            }

            if( !m_IsFading )
            {
                guLogDebug( wxT( "There was not previous playing track in crossfade mode so play this playbin..." ) );
                SetFaderVolume( 1.0 ); //g_object_set( m_Playbin, "volume", 1.0, NULL );
                if( Play() )
                {
                    m_State = guFADERPLAYBIN_STATE_PLAYING;
                    m_Player->Lock();
                    m_Player->m_CurrentPlayBin = this;
                    m_Player->Unlock();

                    guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
                    event.SetInt( GST_STATE_PLAYING );
                    SendEvent( event );
                }
            }

            break;
        }

        case guFADERPLAYBIN_PLAYTYPE_AFTER_EOS :
        {
            Playing = false;
            int Index;
            m_Player->Lock();
            int Count = m_Player->m_FaderPlayBins.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                guFaderPlayBin * FaderPlaybin = m_Player->m_FaderPlayBins[ Index ];

                if( FaderPlaybin == this )
                    continue;

                switch( FaderPlaybin->m_State )
                {
                    case guFADERPLAYBIN_STATE_PLAYING :
                    case guFADERPLAYBIN_STATE_FADEIN :
                    case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
                        //guLogDebug( wxT( "Stream %s already playing" ), FaderPlaybin->m_Uri.c_str() );
                        Playing = true;
                        break;

                    case guFADERPLAYBIN_STATE_PAUSED :
                        //guLogDebug( wxT( "stream %s is paused; replacing it" ), FaderPlaybin->m_Uri.c_str() );
                        FaderPlaybin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;

                    case guFADERPLAYBIN_STATE_PENDING_REMOVE :
                        NeedReap = true;
                        break;

                    default:
                        break;
                }
            }

            m_Player->Unlock();

            if( Playing )
            {
                m_State = guFADERPLAYBIN_STATE_WAITING_EOS;
            }
            else
            {
                if( Play() )
                {
                    m_State = guFADERPLAYBIN_STATE_PLAYING;
                    m_Player->Lock();
                    m_Player->m_CurrentPlayBin = this;
                    m_Player->Unlock();

                    guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
                    event.SetInt( GST_STATE_PLAYING );
                    SendEvent( event );
                }
            }

            break;
        }

        case guFADERPLAYBIN_PLAYTYPE_REPLACE :
        {
            int Index;
            m_Player->Lock();
            int Count = m_Player->m_FaderPlayBins.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                guFaderPlayBin * FaderPlaybin = m_Player->m_FaderPlayBins[ Index ];

                if( FaderPlaybin == this )
                    continue;

                switch( FaderPlaybin->m_State )
                {
                    case guFADERPLAYBIN_STATE_PLAYING :
                    case guFADERPLAYBIN_STATE_PAUSED :
                    case guFADERPLAYBIN_STATE_FADEIN :
                    case guFADERPLAYBIN_STATE_PENDING_REMOVE :
                        // kill this one
                        //guLogDebug( wxT( "stopping stream %s (replaced by new stream)" ), FaderPlaybin->m_Uri.c_str() );
                        FaderPlaybin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                        NeedReap = true;
                        break;

                    default:
                        break;
                }
            }

            m_Player->Unlock();

            if( Play() )
            {
                m_State = guFADERPLAYBIN_STATE_PLAYING;
                m_Player->Lock();
                m_Player->m_CurrentPlayBin = this;
                m_Player->Unlock();

                guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
                event.SetInt( GST_STATE_PLAYING );
                SendEvent( event );
            }

            break;
        }
    }

    if( NeedReap )
    {
        m_Player->ScheduleCleanUp();
    }

    return Ret;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::AboutToFinish( void )
{
    guLogDebug( wxT( "guFaderPlayBin::AboutToFinish (%i)" ), m_Id );
    Load( m_NextUri, false );
    m_NextUri = wxEmptyString;
    m_AboutToFinishPending = true;
}

// -------------------------------------------------------------------------------- //
static bool reset_about_to_finish( guFaderPlayBin * faderplaybin )
{
    faderplaybin->ResetAboutToFinishPending();
    return false;
}


// -------------------------------------------------------------------------------- //
void guFaderPlayBin::AudioChanged( void )
{
    guLogDebug( wxT( "guFaderPlayBin::AudioChanged (%i)" ), m_Id );
    if( m_AboutToFinishPending )
    {
        m_Id = m_NextId;
        m_NextId = 0;

        guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
        event.SetInt( GST_STATE_PLAYING );
        SendEvent( event );
        //m_AboutToFinishPending = false;
        m_AboutToFinishPendingId = g_timeout_add( 2000, GSourceFunc( reset_about_to_finish ), this );
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::StartFade( double volstart, double volend, int timeout )
{
    guLogDebug( wxT( "guFaderPlayBin::StartFade (%i) %0.2f, %0.2f, %i" ), m_Id, volstart, volend, timeout );
//    SetFaderVolume( volstart );

    m_EmittedStartFadeIn = false;
    m_IsFading = true;
    if( m_FaderTimeLine )
    {
//        guLogDebug( wxT( "Reversed the fader for %i" ), m_Id );
//        m_FaderTimeLine->ToggleDirection();
        EndFade();
    }

    m_FaderTimeLine = new guFaderTimeLine( timeout, NULL, this, volstart, volend );
    m_FaderTimeLine->SetDirection( volstart > volend ? guFaderTimeLine::Backward : guFaderTimeLine::Forward );
    m_FaderTimeLine->SetCurveShape( guTimeLine::LinearCurve );
    m_FaderTimeLine->Start();

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::Pause( void )
{
    guLogDebug( wxT( "guFaderPlayBin::Pause (%i)" ), m_Id );
    return ( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) != GST_STATE_CHANGE_FAILURE );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::Stop( void )
{
    guLogDebug( wxT( "guFaderPlayBin::Stop (%i)" ), m_Id );
    return ( gst_element_set_state( m_Playbin, GST_STATE_READY ) != GST_STATE_CHANGE_FAILURE );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::Seek( wxFileOffset where )
{
    guLogDebug( wxT( "guFaderPlayBin::Seek (%i) %li )" ), m_Id, where );

    return gst_element_seek( m_Playbin, 1, GST_FORMAT_TIME,
          GstSeekFlags( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
          GST_SEEK_TYPE_SET, where * GST_MSECOND,
          GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE );

}

// -------------------------------------------------------------------------------- //
wxFileOffset guFaderPlayBin::Position( void )
{
    GstFormat Format = GST_FORMAT_TIME;
    wxFileOffset Position;
    gst_element_query_position( m_OutputSink, &Format, &Position );
    return Position;
}

// -------------------------------------------------------------------------------- //
wxFileOffset guFaderPlayBin::Length( void )
{
    GstFormat Format = GST_FORMAT_TIME;
    wxFileOffset Length;
    gst_element_query_duration( m_OutputSink, &Format, &Length );
    return Length;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::EnableRecord( const wxString &recfile, const int format, const int quality )
{
    guLogDebug( wxT( "guFaderPlayBin::EnableRecord  %i %i '%s'" ), format, quality, recfile.c_str() );
    GstElement * Encoder = NULL;
    GstElement * Muxer = NULL;
    gint Mp3Quality[] = { 320, 192, 128, 96, 64 };
    float OggQuality[] = { 0.9, 0.7, 0.5, 0.3, 0.1 };
    gint FlacQuality[] = { 8, 7, 5, 3, 1 };

    //
    switch( format )
    {
        case guRECORD_FORMAT_MP3  :
        {
            Encoder = gst_element_factory_make( "lame", "rb_lame" );
            if( IsValidElement( Encoder ) )
            {
                g_object_set( Encoder, "bitrate", Mp3Quality[ quality ], NULL );
            }
            else
            {
                Encoder = NULL;
            }
            break;
        }

        case guRECORD_FORMAT_OGG  :
        {
            Encoder = gst_element_factory_make( "vorbisenc", "rb_vorbis" );
            if( IsValidElement( Encoder ) )
            {
                g_object_set( Encoder, "quality", OggQuality[ quality ], NULL );

                Muxer = gst_element_factory_make( "oggmux", "rb_oggmux" );
                if( !IsValidElement( Muxer ) )
                {
                    guLogError( wxT( "Could not create the record oggmux object" ) );
                    Muxer = NULL;
                }
            }
            else
            {
                Encoder = NULL;
            }
            break;
        }

        case guRECORD_FORMAT_FLAC :
        {
            Encoder = gst_element_factory_make( "flacenc", "rb_flac" );
            if( IsValidElement( Encoder ) )
            {
                g_object_set( Encoder, "quality", FlacQuality[ quality ], NULL );
            }
            else
            {
                Encoder = NULL;
            }
            break;
        }
    }

    if( Encoder )
    {
        if( BuildRecordBin( recfile, Encoder, Muxer ) )
        {
            bool NeedBlock = true;
            GstPad * AddPad;
            GstPad * BlockPad;

            AddPad = gst_element_get_static_pad( m_Tee, "sink" );
            BlockPad = gst_pad_get_peer( AddPad );
            gst_object_unref( AddPad );

            if( NeedBlock )
            {
                gst_pad_set_blocked_async( BlockPad, true, GstPadBlockCallback( add_record_element ), this );
            }
            else
            {
                add_record_element( BlockPad, false, this );
            }

            return true;
        }
        else
        {
            guLogMessage( wxT( "Could not build the recordbin object." ) );
        }
    }
    else
    {
        guLogError( wxT( "Could not create the recorder encoder object" ) );
    }

    return false;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::DisableRecord( void )
{
    guLogDebug( wxT( "guFaderPlayBin::DisableRecord" ) );
    bool NeedBlock = true;

    GstPad * AddPad;
    GstPad * BlockPad;

    AddPad = gst_element_get_static_pad( m_Tee, "sink" );
    BlockPad = gst_pad_get_peer( AddPad );
    gst_object_unref( AddPad );

    if( NeedBlock )
    {
        gst_pad_set_blocked_async( BlockPad, true, GstPadBlockCallback( remove_record_element ), this );
    }
    else
    {
        remove_record_element( BlockPad, false, this );
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::SetRecordFileName( const wxString &filename )
{
    guLogDebug( wxT( "guFaderPlayBin::SetRecrodFileName %i" ), m_IsBuffering );
    if( !m_RecordBin || m_IsBuffering )
        return false;

//    m_RecordFileName = m_RecordPath + filename + m_RecordExt;
    if( !wxDirExists( wxPathOnly( filename ) ) )
        wxFileName::Mkdir( wxPathOnly( filename ), 0770, wxPATH_MKDIR_FULL );

    gst_pad_set_blocked( m_RecordPad, true );

    if( gst_element_set_state( m_RecordBin, GST_STATE_NULL ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Could not reset recordbin state changing location" ) );
    }

    g_object_set( m_FileSink, "location", ( const char * ) filename.mb_str( wxConvFile ), NULL );

    //if( !set_state_and_wait( m_FileSink, PrevState, this ) )
    if( gst_element_set_state( m_RecordBin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Could not restore recordbin state changing location" ) );
    }

    if( gst_element_set_state( m_RecordBin, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Could not set running recordbin changing location" ) );
    }

    gst_pad_set_blocked( m_RecordPad, false );

    return true;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::AddRecordElement( GstPad * pad, bool isblocked )
{
    guLogDebug( wxT( "guFaderPlayBin::AddRecordElement" ) );
	gst_bin_add( GST_BIN( m_Playbackbin ), m_RecordBin );
    m_RecordPad = gst_element_get_request_pad( m_Tee, "src%d" );

	gst_pad_link( m_RecordPad, m_RecordGhostPad );

	// if we're supposed to be playing, unblock the sink */
	if( isblocked )
	{
		gst_element_set_state( m_Playbackbin, GST_STATE_PLAYING );
		gst_object_ref( m_RecordGhostPad );
		gst_pad_set_blocked_async( pad, false, GstPadBlockCallback( record_unlocked ), m_RecordGhostPad );
	}
	else
	{
		gst_element_set_state( m_Playbackbin, GST_STATE_PAUSED );
		gst_object_ref( m_RecordGhostPad );
		record_unlocked( NULL, false, m_RecordGhostPad );
	}
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::RemoveRecordElement( GstPad * pad, bool isblocked )
{
    guLogDebug( wxT( "guFaderPlayBin::RemoveRecordElement" ) );
    g_object_ref( m_RecordBin );
    gst_bin_remove( GST_BIN( m_Playbackbin ), m_RecordBin );

    gst_element_set_state( m_RecordBin, GST_STATE_NULL );
    g_object_unref( m_RecordBin );
    SetRecordBin( NULL );
    if( isblocked )
    {
        gst_pad_set_blocked_async( pad, false, GstPadBlockCallback( record_unlocked ), NULL );
    }
}





// -------------------------------------------------------------------------------- //
// guFaderTimeLine
// -------------------------------------------------------------------------------- //
guFaderTimeLine::guFaderTimeLine( const int timeout, wxEvtHandler * parent, guFaderPlayBin * faderplaybin,
    double volstart, double volend ) :
    guTimeLine( timeout, parent )
{
    m_FaderPlayBin = faderplaybin;
    m_VolStart = volstart;
    m_VolEnd = volend;

    if( volstart > volend )
        m_VolStep = volstart - volend;
    else
        m_VolStep = volend - volstart;

    //guLogDebug( wxT( "Created the fader timeline for %i msecs %0.2f -> %0.2f (%0.2f)" ), timeout, volstart, volend, m_VolStep );
}

// -------------------------------------------------------------------------------- //
guFaderTimeLine::~guFaderTimeLine()
{
    //guLogDebug( wxT( "Destroyed the fader timeline" ) );
}

// -------------------------------------------------------------------------------- //
void guFaderTimeLine::ValueChanged( float value )
{
    if( m_Direction == guTimeLine::Backward )
    {
        m_FaderPlayBin->SetFaderVolume( m_VolEnd + ( value * m_VolStep ) );
    }
    else
    {
        m_FaderPlayBin->SetFaderVolume( m_VolStart + ( value * m_VolStep ) );
    }
}

// -------------------------------------------------------------------------------- //
void guFaderTimeLine::Finished( void )
{
    m_FaderPlayBin->EndFade();
}

// -------------------------------------------------------------------------------- //
static bool TimerUpdated( guFaderTimeLine * timeline )
{
    timeline->TimerEvent();
    return true;
}

// -------------------------------------------------------------------------------- //
int guFaderTimeLine::TimerCreate( void )
{
    return g_timeout_add( m_UpdateInterval, GSourceFunc( TimerUpdated ), this );
}




// -------------------------------------------------------------------------------- //
// guMediaRecordCtrl
// -------------------------------------------------------------------------------- //
guMediaRecordCtrl::guMediaRecordCtrl( guPlayerPanel * playerpanel, guMediaCtrl * mediactrl )
{
    m_PlayerPanel = playerpanel;
    m_MediaCtrl = mediactrl;
    m_Recording = false;
    m_FirstChange = false;

    UpdatedConfig();
}

// -------------------------------------------------------------------------------- //
guMediaRecordCtrl::~guMediaRecordCtrl()
{
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::UpdatedConfig( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_MainPath = Config->ReadStr( wxT( "Path" ), wxGetHomeDir() + wxT( "/recordings" ), wxT( "Record" ) );
    m_Format = Config->ReadNum( wxT( "Format" ), guRECORD_FORMAT_MP3, wxT( "Record" ) );
    m_Quality = Config->ReadNum( wxT( "Quality" ), guRECORD_QUALITY_NORMAL, wxT( "Record" ) );
    m_SplitTracks = Config->ReadBool( wxT( "Split" ), false, wxT( "Record" ) );
    m_DeleteTracks = Config->ReadBool( wxT( "DeleteTracks" ), false, wxT( "Record" ) );
    m_DeleteTime = Config->ReadNum( wxT( "DeleteTime" ), 55, wxT( "Record" ) );

    if( !m_MainPath.EndsWith( wxT( "/" ) ) )
        m_MainPath += wxT( "/" );

    switch( m_Format )
    {
        case guRECORD_FORMAT_MP3 :
            m_Ext = wxT( ".mp3" );
            break;
        case guRECORD_FORMAT_OGG :
            m_Ext = wxT( ".ogg" );
            break;
        case guRECORD_FORMAT_FLAC :
            m_Ext = wxT( ".flac" );
            break;
    }

    //guLogDebug( wxT( "Record to '%s' %i, %i '%s'" ), m_MainPath.c_str(), m_Format, m_Quality, m_Ext.c_str() );
}

// -------------------------------------------------------------------------------- //
bool guMediaRecordCtrl::Start( const guTrack * track )
{
    guLogDebug( wxT( "guMediaRecordCtrl::Start" ) );
    m_TrackInfo = * track;

    if( m_TrackInfo.m_SongName.IsEmpty() )
        m_TrackInfo.m_SongName = wxT( "Record" );

    m_FileName = GetRecordFileName();

    wxFileName::Mkdir( wxPathOnly( m_FileName ), 0770, wxPATH_MKDIR_FULL );

    m_Recording = m_MediaCtrl->EnableRecord( m_FileName, m_Format, m_Quality );

    return m_Recording;
}

// -------------------------------------------------------------------------------- //
bool guMediaRecordCtrl::Stop( void )
{
    guLogDebug( wxT( "guMediaRecordCtrl::Stop" ) );
    if( m_Recording )
    {
        m_MediaCtrl->DisableRecord();
        m_Recording = false;

        SaveTagInfo( m_PrevFileName, &m_PrevTrack );
        m_PrevFileName = wxEmptyString;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMediaRecordCtrl::GetRecordFileName( void )
{
    wxString FileName = m_MainPath;

    if( !m_TrackInfo.m_AlbumName.IsEmpty() )
    {
        FileName += NormalizeField( m_TrackInfo.m_AlbumName );
    }
    else
    {
        wxURI Uri( m_TrackInfo.m_FileName );
        FileName += NormalizeField( Uri.GetServer() + wxT( "-" ) + Uri.GetPath() );
    }

    FileName += wxT( "/" );
    if( !m_TrackInfo.m_ArtistName.IsEmpty() )
    {
        FileName += NormalizeField( m_TrackInfo.m_ArtistName ) + wxT( " - " );
    }
    FileName += NormalizeField( m_TrackInfo.m_SongName ) + m_Ext;

    //guLogDebug( wxT( "The New Record Location is : '%s'" ), FileName.c_str() );
    return FileName;
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SplitTrack( void )
{
    m_FileName = GetRecordFileName();
    m_MediaCtrl->SetRecordFileName( m_FileName );

    SaveTagInfo( m_PrevFileName, &m_PrevTrack );

    m_PrevFileName = m_FileName;
    m_PrevTrack = m_TrackInfo;
}

// -------------------------------------------------------------------------------- //
bool guMediaRecordCtrl::SaveTagInfo( const wxString &filename, const guTrack * Track )
{
    bool RetVal = true;
    guTagInfo * TagInfo;

    if( !filename.IsEmpty() && wxFileExists( filename ) )
    {
        TagInfo = guGetTagInfoHandler( filename );

        if( TagInfo )
        {
            if( m_DeleteTracks )
            {
                TagInfo->Read();
                if( TagInfo->m_Length < m_DeleteTime )
                {
                    wxRemoveFile( filename );
                    delete TagInfo;
                    return true;
                }
            }

            TagInfo->m_AlbumName = Track->m_AlbumName;
            TagInfo->m_ArtistName = Track->m_ArtistName;
            TagInfo->m_GenreName = Track->m_GenreName;
            TagInfo->m_TrackName = Track->m_SongName;

            if( !( RetVal = TagInfo->Write() ) )
            {
                guLogError( wxT( "Could not set tags to the record track" ) );
            }

            delete TagInfo;
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SetTrack( const guTrack &track )
{
    m_TrackInfo = track;
    SplitTrack();
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SetTrackName( const wxString &artistname, const wxString &trackname )
{
    // If its the first file Set it so the tags are saved
    if( m_PrevFileName.IsEmpty() )
    {
        m_PrevFileName = m_FileName;
        m_PrevTrack = m_TrackInfo;
    }
    m_TrackInfo.m_ArtistName = artistname;
    m_TrackInfo.m_SongName = trackname;
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SetStation( const wxString &station )
{
    if( m_TrackInfo.m_AlbumName != station )
    {
        m_TrackInfo.m_AlbumName = station;
        SplitTrack();
    }
}

// -------------------------------------------------------------------------------- //
