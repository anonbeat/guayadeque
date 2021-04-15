
#ifndef __GSTPIPELINEACTUATOR_H__
#define __GSTPIPELINEACTUATOR_H__

#include "Utils.h"
#include "GstTypes.h"
#include "GstUtils.h"

namespace Guayadeque {

/*

Pipeline actuator class to support adding & removing pipeline elements on the fly 

*/

class guGstPipelineActuator
{
private:
    guGstElementsChain              m_Chain;
    guGstResultHandler              * m_ResultHandler;

    guGstPipelineActuator() { SetHandler( NULL ); }

public:
    guGstPipelineActuator( GstElement *element );
    guGstPipelineActuator( guGstElementsChain chain );
    ~guGstPipelineActuator() { guLogDebug( "~guGstPipelineActuator" ); }

    void SetHandler( guGstResultHandler *rhandler = NULL ) { m_ResultHandler = rhandler; }

    bool Enable( GstElement *element );
    bool Enable( int element_nr ) { return Enable( m_Chain[ element_nr ] ); }
    bool Enable() { return Enable(0); }

    bool Disable( GstElement *element );
    bool Disable( int element_nr ) { return Disable( m_Chain[ element_nr ] ); }
    bool Disable() { return Disable(0); }

    bool Toggle( GstElement *element );
    bool Toggle( int element_nr ) { return Toggle( m_Chain[ element_nr ] ); }
    bool Toggle()  { return Toggle(0); }

};

} // namespace Guayadeque

#endif
// __GSTPIPELINEACTUATOR_H__
