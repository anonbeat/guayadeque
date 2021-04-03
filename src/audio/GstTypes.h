
#ifndef __GSTTYPES_H__
#define __GSTTYPES_H__

#include <vector>
#include <gst/gst.h>

#include "Utils.h"

namespace Guayadeque {

typedef std::vector<GstElement*> guGstElementsChain;

struct guGstPipelineElementPack
{
    GstElement *    element;
    GstElement **   element_ref;
};

// struct template for a nicer gstreamer objects unref
template< class GstObjectType = GstObject >
struct guGstPtr
{
    GstObjectType   *ptr;
    guGstPtr( GstObjectType *go ) { ptr = go; }
    ~guGstPtr() { if( ptr != NULL ) gst_object_unref( ptr ); }
};

// stateful GstElement unref'er
struct guGstElementStatePtr : guGstPtr<GstElement>
{
    guGstElementStatePtr( GstElement *go ): guGstPtr( go ) { }
    ~guGstElementStatePtr() { if( GST_IS_ELEMENT(ptr) ) gst_element_set_state( ptr, GST_STATE_NULL ); }
};

// stateful GstMessage unref'er
struct guGstMessagePtr : guGstPtr<GstMessage>
{
    guGstMessagePtr( GstMessage *go ): guGstPtr( go ) { }
    ~guGstMessagePtr() { if( GST_IS_MESSAGE(ptr) ) { gst_message_unref( ptr ); ptr=NULL; } }
};

}

#endif
