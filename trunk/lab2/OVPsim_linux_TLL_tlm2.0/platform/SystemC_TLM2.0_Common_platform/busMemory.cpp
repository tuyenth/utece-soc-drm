/**********************************************************************
  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2008 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 3.0 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.
 *********************************************************************/


/* derived from TLM examples memory.cpp by ImperasLtd */

#include "busMemory.h"                             // our header
#include "reporting.h"                            // reporting macros

using namespace  std;

//static const char *filename = "busMemory.cpp"; ///< filename for reporting

SC_HAS_PROCESS(busMemory);
///Constructor
busMemory::busMemory
( sc_core::sc_module_name module_name               // module name
, const char                *memory_socket          // socket name
, Addr                      memory_size             // memory size (bytes)
, unsigned int              memory_width            // memory width (bytes)
, const sc_core::sc_time    accept_delay            // accept delay (SC_TIME)
, const sc_core::sc_time    read_response_delay     // read response delay (SC_TIME)
, const sc_core::sc_time    write_response_delay    // write response delay (SC_TIME)
)
: sc_module               (module_name)             /// init module name
, m_memory_socket         (memory_socket)           /// init socket name
, m_memory_size           (memory_size)             /// init memory size (bytes)
, m_memory_width          (memory_width)            /// init memory width (bytes)
, m_accept_delay          (accept_delay)            /// init accept delay
, m_read_response_delay   (read_response_delay)     /// init read response delay
, m_write_response_delay  (write_response_delay)    /// init write response delay

, m_target_memory
  ( m_read_response_delay         // delay for reads
  , m_write_response_delay        // delay for writes
  , m_memory_size                 // memory size (bytes)
  , m_memory_width                // memory width (bytes)
  )

{

    m_memory_socket.register_b_transport(this, &busMemory::custom_b_transport);
    m_memory_socket.register_transport_dbg(this, &busMemory::debug_transport);
    m_memory_socket.register_get_direct_mem_ptr(this, &busMemory::get_direct_mem_ptr);

}

//==============================================================================
//  b_transport implementation calls from initiators
//
//=============================================================================
void
busMemory::custom_b_transport( tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time)
{
    sc_core::sc_time  mem_op_time;
    m_target_memory.operation(payload, mem_op_time);
    return;
}

unsigned int  busMemory::debug_transport (tlm::tlm_generic_payload &payload)
{
    sc_core::sc_time  mem_op_time;
    m_target_memory.operation(payload, mem_op_time);
    return 0;
}


bool busMemory::get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi_data)
{
    Addr address = trans.get_address();
    if (address > m_memory_size) {
        // should not happen
        cerr << "busMemory::get_direct_mem_ptr: address overflow";
        return False;
    }

    // if (m_invalidate) m_invalidate_dmi_event.notify(m_invalidate_dmi_time);
    dmi_data.allow_read_write();
    dmi_data.set_start_address(0x0);
    dmi_data.set_end_address(m_memory_size);

    unsigned char *ptr = m_target_memory.get_mem_ptr();
    dmi_data.set_dmi_ptr(ptr);
    //dmi_data.set_read_latency(sc_core::sc_time(100, sc_core::SC_NS));
    //dmi_data.set_write_latency(sc_core::sc_time(10, sc_core::SC_NS));
    return true;
}





