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

#include "Utils.h"
#include "PlayerPanel.h"

DEFINE_EVENT_TYPE( wxEVT_MEDIA_LOADED )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_STATECHANGED )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_ABOUT_TO_FINISH )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_FINISHED )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_TAG )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_BUFFERING )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_BITRATE )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_LEVEL )
DEFINE_EVENT_TYPE( wxEVT_MEDIA_ERROR )

// -------------------------------------------------------------------------------- //
extern "C" {

static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guMediaCtrl * ctrl )
{
    switch( GST_MESSAGE_TYPE( message ) )
    {
        case GST_MESSAGE_ERROR :
        {
            GError * err;
            //gchar * debug;
            gst_message_parse_error( message, &err, NULL );

            if( ctrl->GetLastError() != err->code )
            {
                ctrl->SetLastError( err->code );

                wxString * ErrorStr = new wxString( err->message, wxConvUTF8 );

                guLogError( wxT( "Gstreamer error '%s'" ), ErrorStr->c_str() );

                wxMediaEvent event( wxEVT_MEDIA_ERROR );
                event.SetClientData( ( void * ) ErrorStr );
                ctrl->AddPendingEvent( event );
            }


            g_error_free( err );
            //g_free( debug );
            break;
        }

        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState oldstate, newstate, pendingstate;
            gst_message_parse_state_changed( message, &oldstate, &newstate, &pendingstate );

            //guLogMessage( wxT( "State changed %u -> %u (%u)" ), oldstate, newstate, pendingstate );
            if( pendingstate == GST_STATE_VOID_PENDING )
            {
                wxMediaEvent event( wxEVT_MEDIA_STATECHANGED );
                ctrl->AddPendingEvent( event );
            }
            break;
        }

        case GST_MESSAGE_BUFFERING :
        {
            gint        Percent;
            GstState    cur_state;

            gst_message_parse_buffering( message, &Percent );

            guLogMessage( wxT( "Buffering: %i%%" ), Percent );
            if( Percent >= 100 )
            {
                ctrl->m_Buffering = false;
                if( ctrl->m_WasPlaying )
                {
                    gst_element_set_state( ctrl->m_Playbin, GST_STATE_PLAYING );
                    if( ctrl->m_Recordbin )
                    {
                        //gst_element_set_state( ctrl->m_Playbackbin, GST_STATE_PLAYING );
                        gst_element_set_state( ctrl->m_Recordbin, GST_STATE_PLAYING );
                    }
                    ctrl->m_WasPlaying = false;
                }
            }
            else
            {
                gst_element_get_state( ctrl->m_Playbin, &cur_state, NULL, 0 );
                if( cur_state == GST_STATE_PLAYING )
                {
                    ctrl->m_WasPlaying = true;
                    if( ctrl->m_Recordbin )
                    {
                        //gst_element_set_state( ctrl->m_Playbackbin, GST_STATE_PAUSED );
                        gst_element_set_state( ctrl->m_Recordbin, GST_STATE_PAUSED );
                    }
                    gst_element_set_state( ctrl->m_Playbin, GST_STATE_PAUSED );
                }
                ctrl->m_Buffering = true;
            }
            wxMediaEvent event( wxEVT_MEDIA_BUFFERING );
            event.SetInt( Percent );
            ctrl->AddPendingEvent( event );
            //printf( "Buffering %d%%\n", Percent );
            break;
        }

        case GST_MESSAGE_EOS :
        {
            wxMediaEvent event( wxEVT_MEDIA_FINISHED );
            ctrl->AddPendingEvent( event );
          break;
        }

        case GST_MESSAGE_TAG :
        {
            /* The stream discovered new tags. */
            GstTagList * tags;
            //gchar * title = NULL;
            unsigned int bitrate = 0;
            /* Extract from the message the GstTagList.
            * This generates a copy, so we must remember to free it.*/
            gst_message_parse_tag( message, &tags );

            guRadioTagInfo * RadioTagInfo = new guRadioTagInfo();

            gst_tag_list_get_string( tags, GST_TAG_ORGANIZATION, &RadioTagInfo->m_Organization );
            gst_tag_list_get_string( tags, GST_TAG_LOCATION, &RadioTagInfo->m_Location );
            gst_tag_list_get_string( tags, GST_TAG_TITLE, &RadioTagInfo->m_Title );

            //guLogMessage( wxT( "New Tag Found:\n'%s'\n'%s'\n'%s'" ),
            //    wxString( RadioTagInfo->m_Organization, wxConvUTF8 ).c_str(),
            //    wxString( RadioTagInfo->m_Location, wxConvUTF8 ).c_str(),
            //    wxString( RadioTagInfo->m_Title, wxConvUTF8 ).c_str() );

            if( RadioTagInfo->m_Organization || RadioTagInfo->m_Location || RadioTagInfo->m_Title )
            {
                //guLogMessage( wxT( "Tit: %s" ), wxString( title, wxConvUTF8 ).c_str() );
                wxMediaEvent event( wxEVT_MEDIA_TAG );
                event.SetClientData( RadioTagInfo );
                ctrl->AddPendingEvent( event );
            }
            else
            {
                delete RadioTagInfo;
            }

            gst_tag_list_get_uint( tags, GST_TAG_BITRATE, &bitrate );
            if( bitrate )
            {
                wxMediaEvent event( wxEVT_MEDIA_BITRATE );
                event.SetInt( bitrate );
                ctrl->AddPendingEvent( event );
            }

            /* Free the tag list */
            gst_tag_list_free( tags );
            break;
        }

        case GST_MESSAGE_ELEMENT :
        {
            const GstStructure * s = gst_message_get_structure( message );
            const gchar * name = gst_structure_get_name( s );

            //guLogMessage( wxT( "MESSAGE_ELEMENT %s" ), wxString( element ).c_str() );
            if( !strcmp( name, "level" ) )
            {
                wxMediaEvent event( wxEVT_MEDIA_LEVEL );
                gint channels;
                const GValue * list;
                const GValue * value;
                if( !gst_structure_get_clock_time( s, "endtime", &event.m_LevelInfo.m_EndTime ) )
                    guLogWarning( wxT( "Could not parse endtime" ) );

                //guLogMessage( wxT( "endtime: %" GST_TIME_FORMAT ", channels: %d" ), GST_TIME_ARGS( endtime ), channels );

                // we can get the number of channels as the length of any of the value lists
                list = gst_structure_get_value( s, "rms" );
                channels = event.m_LevelInfo.m_Channels = gst_value_list_get_size( list );
                value = gst_value_list_get_value( list, 0 );
                event.m_LevelInfo.m_RMS_L = g_value_get_double( value );
                if( channels > 1 )
                {
                    value = gst_value_list_get_value( list, 1 );
                    event.m_LevelInfo.m_RMS_R = g_value_get_double( value );
                }

                list = gst_structure_get_value( s, "peak" );
                value = gst_value_list_get_value( list, 0 );
                event.m_LevelInfo.m_Peak_L = g_value_get_double( value );
                if( channels > 1 )
                {
                    value = gst_value_list_get_value( list, 1 );
                    event.m_LevelInfo.m_Peak_R = g_value_get_double( value );
                }

                list = gst_structure_get_value( s, "decay" );
                value = gst_value_list_get_value( list, 0 );
                event.m_LevelInfo.m_Decay_L = g_value_get_double( value );
                if( channels > 1 )
                {
                    value = gst_value_list_get_value( list, 1 );
                    event.m_LevelInfo.m_Decay_R = g_value_get_double( value );
                }


//                guLogMessage( wxT( "    RMS: %f dB, peak: %f dB, decay: %f dB" ),
//                    event.m_LevelInfo.m_RMS_L,
//                    event.m_LevelInfo.m_Peak_L,
//                    event.m_LevelInfo.m_Decay_L );

//                // converting from dB to normal gives us a value between 0.0 and 1.0 */
//                rms = pow( 10, rms_dB / 20 );
//                  guLogMessage( wxT( "    normalized rms value: %f" ), rms );
                //ctrl->AddPendingEvent( event );
                wxPostEvent( ctrl, event );
            }
            break;
        }

        default:
            break;
    }

    return TRUE;
}

