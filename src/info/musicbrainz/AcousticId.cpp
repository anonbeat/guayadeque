// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#include "AcousticId.h"

#include "EventCommandIds.h"
#include "MusicBrainz.h"
#include "Utils.h"
#include "Version.h"

#include <wx/sstream.h>
#include <wx/uri.h>

namespace Guayadeque {

#define guACOUSTICID_URL      wxT( "http://api.acoustid.org/v2/lookup?client=0ND2BN8mP3&duration=%i&format=xml&meta=recordings+compress&fingerprint=%s" )
#define guACOUSTICID_CLIENT_ID    "0ND2BN8mP3"

// -------------------------------------------------------------------------------- //
// guAcousticIdThread
// -------------------------------------------------------------------------------- //
class guAcousticIdThread : public wxThread
{
  protected :
    wxString            m_FileName;
    bool                m_Running;
    guAcousticId *      m_AcousticId;
    GstElement *        m_Pipeline;
    unsigned int        m_Start;
    unsigned int        m_Length;

    bool                BuildPipeline( void );

  public :
    guAcousticIdThread( guAcousticId * acousticid, const wxChar * filename,
                      const unsigned int start = 0, const unsigned int length = wxNOT_FOUND );
    ~guAcousticIdThread();

    virtual ExitCode    Entry();
    void                SetFingerprint( const char * fingerprint );
    void                Stop( void );
};




// -------------------------------------------------------------------------------- //
extern "C" {

#define GST_TAG_CHROMAPRINT_FINGERPRINT "chromaprint-fingerprint"

// -------------------------------------------------------------------------------- //
static gboolean gst_bus_async_callback( GstBus * bus, GstMessage * message, guAcousticIdThread * pobj )
{
    //guLogMessage( wxT( "Got gstreamer message %u" ), GST_MESSAGE_TYPE( message ) );
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

            pobj->Stop();
            break;
        }

        case GST_MESSAGE_EOS :
        {
          //guLogMessage( wxT( "EOS Detected..." ) );
          pobj->Stop();
          break;
        }

        case GST_MESSAGE_TAG :
        {
            /* The stream discovered new tags. */
            GstTagList * tags;
            gchar * fingerprint = NULL;
            /* Extract from the message the GstTagList.
            * This generates a copy, so we must remember to free it.*/
            gst_message_parse_tag( message, &tags );

            gst_tag_list_get_string( tags, GST_TAG_CHROMAPRINT_FINGERPRINT, &fingerprint );

            if( fingerprint )
            {
                //guLogMessage( wxT( "Gstreamer got fingerprint '%s'" ), wxString( fingerprint, wxConvUTF8 ).c_str() );
                pobj->SetFingerprint( fingerprint );
                g_free( fingerprint );
            }

            /* Free the tag list */
            gst_tag_list_unref( tags );
            break;
        }

        default:
            break;
    }

    return TRUE;
}

// -------------------------------------------------------------------------------- //
static void on_pad_added( GstElement * comp, GstPad * pad, GstElement * conv )
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
  caps = gst_pad_query_caps( pad, NULL );
  str = gst_caps_get_structure( caps, 0 );
  if( !g_strrstr( gst_structure_get_name( str ), "audio" ) )
  {
    gst_caps_unref( caps );
    gst_object_unref( convpad );
    return;
  }
  gst_caps_unref( caps );

  //guLogMessage( wxT( "Linked composer and converter..." ) );
  /* link'n'play */
  gst_pad_link( pad, convpad );
  g_object_unref( convpad );
}


}


// -------------------------------------------------------------------------------- //
// guAcousticIdThread
// -------------------------------------------------------------------------------- //
guAcousticIdThread::guAcousticIdThread( guAcousticId * AcousticId, const wxChar * filename,
    const unsigned int start, const unsigned int length )
{
  //guLogMessage( wxT( "guAcousticIdThread..." ) );
  m_AcousticId = AcousticId;
  m_FileName = wxString( filename );
  m_Running = false;
  int Error = guAcousticId::guAID_STATUS_ERROR_GSTREAMER;
  m_Start = start;
  m_Length = length;

  if( BuildPipeline() )
  {
    if( Create() == wxTHREAD_NO_ERROR )
    {
      SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
      Run();
    }
    else
    {
      Error = guAcousticId::guAID_STATUS_ERROR_THREAD;
      m_AcousticId->SetStatus( Error );
    }
  }
}

