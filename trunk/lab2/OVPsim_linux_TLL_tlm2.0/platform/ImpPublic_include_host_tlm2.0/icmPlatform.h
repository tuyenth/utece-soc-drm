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

/*
 * icmPlatform contains the ICM platform object and other mechanisms
 * need to interface TLM2.0 to ICM.
 */

#ifndef ICM_TLM_PLATFORM_H
#define ICM_TLM_PLATFORM_H

#include "icm/icmCpuManager.hpp"
#include "tlm.h"
#include "advanceTime.h"

using namespace icmCpuManager;
using namespace sc_core;

/// The Platform object.
/// Instantiating this is optional; the underlying constructors
/// are called anyway by calls to other constructors.
/// If you wish to set global simulation attributes you should construct
/// one of these before all other OVP objects.

class icmTLMPlatform : public sc_module, public icmPlatform {

private:

    // registers the time advance callback.
    time_advance  m_timeAdvance;

    // period of one quantum for all processors
    sc_time       m_quantum;

    static void advance(icmTime t, void *user) {
        icmTLMPlatform *p = (icmTLMPlatform*) user;
        double dt = t;

        // use the double, not the long double.
        p->advanceTime(dt);
    }

    // pointer to the (static) platform
    static icmTLMPlatform *m_singleton;

    // defining this virtual fn from sc_module causes our startup to be called
    // before simulation starts
    void start_of_simulation(void) { simulationStarting(); }

    // defining this virtual fn from sc_module causes our shutdown to be called
    // before SystemC starts calling destructors.
    void end_of_simulation(void) { simulationEnding(); }

public:

    /// The constructor. Ensure this is called before any other ICM constructors.
    /// It must be called only once.
    /// @param module_name     A unique name for the platform.
    /// @param simAttrs        (optional) global simulation attributes.
    /// @param dbgHostName     (optional) Name of host to connect to a debug port.
    /// @param dbgPort         (optional) Debugger port number.

    icmTLMPlatform(
        sc_module_name module_name,
        icmInitAttrs   simAttrs    = ICM_INIT_DEFAULT,   // default to no global options
        const char    *dbgHostName = NULL,               // default to no gdb interface
        Uns32          dbgPort     = 0
    );

    /// Destructor (not usually called explicitly).

    ~icmTLMPlatform();

    /// Override the default global quantum (length of each time-slice).
    /// Should be called from the platform constructor (not during simulation).

    void    quantum(sc_time q) { m_quantum = q; }

    /// Return the current global quantum.
    /// @return current quantum.

    sc_time quantum() { return m_quantum; }

    /// Return the ICM platform object.

    static icmTLMPlatform *Instance() { return m_singleton; }
};

#endif
