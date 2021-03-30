
#ifndef __GSTPIPELINEBUILDER_H__
#define __GSTPIPELINEBUILDER_H__

#include <gst/gst.h>
#include <stack>

#include "Utils.h"

namespace Guayadeque {

struct guGstPipelineElement
{
    GstElement *    element;
    GstElement **   element_ref;
};

class guGstPipelineBuilder
{
private:
    std::stack<guGstPipelineElement>    m_ElementStack;
    bool                    m_CanPlay;
    bool                    m_Cleanup;
    GstElement *            m_Bin;
    GstElement *            m_Last;



public:
    guGstPipelineBuilder( const char * bin_name, GstElement ** element_ref );
    ~guGstPipelineBuilder();

    static bool IsValidElement( GstElement * element );
    void PushElement( GstElement * element, GstElement ** element_ref );

    bool CanPlay() { return m_CanPlay; }
    void SetCleanup( const bool value ) { m_Cleanup = value; }

    bool Link( GstElement * element );

    GstElement * Add( const char * factoryname, const char * name, GstElement ** element_ref = NULL, const bool link = true )
    {
        return Add( factoryname, name, element_ref, link, "name", name );
    }
    
    template<typename... PropArgs>
    GstElement * Add( const char * factoryname, const char * name, GstElement ** element_ref, const bool link, PropArgs... properties )
    {
        guLogDebug( "guGstPipelineBuilder::Add %s <%s> @ [%p]", name, factoryname, element_ref );
        
        GstElement * ge = gst_element_factory_make( factoryname, name );
        if( IsValidElement( ge ) )
        {
            PushElement( ge, element_ref );
            g_object_set( G_OBJECT( ge ), properties... , NULL );

            if( element_ref != NULL )
                *element_ref = ge;

            if( !link )
                return ge;

            if( Link( ge ) )
            {
                m_ElementStack.pop();
                return ge;
        }
        }
        else
            guLogError( "Unable to create gstreamer element: %s <%s>", name, factoryname );

        if( element_ref != NULL )
            *element_ref = NULL;

        m_CanPlay = false;
        return NULL;

    }

};

}

#endif
// -------------------------------------------------------------------------------- //