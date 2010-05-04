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

//
// This is baded in Idea and Code by  Jonathan Matthew  <jonathan@d14n.org> From Rhythmbox
//


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

#define guFADERPLAYBIN_FAST_FADER_TIME          (GST_SECOND)

#define GST_AUDIO_TEST_SRC_WAVE_SILENCE         4

#define GST_TO_WXSTRING( str )  ( wxString( str, wxConvUTF8 ) )

#define guLogDebug(...)  guLogMessage(__VA_ARGS__)
#define guSHOW_DUMPFADERPLAYBINS     1

#ifdef guSHOW_DUMPFADERPLAYBINS
// -------------------------------------------------------------------------------- //
static void DumpFaderPlayBins( const guFaderPlayBinArray &playbins )
{
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

        switch( FaderPlayBin->m_State )
        {
            case guFADERPLAYBIN_STATE_WAITING :	 	        StateName = wxT( "waiting" );		    break;
            case guFADERPLAYBIN_STATE_PLAYING :	 	        StateName = wxT( "playing" );		    break;
            case guFADERPLAYBIN_STATE_PAUSED :	 	        StateName = wxT( "paused" );		    break;

            case guFADERPLAYBIN_STATE_REUSING :		        StateName = wxT( "reusing" );		    break;
            case guFADERPLAYBIN_STATE_PREROLLING : 	        StateName = wxT( "prerolling" ); 	    break;
            case guFADERPLAYBIN_STATE_PREROLL_PLAY : 	    StateName = wxT( "preroll->play" );     break;
            case guFADERPLAYBIN_STATE_FADING_IN : 	        StateName = wxT( "fading in" ); 	    break;
            case guFADERPLAYBIN_STATE_SEEKING :		        StateName = wxT( "seeking" );		    break;
            case guFADERPLAYBIN_STATE_SEEKING_PAUSED :	    StateName = wxT( "seeking->paused" );   break;
            case guFADERPLAYBIN_STATE_SEEKING_EOS :	        StateName = wxT( "seeking post EOS" );  break;
            case guFADERPLAYBIN_STATE_WAITING_EOS : 	    StateName = wxT( "waiting for EOS" );   break;
            case guFADERPLAYBIN_STATE_FADING_OUT : 	        StateName = wxT( "fading out" ); 	    break;
            case guFADERPLAYBIN_STATE_FADING_OUT_PAUSED :   StateName = wxT( "fading->paused" );    break;

            case guFADERPLAYBIN_STATE_PENDING_REMOVE:	    StateName = wxT( "pending remove" );    break;
        }

        guLogDebug( wxT( "[%s] '%s'" ), StateName.c_str(), FaderPlayBin->m_Uri.AfterLast( '/' ).c_str() );
    }
    guLogDebug( wxT( " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " ) );
}
#endif

//// -------------------------------------------------------------------------------- //
//static guFaderPlayBin * FindFaderPlayBin( const guFaderPlayBinArray &playbins, const wxString &uri )
//{
//    int Index;
//    int Count = playbins.Count();
//    for( Index = 0; Index < Count; Index++ )
//    {
//        if( playbins[ Index ]->m_Uri == uri )
//            return playbins[ Index ];
//    }
//    return NULL;
//}

// -------------------------------------------------------------------------------- //
static guFaderPlayBin * FindFaderPlayBin( const guFaderPlayBinArray &playbins, GstElement * element )
{
    int Index;
    int Count = playbins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        GstElement * e = element;
        while( e )
        {
            if( e == GST_ELEMENT( playbins[ Index ]->m_PlayBin ) )
            {
                return playbins[ Index ];
            }
			e = GST_ELEMENT_PARENT( e );
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
static guFaderPlayBin * FindFaderPlayBin( const guFaderPlayBinArray &playbins, int statemask )
{
    int Index;
    int Count = playbins.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( playbins[ Index ]->m_State & statemask )
            return playbins[ Index ];
    }
    return NULL;
}


// -------------------------------------------------------------------------------- //
extern "C" {

static void unlink_blocked_cb( GstPad * pad, gboolean blocked, guFaderPlayBin * faderplaybin );
static void unlink_reuse_relink( guMediaCtrl * mediactrl, guFaderPlayBin * faderplaybin );

// -------------------------------------------------------------------------------- //
static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guMediaCtrl * ctrl )
{
	guFaderPlayBin * FaderPlayBin;
	GstObject * MessageSrc;


    if( !ctrl )
        return false;

    ctrl->Lock();
	MessageSrc = GST_MESSAGE_SRC( message );
	if( GST_IS_PAD( MessageSrc ) )
	{
		MessageSrc = GST_OBJECT_PARENT( MessageSrc );
	}
	FaderPlayBin = FindFaderPlayBin( ctrl->m_FaderPlayBins, GST_ELEMENT( MessageSrc ) );
    ctrl->Unlock();

    switch( GST_MESSAGE_TYPE( message ) )
    {
        case GST_MESSAGE_ERROR :
        {
            GError * Error;
            bool     Emit = false;
//            int      Code;

            //gchar * debug;
            gst_message_parse_error( message, &Error, NULL );

            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS( GST_BIN( ctrl->m_Pipeline ), GST_DEBUG_GRAPH_SHOW_ALL , "guayadeque" );

            if( !FaderPlayBin )
            {
                guLogMessage( wxT( "Couldn't find stream for error '%s'" ), GST_TO_WXSTRING( Error->message ).c_str() );
                g_error_free( Error );
                break;
            }

            // If we've already got an error, ignore 'internal data flow error'
            // type messages, as they're too generic to be helpful.
            if( FaderPlayBin->m_EmittedError &&
                Error->domain == GST_STREAM_ERROR &&
                Error->code == GST_STREAM_ERROR_FAILED )
            {
                guLogMessage( wxT( "Ignoring generic error '%s'" ),
                        GST_TO_WXSTRING( Error->message ).c_str() );
                Emit = FALSE;
            }

//            if( ( Error->domain == GST_CORE_ERROR )
//                || ( Error->domain == GST_LIBRARY_ERROR )
//                || ( Error->domain == GST_RESOURCE_ERROR && Error->code == GST_RESOURCE_ERROR_BUSY ) )
//            {
//                Code = RB_PLAYER_ERROR_NO_AUDIO;
//            }
//            else
//            {
//                Code = RB_PLAYER_ERROR_GENERAL;
//            }

            if( Emit )
            {
                //guLogDebug( wxT( "emitting error '%s' for stream %s" ), GST_TO_WXSTRING( Error->message ).c_str(), FaderPlayBin->m_Uri.c_str() );
//                sig_error = g_error_new_literal (RB_PLAYER_ERROR,
//                                 code,
//                                 error->message);
                FaderPlayBin->m_EmittedError = true;

                //_rb_player_emit_error (RB_PLAYER (player), stream->stream_data, sig_error);
                wxString * ErrorStr = new wxString( Error->message, wxConvUTF8 );

                guLogError( wxT( "Gstreamer error '%s'" ), ErrorStr->c_str() );

                guMediaEvent event( guEVT_MEDIA_ERROR );
                event.SetClientData( ( void * ) ErrorStr );
                ctrl->AddPendingEvent( event );
            }

//            if( ctrl->GetLastError() != err->code )
//            {
//                ctrl->SetLastError( err->code );
//
//                wxString * ErrorStr = new wxString( err->message, wxConvUTF8 );
//
//                guLogError( wxT( "Gstreamer error '%s'" ), ErrorStr->c_str() );
//
//                guMediaEvent event( guEVT_MEDIA_ERROR );
//                event.SetClientData( ( void * ) ErrorStr );
//                ctrl->AddPendingEvent( event );
//            }
            g_error_free( Error );

            break;
        }

        case GST_MESSAGE_TAG :
        {
            if( !FaderPlayBin )
            {
                guLogMessage( wxT( "Got tag message for unknown stream" ) );
            }
            else
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
                    ctrl->AddPendingEvent( event );
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
                    ctrl->AddPendingEvent( event );
                }

                // Free the tag list
                gst_tag_list_free( tags );
            }
            break;
        }

//        case GST_MESSAGE_STATE_CHANGED:
//        {
//            GstState oldstate, newstate, pendingstate;
//            gst_message_parse_state_changed( message, &oldstate, &newstate, &pendingstate );
//
//            if( pendingstate == GST_STATE_VOID_PENDING )
//            {
//                ////guLogDebug( wxT( "State changed %u -> %u (%u)" ), oldstate, newstate, pendingstate );
////                if( ctrl->GetLastState() != newstate )
////                {
////                    ctrl->SetLastState( newstate );
////                }
//            }
//            break;
//        }

        case GST_MESSAGE_DURATION :
        {
            if( !FaderPlayBin )
            {
                guLogMessage( wxT( "got duration message for unknown stream" ) );
            }
            else
            {
                gint64 duration;
                GstFormat format;
                gst_message_parse_duration( message, &format, &duration );
                //guLogDebug( wxT( "got duration %" G_GINT64_FORMAT" for stream %s" ), duration, FaderPlayBin->m_Uri.c_str() );
            }
            break;
        }

        case GST_MESSAGE_BUFFERING :
        {
            gint        Percent;
            GstState    cur_state;

            //GstElement * src;

            gst_message_parse_buffering( message, &Percent );

            //match = gst_bin_get_by_name (GST_BIN (sink), name);
            //guLogDebug( wxT( "Buffering: %i%%" ), Percent );

            if( Percent >= 100 )
            {
                ctrl->m_Buffering = false;
                if( ctrl->m_WasPlaying )
                {
//                    gst_element_set_state( ctrl->m_Pipeline, GST_STATE_PLAYING );
//                    gst_element_set_state( ctrl->m_PlaybackBin, GST_STATE_PLAYING );
//                    if( ctrl->m_RecordBin )
//                    {
//                        //gst_element_set_state( ctrl->m_PlaybackBin, GST_STATE_PLAYING );
//                        gst_element_set_state( ctrl->m_RecordBin, GST_STATE_PLAYING );
//                    }
                    ctrl->m_WasPlaying = false;
                }
            }
            else
            {
                gst_element_get_state( ctrl->m_Pipeline, &cur_state, NULL, 0 );
                if( cur_state == GST_STATE_PLAYING )
                {
//                    ctrl->m_WasPlaying = true;
//                    if( ctrl->m_RecordBin )
//                    {
//                        gst_element_set_state( ctrl->m_RecordBin, GST_STATE_PAUSED );
//                    }
//                    gst_element_set_state( ctrl->m_Pipeline, GST_STATE_PAUSED );
                    //gst_element_set_state( ctrl->m_PlaybackBin, GST_STATE_PAUSED );
                }
                ctrl->m_Buffering = true;
            }
            guMediaEvent event( guEVT_MEDIA_BUFFERING );
            event.SetInt( Percent );
            ctrl->AddPendingEvent( event );
            //printf( "Buffering %d%%\n", Percent );
            break;
        }

        case GST_MESSAGE_EOS :
        {
            guMediaEvent event( guEVT_MEDIA_FINISHED );
            ctrl->AddPendingEvent( event );
          break;
        }

        case GST_MESSAGE_ELEMENT :
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
                if( !gst_structure_get_clock_time( s, "endtime", &LevelInfo->m_EndTime ) )
                    guLogWarning( wxT( "Could not parse endtime" ) );

                ////guLogDebug( wxT( "endtime: %" GST_TIME_FORMAT ", channels: %d" ), GST_TIME_ARGS( endtime ), channels );

                // we can get the number of channels as the length of any of the value lists
                list = gst_structure_get_value( s, "rms" );
                channels = LevelInfo->m_Channels = gst_value_list_get_size( list );
                value = gst_value_list_get_value( list, 0 );
                LevelInfo->m_RMS_L = g_value_get_double( value );
                if( channels > 1 )
                {
                    value = gst_value_list_get_value( list, 1 );
                    LevelInfo->m_RMS_R = g_value_get_double( value );
                }

                list = gst_structure_get_value( s, "peak" );
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
                ctrl->AddPendingEvent( event );

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
            //guLogDebug( wxT( "Got Application Message %s" ), GST_TO_WXSTRING( Name ).c_str() );

            if( !FaderPlayBin )
            {
                guLogMessage( wxT( "got application message %s for unknown stream" ), GST_TO_WXSTRING( Name ).c_str() );
            }
            else if( !strcmp( Name, guFADERPLAYBIN_MESSAGE_PLAYING ) )
            {
                //GList *t;

                //guLogDebug( wxT( "got stream playing message for %s" ), FaderPlayBin->m_Uri.c_str() );
                //_rb_player_emit_playing_stream (RB_PLAYER (player), stream->stream_data);
                //guLogDebug( wxT( "** PLAYING ** now stream %s" ), FaderPlayBin->m_Uri.c_str() );
                ctrl->Lock();
                if( ctrl->m_FaderPlayBins.Count() == 1 )
                    FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PLAYING;
                ctrl->Unlock();
                ctrl->SetCurrentState( GST_STATE_PLAYING );

                /* process any buffered tag lists we received while prerolling the stream */
//                for(t = stream->tags; t != NULL; t = t->next) {
//                    GstTagList *tags;
//
//                    tags = (GstTagList *)t->data;
//                    rb_debug ("processing buffered taglist");
//                    gst_tag_list_foreach (tags, (GstTagForeachFunc) process_tag, stream);
//                    gst_tag_list_free (tags);
//                }
//                g_list_free (stream->tags);
                FaderPlayBin->m_Tags.Clear(); // = NULL;
            }
            else if( !strcmp( Name, guFADERPLAYBIN_MESSAGE_FADEIN_START ) )
            {
                ctrl->Lock();
                int Index;
                int Count = ctrl->m_FaderPlayBins.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    guFaderPlayBin * FaderPlayBin = ctrl->m_FaderPlayBins[ Index ];
                    if( FaderPlayBin->m_State == guFADERPLAYBIN_STATE_WAITING )
                    {
                        //guLogDebug( wxT( "got fade-in-start for stream %s -> FADE_IN" ), FaderPlayBin->m_Uri.c_str() );
                        FaderPlayBin->StartFade( FaderPlayBin->m_Player->m_FadeInVolStart, 1.0, FaderPlayBin->m_Player->m_FadeInTime );
                        FaderPlayBin->LinkAndUnblock( NULL );
                    }
                }
                ctrl->Unlock();
            }
            else if( !strcmp( Name, guFADERPLAYBIN_MESSAGE_FADEOUT_DONE ) )
            {
                switch( FaderPlayBin->m_State )
                {
                    case guFADERPLAYBIN_STATE_FADING_OUT :
                    {
                        // stop the stream and dispose of it
                        //guLogDebug( wxT( "got fade-out-done for stream %s -> PENDING_REMOVE" ), FaderPlayBin->m_Uri.c_str() );
                        FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                        FaderPlayBin->m_Player->ScheduleReap();

                        guMediaEvent event( guEVT_MEDIA_FADEOUT_FINISHED );
                        ctrl->AddPendingEvent( event );
                        break;
                    }

                    case guFADERPLAYBIN_STATE_FADING_OUT_PAUSED :
                    {
                        // try to seek back a bit to account for the fade
                        GstFormat Format = GST_FORMAT_TIME;
                        gint64 Pos = -1;
                        gst_element_query_position( FaderPlayBin->m_Volume, &Format, &Pos );
                        if( Pos != -1 )
                        {
                            FaderPlayBin->m_SeekTarget = Pos > guFADERPLAYBIN_FAST_FADER_TIME ? Pos - guFADERPLAYBIN_FAST_FADER_TIME : 0;
                            FaderPlayBin->m_State = guFADERPLAYBIN_STATE_SEEKING_PAUSED;
                            //guLogDebug( wxT( "got fade-out-done for stream %s -> SEEKING_PAUSED [%" G_GINT64_FORMAT "]" ), FaderPlayBin->m_Uri.c_str(), FaderPlayBin->m_SeekTarget );
                        }
                        else
                        {
                            FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PAUSED;
                            //guLogDebug( wxT( "got fade-out-done for stream %s -> PAUSED (position query failed)" ), FaderPlayBin->m_Uri.c_str() );
                        }
                        FaderPlayBin->UnlinkAndBlock();

                        ctrl->SetCurrentState( GST_STATE_PAUSED );
                        break;
                    }

                    default:
                        break;
                }
            }
            else if( !strcmp( Name, guFADERPLAYBIN_MESSAGE_EOS ) )
            {
                // emit EOS (if we aren't already reusing the stream), then unlink it.
                // the stream stay around so we can seek back in it.
                FaderPlayBin->m_NeedsUnlink = true;
                if( FaderPlayBin->m_State != guFADERPLAYBIN_STATE_REUSING )
                {
                    //guLogDebug( wxT( "got EOS message for stream %s -> PENDING_REMOVE" ), FaderPlayBin->m_Uri.c_str() );
                    //_rb_player_emit_eos (RB_PLAYER (player), stream->stream_data, FALSE);
                    guMediaEvent event( guEVT_MEDIA_FINISHED );
                    ctrl->AddPendingEvent( event );
                    FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;

                    unlink_blocked_cb( FaderPlayBin->m_SourcePad, true, FaderPlayBin );

                    // We finished a fading out without receiving the fade in start
                    // prolly because the seek was dragged after the position to do so
                    // We start here the pending playbins
                    guLogDebug( wxT( "Going to check if pending streams after EOS" ) );
                    ctrl->Lock();
#ifdef guSHOW_DUMPFADERPLAYBINS
                    DumpFaderPlayBins( ctrl->m_FaderPlayBins );
#endif
                    int Index;
                    int Count = ctrl->m_FaderPlayBins.Count();
                    for( Index = 0; Index < Count; Index++ )
                    {
                        guFaderPlayBin * FaderPlayBin = ctrl->m_FaderPlayBins[ Index ];
                        if( FaderPlayBin->m_State == guFADERPLAYBIN_STATE_WAITING )
                        {
                            guLogDebug( wxT( "waiting stream on fade-out-done for stream %s -> FADE_IN" ), FaderPlayBin->m_Uri.c_str() );
                            FaderPlayBin->StartFade( FaderPlayBin->m_Player->m_FadeInVolStart, 1.0, FaderPlayBin->m_Player->m_FadeInTime );
                            FaderPlayBin->LinkAndUnblock( NULL );
                        }
                    }
                    ctrl->Unlock();

                    ctrl->ScheduleReap();
                }
                else
                {
                    // no need to emit EOS here, we already know what to do next
                    //guLogDebug( wxT( "got EOS message for stream %s in REUSING state" ), FaderPlayBin->m_Uri.c_str() );

                    unlink_reuse_relink( ctrl, FaderPlayBin );
                }
            }
            else
            {
                //_rb_player_emit_event (RB_PLAYER (player), stream->stream_data, name, NULL);
            }

            break;
        }

        default:
            break;
    }

	gst_bus_async_signal_func( bus, message, NULL );

    return true;
}