static void gst_about_to_finish( GstElement * playbin, guMediaCtrl * ctrl )
{
    ctrl->AboutToFinish();
    wxMediaEvent event( wxEVT_MEDIA_ABOUT_TO_FINISH );
    //ctrl->AddPendingEvent( event );
    wxPostEvent( ctrl, event );
}

static gboolean set_state_and_wait( GstElement * bin, GstState target, guMediaCtrl * ctrl )
{
    GstBus * bus;
    gboolean waiting;
    gboolean result;

    //guLogMessage( wxT( "setting playbin state to %s" ), wxString( gst_element_state_get_name( target ), wxConvUTF8 ).c_str() );

    switch( gst_element_set_state( bin, target ) )
    {
        case GST_STATE_CHANGE_SUCCESS :
            //guLogMessage( wxT( "State change was successful" ) );
            return TRUE;

        case GST_STATE_CHANGE_NO_PREROLL:
            //guLogMessage( wxT( "state change was successful (no preroll)" ) );
            return TRUE;

        case GST_STATE_CHANGE_ASYNC:
            //guLogMessage( wxT( "state is changing asynchronously" ) );
            result = TRUE;
            break;

        case GST_STATE_CHANGE_FAILURE:
            //guLogMessage( wxT( "state change failed" ) );
            result = FALSE;
            break;

        default:
            //guLogMessage( wxT( "unknown state change return.." ) );
            result = TRUE;
            break;
    }

    bus = gst_element_get_bus( bin );

    waiting = TRUE;

    while( waiting )
    {
        GstMessage * message;

        message = gst_bus_timed_pop( bus, GST_SECOND * 5 );
        if( message == NULL )
        {
            //guLogError( wxT( "Timeout waiting for state change" ) );
            break;
        }

        switch( GST_MESSAGE_TYPE( message ) )
        {
            case GST_MESSAGE_ERROR :
            {
                GError *gst_error = NULL;
                gst_message_parse_error( message, &gst_error, NULL );
                guLogError( wxT( "State change error '%s'" ), wxString( gst_error->message, wxConvUTF8 ).c_str() );
                g_error_free( gst_error );

                waiting = FALSE;
                result = FALSE;
                break;
            }

            case GST_MESSAGE_STATE_CHANGED:
            {
                GstState oldstate;
                GstState newstate;
                GstState pending;
                gst_message_parse_state_changed( message, &oldstate, &newstate, &pending );
                if( GST_MESSAGE_SRC( message ) == GST_OBJECT( bin ) )
                {
                    //guLogMessage( wxT( "playbin reached state %s" ), wxString( gst_element_state_get_name( newstate ), wxConvUTF8 ).c_str() );
                    if( pending == GST_STATE_VOID_PENDING && newstate == target )
                    {
                        waiting = FALSE;
                    }
                }
                break;
            }

            default:
                /* pass back to regular message handler */
                gst_bus_async_callback( bus, message, ctrl );
            break;
        }
    }

    if( result == FALSE )
    {
        guLogError( wxT( "Unable to start playback pipeline" ) );
    }

    return result;
}


}

