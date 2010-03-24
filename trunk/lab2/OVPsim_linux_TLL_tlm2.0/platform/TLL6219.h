/*
 * Copyright (c) 2005-2010 Imperas Software Ltd., www.imperas.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __TLL6219_H__
#define __TLL6219_H__



#include "tlm.h"
#include "icmPlatform.h"
#include "arm.h"
#include "DecodeBus.h"
#include "CoreModule9x6.h"
#include "IntICP.h"
#include "IcpCounterTimer.h"
#include "IcpControl.h"
#include "DebugLedAndDipSwitch.h"
#include "KbPL050.h"
#include "RtcPL031.h"
#include "UartPL011.h"
#include "MmciPL181.h"
#include "SmartLoaderArmLinux.h"

#include "busMemory.h"


class TLL6219
: public sc_core::sc_module
{
public:
    typedef tlm::tlm_initiator_socket<> initiator_socket_type;

    initiator_socket_type emi;

public:
    typedef enum memoryConfigE { MCONF_TLM, MCONF_OVP } memoryConfig;
    typedef enum bootConfigE { BCONF_UBOOT, BCONF_LINUX, BCONF_BAREMETAL } bootConfig;

    TLL6219 (sc_core::sc_module_name name, const char *exe, 
             memoryConfig mconf, bootConfig bconf, 
             bool coverage = false);

    icmInputNetPort *externalInterrupt(void) {
        return &m_pic_1.m_ir9;
    }

private:
    icmTLMPlatform    m_platform; // platform construction
    DecodeBusLT<2, 14>m_bus;     // simple bus decoder  <1,32>

    // --- i.MX21

    arm               m_cpu_1;   // TLM2 processor

    SmartLoaderArmLinux	m_smartload_1;  // Smart Loader

    CM9x6             m_core_1;  // Integrator Core Module 9x6
    IntICP            m_pic_1;   // Integrator Board interrupt controller
    IntICP            m_pic_2;   // Integrator Board interrupt controller

    ICPPIT            m_pit_1;   // IPC Timer Module

    RtcPL031          m_rtc_1;   // RTC

    UartPL011         m_uart_1;  // UART


    // --- TLL6219 base board

    ICPcontrol        m_icp_1;   // Board Controller

    KbPL050           m_kb_1;    // Keyboard
    KbPL050           m_ms_1;    // Mouse

    MmciPL181         m_mmci_1;  // Multimedia Card Interface

    DebugLedAndDipSwitch m_led_1;

    busMemory         m_ram_1;   // RAM
    busMemory         m_ram_2;   // ambaDummy


    icmAttrListObject *defaultKbAttrs() {
        icmAttrListObject *p = new icmAttrListObject();

        p->addAttr("isMouse", 0);
        p->addAttr("grabDisable", 0);
        return p;
    }

    icmAttrListObject *defaultMsAttrs() {
        icmAttrListObject *p = new icmAttrListObject();

        p->addAttr("isMouse", 1);
        p->addAttr("grabDisable", 1);
        return p;
    }

    icmAttrListObject *defaultUart1Attrs() {
        icmAttrListObject *p = new icmAttrListObject();

        p->addAttr("outfile", "uart1.log");
	//p->addAttr("infile", "uart1.log");
	p->addAttr("portnum", 9554);
        p->addAttr("variant", "ARM");
        return p;
    }

    icmAttrListObject *defaultSmartLoaderAttrs(bootConfig bc) {
        icmAttrListObject *p = new icmAttrListObject();

        p->addAttr("initrd", "fs.img");
        p->addAttr("kernel", "zImage");
	if (bc != BCONF_LINUX)
	p->addAttr("disable", "True");
        return p;
    }

};




#endif