// -------------------------------------------------------------------------------- //
guAcousticIdThread::~guAcousticIdThread()
{
    if( GST_IS_ELEMENT( m_Pipeline ) )
    {
        gst_element_set_state( m_Pipeline, GST_STATE_NULL );
        GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Pipeline ) );
        gst_bus_remove_watch( bus );
        gst_object_unref( G_OBJECT( bus ) );
        gst_object_unref( GST_OBJECT( m_Pipeline ) );
        m_Pipeline = NULL;
    }

    m_AcousticId->ClearAcousticIdThread();
    //guLogMessage( wxT( "Destroyed AcousticIdThread..." ) );
}

// -------------------------------------------------------------------------------- //
bool guAcousticIdThread::BuildPipeline( void )
{
  m_Pipeline = gst_pipeline_new( "guPipeline" );
  if( GST_IS_ELEMENT( m_Pipeline ) )
  {
    GstElement * src;
    src = gst_element_factory_make( "giosrc", "guTransSource" );
    if( GST_IS_ELEMENT( src ) )
    {
      wxString Location;
      wxURI URI( m_FileName );
      if( URI.IsReference() )
      {
        Location = wxT( "file://" ) + m_FileName;
      }
      else
      {
        if( !URI.HasScheme() )
        {
          Location = wxT( "http://" ) + m_FileName;
        }
        else
        {
          Location = m_FileName;
        }
      }

      g_object_set( G_OBJECT( src ), "location", ( const char * ) Location.mb_str( wxConvFile ), NULL );

      GstElement * dec;
      dec = gst_element_factory_make( "decodebin", "guTransDecoder" );
      if( GST_IS_ELEMENT( dec ) )
      {
        GstElement * conv = gst_element_factory_make( "audioconvert", "guConverter" );
        if( GST_IS_ELEMENT( conv ) )
        {
          GstElement * chrom = gst_element_factory_make( "chromaprint", "guChroma" );
          if( GST_IS_ELEMENT( chrom ) )
          {
            g_object_set( G_OBJECT( chrom ), "duration", 60, NULL );

            GstElement * fake = gst_element_factory_make( "fakesink", "guFakeSink" );
            if( GST_IS_ELEMENT( fake ) )
            {
              g_object_set( G_OBJECT( fake ), "sync", 0, NULL );

              gst_bin_add_many( GST_BIN( m_Pipeline ), src, dec, conv, chrom, fake, NULL );

              g_object_set( m_Pipeline, "async-handling", true, NULL );

              GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_Pipeline ) );
              gst_bus_add_watch( bus, ( GstBusFunc ) gst_bus_async_callback, this );
              gst_object_unref( G_OBJECT( bus ) );

              if( gst_element_link( src, dec ) )
              {
                g_signal_connect( dec, "pad-added", G_CALLBACK( on_pad_added ), conv );

                if( gst_element_link_many( conv, chrom, fake, NULL ) )
                {
                  gst_element_set_state( m_Pipeline, GST_STATE_PAUSED );

                  //guLogMessage( wxT( "Created the pipeline..." ) );
                  return true;
                }
                else
                {
                  guLogError( wxT( "Error linking the objects src, dec, conv, chrom, fake" ) );
                }
              }
              else
              {
                guLogError( wxT( "couldnt link the source and the decoder" ) );
              }


              gst_object_unref( fake );
            }
            else
            {
                guLogError( wxT( "Error creating the AcousticId sink" ) );
            }

            gst_object_unref( chrom );
          }
          else
          {
            guLogError( wxT( "Error creating the AcousticId chromaprint" ) );
          }

          gst_object_unref( conv );
        }
        else
        {
          guLogError( wxT( "Error creating the AcousticId converter" ) );
        }

        gst_object_unref( dec );
      }
      else
      {
        guLogError( wxT( "Error creating the AcousticId decoder" ) );
      }

      gst_object_unref( src );
    }
    else
    {
      guLogError( wxT( "Error creating the AcousticId source" ) );
    }

    gst_object_unref( m_Pipeline );
  }
  else
  {
        guLogError( wxT( "Error creating the AcousticId pipeline" ) );
  }

  return false;
}