//// -------------------------------------------------------------------------------- //
//static void gst_about_to_finish( GstElement * playbin, guMediaCtrl * ctrl )
//{
//    ctrl->AboutToFinish();
//    guMediaEvent event( guEVT_MEDIA_SET_NEXT_MEDIA );
//    ctrl->AddPendingEvent( event );
//}
//

static void unlink_reuse_relink( guMediaCtrl * mediactrl, guFaderPlayBin * faderplaybin )
{
	GError * Error = NULL;

	faderplaybin->Lock();

	if( !faderplaybin->m_AdderPad )
	{
		//guLogDebug( wxT( "stream %s doesn't need to be unlinked.. weird." ), faderplaybin->m_Uri.c_str() );
	}
	else
	{
		//guLogDebug( wxT( "unlinking stream %s for reuse" ), faderplaybin->m_Uri.c_str() );

		if( !gst_pad_unlink( faderplaybin->m_GhostPad, faderplaybin->m_AdderPad ) )
		{
			//guLogDebug( wxT( "Couldn't unlink stream %s: this is going to suck." ), faderplaybin->m_Uri.c_str() );
		}

		gst_element_release_request_pad( mediactrl->m_Adder, faderplaybin->m_AdderPad );
		faderplaybin->m_AdderPad = NULL;

		mediactrl->m_LinkedStreams--;
		//guLogDebug( wxT( "%d linked streams left" ), mediactrl->m_LinkedStreams );
	}

	faderplaybin->m_NeedsUnlink = false;
	faderplaybin->m_EmittedPlaying = false;

	faderplaybin->Unlock();

	faderplaybin->Reuse();
	if( !faderplaybin->LinkAndUnblock( &Error ) )
	{
		faderplaybin->EmitError( Error );
	}
}

// -------------------------------------------------------------------------------- //
static gboolean set_state_and_wait( GstElement * bin, GstState target, guMediaCtrl * ctrl )
{
    GstBus * bus;
    gboolean waiting;
    gboolean result;

    ////guLogDebug( wxT( "setting playbin state to %s" ), wxString( gst_element_state_get_name( target ), wxConvUTF8 ).c_str() );

    switch( gst_element_set_state( bin, target ) )
    {
        case GST_STATE_CHANGE_SUCCESS :
            ////guLogDebug( wxT( "State change was successful" ) );
            return true;

        case GST_STATE_CHANGE_NO_PREROLL:
            ////guLogDebug( wxT( "state change was successful (no preroll)" ) );
            return true;

        case GST_STATE_CHANGE_ASYNC:
            ////guLogDebug( wxT( "state is changing asynchronously" ) );
            result = true;
            break;

        case GST_STATE_CHANGE_FAILURE:
            ////guLogDebug( wxT( "state change failed" ) );
            result = FALSE;
            break;

        default:
            ////guLogDebug( wxT( "unknown state change return.." ) );
            result = true;
            break;
    }

    bus = gst_element_get_bus( bin );

    waiting = true;

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
                guLogError( wxT( "State change error '%s'" ), GST_TO_WXSTRING( gst_error->message ).c_str() );
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
                    ////guLogDebug( wxT( "playbin reached state %s" ), wxString( gst_element_state_get_name( newstate ), wxConvUTF8 ).c_str() );
                    if( pending == GST_STATE_VOID_PENDING && newstate == target )
                    {
                        waiting = FALSE;
                    }
                }
                break;
            }

            default:
                // pass back to regular message handler
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

// -------------------------------------------------------------------------------- //
static void faderplaybin_notify_source_cb( GstElement * decoder, GParamSpec * pspec, guFaderPlayBin * faderplaybin )
{
//	GstElement * Source;
	//guLogDebug( wxT( "got source notification for stream %s" ), faderplaybin->m_Uri.c_str() );
//	g_object_get( decoder, "source", &Source, NULL );
//	faderplaybin->PrepareSource( Source );
//	g_object_unref (source);
}

// -------------------------------------------------------------------------------- //
static void faderplaybin_pad_added_cb( GstElement * decoder, GstPad * pad, guFaderPlayBin * faderplaybin )
{
	GstCaps * Caps;
	GstStructure * Structure;
	const char * MediaType;
	GstPad * VPad;

	// make sure this is an audio pad
	Caps = gst_pad_get_caps( pad );
	if( gst_caps_is_empty( Caps ) || gst_caps_is_any( Caps ) )
	{
		//guLogDebug( wxT( "got empty/any decoded caps.  hmm?" ) );
		gst_caps_unref( Caps );
		return;
	}

	Structure = gst_caps_get_structure( Caps, 0 );
	MediaType = gst_structure_get_name( Structure );
	if( g_str_has_prefix( MediaType, "audio/x-raw" ) == FALSE )
	{
		guLogMessage( wxT( "got non-audio decoded caps: %s" ), MediaType );
	}
	else if( faderplaybin->m_DecoderLinked )
	{
		// probably should never happen
		guLogMessage( wxT( "hmm, decoder is already linked" ) );
	}
	else
	{
		//guLogDebug( wxT( "got decoded audio pad for stream %s" ), faderplaybin->m_Uri.c_str() );
		VPad = gst_element_get_static_pad( faderplaybin->m_Identity, "sink" );
		gst_pad_link( pad, VPad );
		gst_object_unref( VPad );
		faderplaybin->m_DecoderLinked = true;

		faderplaybin->m_DecoderPad = ( GstPad * ) gst_object_ref( pad );
	}

	gst_caps_unref( Caps );
}

// -------------------------------------------------------------------------------- //
static void faderplaybin_pad_removed_cb( GstElement * decoder, GstPad * pad, guFaderPlayBin * faderplaybin )
{
    if( pad == faderplaybin->m_DecoderPad )
    {
		//guLogDebug( wxT( "active output pad for stream %s removed" ), faderplaybin->m_Uri.c_str() );
		faderplaybin->m_DecoderLinked = false;

		gst_object_unref( faderplaybin->m_DecoderPad );
		faderplaybin->m_DecoderPad = NULL;
	}
}

// -------------------------------------------------------------------------------- //
static void faderplaybin_volume_changed_cb( GObject * object, GParamSpec * pspec, guFaderPlayBin * faderplaybin )
{
	const char * Message = NULL;
    gdouble Vol;

    faderplaybin->Lock();


	g_object_get( faderplaybin->m_Volume, "volume", &Vol, NULL );
	////guLogDebug( wxT( "== Volume Changed to %0.2f ===========================================================" ), Vol );

	switch( faderplaybin->m_State )
	{
	    case guFADERPLAYBIN_STATE_FADING_IN :
	    {
	        //if( faderplaybin->m_Fading && ( Vol == faderplaybin->m_FadeEnd ) )
            if( Vol > ( faderplaybin->m_FadeEnd - 0.001 ) && faderplaybin->m_Fading )
	        {
                //guLogDebug( wxT( "stream %s fully faded in (at %f) -> PLAYING state" ), faderplaybin->m_Uri.c_str(), Vol );
                faderplaybin->m_Fading = false;
                faderplaybin->m_State = guFADERPLAYBIN_STATE_PLAYING;
	        }
	        break;
	    }

	    case guFADERPLAYBIN_STATE_FADING_OUT :
	    case guFADERPLAYBIN_STATE_FADING_OUT_PAUSED :
	    {
	        //if( Vol == faderplaybin->m_FadeEnd )
            if( Vol < ( faderplaybin->m_FadeEnd + 0.001 ) )
	        {
                //guLogDebug( wxT( "stream %s fully faded out (at %f)" ), faderplaybin->m_Uri.c_str(), Vol );
                if( faderplaybin->m_Fading )
                {
                    Message = guFADERPLAYBIN_MESSAGE_FADEOUT_DONE;
                    faderplaybin->m_Fading = false;
                }
	        }
	        else if( !faderplaybin->m_EmittedStartFadeIn && Vol < ( faderplaybin->m_Player->m_FadeInVolTriger + 0.001 ) )
	        {
                //guLogDebug( wxT( "stream %s sending fade in message (at %f)" ), faderplaybin->m_Uri.c_str(), Vol );
                if( faderplaybin->m_Fading )
                {
                    Message = guFADERPLAYBIN_MESSAGE_FADEIN_START;
                    //faderplaybin->m_Fading = false;
                    faderplaybin->m_EmittedStartFadeIn = true;
                }
	        }
	        else
	        {
                // force the volume element out of passthrough mode so it
                // continues to update the controller (otherwise, if the
                // fade out starts at 1.0, it never gets anywhere)
//                gst_base_transform_set_passthrough( GST_BASE_TRANSFORM( faderplaybin->m_Volume ), false );
	        }
	        break;
	    }

	    default :
            break;
	}

    faderplaybin->Unlock();


//	if( faderplaybin->IsFadeOut() )
//	{
//	    if( CurVol == 0.0 )
//	    {
//	        //guLogDebug( wxT( ">>>>>>>>>> Finished the FadeOut" ) );
//	        //faderplaybin->DoFaderEnd();
//	        Message = guFADE_OUT_DONE;
//	    }
//	    else if( CurVol <= 0.5 && faderplaybin->NeedFadeInStart() )
//	    {
//	        faderplaybin->NeedFadeInStart( false );
//	        //guLogDebug( wxT( ">>>>>>>>>> FadeIn should start now" ) );
//	        Message = guFADE_IN_START;
//	    }
//	}
//	else
//	{
//	    if( CurVol == 1.0 )
//	    {
//	        //guLogDebug( wxT( ">>>>>>>>>> Finished the FadeIn" ) );
//	        //faderplaybin->DoFaderEnd();
//	        Message = guFADE_IN_DONE;
//	    }
//	}

	if( Message )
	{
		GstMessage * Msg;
		GstStructure * Struct;

		//guLogDebug( wxT( "posting %s message for stream %s" ), GST_TO_WXSTRING( Message ).c_str(), faderplaybin->m_Uri.c_str() );

		Struct = gst_structure_new( Message, NULL );
		Msg = gst_message_new_application( GST_OBJECT( object ), Struct );

		gst_element_post_message( GST_ELEMENT( object ), Msg );
	}
}

