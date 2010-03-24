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

#include "reporting.h"
#include "icmCpu.h"
#include "icmPlatform.h"

/// Constructor
SC_HAS_PROCESS(icmCpu);

icmCpu::icmCpu(
  sc_module_name          name
, const unsigned int      ID
, const char             *type
, const char             *model
, const char             *attrTable
, const char             *semihostLibrary
, icmAttrListObject      *initialAttrs
, icmNewProcAttrs         procAttrs
, Uns32                   addressBits
, bool                    dmi
)
: sc_module (name)          /// instance name
, icmProcessorObject(
    name,
    type,
    ID,
    0,
    addressBits,
    model,
    attrTable,
    procAttrs | ICM_ATTR_SYSTEMC | ICM_ATTR_RELAXED_SCHED,
    initialAttrs,
    semihostLibrary,
    semihostLibrary ? "modelAttrs" : 0
  )
, m_socket            ("data_initiator")
, m_addrBits          (addressBits)
, m_instPerSecond     (100000000)                     // 100MHz
, m_cpu_bus           ("cpu_bus", name, m_addrBits)
, m_tlm_bus           ("tlm_bus", name, m_addrBits)
, m_use_dmi           (dmi)
, m_zero_delay        (SC_ZERO_TIME)
, m_name              (name)
{
    SC_THREAD(icmCpuThread);

    // note that OVPsim PSEs are run on the first thread to be run.
    set_stack_size(0x50000);

    // processor to local bus
    icmProcessorObject::connect(m_cpu_bus,m_cpu_bus);

    // all of TLM bus gets a callback to TLM
    icmMemCallback *cb = new icmMemCallback(
        (icmMemCallback::readCallback)&icmCpu::read,
        (icmMemCallback::writeCallback)&icmCpu::write,
        (void*)this // this cpu
    );
    m_tlm_bus.mapExternalMemory(name, ICM_PRIV_RWX, 0, maxValue(m_addrBits), cb);

    // by default, all address space starts mapped to TLM
    m_cpu_bus.bridge(m_tlm_bus, 0, maxValue(m_addrBits), 0);
}

void icmCpu::icmCpuThread (void)
{
    icmTLMPlatform *platform = icmTLMPlatform::Instance();
    while(!m_finished) {
        sc_time quantum = platform->quantum();
        Uns64  inst     = (Uns64)(quantum.to_seconds() * m_instPerSecond);
        icmStopReason r = simulate(inst);
        switch(r) {
            case ICM_SR_FINISH:
                m_finished = 1; // signal 'done' to other cpus.
                // fall through
            case ICM_SR_EXIT:
                return;
            case ICM_SR_HALT:
                // restart on a halt
            default:
                break;
        }
        wait(quantum);
    }
}

// Read and write methods construct a TLM transaction for each memory access.
// Note that an OVP processor does not model byte lanes and byte lane enables.

// static read callback
void icmCpu::read(
      Addr          address,
      Uns32         bytes,
      void         *value,
      void         *userData,
      Bool          simulate
) {
    icmCpu *classPtr = (icmCpu *)userData;
    Uns32 mask = 0;

    while (bytes) {
        Uns32 rd = (bytes > 4) ? 4 : bytes;
        if (simulate && rd == 3) {
            cerr << "Processor model attempting a 3-byte read" << endl;
        }
        switch(rd) {
            case 1: mask = 0x000000FF;          break;
            case 2: mask = 0x0000FFFF;          break;
            case 3: mask = 0x0000FFFF; rd=2;    break; // 2 then 1
            case 4: mask = 0xFFFFFFFF;          break;
        }
        unsigned long int v = classPtr->readUpcall(address, mask, rd, simulate);
        switch(rd) {
            case 1: *(Uns8*) value = (Uns8)v;         break; // loss of data
            case 2: *(Uns16*)value = (Uns16)v;        break; // loss of data
            case 4: *(Uns32*)value = v;               break;
        }
        value = (void*) ((int)value + rd);
        address += rd;
        bytes   -= rd;
    }
}

// static write callback
void icmCpu::write(
      Addr          address,
      Uns32         bytes,
      const void   *value,
      void         *userData,
      Bool          simulate
) {
    icmCpu *classPtr = (icmCpu *)userData;
    while (bytes) {
        Uns32 wr = (bytes > 4) ? 4 : bytes;
        Uns32 v;
        Uns32 mask = 0;
        if (simulate && wr == 3) {
            cerr << "Processor model attempting a 3-byte write" << endl;
        }
        switch(wr) {
            case 1: v = *(const Uns8*) value;        mask = 0x000000FF; break;
            case 2: v = *(const Uns16*)value;        mask = 0x0000FFFF; break;
            case 3: v = *(const Uns16*)value; wr=2;  mask = 0x0000FFFF; break;
            case 4: v = *(const Uns32*)value;        mask = 0xFFFFFFFF; break;
        }
        classPtr->writeUpcall(address, mask, (Uns32)v, wr, simulate );
        value = (const void*) ((int)value + wr);
        address += wr;
        bytes -= wr;
    }
}

// try for dmi
void icmCpu::tryDmi(Addr addr) {
    m_trans.set_address( addr );
    tlm::tlm_dmi tmp;
    bool y = m_socket->get_direct_mem_ptr(m_trans, tmp);
    if (y && tmp.is_read_write_allowed()) {
        Addr  lo  = tmp.get_start_address();
        Addr  hi  = tmp.get_end_address();
        void *ptr = tmp.get_dmi_ptr();

        char num[32];
        sprintf(num, "%d", m_nextName++);
        string n = m_name + "_dmi_" + num;
        mapNativeMemory( n.c_str(), (unsigned char*)ptr, lo, hi);
    }
}

