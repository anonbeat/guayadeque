
#include "GstPipelineActuator.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guGstPipelineBuilder
// -------------------------------------------------------------------------------- //


// -------------------------------------------------------------------------------- //
guGstPipelineActuator::guGstPipelineActuator( GstElement *element )
{
    guLogDebug( "guGstPipelineActuator <%s>", GST_ELEMENT_NAME(element) );
    m_Chain.push_back( element );
}


// -------------------------------------------------------------------------------- //
guGstPipelineActuator::guGstPipelineActuator( guGstElementsChain chain )
{
    guLogDebug( "guGstPipelineActuator [%lu]", chain.size() );
    m_Chain = chain;
}


// -------------------------------------------------------------------------------- //
static GstPadProbeReturn
guPlugGstElementProbe( GstPad *pad, GstPadProbeInfo *info, gpointer data )
{

    GstElement *what = (GstElement*)data;
    guLogDebug( "guPlugGstElementProbe << %s", GST_ELEMENT_NAME(data) );
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
guPlugGstElement( GstElement *plug_element, GstElement *previous_element )
{
    guLogDebug( "guPlugGstElement << <%s> after <%s>", GST_ELEMENT_NAME(plug_element), GST_ELEMENT_NAME(previous_element) );
    if( !guIsGstElementLinked(previous_element) )
    {
        guLogError( "Element plug error: previous element is unplugged" );
        return false;
    }
    GstBin *previous_element_parent = GST_BIN(GST_OBJECT_PARENT(previous_element));
    if( GST_BIN(GST_OBJECT_PARENT(plug_element)) != previous_element_parent )
    {
        guLogDebug( "guPlugGstElement add <%s> to bin <%s>", GST_ELEMENT_NAME(plug_element), GST_ELEMENT_NAME(previous_element_parent) );
        if( !gst_bin_add( previous_element_parent, plug_element) )
            guLogTrace( "Element plug problem: failed to add element <%s> to bin <%s>", GST_ELEMENT_NAME(plug_element), GST_ELEMENT_NAME(previous_element_parent) );
    }

    GstPad *src_pad = gst_element_get_static_pad( previous_element, "src" );
    if( src_pad == NULL )
    {
        guLogError( "Unable to get source pad of the element <%s>", GST_ELEMENT_NAME(previous_element) );
        return false;
    }
    gulong res = gst_pad_add_probe( src_pad, GST_PAD_PROBE_TYPE_IDLE, guPlugGstElementProbe, plug_element, NULL );
    if( res == 0 )
    {
        if( !guIsGstElementLinked( plug_element ) )
            guLogError( "Failed to add the probe on source pad <%s> of the element <%s>", GST_ELEMENT_NAME(src_pad), GST_ELEMENT_NAME(previous_element) );
        else
            res = ULONG_MAX; // guPlugGstElementProbe was executed ok in the current thread
    }

    gst_object_unref( src_pad );
    return res;

}


// -------------------------------------------------------------------------------- //
bool guGstPipelineActuator::Enable( GstElement *element )
{
    guLogDebug( "guGstPipelineActuator::Enable << %s", GST_ELEMENT_NAME(element) );
    if( guIsGstElementLinked( element ) )
    {
        guLogDebug( "guGstPipelineActuator::Enable >> already enabled" );
        return true;
    }

    GstElement *found_here = NULL, *plug_here = NULL;

    for( GstElement *pf : m_Chain )
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
        guLogError( "Unable to locate previous element in the chain, <%s> can not be plugged", GST_ELEMENT_NAME(element) );
        return false;
    }
    guLogDebug( "guGstPipelineActuator::Enable previous_element_name <%s>", GST_ELEMENT_NAME(plug_here) );

    bool res = guPlugGstElement( element, plug_here );
    guLogDebug( "guGstPipelineActuator::Enable >> %i", res );
    return res;
}


