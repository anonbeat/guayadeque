

#include "GstUtils.h"

namespace Guayadeque {

//
// debugging routines
//
#ifdef GU_DEBUG

// log pad info
void guLogGstPadData(const char * msg, GstPad *pad)
{
    if( pad != NULL )
        guLogDebug( "%s name=%s parent=%s",
            msg,
            GST_OBJECT_NAME(pad),
            GST_ELEMENT_NAME(GST_OBJECT_PARENT(pad))
            );
    else
        guLogDebug( "%s pad is null", msg );
}

#endif
// GU_DEBUG

//
// get actual peer of the pad avoiding proxy pads
//
GstPad * guGetPeerPad( GstPad * pad )
{
    guLogGstPadData( "guGetPeerPad <<", pad );
    if ( !pad )
        return NULL;

    GstPad *peer = gst_pad_get_peer( pad );
    guLogGstPadData( "guGetPeerPad peer", peer );

    while( peer !=NULL && GST_IS_PROXY_PAD( peer ) )
    {
        GstPad *next_pad;

        if( GST_IS_GHOST_PAD( peer ) )
        {
            next_pad = gst_ghost_pad_get_target( GST_GHOST_PAD( peer ) );
            guLogGstPadData( "guGetPeerPad ghost pad target", next_pad );
        }
        else
        {
            GstPad * opp = GST_PAD( gst_proxy_pad_get_internal( GST_PROXY_PAD( peer ) ) );
            guLogGstPadData( "guGetPeerPad proxy pad peer", opp );
            next_pad = gst_pad_get_peer( opp );
            gst_object_unref( opp );
        }

        gst_object_unref( peer );
        peer = next_pad;
    }

    guLogGstPadData( "guGetPeerPad >>", peer );
    return peer;
}



//
// check if any of element pads is linked to another element
//
bool guIsGstElementLinked( GstElement *element )
{
    if( element == NULL || !GST_IS_ELEMENT(element) )
    {
        guLogDebug( "guIsGstElementLinked << bad element %p", element );
        return false;
    }
    else
        guLogDebug( "guIsGstElementLinked << %s", GST_ELEMENT_NAME(element) );
    bool pads_connected = false;
    gst_element_foreach_pad( element,
        [](GstElement *e, GstPad *p, gpointer d)
        {
            bool *pads_connected = (bool *)d;
            GstPad *peer = guGetPeerPad(p);
            guLogGstPadData( "guIsGstElementLinked peer", peer );

            if( peer != NULL )
            {
                if( GST_OBJECT_PARENT(peer) != NULL )
                    *pads_connected = true;
                gst_object_unref( peer );
            }
            return 1;
        }, &pads_connected);
    guLogDebug( "guIsGstElementLinked >> %i", pads_connected );
    return pads_connected;
}

//
// set element state to NULL if unlinked
//
bool guGstStateToNullIfUnlinked( GstElement *element )
{
    if( element == NULL )
        return false;

    if( guIsGstElementLinked( element ) )
    {
        guLogDebug( "guGstStateToNullIfUnlinked: element is linked" );
        return false;
    }
    else
    {
        guLogDebug( "guGstStateToNullIfUnlinked: setting state" );
        if( GST_IS_ELEMENT( GST_OBJECT_PARENT( element ) ) )
        {
            gst_object_ref( element );
            if( !gst_bin_remove( GST_BIN( GST_OBJECT_PARENT( element ) ), element ) )
            {
                guLogDebug( "guGstStateToNullIfUnlinked gst_bin_remove fail" );
                gst_object_unref( element );
                return false;
            }
        }
        return gst_element_set_state( element, GST_STATE_NULL ) == GST_STATE_CHANGE_SUCCESS;
    }
}

//
// set element state to NULL and unref
//
bool guGstStateToNullAndUnref( GstElement *element )
{
    if( element == NULL || !GST_IS_ELEMENT( element ) )
        return true;

    if( GST_IS_ELEMENT( GST_OBJECT_PARENT( element ) ) )
    {
        gst_object_ref( element );
        if( !gst_bin_remove( GST_BIN( GST_OBJECT_PARENT( element ) ), element ) )
        {
            guLogTrace( "Failed to remove element <%s> from the bin", GST_ELEMENT_NAME( element ) );
            gst_object_unref( element );
        }
    }

    bool res = gst_element_set_state( element, GST_STATE_NULL ) == GST_STATE_CHANGE_SUCCESS;
    gst_object_unref( element );

    return res;
}

} // namespace Guayadeque
