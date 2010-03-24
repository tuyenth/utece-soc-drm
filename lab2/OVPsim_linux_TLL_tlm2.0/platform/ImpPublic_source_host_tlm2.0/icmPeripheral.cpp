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

#include "icmPeripheral.h"

#define CONCAT(s1,s2) ((string) s1 + "_" + (string) s2).c_str()

//
// Slave port implementation
//
icmSlavePort::icmSlavePort(icmPeripheral *pse, const char *name, Addr bytes)
: m_socket ("target")
, m_addrBits(32)
, m_bus    ("sp_bus", CONCAT((*pse).instanceName, name), m_addrBits)
, m_bytes  (bytes)
{
    m_socket.register_b_transport  (this, &icmSlavePort::b_transport);
    m_socket.register_transport_dbg(this, &icmSlavePort::debug_transport);
		
	if (bytes > 0)
	{
    	pse->icmCpuManager::icmPseObject::connect(m_bus, name, false, 0, bytes-1);
	}
	else
		pse->icmCpuManager::icmPseObject::connect(m_bus, name, false);
}

// target regular method
void icmSlavePort::b_transport( tlm::tlm_generic_payload &payload, sc_time &delay)
{
    Addr             address   = payload.get_address();     // memory address
    tlm::tlm_command command   = payload.get_command();     // memory command
    unsigned char    *data     = payload.get_data_ptr();    // data pointer
    unsigned  int     length   = payload.get_data_length(); // data length

    bool          ok      = true;

    if (address + length > m_bytes) {
        ok = false;
    }
    if (length > 4) {
        ok = false;   // perhaps we can handle more?
    }
    switch(command) {
        case tlm::TLM_WRITE_COMMAND:
            m_bus.write(address, (void*)data, length);
            break;
        case tlm::TLM_READ_COMMAND:
            m_bus.read(address, (void*)data, length);
            break;
        default:
            ok = false;
    }
    payload.set_response_status(ok ? tlm::TLM_OK_RESPONSE : tlm::TLM_ADDRESS_ERROR_RESPONSE);
}

// target debug method; not supported
unsigned int icmSlavePort::debug_transport(tlm::tlm_generic_payload &payload)
{
    payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    return 0;
}

//
// Bus master port
//
icmMasterPort::icmMasterPort(icmPeripheral *pse, const char *name, Uns32 addrBits)
: m_addrBits(addrBits)
, m_bus("m_bus", CONCAT((*pse).instanceName,name), m_addrBits)
, m_zero_delay(SC_ZERO_TIME)
{
    icmMemCallback *cb = new icmMemCallback(
         (icmMemCallback::readCallback)&icmMasterPort::read,
         (icmMemCallback::writeCallback)&icmMasterPort::write,
         (void*)this // this master port
     );
    m_bus.mapExternalMemory("callback", ICM_PRIV_RW, 0, 0xFFFFFFFF, cb);
    pse->icmPseObject::connect(m_bus, name, true, 0, 0xFFFFFFFF);
}
// Read and write methods construct a TLM transaction for each memory access.
// Note that OVP processor does not model byte lanes and byte lane enables.

// static read callback
void icmMasterPort::read(
      Addr          address,
      Uns32         bytes,
      void         *value,
      void         *userData
) {
    icmMasterPort *classPtr = (icmMasterPort*)userData;
    Uns32 mask = 0;
    while (bytes) {
        Uns32 rd = (bytes > 4) ? 4 : bytes;
        switch(rd) {
            case 1:   mask = 0x000000FF;   break;
            case 2:   mask = 0x0000FFFF;   break;
            case 4:   mask = 0xFFFFFFFF;   break;
        }
        unsigned long int v = classPtr->readUpcall(address, mask, rd);
        switch(rd) {
            case 1:   *(Uns8*) value = (Uns8)v;    break; // loss of data
            case 2:   *(Uns16*)value = (Uns16)v;   break; // loss of data
            case 4:   *(Uns32*)value = v;          break;
        }
        value = (void*) ((int)value + rd);
        address += rd;
        bytes   -= rd;
    }
}