// -------------------------------------------------------------------------------- //
guAcousticIdThread::ExitCode guAcousticIdThread::Entry()
{
    gst_element_set_state( m_Pipeline, GST_STATE_PLAYING );

    m_Running = true;
    while( !TestDestroy() && m_Running )
    {
        Sleep( 2000 );
        //gint64 Pos = -1;
        //gst_element_query_position( m_Pipeline, GST_FORMAT_TIME, &Pos );
        //guLogMessage( wxT( "Running...%llu" ), Pos );
    }
    //guLogMessage( wxT( "Finished guAcousticIdThread..." ) );
    return 0;
}

// -------------------------------------------------------------------------------- //
void guAcousticIdThread::SetFingerprint( const char * fingerprint )
{
    m_AcousticId->SetFingerprint( fingerprint );
    Stop();
}

// -------------------------------------------------------------------------------- //
void guAcousticIdThread::Stop( void )
{
    //gst_element_set_state( m_Pipeline, GST_STATE_PAUSED );

    m_Running = false;
}

// -------------------------------------------------------------------------------- //
// guAcousticId
// -------------------------------------------------------------------------------- //
guAcousticId::guAcousticId( guAcousticIdClient * acousticidclient )
{
    m_AcousticIdThread = NULL;
    m_AcousticIdClient = acousticidclient;
    m_Status = guAID_STATUS_OK;
}

// -------------------------------------------------------------------------------- //
guAcousticId::~guAcousticId()
{
    if( m_AcousticIdThread )
    {
        m_AcousticIdThread->Pause();
        m_AcousticIdThread->Delete();
    }
}

// -------------------------------------------------------------------------------- //
void guAcousticId::SearchTrack( const guTrack * track )
{
    m_Track = track;
    m_MBId.Clear();
    m_Fingerprint.Clear();

    DoGetFingerprint();
}

// -------------------------------------------------------------------------------- //
wxString guAcousticId::GetXmlDoc( void )
{
    return m_XmlDoc;
}

// -------------------------------------------------------------------------------- //
void guAcousticId::SetFingerprint( const wxString &fingerprint )
{
    m_Fingerprint = fingerprint;
    m_AcousticIdClient->OnAcousticIdFingerprintFound( m_Fingerprint );

    if( !m_Fingerprint.IsEmpty() )
    {
        DoGetMetadata();
    }
    else
    {
        SetStatus( guAID_STATUS_ERROR_NO_FINGERPRINT );
    }
}

// -------------------------------------------------------------------------------- //
void guAcousticId::SetFingerprint( const char * fingerprint )
{
    SetFingerprint( wxString( fingerprint, wxConvUTF8 ) );
}

// -------------------------------------------------------------------------------- //
void guAcousticId::SetXmlDoc( const wxString &xmldoc )
{
}

// -------------------------------------------------------------------------------- //
wxString guAcousticId::GetMBId( void )
{
    return m_MBId;
}

// -------------------------------------------------------------------------------- //
void guAcousticId::SetMBId( const wxString &mbid )
{
    m_MBId = mbid;
    if( m_AcousticIdThread )
    {
        CancelSearch();
    }
    //guLogMessage( wxT( "Calling OnFingerprintFound..." ) );
    m_AcousticIdClient->OnAcousticIdMBIdFound( m_MBId );
}


// -------------------------------------------------------------------------------- //
void guAcousticId::ClearAcousticIdThread( void )
{
    m_AcousticIdThread = NULL;
}

// -------------------------------------------------------------------------------- //
bool guAcousticId::IsRunning( void )
{
    return m_AcousticIdThread != NULL;
}