//! This is the entry point used by ::staticReadUpcall(). Constructs a
//! Generic transactional payload for the read, then passes it to the target.

//! @param addr     The address for the write
//! @param mask     The byte enable mask for the write
//! @param wdata    The data to be written (matching the mask)
//! @param simulate If true, this access should be simulated, if not it should be a back-door access

Uns32 icmCpu::readUpcall(Addr addr, Uns32 mask, Uns32 bytes, Bool simulate)
{
    Uns32        rdata;		// For the result

    m_trans.set_read();
    m_trans.set_address( addr );

    m_trans.set_data_length(bytes);
    m_trans.set_data_ptr( (unsigned char *)&rdata );

    m_trans.set_byte_enable_length( sizeof(mask));
    m_trans.set_byte_enable_ptr( (unsigned char *)&mask );

    m_trans.set_streaming_width(sizeof(rdata));
    m_trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    m_trans.set_dmi_allowed(false);

    // Transport. Then return the result
    if (simulate) {
        m_socket->b_transport( m_trans, m_zero_delay );
        if (m_trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
//TODO: Temp removed for ArmIntegratorCP Platform work
//            cerr << hex << "Read transaction at 0x" << addr << " did not complete correctly" << endl;
            return 0;
        }
        if (m_use_dmi && m_trans.is_dmi_allowed()) {
            m_trans.set_read();
            tryDmi(addr);
        }
    } else {
        m_socket->transport_dbg(m_trans);
        // ignore response in debug mode
    }
    return  rdata;
}


//! This is the entry point used by ::staticWriteUpcall(). Constructs a
//! Generic transactional payload for the write, then passes the target.

//! @param addr     The address for the write
//! @param mask     The byte enable mask for the write
//! @param wdata    The data to be written (matching the mask)
//! @param simulate If true, this access should be simulated, if not it should be a back-door access

void icmCpu::writeUpcall(Addr addr, Uns32 mask, Uns32 wdata, Uns32 bytes, Bool simulate)
{
    m_trans.set_write();
    m_trans.set_address( addr );

    m_trans.set_data_length(bytes);
    m_trans.set_data_ptr( (unsigned char *)&wdata );

    m_trans.set_byte_enable_length(sizeof(mask));
    m_trans.set_byte_enable_ptr( (unsigned char *)&mask );

    m_trans.set_streaming_width(sizeof(wdata));
    m_trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    m_trans.set_dmi_allowed(false);

    // Transport.
    if (simulate) {
        m_socket->b_transport( m_trans, m_zero_delay );
        if (m_trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
//TODO: Temp removed for ArmIntegratorCP Platform work
//            cerr << hex << "Write transaction at 0x" << addr << " did not complete correctly" << endl;
            return;
        }
        if (m_use_dmi && m_trans.is_dmi_allowed()) {
            m_trans.set_write();
            tryDmi(addr);
        }
    } else {
        m_socket->transport_dbg(m_trans);
        // ignore response in debug mode
    }
}

  // This region will be local memory in OVP
void icmCpu::mapLocalMemory(const char *name, Addr low, Addr high) {
    icmBusObject *newBus = new icmBusObject(name, m_addrBits);
    Addr bytesM1 = high - low;
    newBus->mapLocalMemory( name, ICM_PRIV_RWX, 0, bytesM1);

    m_cpu_bus.unbridge(low, high);
    m_cpu_bus.bridge(*newBus, low, high, 0);
}

  // This region will be native memory
void icmCpu::mapNativeMemory(const char *name, unsigned char *ptr, Addr low, Addr high) {
    //cout << hex << "mapNativeMemory " << name << " l:" << low << " h:" << high << endl;

    icmBusObject *newBus = new icmBusObject(name, m_addrBits);
    Addr bytesM1 = high - low;
    newBus->mapNativeMemory( name, (void*)ptr, 0, bytesM1);

    m_cpu_bus.unbridge(low, high);
    m_cpu_bus.bridge(*newBus, low, high, 0);
}

// Remap this region to TLM. what happens to what was there before?
void icmCpu::mapTLMMemory(const char *name, Addr low, Addr high) {

    m_cpu_bus.unbridge(low, high);
    m_cpu_bus.bridge(m_tlm_bus, low, high, low);
}

void icmCpu::connectInterrupts(icmCpuInterrupt *ints[], const char *nameStem, Uns32 low, Uns32 high) {

    Uns32 i;
    for(i=low; i <= high; i++) {
        char tmp[128];
        sprintf(tmp, "%s%d", nameStem, i);
        ints[i] = new icmCpuInterrupt(tmp, this);
    }
}

void icmCpu::connectInterrupt(icmCpuInterrupt **intr, const char *name) {

    *intr = new icmCpuInterrupt(name, this);
}

bool icmCpu::m_finished = 0;

//
////////////////////////// Interrupt input //////////////////////////////////////
//
icmCpuInterrupt::icmCpuInterrupt(sc_module_name name, icmProcessorObject *processor)
: m_net(0)  // create a net with anon name
{
    // connect net to netport. Writing to the net will send an input into the model.
    processor->connect(m_net, (const char*)name, ICM_INPUT);
}

// write

void icmCpuInterrupt::write(const int &value)
{
    // cout << "icmCpuInterrupt::write" << value << endl;
    m_net.write(value);
}

