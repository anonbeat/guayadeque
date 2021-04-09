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
#include "FaderPlaybin.h"

#include "LevelInfo.h"
#include "MediaCtrl.h"
#include "RadioTagInfo.h"
#include "GstPipelineBuilder.h"
#include "GstPipelineActuator.h"

#include <wx/wx.h>
#include <wx/url.h>

namespace Guayadeque {

// GstPlayFlags flags from playbin2. It is the policy of GStreamer to
// not publicly expose element-specific enums. That's why this
// GstPlayFlags enum has been copied here.
typedef enum {
    GST_PLAY_FLAG_VIDEO         = 0x00000001,
    GST_PLAY_FLAG_AUDIO         = 0x00000002,
    GST_PLAY_FLAG_TEXT          = 0x00000004,
    GST_PLAY_FLAG_VIS           = 0x00000008,
    GST_PLAY_FLAG_SOFT_VOLUME   = 0x00000010,
    GST_PLAY_FLAG_NATIVE_AUDIO  = 0x00000020,
    GST_PLAY_FLAG_NATIVE_VIDEO  = 0x00000040,
    GST_PLAY_FLAG_DOWNLOAD      = 0x00000080,
    GST_PLAY_FLAG_BUFFERING     = 0x000000100
} GstPlayFlags;


// -------------------------------------------------------------------------------- //
extern "C" {

static char ProxyServer[ 200 ] = "";
static char ProxyUser[ 200 ] = "";
static char ProxyPass[ 200 ] = "";

// -------------------------------------------------------------------------------- //
static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guFaderPlaybin * ctrl )
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

       case GST_MESSAGE_STATE_CHANGED:
       {
           GstState oldstate, newstate, pendingstate;
           gst_message_parse_state_changed( message, &oldstate, &newstate, &pendingstate );
//
           guLogDebug( wxT( "State changed %u -> %u (%u)" ), oldstate, newstate, pendingstate );
////            if( pendingstate == GST_STATE_VOID_PENDING )
////            {
////                wxMediaEvent event( wxEVT_MEDIA_STATECHANGED );
////                ctrl->AddPendingEvent( event );
////            }
           break;
       }