// -------------------------------------------------------------------------------- //
bool IsValidElement( GstElement * outputsink )
{
    if( !GST_IS_ELEMENT( outputsink ) )
    {
        if( G_IS_OBJECT( outputsink ) )
            g_object_unref( outputsink );
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetProperty( GstElement * element, const char * name, gint64 value )
{
    bool Done = false;
    bool RetVal = false;

    if( g_object_class_find_property( G_OBJECT_GET_CLASS( element ), name ) )
    {
        g_object_set( element, name, value, NULL );
        return true;
    }

    if( GST_IS_BIN( element ) )
    {
        // Iterate in sorted order, so we look at sinks first
        GstIterator * It = gst_bin_iterate_sorted( ( GstBin * ) element );

        while( !Done )
        {
            gpointer Data;
            GstElement * Child;
            switch( gst_iterator_next( It, &Data ) )
            {
                case GST_ITERATOR_OK:
                    Child = GST_ELEMENT_CAST( Data );
                    if( SetProperty( Child, name, value ) )
                    {
                        RetVal = true;
                        Done = true;
                    }
                    gst_object_unref( Child );
                    break;
                case GST_ITERATOR_DONE:
                    Done = TRUE;
                    break;
                case GST_ITERATOR_RESYNC:
                    gst_iterator_resync( It );
                    break;
                case GST_ITERATOR_ERROR:
                    Done = true;
                    break;
            }
        }

        gst_iterator_free( It );
    }
    return RetVal;
}


// -------------------------------------------------------------------------------- //
GstElement * guMediaCtrl::BuildOutputBin( void )
{
    GstElement * outputsink;
    outputsink = gst_element_factory_make( "gconfaudiosink", "out_gconfaudiosink" );
    if( !IsValidElement( outputsink ) )
    {
        // fallback to autodetection
        outputsink = gst_element_factory_make( "autoaudiosink", "out_autoaudiosink" );
        if( !IsValidElement( outputsink ) )
        {
            outputsink = gst_element_factory_make( "alsasink", "out_alsasink" );
            if( !IsValidElement( outputsink ) )
            {
                outputsink = gst_element_factory_make( "pulsesink", "out_pulsesink" );
                if( !IsValidElement( outputsink ) )
                {
                    outputsink = gst_element_factory_make( "osssink", "out_osssink" );
                    if( !IsValidElement( outputsink ) )
                    {
                        guLogError( wxT( "Could not find a valid audiosink" ) );
                        return NULL;
                    }
                }
            }
        }
    }
    return outputsink;
}

// -------------------------------------------------------------------------------- //
GstElement * guMediaCtrl::BuildPlaybackBin( GstElement * outputsink )
{
    GstElement * sinkbin = gst_bin_new( "outsinkbin" );
    if( IsValidElement( sinkbin ) )
    {
        GstElement * converter = gst_element_factory_make( "audioconvert", "pb_audioconvert" );
        if( IsValidElement( converter ) )
        {
            GstElement * replay = gst_element_factory_make( "rgvolume", "pb_rgvolume" );
            if( IsValidElement( replay ) )
            {
                g_object_set( G_OBJECT( replay ), "album-mode", false, NULL );
                g_object_set( G_OBJECT( replay ), "pre-amp", gdouble( 6 ), NULL );

                GstElement * level = gst_element_factory_make( "level", "pb_level" );
                if( IsValidElement( level ) )
                {
                    g_object_set( level, "message", TRUE, NULL );
                    g_object_set( level, "interval", gint64( 200000000 ), NULL );

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
                                            g_object_set( queue, "max-size-time", guint64( 250000000 ), NULL );

                                            gst_bin_add_many( GST_BIN( sinkbin ), m_Tee, queue, converter, replay, level, m_Equalizer, limiter, m_Volume, outconverter, outputsink, NULL );
                                            gst_element_link_many( m_Tee, queue, converter, replay, level, m_Equalizer, limiter, m_Volume, outconverter, outputsink, NULL );

                                            GstPad * pad = gst_element_get_pad( m_Tee, "sink" );
                                            if( GST_IS_PAD( pad ) )
                                            {
                                                GstPad * ghostpad = gst_ghost_pad_new( "sink", pad );
                                                gst_element_add_pad( sinkbin, ghostpad );
                                                gst_object_unref( pad );

                                                return sinkbin;
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

                g_object_unref( replay );
            }
            else
            {
                guLogError( wxT( "Could not create the replay gain object" ) );
            }

            g_object_unref( converter );
        }
        else
        {
            guLogError( wxT( "Could not create the audioconvert object" ) );
        }

        g_object_unref( sinkbin );
    }
    else
    {
        guLogError( wxT( "Could not create the outsinkbin object" ) );
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
GstElement * guMediaCtrl::BuildRecordBin( const wxString &path, GstElement * encoder, GstElement * muxer )
{
    GstElement * recordbin = gst_bin_new( "recordbin" );
    if( IsValidElement( recordbin ) )
    {
        GstElement * converter = gst_element_factory_make( "audioconvert", "rb_audioconvert" );
        if( IsValidElement( converter ) )
        {
            m_FileSink = gst_element_factory_make( "filesink", "rb_filesink" );
            if( IsValidElement( m_FileSink ) )
            {
                g_object_set( m_FileSink, "location", ( const char * ) path.mb_str(), NULL );

                GstElement * queue = gst_element_factory_make( "queue", "rb_queue" );
                if( IsValidElement( queue ) )
                {
                    g_object_set( queue, "max-size-time", 10 * GST_SECOND, "max-size-buffers", 0, "max-size-bytes", 0, NULL );
                    //g_object_set( queue, "max-size-time", guint64( 250000000 ), NULL );

                    if( muxer )
                    {
                        gst_bin_add_many( GST_BIN( recordbin ), queue, converter, encoder, muxer, m_FileSink, NULL );
                        gst_element_link_many( queue, converter, encoder, muxer, m_FileSink, NULL );
                    }
                    else
                    {
                        gst_bin_add_many( GST_BIN( recordbin ), queue, converter, encoder, m_FileSink, NULL );
                        gst_element_link_many( queue, converter, encoder, m_FileSink, NULL );
                    }

                    gst_bin_add( GST_BIN( m_Playbackbin ), recordbin );

                    GstPad * pad = gst_element_get_pad( queue, "sink" );
                    if( GST_IS_PAD( pad ) )
                    {
                        GstPad * ghostpad = gst_ghost_pad_new( "sink", pad );
                        gst_element_add_pad( recordbin, ghostpad );
                        gst_object_unref( pad );

                        gst_element_link( m_Tee, recordbin );

                        //g_object_set( recordbin, "async-handling", true, NULL );

                        return recordbin;
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

        g_object_unref( recordbin );
    }
    else
    {
        guLogError( wxT( "Could not create the recordbin object" ) );
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guMediaCtrl::guMediaCtrl( guPlayerPanel * playerpanel )
{
    m_PlayerPanel = playerpanel;
    m_Playbin = NULL;
    m_Recordbin = NULL;
    m_Playbackbin = NULL;
    m_FileSink = NULL;
    m_Tee = NULL;
    m_Buffering = false;
    m_WasPlaying = false;
    m_llPausedPos = 0;
    m_LastError = 0;

    if( Init() )
    {
        // Get the audio output sink
        GstElement * outputsink = BuildOutputBin();

        //
        m_Playbin = gst_element_factory_make( "playbin2", "play" );
        if( !IsValidElement( m_Playbin ) )
        {
            m_Playbin = NULL;
            guLogError( wxT( "Could not create the gstreamer playbin." ) );
        }

        m_Playbackbin = BuildPlaybackBin( outputsink );

        g_object_set( G_OBJECT( m_Playbin ), "audio-sink", m_Playbackbin, NULL );

            // This dont make any difference in gapless playback :(
//        if( !SetProperty( outputsink, "buffer-time", (gint64) 5000*1000 ) )
//            guLogMessage( wxT( "Could not set buffer time to gstreamer object." ) );

        // Be sure we only play audio
        g_object_set( G_OBJECT( m_Playbin ), "flags", 0x02 | 0x10, NULL );
        //g_object_set( G_OBJECT( m_Playbin ), "buffer-size", 256*1024, NULL );

        g_signal_connect( G_OBJECT( m_Playbin ), "about-to-finish",
            G_CALLBACK( gst_about_to_finish ), ( void * ) this );
        //
        gst_bus_add_watch( gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) ),
            ( GstBusFunc ) gst_bus_async_callback, this );
    }
}

// -------------------------------------------------------------------------------- //
guMediaCtrl::~guMediaCtrl()
{
    if( m_Playbin )
    {
        wxASSERT( GST_IS_OBJECT( m_Playbin ) );
        gst_element_set_state( m_Playbin, GST_STATE_NULL );
        gst_object_unref( GST_OBJECT( m_Playbin ) );
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::EnableRecord( const wxString &path, const int format, const int quality )
{
    GstElement * Encoder = NULL;
    GstElement * Muxer = NULL;
    gint Mp3Quality[] = { 320, 192, 128, 96, 64 };
    float OggQuality[] = { 0.9, 0.7, 0.5, 0.3, 0.1 };
    gint FlacQuality[] = { 9, 7, 5, 3, 1 };

    SetRecordPath( path );

    //
    switch( format )
    {
        case guRECORD_FORMAT_MP3  :
        {
            m_RecordExt = wxT( ".mp3" );
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
            m_RecordExt = wxT( ".ogg" );
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
            m_RecordExt = wxT( ".flac" );
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
        wxMediaState State = GetState();
        if( State == wxMEDIASTATE_PLAYING )
        {
            if( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
            {
                guLogMessage( wxT( "Could not set state inserting record object" ) );
                return false;
            }
        }

        wxString RecordFile = m_RecordPath + wxT( "record" ) + m_RecordExt;
//        if( wxFileExists( RecordFile ) )
//        {
//            int Index = 1;
//            do {
//                RecordFile = m_RecordPath + wxString::Format( wxT( "record%i" ), Index++ ) + m_RecordExt;
//            } while( wxFileExists( RecordFile ) );
//        }

        m_Recordbin = BuildRecordBin( RecordFile, Encoder, Muxer );

        if( State == wxMEDIASTATE_PLAYING )
        {
            if( gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
            {
                guLogMessage( wxT( "Could not restore state inserting record object" ) );
                return false;
            }
        }
        return true;
    }
    else
    {
        guLogError( wxT( "Could not create the recorder encoder object" ) );
    }

    return false;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::DisableRecord( void )
{
    GstState    CurState;

    gst_element_get_state( m_Playbin, &CurState, NULL, 0 );

    if( CurState == GST_STATE_PLAYING )
    {
        //guLogMessage( wxT( "Trying to set state to pased" ) );
        if( gst_element_set_state( m_Recordbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogMessage( wxT( "Could not set record state removing record object" ) );
        }

        //if( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        if( !set_state_and_wait( m_Playbin, GST_STATE_PAUSED, this ) )
        {
            guLogMessage( wxT( "Could not set playbin state removing record object" ) );
        }
    }

    gst_element_set_state( m_Recordbin, GST_STATE_NULL );

    gst_bin_remove( GST_BIN( m_Playbackbin ), m_Recordbin );
    gst_object_unref( m_Recordbin );
    m_Recordbin = NULL;
    m_FileSink = NULL;

    if( CurState == GST_STATE_PLAYING )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogMessage( wxT( "Could not restore state inserting record object" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetRecordFileName( const wxString &path, const wxString &track )
{
    if( !m_Recordbin )
        return false;

    GstState    PlayState;
    GstState    RecState;

    {
        GstState State;
        gst_element_get_state( m_Recordbin, &State, NULL, 0 );
        guLogMessage( wxT( "0) SetRecordName: recordbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );

        gst_element_get_state( m_Playbin, &State, NULL, 0 );
        guLogMessage( wxT( "1) SetRecordName: playbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );
    }

    gst_element_get_state( m_Recordbin, &RecState, NULL, 0 );
    if( RecState != GST_STATE_NULL )
    {
        if( gst_element_set_state( m_Recordbin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
        //if( !set_state_and_wait( this, m_Recordbin, GST_STATE_READY, &gsterror ) )
        {
            guLogMessage( wxT( "Could not set state inserting record object" ) );
            return false;
        }

        //gst_element_set_state( m_Recordbin, GST_STATE_NULL ); // == GST_STATE_CHANGE_FAILURE )
        if( !set_state_and_wait( m_Recordbin, GST_STATE_NULL, this ) )
        {
            guLogError( wxT( "Could not reset the record object chaning the filename." ) );
        }
    }

    {
        GstState State;
        gst_element_get_state( m_Recordbin, &State, NULL, 0 );
        guLogMessage( wxT( "2) SetRecordName: recordbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );

        gst_element_get_state( m_Playbin, &State, NULL, 0 );
        guLogMessage( wxT( "3) SetRecordName: playbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );
    }

    wxString FileName = m_RecordPath + path + wxT( "/" ) + track + m_RecordExt;
    wxFileName::Mkdir( wxPathOnly( FileName ), 0770, wxPATH_MKDIR_FULL );
    guLogMessage( wxT( "The new Record File is '%s'" ), FileName.c_str() );

    g_object_set( m_FileSink, "location", ( const char * ) FileName.mb_str(), NULL );

    {
        GstState State;
        gst_element_get_state( m_Recordbin, &State, NULL, 0 );
        guLogMessage( wxT( "4) SetRecordName: recordbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );

        gst_element_get_state( m_Playbin, &State, NULL, 0 );
        guLogMessage( wxT( "5) SetRecordName: playbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );
    }

//    gst_element_get_state( m_Playbin, &PlayState, NULL, 0 );

    //if( !set_state_and_wait( m_Recordbin, PlayState, this ) )
    //if( gst_element_set_state( m_Recordbin, PlayState ) == GST_STATE_CHANGE_FAILURE )
    if( gst_element_set_state( m_Recordbin, RecState ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Could not restore state inserting record object" ) );
        return false;
    }

    {
        GstState State;
        gst_element_get_state( m_Recordbin, &State, NULL, 0 );
        guLogMessage( wxT( "6) SetRecordName: recordbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );

        gst_element_get_state( m_Playbin, &State, NULL, 0 );
        guLogMessage( wxT( "7) SetRecordName: playbin state: %s" ), wxString( gst_element_state_get_name( State ), wxConvUTF8 ).c_str() );
    }

    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::SetRecordPath( const wxString &path )
{
    m_RecordPath = path;
    // Be sure the path exists
    if( !m_RecordPath.EndsWith( wxT( "/" ) ) )
        m_RecordPath.Append( wxT( "/" ) );
    wxFileName::Mkdir( m_RecordPath, 0770, wxPATH_MKDIR_FULL );
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Load( const wxString &uri, bool restart )
{
    //guLogMessage( wxT( "uri set to %u %s" ), restart, uri.c_str() );

    // Reset positions & rate
    m_llPausedPos = 0;

    SetLastError( 0 );

    if( restart )
    {
        if( m_Recordbin )
        {
            gst_element_set_state( m_Recordbin, GST_STATE_READY );
        }
        // Set playbin to ready to stop the current media...
        if( gst_element_set_state( m_Playbin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
        {
            return false;
        }

        // free current media resources
        if( m_Recordbin )
        {
            gst_element_set_state( m_Recordbin, GST_STATE_NULL );
        }
        gst_element_set_state( m_Playbin, GST_STATE_NULL );
    }

    // Make sure the passed URI is valid and tell playbin to load it
    // non-file uris are encoded
    //wxASSERT( gst_uri_protocol_is_valid( "file" ) );
    //wxASSERT( gst_uri_is_valid( ( const char * ) uri.mb_str() ) );
    if( !gst_uri_is_valid( ( const char * ) uri.mb_str() ) )
        return false;

    g_object_set( G_OBJECT( m_Playbin ), "uri", ( const char * ) uri.mb_str(), NULL );

    if( restart )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        {
            return false; // no real error message needed here as this is
        }
        if( m_Recordbin )
        {
            gst_element_set_state( m_Recordbin, GST_STATE_PAUSED );
        }
    }

    wxMediaEvent event( wxEVT_MEDIA_LOADED );
    event.SetInt( restart );
    AddPendingEvent( event );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Play()
{
    return ( !m_Recordbin || ( gst_element_set_state( m_Recordbin, GST_STATE_PLAYING ) != GST_STATE_CHANGE_FAILURE ) ) &&
    ( gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) != GST_STATE_CHANGE_FAILURE );
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Pause()
{
    return ( !m_Recordbin || ( gst_element_set_state( m_Recordbin, GST_STATE_PAUSED ) != GST_STATE_CHANGE_FAILURE ) ) &&
           ( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) != GST_STATE_CHANGE_FAILURE );
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Stop()
{
    if( Pause() && Seek( 0 ) )
    {
        m_llPausedPos = 0;
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Seek( wxLongLong where )
{
    gst_element_seek( m_Playbin, 1, GST_FORMAT_TIME,
          ( GstSeekFlags ) ( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
          GST_SEEK_TYPE_SET, where.GetValue() * GST_MSECOND,
          GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE );

    m_llPausedPos = where;
    return true;
}

// -------------------------------------------------------------------------------- //
wxFileOffset guMediaCtrl::Tell()
{
    gint64 pos;
    GstFormat format = GST_FORMAT_TIME;

    if( gst_element_query_position( m_Playbin, &format, &pos ) && pos != -1 )
    {
        return pos / GST_MSECOND;
    }
    else
    {
        return m_llPausedPos.ToLong();
    }
}

// -------------------------------------------------------------------------------- //
wxFileOffset guMediaCtrl::GetLength()
{
    gint64 len;
    GstFormat format = GST_FORMAT_TIME;

    if( gst_element_query_duration( m_Playbin, &format, &len ) && len != -1 )
    {
        //guLogMessage( wxT( "TrackLength: %lld / %lld = %lld" ), len, GST_SECOND, len / GST_SECOND );
        return ( len / GST_SECOND );
    }
    else
    {
        return 0;
    }
}

// -------------------------------------------------------------------------------- //
wxMediaState guMediaCtrl::GetState()
{
    //guLogMessage( wxT( "MediaCtrl->GetState" ) );
    switch( GST_STATE( m_Playbin ) )
    {
        case GST_STATE_PLAYING:
            return wxMEDIASTATE_PLAYING;

        case GST_STATE_PAUSED:
            if( m_llPausedPos == 0 )
                return wxMEDIASTATE_STOPPED;
            else
                return wxMEDIASTATE_PAUSED;
        default://case GST_STATE_READY:
            return wxMEDIASTATE_STOPPED;
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetVolume( double dVolume )
{
    g_object_set( G_OBJECT( m_Volume ), "volume", dVolume, NULL );
    return true;
}

// -------------------------------------------------------------------------------- //
double guMediaCtrl::GetVolume()
{
    double dVolume = 1.0;
    g_object_get( G_OBJECT( m_Volume ), "volume", &dVolume, NULL );
    return dVolume;
}

// -------------------------------------------------------------------------------- //
int guMediaCtrl::GetEqualizerBand( const int band )
{
    gdouble value;
    g_object_get( G_OBJECT( m_Equalizer ), wxString::Format( wxT( "band%u" ),
                   band ).char_str(), &value, NULL );
    return value;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::SetEqualizerBand( const int band, const int value )
{
    g_object_set( G_OBJECT( m_Equalizer ), wxString::Format( wxT( "band%u" ),
                   band ).char_str(), gdouble( value ), NULL );
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetEqualizer( const wxArrayInt &eqset )
{
    if( m_Equalizer && ( eqset.Count() == guEQUALIZER_BAND_COUNT ) )
    {
        int index;
        int count = eqset.Count();
        for( index = 0; index < count; index++ )
        {
            SetEqualizerBand( index, eqset[ index ] );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ResetEqualizer( void )
{
    int index;
    for( index = 0; index < guEQUALIZER_BAND_COUNT; index++ )
    {
        SetEqualizerBand( index, 0 );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::AboutToFinish( void )
{
     m_PlayerPanel->OnAboutToFinish();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ClearError( void )
{
    if( gst_element_set_state( m_Playbin, GST_STATE_NULL ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Error restoring the gstreamer status." ) );
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
