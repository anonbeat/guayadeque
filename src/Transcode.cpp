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
#include "Transcode.h"

#include "Commands.h"
#include "TagInfo.h"
#include "Utils.h"
#include "Version.h"

extern "C" {

// -------------------------------------------------------------------------------- //
static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guTranscodeThread * pobj )
{
    //guLogMessage( wxT( "Got gstreamer message %u" ), GST_MESSAGE_TYPE( message ) );
    switch( GST_MESSAGE_TYPE( message ) )
    {
        case GST_MESSAGE_ERROR :
        {
            GError * err;
            gchar * debug;
            gst_message_parse_error( message, &err, &debug );
            guLogError( wxT( "Transcode gstreamer error '%s'\n'%s'" ),
                wxString( err->message, wxConvUTF8 ).c_str(),
                wxString( debug, wxConvUTF8 ).c_str() );
            g_error_free( err );
            g_free( debug );
            pobj->SetError( true );
            pobj->Stop();
            break;
        }

        case GST_MESSAGE_EOS :
        {
          //guLogMessage( wxT( "EOS Detected..." ) );
          pobj->Stop();
          break;
        }

        default:
            break;
    }

    return TRUE;
}

// -------------------------------------------------------------------------------- //
static void on_pad_added( GstElement * decodebin, GstPad * pad, gboolean last, GstElement * conv )
{
  GstCaps * caps;
  GstStructure * str;
  GstPad * convpad;

  //guLogMessage( wxT( "New pad created..." ) );

  convpad = gst_element_get_static_pad( conv, "sink" );
  if( GST_PAD_IS_LINKED( convpad ) )
  {
      g_object_unref( convpad );
      return;
  }

  /* check media type */
  caps = gst_pad_get_caps( pad );
  str = gst_caps_get_structure( caps, 0 );
  if( !g_strrstr( gst_structure_get_name( str ), "audio" ) )
  {
    gst_caps_unref( caps );
    gst_object_unref( convpad );
    return;
  }
  gst_caps_unref( caps );

  //guLogMessage( wxT( "Linked decoder and converter..." ) );
  /* link'n'play */
  gst_pad_link( pad, convpad );
  g_object_unref( convpad );
}


}

