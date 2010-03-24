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

#ifndef __OVP_BUSMEMORY_H__
#define __OVP_BUSMEMORY_H__

#include "tlm.h"                                // TLM headers
#include "simpleMemory.h"
#include "tlm_utils/simple_target_socket.h"
#include "icm/icmCpuManager.hpp"

using namespace icmCpuManager;    // for Addr type

class busMemory
:     public sc_core::sc_module           	/// inherit from SC module base clase

{
// Member Methods =====================================================

  public:

  busMemory
  ( sc_core::sc_module_name   module_name              ///< SC module name
  , const char               *memory_socket            ///< socket name
  , Addr                      memory_size              ///< memory size (bytes)
  , unsigned int              memory_width             ///< memory width (bytes)
  , const sc_core::sc_time    accept_delay         =sc_core::sc_time(0,sc_core::SC_NS)  ///< accept delay (SC_TIME, SC_NS)
  , const sc_core::sc_time    read_response_delay  =sc_core::sc_time(0,sc_core::SC_NS)  ///< read response delay (SC_TIME, SC_NS)
  , const sc_core::sc_time    write_response_delay =sc_core::sc_time(0,sc_core::SC_NS)  ///< write response delay (SC_TIME, SC_NS)
  );

  private:
    void         custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);
    unsigned int debug_transport   (tlm::tlm_generic_payload &payload);
    bool         get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi&  dmi_data);

// Member Variables ===================================================

  public:

  tlm_utils::simple_target_socket<busMemory>  m_memory_socket; ///<  busMemory socket
  memory *getMemory() { return &m_target_memory; }

  private:

        Addr                m_memory_size;          ///< memory size (bytes)
        unsigned int        m_memory_width;         ///< word size (bytes)
  const sc_core::sc_time    m_accept_delay;         ///< accept delay
  const sc_core::sc_time    m_read_response_delay;  ///< read response delay
  const sc_core::sc_time    m_write_response_delay; ///< write response delays
        bool                m_trans_dbg_prev_warning;
        memory              m_target_memory;
};

#endif /* __BUSMEMORY_H__ */
