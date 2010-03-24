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


#include "TLL6219.h"

#include <iostream>



TLL6219::TLL6219  ( sc_core::sc_module_name name, const char *exe, memoryConfig mconf, bootConfig bconf, bool coverage )
  : sc_core::sc_module (name)

  , m_platform ("icm", ICM_VERBOSE | ICM_WALLCLOCK | ICM_STOP_ON_CTRLC | ICM_ENABLE_IMPERAS_INTERCEPTS,"localhost",8888)
  , m_bus          ("bus1")

  , m_cpu_1    ("cpu_1", 1, ICM_ATTR_SIMEX | ICM_ATTR_TRACE_ICOUNT | ICM_ATTR_APPROX_TIMER | ICM_ATTR_RELAXED_SCHED )

  , m_smartload_1   ("smartLoader",defaultSmartLoaderAttrs(bconf) )

  , m_core_1   ("cm")

  , m_pic_1   ("pic1")
  , m_pic_2   ("pic2")

  , m_pit_1   ("pit")

  , m_rtc_1   ("rtc")

  , m_uart_1  ("uart1",defaultUart1Attrs())

  , m_icp_1   ("icp")

  , m_kb_1    ("kb1",defaultKbAttrs())
  , m_ms_1    ("ms1",defaultMsAttrs())

  , m_mmci_1   ("mmci")

  , m_led_1   ("led")

  , m_ram_1    ("ram1",  "memory_socket", 0x08000000 , 4)
  , m_ram_2    ("ambaDummy","memory_socket", 0x1000, 4)

 {
    // --- i.MX21

    // ARM9 CPU
    m_cpu_1.m_socket(m_bus.target_socket[0]);
    m_smartload_1.m_bport1.m_socket(m_bus.target_socket[1]);

    // Core Module 9x6
    m_bus.initiator_socket[1](m_core_1.m_bport1.m_socket);
    m_bus.setDecode(1, 0x10000000, 0x10000fff);

    // Interrupt controller 1
    m_bus.initiator_socket[2](m_pic_1.m_bport1.m_socket);
    m_bus.setDecode(2, 0x14000000, 0x14000fff);

    // Interrupt controller 2
    m_bus.initiator_socket[3](m_pic_2.m_bport1.m_socket);
    m_bus.setDecode(3, 0xca000000, 0xca000fff);

    // IPC Timer Module
    m_bus.initiator_socket[4](m_pit_1.m_bport1.m_socket);
    m_bus.setDecode(4, 0x13000000, 0x13000fff);

    // RTC
    m_bus.initiator_socket[5](m_rtc_1.m_bport1.m_socket);
    m_bus.setDecode(5, 0x15000000, 0x15000fff);

    // UART 1
    m_bus.initiator_socket[6](m_uart_1.m_bport1.m_socket);
    m_bus.setDecode(6, 0x16000000, 0x16000fff);

    // Connectivity
    m_pic_1.m_irq.bind(*m_cpu_1.irq);
    m_pic_1.m_fiq.bind(*m_cpu_1.fiq);

    m_uart_1.m_irq.bind(m_pic_1.m_ir1);

    m_pit_1.m_irq0.bind(m_pic_1.m_ir5);
    m_pit_1.m_irq1.bind(m_pic_1.m_ir6);
    m_pit_1.m_irq2.bind(m_pic_1.m_ir7);

    m_rtc_1.m_irq.bind(m_pic_1.m_ir8);

    m_mmci_1.m_irq0.bind(m_pic_1.m_ir23);
    m_mmci_1.m_irq1.bind(m_pic_1.m_ir24);


    // --- TLL6219 base board

    // Board controller
    m_bus.initiator_socket[7](m_icp_1.m_bport1.m_socket);
    m_bus.setDecode(7, 0xcb000000, 0xcb00000f);

    // Keyboard interface
    m_bus.initiator_socket[8](m_kb_1.m_bport1.m_socket);
    m_bus.setDecode(8, 0x18000000, 0x18000fff);

    // Mouse interface
    m_bus.initiator_socket[9](m_ms_1.m_bport1.m_socket);
    m_bus.setDecode(9, 0x19000000, 0x19000fff);

    // MMCI
    m_bus.initiator_socket[10](m_mmci_1.m_bport1.m_socket);
    m_bus.setDecode(10, 0x1c000000, 0x1c000fff);

    // LEDs and DIP switches
    m_bus.initiator_socket[11](m_led_1.m_bport1.m_socket);
    m_bus.setDecode(11, 0x1a000000, 0x1a000fff);

    // Memory
    m_bus.initiator_socket[12](m_ram_1.m_memory_socket);
    m_bus.setDecode(12, 0x0, 0x7ffffff);

    // Memory
    m_bus.initiator_socket[13](m_ram_2.m_memory_socket);
    m_bus.setDecode(13, 0x1d000000, 0x1d000fff);

    // EMI
    m_bus.initiator_socket[0](emi);
    m_bus.setDecode(0, 0xcc000000, 0xd3ffffff);
    

    // Connectivity
    m_kb_1.m_irq.bind(m_pic_1.m_ir3);
    m_ms_1.m_irq.bind(m_pic_1.m_ir4);


    // --- Simulation parameters

    m_cpu_1.setIPS(200000000);

    m_core_1.setDiagnosticLevel(3);

    m_uart_1.setDiagnosticLevel(1);

    m_smartload_1.setDiagnosticLevel(7);

//  m_cpu_1.traceOnAfter(766138950);


    // one of 2 ways to configure the memory
    switch (mconf) {
        case MCONF_TLM:{
            cout << "Using SystemC TLM2.0 Memory with DMI" << endl;

            // Use the TLM code memory. It needs to be loaded with the program.
            m_cpu_1.mapTLMMemory("ram1", 0x0, 0x7ffffff);
            m_cpu_1.mapTLMMemory("ambaDummy",   0x1d000000, 0x1d000fff );
            break;
        }
        case MCONF_OVP: {
            cout << "Using OVP Memory" << endl;

            // Do not use the TLM memory. Instead create fast (sparse) RAM inside OVP.
            // Give the internal memory similar names to the TLM.

            m_cpu_1.mapNativeMemory("ram1", m_ram_1.getMemory()->get_mem_ptr(), 0x0, 0x7ffffff);
            m_cpu_1.mapNativeMemory("ambaDummy", m_ram_2.getMemory()->get_mem_ptr(),  0x1d000000, 0x1d000fff );

            break;
        }
    }
}



