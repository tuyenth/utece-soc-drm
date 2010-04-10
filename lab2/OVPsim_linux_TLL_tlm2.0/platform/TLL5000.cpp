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


#include "TLL5000.h"

#include <iostream>



TLL5000::TLL5000  ( sc_core::sc_module_name name, 
                    const char *exe, 
                    TLL6219::memoryConfig mconf, 
                    TLL6219::bootConfig bconf, 
                    bool coverage )
  : sc_core::sc_module (name)

  , m_adderLow  ( 0xd3000000 )
  , m_adderHigh ( 0xd3006000 )

  , m_board_1 ("board1", exe, mconf, bconf, coverage )

  , m_bus     ("bus")

  , m_viterbi0   ("Viterbi0")
  /*
  , m_viterbi1   ("Viterbi1", 1)
  , m_viterbi2   ("Viterbi2", 2)
  , m_viterbi3   ("Viterbi3", 3)
  , m_viterbi4   ("Viterbi4", 4)
  , m_viterbi5   ("Viterbi5", 5)
  */

{
    m_board_1.emi(m_bus.target_socket);

    // FPGA hardware
	m_bus.initiator_socket[0](m_viterbi0.bus);
    /*
	m_bus.initiator_socket[0](m_viterbi0.bus);
	m_bus.initiator_socket[1](m_viterbi1.bus);
    m_bus.initiator_socket[2](m_viterbi2.bus);
    m_bus.initiator_socket[3](m_viterbi3.bus);
    m_bus.initiator_socket[4](m_viterbi4.bus);
    m_bus.initiator_socket[5](m_viterbi5.bus);
    */
	
	m_bus.setDecode(0, m_adderLow       , m_adderHigh);
	
	/*
	m_bus.setDecode(0, m_adderLow       , m_adderLow+0x0900);
	m_bus.setDecode(1, m_adderLow+0x1000, m_adderLow+0x1900);
	m_bus.setDecode(2, m_adderLow+0x2000, m_adderLow+0x2900);
	m_bus.setDecode(3, m_adderLow+0x3000, m_adderLow+0x3900);
	m_bus.setDecode(4, m_adderLow+0x4000, m_adderLow+0x4900);
	m_bus.setDecode(5, m_adderLow+0x5000, m_adderLow+0x5900);*/
    //m_adder.intr.bind(*m_board_1.externalInterrupt());
}

