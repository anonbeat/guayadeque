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

// -------------------------------------------------------------------------------- //
extern "C" {

static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guMediaCtrl * ctrl )
{
    switch( GST_MESSAGE_TYPE( message ) )
    {
        case GST_MESSAGE_ERROR :
        {
            GError * err;
            gchar * debug;
            gst_message_parse_error( message, &err, &debug );
            guLogError( wxT( "Gstreamer error '%s'" ), wxString( err->message, wxConvUTF8 ).c_str() );
            g_error_free( err );
            g_free( debug );
            break;
        }

        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState oldstate, newstate, pendingstate;
            gst_message_parse_state_changed( message, &oldstate, &newstate, &pendingstate );

            //guLogMessage( wxT( "State changed %u -> %u (%u)" ), oldstate, newstate, pendingstate );

            wxMediaEvent event( wxEVT_MEDIA_STATECHANGED );
            ctrl->AddPendingEvent( event );
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


        default:
            break;
    }

    return TRUE;
}

static void gst_about_to_finish( GstElement * playbin, guMediaCtrl * ctrl )
{
    ctrl->AboutToFinish();
    wxMediaEvent event( wxEVT_MEDIA_ABOUT_TO_FINISH );
    ctrl->AddPendingEvent( event );
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
guMediaCtrl::guMediaCtrl( guPlayerPanel * playerpanel )
{
    m_PlayerPanel = playerpanel;
    m_Playbin = NULL;
    m_Buffering = false;
    m_WasPlaying = false;
    m_llPausedPos = 0;

    if( Init() )
    {
        // Get the audio output sink
        GstElement * outputsink = gst_element_factory_make( "gconfaudiosink", "audio-sink" );
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
            g_object_set( replay, "album-mode", false, NULL );
        }

        gst_bin_add( GST_BIN( m_Playbin ), replay );
        g_object_set( G_OBJECT( m_Playbin ), "audio-sink", outputsink, NULL );

//        GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) );
//        gst_bus_add_signal_watch( bus );

        g_signal_connect( G_OBJECT( m_Playbin ), "about-to-finish",
				 G_CALLBACK( gst_about_to_finish ), ( void * ) this );
        //
        gst_bus_add_watch( gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) ), ( GstBusFunc ) gst_bus_async_callback, this );

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
    wxASSERT( gst_uri_is_valid( ( const char * ) uri.mb_str() ) );

    g_object_set( G_OBJECT( m_Playbin ), "uri", ( const char * ) uri.mb_str(), NULL );

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

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Play()
{
    //guLogMessage( wxT( "MediaCtrl->Play" ) );
    return gst_element_set_state( m_Playbin, GST_STATE_PLAYING ) != GST_STATE_CHANGE_FAILURE;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Pause()
{
    //guLogMessage( wxT( "MediaCtrl->Pause" ) );
    m_llPausedPos = Tell();
    return gst_element_set_state( m_Playbin, GST_STATE_PAUSED ) != GST_STATE_CHANGE_FAILURE;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Stop()
{
    //guLogMessage( wxT( "MediaCtrl->Stop" ) );
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
    //guLogMessage( wxT( "MediaCtrl->Seek" ) );
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
    //guLogMessage( wxT( "MediaCtrl->Tell" ) );
    if( GetState() != wxMEDIASTATE_PLAYING )
        return m_llPausedPos.ToLong();
    else
    {
        gint64 pos;
        GstFormat fmtTime = GST_FORMAT_TIME;

        if( !gst_element_query_position( m_Playbin, &fmtTime, &pos ) ||
            fmtTime != GST_FORMAT_TIME || pos == -1 )
            return 0;
        return pos / GST_MSECOND ;
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
    g_object_set( G_OBJECT( m_Playbin ), "volume", dVolume, NULL );
    return true;
}

// -------------------------------------------------------------------------------- //
double guMediaCtrl::GetVolume()
{
    double dVolume = 1.0;
    g_object_get( G_OBJECT( m_Playbin ), "volume", &dVolume, NULL );
    return dVolume;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::AboutToFinish( void )
{
    m_PlayerPanel->OnAboutToFinish();
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
