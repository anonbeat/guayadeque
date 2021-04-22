
#ifndef __GSTUTILS_H__
#define __GSTUTILS_H__

#include <gst/gst.h>

#include "Utils.h"

namespace Guayadeque {

// debugging routines

#ifdef GU_DEBUG

// log pad info
void guLogGstPadData( const char * msg, GstPad *pad );

#else

#define guLogGstPadData(...)

#endif
// GU_DEBUG

// get actual peer of the pad avoiding proxy pads
GstPad * guGetPeerPad( GstPad * pad );

// check if any of element pads is linked to another element
bool guIsGstElementLinked( GstElement *element );

// set element state to NULL if unlinked
bool guGstStateToNullIfUnlinked( GstElement *element );

// set element state to NULL and unref
bool guGstStateToNullAndUnref( GstElement *element );


} // namespace Guayadeque

#endif
// __GSTUTILS_H__

