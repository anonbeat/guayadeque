
#ifndef __GSTPIPELINEBUILDER_H__
#define __GSTPIPELINEBUILDER_H__

#include <gst/gst.h>
#include <stack>

#include "GstTypes.h"
#include "Utils.h"

namespace Guayadeque {

/*

Conditional pointer unref'er + chain builder to build & link GstElements in GstBin.
    unref condition is enabled by default, call SetCleanup(false) to disable.
    pointer state under element_ref gets actualized by class methods.

*/

class guGstPipelineBuilder
{
private:
    
    guGstElementsChain                      m_ElementChain;
    std::stack<guGstPipelineElementPack>    m_UnrefElementStack;

    bool    m_CanPlay;
    bool    m_Cleanup;
    
    GstElement *    m_Bin;
    GstElement *    m_Last;



public:
    guGstPipelineBuilder( const char * bin_name, GstElement ** element_ref );
    guGstPipelineBuilder( GstElement * the_bin );
    ~guGstPipelineBuilder();

    static bool IsValidElement( GstElement * element );
    void PushElement( GstElement * element, GstElement ** element_ref );

    bool CanPlay() { return m_CanPlay; }
    void SetCleanup( const bool value ) { m_Cleanup = value; }

    bool Link( GstElement * element );

    guGstElementsChain GetChain() { return m_ElementChain; };

    // add element followed by valve in order to eliminate
    // racing condition on pad probes between pluggables
    // when multiple dynamic elements are switched on or off at exactly the same time
    //   this methode probably can be enchanced by using proxy pads instead of valve :)
    GstElement * AddV( const char * factoryname, const char * name, GstElement ** element_ref = NULL, const bool link = true )
    {
        return AddV( factoryname, name, element_ref, link, "name", name );
    }
    
    template<typename... PropArgs>
    GstElement * AddV( const char * factoryname, const char * name, GstElement ** element_ref, const bool link, PropArgs... properties )
    {
        GstElement * res = Add( factoryname, name, element_ref, link, properties... );
        if( GST_IS_ELEMENT(res) )
        {
            std::string v_name = "gpb_valve_" + std::string( name );
            Add( "valve", v_name.c_str() );
            return res;
        }
        return NULL;
    }
    
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
                m_UnrefElementStack.pop();
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

} // namespace Guayadeque

#endif
// -------------------------------------------------------------------------------- //