// -------------------------------------------------------------------------------- //
static gboolean faderplaybin_src_event_cb( GstPad * pad, GstEvent * event, guFaderPlayBin * faderplaybin )
{
	GstMessage * Msg;
	GstStructure * S;
	guMediaCtrl * Player;

    //guLogDebug( wxT( "Source Pad event for playbin '%s'" ), faderplaybin->m_Uri.c_str() );

	switch( GST_EVENT_TYPE( event ) )
	{
        case GST_EVENT_EOS :
        {
            guFaderPlayBinArray ToStart;
            //guLogDebug( wxT( "posting EOS message for stream %s" ), faderplaybin->m_Uri.c_str() );

            S = gst_structure_new( guFADERPLAYBIN_MESSAGE_EOS, NULL );
            Msg = gst_message_new_application( GST_OBJECT( faderplaybin->m_PlayBin ), S );
            gst_element_post_message( GST_ELEMENT( faderplaybin->m_PlayBin ), Msg );

            // start playing any streams that were waiting on an EOS
            // (are we really allowed to do this on a stream thread?)
            Player = faderplaybin->m_Player;
            Player->Lock();
            int Index;
            int Count = Player->m_FaderPlayBins.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( Player->m_FaderPlayBins[ Index ]->m_State == guFADERPLAYBIN_STATE_WAITING_EOS )
                {
                    ToStart.Insert( Player->m_FaderPlayBins[ Index ], 0 );
                }
            }
            Player->Unlock();

            Count = ToStart.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                GError * error = NULL;

                //guLogDebug( wxT( "starting stream %s on EOS from previous" ), ToStart[ Index ]->m_Uri.c_str() );
                if( ToStart[ Index ]->LinkAndUnblock( &error ) == false )
                {
                    ToStart[ Index ]->EmitError( error );
                }
            }
            break;
        }

        case GST_EVENT_NEWSEGMENT :
        {
            //guLogDebug( wxT( "got new segment for stream %s" ), faderplaybin->m_Uri.c_str() );
            faderplaybin->AdjustBaseTime();
            break;
        }

        case GST_EVENT_FLUSH_STOP :
        case GST_EVENT_FLUSH_START :
        {
            //guLogDebug( wxT( "dropping %s event for stream %s" ), GST_TO_WXSTRING( GST_EVENT_TYPE_NAME( event ) ).c_str(), faderplaybin->m_Uri.c_str() );
            return false;
        }

        default:
        {
            //guLogDebug( wxT( "got %s event for stream %s" ), GST_TO_WXSTRING( GST_EVENT_TYPE_NAME( event ) ).c_str(), faderplaybin->m_Uri.c_str() );
            break;
        }
	}
	return true;
}

// -------------------------------------------------------------------------------- //
static void perform_seek( guFaderPlayBin * faderplaybin )
{
	GstEvent * Event;

	//guLogDebug( wxT( "sending seek event to %li" ), faderplaybin->m_SeekTarget );

	Event = gst_event_new_seek( 1.0, GST_FORMAT_TIME,
				    GST_SEEK_FLAG_FLUSH,
				    GST_SEEK_TYPE_SET, faderplaybin->m_SeekTarget,
				    GST_SEEK_TYPE_NONE, -1 );

	gst_pad_send_event( faderplaybin->m_SourcePad, Event );

	switch( faderplaybin->m_State )
	{
        case guFADERPLAYBIN_STATE_SEEKING:
            faderplaybin->m_State = guFADERPLAYBIN_STATE_PLAYING;
            break;

        case guFADERPLAYBIN_STATE_SEEKING_PAUSED:
            //guLogDebug( wxT( "leaving paused stream %s unlinked" ), faderplaybin->m_Uri.c_str() );
            faderplaybin->m_State = guFADERPLAYBIN_STATE_PAUSED;
            break;

        case guFADERPLAYBIN_STATE_SEEKING_EOS:
            //guLogDebug( wxT( "waiting for pad block to complete for %s before unlinking" ), faderplaybin->m_Uri.c_str() );
            break;

        default:
            break;
	}
}

// -------------------------------------------------------------------------------- //
static gboolean perform_seek_idle( guFaderPlayBin * faderplaybin )
{
	perform_seek( faderplaybin );

	return false;   // So the timeout dont get repeated
}


// -------------------------------------------------------------------------------- //
static void link_unblocked_cb( GstPad *pad, gboolean blocked, guFaderPlayBin * faderplaybin )
{
	GstStateChangeReturn StateChangeReturn;
	//guLogDebug( wxT( "link_unblocked_cb" ) );

	faderplaybin->Lock();

	// sometimes we seem to get called twice
	if( ( faderplaybin->m_State == guFADERPLAYBIN_STATE_FADING_IN ) ||
        ( faderplaybin->m_State == guFADERPLAYBIN_STATE_PLAYING ) )
    {
		//guLogDebug( wxT( "stream %s already unblocked" ), faderplaybin->m_Uri.c_str() );
		faderplaybin->Unlock();
		return;
	}

	//guLogDebug( wxT( "stream %s is unblocked -> FADING_IN | PLAYING" ), faderplaybin->m_Uri.c_str() );
	faderplaybin->m_SoureBlocked = false;

	if( faderplaybin->m_Fading )
		faderplaybin->m_State = guFADERPLAYBIN_STATE_FADING_IN;
	else
		faderplaybin->m_State = guFADERPLAYBIN_STATE_PLAYING;

    faderplaybin->Unlock();

	faderplaybin->AdjustBaseTime();

	// should handle state change failures here..
	StateChangeReturn = gst_element_set_state( GST_ELEMENT( faderplaybin->m_PlayBin ), GST_STATE_PLAYING );

	//guLogDebug( wxT( "stream %s state change returned: %s" ), faderplaybin->m_Uri.c_str(),GST_TO_WXSTRING( gst_element_state_change_return_get_name( StateChangeReturn ) ).c_str() );

	faderplaybin->PostPlayMessage( false );
}

// -------------------------------------------------------------------------------- //
// called when a stream's source pad is blocked, so it can be unlinked
// from the pipeline.
static void unlink_blocked_cb( GstPad * pad, gboolean blocked, guFaderPlayBin * faderplaybin )
{
	int             StreamState;
	bool            Last;
	guMediaCtrl *   Player;

    faderplaybin->Lock();

	if( !faderplaybin->m_NeedsUnlink || !faderplaybin->m_AdderPad )
	{
		//guLogDebug( wxT( "stream %s doesn't need to be unlinked" ), faderplaybin->m_Uri.c_str() );
		faderplaybin->Unlock();
		return;
	}

	//guLogDebug( wxT( "**stream %s is blocked; unlinking" ), faderplaybin->m_Uri.c_str() );

	if( !gst_pad_unlink( faderplaybin->m_GhostPad, faderplaybin->m_AdderPad ) )
	{
		guLogWarning( wxT( "Couldn't unlink stream %s: things will probably go quite badly from here on" ), faderplaybin->m_Uri.c_str() );
	}
	faderplaybin->m_NeedsUnlink = false;

	gst_element_release_request_pad( GST_PAD_PARENT( faderplaybin->m_AdderPad ), faderplaybin->m_AdderPad );
	faderplaybin->m_AdderPad = NULL;

	faderplaybin->m_SoureBlocked = true;
	faderplaybin->m_EmittedPlaying = false;

	StreamState = faderplaybin->m_State;
	Player = faderplaybin->m_Player;

	faderplaybin->Unlock();

	// might want a stream-paused signal here?
	//guLogDebug( wxT( "** PAUSED ** now stream %s" ), faderplaybin->m_Uri.c_str() );
    //Player->SetCurrentState( GST_STATE_PAUSED );


	Player->m_LinkedStreams--;
	Last = !Player->m_LinkedStreams;
	//guLogDebug( wxT( "%d linked streams left" ), Player->m_LinkedStreams );

	// handle unlinks for seeking and stream reuse
	switch( StreamState )
	{
        case guFADERPLAYBIN_STATE_REUSING :
        {
//	          GError *        Error = NULL;
//            Player->ReuseStream( this );
//            if( link_and_unblock_stream (stream, &error) == FALSE) {
//                emit_stream_error (stream, error);
//            }
            break;
        }

        case guFADERPLAYBIN_STATE_SEEKING_PAUSED :
        {
            g_idle_add( GSourceFunc( perform_seek_idle ), faderplaybin );
            // fall through.  this only happens when pausing, so it's OK
            // to stop the sink here.
        }

        default:
        {
            // consider pausing the sink if this is the linked last stream
            if( Last )
            {
                Player->MaybeStopSink();
            }
            break;
        }
	}
}

// -------------------------------------------------------------------------------- //
static bool adjust_base_time_probe_cb( GstPad * pad, GstBuffer * data, guFaderPlayBin * faderplaybin )
{
	//guLogDebug( wxT( "attempting to adjust base time for stream %s" ), faderplaybin->m_Uri.c_str() );
	faderplaybin->AdjustBaseTime();
	return true;
}

// -------------------------------------------------------------------------------- //
static bool emit_stream_error_cb( guFaderPlayBin * faderplaybin )
{
	faderplaybin->m_ErrorIdleId = 0;
//	_rb_player_emit_error (RB_PLAYER (stream->player),
//			       stream->stream_data,
//			       stream->error);
	g_error_free( faderplaybin->m_Error );
	faderplaybin->m_Error = NULL;
	return FALSE;
}


// -------------------------------------------------------------------------------- //
//
// returns the RBXFadeStream, playback position, and duration of the current
// playing stream.
//
static bool get_times_and_stream( guMediaCtrl * player, guFaderPlayBin ** pfaderplaybin,
                                gint64 * pos, gint64 * duration )
{
	bool                GotTime = false;
	bool                Buffering = false;
	guFaderPlayBin *    FaderPlayBin;

	if( !player->m_Pipeline )
		return false;

	player->Lock();

	// first look for a network stream that is buffering during preroll
	FaderPlayBin = FindFaderPlayBin( player->m_FaderPlayBins,
                guFADERPLAYBIN_STATE_PREROLLING | guFADERPLAYBIN_STATE_PREROLL_PLAY );
	if( FaderPlayBin )
	{
		if( !FaderPlayBin->m_EmittedFakePlaying )
		{
			FaderPlayBin = NULL;
		}
		else
		{
			//guLogDebug( wxT( "found buffering stream %s as current" ), FaderPlayBin->m_Uri.c_str() );
			Buffering = true;
		}
	}

	// otherwise, the stream that is playing
	if( !FaderPlayBin )
	{
		FaderPlayBin = FindFaderPlayBin( player->m_FaderPlayBins,
                            guFADERPLAYBIN_STATE_FADING_IN |
                            guFADERPLAYBIN_STATE_PLAYING |
                            guFADERPLAYBIN_STATE_FADING_OUT |
                            guFADERPLAYBIN_STATE_FADING_OUT_PAUSED |
                            guFADERPLAYBIN_STATE_PAUSED |
                            guFADERPLAYBIN_STATE_PENDING_REMOVE |
                            guFADERPLAYBIN_STATE_REUSING );
	}
	player->Unlock();

	if( FaderPlayBin )
	{
		if( pfaderplaybin )
		{
			* pfaderplaybin = FaderPlayBin;
		}

		if( pos )
		{
			if( Buffering )
			{
				* pos = 0;
			}
			else if( FaderPlayBin->m_State == guFADERPLAYBIN_STATE_PAUSED )
			{
				GstFormat Format = GST_FORMAT_TIME;
				* pos = -1;
				gst_element_query_position( FaderPlayBin->m_Volume, &Format, pos );
			}
			else
			{
				// for playing streams, we subtract the current output position
				// (a running counter generated by the adder) from the position
				// at which we started playback.
				GstFormat Format = GST_FORMAT_TIME;
				* pos = -1;
				gst_element_query_position( player->m_Pipeline, &Format, pos );
				if( *pos != -1 )
				{
					* pos -= FaderPlayBin->m_BaseTime;
				}
				else
				{
					guLogMessage( wxT( "position query failed" ) );
				}
			}
		}

		if( duration )
		{
			GstFormat Format = GST_FORMAT_TIME;
			* duration = -1;
			// queries are supposed to go to sinks, but the closest thing we
			// have in the stream bin is the volume element, which is the last
			// linked element.
			gst_element_query_duration( FaderPlayBin->m_Volume, &Format, duration );
		}
		GotTime = true;
	}
	else
	{
		guLogMessage( wxT( "not playing" ) );
	}

	return GotTime;
}