// -------------------------------------------------------------------------------- //
// guTranscodeThread
// -------------------------------------------------------------------------------- //
guTranscodeThread::guTranscodeThread( const wxChar * source, const wxChar * target,
        const int format, const int quality )
{
    m_Source = wxString( source );
    m_Target = wxString( target );
    m_Format = format;
    m_Quality = quality;

    m_Running = false;
    m_HasError = false;

    BuildPipeline();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guTranscodeThread::~guTranscodeThread()
{
    if( GST_IS_ELEMENT( m_Pipeline ) )
    {
        gst_element_set_state( m_Pipeline, GST_STATE_NULL );
        gst_object_unref( GST_OBJECT( m_Pipeline ) );
    }
}

int guTranscodeMp3Bitrates[] = {
    128,
    320,
    256,
    192,
    160,
    128,
    96,
    64
};

float guTranscodeOggQuality[] = {
    0.6,
    1,
    0.9,
    0.8,
    0.7,
    0.6,
    0.5,
    0.2
};

int guTranscodeFlacQuality[] = {
    4,
    8,
    7,
    6,
    5,
    4,
    3,
    1
};

unsigned long guTranscodeWmaBitrates[] = {
    128000,
    320000,
    256000,
    192000,
    160000,
    128000,
    96000,
    64000
};

// -------------------------------------------------------------------------------- //
wxArrayString TranscodeFormatStrings;
wxArrayString TranscodeQualityStrings;

// -------------------------------------------------------------------------------- //
wxArrayString guTranscodeFormatStrings( void )
{
    if( !TranscodeFormatStrings.Count() )
    {
        TranscodeFormatStrings.Add( _( "Keep Format" ) );
        TranscodeFormatStrings.Add( wxT( "mp3" ) );
        TranscodeFormatStrings.Add( wxT( "ogg" ) );
        TranscodeFormatStrings.Add( wxT( "flac" ) );
        TranscodeFormatStrings.Add( wxT( "m4a" ) );
        TranscodeFormatStrings.Add( wxT( "wma" ) );
    }
    return TranscodeFormatStrings;
}

// -------------------------------------------------------------------------------- //
wxString guTranscodeFormatString( const int format )
{
    if( !TranscodeFormatStrings.Count() )
        guTranscodeFormatStrings();
    return TranscodeFormatStrings[ format ];
}

// -------------------------------------------------------------------------------- //
wxArrayString guTranscodeQualityStrings( void )
{
    if( !TranscodeQualityStrings.Count() )
    {
        TranscodeQualityStrings.Add( _( "Keep Quality" ) );
        TranscodeQualityStrings.Add( _( "Very High" ) );
        TranscodeQualityStrings.Add( _( "High" ) );
        TranscodeQualityStrings.Add( _( "Very Good" ) );
        TranscodeQualityStrings.Add( _( "Good" ) );
        TranscodeQualityStrings.Add( _( "Normal" ) );
        TranscodeQualityStrings.Add( _( "Low" ) );
        TranscodeQualityStrings.Add( _( "Very Low" ) );
    }
    return TranscodeQualityStrings;
}

// -------------------------------------------------------------------------------- //
wxString guTranscodeQualityString( const int quality )
{
    if( !TranscodeQualityStrings.Count() )
        guTranscodeQualityStrings();
    return TranscodeQualityStrings[ quality ];
}

// -------------------------------------------------------------------------------- //
bool guTranscodeThread::BuildEncoder( GstElement ** enc, GstElement ** mux )
{
    switch( m_Format )
    {
        case guTRANSCODE_FORMAT_MP3 :
        {
            guLogMessage( wxT( "Creating mp3 encoder with %i bitrate" ), guTranscodeMp3Bitrates[ m_Quality ] );
            * enc = gst_element_factory_make( "lame", "guTransLame" );
            if( GST_IS_ELEMENT( * enc ) )
            {
                g_object_set( * enc, "bitrate", guTranscodeMp3Bitrates[ m_Quality ], NULL );
                return true;
            }
            break;
        }

        case guTRANSCODE_FORMAT_OGG :
        {
            * enc = gst_element_factory_make( "vorbisenc", "guTransVorbis" );
            if( GST_IS_ELEMENT( * enc ) )
            {
                g_object_set( * enc, "quality", guTranscodeOggQuality[ m_Quality ], NULL );

                * mux = gst_element_factory_make( "oggmux", "guTransOggMux" );
                if( GST_IS_ELEMENT( * mux ) )
                {
                    return true;
                }

                g_object_unref( * enc );
                * enc = NULL;
            }
            break;
        }

        case guTRANSCODE_FORMAT_FLAC :
        {
            * enc = gst_element_factory_make( "flacenc", "guTransFlac" );
            if( GST_IS_ELEMENT( * enc ) )
            {
                g_object_set( * enc, "quality", guTranscodeFlacQuality[ m_Quality ], NULL );
                return true;
            }
            break;
        }

        case guTRANSCODE_FORMAT_AAC :
        {
            * enc = gst_element_factory_make( "ffenc_aac", "guTransAAC" );
            if( GST_IS_ELEMENT( * enc ) )
            {
                g_object_set( * enc, "bitrate", guTranscodeWmaBitrates[ m_Quality ], NULL );

                * mux = gst_element_factory_make( "ffmux_mp4", "guTransAACMux" );
                if( GST_IS_ELEMENT( * mux ) )
                {
                    return true;
                }

                g_object_unref( * enc );
                * enc = NULL;
            }
            break;
        }

        case guTRANSCODE_FORMAT_WMA :
        {
            * enc = gst_element_factory_make( "ffenc_wmav2", "guTransWMA" );
            if( GST_IS_ELEMENT( * enc ) )
            {
                g_object_set( * enc, "bitrate", guTranscodeWmaBitrates[ m_Quality ], NULL );

                * mux = gst_element_factory_make( "ffmux_asf", "guTransWMAMux" );
                if( GST_IS_ELEMENT( * mux ) )
                {
                    return true;
                }

                g_object_unref( * enc );
                * enc = NULL;
            }
            break;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guTranscodeThread::BuildPipeline( void )
{
  guLogMessage( wxT( "guTranscodeThread::BuildPipeline" ) );
  m_Pipeline = gst_pipeline_new( "guTransPipeline" );
  if( GST_IS_ELEMENT( m_Pipeline ) )
  {
    GstElement * src;
    src = gst_element_factory_make( "filesrc", "guTransSource" );
    if( GST_IS_ELEMENT( src ) )
    {
      g_object_set( G_OBJECT( src ), "location", ( const char * ) m_Source.mb_str( wxConvFile ), NULL );

      GstElement * dec;
      dec = gst_element_factory_make( "decodebin2", "guTransDecoder" );
      if( GST_IS_ELEMENT( dec ) )
      {
        GstElement * conv;
        conv = gst_element_factory_make( "audioconvert", "guTransAudioConv" );
        if( GST_IS_ELEMENT( conv ) )
        {
          GstElement * enc = NULL;
          GstElement * mux = NULL;

          if( BuildEncoder( &enc, &mux ) )
          {
            GstElement * filesink;
            filesink = gst_element_factory_make( "filesink", "guTransFileSink" );
            if( GST_IS_ELEMENT( filesink ) )
            {
              g_object_set( filesink, "location", ( const char * ) m_Target.mb_str( wxConvFile ), NULL );

              if( mux )
              {
                gst_bin_add_many( GST_BIN( m_Pipeline ), src, dec, conv, enc, mux, filesink, NULL );
              }
              else
              {
                gst_bin_add_many( GST_BIN( m_Pipeline ), src, dec, conv, enc, filesink, NULL );
              }

              g_object_set( m_Pipeline, "async-handling", true, NULL );

              GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Pipeline ) );
              gst_bus_add_watch( bus, ( GstBusFunc ) gst_bus_async_callback, this );
              gst_object_unref( G_OBJECT( bus ) );

              if( gst_element_link( src, dec ) )
              {
                g_signal_connect( dec, "new-decoded-pad", G_CALLBACK( on_pad_added ), conv );

                if( mux )
                {
                    gst_element_link_many( conv, enc, mux, filesink, NULL );
                }
                else
                {
                    gst_element_link_many( conv, enc, filesink, NULL );
                }

                gst_element_set_state( m_Pipeline, GST_STATE_PAUSED );

                return;
              }
              else
              {
                  guLogError( wxT( "couldnt link the source and the decoder" ) );
              }

              gst_object_unref( filesink );
            }
            else
            {
              guLogError( wxT( "Error creating the transcode filesink" ) );
            }
            gst_object_unref( enc );
            if( mux )
            {
              gst_object_unref( mux );
            }
          }
          else
          {
            guLogError( wxT( "Error creating the transcode encoder" ) );
          }
          gst_object_unref( conv );
        }
        else
        {
          guLogError( wxT( "Error creating the transcode converter" ) );
        }
        gst_object_unref( dec );
      }
      else
      {
        guLogError( wxT( "Error creating the transcode decoder" ) );
      }
      gst_object_unref( src );
    }
    else
    {
        guLogError( wxT( "Error creating the transcode source" ) );
    }
    gst_object_unref( m_Pipeline );
    m_Pipeline = NULL;
  }
  else
  {
    guLogError( wxT( "Error creating the transcode pipeline" ) );
  }
  m_HasError = true;
}

// -------------------------------------------------------------------------------- //
guTranscodeThread::ExitCode guTranscodeThread::Entry()
{
    if( m_Pipeline )
    {
        gst_element_set_state( m_Pipeline, GST_STATE_PLAYING );

        m_Running = true;
        while( !TestDestroy() && m_Running )
        {
            Sleep( 100 );
        }
        Sleep( 500 );
    }
    //guLogMessage( wxT( "Finished Transcode thread..." ) );
    return 0;
}

// -------------------------------------------------------------------------------- //
void guTranscodeThread::Stop( void )
{

    guTagInfo * InTagInfo = guGetTagInfoHandler( m_Source );
    if( InTagInfo )
    {
        InTagInfo->Read();
        guTagInfo * OutTagInfo = guGetTagInfoHandler( m_Target );
        if( OutTagInfo )
        {
            OutTagInfo->m_TrackName = InTagInfo->m_TrackName;
            OutTagInfo->m_GenreName = InTagInfo->m_GenreName;
            OutTagInfo->m_ArtistName = InTagInfo->m_ArtistName;
            OutTagInfo->m_AlbumArtist = InTagInfo->m_AlbumArtist;
            OutTagInfo->m_AlbumName = InTagInfo->m_AlbumName;
            OutTagInfo->m_Composer = InTagInfo->m_Composer;
            OutTagInfo->m_Comments = InTagInfo->m_Comments;
            OutTagInfo->m_Track = InTagInfo->m_Track;
            OutTagInfo->m_Year = InTagInfo->m_Year;
            OutTagInfo->m_Disk = InTagInfo->m_Disk;
            OutTagInfo->m_TrackLabels = InTagInfo->m_TrackLabels;
            OutTagInfo->m_TrackLabelsStr = InTagInfo->m_TrackLabelsStr;
            OutTagInfo->m_ArtistLabels = InTagInfo->m_ArtistLabels;
            OutTagInfo->m_ArtistLabelsStr = InTagInfo->m_ArtistLabelsStr;
            OutTagInfo->m_AlbumLabels = InTagInfo->m_AlbumLabels;
            OutTagInfo->m_AlbumLabelsStr = InTagInfo->m_AlbumLabelsStr;
            OutTagInfo->m_Compilation = InTagInfo->m_Compilation;
            OutTagInfo->Write();

            delete OutTagInfo;
        }

        delete InTagInfo;
    }

    m_Running = false;
}

// -------------------------------------------------------------------------------- //