// -------------------------------------------------------------------------------- //
void guAcousticId::CancelSearch( void )
{
    if( m_AcousticIdThread )
    {
        m_AcousticIdThread->Pause();
        m_AcousticIdThread->Delete();
        m_AcousticIdThread = NULL;
    }
}

// -------------------------------------------------------------------------------- //
bool guAcousticId::DoGetFingerprint( void )
{
    //guLogMessage( wxT( "DoGetFingerprint..." ) );
    if( m_Track )
    {
        if( m_AcousticIdThread )
        {
            CancelSearch();
        }
        m_AcousticIdThread = new guAcousticIdThread( this, m_Track->m_FileName.c_str() );
    }
    return m_AcousticIdThread != NULL;
}

// -------------------------------------------------------------------------------- //
bool guAcousticId::DoGetMetadata( void )
{
    wxString ReqUrl = wxString::Format( guACOUSTICID_URL,
        m_Track->m_Length / 1000,
        m_Fingerprint.c_str() );

    //guLogMessage( ReqUrl );
    wxString Content = GetUrlContent( ReqUrl );
    if( !Content.IsEmpty() )
    {
        m_XmlDoc = Content;

        return DoParseXmlDoc();
    }
    else
    {
        SetStatus( guAID_STATUS_ERROR_HTTP );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guAcousticId::DoParseXmlDoc( void )
{
    /*
    <response>
        <status>ok</status>
        <results>
            <result>
                <recordings>
                    <recording>
                        <duration>138</duration>
                        <title>Don't Panic</title>
                        <id>0f87975c-5e6a-4a42-a19a-fed67f0d3560</id>
                        <artists>
                            <artist>
                                <id>cc197bad-dc9c-440d-a5b5-d52ba2e14234</id>
                                <name>Coldplay</name>
                            </artist>
                        </artists>
                    </recording>
                    <recording>...</recording>
                    <recording>...</recording>
                </recordings>
                <score>0.994859</score>
                <id>0fc6a463-6845-4181-a354-66e326c6db57</id>
            </result>
            <result>...</result>
            <result>...</result>
        </results>
    </response>
    */
    //guLogMessage( wxT( "XML Response: %s" ), m_XmlDoc.c_str() );
    wxStringInputStream ins( m_XmlDoc );
    wxXmlDocument XmlDoc( ins );
    wxXmlNode * XmlNode = XmlDoc.GetRoot();
    if( XmlNode && ( XmlNode->GetName() == wxT( "response" ) ) )
    {
        XmlNode = XmlNode->GetChildren();
        while( XmlNode )
        {
            //guLogMessage( wxT( "AcousticId xml %s => %s" ), XmlNode->GetName().c_str(), XmlNode->GetNodeContent().c_str() );
            if( XmlNode->GetName() == wxT( "status" ) )
            {
                if( XmlNode->GetNodeContent() != wxT( "ok" ) )
                {
                    SetStatus( guAID_STATUS_ERROR_BAD_STATUS );
                    return false;
                }
            }
            else if( XmlNode->GetName() == wxT( "results" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "result" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "recordings" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "recording" ) )
            {
                XmlNode = XmlNode->GetChildren();
                continue;
            }
            else if( XmlNode->GetName() == wxT( "id" ) )
            {
                SetMBId( XmlNode->GetNodeContent() );
                return true;
            }

            XmlNode = XmlNode->GetNext();
        }
        SetStatus( guAID_STATUS_ERROR_XMLPARSE );
    }
    else
    {
        SetStatus( guAID_STATUS_ERROR_XMLERROR );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guAcousticId::SetStatus( const int status )
{
    m_Status = status;
    m_AcousticIdClient->OnAcousticIdError( m_Status );
}

// -------------------------------------------------------------------------------- //
int guAcousticId::GetStatus( void )
{
    return m_Status;
}

// -------------------------------------------------------------------------------- //
bool guAcousticId::IsOk( void )
{
    return m_Status == guAID_STATUS_OK;
}

}

// -------------------------------------------------------------------------------- //
