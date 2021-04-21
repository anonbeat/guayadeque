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
static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guFaderPlaybin::WeakPtr * wpp )
{
    if( wpp == NULL)
    {
        guLogTrace( "Gst async fail: parent fader playbin is null" );
        return FALSE;
    }
    auto sp = wpp->lock();
    if( !sp )
    {
        guLogTrace( "Gst async fail: parent fader playbin is gone" );
        delete wpp;
        return FALSE;
    }

    guFaderPlaybin * ctrl = (*sp);
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
static void gst_about_to_finish( GstElement * playbin, guFaderPlaybin::WeakPtr * wpp )
{
    guLogDebug( "gst_about_to_finish << %p", wpp );
    if( wpp == NULL)
    {
        guLogTrace( "Gst about to finish: parent fader playbin is null" );
        return;
    }
    auto sp = wpp->lock();
    if( !sp )
    {
        guLogTrace( "Gst about to finish: parent fader playbin is gone" );
        delete wpp;
        return;
    }

    guFaderPlaybin * ctrl = (*sp);
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
static void gst_audio_changed( GstElement * playbin, guFaderPlaybin::WeakPtr * wpp )
{
    guLogDebug( "gst_audio_changed << %p", wpp );
    if( wpp == NULL)
    {
        guLogTrace( "gst_audio_changed: parent fader playbin is null" );
        return;
    }
    if( auto sp = wpp->lock() )
    {
        (*sp)->AudioChanged();
    }
    else
    {
        guLogTrace( "gst_audio_changed: parent fader playbin is gone" );
        delete wpp;
    }
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
static bool seek_timeout( guFaderPlaybin::WeakPtr * wpp )
{
    if( auto sp = wpp->lock() )
        (*sp)->DoStartSeek();
    else
        guLogTrace( "Seek timeout fail: parent bin is gone" );

    delete wpp;
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
// guFaderPlaybin
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
    m_ReplayGain = NULL;
    m_ReplayGainLimiter = NULL;
    m_SharedPointer = std::make_shared<guFaderPlaybin*>( this );
    m_RecordBin = NULL;
    m_Valve = NULL;

    guLogDebug( wxT( "guFaderPlaybin::guFaderPlaybin (%li)  %i" ), m_Id, playtype );

    if( BuildOutputBin() && BuildPlaybackBin() )
    {
        //Load( uri, false );
        if( startpos )
        {
            m_SeekTimerId = g_timeout_add( 100, GSourceFunc( seek_timeout ), GetWeakPtr() );
        }
        SetVolume( m_Player->GetVolume() );
        SetEqualizer( m_Player->GetEqualizer() );
    }
}

// -------------------------------------------------------------------------------- //
guFaderPlaybin::~guFaderPlaybin()
{
    guLogDebug( wxT( "guFaderPlaybin::~guFaderPlaybin (%li) e: %i" ), m_Id, m_ErrorCode );
    if( m_RecordBin != NULL )
        guGstPipelineActuator( m_RecordBin ).Disable();
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
            guLogDebug( "guFaderPlaybin::~guFaderPlaybin wait on GST_STATE_CHANGE_ASYNC" );
            gst_element_get_state( m_Playbin, NULL, NULL, GST_SECOND );
        }
        // guLogDebug( "mPlaybin refcount: %i", GST_OBJECT_REFCOUNT( m_Playbin ) );
        GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) );
        gst_bus_remove_watch( bus );
        // guLogDebug( "mPlaybin bus refcount: %i", GST_OBJECT_REFCOUNT( bus ) - 1 );
        gst_object_unref( bus );
        gst_object_unref( GST_OBJECT( m_Playbin ) );

        guGstStateToNullAndUnref( m_FaderVolume );
        guGstStateToNullAndUnref( m_Volume );

        guGstStateToNullAndUnref( m_Equalizer ) ;

        guGstStateToNullAndUnref( m_ReplayGain );
        guGstStateToNullAndUnref( m_ReplayGainLimiter );
    }

    if( m_FaderTimeLine )
    {
        EndFade();
    }

    guGstStateToNullAndUnref( m_RecordBin );

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
//      playbin > valve > tee > queue > audioconvert > 
//        [ equalizer-10bands ] > [ rgvolume > rglimiter ] >
//        [ volume > fader volume ] > level > audioresample > sink
//
    guGstPipelineBuilder gpb( "playbackbin", &m_Playbackbin );

    gpb.Add( "playbin", "play", &m_Playbin, false,
        "uri", ( const char * ) m_Uri.mb_str( wxConvFile ),
        "buffer-size", gint( m_Player->BufferSize() * 1024 )
        );
    gpb.Add( "valve", "pb_valve", &m_Valve );
    gpb.Add( "tee", "pb_tee", &m_Tee );

    gpb.Add( "queue", "pb_queue", NULL, true,
        "max-size-time", guint64( 250000000 ),
        "max-size-buffers", 0,
        "max-size-bytes", 0
        );

    gpb.Add( "audioconvert", "pb_audioconvert" );
    
    // add with valve's since we want to mass-plug those & don't want be racing on pads
    gpb.AddV( "equalizer-10bands", "pb_equalizer", &m_Equalizer, m_Player->m_EnableEq );

    gpb.AddV( "rgvolume", "pb_rgvolume", &m_ReplayGain, m_Player->m_ReplayGainMode );
    SetRGProperties();
    gpb.AddV( "rglimiter", "pb_rglimiter", &m_ReplayGainLimiter, m_Player->m_ReplayGainMode );

    gpb.AddV( "volume", "pb_volume", &m_Volume, m_Player->m_EnableVolCtls );

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
    GstPad * pad = gst_element_get_static_pad( m_Valve, "sink" );
    if( GST_IS_PAD( pad ) )
    {
        GstPad * ghostpad = gst_ghost_pad_new( "sink", pad );
        gst_element_add_pad( m_Playbackbin, ghostpad );
        gst_object_unref( pad );

        g_object_set( G_OBJECT( m_Playbin ), "audio-sink", m_Playbackbin, NULL );

        g_object_set( G_OBJECT( m_Playbin ), "flags", GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_SOFT_VOLUME, NULL );

        g_signal_connect( G_OBJECT( m_Playbin ), "about-to-finish",
            G_CALLBACK( gst_about_to_finish ), ( void * ) GetWeakPtr() );
        //
        g_signal_connect( G_OBJECT( m_Playbin ), "audio-changed",
            G_CALLBACK( gst_audio_changed ), ( void * ) GetWeakPtr() );

        g_signal_connect( G_OBJECT( m_Playbin ), "source-setup",
            G_CALLBACK( gst_source_setup ), ( void * ) m_Player );

        GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) );
        gst_bus_add_watch( bus, GstBusFunc( gst_bus_async_callback ), GetWeakPtr() );
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
bool guFaderPlaybin::BuildRecordBin( const wxString &path )
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
    gpb.Link( m_Encoder );
    if( m_Muxer != NULL )
        gpb.Link( m_Muxer );
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
    guLogDebug( wxT( "guFaderPlaybin::SetVolume (%li)  %0.2f" ), m_Id, volume );
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
        //guLogDebug( wxT( "guFaderPlaybin::SetFaderVolume (%li)   %0.2f  %i" ), m_Id, volume, m_State == guFADERPLAYBIN_STATE_FADEIN );
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
    guLogDebug( wxT( "guFaderPlaybin::Load (%li) %i" ), m_Id, restart );
    
    DisableRecord();

    if( GST_IS_ELEMENT( m_Playbin ) )

    if( restart )
    {
        // recording data loss here => do not reuse recording bins
        if( gst_element_set_state( m_Playbin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogDebug( wxT( "guFaderPlaybin::Load => Could not set state to ready..." ) );
            return false;
        }
        gst_element_set_state( m_Playbin, GST_STATE_NULL );
    }

    if( !gst_uri_is_valid( ( const char * ) uri.mb_str( wxConvFile ) ) )
    {
        guLogDebug( wxT( "guFaderPlaybin::Load => Invalid uri: '%s'" ), uri.c_str() );
        return false;
    }

    g_object_set( G_OBJECT( m_Playbin ), "uri", ( const char * ) uri.mb_str( wxConvFile ), NULL );

    if( restart )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogDebug( wxT( "guFaderPlaybin::Load => Could not restore state to paused..." ) );
            return false;
        }
    }
    m_PositionDelta = startpos - gst_clock_get_time( gst_element_get_clock( m_Playbin ) );

    if( startpos )
    {
        m_StartOffset = startpos;
        m_SeekTimerId = g_timeout_add( 100, GSourceFunc( seek_timeout ), GetWeakPtr() );
    }

    guMediaEvent event( guEVT_MEDIA_LOADED );
    event.SetInt( restart );
    SendEvent( event );
    guLogDebug( wxT( "guFaderPlaybin::Load => Sent the loaded event..." ) );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Play( void )
{
    guLogDebug( wxT( "guFaderPlaybin::Play (%li)" ), m_Id );
    if( m_State != guFADERPLAYBIN_STATE_PENDING_REMOVE )
    {
        SetValveDrop( false );
        return ( gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) != GST_STATE_CHANGE_FAILURE );
    }
    else
    {
        return false;
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::StartPlay( void )
{
    guLogMessage( wxT( "guFaderPlaybin::StartPlay (%li)" ), m_Id );
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
    guLogDebug( wxT( "guFaderPlaybin::AboutToFinish (%li)" ), m_Id );
    Load( m_NextUri, false );
    m_NextUri = wxEmptyString;
    m_AboutToFinishPending = true;
}

// -------------------------------------------------------------------------------- //
static bool guFaderPlaybin__AudioChanged_timeout( guFaderPlaybin::WeakPtr * wpp )
{
    guLogDebug( "guFaderPlaybin__AudioChanged_timeout << %p", wpp );
    if( auto sp = wpp->lock() )
    {
        (*sp)->ResetAboutToFinishPending();
    }
    else
    {
        guLogTrace( "Audio changed event: parent fader playbin is gone" );
    }
    delete wpp;
    return false;
}


// -------------------------------------------------------------------------------- //
void guFaderPlaybin::AudioChanged( void )
{
    guLogDebug( wxT( "guFaderPlaybin::AudioChanged (%li)" ), m_Id );
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
        m_AboutToFinishPendingId = g_timeout_add( 3000, GSourceFunc( guFaderPlaybin__AudioChanged_timeout ), GetWeakPtr() );
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::StartFade( double volstart, double volend, int timeout )
{
    guLogDebug( wxT( "guFaderPlaybin::StartFade (%li) %0.2f, %0.2f, %i" ), m_Id, volstart, volend, timeout );
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
    guLogDebug( wxT( "guFaderPlaybin::Pause (%li)" ), m_Id );
    
    // we do not pause while recording, filesinks don't like it
    if( IsRecording() )
    {
        guLogTrace( "Pause: can not pause while recording" );
        return false;
    }
    else
    {
        guLogDebug( "guFaderPlaybin::Pause ok" );
        g_timeout_add( 200, GSourceFunc( pause_timeout ), m_Playbin );
        return true;
    }
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Stop( void )
{
    guLogDebug( wxT( "guFaderPlaybin::Stop (%li)" ), m_Id );
    SetValveDrop( true );
    SetBuffering( false );
    SetVolume( 0 );
    if( m_RecordBin == NULL )
        return ( gst_element_set_state( m_Playbin, GST_STATE_READY ) != GST_STATE_CHANGE_FAILURE );
    else
        return DisableRecordAndStop(); // async soft-stop
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::Seek( wxFileOffset where, bool accurate )
{
    guLogDebug( wxT( "guFaderPlaybin::Seek (%li) %li )" ), m_Id, where );

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

    // actual position detection 
    gst_element_query_position( m_Playbin, GST_FORMAT_TIME, &Position );

    // feature R&D {
    #ifdef GU_DEBUG
        wxFileOffset play_pos_estimation = 0;
        if( GST_IS_ELEMENT( m_Playbin ) )
        {
            // calculate the position from the sink total playback time
            // to support getting position if gst_element_query_position() is tripping
            GstClock * playbin_clock = gst_element_get_clock( m_Playbin );
            if( playbin_clock )
            {
                // delta is adjusted in Seek() and reset in Load()
                play_pos_estimation = gst_clock_get_time( playbin_clock ) + m_PositionDelta;
                // only as stats for now
            }
            gst_object_unref( playbin_clock );
        }
        guLogStats( "track position: estimated=%lu real=%lu", play_pos_estimation, Position );
    #endif
    // } feature R&D

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
bool guFaderPlaybin::AddRecordElement( GstPad * pad )
{
    guLogGstPadData( "guFaderPlaybin::AddRecordElement << ", pad );

    if( !gst_bin_add( GST_BIN( m_Playbackbin ), m_RecordBin ) )
    {
        guLogError( "Record error: failed to add recorder into the pipeline" );
        return false;
    }

    if( !gst_element_sync_state_with_parent( m_RecordBin ) )
    {
        guLogError( "Record error: unable to set recorder state" );
        return false;
    }

    m_TeeSrcPad = gst_element_get_request_pad( m_Tee, "src_%u" );
    guLogGstPadData( "guFaderPlaybin::AddRecordElement src request pad", m_TeeSrcPad );
    if( m_TeeSrcPad == NULL )
    {
        guLogError( "Record error: request pad is null" );
        return false;
    }
    
    int lres = gst_pad_link( m_TeeSrcPad, m_RecordSinkPad );
    if( lres == GST_PAD_LINK_OK )
    {
        guLogDebug( "guFaderPlaybin::AddRecordElement link ok" );
        return true;
    }
    guLogError( "Record error: pads do not link (code %i)", lres );
    return false;
    // gst_object_ref( m_RecordSinkPad );
}


// -------------------------------------------------------------------------------- //
static GstPadProbeReturn guFaderPlaybin__EnableRecord( GstPad * pad, GstPadProbeInfo * info, guFaderPlaybin::WeakPtr * wpp )
{
    guLogDebug( "guFaderPlaybin__EnableRecord << %p", wpp );
    if( auto sp = wpp->lock() )
    {
        if( (*sp)->AddRecordElement( pad ) )
            guLogDebug( "guFaderPlaybin__EnableRecord recorder added" );
        else
            guLogTrace( "Recorder fail" );
        guMediaEvent e( guEVT_PIPELINE_CHANGED );
        e.SetClientData( wpp );
        (*sp)->SendEvent( e ); // returns in ::RefreshPlaybackItems
    }
    else
    {
        guLogTrace( "Enable recording: parent fader playbin is gone" );
        delete wpp;
    }

    guLogDebug( "guFaderPlaybin__EnableRecord >>" );

    return GST_PAD_PROBE_REMOVE;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::EnableRecord( const wxString &recfile, const int format, const int quality )
{
    guLogDebug( wxT( "guFaderPlaybin::EnableRecord  %i %i '%s'" ), format, quality, recfile.c_str() );
    m_Encoder = NULL;
    m_Muxer = NULL;
    gint Mp3Quality[] = { 320, 192, 128, 96, 64 };
    gfloat OggQuality[] = { 0.9f, 0.7f, 0.5f, 0.3f, 0.1f };
    gint FlacQuality[] = { 8, 7, 5, 3, 1 };

    //
    switch( format )
    {
        case guRECORD_FORMAT_MP3  :
        {
            m_Encoder = gst_element_factory_make( "lamemp3enc", "rb_lame" );
            if( IsValidElement( m_Encoder ) )
            {
                g_object_set( m_Encoder, "bitrate", Mp3Quality[ quality ], NULL );

                m_Muxer = gst_element_factory_make( "xingmux", "rb_xingmux" );
                if( !IsValidElement( m_Muxer ) )
                {
                    guLogError( wxT( "Could not create the record xingmux object" ) );
                    m_Muxer = NULL;
                }
            }
            else
            {
                m_Encoder = NULL;
            }
            break;
        }

        case guRECORD_FORMAT_OGG  :
        {
            m_Encoder = gst_element_factory_make( "vorbisenc", "rb_vorbis" );
            if( IsValidElement( m_Encoder ) )
            {
                g_object_set( m_Encoder, "quality", OggQuality[ quality ], NULL );

                m_Muxer = gst_element_factory_make( "oggmux", "rb_oggmux" );
                if( !IsValidElement( m_Muxer ) )
                {
                    guLogError( wxT( "Could not create the record oggmux object" ) );
                    m_Muxer = NULL;
                }
            }
            else
            {
                m_Encoder = NULL;
            }
            break;
        }

        case guRECORD_FORMAT_FLAC :
        {
            m_Encoder = gst_element_factory_make( "flacenc", "rb_flac" );
            if( IsValidElement( m_Encoder ) )
            {
                g_object_set( m_Encoder, "quality", FlacQuality[ quality ], NULL );
            }
            else
            {
                m_Encoder = NULL;
            }
            break;
        }
    }

    if( m_Encoder )
    {
        if( BuildRecordBin( recfile ) )
        {
            guGstPtr<GstPad> AddPad( gst_element_get_static_pad( m_Tee, "sink" ) );
            guGstPtr<GstPad> IdlePad( gst_pad_get_peer( AddPad.ptr ) );

            WeakPtr * p = GetWeakPtr();
            if( gst_pad_add_probe( IdlePad.ptr, GST_PAD_PROBE_TYPE_IDLE,
                GstPadProbeCallback( guFaderPlaybin__EnableRecord ), p, NULL ) )
            {
                guLogDebug( "guFaderPlaybin::EnableRecord probe added" );
                return true;
            }
            else
            {
                if( guIsGstElementLinked( m_RecordBin ) )
                {
                    guLogDebug( "guFaderPlaybin::EnableRecord enabled in the current thread" );
                    return true;
                }
                else
                {
                    guLogError( "Record error: unable to add element probe" );
                }
                delete p;
            }
        }
        else
        {
            guLogMessage( "Record error: could not build the recordbin object." );
        }
    }
    else
    {
        guLogError( "Record error: could not create the encoder object" );
    }

    return false;
}

// -------------------------------------------------------------------------------- //
static void guFaderPlaybin__RefreshPlaybackItems( guFaderPlaybin::WeakPtr * wpp )
{
    guLogDebug( "guFaderPlaybin__RefreshPlaybackItems << %p", wpp );
    if( auto sp = wpp->lock() )
    {
        // this function runs inside pad probe thread, so
        // elements state changes should happen in another
        guLogDebug( "guFaderPlaybin__RefreshPlaybackItems found the bin" );
        guMediaEvent e( guEVT_PIPELINE_CHANGED );
        e.SetClientData( wpp );
        (*sp)->SendEvent( e ); // returns in ::RefreshPlaybackItems
    }
    else
    {
        guLogTrace( "Refresh pipeline elements: parent fader playbin is gone" );
        delete wpp;
    }
    guLogDebug( "guFaderPlaybin__RefreshPlaybackItems >>" );
}

// -------------------------------------------------------------------------------- //
static bool guFaderPlaybin__ElementCleanup_finish( GstElement * element )
{
    guGstStateToNullAndUnref( element );
    return false;
}

// -------------------------------------------------------------------------------- //
static void guFaderPlaybin__ElementCleanup( GstElement * element )
{
    guLogDebug( "guFaderPlaybin__ElementCleanup << %p", element );
    g_timeout_add( 10, GSourceFunc( guFaderPlaybin__ElementCleanup_finish ), element );
}

// this should be used only to stop the recording while playing
// use ::DisableRecordAndStop if you need to stop playback ensuring recording data safety
// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::DisableRecord( void )
{

    guLogDebug( "guFaderPlaybin::DisableRecord" );
    if( m_RecordBin == NULL || !guIsGstElementLinked( m_RecordBin ) )
        return true;

    guGstPipelineActuator gpa( m_RecordBin );
    guGstResultHandler rh(
        guGstResultHandler::Func( guFaderPlaybin__ElementCleanup ),
        m_RecordBin
    );
    gpa.SetHandler( &rh );
    m_RecordBin = NULL;
    return gpa.Disable();
}

// guFaderPlaybin::DisableRecordAndStop callout {
// -------------------------------------------------------------------------------- //
void guFaderPlaybin::DisableRecordAndStop_finish( void )
{
    guGstStateToNullAndUnref( m_RecordBin );
    m_RecordBin = NULL;
    gst_element_set_state( m_Playbin, GST_STATE_READY );
    guMediaEvent e( guEVT_PIPELINE_CHANGED );
    e.SetClientData( NULL ); 
    SendEvent( e ); // will not return
}

// -------------------------------------------------------------------------------- //
static bool guFaderPlaybin__DisableRecordAndStop_finish( guFaderPlaybin::WeakPtr * wpp )
{
    if( auto sp = wpp->lock() )
    {
        guLogDebug( "guFaderPlaybin__DisableRecordAndStop_finish found the bin" );
        (*sp)->DisableRecordAndStop_finish();
    }
    else
    {
        guLogTrace( "Disable record: parent fader playbin is gone" );
    }
    delete wpp;
    return false;
}

// -------------------------------------------------------------------------------- //
static void guFaderPlaybin__DisableRecordAndStop( guFaderPlaybin::WeakPtr * wpp )
{
    // may run in gstreamer thread
    guLogDebug( "guFaderPlaybin__DisableRecordAndStop << %p", wpp );
    g_timeout_add( 10, GSourceFunc( guFaderPlaybin__DisableRecordAndStop_finish ), wpp );
}

// clean way to stop playback while recording without loosing data 
// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::DisableRecordAndStop( void )
{

    guLogDebug( "guFaderPlaybin::DisableRecordAndStop" );
    if( m_RecordBin == NULL )
        return true;
    if( !guIsGstElementLinked( m_RecordBin ) )
    {
        guGstStateToNullAndUnref( m_RecordBin );
        m_RecordBin = NULL;
        return true;
    }

    guGstPipelineActuator gpa( m_RecordBin );
    guGstResultHandler rh(
        guGstResultHandler::Func( guFaderPlaybin__DisableRecordAndStop ),
        GetWeakPtr()
    );
    gpa.SetHandler( &rh );
    return gpa.Disable();
}
// } guFaderPlaybin::DisableRecordAndStop callout

// -------------------------------------------------------------------------------- //
bool guFaderPlaybin::SetRecordFileName( const wxString &filename )
{
    if( filename.IsNull() )
        return false;
    if( filename == m_LastRecordFileName )
        return true;

    if( !m_RecordBin || m_SettingRecordFileName )
        return false;

    if( m_IsBuffering )
    {
        m_PendingNewRecordName = filename;
        return true;
    }

    guLogDebug( "guFaderPlaybin::SetRecordFileName << %s (%i)", filename, m_IsBuffering );
    m_SettingRecordFileName = true;
    m_LastRecordFileName = filename;
    m_PendingNewRecordName.Clear();
  
    if( !wxDirExists( wxPathOnly( m_LastRecordFileName ) ) )
        wxFileName::Mkdir( wxPathOnly( m_LastRecordFileName ), 0770, wxPATH_MKDIR_FULL );

    static int sink_count = 0;
    std::string sink_name = "gurb_filesink_" + std::to_string( ++sink_count );
    GstElement * new_sink = gst_element_factory_make( "filesink", sink_name.c_str() );
    if( !IsValidElement( new_sink ) )
    {
        guLogError( "GStreamer error: unable to create a new filesink" );
        m_SettingRecordFileName = false;
        return false;
    }
    g_object_set( G_OBJECT( new_sink ),
        "location", gchararray( (const char *)filename.mb_str() ), NULL );

    guGstElementsChain chain = {
        m_Muxer != NULL ? m_Muxer : m_Encoder,
        new_sink,
        m_FileSink
        };
    guGstPipelineActuator gpa( &chain );
    guGstResultHandler rh(
        guGstResultHandler::Func( guFaderPlaybin__ElementCleanup ),
        m_FileSink
    );
    gpa.SetHandler( &rh );
    bool res = false;
    // ::Enable will replace m_FileSink since new_sink doesn't have src pad
    if( gpa.Enable( new_sink ) )
    {
        guLogDebug( "guFaderPlaybin::SetRecordFileName new_sink plugged" );
        m_FileSink = new_sink;
        res = true;
    }
    else
    {
        guLogError( "Failed to set recording filename to <%s>", filename );
    }
    m_SettingRecordFileName = false;
    return res;
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::RefreshPlaybackItems( void )
{
    // just a stub for now
    guLogDebug( "guFaderPlaybin::RefreshPlaybackItems" );
}


// -------------------------------------------------------------------------------- //
void guFaderPlaybin::ToggleEqualizer( void )
{
    guLogDebug( "guFaderPlaybin::ToggleEqualizer" );
 
    guGstPipelineActuator gpa( &m_PlayChain );
    guGstResultHandler rh(
        guGstResultHandler::Func( guFaderPlaybin__RefreshPlaybackItems ),
        GetWeakPtr()
        );
    gpa.SetHandler( &rh );
    gpa.Toggle( m_Equalizer );
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::ToggleVolCtl( void )
{
    guLogDebug( "guFaderPlaybin::ToggleVolCtl" );

    guGstPipelineActuator gpa( &m_PlayChain );
    guGstResultHandler rh(
        guGstResultHandler::Func( guFaderPlaybin__RefreshPlaybackItems ),
        GetWeakPtr()
        );
    gpa.SetHandler( &rh );
    gpa.Toggle( m_FaderVolume );
    // renew weak_ptr for every next call
    gpa.Toggle( m_Volume, GetWeakPtr() );
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::SetRGProperties( void )
{
    guLogDebug( "guFaderPlaybin::SetRGProperties" );
    g_object_set( m_ReplayGain,
        "pre-amp", gdouble( m_Player->m_ReplayGainPreAmp ),
        "album-mode", gboolean( m_Player->m_ReplayGainMode == 2 ),
        NULL );
    //g_object_set( G_OBJECT( m_ReplayGain ), "fallback-gain", gdouble( -6 ), NULL );
}

// -------------------------------------------------------------------------------- //
void guFaderPlaybin::ReconfigureRG( void )
{
    guLogDebug( "guFaderPlaybin::ReconfigureRG" );

    SetRGProperties();
    guGstPipelineActuator gpa( &m_PlayChain );
    guGstResultHandler rh(
        guGstResultHandler::Func( guFaderPlaybin__RefreshPlaybackItems ),
        GetWeakPtr()
        );
    gpa.SetHandler( &rh );
    if( m_Player->m_ReplayGainMode )
    {
        gpa.Enable( m_ReplayGainLimiter );
        gpa.Enable( m_ReplayGain, GetWeakPtr() );

    }
    else
    {
        gpa.Disable( m_ReplayGain );
        gpa.Disable( m_ReplayGainLimiter, GetWeakPtr() );
    }
}

void guFaderPlaybin::SetValveDrop( bool drop )
{
    guLogDebug( "guFaderPlaybin::SetValveDrop << %i", drop );
    if( m_Valve != NULL )
    {
        g_object_set( m_Valve, "drop", gboolean( drop ), NULL );
    }

}

}

// -------------------------------------------------------------------------------- //
