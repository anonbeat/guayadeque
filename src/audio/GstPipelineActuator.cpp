
#include "GstPipelineActuator.h"

namespace Guayadeque {

struct guGstElementProbeData
{
    guGstResultHandler      * rhandler;
    void                    * probe_data;
    guGstElementProbeData( void * e, guGstResultHandler * rh )
    {
        rhandler = rh;
        probe_data = e;
    }
};

// -------------------------------------------------------------------------------- //
// guGstPipelineActuator
// -------------------------------------------------------------------------------- //


// -------------------------------------------------------------------------------- //
guGstPipelineActuator::guGstPipelineActuator( GstElement *element ) : guGstPipelineActuator()
{
    guLogDebug( "guGstPipelineActuator <%s>", GST_ELEMENT_NAME(element) );
    m_PrivateChain = true;
    m_Chain = new guGstElementsChain();
    m_Chain->push_back( element );
}


// -------------------------------------------------------------------------------- //
guGstPipelineActuator::guGstPipelineActuator( guGstElementsChain *chain ) : guGstPipelineActuator()
{
    guLogDebug( "guGstPipelineActuator [%lu]", chain->size() );
    m_Chain = chain;
    m_PrivateChain = false;
}

guGstPipelineActuator::~guGstPipelineActuator()
{
    guLogDebug( "~guGstPipelineActuator" );
    if( m_PrivateChain && m_Chain != NULL )
        delete m_Chain;
}

// -------------------------------------------------------------------------------- //
static GstPadProbeReturn guUnplugGstElementEOS( GstPad * my_pad, GstPadProbeInfo * info, guGstElementProbeData * pd )
{
    guLogGstPadData( "guUnplugGstElementEOS << ", my_pad );

    GstEvent *event = GST_PAD_PROBE_INFO_EVENT (info);
    if( GST_EVENT_TYPE( event ) != GST_EVENT_EOS )
        return GST_PAD_PROBE_OK;

    guGstResultExec rexec( pd->rhandler );
    GstElement *what = (GstElement *)pd->probe_data;
    delete pd;

    gst_object_ref( what );
    gst_bin_remove( GST_BIN( GST_OBJECT_PARENT( what ) ), what );

    guLogDebug( "guUnplugGstElementEOS >> " );
    return GST_PAD_PROBE_REMOVE;

}

// -------------------------------------------------------------------------------- //
static bool guScheduleEOSProbe( GstPad * pad, GstElement * unplug_me, guGstResultExec * rexec )
{
    GstPad * wait_eos_here = pad;
    guGstPtr<GstPad> wait_eos_here_unref( NULL );
    if( GST_IS_BIN( unplug_me ) ) // need to wait on the last sink
    {
        guLogDebug( "guScheduleEOSProbe unplug_me is bin");
        GstIterator * gi = gst_bin_iterate_sinks( GST_BIN( unplug_me ) );
        GValue v = G_VALUE_INIT;
        if( gi != NULL && gst_iterator_next( gi, &v ) == GST_ITERATOR_OK )
        {
            GstElement * e = GST_ELEMENT( g_value_get_object( &v ) );
            guLogDebug( "guScheduleEOSProbe bin sink: <%s>", GST_ELEMENT_NAME(e) );
            wait_eos_here = gst_element_get_static_pad( e, "sink" );
            wait_eos_here_unref.ptr = wait_eos_here;
            g_value_reset( &v );
            gst_iterator_free( gi );
        }
        else
            guLogTrace( "EOS scheduler: failed to list bin sinks, trying pad probe" );
    }
    guLogGstPadData( "guScheduleEOSProbe wait_eos_here", wait_eos_here );
    guGstElementProbeData * pd = new guGstElementProbeData( unplug_me, rexec->Pass() );
    if( gst_pad_add_probe( wait_eos_here, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
        GstPadProbeCallback( guUnplugGstElementEOS ), pd, NULL) )
    {
        gst_pad_send_event( pad, gst_event_new_eos() );
        return true;
    }
    guLogTrace( "EOS scheduler: add event probe failure" );
    rexec->Retake( pd->rhandler );
    delete pd;
    return false;
}

// -------------------------------------------------------------------------------- //
static GstPadProbeReturn
guPlugGstElementProbe( GstPad *pad, GstPadProbeInfo *info, gpointer data )
{

    guGstElementProbeData * pd = (guGstElementProbeData *)data;
    GstElement *what = (GstElement *)pd->probe_data;
    guGstResultExec rexec( pd->rhandler );
    delete pd;

    guLogDebug( "guPlugGstElementProbe << %s", GST_ELEMENT_NAME(what) );
    guLogGstPadData( "guPlugGstElementProbe my pad", pad );

    guGstPtr<GstPad> peer_gp( gst_pad_get_peer( pad ) );
    GstPad *peer = peer_gp.ptr;
    guLogGstPadData( "guPlugGstElementProbe plug to", peer );
    if( peer == NULL )
    {
        guLogError( "Plug failed: unable to get pad peer" );
        return GST_PAD_PROBE_REMOVE;
    }
    if( !gst_pad_unlink( pad, peer ) )
    {
        guLogError( "Plug failed: unable to unlink pads" );
        return GST_PAD_PROBE_REMOVE;
    }

    if( !gst_element_sync_state_with_parent( what ) )
    {
        guLogError( "Plug failed: unable to sync element <%s> state", GST_ELEMENT_NAME(what) );
        gst_pad_link( pad, peer );
        return GST_PAD_PROBE_REMOVE;
    }

    if( gst_element_link_pads( GST_ELEMENT( GST_OBJECT_PARENT( pad ) ), GST_OBJECT_NAME( pad ), what, NULL ) )
    {
        guGstPtr<GstPad> what_src_pad( gst_element_get_static_pad( what, "src" ) );
        if( what_src_pad.ptr == NULL || gst_element_link_pads( what, NULL, GST_ELEMENT( GST_OBJECT_PARENT( peer ) ), GST_OBJECT_NAME( peer ) ) )
        {
            guLogDebug( "guPlugGstElementProbe plugged ok" );
            if( what_src_pad.ptr == NULL )
            {
                guLogTrace( "Plug: element <%s> has no 'src' pad (replacing sink or bin?)", GST_ELEMENT_NAME( what ) );
                if( guScheduleEOSProbe( peer, GST_ELEMENT( GST_OBJECT_PARENT( peer ) ), &rexec ) )
                    guLogDebug( "guPlugGstElementProbe guScheduleEOSProbe ok" );
                else
                    guLogDebug( "guPlugGstElementProbe guScheduleEOSProbe fail" );
            }
            rexec.SetErrorMode( false );
            return GST_PAD_PROBE_REMOVE;
        }
        else
        {
            guLogError( "Plug failed: unable to link element <%s> sink pads", GST_ELEMENT_NAME( GST_OBJECT_PARENT( pad ) ) );
        }
    }
    else
        guLogError( "Plug failed: unable to link element <%s> src pads", GST_ELEMENT_NAME( GST_OBJECT_PARENT( pad ) ) );

    if( peer != NULL )
        gst_pad_link( pad, peer );

    return GST_PAD_PROBE_REMOVE;
}


// -------------------------------------------------------------------------------- //
static gulong
guPlugGstElement( GstElement *plug_element, GstElement *previous_element, const guGstResultHandler &rhandler )
{
    guLogDebug( "guPlugGstElement << <%s> after <%s>", GST_ELEMENT_NAME(plug_element), GST_ELEMENT_NAME(previous_element) );
    if( !guIsGstElementLinked(previous_element) )
    {
        guLogError( "Plug error: previous element is unplugged" );
        return false;
    }
    GstBin *previous_element_parent = GST_BIN(GST_OBJECT_PARENT(previous_element));
    if( GST_BIN(GST_OBJECT_PARENT(plug_element)) != previous_element_parent )
    {
        guLogDebug( "guPlugGstElement add <%s> to bin <%s>", GST_ELEMENT_NAME(plug_element), GST_ELEMENT_NAME(previous_element_parent) );
        if( !gst_bin_add( previous_element_parent, plug_element) )
            guLogTrace( "Plug problem: failed to add element <%s> to bin <%s>", GST_ELEMENT_NAME(plug_element), GST_ELEMENT_NAME(previous_element_parent) );
    }

    GstPad *src_pad = gst_element_get_static_pad( previous_element, "src" );
    if( src_pad == NULL )
    {
        guLogError( "Plug error: unable to get source pad of the element <%s>", GST_ELEMENT_NAME(previous_element) );
        return false;
    }
    guGstResultHandler * rh = new guGstResultHandler( rhandler );
    guGstElementProbeData * pd = new guGstElementProbeData( plug_element, rh );
    gulong res = gst_pad_add_probe( src_pad, GST_PAD_PROBE_TYPE_IDLE, guPlugGstElementProbe, pd, NULL );
    if( res == 0 )
    {
        if( guIsGstElementLinked( plug_element ) )
        {
            guLogDebug( "guPlugGstElement probe was executed ok in the current thread" );
            res = ULONG_MAX;
        }
        else
        {
            guLogError( "Plug error: failed to add the probe on source pad <%s> of the element <%s>", GST_ELEMENT_NAME(src_pad), GST_ELEMENT_NAME(previous_element) );
            delete rh;
            delete pd;
        }
    }

    gst_object_unref( src_pad );
    return res;

}


// -------------------------------------------------------------------------------- //
bool guGstPipelineActuator::Enable( GstElement *element, void * new_data )
{
    guLogDebug( "guGstPipelineActuator::Enable << %s", GST_ELEMENT_NAME(element) );
    if( new_data != NULL && m_ResultHandler != NULL )
        m_ResultHandler->m_ExecData = new_data;

    if( guIsGstElementLinked( element ) )
    {
        guLogDebug( "guGstPipelineActuator::Enable >> already enabled" );
        return true;
    }

    GstElement *found_here = NULL, *plug_here = NULL;

    for( GstElement *pf : *m_Chain )
    {
        if( pf == element )
        {
            found_here = pf;
            break;
        }
        if( guIsGstElementLinked( pf ) )
            plug_here = pf;
    }
    if( found_here != element || plug_here == NULL )
    {
        guLogError( "Plug error: unable to locate previous element in the chain, <%s> can not be plugged", GST_ELEMENT_NAME(element) );
        return false;
    }
    guLogDebug( "guGstPipelineActuator::Enable previous_element_name <%s>", GST_ELEMENT_NAME(plug_here) );

    bool res = guPlugGstElement( element, plug_here, m_ResultHandler != NULL ? *m_ResultHandler : guGstResultHandler( "Actuator Enable default handler" ) );
    guLogDebug( "guGstPipelineActuator::Enable >> %i", res );
    return res;
}

// -------------------------------------------------------------------------------- //
static GstPadProbeReturn
guUnplugGstElementProbe( GstPad *previous_src_pad, GstPadProbeInfo *info, gpointer data )
{
    guLogDebug( "guUnplugGstElementProbe on pad <%s> of <%s>", GST_OBJECT_NAME(previous_src_pad), GST_ELEMENT_NAME(GST_OBJECT_PARENT(previous_src_pad)) );

    guGstElementProbeData * pd = (guGstElementProbeData *)data;
    guGstResultExec rexec( pd->rhandler );
    GstElement *what = (GstElement *)pd->probe_data;
    delete pd;

    guGstPtr<GstPad> previous_src_pad_peer_gp( gst_pad_get_peer( previous_src_pad ) );
    GstPad *previous_src_pad_peer = previous_src_pad_peer_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe previous_src_pad_peer", previous_src_pad_peer );
    if( previous_src_pad_peer == NULL )
    {
        guLogError( "Unplug failed: pad <%s> of element <%s> has null peer", GST_OBJECT_NAME(previous_src_pad), GST_ELEMENT_NAME(GST_OBJECT_PARENT(previous_src_pad)) );
        return GST_PAD_PROBE_REMOVE;
    }

    GstElement *unplug_me = what;
    guLogDebug( "guUnplugGstElementProbe unplug_me is <%s>", GST_OBJECT_NAME(unplug_me) );
    if( unplug_me == NULL )
    {
        guLogError( "Unplug failed: target element of pad <%s> (<%s>) is null", GST_OBJECT_NAME(previous_src_pad), GST_ELEMENT_NAME(GST_OBJECT_PARENT(previous_src_pad)) );
        return GST_PAD_PROBE_REMOVE;
    }

    guGstPtr<GstPad> unplug_me_src_pad_gp( gst_element_get_static_pad( unplug_me, "src" ) );
    GstPad *unplug_me_src_pad = unplug_me_src_pad_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe unplug_me_src_pad", unplug_me_src_pad );
    bool unplug_me_is_sink = unplug_me_src_pad == NULL;
    if( unplug_me_is_sink )
    {
        guLogTrace( "Unplug: target element <%s> has no 'src' pad (last element?)", GST_ELEMENT_NAME(unplug_me) );
    }

    guGstPtr<GstPad> next_sink_pad_gp( unplug_me_is_sink ? NULL : gst_pad_get_peer( unplug_me_src_pad ) );
    GstPad *next_sink_pad = next_sink_pad_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe next_sink_pad", next_sink_pad );
    if( !unplug_me_is_sink && next_sink_pad == NULL )
    {
        guLogError( "Unplug failed: sink pad of <%s> pad is null", GST_OBJECT_NAME(unplug_me_src_pad) );
        return GST_PAD_PROBE_REMOVE;
    }

    guGstPtr<GstPad> next_sink_pad_peer_gp( unplug_me_is_sink ? NULL : gst_pad_get_peer( next_sink_pad ) );
    GstPad *next_sink_pad_peer = next_sink_pad_peer_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe next_sink_pad_peer", next_sink_pad_peer );
    if( !unplug_me_is_sink && next_sink_pad_peer == NULL )
    {
        guLogError( "Unplug failed: peer pad of <%s> pad is null", GST_OBJECT_NAME(next_sink_pad) );
        return GST_PAD_PROBE_REMOVE;
    }

    if( gst_pad_unlink( previous_src_pad, previous_src_pad_peer ) )
    {
        if( unplug_me_is_sink || gst_pad_unlink( next_sink_pad_peer, next_sink_pad ) )
        {
            int lres = GST_PAD_LINK_OK;
            if( !unplug_me_is_sink )
            {
                lres = gst_pad_link( previous_src_pad, next_sink_pad );
                guLogDebug( "guUnplugGstElementProbe link result=%i", lres );
            }
            if( lres == GST_PAD_LINK_OK )
            {
                // happy finish
                //
                if( unplug_me_is_sink && GST_STATE(unplug_me) == GST_STATE_PLAYING ) // need EOS
                {
                    if( guScheduleEOSProbe( previous_src_pad_peer, unplug_me, &rexec ) )
                    {
                        guLogDebug( "guUnplugGstElementProbe guScheduleEOSProbe ok" );
                        return GST_PAD_PROBE_REMOVE;
                    }
                    else
                        guLogDebug( "guUnplugGstElementProbe guScheduleEOSProbe fail" );
                }
                guLogDebug( "guUnplugGstElementProbe no eos unplug" );
                gst_object_ref( unplug_me );
                if( !gst_bin_remove( GST_BIN( GST_OBJECT_PARENT( unplug_me ) ), unplug_me ) )
                {
                    guLogDebug( "guUnplugGstElementProbe gst_bin_remove fail" );
                    gst_object_unref( unplug_me );
                }
                rexec.SetErrorMode( false );
                return GST_PAD_PROBE_REMOVE;
            }
            else
            {
                guLogError( "Unplug failed: unable to link pads" );
                gst_pad_link( next_sink_pad_peer, next_sink_pad );
            }

        }
        gst_pad_link( previous_src_pad, previous_src_pad_peer );
    }
    else
        guLogError( "Unplug failed: unable to unlink source pads" );

    return GST_PAD_PROBE_REMOVE;
}


// -------------------------------------------------------------------------------- //
static bool
guUnplugGstElement( GstElement *unplug_me, const guGstResultHandler &rhandler )
{
    guLogDebug( "guUnplugGstElement << <%s>", GST_ELEMENT_NAME(unplug_me) );
    bool unplug_result = false;
    guGstElementProbeData pd( &unplug_result, (guGstResultHandler*)&rhandler );
    gst_element_foreach_sink_pad( unplug_me,
        [ ] ( GstElement * element, GstPad * sink_pad, void * user_data ) 
        {
            guGstElementProbeData * pd = (guGstElementProbeData *)user_data;
            bool *res_ptr = (bool *)pd->probe_data;
            guLogGstPadData( "guUnplugGstElement unplug sink pad", sink_pad );
            GstPad *sink_peer = gst_pad_get_peer( sink_pad );
            guLogGstPadData( "guUnplugGstElement sink_peer is", sink_peer );
            if ( sink_peer != NULL )
            {
                guGstResultHandler * rh = new guGstResultHandler( *pd->rhandler );
                guGstElementProbeData * for_probe = new guGstElementProbeData( element, rh );
                if( gst_pad_add_probe( sink_peer, GST_PAD_PROBE_TYPE_IDLE, guUnplugGstElementProbe, for_probe, NULL ) )
                    *res_ptr = true;
                else
                {
                    if( guIsGstElementLinked( element ) )
                    {
                        guLogTrace( "Unplug: failed to add element probe for element <%s> pad <%s>", GST_ELEMENT_NAME(GST_OBJECT_PARENT(sink_peer)), GST_OBJECT_NAME(sink_peer) );
                        delete rh;
                        delete for_probe;
                    }
                    else
                    {
                        guLogDebug( "guUnplugGstElement unplugged in current thread" );
                        *res_ptr = true;
                    }
                }
                gst_object_unref( sink_peer );
            }
            else
                guLogTrace( "Unplug: target is null for element <%s> pad <%s>", GST_ELEMENT_NAME(GST_OBJECT_PARENT(sink_pad)), GST_OBJECT_NAME(sink_pad) );
            return 1;
        }, &pd );
    guLogDebug( "guUnplugGstElement >> %i", unplug_result);
    return unplug_result;
}


// -------------------------------------------------------------------------------- //
bool guGstPipelineActuator::Disable( GstElement *element, void * new_data )
{
/*

safe gstreamer element unplug sequence (kinda fiddly):
 wait for GST_PAD_PROBE_TYPE_IDLE on src pad of the previous element
 unlink there the element, send EOS to the element for a happy finish
   wait for EOS during GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM on the element src/sink pad
   (or on the sink pad of the the last sink if the unplugged element is the GstBin)
   do gst_bin_remove during EOS and give control to handler function
     further element handling (like state change & unref) should be done by caller (in another thread)

*/
    guLogDebug( "guGstPipelineActuator::Disable << %s", GST_ELEMENT_NAME(element) );
    if( new_data != NULL && m_ResultHandler != NULL )
        m_ResultHandler->m_ExecData = new_data;
    if( !guIsGstElementLinked( element ) )
    {
        guLogDebug( "guGstPipelineActuator::Disable >> already disabled" );
        return true;
    }
    return guUnplugGstElement( element, m_ResultHandler != NULL ? *m_ResultHandler : guGstResultHandler( "Actuator Disable handler" ) );
}


// -------------------------------------------------------------------------------- //
bool guGstPipelineActuator::Toggle( GstElement *element, void * new_data )
{
    guLogDebug( "guGstPipelineActuator::Toggle <%s>", GST_ELEMENT_NAME(element) );
    return guIsGstElementLinked( element ) ? Disable( element, new_data ) : Enable( element, new_data );
}

} // namespace Guayadeque
