
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

static void guGstResultHandler_noop_success_handler( const void *hint )
{
    guLogDebug( "guGstResultHandler_noop_success_handler: <%s>", (const char *)hint );
}

static void guGstResultHandler_noop_error_handler( const void *hint )
{
    guLogTrace( "Unhandled gstreamer error (hint: %s)", (const char *)hint );
}

struct guGstResultHandler
{
    
    typedef  void   (*Func)(const void *);

    void     * m_ExecData;
    Func    m_SuccessExec;
    Func    m_ErrorExec;

    guGstResultHandler( const guGstResultHandler &copy )
    {
        m_SuccessExec = copy.m_SuccessExec;
        m_ErrorExec = copy.m_ErrorExec;
        m_ExecData = copy.m_ExecData;
    }
    guGstResultHandler( const char * hint = NULL )
    {
        m_SuccessExec = guGstResultHandler_noop_success_handler;
        m_ErrorExec = guGstResultHandler_noop_error_handler;
        m_ExecData = (void *)hint;
    }
    guGstResultHandler( Func return_func, void *exec_data = NULL )
    {
        m_ExecData = exec_data;
        m_SuccessExec = return_func;
        m_ErrorExec = return_func;
    }
    guGstResultHandler( Func success_func, Func error_func, void *exec_data = NULL )
    {
        m_ExecData = exec_data;
        m_SuccessExec = success_func;
        m_ErrorExec = error_func;
    }
};

class guGstResultExec
{
private:
    bool                 m_ErrorMode;
    guGstResultHandler   * m_Handler;
public:
    guGstResultExec( guGstResultHandler *rhandler = NULL, bool error_mode = true )
    { 
        m_ErrorMode = error_mode;
        m_Handler = rhandler;
        if( m_Handler == NULL )
            m_Handler = new guGstResultHandler( "empty result handler" );
    }
    ~guGstResultExec()
    {
        if( m_Handler != NULL )
        {
            if( m_ErrorMode )
                m_Handler->m_ErrorExec( m_Handler->m_ExecData );
            else
                m_Handler->m_SuccessExec( m_Handler->m_ExecData );
            delete m_Handler;
        }
    }

    void                    SetErrorMode( bool err ) { m_ErrorMode = err; }

    void                    Retake( guGstResultHandler * rhandler ) { m_Handler = rhandler; }
    guGstResultHandler *    Pass()
    {
        guGstResultHandler * res = m_Handler;
        m_Handler = NULL;
        return res;
    }
};

// struct template for a nicer gstreamer objects unref
template< class GstObjectType = GstObject >
struct guGstPtr
{
    GstObjectType   * ptr;
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
