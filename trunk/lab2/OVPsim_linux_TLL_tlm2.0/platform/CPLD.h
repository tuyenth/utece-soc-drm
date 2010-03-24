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

/* Derived from DecodeBus by A. Gerstlauer */

#ifndef __CPLD_H__
#define __CPLD_H__

#include "tlm.h"

#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"

template <Addr BASE_ADDR, int NR_OF_TARGETS>
class CPLD : public sc_core::sc_module
{

private:
  typedef struct rangeS {
    Addr       lowAddress;
    Addr       highAddress;
    bool         set;
  } range;

  range decodes[NR_OF_TARGETS];

  unsigned int m_id;

public:
  typedef tlm::tlm_generic_payload                 transaction_type;
  typedef tlm::tlm_phase                           phase_type;
  typedef tlm::tlm_sync_enum                       sync_enum_type;
  typedef tlm_utils::simple_target_socket<CPLD>    target_socket_type;
  typedef tlm_utils::simple_initiator_socket_tagged<CPLD> initiator_socket_type;

  target_socket_type target_socket;
  initiator_socket_type initiator_socket[NR_OF_TARGETS];

public:
  SC_HAS_PROCESS(CPLD);

  CPLD(sc_core::sc_module_name name) : sc_core::sc_module(name)
  {
      target_socket.register_b_transport(this, &CPLD::initiatorBTransport);
      target_socket.register_transport_dbg(this, &CPLD::transportDebug);
      target_socket.register_get_direct_mem_ptr(this, &CPLD::getDMIPointer);

      for (int i = 0; i < NR_OF_TARGETS; ++i) {
          initiator_socket[i].register_invalidate_direct_mem_ptr(this, &CPLD::invalidateDMIPointers, i);
          decodes[i].set = false;
      }
  }

  void setDecode(int portId, Addr lowAddress, Addr highAddress) {
      assert(portId < NR_OF_TARGETS);
      if (decodes[portId].set == false ) {
          decodes[portId].set         = true;
          decodes[portId].lowAddress  = lowAddress;
          decodes[portId].highAddress = highAddress;
      } else {
          cerr << "Port " << portId << " has already been set" << endl;
          assert(0);
      }
  }

 private:

  int getPortId(const Addr& address)
  {
      int i;
      for(i = 0; i < NR_OF_TARGETS; i++) {
          if (decodes[i].set && address >= decodes[i].lowAddress && address <= decodes[i].highAddress) {
              return i;
          }
      }
      cerr << hex << "Decode of 0x" << address << " failed" << endl;
      assert(0); // Oops, no match
      return -1;
  }

  Addr getAddressOffset(int portId)
  {
      return decodes[portId].lowAddress;
  }

  Addr getAddressMask(int portId, Addr base)
  {
      return base - getAddressOffset(portId);
  }


  //
  // interface methods
  //

  //
  // LT protocol
  // - forward each call to the target/initiator
  //
  void initiatorBTransport(transaction_type& trans,
                           sc_core::sc_time& t)
  {
    initiator_socket_type* decodeSocket;
    Addr orig        = trans.get_address() + BASE_ADDR;
    int  portId      = getPortId(orig);
    assert(portId < NR_OF_TARGETS);

    cout << hex << "CPLD transport 0x" << orig << endl;

    if(portId >= 0) {
        decodeSocket     = &initiator_socket[portId];
        trans.set_address(orig);
        (*decodeSocket)->b_transport(trans, t);
    }
  }

  unsigned int transportDebug(transaction_type& trans)
  {
    initiator_socket_type* decodeSocket;
    Addr orig        = trans.get_address() + BASE_ADDR;
    int  portId      = getPortId(orig);
    assert(portId < NR_OF_TARGETS);

    cout << hex << "CPLD transport debug 0x" << orig << endl;

    decodeSocket     = &initiator_socket[portId];
    trans.set_address(orig);
    return (*decodeSocket)->transport_dbg(trans);
  }

  bool limitRange(int portId, Addr& low, Addr& high)
  {
      if (low < decodes[portId].lowAddress)
          low = decodes[portId].lowAddress;
      if (high > decodes[portId].highAddress)
          high = decodes[portId].highAddress;

    return true;
  }

  void adjustRange(int portId, Addr& low, Addr& high)
  {
      Addr portLo    = decodes[portId].lowAddress;
      Addr portHi    = decodes[portId].highAddress;
      Addr portRange = portHi - portLo;
      Addr range     = high - low;
      Addr minRange  = (range < portRange) ? range : portRange;
      low  += portLo;
      high  = portLo + minRange;
  }

  bool getDMIPointer(transaction_type& trans,
                     tlm::tlm_dmi&  dmi_data)
  {
    Addr address = trans.get_address() + BASE_ADDR;

    int portId = getPortId(address);
    assert(portId < NR_OF_TARGETS);
    initiator_socket_type* decodeSocket = &initiator_socket[portId];
    trans.set_address(address);

    bool result =
      (*decodeSocket)->get_direct_mem_ptr(trans, dmi_data);

    if (result)
    {
      // Range must contain address
      assert(dmi_data.get_start_address() <= address);
      assert(dmi_data.get_end_address() >= address);
    }

    // Should always succeed
    Addr start, end;
    start = dmi_data.get_start_address();
    end = dmi_data.get_end_address();

    adjustRange(portId, start, end);

    dmi_data.set_start_address(start);
    dmi_data.set_end_address(end);

    return result;
  }

  void invalidateDMIPointers(int port_id,
                             Addr start_range,
                             Addr end_range)
  {
    if (!limitRange(port_id, start_range, end_range)) {
      // Range does not fall into address range of target
      return;
    }

    (target_socket)->invalidate_direct_mem_ptr(start_range, end_range);
  }

};

#endif
