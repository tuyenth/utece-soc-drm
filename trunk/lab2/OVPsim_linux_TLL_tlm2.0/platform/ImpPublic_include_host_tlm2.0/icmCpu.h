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

#ifndef __ICM_CPU_H__
#define __ICM_CPU_H__

#include "tlm.h"                              	// TLM headers
#include "tlm_utils/simple_initiator_socket.h"  // equivalent of OVP Bus Master Port

#include "icm/icmCpuManager.hpp"

using namespace icmCpuManager;
using namespace std;
using namespace sc_core;

/// The interrupt object class.
/// Use this to connect a SystemC sc_in to an OVP processor interrupt.

class icmCpuInterrupt: public sc_module, public tlm::tlm_analysis_if<int>
{

public:

    /// icmCpuInterrupt Constructor.
    /// @param name      sc_module_name (must be unique within this processor model).
    /// @param processor pointer to icmProcessorObject.
    icmCpuInterrupt(sc_module_name name, icmProcessorObject *processor);

    virtual void write(const int &value);

private:
    icmNetObject m_net;

};

/// Generic TLM/OVP processor.
/// It instances one OVP Processor model which can be connected to a TLM initiator socket.
/// The memory map of the processor can be set using the mapXxxMemory methods.
/// Note that this model connects both the INSTRUCTION and DATA memory spaces
/// of the OVP model to a single TLM2.0 bus.

class icmCpu : public sc_module, public icmProcessorObject
{

public:

    /// icmCpu constructor
    /// Constructs an instance of an OVP CPU model.
    /// @param name            Unique instance name.
    /// @param ID              Unique processor identifier.
    /// @param type            The type of the processor. Should match the 'N'
    ///                        in the VLNV.
    /// @param model           Path to the model files (dll or shared object).
    /// @param attrTable       Name of the attributes table in the model file.
    ///                        Note that some processor model files can contain
    ///                        more than one table, each supporting a different model.
    /// @param semihostLibrary (optional) Path to semihost library (dll or shared object).
    /// @param initialAttrs    (optional) list of user-defined attributes.
    ///                        Used to configure options in the model.
    /// @param procAttrs       (optional) processor simulator attributes.
    ///                        Used to control how the simulator treats this model.
    /// @param addressBits     Number of bits used by code and data address buses.
    /// @param dmi             (defaults to true) Use DMI to bypass TML where possible.

    icmCpu(
    sc_module_name     name
  , const unsigned int ID
  , const char        *type
  , const char        *model
  , const char        *attrTable
  , const char        *semihostLibrary
  , icmAttrListObject *initialAttrs = 0
  , icmNewProcAttrs   procAttrs     = ICM_ATTR_DEFAULT
  , Uns32             addressBits   = 32
  , bool              dmi           = true
  );

   /// Initiator port for data accesses.
   /// This must be bound in the platform constructor.
   tlm_utils::simple_initiator_socket<icmCpu> m_socket;

private:
    static bool         m_finished;       // set if any cpu finishes.
    const unsigned int  m_addrBits;       // 32 for now, but how do we set this?
    Uns64               m_instPerSecond;  // instructions to execute per second
    icmBusObject        m_cpu_bus;        // local address space for this processor
    icmBusObject        m_tlm_bus;        // region mapped to TLM
    bool                m_use_dmi;        // try dmi where possible.
    sc_time             m_zero_delay;     // note: is const but cannot be declared as such
    int                 m_nextName;       // for constructing unique names.
    string              m_name;

    tlm::tlm_generic_payload m_trans;     /// transactions cannot be deferred, so only one needed

    // I/O up-calls from OVP, with a common synchronized transport
    // utility. These are not changed in later derived classes.

    Uns32 readUpcall (Addr addr, Uns32 mask, Uns32 bytes, bool simulate);
    void  writeUpcall(Addr addr, Uns32 mask, Uns32 wdata, Uns32 bytes, bool simulate);

    // see if DMI is available and use it to map the memory if possible.
    void tryDmi(Addr addr);

    // main thread
    void icmCpuThread (void);

    // convert bits to mask
    Addr maxValue(Uns32 bits) { return (bits >= 64) ? 0xFFFFFFFFFFFFFFFFll : ((Addr)1 << bits)-1; }

    // static read and write callbacks.
    // These are required to recover the instance pointer from the (void*) userData.

    static void read(
          Addr          address,
          Uns32         bytes,
          void         *value,
          void         *userData,
          Bool          simulate
    );

    static void write(
          Addr          address,
          Uns32         bytes,
          const void   *value,
          void         *userData,
          Bool          simulate
    );

public:
    /// Memory mapping function. Note that any region within the processor's address
    /// space not mapped by one of these functions will default to TLM memory.
    /// Map this region to local OVP memory.
    /// @param name      A unique name for this memory region.
    /// @param low       Lower extent of the region.
    /// @param high      Upper extent of the region.

    void mapLocalMemory(const char *name, Addr low, Addr high);

    /// Map this region to native memory.
    /// @param name      A unique name for this memory region.
    /// @param ptr       Pointer to the native memory.
    /// @param low       Lower extent of the region.
    /// @param high      Upper extent of the region.
    void mapNativeMemory(const char *name, unsigned char *ptr, Addr low, Addr high);

    /// Map this region to TLM memory.
    /// @param name      A unique name for this memory region.
    /// @param low       Lower extent of the region.
    /// @param high      Upper extent of the region.
    void mapTLMMemory(const char *name, Addr low, Addr high);

    /// Override the number of instructions to run per second.
    /// @param ips       Instructions per second.
    void setIPS    (Uns64 ips)   { m_instPerSecond = ips; }

    /// Install an array of interrupts
    /// @param ints      Pointer to an array of high+1  icmCpuInterrupts.
    /// @param nameStem  Stem of the name of the interrupt ports in the OVP model
    ///                  The numbers low...high are appended to nameStem.
    /// @param low       First interupt number.
    /// @param high      Last interrupt number.
    void connectInterrupts(icmCpuInterrupt *ints[], const char *nameStem, Uns32 low, Uns32 high);

    /// Install one interrupt.
    /// @param intr      Pointer to interrupt object.
    /// @param name      Name of interrupt object as it appears in the OVP model.
    void connectInterrupt(icmCpuInterrupt **intr, const char *name);
};

#endif
