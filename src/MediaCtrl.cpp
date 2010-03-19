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
            if( !ctrl->GetLastError() )
            {
                ctrl->SetLastError( 1 );

                GError * err;
                gchar * debug;
                gst_message_parse_error( message, &err, &debug );

                //guLogError( wxT( "Gstreamer error '%s'\'%s'" ),
                //    wxString( err->message, wxConvUTF8 ).c_str(),
                //    wxString( debug, wxConvUTF8 ).c_str() );
                wxString * ErrorStr = new wxString( err->message, wxConvUTF8 );
                wxMediaEvent event( wxEVT_MEDIA_ERROR );
                event.SetClientData( ( void * ) ErrorStr );
                ctrl->AddPendingEvent( event );

                g_error_free( err );
                g_free( debug );

            }
            //ctrl->Stop();
            //gst_element_set_state( ctrl->m_Playbin, GST_STATE_NULL );
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

            if( Percent >= 100 )
            {
                ctrl->m_Buffering = false;
                if( ctrl->m_WasPlaying )
                {
                    gst_element_set_state( ctrl->m_Playbin, GST_STATE_PLAYING );
                    ctrl->m_WasPlaying = false;
                }
            }
            else
            {
                gst_element_get_state( ctrl->m_Playbin, &cur_state, NULL, 0 );
                if( cur_state == GST_STATE_PLAYING )
                {
                    ctrl->m_WasPlaying = true;
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
            gchar * title = NULL;
            unsigned int bitrate = 0;
            /* Extract from the message the GstTagList.
            * This generates a copy, so we must remember to free it.*/
            gst_message_parse_tag( message, &tags );

            /* Extract the title and artist tags - if they exist */
            gst_tag_list_get_string( tags, GST_TAG_TITLE, &title );

            if( title )
            {
                wxMediaEvent event( wxEVT_MEDIA_TAG );
                event.SetClientData( new wxString( title, wxConvUTF8 ) );
                ctrl->AddPendingEvent( event );
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

}

// -------------------------------------------------------------------------------- //
bool IsValidOutput( GstElement * outputsink )
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
guMediaCtrl::guMediaCtrl( guPlayerPanel * playerpanel )
{
    m_PlayerPanel = playerpanel;
    m_Playbin = NULL;
    m_Buffering = false;
    m_WasPlaying = false;
    m_llPausedPos = 0;
    m_LastError = 0;

    if( Init() )
    {
        // Get the audio output sink
        GstElement * outputsink;
        outputsink = gst_element_factory_make( "gconfaudiosink", "audio-sink" );
        if( !IsValidOutput( outputsink ) )
        {
            // fallback to autodetection, then alsa, then oss as a stopgap
            outputsink = gst_element_factory_make( "autoaudiosink", "audio-sink" );
            if( !IsValidOutput( outputsink ) )
            {
                outputsink = gst_element_factory_make( "alsasink", "alsa-output" );
                if( !IsValidOutput( outputsink ) )
                {
                    outputsink = gst_element_factory_make( "osssink", "play_audio" );
                    if( !IsValidOutput( outputsink ) )
                    {
                        guLogError( wxT( "Could not find a valid audiosink" ) );
                        return;
                    }
                }
            }
        }

        //
        m_Playbin = gst_element_factory_make( "playbin2", "play" );
        if( !GST_IS_ELEMENT( m_Playbin ) )
        {
            if( G_IS_OBJECT( m_Playbin ) )
                g_object_unref( m_Playbin );
            m_Playbin = NULL;
            guLogError( wxT( "Could not create the gstreamer playbin." ) );
            return;
        }

//        GstCaps *caps;
//        caps = gst_caps_from_string( "audio/x-raw-int,channels=2" );
//
//        gst_bin_add( GST_BIN( m_Playbin ), level );
//        gst_bin_add( GST_BIN( m_Playbin ), replay );
//        gst_element_link_filtered( replay, level, caps );

        GstElement * sinkbin = gst_bin_new( "outsinkbin" );
        if( !GST_IS_ELEMENT( sinkbin ) )
        {
            if( G_IS_OBJECT( sinkbin ) )
                g_object_unref( sinkbin );
            sinkbin = NULL;
            guLogError( wxT( "Could not create the outsinkbin object" ) );
            return;
        }

        GstElement * converter = gst_element_factory_make( "audioconvert", "aconvert" );
        if( !GST_IS_ELEMENT( converter ) )
        {
            if( G_IS_OBJECT( converter ) )
                g_object_unref( converter );
            converter = NULL;
            guLogError( wxT( "Could not create the audioconvert object" ) );
            return;
        }

        GstElement * replay = gst_element_factory_make( "rgvolume", "replaygain" );
        if( !GST_IS_ELEMENT( replay ) )
        {
            if( G_IS_OBJECT( replay ) )
                g_object_unref( replay );
            replay = NULL;
            guLogError( wxT( "Could not create the replay gain object" ) );
            return;
        }
        else
        {
            g_object_set( G_OBJECT( replay ), "album-mode", false, NULL );
            g_object_set( G_OBJECT( replay ), "pre-amp", gdouble( 6 ), NULL );
        }

        GstElement * level = gst_element_factory_make( "level", "gulevelctrl" );
        if( !GST_IS_ELEMENT( level ) )
        {
            if( G_IS_OBJECT( level ) )
                g_object_unref( level );
            level = NULL;
            guLogError( wxT( "Could not create the level object" ) );
            return;
        }
        else
        {
            g_object_set( level, "message", TRUE, NULL );
            g_object_set( level, "interval", gint64( 200000000 ), NULL );
        }

        m_Volume = gst_element_factory_make( "volume", "mastervolume" );
        if( !GST_IS_ELEMENT( m_Volume ) )
        {
            if( G_IS_OBJECT( m_Volume ) )
                g_object_unref( m_Volume );
            m_Volume = NULL;
            guLogError( wxT( "Could not create the volume object" ) );
            return;
        }

        m_Equalizer = gst_element_factory_make( "equalizer-10bands", "equalizer" );
        if( !GST_IS_ELEMENT( m_Equalizer ) )
        {
            if( G_IS_OBJECT( m_Equalizer ) )
                g_object_unref( m_Equalizer );
            m_Equalizer = NULL;
            guLogError( wxT( "Could not create the equalizer object" ) );
            return;
        }

        GstElement * limiter = gst_element_factory_make( "rglimiter", "limiter" );
        if( !GST_IS_ELEMENT( limiter ) )
        {
            if( G_IS_OBJECT( limiter ) )
                g_object_unref( limiter );
            limiter = NULL;
            guLogError( wxT( "Could not create the limiter object" ) );
            return;
        }
//        else
//        {
//            g_object_set( G_OBJECT( limiter ), "enabled", TRUE, NULL );
//        }

        GstElement * outconverter = gst_element_factory_make( "audioconvert", "outconvert" );
        if( !GST_IS_ELEMENT( outconverter ) )
        {
            if( G_IS_OBJECT( outconverter ) )
                g_object_unref( outconverter );
            outconverter = NULL;
            guLogError( wxT( "Could not create the output audioconvert object" ) );
            return;
        }

        GstPad * pad;
        GstPad * ghostpad;

        gst_bin_add_many( GST_BIN( sinkbin ), converter, replay, level, m_Equalizer, limiter, m_Volume, outconverter, outputsink, NULL );
        gst_element_link_many( converter, replay, level, m_Equalizer, limiter, m_Volume, outconverter, outputsink, NULL );

        pad = gst_element_get_pad( converter, "sink" );
        ghostpad = gst_ghost_pad_new( "sink", pad );
        gst_element_add_pad( sinkbin, ghostpad );
        gst_object_unref( pad );

        g_object_set( G_OBJECT( m_Playbin ), "audio-sink", sinkbin, NULL );

            // This dont make any difference in gapless playback :(
//        if( !SetProperty( outputsink, "buffer-time", (gint64) 5000*1000 ) )
//            guLogMessage( wxT( "Could not set buffer time to gstreamer object." ) );

        // Be sure we only play audio
        g_object_set( G_OBJECT( m_Playbin ), "flags", 0x02|0x10, NULL );
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
bool guMediaCtrl::Load( const wxString &uri, bool restart )
{
    // Reset positions & rate
    m_llPausedPos = 0;

    //guLogMessage( wxT( "uri set to %u %s" ), restart, uri.c_str() );

    if( restart )
    {
        // Set playbin to ready to stop the current media...
        if( gst_element_set_state( m_Playbin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
        {
            return false;
        }

        // free current media resources
        gst_element_set_state( m_Playbin, GST_STATE_NULL );
    }

    // Make sure the passed URI is valid and tell playbin to load it
    // non-file uris are encoded
    //wxASSERT( gst_uri_protocol_is_valid( "file" ) );
    //wxASSERT( gst_uri_is_valid( ( const char * ) uri.mb_str() ) );
    if( !gst_uri_is_valid( ( const char * ) uri.mb_str() ) )
        return false;

//    char * uristr = ( char * ) malloc( strlen( uri.mb_str() ) + 1 );
//    strcpy( uristr, uri.mb_str() );

    g_object_set( G_OBJECT( m_Playbin ), "uri", ( const char * ) uri.mb_str(), NULL );
//    g_object_set( G_OBJECT( m_Playbin ), "uri", ( const char * ) uristr, NULL );

    if( restart )
    {
        if( gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        {
            return false; // no real error message needed here as this is
        }
    }

    wxMediaEvent event( wxEVT_MEDIA_LOADED );
    event.SetInt( restart );
    AddPendingEvent( event );

    SetLastError( 0 );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Play()
{
    return gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) != GST_STATE_CHANGE_FAILURE;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Pause()
{
    m_llPausedPos = Tell();
    return gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) != GST_STATE_CHANGE_FAILURE;
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
    while( gst_element_set_state( m_Playbin, GST_STATE_NULL ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Error clearing the error..." ) );
        wxMilliSleep( 10 );
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