// -------------------------------------------------------------------------------- //
static bool tick_timeout( guMediaCtrl * mediactrl )
{
	gint64 Pos = -1;
	gint64 Duration = -1;
	guFaderPlayBin * FaderPlayBin = NULL;

	if( get_times_and_stream( mediactrl, &FaderPlayBin, &Pos, &Duration ) )
	{
		//_rb_player_emit_tick (RB_PLAYER (player), stream->stream_data, pos, duration);
        guMediaEvent event( guEVT_MEDIA_CHANGED_POSITION );
        event.SetInt( Pos / GST_MSECOND );
        event.SetExtraLong( Duration / GST_MSECOND );
        mediactrl->AddPendingEvent( event );
	}
//	if( FaderPlayBin )
//        //guLogDebug( wxT( "%i %s" ), FaderPlayBin->m_State, FaderPlayBin->m_Uri.c_str() );
//
//    //guLogDebug( wxT( "%"G_GINT64_FORMAT"  %"G_GINT64_FORMAT ), Pos, Duration );
	return true;
}

// -------------------------------------------------------------------------------- //
static gboolean stop_sink_later( guMediaCtrl * player )
{
	player->Lock();
	player->m_SinkLock.Lock();
	player->m_StopSinkId = 0;
	//if( !player->m_LinkedStreams )
	if( !player->m_FaderPlayBins.Count() )
	{
		player->StopSink();
	}
	player->m_SinkLock.Unlock();
	player->Unlock();

	return false;
}

// -------------------------------------------------------------------------------- //
// idle handler used to clean up finished streams
static gboolean reap_streams( guMediaCtrl * player )
{
	wxArrayPtrVoid ToDelete;

    player->Lock();
	player->m_PlayBinReapId = 0;
	//dump_stream_list (player);
	int Index;
	int Count = player->m_FaderPlayBins.Count();
	for( Index = Count - 1; Index >= 0; Index-- )
	{
	    guFaderPlayBin * FaderPlayBin = player->m_FaderPlayBins[ Index ];
		if( FaderPlayBin->m_State == guFADERPLAYBIN_STATE_PENDING_REMOVE )
		{
			ToDelete.Add( FaderPlayBin );
			player->m_FaderPlayBins.RemoveAt( Index );
		}
	}
	player->Unlock();

	Count = ToDelete.Count();

	for( Index = 0; Index < Count; Index++ )
	{
		//guLogDebug( wxT( "reaping stream %s" ), ( ( guFaderPlayBin * ) ToDelete[ Index ] )->m_Uri.c_str() );
		delete ( ( guFaderPlayBin * ) ToDelete[ Index ] );
	}

#ifdef guSHOW_DUMPFADERPLAYBINS
    player->Lock();
    DumpFaderPlayBins( player->m_FaderPlayBins );
    player->Unlock();
#endif

    player->MaybeStopSink();

	return false;
}

// called on a streaming thread when the stream src pad is blocked
// (that is, when prerolling is complete).  in some situations we
// start playback immediately, otherwise we wait for something else
// to happen.
static void stream_src_blocked_cb( GstPad * pad, gboolean blocked, guFaderPlayBin * faderplaybin )
{
	GError *    error = NULL;
	bool        StartStream = false;

    //guLogDebug( wxT( "Preroll is complete now...?" ) );

	faderplaybin->Lock();

	if( faderplaybin->m_SoureBlocked )
	{
		//guLogDebug( wxT( "stream %s already blocked" ), faderplaybin->m_Uri.c_str() );
		faderplaybin->Unlock();
		return;
	}
	faderplaybin->m_SoureBlocked = true;

	g_object_set( faderplaybin->m_PreRoll,
        "min-threshold-time", G_GINT64_CONSTANT( 0 ),
		"max-size-buffers", 200,		            // back to normal value
		NULL );

	// update stream state
	switch( faderplaybin->m_State )
	{
        case guFADERPLAYBIN_STATE_PREROLLING :
        {
            //guLogDebug( wxT( "stream %s is prerolled, not starting yet -> WAITING" ), faderplaybin->m_Uri.c_str() );
            faderplaybin->m_State = guFADERPLAYBIN_STATE_WAITING;

            guMediaEvent event( guEVT_MEDIA_LOADED );
            event.SetInt( true );
            faderplaybin->m_Player->AddPendingEvent( event );

            break;
        }

        case guFADERPLAYBIN_STATE_PREROLL_PLAY:
            //guLogDebug( wxT( "stream %s is prerolled, need to start it" ), faderplaybin->m_Uri.c_str() );
            StartStream = true;
            break;

        default:
            //guLogDebug( wxT( "didn't expect to get preroll completion callback in this state (%d)" ), faderplaybin->m_State );
            break;

	}

	faderplaybin->Unlock();

	if( StartStream )
	{
		// not sure this is actually an acceptable thing to do on a streaming thread..
		if( !faderplaybin->ActuallyStart( &error ) )
		{
			faderplaybin->EmitError( error );
		}
	}
}