// -------------------------------------------------------------------------------- //
static GstPadProbeReturn
guUnplugGstElementProbe( GstPad *previous_src_pad, GstPadProbeInfo *info, gpointer data )
{
    guLogDebug( "guUnplugGstElementProbe on pad <%s> of <%s>", GST_OBJECT_NAME(previous_src_pad), GST_ELEMENT_NAME(GST_OBJECT_PARENT(previous_src_pad)) );

    guGstPtr<GstPad> previous_src_pad_peer_gp( gst_pad_get_peer( previous_src_pad ) );
    GstPad *previous_src_pad_peer = previous_src_pad_peer_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe previous_src_pad_peer", previous_src_pad_peer );
    if( previous_src_pad_peer == NULL )
    {
        guLogError( "Unplug failed: pad <%s> of element <%s> has null peer", GST_OBJECT_NAME(previous_src_pad), GST_ELEMENT_NAME(GST_OBJECT_PARENT(previous_src_pad)) );
        return GST_PAD_PROBE_REMOVE;
    }

    guGstPtr<GstPad> unplug_me_sink_pad_gp( guGetPeerPad( previous_src_pad ) );
    GstPad *unplug_me_sink_pad = unplug_me_sink_pad_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe unplug_me_sink_pad", unplug_me_sink_pad );
    if( unplug_me_sink_pad == NULL )
    {
        guLogError( "Unplug failed: pad <%s> of element <%s> has null end peer", GST_OBJECT_NAME(previous_src_pad), GST_ELEMENT_NAME(GST_OBJECT_PARENT(previous_src_pad)) );
        return GST_PAD_PROBE_REMOVE;
    }

    guGstPtr<GstElement> unplug_me_gp( gst_pad_get_parent_element( unplug_me_sink_pad ) );
    GstElement *unplug_me = unplug_me_gp.ptr;
    guLogDebug( "guUnplugGstElementProbe unplug_me is <%s>", GST_OBJECT_NAME(unplug_me) );
    if( unplug_me == NULL )
    {
        guLogError( "Unplug failed: target element of pad <%s> (<%s>) is null", GST_OBJECT_NAME(previous_src_pad), GST_ELEMENT_NAME(GST_OBJECT_PARENT(previous_src_pad)) );
        return GST_PAD_PROBE_REMOVE;
    }

    guGstPtr<GstPad> unplug_me_src_pad_gp( gst_element_get_static_pad( unplug_me, "src" ) );
    GstPad *unplug_me_src_pad = unplug_me_src_pad_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe unplug_me_src_pad", unplug_me_src_pad );
    if( unplug_me_src_pad == NULL )
    {
        guLogError( "Unplug failed: target element <%s> has no 'src' pad", GST_ELEMENT_NAME(unplug_me) );
        return GST_PAD_PROBE_REMOVE;
    }

    guGstPtr<GstPad> next_sink_pad_gp( guGetPeerPad( unplug_me_src_pad ) );
    GstPad *next_sink_pad = next_sink_pad_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe next_sink_pad", next_sink_pad );
    if( next_sink_pad == NULL )
    {
        guLogError( "Unplug failed: sink pad of <%s> pad is null", GST_OBJECT_NAME(unplug_me_src_pad) );
        return GST_PAD_PROBE_REMOVE;
    }

    guGstPtr<GstPad> next_sink_pad_peer_gp( gst_pad_get_peer( next_sink_pad ) );
    GstPad *next_sink_pad_peer = next_sink_pad_peer_gp.ptr;
    guLogGstPadData( "guUnplugGstElementProbe next_sink_pad_peer", next_sink_pad_peer );
    if( next_sink_pad_peer == NULL )
    {
        guLogError( "Unplug failed: peer pad of <%s> pad is null", GST_OBJECT_NAME(next_sink_pad) );
        return GST_PAD_PROBE_REMOVE;
    }

    if( gst_pad_unlink( previous_src_pad, previous_src_pad_peer ) && gst_pad_unlink( next_sink_pad_peer, next_sink_pad ) )
    {
        int lres = gst_pad_link( previous_src_pad, next_sink_pad );
        guLogDebug( "guUnplugGstElementProbe link result=%i", lres );
        if( lres == GST_PAD_LINK_OK )
        {
            // happy finish
            gst_pad_send_event( previous_src_pad_peer, gst_event_new_eos() );
            gst_element_set_state( unplug_me, GST_STATE_NULL);
            return GST_PAD_PROBE_REMOVE;
        }
        else
            guLogError( "Unplug failed: unable to link pads" );
    }
    else
        guLogError( "Unplug failed: unable to unlink pads" );

    gst_pad_link( previous_src_pad, previous_src_pad_peer );
    gst_pad_link( next_sink_pad_peer, next_sink_pad );

    return GST_PAD_PROBE_REMOVE;
}


// -------------------------------------------------------------------------------- //
static bool
guUnplugGstElement( GstElement *unplug_me )
{
    guLogDebug( "guUnplugGstElement << <%s>", GST_ELEMENT_NAME(unplug_me) );
    bool unplug_result = false;
    gst_element_foreach_sink_pad( unplug_me,
        [] ( GstElement * element, GstPad * sink_pad, void * user_data ) 
        {
            bool *res_ptr = (bool *)user_data;
            guLogGstPadData( "guUnplugGstElement unplug sink pad",sink_pad );
            GstPad *sink_peer = guGetPeerPad( sink_pad );
            guLogDebug( "guUnplugGstElement sink_peer is <%s>", GST_OBJECT_NAME(sink_peer) );
            if ( sink_peer != NULL )
            {
                if( gst_pad_add_probe( sink_peer, GST_PAD_PROBE_TYPE_IDLE, guUnplugGstElementProbe, NULL, NULL ) )
                    *res_ptr = true;
                else
                    guLogTrace( "Failed to add element probe for element <%s> pad <%s>", GST_ELEMENT_NAME(GST_OBJECT_PARENT(sink_peer)), GST_OBJECT_NAME(sink_peer) );
            }
            else
                guLogTrace( "Unplug target is null for element <%s> pad <%s>", GST_ELEMENT_NAME(GST_OBJECT_PARENT(sink_pad)), GST_OBJECT_NAME(sink_pad) );
            return 1;
        }, &unplug_result );
    guLogDebug( "guUnplugGstElement >> %i", unplug_result);
    return unplug_result;
}


// -------------------------------------------------------------------------------- //
bool guGstPipelineActuator::Disable( GstElement *element )
{
    guLogDebug( "guGstPipelineActuator::Disable << %s", GST_ELEMENT_NAME(element) );
    if( !guIsGstElementLinked( element ) )
    {
        guLogDebug( "guGstPipelineActuator::Disable >> already disabled" );
        return true;
    }
    return guUnplugGstElement( element );
}


// -------------------------------------------------------------------------------- //
bool guGstPipelineActuator::Toggle( GstElement *element )
{
    guLogDebug( "guGstPipelineActuator::Toggle <%s>", GST_ELEMENT_NAME(element) );
    return guIsGstElementLinked( element ) ? Disable( element ) : Enable( element );
}

} // namespace Guayadeque