        case GST_MESSAGE_BUFFERING :
        {
            gint        Percent;
            gst_message_parse_buffering( message, &Percent );

            guLogDebug( wxT( "Buffering (%li): %i%%" ), ctrl->GetId(), Percent );

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
            #ifdef GU_DEBUG
            GstElement * pb = ctrl->Playbin();
            GstState st, ps;
            int gsr = gst_element_get_state( pb, &st, &ps, 5*GST_SECOND);
            guLogDebug("GST_MESSAGE_EOS gst_element_get_state=%i state=%i pending=%i", gsr, st, ps);
            #endif

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
            gchar * audio_codec = NULL;
            unsigned int bitrate = 0;
            // Extract from the message the GstTagList.
            //This generates a copy, so we must remember to free it.
            gst_message_parse_tag( message, &tags );

            guRadioTagInfo * RadioTagInfo = new guRadioTagInfo();

            gst_tag_list_get_string( tags, GST_TAG_ORGANIZATION, &RadioTagInfo->m_Organization );
            gst_tag_list_get_string( tags, GST_TAG_LOCATION, &RadioTagInfo->m_Location );
            gst_tag_list_get_string( tags, GST_TAG_TITLE, &RadioTagInfo->m_Title );
            gst_tag_list_get_string( tags, GST_TAG_GENRE, &RadioTagInfo->m_Genre );

            //guLogMessage( wxT( "New Tag Found:\n'%s'\n'%s'\n'%s'\n'%s'" ),
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

            if( gst_tag_list_get_string( tags, GST_TAG_AUDIO_CODEC, &audio_codec ) )
            {
                if( audio_codec )
                {
                    guMediaEvent event( guEVT_MEDIA_CHANGED_CODEC );
                    event.SetString( wxString::FromUTF8( audio_codec ) );
                    event.SetExtraLong( ctrl->GetId() );
                    ctrl->SendEvent( event );
                    g_free( audio_codec );
                }
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

                // guLogDebug( wxT( "MESSAGE_ELEMENT %s" ), wxString( name ).c_str() );
                if( !strcmp( name, "level" ) )
                {
                    guLevelInfo * LevelInfo = new guLevelInfo();
                    guMediaEvent event( guEVT_MEDIA_LEVELINFO );
                    gint channels;
                    const GValue * list;
                    const GValue * value;
                    GValueArray * avalue;

//                    if( !gst_structure_get_clock_time( s, "endtime", &LevelInfo->m_EndTime ) )
//                        guLogWarning( wxT( "Could not parse endtime" ) );
//
//                    LevelInfo->m_EndTime /= GST_MSECOND;

//                    GstFormat format = GST_FORMAT_TIME;
//                    if( gst_element_query_position( ctrl->OutputSink(), &format, ( gint64 * ) &LevelInfo->m_OutTime ) )
//                    {
//                        LevelInfo->m_OutTime /= GST_MSECOND;
//                    }

                    ////guLogDebug( wxT( "endtime: %" GST_TIME_FORMAT ", channels: %d" ), GST_TIME_ARGS( endtime ), channels );

                    // we can get the number of channels as the length of any of the value lists
                    list = gst_structure_get_value( s, "rms" );
                    avalue = ( GValueArray * ) g_value_get_boxed( list );

                    channels = avalue->n_values;
                    LevelInfo->m_Channels = channels;
                    //value = g_value_array_get_nth( avalue, 0 );
                    value = avalue->values;
                    LevelInfo->m_RMS_L = g_value_get_double( value );
                    if( channels > 1 )
                    {
                        //value = g_value_array_get_nth( avalue, 1 );
                        value = avalue->values + 1;
                        LevelInfo->m_RMS_R = g_value_get_double( value );
                    }

                    list = gst_structure_get_value( s, "peak" );
                    avalue = ( GValueArray * ) g_value_get_boxed( list );

                    //channels = avalue->n_values;
                    //value = g_value_array_get_nth( avalue, 0 );
                    value = avalue->values;
                    LevelInfo->m_Peak_L = g_value_get_double( value );
                    if( channels > 1 )
                    {
                        //value = g_value_array_get_nth( avalue, 1 );
                        value = avalue->values + 1;
                        LevelInfo->m_Peak_R = g_value_get_double( value );
                    }


                    list = gst_structure_get_value( s, "decay" );
                    avalue = ( GValueArray * ) g_value_get_boxed( list );

                    //value = g_value_array_get_nth( avalue, 0 );
                    value = avalue->values;
                    LevelInfo->m_Decay_L = g_value_get_double( value );
                    if( channels > 1 )
                    {
                        //value = g_value_array_get_nth( avalue, 1 );
                        value = avalue->values + 1;
                        LevelInfo->m_Decay_R = g_value_get_double( value );
                    }

                    // current timestamp - can be used further in the event
                    GstClockTime timestamp;
                    if( gst_structure_get_clock_time( s, "timestamp", &timestamp) )
                    {
                        // guLogDebug( "GST_MESSAGE_ELEMENT timestamp: %" GST_TIME_FORMAT, GST_TIME_ARGS(timestamp) );
                        LevelInfo->m_OutTime = timestamp / GST_MSECOND;
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
            // guLogDebug( wxT( "Got Application Message %s" ), GST_TO_WXSTRING( Name ).c_str() );

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
static void gst_about_to_finish( GstElement * playbin, guFaderPlaybin * ctrl )
{
    guLogDebug( "gst_about_to_finish" );
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
static void gst_audio_changed( GstElement * playbin, guFaderPlaybin * ctrl )
{
    guLogDebug( "gst_audio_changed" );
    ctrl->AudioChanged();
}

// -------------------------------------------------------------------------------- //
void gst_source_setup( GstElement * playbin, GstElement * source, guMediaCtrl * ctrl )
{
    guLogDebug( "gst_source_setup" );
    if( ctrl && ctrl->ProxyEnabled() )
    {
        if( g_object_class_find_property( G_OBJECT_GET_CLASS( source ), "proxy" ) )
        {
            //guLogMessage( wxT( "Found proxy property... '%s'" ), ctrl->ProxyServer().c_str() );
            strncpy( ProxyServer, ctrl->ProxyServer().char_str(), sizeof( ProxyServer ) - 1 );
            g_object_set( source,
                          "proxy", ProxyServer,
                          NULL );

            if( !ctrl->ProxyUser().IsEmpty() )
            {
                strncpy( ProxyUser, ctrl->ProxyUser().char_str(), sizeof( ProxyUser ) - 1 );
                strncpy( ProxyPass, ctrl->ProxyPass().char_str(), sizeof( ProxyPass ) - 1 );
                g_object_set( source,
                          "proxy-id", ProxyUser,
                          "proxy-pw", ProxyPass,
                          NULL );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
static bool seek_timeout( guFaderPlaybin * faderplaybin )
{
    faderplaybin->DoStartSeek();
    return false;
}

// -------------------------------------------------------------------------------- //
static bool pause_timeout( GstElement * playbin )
{
    guLogDebug("pause_timeout: GST_STATE_PAUSED");
    gst_element_set_state( playbin, GST_STATE_PAUSED );
    return false;
}

// -------------------------------------------------------------------------------- //
static GstPadProbeReturn record_locked( GstPad * pad, GstPadProbeInfo * info, guFaderPlaybin * ctrl )
{
    guLogDebug( wxT( "Record_Unlocked" ) );

    ctrl->SetRecordFileName();

    gst_pad_remove_probe( pad, GST_PAD_PROBE_INFO_ID( info ) );

    return GST_PAD_PROBE_OK;
}

// -------------------------------------------------------------------------------- //
static GstPadProbeReturn add_record_element( GstPad * pad, GstPadProbeInfo * info, guFaderPlaybin * ctrl )
{
    guLogDebug( wxT( "add_record_element" ) );

    ctrl->AddRecordElement( pad );

    gst_pad_remove_probe( pad, GST_PAD_PROBE_INFO_ID( info ) );

    return GST_PAD_PROBE_OK;
}

// -------------------------------------------------------------------------------- //
static GstPadProbeReturn remove_record_element( GstPad * pad, GstPadProbeInfo * info, guFaderPlaybin * ctrl )
{
    guLogDebug( wxT( "remove_record_element" ) );

    ctrl->RemoveRecordElement( pad );

    gst_pad_remove_probe( pad, GST_PAD_PROBE_INFO_ID( info ) );

    return GST_PAD_PROBE_OK;
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
// guFaderPlayBin
// -------------------------------------------------------------------------------- //
guFaderPlaybin::guFaderPlaybin( guMediaCtrl * mediactrl, const wxString &uri, const int playtype, const int startpos )
{
    //guLogDebug( wxT( "creating new stream for %s" ), uri.c_str() );
    m_Player = mediactrl;
    m_Uri = uri;
    m_Id = wxGetLocalTimeMillis().GetLo();
    m_State = guFADERPLAYBIN_STATE_WAITING;
    m_ErrorCode = 0;
    m_IsFading = false;
    m_IsBuffering = false;
    m_EmittedStartFadeIn = false;
    m_PlayType = playtype;
    m_FaderTimeLine = NULL;
    m_AboutToFinishPending = false;
    m_LastFadeVolume = -1;
    m_StartOffset = startpos;
    m_SeekTimerId = 0;
    m_SettingRecordFileName = false;
    m_PositionDelta = 0;

    guLogDebug( wxT( "guFaderPlayBin::guFaderPlayBin (%li)  %i" ), m_Id, playtype );

    if( BuildOutputBin() && BuildPlaybackBin() )
    {
        //Load( uri, false );
        if( startpos )
        {
            m_SeekTimerId = g_timeout_add( 100, GSourceFunc( seek_timeout ), this );
        }
        SetVolume( m_Player->GetVolume() );
        SetEqualizer( m_Player->GetEqualizer() );
    }
}

// -------------------------------------------------------------------------------- //
guFaderPlaybin::~guFaderPlaybin()
{
    guLogDebug( wxT( "guFaderPlayBin::~guFaderPlayBin (%li) e: %i" ), m_Id, m_ErrorCode );
    //m_Player->RemovePlayBin( this );
    if( m_SeekTimerId )
    {
        g_source_remove( m_SeekTimerId );
    }

    if( m_Playbin )
    {
        GstStateChangeReturn ChangeState = gst_element_set_state( m_Playbin, GST_STATE_NULL );
//        typedef enum {
//          GST_STATE_CHANGE_FAILURE             = 0,
//          GST_STATE_CHANGE_SUCCESS             = 1,
//          GST_STATE_CHANGE_ASYNC               = 2,
//          GST_STATE_CHANGE_NO_PREROLL          = 3
//        } GstSt
        //guLogMessage( wxT( "Set To NULL: %i" ), ChangeState );
        if( ChangeState == GST_STATE_CHANGE_ASYNC )
        {
            gst_element_get_state( m_Playbin, NULL, NULL, GST_CLOCK_TIME_NONE );
        }
        // guLogDebug( "mPlaybin refcount: %i", GST_OBJECT_REFCOUNT( m_Playbin ) );
        GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) );
        gst_bus_remove_watch( bus );
        // guLogDebug( "mPlaybin bus refcount: %i", GST_OBJECT_REFCOUNT( bus ) - 1 );
        gst_object_unref( bus );
        gst_object_unref( GST_OBJECT( m_Playbin ) );
        if( !m_Player->m_EnableVolCtls )
            gst_object_unref( GST_OBJECT( m_FaderVolume ) );
    }

    if( m_FaderTimeLine )
    {
        EndFade();
    }
    guLogDebug( wxT( "Finished destroying the playbin %li" ), m_Id );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::BuildOutputBin( void )
{
    GstElement * outputsink;

    const char * ElementNames[] = {
        "autoaudiosink",
        "gconfaudiosink",
        "alsasink",
        "pulsesink",
        "osssink",
        NULL
    };

    int OutputDevice = m_Player->OutputDevice();
    if( ( OutputDevice >= guOUTPUT_DEVICE_AUTOMATIC ) && ( OutputDevice < guOUTPUT_DEVICE_OTHER ) )
    {
        outputsink = gst_element_factory_make( ElementNames[ OutputDevice ], "OutputSink" );
        if( IsValidElement( outputsink ) )
        {
            if( OutputDevice > guOUTPUT_DEVICE_GCONF )
            {
                wxString OutputDeviceName = m_Player->OutputDeviceName();
                if( !OutputDeviceName.IsEmpty() )
                {
                    g_object_set( outputsink, "device", ( const char * ) OutputDeviceName.mb_str( wxConvFile ), NULL );
                }
            }
            m_OutputSink = outputsink;
            return true;
        }
    }
    else if( OutputDevice == guOUTPUT_DEVICE_OTHER )
    {
        wxString OutputDeviceName = m_Player->OutputDeviceName();
        if( !OutputDeviceName.IsEmpty() )
        {
            GError * err = NULL;
            outputsink = gst_parse_launch( ( const char * ) OutputDeviceName.mb_str( wxConvFile ), &err );
            if( outputsink )
            {
                if( err )
                {
                    guLogMessage( wxT( "Error building output pipeline: '%s'" ), wxString( err->message, wxConvUTF8 ).c_str() );
                    g_error_free( err );
                }
                m_OutputSink = outputsink;
                return true;
            }
        }
    }

    guLogError( wxT( "The configured audio output sink is not valid. Autoconfiguring..." ) );

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

    m_OutputSink = NULL;
    return false;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::BuildPlaybackBin( void )
{
//
// full playback pipeline including [removable] elements:
//      playbin > tee > queue > audioconvert > 
//        [ equalizer-10bands ] > [ rgvolume > rglimiter ] >
//        [ volume > fader volume ] > level > audioresample > sink
//
    guGstPipelineBuilder gpb( "playbackbin", &m_Playbackbin );

    gpb.Add( "playbin", "play", &m_Playbin, false,
        "uri", ( const char * ) m_Uri.mb_str( wxConvFile ),
        "buffer-size", gint( m_Player->BufferSize() * 1024 )
        );
    gpb.Add( "tee", "pb_tee", &m_Tee );

    gpb.Add( "queue", "pb_queue", NULL, true,
        "max-size-time", guint64( 250000000 ),
        "max-size-buffers", 0,
        "max-size-bytes", 0
        );

    gpb.Add( "audioconvert", "pb_audioconvert" );
    
    gpb.Add( "equalizer-10bands", "pb_equalizer", &m_Equalizer, m_Player->m_EnableEq );

    if( m_Player->m_ReplayGainMode )
    {
        gpb.Add( "rgvolume", "pb_rgvolume", &m_ReplayGain, true,
            "pre-amp", gdouble( m_Player->m_ReplayGainPreAmp ),
            "album-mode", gboolean( m_Player->m_ReplayGainMode == 2 )
            );        
        //g_object_set( G_OBJECT( m_ReplayGain ), "fallback-gain", gdouble( -6 ), NULL );
        gpb.Add( "rglimiter", "pb_rglimiter" );
    }
    else
        m_ReplayGain = NULL;

    gpb.Add( "volume", "pb_volume", &m_Volume, m_Player->m_EnableVolCtls );

    if( m_PlayType == guFADERPLAYBIN_PLAYTYPE_CROSSFADE )
        gpb.Add( "volume", "fader_volume", &m_FaderVolume, m_Player->m_EnableVolCtls,
            "volume", gdouble( 0.0 )
            );
    else
        gpb.Add( "volume", "fader_volume", &m_FaderVolume, m_Player->m_EnableVolCtls );

    gpb.Add( "level", "pb_level", NULL, true,
        "message", gboolean( true ),
        "interval", guint64( 100000000 ),
        "peak-falloff", gdouble( 6.0 ),
        "peak-ttl", guint64( 3 * 300000000 )
        );

    // gpb.Add( "audioconvert", "sink_audioconvert" );
    gpb.Add( "audioresample", "sink_audioresample", NULL, true,
        "quality", gint( 10 )
        );

    gpb.Link( m_OutputSink );

    if( !gpb.CanPlay() )
        return false;

    guLogDebug("guFaderPlaybin::BuildPlaybackBin pipeline is built");
    GstPad * pad = gst_element_get_static_pad( m_Tee, "sink" );
    if( GST_IS_PAD( pad ) )
    {
        GstPad * ghostpad = gst_ghost_pad_new( "sink", pad );
        gst_element_add_pad( m_Playbackbin, ghostpad );
        gst_object_unref( pad );

        g_object_set( G_OBJECT( m_Playbin ), "audio-sink", m_Playbackbin, NULL );

        g_object_set( G_OBJECT( m_Playbin ), "flags", GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_SOFT_VOLUME, NULL );

        g_signal_connect( G_OBJECT( m_Playbin ), "about-to-finish",
            G_CALLBACK( gst_about_to_finish ), ( void * ) this );
        //
        g_signal_connect( G_OBJECT( m_Playbin ), "audio-changed",
            G_CALLBACK( gst_audio_changed ), ( void * ) this );

        g_signal_connect( G_OBJECT( m_Playbin ), "source-setup",
            G_CALLBACK( gst_source_setup ), ( void * ) m_Player );

        GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) );
        gst_bus_add_watch( bus, GstBusFunc( gst_bus_async_callback ), this );
        gst_object_unref( bus );

        gpb.SetCleanup( false );

        m_PlayChain = gpb.GetChain();

        return true;
    }
    else
    {
        if( G_IS_OBJECT( pad ) )
            gst_object_unref( pad );
        guLogError( wxT( "Could not create the pad element" ) );
    }

    return false;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::BuildRecordBin( const wxString &path, GstElement * encoder, GstElement * muxer )
{
    guLogDebug( wxT( "BuildRecordBin( '%s' )" ), path.c_str() );

    guGstPipelineBuilder gpb( "gurb_recordbin", &m_RecordBin );
    g_object_set( m_RecordBin, "async-handling", gboolean( true ), NULL );
    GstElement * queue = gpb.Add( "queue", "gurb_queue", NULL, true,
                                "max-size-buffers", guint( 3 ),
                                "max-size-time", 0,
                                "max-size-bytes", 0
                                );
    gpb.Add( "audioconvert", "gurb_audioconvert" );
    gpb.Add( "audioresample", "gurb_audioresample" , NULL, true,
        "quality", gint( 10 )
        );
    gpb.Link( encoder );
    if( muxer )
        gpb.Link( muxer );
    gpb.Add( "filesink", "gurb_filesink", &m_FileSink, true,
        "location", ( const char * ) path.mb_str( wxConvFile )
        );

    if( !gpb.CanPlay() )
        return false;

    GstPad * pad = gst_element_get_static_pad( queue, "sink" );
    if( GST_IS_PAD( pad ) )
    {
        m_RecordSinkPad = gst_ghost_pad_new( "sink", pad );
        gst_element_add_pad( m_RecordBin, m_RecordSinkPad );
        gst_object_unref( pad );

        gpb.SetCleanup( false );

        return true;
    }
    else
    {
        if( G_IS_OBJECT( pad ) )
            gst_object_unref( pad );
        guLogError( wxT( "Could not create the pad element" ) );
    }

    return false;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::DoStartSeek( void )
{
    guLogDebug( wxT( "DoStartSeek( %i )" ), m_StartOffset );
    m_SeekTimerId = 0;
    if( GST_IS_ELEMENT( m_Playbin ) )
    {
        return Seek( m_StartOffset, true );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::SendEvent( guMediaEvent &event )
{
    m_Player->SendEvent( event );
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::SetBuffering( const bool isbuffering )
{
    if( m_IsBuffering != isbuffering )
    {
        m_IsBuffering = isbuffering;
        if( !isbuffering )
        {
            if( !m_PendingNewRecordName.IsEmpty() )
            {
                SetRecordFileName( m_PendingNewRecordName );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::SetVolume( double volume )
{
    guLogDebug( wxT( "guFaderPlayBin::SetVolume (%li)  %0.2f" ), m_Id, volume );
    g_object_set( m_Volume, "volume", gdouble( wxMax( 0.0001, volume ) ), NULL );
    return true;
}

// -------------------------------------------------------------------------------- //
double guFaderPlaybin::GetFaderVolume( void )
{
    double RetVal;
    g_object_get( m_FaderVolume, "volume", &RetVal, NULL );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::FadeInStart( void )
{
    m_EmittedStartFadeIn = true;
    m_Player->FadeInStart();
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::FadeOutDone( void )
{
     m_Player->FadeOutDone( this );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::SetFaderVolume( double volume )
{
    const char * Message = NULL;
    //guLogMessage( wxT( "Set the VolEnd: %0.2f" ), volume );

    if( volume != m_LastFadeVolume )
    {
        //guLogDebug( wxT( "guFaderPlayBin::SetFaderVolume (%li)   %0.2f  %i" ), m_Id, volume, m_State == guFADERPLAYBIN_STATE_FADEIN );
        Lock();
        m_LastFadeVolume = volume;
        g_object_set( m_FaderVolume, "volume", gdouble( volume ), NULL );

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

        Struct = gst_structure_new( Message, NULL, NULL );
        Msg = gst_message_new_application( GST_OBJECT( m_Playbin ), Struct );
        gst_element_post_message( GST_ELEMENT( m_Playbin ), Msg );
    }

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::SetEqualizer( const wxArrayInt &eqbands )
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
void guFaderPlaybin::SetEqualizerBand( const int band, const int value )
{
    if( m_Equalizer != NULL )
        g_object_set( G_OBJECT( m_Equalizer ), wxString::Format( wxT( "band%u" ),
                band ).char_str(), gdouble( value / 10.0 ), NULL );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Load( const wxString &uri, const bool restart, const int startpos )
{
    guLogDebug( wxT( "guFaderPlayBin::Load (%li) %i" ), m_Id, restart );
    if( GST_IS_ELEMENT( m_Playbin ) )

    if( restart )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogDebug( wxT( "guFaderPlayBin::Load => Could not set state to ready..." ) );
            return false;
        }
        gst_element_set_state( m_Playbin, GST_STATE_NULL );
    }

    if( !gst_uri_is_valid( ( const char * ) uri.mb_str( wxConvFile ) ) )
    {
        guLogDebug( wxT( "guFaderPlayBin::Load => Invalid uri: '%s'" ), uri.c_str() );
        return false;
    }

    g_object_set( G_OBJECT( m_Playbin ), "uri", ( const char * ) uri.mb_str( wxConvFile ), NULL );

    if( restart )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogDebug( wxT( "guFaderPlayBin::Load => Could not restore state to paused..." ) );
            return false;
        }
    }
    m_PositionDelta = startpos - gst_clock_get_time( gst_element_get_clock( m_Playbin ) );

    if( startpos )
    {
        m_StartOffset = startpos;
        m_SeekTimerId = g_timeout_add( 100, GSourceFunc( seek_timeout ), this );
    }

    guMediaEvent event( guEVT_MEDIA_LOADED );
    event.SetInt( restart );
    SendEvent( event );
    guLogDebug( wxT( "guFaderPlayBin::Load => Sent the loaded event..." ) );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Play( void )
{
    guLogDebug( wxT( "guFaderPlayBin::Play (%li)" ), m_Id );
    return ( gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) != GST_STATE_CHANGE_FAILURE );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::StartPlay( void )
{
    guLogMessage( wxT( "guFaderPlayBin::StartPlay (%li)" ), m_Id );
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
                guFaderPlaybin * FaderPlaybin = m_Player->m_FaderPlayBins[ Index ];

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
                int    FadeOutTime = m_Player->m_ForceGapless ? 0 : m_Player->m_FadeOutTime;

                guFaderPlaybin * FaderPlaybin = ToFade[ Index ];

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
                guFaderPlaybin * FaderPlaybin = m_Player->m_FaderPlayBins[ Index ];

                if( FaderPlaybin == this )
                    continue;

                switch( FaderPlaybin->m_State )
                {
                    case guFADERPLAYBIN_STATE_PLAYING :
                    case guFADERPLAYBIN_STATE_FADEIN :
                    case guFADERPLAYBIN_STATE_FADEOUT_PAUSE :
                        guLogDebug( wxT( "Stream %s already playing" ), FaderPlaybin->m_Uri.c_str() );
                        Playing = true;
                        break;

                    case guFADERPLAYBIN_STATE_PAUSED :
                        guLogDebug( wxT( "stream %s is paused; replacing it" ), FaderPlaybin->m_Uri.c_str() );
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
                guFaderPlaybin * FaderPlaybin = m_Player->m_FaderPlayBins[ Index ];

                if( FaderPlaybin == this )
                    continue;

                switch( FaderPlaybin->m_State )
                {
                    case guFADERPLAYBIN_STATE_PLAYING :
                    case guFADERPLAYBIN_STATE_PAUSED :
                    case guFADERPLAYBIN_STATE_FADEIN :
                    case guFADERPLAYBIN_STATE_PENDING_REMOVE :
                        // kill this one
                        guLogDebug( wxT( "stopping stream %s (replaced by new stream)" ), FaderPlaybin->m_Uri.c_str() );
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
void guFaderPlaybin::AboutToFinish( void )
{
    guLogDebug( wxT( "guFaderPlayBin::AboutToFinish (%li)" ), m_Id );
    Load( m_NextUri, false );
    m_NextUri = wxEmptyString;
    m_AboutToFinishPending = true;
}

// -------------------------------------------------------------------------------- //
static bool reset_about_to_finish( guFaderPlaybin * faderplaybin )
{
    faderplaybin->ResetAboutToFinishPending();
    return false;
}


// -------------------------------------------------------------------------------- //
void guFaderPlaybin::AudioChanged( void )
{
    guLogDebug( wxT( "guFaderPlayBin::AudioChanged (%li)" ), m_Id );
    if( m_AboutToFinishPending )
    {
        if( m_NextId )
        {
            m_Id = m_NextId;
            m_NextId = 0;
        }

        guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
        event.SetInt( GST_STATE_PLAYING );
        SendEvent( event );
        //m_AboutToFinishPending = false;
        m_AboutToFinishPendingId = g_timeout_add( 3000, GSourceFunc( reset_about_to_finish ), this );
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::StartFade( double volstart, double volend, int timeout )
{
    guLogDebug( wxT( "guFaderPlayBin::StartFade (%li) %0.2f, %0.2f, %i" ), m_Id, volstart, volend, timeout );
//    SetFaderVolume( volstart );

    m_EmittedStartFadeIn = false;
    m_IsFading = true;
    if( m_FaderTimeLine )
    {
//        guLogDebug( wxT( "Reversed the fader for %li" ), m_Id );
//        m_FaderTimeLine->ToggleDirection();
        EndFade();
    }

    m_FaderTimeLine = new guFaderTimeLine( timeout, NULL, this, volstart, volend );
    m_FaderTimeLine->SetDirection( volstart > volend ? guFaderTimeLine::Backward : guFaderTimeLine::Forward );
    m_FaderTimeLine->SetCurveShape( guTimeLine::EaseInOutCurve );
    m_FaderTimeLine->Start();

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Pause( void )
{
    guLogDebug( wxT( "guFaderPlayBin::Pause (%li)" ), m_Id );
//    return ( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) != GST_STATE_CHANGE_FAILURE );
    g_timeout_add( 200, GSourceFunc( pause_timeout ), m_Playbin );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Stop( void )
{
    guLogDebug( wxT( "guFaderPlayBin::Stop (%li)" ), m_Id );
    return ( gst_element_set_state( m_Playbin, GST_STATE_READY ) != GST_STATE_CHANGE_FAILURE );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Seek( wxFileOffset where, bool accurate )
{
    guLogDebug( wxT( "guFaderPlayBin::Seek (%li) %li )" ), m_Id, where );

    GstSeekFlags SeekFlags = GstSeekFlags( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT );
    if( accurate )
        SeekFlags = GstSeekFlags( SeekFlags | GST_SEEK_FLAG_ACCURATE );

    gboolean seek_ok = gst_element_seek_simple( m_Playbin, GST_FORMAT_TIME, SeekFlags, where * GST_MSECOND );
    if( seek_ok )
        m_PositionDelta = where * GST_MSECOND - gst_clock_get_time( gst_element_get_clock( m_Playbin ) );

    return seek_ok;
}

// -------------------------------------------------------------------------------- //
wxFileOffset guFaderPlaybin::Position( void )
{
    wxFileOffset Position = 0;
    // guLogDebug( "guFaderPlaybin::Position start" );

    // feature R&D {
    #ifdef GU_DEBUG
    if( GST_IS_ELEMENT( m_Playbin ) )
    {
        // calculate the position from the sink total playback time
        // to support getting position if gst_element_query_position() is tripping
        GstClock * clock = gst_element_get_clock( m_Playbin );
        if( clock )
        {
            // delta is adjusted in Seek() and reset in Load()
            wxFileOffset dp = gst_clock_get_time( clock ) + m_PositionDelta;
            guLogDebug( "guFaderPlaybin::Position gst_element_get_clock: %lu", dp );
            // only logging for now
        }
    }
    #endif
    // } feature R&D

    if( Position <= 0 )
    {
        // actual position detection 
        gst_element_query_position( m_Playbin, GST_FORMAT_TIME, &Position );
        guLogDebug( "guFaderPlaybin::Position gst_element_query_position: %lu", Position );
    }

    return Position < 0 ? 0 : Position;
}

// -------------------------------------------------------------------------------- //
wxFileOffset guFaderPlaybin::Length( void )
{
    wxFileOffset Length;
    gst_element_query_duration( m_OutputSink, GST_FORMAT_TIME, ( gint64 * ) &Length );
    return Length;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::EnableRecord( const wxString &recfile, const int format, const int quality )
{
    guLogDebug( wxT( "guFaderPlayBin::EnableRecord  %i %i '%s'" ), format, quality, recfile.c_str() );
    GstElement * Encoder = NULL;
    GstElement * Muxer = NULL;
    gint Mp3Quality[] = { 320, 192, 128, 96, 64 };
    gfloat OggQuality[] = { 0.9f, 0.7f, 0.5f, 0.3f, 0.1f };
    gint FlacQuality[] = { 8, 7, 5, 3, 1 };

    //
    switch( format )
    {
        case guRECORD_FORMAT_MP3  :
        {
            Encoder = gst_element_factory_make( "lamemp3enc", "rb_lame" );
            if( IsValidElement( Encoder ) )
            {
                g_object_set( Encoder, "bitrate", Mp3Quality[ quality ], NULL );

                Muxer = gst_element_factory_make( "xingmux", "rb_xingmux" );
                if( !IsValidElement( Muxer ) )
                {
                    guLogError( wxT( "Could not create the record xingmux object" ) );
                    Muxer = NULL;
                }
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
            GstPad * AddPad;
            GstPad * BlockPad;

            AddPad = gst_element_get_static_pad( m_Tee, "sink" );
            BlockPad = gst_pad_get_peer( AddPad );
            gst_object_unref( AddPad );

            //gst_pad_set_blocked_async( BlockPad, true, GstPadBlockCallback( add_record_element ), this );
            gst_pad_add_probe( BlockPad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                GstPadProbeCallback( add_record_element ), this, NULL );

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
void guFaderPlaybin::DisableRecord( void )
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
        //gst_pad_set_blocked_async( BlockPad, true, GstPadBlockCallback( remove_record_element ), this );
        gst_pad_add_probe( BlockPad, GST_PAD_PROBE_TYPE_BLOCK_UPSTREAM,
            GstPadProbeCallback( remove_record_element ), this, NULL );
    }
    else
    {
        //remove_record_element( BlockPad, false, this );
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::SetRecordFileName( const wxString &filename )
{
    if( filename == m_LastRecordFileName )
        return true;

    if( !m_RecordBin || m_SettingRecordFileName )
        return false;

    if( m_IsBuffering )
    {
        m_PendingNewRecordName = filename;
        return true;
    }

    guLogDebug( wxT( "guFaderPlayBin::SetRecordFileName %i" ), m_IsBuffering );
    m_SettingRecordFileName = true;
    m_LastRecordFileName = filename;
    m_PendingNewRecordName.Clear();

    // Once the pad is locked returns into SetRecordFileName()
    gst_pad_add_probe( m_TeeSrcPad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
        GstPadProbeCallback( record_locked ), this, NULL );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::SetRecordFileName( void )
{
//    m_RecordFileName = m_RecordPath + filename + m_RecordExt;
    if( !wxDirExists( wxPathOnly( m_LastRecordFileName ) ) )
        wxFileName::Mkdir( wxPathOnly( m_LastRecordFileName ), 0770, wxPATH_MKDIR_FULL );

    if( gst_element_set_state( m_RecordBin, GST_STATE_NULL ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Could not reset recordbin state changing location" ) );
    }

    g_object_set( m_FileSink, "location", ( const char * ) m_LastRecordFileName.mb_str( wxConvFile ), NULL );

    if( gst_element_set_state( m_RecordBin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Could not restore recordbin state changing location" ) );
    }

    if( gst_element_set_state( m_RecordBin, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Could not set running recordbin changing location" ) );
    }

    m_SettingRecordFileName = false;

    return true;
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::AddRecordElement( GstPad * pad )
{
    gst_element_set_state( m_RecordBin, GST_STATE_PAUSED );

    gst_bin_add( GST_BIN( m_Playbackbin ), m_RecordBin );
    m_TeeSrcPad = gst_element_get_request_pad( m_Tee, "src_%u" );

    gst_pad_link( m_TeeSrcPad, m_RecordSinkPad );

    gst_element_set_state( m_RecordBin, GST_STATE_PLAYING );
    gst_object_ref( m_RecordSinkPad );
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::RemoveRecordElement( GstPad * pad )
{
    g_object_ref( m_RecordBin );
    gst_element_set_state( m_RecordBin, GST_STATE_PAUSED );

    gst_bin_remove( GST_BIN( m_Playbackbin ), m_RecordBin );

    gst_element_set_state( m_RecordBin, GST_STATE_NULL );
    g_object_unref( m_RecordBin );

    SetRecordBin( NULL );
}

void guFaderPlaybin::ToggleEqualizer( void )
{
    guLogDebug( "guFaderPlaybin::ToggleEqualizer" );
    guGstPipelineActuator gpa( m_PlayChain );
    gpa.Toggle( m_Equalizer );
}
}

// -------------------------------------------------------------------------------- //
