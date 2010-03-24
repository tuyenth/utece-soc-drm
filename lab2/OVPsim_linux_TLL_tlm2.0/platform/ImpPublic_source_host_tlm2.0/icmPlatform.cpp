/*
 *
 * Copyright (c) 2005-2010 Imperas Software Ltd., www.imperas.com
 *
 * The contents of this file are provided under the Software License
 * Agreement that you accepted before downloading this file.
 *
 * This source forms part of the Software and can be used for educational,
 * training, and demonstration purposes but cannot be used for derivative
 * works except in cases where the derivative works require OVP technology
 * to run.
 *
 * For open source models released under licenses that you can use for
 * derivative works, please visit www.OVPworld.org or www.imperas.com
 * for the location of the open source models.
 *
 */

#include "icmPlatform.h"

icmTLMPlatform *icmTLMPlatform::m_singleton;

icmTLMPlatform::icmTLMPlatform(
    sc_module_name module_name,
    icmInitAttrs   simAttrs,
    const char    *dbgHostName,
    Uns32          dbgPort
):
    sc_module(module_name),
    icmPlatform(module_name, simAttrs, dbgHostName, dbgPort),
    m_quantum(1, SC_MS)
{

    // register the trace callback
    sc_get_curr_simcontext()->add_trace_file(&m_timeAdvance);

    // register the time callback
    m_timeAdvance.setCallback(advance, this);

    if (m_singleton) {
        cerr << "Trying to create more than one platform" << endl;
        exit(1);
    }
    // set global pointer to platform
    m_singleton = this;
}

icmTLMPlatform::~icmTLMPlatform() {

}
