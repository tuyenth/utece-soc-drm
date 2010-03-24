/*
 *
 * My SystemC - TLM 2.0 adder example
 * Author: Pablo Salinas
 * Date: 01/25/2010
 * Comments: This simple adder do the following:
 * a) If the application writes to address X, the peripheral should check the address and store the received 32bits data in data member 'a'.
 * b) If the application writes to address Y, the peripheral should check the address and store the received 32bits data in data member 'b'.
 * c) If the application writes to address W, the peripheral should check the address and add 'a' and 'b' and store the result.
 * d) If the application reads from address W, the peripheral should return the previously stored result.
 *
*/

/*
 * Date: 01/28/10
 * Note2: This version adds an interrupt line from the external adder to the arm cpu in order
 * to signal whenever the addition was finished being performed.
*/


#ifndef _ADDER_PERIPH_H_
#define _ADDER_PERIPH_H_


#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"

#include <ostream>

using namespace sc_core;
using namespace tlm;
using namespace std;
	

SC_MODULE(Adder)          // declare adder sc_module
{
public:

  typedef tlm_utils::simple_target_socket<Adder> target_socket_type;
  typedef tlm_generic_payload                    transaction_type;

  tlm_analysis_port<int> intr;  //This is the interface that would work as an interrupt.
				//Contrary to the more general OVP peripheral example,
				//I only used one interrupt line as permited.

  target_socket_type bus;

private:

  int do_add(int val1, int val2)         // a C++ function
  {
    return val1 + val2;
  }
#define ELEMENTS 4096 
#define TANK_SPACE 0x100
  int numTank;
  int idxTank;
  int Tank[ELEMENTS];
  int idxRead;

public:
  int m_value1, m_value2, m_result;

  //constructor
  Adder(sc_core::sc_module_name name)
  : sc_module(name)
  {
	// Reset variables
	m_value1 = 0;
	m_value2 = 0;
	m_result = -1;
	
	// Register callback for incoming b_transport interface method call
        bus.register_b_transport(this, &Adder::initiatorTransport);
        bus.register_transport_dbg(this, &Adder::initiatorDebug);
  }
  
  void initiatorTransport(transaction_type& trans,
                            sc_core::sc_time& t)
  {
        Uns64 addr     = trans.get_address();
        Uns32 *dataPtr = (Uns32*)trans.get_data_ptr();
	unsigned int len = trans.get_data_length();

        Uns32  data    = *dataPtr;
	char *what;
	int i, incorrect = 0;

        switch( trans.get_command() ) 
	{
            case TLM_READ_COMMAND :  
					what = "read";
					if (addr == 0xd3000000) // Return result
					{
						intr.write(0); // This would disable the interrupt line again

						data = m_result;
						memcpy(dataPtr, &data, len); 
					}
					else if(addr >= 0xd3000200 && 
							addr < 0xd3000200 + TANK_SPACE)
					{
						memcpy(dataPtr, &Tank[idxRead], len);
						if(len != 4)
							cout << "ERROR: len = " << len << endl;  
						idxRead++;
					}
					break;
            case TLM_WRITE_COMMAND:  
					what = "write" ;  
					if (addr == 0xd3000000) // Do the addition - Any details regarding timing should go here
					{
						m_result = this->do_add(m_value1, m_value2);   
						intr.write(1);    // Signal an interrupt to the CPU
					}

					else if(addr == 0xd3000004) // Store the first operand
						this->m_value1 = data;
						
					else if(addr == 0xd3000008) // Store the second operand
					{
						this->m_value2 = data;
					}
					else if(addr == 0xd300000C)
					{
						if(data == 1) 
						{
							incorrect = 0;
							for(i = 0; i < ELEMENTS; i++)
							{
								if(Tank[i] != i)
									incorrect++;
							}
							cout << "incorrect = " << incorrect << endl;
							cout << "Ready for Read Back" << endl;
							idxRead = 0;
						}
						/*
						else if(data == 2)
						{
							numToBeRead = ELEMENTS;	

						}*/
					}
					else if(addr == 0xd3000010)
					{
						this->numTank = data;
						this->idxTank = 0;
					}
					else if(addr >= 0xd3000020)
					{
						if(numTank > 0) {
							Tank[idxTank] = data;
							idxTank++;
							numTank--;
						}
					}
					break;
            default               :  	
					what = "not interested";  
					break;
        }
        if (1) {
            cout << hex << "Peripheral " << name() << " "<< what << " addr: " << addr << " data: " << data << " len:" << len << endl;
        }

        trans.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  unsigned int initiatorDebug(transaction_type& trans)
  {
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
        return 0;
  }

};

#endif
