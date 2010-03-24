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

// Simple peripheral which decodes 16 addresses and controls two output nets
// by writes to the first address, bits 0 and 1;

#ifndef _OVP_PERIPH_H_
#define _OVP_PERIPH_H_

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "ostream"
#include <icm/icmCpuManager.hpp>

using namespace sc_core;
using namespace tlm;
using namespace std;
using namespace icmCpuManager;

template <Uns32 NR_OF_INTERRUPTS = 1> class periph;

template <Uns32 NR_OF_INTERRUPTS> class periph : public sc_module{

public:
    typedef tlm_utils::simple_target_socket<periph> target_socket_type;
    typedef tlm_generic_payload                     transaction_type;

    tlm_analysis_port<int> intr[NR_OF_INTERRUPTS];

    target_socket_type bus;

private:
    Uns32 m_intState;  // remember the previous state

    void updateInterrupts(Uns32 newData)
    {
        Uns32 i;
        //cout << "updateInterrupts " << newData << " width=" << NR_OF_INTERRUPTS << endl;
        for (i = 0; i < NR_OF_INTERRUPTS; i++) {
            Uns32 mask = 1 << i;

            if ((newData & mask) != (m_intState & mask)) {
                //cout << "    updateInterrupts write int " << i << endl;
                intr[i].write((newData & mask) != 0);
            }
        }
        m_intState = newData;
    }

public:
    // constructor

    periph(sc_core::sc_module_name name)
    : sc_module(name), m_intState(0)
    {
        if (NR_OF_INTERRUPTS > 32) {
            cerr << "periph: No more than 32 interrupts" << endl;
        }
        bus.register_b_transport(this, &periph::initiatorTransport);
        bus.register_transport_dbg(this, &periph::initiatorDebug);
    }

    void initiatorTransport(transaction_type& trans,
                            sc_core::sc_time& t)
    {
        Uns64          addr    = trans.get_address();
        Uns32 *dataPtr = (Uns32*)trans.get_data_ptr();
        Uns32  data    = *dataPtr;
        char *what;
        switch( trans.get_command() ) {
            case TLM_READ_COMMAND :  what = "read"  ;  break;
            case TLM_WRITE_COMMAND:  what = "write" ;  break;
            default               :  what = "what??";  break;
        }
        if (1) {
            cout << hex << "Peripheral " << name() << " "<< what << " addr: " << addr << " data: " << data << endl;
        }

        if (trans.get_command() == TLM_WRITE_COMMAND && addr == 0) {
            updateInterrupts(data);
        }
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    unsigned int initiatorDebug(transaction_type& trans)
    {
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
        return 0;
    }

};

#endif
