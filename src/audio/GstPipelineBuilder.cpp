
#include "GstPipelineBuilder.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guGstPipelineBuilder
// -------------------------------------------------------------------------------- //
bool guGstPipelineBuilder::IsValidElement( GstElement * element )
{
    if( !GST_IS_ELEMENT( element ) )
    {
        if( G_IS_OBJECT( element ) )
            g_object_unref( element );
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
guGstPipelineBuilder::guGstPipelineBuilder( GstElement * the_bin )
{
    guLogDebug( "guGstPipelineBuilder::guGstPipelineBuilder existing bin: %s", GST_ELEMENT_NAME(the_bin) );    
    m_CanPlay = false;
    m_Last = NULL;
    m_Cleanup = true;
    m_Bin = the_bin;
}

// -------------------------------------------------------------------------------- //
guGstPipelineBuilder::guGstPipelineBuilder( const char * bin_name, GstElement ** element_ref )
{
    
    guLogDebug( "guGstPipelineBuilder::guGstPipelineBuilder %s (%p)", bin_name, element_ref );
    m_CanPlay = false;
    m_Last = NULL;
    m_Cleanup = true;
    m_Bin = gst_bin_new( bin_name );

    if( IsValidElement( m_Bin ) )
    {
        PushElement( m_Bin, element_ref );
        m_CanPlay = true;
        if( element_ref != NULL )
            *element_ref = m_Bin;
    }
    else
        guLogError( "Unable to create gstreamer bin" );

}

// -------------------------------------------------------------------------------- //
guGstPipelineBuilder::~guGstPipelineBuilder()
{
    guLogDebug( "guGstPipelineBuilder::~guGstPipelineBuilder" );

    if( !m_Cleanup )
        return;

    while( !m_UnrefElementStack.empty() )
    {
        guGstPipelineElementPack pe = m_UnrefElementStack.top();
        if( IsValidElement( pe.element ) )
        {
            guLogDebug( "guGstPipelineBuilder::~guGstPipelineBuilder unref: %p", pe.element );
            gst_object_unref( pe.element );
        }
        if( pe.element_ref != NULL )
        {
            guLogDebug( "guGstPipelineBuilder::~guGstPipelineBuilder nullptr: %p", pe.element_ref );
            *pe.element_ref = NULL;
        }
        m_UnrefElementStack.pop();
    }
}

// -------------------------------------------------------------------------------- //
void guGstPipelineBuilder::PushElement( GstElement * element, GstElement ** element_ref )
{
    guLogDebug( "guGstPipelineBuilder::PushElement %p (%p)", element, element_ref );
    guGstPipelineElementPack e;
    e.element = element;
    e.element_ref = element_ref;
    m_UnrefElementStack.push( e );
    m_ElementChain.push_back( element );
}

bool guGstPipelineBuilder::Link( GstElement * element )
{
    guLogDebug( "guGstPipelineBuilder::Link %p", element );
    if( gst_bin_add( GST_BIN(m_Bin), element ) )
    {
        if( m_Last == NULL || gst_element_link( m_Last, element ) )
        {
            m_Last = element;
            return true;
        }
        else
        {
                char * this_name = gst_element_get_name( element );
                char * last_name = gst_element_get_name( m_Last );
                guLogError( "Unable to link elements: %s -> %s", last_name, this_name );
                g_free( this_name );
                g_free( last_name );
        }
    }
    else
    {
        char * this_name = gst_element_get_name( element );
        guLogError( "Gstreamer bin did not accept the element: %s", this_name );
        g_free( this_name );
    }

    m_CanPlay = false;
    return false;
}


} // namespace Guayadeque