static void post_eos_seek_blocked_cb( GstPad * pad, gboolean blocked, guFaderPlayBin * faderplaybin )
{
	GError * Error = NULL;

	faderplaybin->Lock();

	//guLogDebug( wxT( "stream %s is blocked; linking and unblocking" ), faderplaybin->m_Uri.c_str() );
	faderplaybin->m_SoureBlocked = true;
	if( !faderplaybin->LinkAndUnblock( &Error ) )
	{
		faderplaybin->EmitError( Error );
	}
    faderplaybin->Unlock();
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
                    //g_object_set( level, "message", true, NULL );
                    g_object_set( level, "interval", gint64( 200000000 ), NULL );
                    //g_object_set( level, "interval", gint64( 100000000 ), NULL );

                    m_Volume = gst_element_factory_make( "volume", "pb_volume" );
                    if( IsValidElement( m_Volume ) )
                    {
                        m_Equalizer = gst_element_factory_make( "equalizer-10bands", "pb_equalizer" );
                        if( IsValidElement( m_Equalizer ) )
                        {
                            GstElement * limiter = gst_element_factory_make( "rglimiter", "pb_rglimiter" );
                            if( IsValidElement( limiter ) )
                            {
                                //g_object_set( G_OBJECT( limiter ), "enabled", true, NULL );

                                GstElement * outconverter = gst_element_factory_make( "audioconvert", "pb_audioconvert2" );
                                if( IsValidElement( outconverter ) )
                                {

                                    m_Tee = gst_element_factory_make( "tee", "pb_tee" );
                                    if( IsValidElement( m_Tee ) )
                                    {
                                        GstElement * queue = gst_element_factory_make( "queue", "pb_queue" );
                                        if( IsValidElement( queue ) )
                                        {
                                            g_object_set( queue, "max-size-buffers", 10, NULL );
                                            //g_object_set( queue, "max-size-time", guint64( GST_SECOND / 2 ), NULL );
                                            //g_object_set( queue, "max-size-time", 5 * GST_SECOND, "max-size-buffers", 0, "max-size-bytes", 0, NULL );

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
                g_object_set( m_FileSink, "location", ( const char * ) path.mb_str( wxConvFile ), NULL );

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

                    //gst_element_set_state( recordbin, GST_STATE_PAUSED );
                    gst_bin_add( GST_BIN( m_PlaybackBin ), recordbin );

                    GstPad * pad = gst_element_get_pad( queue, "sink" );
                    if( GST_IS_PAD( pad ) )
                    {
                        GstPad * ghostpad = gst_ghost_pad_new( "sink", pad );
                        gst_element_add_pad( recordbin, ghostpad );
                        gst_object_unref( pad );

                        //gst_element_link( m_Tee, recordbin );
                        m_RecordPad = gst_element_get_request_pad( m_Tee, "src%d" );
                        gst_pad_link( m_RecordPad, ghostpad );

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

    m_Pipeline = NULL;
    m_Adder = NULL;

    m_PlaybackBin = NULL;
    m_Tee = NULL;
    m_RecordPad = NULL;

    m_Volume = NULL;
    m_Equalizer = NULL;

    m_RecordBin = NULL;
    m_FileSink = NULL;

    m_Buffering = false;
    m_WasPlaying = false;

    m_LastError = 0;

    m_BusWatchId = 0;
    m_TickTimeoutId = 0;
    m_PlayBinReapId = 0;
    m_StopSinkId = 0;
    m_BusWatchId = 0;

    if( Init() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        // FadeOutTime defines the length of the song to fade out
        // FadeInTIme defines the length of the song to fade in
        // FadeInVolStart defines the inital volume of the song when fading in
        // FadeInVolTriger defines at which fadeout volume the fade in starts
        m_FadeOutTime       = Config->ReadNum( wxT( "FadeOutTime" ), 5, wxT( "Crossfader" ) ) * GST_SECOND;
        m_FadeInTime        = Config->ReadNum( wxT( "FadeInTime" ), 1, wxT( "Crossfader" ) ) * GST_SECOND;
        m_FadeInVolStart    = double( Config->ReadNum( wxT( "FadeInVolStart" ), 8, wxT( "Crossfader" ) ) ) / 10.0;
        m_FadeInVolTriger   = double( Config->ReadNum( wxT( "FadeInVolTriger" ), 5, wxT( "Crossfader" ) ) ) / 10.0;
        //guLogDebug( wxT( "FOT: %li  FIT: %li  FIV: %0.2f  FIVT: %0.2f" ), m_FadeOutTime, m_FadeInTime, m_FadeInVolStart, m_FadeInVolTriger );

        GstCaps * Caps;
        Caps = gst_caps_new_simple( "audio/x-raw-int",
                    "channels", G_TYPE_INT, 2,
                    "rate",	G_TYPE_INT, 44100,
                    "width", G_TYPE_INT, 16,
                    "depth", G_TYPE_INT, 16,
                    NULL );

        m_Pipeline = gst_pipeline_new( "CrossFadeBin" );
        AddBusWatch();

        m_SilenceBin = gst_bin_new( "silencebin" );
        GstElement * AudioTestSrc = gst_element_factory_make( "audiotestsrc", "silencesrc" );
        g_object_set( AudioTestSrc, "wave", GST_AUDIO_TEST_SRC_WAVE_SILENCE, NULL);
        GstElement * SilenceConvert = gst_element_factory_make( "audioconvert", "silenceconvert" );
        GstElement * CapsFilter = gst_element_factory_make( "capsfilter", "silencecaps" );
        g_object_set( CapsFilter, "caps", Caps, NULL );
        gst_caps_unref( Caps );


        gst_bin_add_many( GST_BIN( m_SilenceBin ), AudioTestSrc, SilenceConvert, CapsFilter, NULL );
        gst_element_link_many( AudioTestSrc, SilenceConvert, CapsFilter, NULL );

        GstPad * SilencePad = gst_element_get_static_pad( CapsFilter, "src" );
        GstPad * ghostpad = gst_ghost_pad_new( NULL, SilencePad );
        gst_element_add_pad( m_SilenceBin, ghostpad );
        gst_object_unref( SilencePad );

        m_Adder = gst_element_factory_make( "adder", "adder" );

        // Get the audio output sink
        m_OutputBin = BuildOutputBin();

        m_PlaybackBin = BuildPlaybackBin( m_OutputBin );

        // Add the elements to the pipeline
        gst_bin_add_many( GST_BIN( m_Pipeline ), m_Adder, m_PlaybackBin, m_SilenceBin, NULL );
        gst_element_link_many( m_Adder, m_PlaybackBin, NULL );

        GstPad * ReqPad = gst_element_get_request_pad( m_Adder, "sink%d" );
        gst_pad_link( ghostpad, ReqPad );

//        gst_bus_add_watch( gst_pipeline_get_bus( GST_PIPELINE( m_Pipeline ) ),
//                ( GstBusFunc ) gst_bus_async_callback, this );

        m_SinkState = SINK_STOPPED;

//        //g_timeout_add( 250, ( GSourceFunc ) update_position, this );
//
//        if( gst_element_set_state( m_Pipeline, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
//        {
//            guLogMessage( wxT( "Could not start the main pipeline" ) );
//        }

        return;

//        g_object_set( G_OBJECT( m_Playbin ), "audio-sink", m_PlaybackBin, NULL );

        // Be sure we only play audio
//        g_object_set( G_OBJECT( m_Playbin ), "flags", 0x02 | 0x10, NULL );
        //g_object_set( G_OBJECT( m_Playbin ), "buffer-size", 256*1024, NULL );

//        g_signal_connect( G_OBJECT( m_Playbin ), "about-to-finish",
//            G_CALLBACK( gst_about_to_finish ), ( void * ) this );
        //
//        gst_bus_add_watch( gst_pipeline_get_bus( GST_PIPELINE( m_Playbin ) ),
//            ( GstBusFunc ) gst_bus_async_callback, this );
    }
}

// -------------------------------------------------------------------------------- //
guMediaCtrl::~guMediaCtrl()
{
    if( m_Pipeline )
    {
        wxASSERT( GST_IS_OBJECT( m_Pipeline ) );
        gst_element_set_state( m_Pipeline, GST_STATE_NULL );
        gst_object_unref( GST_OBJECT( m_Pipeline ) );
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::EnableRecord( const wxString &recfile, const int format, const int quality )
{
    GstElement * Encoder = NULL;
    GstElement * Muxer = NULL;
    gint Mp3Quality[] = { 320, 192, 128, 96, 64 };
    float OggQuality[] = { 0.9, 0.7, 0.5, 0.3, 0.1 };
    gint FlacQuality[] = { 9, 7, 5, 3, 1 };

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
        guMediaState State = GetState();
        if( State == guMEDIASTATE_PLAYING )
        {
            if( gst_element_set_state( m_Pipeline, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
            {
                guLogMessage( wxT( "Could not set state inserting record object" ) );
                return false;
            }
        }

        m_RecordBin = BuildRecordBin( recfile, Encoder, Muxer );

        if( State == guMEDIASTATE_PLAYING )
        {
            if( gst_element_set_state( m_Pipeline, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
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

    gst_element_get_state( m_Pipeline, &CurState, NULL, 0 );

    if( CurState == GST_STATE_PLAYING )
    {
        ////guLogDebug( wxT( "Trying to set state to pased" ) );
        if( gst_element_set_state( m_RecordBin, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogMessage( wxT( "Could not set record state removing record object" ) );
        }

        //if( gst_element_set_state( m_Pipeline, GST_STATE_PAUSED ) == GST_STATE_CHANGE_FAILURE )
        if( !set_state_and_wait( m_Pipeline, GST_STATE_PAUSED, this ) )
        {
            guLogMessage( wxT( "Could not set playbin state removing record object" ) );
        }
    }

    gst_element_set_state( m_RecordBin, GST_STATE_NULL );

    gst_bin_remove( GST_BIN( m_PlaybackBin ), m_RecordBin );
    gst_object_unref( m_RecordBin );
    m_RecordBin = NULL;
    m_FileSink = NULL;

    if( CurState == GST_STATE_PLAYING )
    {
        if( gst_element_set_state( m_Pipeline, GST_STATE_PLAYING ) == GST_STATE_CHANGE_FAILURE )
        {
            guLogMessage( wxT( "Could not restore state inserting record object" ) );
        }
    }

}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetRecordFileName( const wxString &filename )
{
    if( !m_RecordBin || m_Buffering )
        return false;

//    m_RecordFileName = m_RecordPath + filename + m_RecordExt;
//    wxFileName::Mkdir( wxPathOnly( m_RecordFileName ), 0770, wxPATH_MKDIR_FULL );

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
wxFileOffset guMediaCtrl::Tell()
{
    gint64 pos;
    GstFormat format = GST_FORMAT_TIME;

    if( gst_element_query_position( m_Pipeline, &format, &pos ) && pos != -1 )
    {
        return pos / GST_MSECOND;
    }
    else
    {
        return 0; //m_PausedPos;
    }
}

// -------------------------------------------------------------------------------- //
wxFileOffset guMediaCtrl::GetLength()
{
    gint64 len;
    GstFormat format = GST_FORMAT_TIME;

    if( gst_element_query_duration( m_Pipeline, &format, &len ) && len != -1 )
    {
        ////guLogDebug( wxT( "TrackLength: %lld / %lld = %lld" ), len, GST_SECOND, len / GST_SECOND );
        return ( len / GST_SECOND );
    }
    else
    {
        return 0;
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::SetVolume( double dVolume )
{
    //guLogDebug( wxT( "Volume: %0.5f" ), dVolume );
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
void guMediaCtrl::ClearError( void )
{
    if( gst_element_set_state( m_Pipeline, GST_STATE_NULL ) == GST_STATE_CHANGE_FAILURE )
    {
        guLogMessage( wxT( "Error restoring the gstreamer status." ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::AddPendingEvent( guMediaEvent &event )
{
    wxPostEvent( m_PlayerPanel, event );
}

// -------------------------------------------------------------------------------- //
guMediaState guMediaCtrl::GetState( void )
{
    if( m_CurrentState == GST_STATE_PLAYING )
    {
        return guMEDIASTATE_PLAYING;
    }
    else if( m_CurrentState == GST_STATE_PAUSED )
    {
        return guMEDIASTATE_PAUSED;
    }
    else
    {
        return guMEDIASTATE_STOPPED;
    }
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::SetCurrentState( GstState state )
{
    m_CurrentState = state;

    //guLogDebug( wxT( "Setting state to %i" ), state );

    guMediaEvent event( guEVT_MEDIA_CHANGED_STATE );
    event.SetInt( state );
    AddPendingEvent( event );
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
    bInited = gst_init_check( &argcGST, &argvGST, &error ) &&
              gst_controller_init( &argcGST, &argvGST );

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
void guMediaCtrl::AddBusWatch( void )
{
	GstBus * Bus;

	Bus = gst_element_get_bus( GST_ELEMENT( m_Pipeline ) );
	m_BusWatchId = gst_bus_add_watch( Bus, GstBusFunc(gst_bus_async_callback ), this );
	gst_object_unref( Bus );
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::StartSink( GError ** error )
{
    //guLogDebug( wxT( "guMediaCtrl::StartSink" ) );

	wxArrayPtrVoid  Messages;
	GstBus *        Bus;
	bool            Ret = false;

    m_SinkLock.Lock();

	switch( m_SinkState )
	{
        case SINK_NULL:
            //g_assert_not_reached ();
            break;

        case SINK_STOPPED:
            // prevent messages from being processed by the main thread while we're starting the sink */
            g_source_remove( m_BusWatchId );
            Ret = StartSinkLocked( Messages, error );
            AddBusWatch();
            break;

        case SINK_PLAYING:
            Ret = true;
            break;

        default:
            //g_assert_not_reached ();
            break;
	}

    m_SinkLock.Unlock();

	Bus = gst_element_get_bus( GST_ELEMENT( m_Pipeline ) );
	int Index;
	int Count = Messages.Count();
	for( Index = 0; Index < Count; Index++ )
	{
		gst_bus_async_callback( Bus, ( GstMessage * ) Messages[ Index ], this );
		gst_message_unref( ( GstMessage * ) Messages[ Index ] );
	}
	gst_object_unref( Bus );

	return Ret;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::StartSinkLocked( wxArrayPtrVoid &messages, GError ** error )
{
	GstStateChangeReturn    sr;
	bool                    Waiting;
//	GError *                GenericError = NULL;
	GstBus *                Bus;

//	g_set_error (&generic_error,
//		     RB_PLAYER_ERROR,
//		     RB_PLAYER_ERROR_INTERNAL,		/* ? */
//		     _("Failed to open output device"));

	//guLogDebug( wxT( "starting sink" ) );

	// first, start the output bin.
	// this won't preroll until we start the silence bin.
	sr = gst_element_set_state( m_PlaybackBin, GST_STATE_PAUSED );
	if( sr == GST_STATE_CHANGE_FAILURE )
	{
		guLogMessage( wxT( "output bin state change failed" ) );
		//g_propagate_error (error, generic_error);
		return false;
	}

	// then the adder
	sr = gst_element_set_state( m_Adder, GST_STATE_PAUSED );
	if( sr == GST_STATE_CHANGE_FAILURE )
	{
		guLogMessage( wxT( "adder state change failed" ) );
		//g_propagate_error (error, generic_error);
		return false;
	}

	// then the silence bin
	sr = gst_element_set_state( m_SilenceBin, GST_STATE_PAUSED );
	if( sr == GST_STATE_CHANGE_FAILURE )
	{
		guLogMessage( wxT( "silence bin state change failed" ) );
		//g_propagate_error (error, generic_error);
		return false;
	}

	// now wait for everything to finish
	Waiting = true;
	Bus = gst_element_get_bus( GST_ELEMENT( m_Pipeline ) );
	while( Waiting )
	{
		GstMessage * Message;
		GstState OldState;
		GstState NewState;
		GstState Pending;

		Message = gst_bus_timed_pop( Bus, GST_SECOND * 5 );
		if( !Message )
		{
			guLogMessage( wxT( "sink is taking too long to start.." ) );
			//g_propagate_error (error, generic_error);
			gst_object_unref( Bus );
			return false;
		}

		switch( GST_MESSAGE_TYPE( Message ) )
		{
            case GST_MESSAGE_ERROR:
            {
                GError * gst_error = NULL;
                GstObject * MessageSrc;
                guFaderPlayBin * FaderPlayBin;

            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS( GST_BIN( m_Pipeline ), GST_DEBUG_GRAPH_SHOW_ALL , "guayadeque" );

                // we only want to process errors from the sink here.
                // errors from streams should go to the normal message handler.
                MessageSrc = GST_MESSAGE_SRC( Message );

                FaderPlayBin = FindFaderPlayBin( m_FaderPlayBins, GST_ELEMENT( MessageSrc ) );

                if( FaderPlayBin )
                {
                    guLogMessage( wxT( "got an error from a stream; passing it to the bus handler" ) );
                    messages.Add( gst_message_ref( Message ) );
                }
                else
                {
                    gst_message_parse_error( Message, &gst_error, NULL );
                    guLogMessage( wxT( "got error message: '%s'" ), GST_TO_WXSTRING( gst_error->message ).c_str() );
                    gst_message_unref( Message );

                    if( error != NULL && * error == NULL )
                    {
                        // Translators: the parameter here is an error message */
//                            g_set_error (error,
//                                     RB_PLAYER_ERROR,
//                                     RB_PLAYER_ERROR_INTERNAL,		/* ? */
//                                     _("Failed to open output device: %s"),
//                                     gst_error->message);
                    }
                    g_error_free( gst_error );
                    //g_error_free( GenericError );

                    gst_element_set_state( m_PlaybackBin, GST_STATE_NULL );
                    gst_element_set_state( m_Adder, GST_STATE_NULL );
                    gst_element_set_state( m_SilenceBin, GST_STATE_NULL );
                    gst_object_unref( Bus );
                    return false;
                }
                break;
            }

            case GST_MESSAGE_STATE_CHANGED:
            {
                gst_message_parse_state_changed( Message, &OldState, &NewState, &Pending );
                if( NewState == GST_STATE_PAUSED && Pending == GST_STATE_VOID_PENDING )
                {
                    if( GST_MESSAGE_SRC( Message ) == GST_OBJECT( m_PlaybackBin ) )
                    {
                        //guLogDebug( wxT( "outputbin is now PAUSED" ) );
                        Waiting = false;
                    }
                    else if( GST_MESSAGE_SRC( Message ) == GST_OBJECT( m_Adder ) )
                    {
                        //guLogDebug( wxT( "adder is now PAUSED" ) );
                    }
                    else if( GST_MESSAGE_SRC( Message ) == GST_OBJECT( m_SilenceBin ) )
                    {
                        //guLogDebug( wxT( "silencebin is now PAUSED" ) );
                    }
                }
                break;
            }

            default:
                // save the message to pass to the bus callback once we've dropped
                // the sink lock.
                //*messages = g_list_append (*messages, gst_message_ref (message));
                messages.Add( gst_message_ref( Message ) );
                break;
		}

		gst_message_unref( Message );
	}
	gst_object_unref( Bus );


	sr = gst_element_set_state( m_SilenceBin, GST_STATE_PLAYING );
	if( sr == GST_STATE_CHANGE_FAILURE )
	{
		guLogMessage( wxT( "silence bin state change failed" ) );
		//g_propagate_error (error, generic_error);
		return false;
	}

	sr = gst_element_set_state( m_Adder, GST_STATE_PLAYING );
	if( sr == GST_STATE_CHANGE_FAILURE )
	{
		guLogMessage( wxT( "adder state change failed" ) );
		//g_propagate_error (error, generic_error);
		return false;
	}

	sr = gst_element_set_state( m_PlaybackBin, GST_STATE_PLAYING );
	if( sr == GST_STATE_CHANGE_FAILURE )
	{
		guLogMessage( wxT( "output bin state change failed" ) );
		//g_propagate_error (error, generic_error);
		return false;
	}

	//guLogDebug( wxT( "sink playing" ) );
	m_SinkState = SINK_PLAYING;

	// set the pipeline to PLAYING so it selects a clock
	gst_element_set_state( m_Pipeline, GST_STATE_PLAYING );

	// now that the sink is running, start polling for playing position.
	// might want to replace this with a complicated set of pad probes
	// to avoid polling, but duration queries on the sink are better
	// as they account for internal buffering etc.  maybe there's a way
	// to account for that in a pad probe callback on the sink's sink pad?
	if( !m_TickTimeoutId )
	{
		gint ms_period = 1000 / 5;
		m_TickTimeoutId = g_timeout_add( ms_period, GSourceFunc( tick_timeout ), this );
	}
	return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::StopSink( void )
{
	switch( m_SinkState )
	{
        case SINK_PLAYING :
        {
            //guLogDebug( wxT( "stopping sink" ) );

            if( m_TickTimeoutId )
            {
                g_source_remove( m_TickTimeoutId );
                m_TickTimeoutId = 0;
            }

            if( gst_element_set_state( m_PlaybackBin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
            {
                guLogMessage( wxT( "couldn't stop output bin" ) );
                return false;
            }

            if( gst_element_set_state( m_Adder, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
            {
                guLogMessage( wxT( "couldn't stop adder" ) );
                return false;
            }

            if( gst_element_set_state( m_SilenceBin, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
            {
                guLogMessage( wxT( "couldn't stop silence bin" ) );
                return false;
            }

            // try stopping the sink, but don't worry if we can't
            if( gst_element_set_state( m_OutputBin, GST_STATE_NULL ) == GST_STATE_CHANGE_FAILURE )
            {
                guLogMessage( wxT( "couldn't set audio sink to NULL state" ) );
            }

//            if (player->priv->volume_handler) {
//                g_object_unref (player->priv->volume_handler);
//                player->priv->volume_handler = NULL;
//            }

            // set the pipeline to READY so we can make it select a clock when we
            // start the sink again.  everything inside the pipeline has its state
            // locked, so this doesn't affect anything else.
            gst_element_set_state( m_Pipeline, GST_STATE_READY );

            m_SinkState = SINK_STOPPED;

            SetCurrentState( GST_STATE_READY );
            break;
        }

        case SINK_STOPPED:
        case SINK_NULL:
            break;
	}

	return true;
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::MaybeStopSink( void )
{
	m_SinkLock.Lock();
	if( !m_StopSinkId )
	{
		m_StopSinkId = g_timeout_add( 1000, GSourceFunc( stop_sink_later ), this );
	}
	m_SinkLock.Unlock();
}

// -------------------------------------------------------------------------------- //
void guMediaCtrl::ScheduleReap( void )
{
    Lock();

	if( !m_PlayBinReapId )
	{
#ifdef guSHOW_DUMPFADERPLAYBINS
        DumpFaderPlayBins( m_FaderPlayBins );
#endif
		m_PlayBinReapId = g_idle_add( GSourceFunc( reap_streams ), this );
	}

	Unlock();
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Load( const wxString &uri, guPlayerPlayType playtype )
{
    //guLogDebug( wxT( "**************************************************************************************************************** MediaCtrl::Load" ) );

	//GError * Error = NULL;
	bool Reused = false;
	guFaderPlayBin * FaderPlayBin;

	// create sink if we don't already have one
	//if( !StartSink( &Error ) )
	//	return false;

	// see if anyone wants us to reuse an existing stream
	Lock();
	int Index;
	int Count = m_FaderPlayBins.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        FaderPlayBin = m_FaderPlayBins[ Index ];
		switch( FaderPlayBin->m_State )
		{
            case guFADERPLAYBIN_STATE_WAITING:
            case guFADERPLAYBIN_STATE_PENDING_REMOVE:
            case guFADERPLAYBIN_STATE_REUSING:
            case guFADERPLAYBIN_STATE_SEEKING:
            case guFADERPLAYBIN_STATE_SEEKING_PAUSED:
            case guFADERPLAYBIN_STATE_SEEKING_EOS:
            case guFADERPLAYBIN_STATE_PREROLLING:
            case guFADERPLAYBIN_STATE_PREROLL_PLAY:
                break;

            case guFADERPLAYBIN_STATE_PLAYING:
            case guFADERPLAYBIN_STATE_FADING_IN:
            case guFADERPLAYBIN_STATE_FADING_OUT:
            case guFADERPLAYBIN_STATE_FADING_OUT_PAUSED:
            case guFADERPLAYBIN_STATE_WAITING_EOS:
            case guFADERPLAYBIN_STATE_PAUSED:
                Reused = CanReuse( uri, FaderPlayBin );
                //g_signal_emit (player,
                //           signals[CAN_REUSE_STREAM], 0,
                //           uri, stream->uri, GST_ELEMENT (stream),
                //           &reused);
                break;
		}

		if( Reused )
		{
			//guLogDebug( wxT( "reusing stream %s for new stream %s" ), FaderPlayBin->m_Uri.c_str(), uri.c_str() );
			FaderPlayBin->m_State = guFADERPLAYBIN_STATE_REUSING;
			FaderPlayBin->m_NewUri = uri;

			// move the stream to the front of the list so it'll be started when
			// _play is called (it's probably already there, but just in case..)
			//
			m_FaderPlayBins.Remove( FaderPlayBin );
			m_FaderPlayBins.Insert( FaderPlayBin, 0 );
			break;
		}
	}
	Unlock();

	if( Reused )
	{
		return true;
	}

	// construct new stream
	FaderPlayBin = new guFaderPlayBin( this, uri );
	if( !FaderPlayBin )
	{
		guLogMessage( wxT( "unable to create pipeline to play %s" ), uri.c_str() );
        //g_set_error (error,
        //         RB_PLAYER_ERROR,
        //         RB_PLAYER_ERROR_GENERAL,
        //         _("Failed to create GStreamer pipeline to play %s"),
        //         uri);
		return false;
	}

    FaderPlayBin->m_PlayType = playtype;
	Lock();
	m_FaderPlayBins.Insert( FaderPlayBin, 0 );
#ifdef guSHOW_DUMPFADERPLAYBINS
    DumpFaderPlayBins( m_FaderPlayBins );
#endif
	Unlock();

	// start prerolling it
	if( !FaderPlayBin->Preroll() )
	{
		guLogMessage( wxT( "unable to preroll stream %s" ), uri.c_str() );
        //g_set_error (error,
        //         RB_PLAYER_ERROR,
        //         RB_PLAYER_ERROR_GENERAL,
        //         _("Failed to start playback of %s"),
        //         uri);
		return false;
	}

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Play( void )
{
    guLogDebug( wxT( "**************************************************************************************************************** MediaCtrl::Play" ) );

	int                 StreamState;
	bool                Ret = true;
	GError *            Error = NULL;
	guFaderPlayBin *    FaderPlayBin;

	Lock();

	// is there anything to play?
	if( !m_FaderPlayBins.Count() )
	{
        //g_set_error (error,
        //         RB_PLAYER_ERROR,
        //         RB_PLAYER_ERROR_GENERAL,
        //         "Nothing to play");		// should never happen
		Unlock();
		return false;
	}

	FaderPlayBin = m_FaderPlayBins[ 0 ];
	Unlock();

	// make sure the sink is playing
	if( !StartSink( &Error ) )
	{
		return false;
	}

	FaderPlayBin->Lock();

	guLogDebug( wxT( "playing stream %s, play type %d, crossfade %" G_GINT64_FORMAT ), FaderPlayBin->m_Uri.c_str(), 0, m_FadeOutTime );

	// handle transitional states while holding the lock, and handle states that
	// require action outside it (lock precedence, mostly)
	switch( FaderPlayBin->m_State )
	{
        case guFADERPLAYBIN_STATE_PREROLLING:
        case guFADERPLAYBIN_STATE_PREROLL_PLAY:
            guLogDebug( wxT( "stream %s is prerolling; will start playback once prerolling is complete -> PREROLL_PLAY" ), FaderPlayBin->m_Uri.c_str() );
            //FaderPlayBin->m_PlayType = guFADERPLAYBIN_PLAYTYPE_CROSSFADE;
            //FaderPlayBin->m_FadeOutTime = m_FadeOutTime;
            FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PREROLL_PLAY;
            break;

        case guFADERPLAYBIN_STATE_SEEKING_PAUSED:
            guLogDebug( wxT( "unpausing seeking stream %s" ), FaderPlayBin->m_Uri.c_str() );
            FaderPlayBin->m_State = guFADERPLAYBIN_STATE_SEEKING;
            break;

        case guFADERPLAYBIN_STATE_PENDING_REMOVE:
            guLogDebug( wxT( "hmm, can't play streams in PENDING_REMOVE state.." ) );
            break;

        default:
            break;
	}

	StreamState = FaderPlayBin->m_State;
	FaderPlayBin->Unlock();

	// is the head stream already playing?
	switch( StreamState )
	{
        case guFADERPLAYBIN_STATE_FADING_IN :
        case guFADERPLAYBIN_STATE_FADING_OUT :
        case guFADERPLAYBIN_STATE_FADING_OUT_PAUSED :
        case guFADERPLAYBIN_STATE_PLAYING :
        case guFADERPLAYBIN_STATE_SEEKING :
        case guFADERPLAYBIN_STATE_SEEKING_EOS :
        {
            guLogDebug( wxT( "stream %s is already playing" ), FaderPlayBin->m_Uri.c_str() );

            //_rb_player_emit_playing_stream (RB_PLAYER (player), stream->stream_data);
            SetCurrentState( GST_STATE_PLAYING );
            break;
        }

        case guFADERPLAYBIN_STATE_PAUSED :
        {
            guLogDebug( wxT( "unpausing stream %s" ), FaderPlayBin->m_Uri.c_str() );
            FaderPlayBin->StartFade( 0.0f, 1.0f, m_FadeOutTime ? guFADERPLAYBIN_FAST_FADER_TIME : 250 );
            Ret = FaderPlayBin->LinkAndUnblock( &Error );
            break;
        }

        case guFADERPLAYBIN_STATE_WAITING_EOS :
        case guFADERPLAYBIN_STATE_WAITING :
        {
            //FaderPlayBin->m_PlayType = guFADERPLAYBIN_PLAYTYPE_CROSSFADE;
            //FaderPlayBin->m_FadeOutTime = m_FadeOutTime;
            Ret = FaderPlayBin->ActuallyStart( &Error );
            break;
        }

        case guFADERPLAYBIN_STATE_REUSING :
            switch( FaderPlayBin->m_PlayType )
            {
                case guFADERPLAYBIN_PLAYTYPE_REPLACE :
                case guFADERPLAYBIN_PLAYTYPE_CROSSFADE :
                    // probably should split this into two states..
                    if( FaderPlayBin->m_SoureBlocked )
                    {
                        guLogDebug( wxT( "reusing and restarting paused stream %s" ), FaderPlayBin->m_Uri.c_str() );
                        FaderPlayBin->Reuse();
                        Ret = FaderPlayBin->LinkAndUnblock( &Error );
                    }
                    else
                    {
                        guLogDebug( wxT( "unlinking stream %s for reuse" ), FaderPlayBin->m_Uri.c_str() );
                        FaderPlayBin->UnlinkAndBlock();
                    }
                    break;
                case guFADERPLAYBIN_PLAYTYPE_AFTER_EOS :
                    guLogMessage( wxT( "waiting for EOS before reusing stream %s" ), FaderPlayBin->m_Uri.c_str() );
                    break;
            }
            break;

        default:
            break;
	}

	return Ret;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Pause( void )
{
    //guLogDebug( wxT( "**************************************************************************************************************** MediaCtrl::Pause" ) );

    guFaderPlayBin * FaderPlayBin = NULL;
	wxArrayPtrVoid  ToFade;

	bool            Done = FALSE;
	double          FadeOutStart = 1.0f;
	gint64          FadeOutTime;

	Lock();
	int Index;
	int Count = m_FaderPlayBins.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        FaderPlayBin = m_FaderPlayBins[ Index ];

		switch( FaderPlayBin->m_State )
		{
            case guFADERPLAYBIN_STATE_WAITING:
            case guFADERPLAYBIN_STATE_WAITING_EOS:
                //guLogDebug( wxT( "stream %s is not yet playing, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;

            case guFADERPLAYBIN_STATE_PREROLLING:
            case guFADERPLAYBIN_STATE_PREROLL_PLAY:
                //guLogDebug( wxT( "stream %s is prerolling, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;

            case guFADERPLAYBIN_STATE_REUSING:
                //guLogDebug( wxT( "stream %s is being reused, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;

            case guFADERPLAYBIN_STATE_PAUSED :
            case guFADERPLAYBIN_STATE_SEEKING_PAUSED :
            case guFADERPLAYBIN_STATE_FADING_OUT_PAUSED :
                //guLogDebug( wxT( "stream %s is already paused" ), FaderPlayBin->m_Uri.c_str() );
                Done = true;
                break;

            case guFADERPLAYBIN_STATE_FADING_IN:
            case guFADERPLAYBIN_STATE_PLAYING:
                //guLogDebug( wxT( "pausing stream %s -> FADING_OUT_PAUSED" ), FaderPlayBin->m_Uri.c_str() );
                ToFade.Insert( FaderPlayBin, 0 );
                Done = true;
                break;

            case guFADERPLAYBIN_STATE_SEEKING:
                //guLogDebug( wxT( "pausing seeking stream %s -> SEEKING_PAUSED" ), FaderPlayBin->m_Uri.c_str() );
                FaderPlayBin->m_State = guFADERPLAYBIN_STATE_SEEKING_PAUSED;
                Done = true;
                break;

            case guFADERPLAYBIN_STATE_SEEKING_EOS:
                //guLogDebug( wxT( "stream %s is seeking after EOS -> SEEKING_PAUSED" ), FaderPlayBin->m_Uri.c_str() );
                FaderPlayBin->m_State = guFADERPLAYBIN_STATE_SEEKING_PAUSED;
                Done = true;
                break;

            case guFADERPLAYBIN_STATE_FADING_OUT:
                //guLogDebug( wxT( "stream %s is fading out, can't be bothered pausing it" ), FaderPlayBin->m_Uri.c_str() );
                break;

            case guFADERPLAYBIN_STATE_PENDING_REMOVE:
                //guLogDebug( wxT( "stream %s is done, can't pause" ), FaderPlayBin->m_Uri.c_str() );
                break;
		}

		if( Done )
			break;
	}

    Unlock();

	FadeOutTime = FaderPlayBin->m_FadeOutTime ? guFADERPLAYBIN_FAST_FADER_TIME : 250;
	Count = ToFade.Count();
	for( Index = 0; Index < Count; Index++ )
	{
		switch( FaderPlayBin->m_State )
		{
            case guFADERPLAYBIN_STATE_FADING_IN :
                g_object_get( FaderPlayBin->m_Volume, "volume", &FadeOutStart, NULL );
                FadeOutTime = ( gint64 ) ( ( ( double ) guFADERPLAYBIN_FAST_FADER_TIME ) * FadeOutStart );
                //guLogDebug( wxT( "============== Fading Out a Fading In playbin =================" ) );

            case guFADERPLAYBIN_STATE_PLAYING:
            {
                FaderPlayBin->m_State = guFADERPLAYBIN_STATE_FADING_OUT_PAUSED;
                FaderPlayBin->StartFade( FadeOutStart, 0.0f, FadeOutTime );
            }

            default:
                // shouldn't happen, but ignore it if it does
                break;
		}
	}

	if( !Done )
		guLogMessage( wxT( "couldn't find a stream to pause" ) );

    return Done;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Stop( void )
{
    //guLogDebug( wxT( "MediaCtrl::Stop" ) );
    Pause();
    Seek( 0 );
    SetCurrentState( GST_STATE_READY );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::Seek( wxFileOffset where )
{
    //guLogDebug( wxT( "MediaCtrl::Seek( %lli )" ), where );

	guFaderPlayBin * FaderPlayBin;

	Lock();
	FaderPlayBin = FindFaderPlayBin( m_FaderPlayBins,
                        guFADERPLAYBIN_STATE_FADING_IN |
                        guFADERPLAYBIN_STATE_PLAYING |
                        guFADERPLAYBIN_STATE_PAUSED |
                        guFADERPLAYBIN_STATE_FADING_OUT_PAUSED |
                        guFADERPLAYBIN_STATE_PENDING_REMOVE );
	Unlock();

	if( !FaderPlayBin )
	{
		guLogMessage( wxT( "got seek while no playing streams exist" ) );
		return false;
	}

	FaderPlayBin->m_SeekTarget = where * GST_MSECOND;

	switch( FaderPlayBin->m_State )
	{
        case guFADERPLAYBIN_STATE_PAUSED:
            //guLogDebug( wxT( "seeking in paused stream %s; target %"G_GINT64_FORMAT ), FaderPlayBin->m_Uri.c_str(), FaderPlayBin->m_SeekTarget );
            perform_seek( FaderPlayBin );
            break;

        case guFADERPLAYBIN_STATE_FADING_OUT_PAUSED:
            // don't unblock and relink when the seek is done
            FaderPlayBin->m_State = guFADERPLAYBIN_STATE_SEEKING_PAUSED;
            //guLogDebug( wxT( "seeking in pausing stream %s; target %"G_GINT64_FORMAT ), FaderPlayBin->m_Uri.c_str(), FaderPlayBin->m_SeekTarget );
            FaderPlayBin->UnlinkAndBlock();
            break;

        case guFADERPLAYBIN_STATE_FADING_IN:
        case guFADERPLAYBIN_STATE_PLAYING:
            FaderPlayBin->m_State = guFADERPLAYBIN_STATE_SEEKING;
            //guLogDebug( wxT( "seeking in playing stream %s; target %"G_GINT64_FORMAT ), FaderPlayBin->m_Uri.c_str(), FaderPlayBin->m_SeekTarget );
            perform_seek( FaderPlayBin );
            break;

        case guFADERPLAYBIN_STATE_PENDING_REMOVE:
            // this should only happen when the stream has ended,
            // which means we can't wait for the src pad to be blocked
            // before we seek.  we unlink the stream when it reaches EOS,
            // so now we just perform the seek and relink.
            //guLogDebug( wxT( "seeking in EOS stream %s; target %"G_GINT64_FORMAT ), FaderPlayBin->m_Uri.c_str(), FaderPlayBin->m_SeekTarget );
            FaderPlayBin->m_State = guFADERPLAYBIN_STATE_SEEKING_EOS;

            gst_pad_set_blocked_async( FaderPlayBin->m_SourcePad, true,
                           GstPadBlockCallback( post_eos_seek_blocked_cb ),
                           FaderPlayBin );
            perform_seek( FaderPlayBin );
            break;

        default:
            //g_assert_not_reached ();
            break;
	}
	return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaCtrl::CanReuse( const wxString &uri, guFaderPlayBin * faderplaybin )
{
    return false;
}

void guMediaCtrl::UpdatedConfig( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_FadeOutTime       = Config->ReadNum( wxT( "FadeOutTime" ), 5, wxT( "Crossfader" ) ) * GST_SECOND;
    m_FadeInTime        = Config->ReadNum( wxT( "FadeInTime" ), 1, wxT( "Crossfader" ) ) * GST_SECOND;
    m_FadeInVolStart    = double( Config->ReadNum( wxT( "FadeInVolStart" ), 8, wxT( "Crossfader" ) ) ) / 10.0;
    m_FadeInVolTriger   = double( Config->ReadNum( wxT( "FadeInVolTriger" ), 5, wxT( "Crossfader" ) ) ) / 10.0;
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
}

// -------------------------------------------------------------------------------- //
bool guMediaRecordCtrl::Start( const guTrack * track )
{
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
guFaderPlayBin::guFaderPlayBin( guMediaCtrl * mediactrl, const wxString &uri )
{
	//guLogDebug( wxT( "creating new stream for %s" ), uri.c_str() );

    m_Player = mediactrl;
    m_Parent = mediactrl->m_Pipeline;
    m_State = guFADERPLAYBIN_STATE_WAITING;

    m_DecoderLinked = false;
    m_EmittedPlaying = false;
    m_EmittedFakePlaying = false;
    m_EmittedStartFadeIn = false;

    m_DecoderPad = NULL;
    m_SourcePad = NULL;
    m_GhostPad = NULL;
    m_AdderPad = NULL;

    m_SoureBlocked = false;
    m_NeedsUnlink = false;

    m_SeekTarget = 0;

    m_FadeOutTime = m_Player->m_FadeOutTime;
    m_PlayType = m_FadeOutTime ? guFADERPLAYBIN_PLAYTYPE_CROSSFADE : guFADERPLAYBIN_PLAYTYPE_AFTER_EOS;
    m_Fading = false;

    m_AdjustProbeId = 0;

    m_FadeEnd = 0.0;

    m_EmittedError = false;
    m_ErrorIdleId = false;
    m_Error = NULL;

    //m_Tags = NULL;

    m_Uri = uri;
    m_PlayBin = gst_bin_new( NULL );

    m_Decoder = gst_element_factory_make( "uridecodebin", NULL );
    gst_object_ref( m_Decoder );
    g_object_set( m_Decoder, "uri", ( const char * ) uri.mb_str( wxConvFile ), NULL );

    g_signal_connect( m_Decoder, "notify::source", G_CALLBACK( faderplaybin_notify_source_cb ), this );
    g_signal_connect( m_Decoder, "pad-added", G_CALLBACK( faderplaybin_pad_added_cb ), this );
    g_signal_connect( m_Decoder, "pad-removed", G_CALLBACK( faderplaybin_pad_removed_cb ), this );

    m_Identity = gst_element_factory_make( "identity", NULL );

    m_AudioConvert = gst_element_factory_make( "audioconvert", NULL );
    gst_object_ref( m_AudioConvert );

    m_AudioResample = gst_element_factory_make( "audioresample", NULL );
    gst_object_ref( m_AudioResample );

    m_CapsFilter = gst_element_factory_make( "capsfilter", NULL );
    gst_object_ref( m_CapsFilter );

    GstCaps *       Caps;
	Caps = gst_caps_new_simple( "audio/x-raw-int",
				    "channels", G_TYPE_INT, 2,
				    "rate",	G_TYPE_INT, 44100,
				    "width", G_TYPE_INT, 16,
				    "depth", G_TYPE_INT, 16,
				    NULL );
	g_object_set( m_CapsFilter, "caps", Caps, NULL );
	gst_caps_unref( Caps );

	m_Volume = gst_element_factory_make( "volume", NULL );
	gst_object_ref( m_Volume );

	g_signal_connect( m_Volume, "notify::volume", G_CALLBACK( faderplaybin_volume_changed_cb ), this );


	m_Fader = gst_object_control_properties( G_OBJECT( m_Volume ), ( gchar * ) "volume", NULL );
	gst_object_ref( m_Fader );
	gst_controller_set_interpolation_mode( m_Fader, ( gchar * ) "volume", GST_INTERPOLATE_LINEAR );

    m_PreRoll = gst_element_factory_make( "queue", NULL );
	g_object_set( m_PreRoll,
        "min-threshold-time", GST_SECOND,
        "max-size-buffers", 1000,
        NULL );

	gst_bin_add_many( GST_BIN( m_PlayBin ),
        m_Decoder,
        m_Identity,
        m_AudioConvert,
        m_AudioResample,
        m_CapsFilter,
        m_PreRoll,
        m_Volume,
        NULL );

	gst_element_link_many( m_Identity, m_AudioConvert,
        m_AudioResample,
        m_CapsFilter,
		m_PreRoll,
		m_Volume,
		NULL );

	m_SourcePad = gst_element_get_static_pad( m_Volume, "src" );

	// ghost the stream src pad up to the bin
	m_GhostPad = gst_ghost_pad_new( "src", m_SourcePad );
	gst_element_add_pad( GST_ELEMENT( m_PlayBin ), m_GhostPad );

	// watch for EOS events using a pad probe
	gst_pad_add_event_probe( m_SourcePad, GCallback( faderplaybin_src_event_cb ), this );

	// use the pipeline bus even when not inside the pipeline (?)
	gst_element_set_bus( GST_ELEMENT( m_PlayBin ), gst_element_get_bus( m_Parent ) );

}

// -------------------------------------------------------------------------------- //
guFaderPlayBin::~guFaderPlayBin()
{
    bool WasLinked = false;
    bool WasInPipeline = false;

    //guLogDebug( wxT( "Deleting stream %s" ), m_Uri.c_str() );

	if( gst_element_set_state( GST_ELEMENT( m_PlayBin ), GST_STATE_NULL ) == GST_STATE_CHANGE_ASYNC )
	{
	    //guLogDebug( wxT( "!!! stream %s isn't cooperating" ), m_Uri.c_str() );
		gst_element_get_state( GST_ELEMENT( m_PlayBin ), NULL, NULL, GST_CLOCK_TIME_NONE );
	}

	Lock();

    if( m_AdderPad )
    {
        //guLogDebug( wxT( "Unlinking stream %s" ), m_Uri.c_str() );
		if( gst_pad_unlink( m_GhostPad, m_AdderPad ) == FALSE )
		{
			guLogWarning( wxT( "Couldn't unlink stream %s: things will probably go quite badly from here on" ), m_Uri.c_str() );
		}

		gst_element_release_request_pad( GST_PAD_PARENT( m_AdderPad ), m_AdderPad );
		m_AdderPad = NULL;

        WasLinked = true;
    }

    WasInPipeline = ( GST_ELEMENT_PARENT( GST_ELEMENT( m_PlayBin ) ) == m_Player->m_Pipeline );

    Unlock();

    if( WasInPipeline )
        gst_bin_remove( GST_BIN( m_Player->m_Pipeline ), GST_ELEMENT( m_PlayBin ) );

    if( WasLinked )
    {
        m_Player->m_LinkedStreams--;
		//guLogDebug( wxT( "now have %d linked streams" ), m_Player->m_LinkedStreams );
        if( !m_Player->m_LinkedStreams )
            m_Player->MaybeStopSink();
    }

    m_Player->m_FaderPlayBinsMutex.Lock();
    m_Player->m_FaderPlayBins.Remove( this );
    m_Player->m_FaderPlayBinsMutex.Unlock();

	if( m_Decoder )
	{
		gst_object_unref( m_Decoder );
	}

	if( m_Volume )
	{
		gst_object_unref( m_Volume );
	}

	if( m_Fader )
	{
		gst_object_unref( m_Fader );
	}

	if( m_AudioConvert )
	{
		gst_object_unref( m_AudioConvert );
	}

	if( m_AudioResample )
	{
		gst_object_unref( m_AudioResample );
	}

	if( m_CapsFilter )
    {
		gst_object_unref( m_CapsFilter );
    }

#ifdef guSHOW_DUMPFADERPLAYBINS
    m_Player->Lock();
    DumpFaderPlayBins( m_Player->m_FaderPlayBins );
    m_Player->Unlock();
#endif
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::UnlinkAndBlock( void )
{
	if( !m_AdderPad )
	{
		//guLogDebug( wxT( "stream %s is not linked" ), m_Uri.c_str() );
		return;
	}

	m_NeedsUnlink = true;

	if( m_SoureBlocked )
	{
		// probably shouldn't happen, but we'll handle it anyway
		unlink_blocked_cb( m_SourcePad, true, this );
	}
	else
	{
		gst_pad_set_blocked_async( m_SourcePad, true, GstPadBlockCallback( unlink_blocked_cb ), this );
	}
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::StartFade( double start, double end, gint64 time )
{
	GValue V = { 0, };
	gint64 Pos = -1;
	GstFormat Format = GST_FORMAT_TIME;

	// hmm, can we take the stream lock safely here?  probably should..
	gst_element_query_position( m_Volume, &Format, &Pos );
	if( Pos < 0 )
	{
		// probably means we haven't actually started the stream yet.
		// we also get (weird) negative results with some decoders
		// (mad but not flump3dec, for instance) immediately after prerolling.
		// the controller doesn't seem to work if we give it a 0 timestamp
		// here, but something unnoticeably later does work.
		Pos = 100000;
	}

	if( Format != GST_FORMAT_TIME )
	{
		//guLogDebug( wxT( "got position query results in some other format: %s" ), gst_format_get_name( Format ) );
		Pos = 0;
	}

	guLogDebug( wxT( "fading stream %s: [%f, %" G_GINT64_FORMAT "] to [%f, %" G_GINT64_FORMAT "]" ), m_Uri.c_str(), ( float ) start, Pos, ( float ) end, Pos + time );

	g_signal_handlers_block_by_func( m_Volume, ( void * ) faderplaybin_volume_changed_cb, this );

	// apparently we need to set the starting volume, otherwise fading in doesn't work.
	m_FadeEnd = end;
	g_object_set( m_Volume, "volume", start, NULL );

	gst_controller_unset_all( m_Fader, ( gchar * ) "volume" );

	g_value_init( &V, G_TYPE_DOUBLE );
	g_value_set_double( &V, start );
	if( !gst_controller_set( m_Fader, ( gchar * ) "volume", Pos, &V ) )
	{
		guLogMessage( wxT( "controller didn't like our start point" ) );
	}

	if( !gst_controller_set( m_Fader, ( gchar * ) "volume", 0, &V ) )
	{
		guLogMessage( wxT( "controller didn't like our 0 start point" ) );
	}
	g_value_unset( &V );

	g_value_init( &V, G_TYPE_DOUBLE );
	g_value_set_double( &V, end );
	if( !gst_controller_set( m_Fader, ( gchar * ) "volume", Pos + time, &V ) )
	{
		guLogMessage( wxT( "controller didn't like our end point" ) );
	}
	g_value_unset( &V );

	g_signal_handlers_unblock_by_func( m_Volume, ( void * ) faderplaybin_volume_changed_cb, this );

	m_Fading = true;

	// tiny hack:  if the controlled element is in passthrough mode, the
	// controller won't get updated.
//	gst_base_transform_set_passthrough( GST_BASE_TRANSFORM( m_Volume ), false );
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::ActuallyStart( GError ** error )
{
    bool        Ret = true;
    bool        NeedReap = false;
    GError *    Error;

    bool        Playing = false;

    guFaderPlayBinArray ToFade;

	guLogDebug( wxT( "going to start playback for stream %s (play type %d, crossfade %" G_GINT64_FORMAT ") -> FADING_IN | PLAYING" ), m_Uri.c_str(), m_PlayType, m_FadeOutTime );

    switch( m_PlayType )
    {
        case guFADERPLAYBIN_PLAYTYPE_CROSSFADE :
        {
            //guLogDebug( wxT( "Going to do a clean up of previous streams..." ) );
            int Index;
            m_Player->Lock();
#ifdef guSHOW_DUMPFADERPLAYBINS
            DumpFaderPlayBins( m_Player->m_FaderPlayBins );
#endif
            int Count = m_Player->m_FaderPlayBins.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                guFaderPlayBin * FaderPlayBin = m_Player->m_FaderPlayBins[ Index ];
                if( FaderPlayBin == this )
                    continue;

                switch( FaderPlayBin->m_State )
                {
                    case guFADERPLAYBIN_STATE_FADING_IN :
                    case guFADERPLAYBIN_STATE_PLAYING :
                        //guLogDebug( wxT( "stream %s is playing; crossfading -> FADING_OUT" ), FaderPlayBin->m_Uri.c_str() );
                        ToFade.Add( FaderPlayBin );
                        break;

                    case guFADERPLAYBIN_STATE_PAUSED :
                    case guFADERPLAYBIN_STATE_WAITING :
                    case guFADERPLAYBIN_STATE_WAITING_EOS :
                    case guFADERPLAYBIN_STATE_SEEKING :
                    case guFADERPLAYBIN_STATE_SEEKING_PAUSED :
                    case guFADERPLAYBIN_STATE_PREROLLING :
                    case guFADERPLAYBIN_STATE_PREROLL_PLAY :
                        //guLogDebug( wxT( "stream %s is paused; replacing it" ), FaderPlayBin->m_Uri.c_str() );
                        FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;

                    case guFADERPLAYBIN_STATE_PENDING_REMOVE :
                        NeedReap = true;
                        break;

                    default :
                        break;

                }
            }
            m_Player->Unlock();

            Count = ToFade.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                double FadeOutStart = 1.0;
                gint64 FadeOutTime = m_FadeOutTime;

                guFaderPlayBin * FaderPlayBin = ToFade[ Index ];

                switch( FaderPlayBin->m_State )
                {
                    case guFADERPLAYBIN_STATE_FADING_IN :
                        g_object_get( FaderPlayBin->m_Volume, "volume", &FadeOutStart, NULL );
                        FadeOutTime = ( gint64 ) ( ( ( double ) FaderPlayBin->m_FadeOutTime ) * FadeOutStart );

                    case guFADERPLAYBIN_STATE_PLAYING :
                        //guLogDebug( wxT( "))))))))(((((((( Fading Out stream %s" ), FaderPlayBin->m_Uri.c_str() );
                        FaderPlayBin->StartFade( FadeOutStart, 0.0, FadeOutTime );
                        FaderPlayBin->m_State = guFADERPLAYBIN_STATE_FADING_OUT;
                        m_Fading = true;
                        //StartFade( 0.0, 1.0, m_FadeOutTime );
                        break;

                    default :
                        break;
                }
            }

            if( !m_Fading )
            {
                GValue V = { 0, };

                //guLogDebug( wxT( "stream isn't fading; setting volume to 1.0" ) );
                g_value_init( &V, G_TYPE_DOUBLE );
                g_value_set_double( &V, 1.0 );
                if( !gst_controller_set( m_Fader, ( gchar * ) "volume", 0, &V ) )
                {
                    guLogMessage( wxT( "controller didn't like our start point") );
                }
                g_value_unset( &V );
                Ret = LinkAndUnblock( &Error );
            }
            else
            {
                Ret = true; //LinkAndUnblock( &Error );
            }
            break;
        }

        case guFADERPLAYBIN_PLAYTYPE_AFTER_EOS :
        {
            Playing = false;
            int Index;
            m_Player->m_FaderPlayBinsMutex.Lock();
            int Count = m_Player->m_FaderPlayBins.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                guFaderPlayBin * FaderPlayBin = m_Player->m_FaderPlayBins[ Index ];
                if( FaderPlayBin == this )
                    continue;
                switch( FaderPlayBin->m_State )
                {
                    case guFADERPLAYBIN_STATE_PLAYING :
                    case guFADERPLAYBIN_STATE_FADING_IN :
                    case guFADERPLAYBIN_STATE_FADING_OUT :
                        //guLogDebug( wxT( "Stream %s already playing" ), FaderPlayBin->m_Uri.c_str() );
                        Playing = true;
                        break;

                    case guFADERPLAYBIN_STATE_PAUSED :
                        //guLogDebug( wxT( "stream %s is paused; replacing it" ), FaderPlayBin->m_Uri.c_str() );
                        FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;

                    case guFADERPLAYBIN_STATE_PENDING_REMOVE :
                        NeedReap = true;
                        break;

                    default:
                        break;
                }
            }

            m_Player->m_FaderPlayBinsMutex.Unlock();

            if( Playing )
            {
                // wait for current stream's EOS
                //guLogDebug( wxT( "existing playing stream found; waiting for its EOS -> WAITING_EOS" ) );
                m_State = guFADERPLAYBIN_STATE_WAITING_EOS;
            }
            else
            {
                //guLogDebug( wxT( "no playing stream found, so starting immediately" ) );
                Ret = LinkAndUnblock( &Error );
            }

            break;
        }

        case guFADERPLAYBIN_PLAYTYPE_REPLACE :
        {
            int Index;
            m_Player->m_FaderPlayBinsMutex.Lock();
            int Count = m_Player->m_FaderPlayBins.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                guFaderPlayBin * FaderPlayBin = m_Player->m_FaderPlayBins[ Index ];
                if( FaderPlayBin == this )
                    continue;
                switch( FaderPlayBin->m_State )
                {
                    case guFADERPLAYBIN_STATE_PLAYING :
                    case guFADERPLAYBIN_STATE_PAUSED :
                    case guFADERPLAYBIN_STATE_FADING_IN :
                    case guFADERPLAYBIN_STATE_PENDING_REMOVE :
                        // kill this one
                        //guLogDebug( wxT( "stopping stream %s (replaced by new stream)" ), FaderPlayBin->m_Uri.c_str() );
                        NeedReap = true;
                        FaderPlayBin->m_State = guFADERPLAYBIN_STATE_PENDING_REMOVE;
                        break;

                    default:
                        break;
                }
            }

            m_Player->m_FaderPlayBinsMutex.Unlock();

            //guLogDebug( wxT( "no playing stream found, so starting immediately" ) );
            Ret = LinkAndUnblock( &Error );

            break;
        }
    }

    if( NeedReap )
    {
        m_Player->ScheduleReap();
    }

    return Ret;
}

// -------------------------------------------------------------------------------- //
bool guFaderPlayBin::LinkAndUnblock( GError ** error )
{
	GstPadLinkReturn PadLinkReturn;
	GstStateChangeReturn StateChangeReturn;

	//guLogDebug( wxT( "guFaderPlayBin::LinkAndUnblock" ) );

	if( !m_Player->StartSink( error ) )
	{
		guLogMessage( wxT( "sink didn't start, so we're not going to link the stream" ) );
		return false;
	}

	if( m_AdderPad )
	{
	    //guLogDebug( wxT( "stream %s already linked." ), m_Uri.c_str() );
	    return true;
	}

	m_NeedsUnlink = false;

	//guLogDebug( wxT( "Linking stream %s" ), m_Uri.c_str() );
	if( !GST_ELEMENT_PARENT( GST_ELEMENT( m_PlayBin ) ) )
		gst_bin_add( GST_BIN( m_Player->m_Pipeline ), GST_ELEMENT( m_PlayBin ) );

	m_AdderPad = gst_element_get_request_pad( m_Player->m_Adder, "sink%d" );
	if( !m_AdderPad )
	{
		// this error message kind of sucks
		guLogMessage( wxT( "couldn't get adder pad to link in new stream" ) );
		return false;
	}

	PadLinkReturn = gst_pad_link( m_GhostPad, m_AdderPad );
	if( GST_PAD_LINK_FAILED( PadLinkReturn ) )
	{
		gst_element_release_request_pad( m_Player->m_Adder, m_AdderPad );
		m_AdderPad = NULL;

		// this error message kind of sucks
		guLogMessage( wxT( "linking stream pad to adder pad failed: %d" ), PadLinkReturn );
		return false;
	}

    m_Player->m_LinkedStreams++;
	//guLogDebug( wxT( "now have %d linked streams" ), m_Player->m_LinkedStreams );

    if( m_SoureBlocked )
    {
        //guLogDebug( wxT( "Unblock now the source pad..." ) );
		gst_pad_set_blocked_async( m_SourcePad, false, GstPadBlockCallback( link_unblocked_cb ), this );
    }
    else
    {
		//guLogDebug( wxT( "??? stream %s is already unblocked -> PLAYING" ), m_Uri.c_str() );
		m_State = guFADERPLAYBIN_STATE_PLAYING;

		AdjustBaseTime();

		StateChangeReturn = gst_element_set_state( GST_ELEMENT( m_PlayBin ), GST_STATE_PLAYING );

		PostPlayMessage( false );

		if( StateChangeReturn == GST_STATE_CHANGE_FAILURE )
		{
			return false;
		}
    }

    return true;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::AdjustBaseTime( void )
{
	GstFormat Format;
	gint64 OutputPos = -1;
	gint64 StreamPos = -1;

	Lock();

	if( !m_AdderPad )
	{
		guLogMessage( wxT( "stream isn't linked, can't adjust base time" ) );
		Unlock();
		return;
	}

	Format = GST_FORMAT_TIME;
	gst_element_query_position( GST_PAD_PARENT( m_AdderPad ), &Format, &OutputPos );
	if( OutputPos != -1 )
	{
		m_BaseTime = OutputPos;
	}

	// offset the base position to account for the current stream position
	Format = GST_FORMAT_TIME;
	gst_element_query_position( m_Volume, &Format, &StreamPos );
	if( StreamPos != -1 )
	{
		//guLogDebug( wxT( "adjusting base time: %" G_GINT64_FORMAT" - %" G_GINT64_FORMAT " => %" G_GINT64_FORMAT ), m_BaseTime, StreamPos, m_BaseTime - StreamPos );
		m_BaseTime -= StreamPos;

		// once we've successfully adjusted the base time, we don't need the data probe
		if( m_AdjustProbeId )
		{
			gst_pad_remove_buffer_probe( m_GhostPad, m_AdjustProbeId );
			m_AdjustProbeId = 0;
		}
	}
	else
	{
		guLogMessage( wxT( "unable to adjust base time as position query failed" ) );

		// add a pad probe to attempt to adjust when the next buffer goes out
		if( !m_AdjustProbeId )
		{
			m_AdjustProbeId = gst_pad_add_buffer_probe( m_GhostPad, G_CALLBACK( adjust_base_time_probe_cb ), this );
		}
	}

	Unlock();
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::PostPlayMessage( bool fake )
{
	GstMessage * Msg;
	GstStructure * S;

	if( m_EmittedPlaying )
	{
		return;
	}

	//guLogDebug( wxT( "posting " guFADERPLAYBIN_MESSAGE_PLAYING " message for stream %s" ), m_Uri.c_str() );
	S = gst_structure_new( guFADERPLAYBIN_MESSAGE_PLAYING, NULL );
	Msg = gst_message_new_application( GST_OBJECT( m_PlayBin ), S );
    gst_element_post_message( GST_ELEMENT( m_PlayBin ), Msg );

	if( !fake )
	{
		m_EmittedPlaying = true;
	}
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::EmitError( GError * error )
{
	if( m_ErrorIdleId != 0 )
	{
		g_error_free( error );
	}
	else
	{
		m_Error = error;
		m_ErrorIdleId = g_idle_add( GSourceFunc( emit_stream_error_cb ), this );
	}
}

// -------------------------------------------------------------------------------- //
// starts prerolling for a stream.
// since the stream isn't linked to anything yet, we
// block the src pad.  when the pad block callback
// is called, prerolling is complete and the stream
// can be linked and played immediately if required.
//
// must be called *without* the stream list lock?
bool guFaderPlayBin::Preroll( void )
{
    //guLogDebug( wxT( "FaderPlayBin::Preroll" ) );

	GstStateChangeReturn    State;
	bool                    Ret = true;
//	bool                    Unblock = false;

    //GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS( GST_BIN( m_PlayBin ), GST_DEBUG_GRAPH_SHOW_ALL , "faderplaybin" );

	gst_pad_set_blocked_async( m_SourcePad, true, GstPadBlockCallback( stream_src_blocked_cb ), this );

	m_EmittedPlaying = false;
	m_State = guFADERPLAYBIN_STATE_PREROLLING;
	State = gst_element_set_state( GST_ELEMENT( m_PlayBin ), GST_STATE_PAUSED );
	switch( State )
	{
        case GST_STATE_CHANGE_FAILURE:
            //guLogDebug( wxT( "preroll for stream %s failed (state change failed)" ), m_Uri.c_str() );
            Ret = false;
            // attempting to unblock here causes deadlock
            break;

        case GST_STATE_CHANGE_NO_PREROLL:
            //guLogDebug( wxT( "no preroll for stream %s, setting to PLAYING instead?" ), m_Uri.c_str() );
            gst_element_set_state( GST_ELEMENT( m_PlayBin ), GST_STATE_PLAYING );
            break;

        case GST_STATE_CHANGE_SUCCESS:
        case GST_STATE_CHANGE_ASYNC:
            // uridecodebin returns SUCCESS from state changes when streaming, so we can't
            // use that to figure out what to do next.  instead, we wait for pads to be added
            // and for our pad block callbacks to be called.
            //guLogDebug( wxT( "preroll was sucessfully?..." ) );
            break;

        default:
            //g_assert_not_reached();
            break;
	}

//	if( Unblock )
//	{
//		//guLogDebug( wxT( "unblocking stream source pad" ) );
//		gst_pad_set_blocked_async( m_SourcePad, false, NULL, NULL );
//	}

	return Ret;
}

// -------------------------------------------------------------------------------- //
void guFaderPlayBin::Reuse( void )
{
//	g_signal_emit (stream->player,
//		       signals[REUSE_STREAM], 0,
//		       stream->new_uri, stream->uri, GST_ELEMENT (stream));

	// replace URI and stream data
	m_Uri = m_NewUri;

	m_NewUri = wxEmptyString;

	m_EmittedPlaying = false;
}

// -------------------------------------------------------------------------------- //