// static write callback
void icmMasterPort::write(
      Addr          address,
      Uns32         bytes,
      const void   *value,
      void         *userData
) {
    icmMasterPort *classPtr = (icmMasterPort*)userData;
    while (bytes) {
        Uns32 wr = (bytes > 4) ? 4 : bytes;
        Uns32 v;
        Uns32 mask = 0;
        switch(wr) {
            case 1:   v = *(const Uns8*) value;   mask = 0x000000FF; break;
            case 2:   v = *(const Uns16*)value;   mask = 0x0000FFFF; break;
            case 4:   v = *(const Uns32*)value;   mask = 0xFFFFFFFF; break;
        }
        classPtr->writeUpcall(address, mask, (Uns32)v, wr);
        value = (const void*) ((int)value + wr);
        address += wr;
        bytes -= wr;
    }
}

Uns32 icmMasterPort::readUpcall(Addr addr, Uns32 mask, Uns32 bytes)
{
    Uns32        rdata;     // For the result

    m_trans.set_read();
    m_trans.set_address( addr );

    m_trans.set_data_length(bytes);
    m_trans.set_data_ptr( (unsigned char *)&rdata );

    m_trans.set_byte_enable_length( sizeof(mask));
    m_trans.set_byte_enable_ptr( (unsigned char *)&mask );

    m_trans.set_streaming_width(sizeof(rdata));
    m_trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    // Transport. Then return the result
    m_socket->b_transport( m_trans, m_zero_delay );
    if (m_trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
        cerr << hex << "Read transaction at 0x" << addr << " did not complete correctly" << endl;
        return 0;
    }

    return  rdata;
}

void icmMasterPort::writeUpcall(Addr addr, Uns32 mask, Uns32 wdata, Uns32 bytes)
{
    m_trans.set_write();
    m_trans.set_address( addr );

    m_trans.set_data_length(bytes);
    m_trans.set_data_ptr( (unsigned char *)&wdata );

    m_trans.set_byte_enable_length(sizeof(mask));
    m_trans.set_byte_enable_ptr( (unsigned char *)&mask );

    m_trans.set_streaming_width(sizeof(wdata));
    m_trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    // Transport.
    m_socket->b_transport( m_trans, m_zero_delay );
    if (m_trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
        cerr << hex << "Write transaction at 0x" << addr << " did not complete correctly" << endl;
        return;
    }
}

//
// Output net port
//

icmOutputNetPort::icmOutputNetPort(icmPeripheral *pse,  const char *portName)
: m_net(CONCAT((*pse).instanceName,portName))
{
    pse->icmPseObject::connect(m_net, portName, ICM_OUTPUT);
    m_net.addCallback(icmOutputNetPort::update, this);
}

void icmOutputNetPort::update(void *user, Uns32 value)
{
    icmOutputNetPort *thisPtr = (icmOutputNetPort*)user;
    int t = value;
    //cout << "icmOutputNetPort::update" << t << endl;
    thisPtr->write(t); // SystemC net write
}

//
// Peripheral
//

icmPeripheral::icmPeripheral(
  sc_module_name          name
, const char             *model
, const char             *semihostLibrary
, icmAttrListObject      *initialAttrs
)
: sc_module (name)   // instance name
, icmPseObject(name, model, initialAttrs, semihostLibrary, semihostLibrary ? "modelAttrs" : 0)
{
	instanceName = name;
}


//
/////////////////////////// Input net port //////////////////////////////
//
icmInputNetPort::icmInputNetPort(icmPeripheral *pse, sc_module_name name)
  : m_net(0)  // anon name
{
  pse->icmPseObject::connect(m_net, (const char*)name, ICM_INPUT);
}

void icmInputNetPort::write(const int &value) {
  //cout << "icmInputNetPort::write" << value << endl;
  m_net.write(value);
}


