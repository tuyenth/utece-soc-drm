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

#ifndef _ICM_PERIPHERAL_H__
#define _ICM_PERIPHERAL_H__

#include "tlm.h"                              	// TLM headers
#include "tlm_utils/simple_initiator_socket.h"  // equivalent of OVP Bus Master Port
#include "tlm_utils/simple_target_socket.h"  // equivalent of OVP Bus Master Port

#include <icm/icmCpuManager.hpp>

using namespace icmCpuManager;
using namespace std;
using namespace sc_core;

// forward ref to peripheral
class icmPeripheral;


/// Use one of these for each peripheral slave port.

class icmSlavePort
{
public:

    /// The constructor.
    /// @param pse      Pointer to the peripheral object.
    /// @param name     Name of the slave port in the OVP model.
    /// @param bytes    Number of bytes decoded by this slave port.

    icmSlavePort(icmPeripheral *pse, const char *name, Addr bytes);

    /// Destructor (not usually called explicitly).
    ~icmSlavePort() {}

    /// Target port for incoming data access.
    /// This name (m_socket) must be used in the platform to bind this port.
    tlm_utils::simple_target_socket<icmSlavePort> m_socket;

private:
    Uns32         m_addrBits;
    icmBusObject  m_bus;        // OVP address space for this port
    Addr          m_bytes;      // size of port

    // registered as callbacks on the port
    void b_transport( tlm::tlm_generic_payload &payload, sc_time &delay);
    unsigned int  debug_transport(tlm::tlm_generic_payload &payload);
};


/// Use one of these for each peripheral master port.

class icmMasterPort
{
private:
    Uns32                    m_addrBits;
    icmBusObject             m_bus;        // OVP address space for this port

    sc_time                  m_zero_delay; // note: is const but cannot be declared as such
    tlm::tlm_generic_payload m_trans;      // transactions cannot be deferred
                                           // so only one of these is needed.

    // Used by each bus master operation
    Uns32 readUpcall (Addr addr, Uns32 mask, Uns32 bytes);
    void  writeUpcall(Addr addr, Uns32 mask, Uns32 wdata, Uns32 bytes);

    // Transaction method
    void doTrans( tlm::tlm_generic_payload &trans );

    // Static callback
    static void read(
          Addr          address,
          Uns32         bytes,
          void         *value,
          void         *userData
    );

    static void write(
          Addr          address,
          Uns32         bytes,
          const void   *value,
          void         *userData
    );

public:

    /// The constructor.
    /// @param pse      Pointer to PSE.
    /// @param name     Namer of bus port as appears in OVP model.
    /// @param addrBits Number of address bits supported by this port.

    icmMasterPort(icmPeripheral *pse, const char *name, Uns32 addrBits);

    /// Destructor (not usually called explicitly).
    ~icmMasterPort() {}

    /// The TLM initiator socket. This name  must be used in the
    /// binding in the platform.
   tlm_utils::simple_initiator_socket<icmMasterPort> m_socket;
};


/// Use one of these for each output net port

class icmOutputNetPort : public tlm::tlm_analysis_port<int>
{
public:

    /// The constructor.
    /// @param pse       Pointer to PSE.
    /// @param portName  Name of output net port as appears in OVP model.
    icmOutputNetPort(icmPeripheral *pse, const char *portName);

private:
    icmNetObject m_net;

    // called by a change on the net
    static void update(void *user, Uns32 value);
};

/// Use one of these for each peripheral input net port.
/// It connects a TLM net to an OVP peripheral net input.

class icmInputNetPort : public sc_module, public tlm::tlm_analysis_if<int>
{
private:
    icmNetObject m_net;
    virtual void write(const int &value);

public:
    /// The constructor.
    /// @param name      Name of this object as it appears in the OVP model.
    /// @param pse       Pointer to the PSE object.

    icmInputNetPort(icmPeripheral *pse, sc_module_name name);

};


/// This is the generic OVP Peripheral.
/// It instances one OVP Peripheral model.

class icmPeripheral : public sc_module, public icmPseObject
{

public:

    /// The peripheral constructor.
    /// @param name             Instance name. Must be unique in the platform.
    /// @param model            Path to model PSE object file. See icmGetVlnvString().
    /// @param semihostLibrary  Path to intercept object (.dll or .so) if required.
    /// @param initialAttrs     (optional) List of user-defined attributes passed to the model.
    icmPeripheral(
      sc_module_name          name
    , const char             *model
    , const char             *semihostLibrary
    , icmAttrListObject      *initialAttrs = 0
    );
    const char*          instanceName;
};

#endif
