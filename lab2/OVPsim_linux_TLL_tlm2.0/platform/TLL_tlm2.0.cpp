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




#include "TLL5000.h"                    // top module

#include "tlm.h"                        // TLM header
#define REPORT_DEFINE_GLOBALS           // reporting overhead
#include "reporting.h"
//#include "systemc.h"



int sc_main (int argc, char *argv[] )
{
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

    sc_set_time_resolution(1,SC_NS);

    const char *exe = "vmlinux"; //defaultFile";

    TLL6219::bootConfig bc = TLL6219::BCONF_LINUX;

    bool  coverage = false;

    if (argc >= 2) {
		if (*argv[1] == 'u')
		{
			bc = TLL6219::BCONF_UBOOT;
			exe = "application/u-boot";
		}
		else if (*argv[1] == 'l')
		{
			bc = TLL6219::BCONF_LINUX;
		}
		else
		{
			bc = TLL6219::BCONF_BAREMETAL;
			exe = argv[1];
		}
    }

    TLL6219::memoryConfig mc = TLL6219::MCONF_TLM; //OVP;
    if (argc > 2) {

        switch(*argv[2]) {
            case 'o':
                mc = TLL6219::MCONF_OVP;
                break;
           case 's':
                mc = TLL6219::MCONF_TLM;
                break;
            case 'c':
                mc = TLL6219::MCONF_OVP;
                coverage = true;
                break;
            default:
                cout << "Usage: TLL_tlm2.0.exe [u|l|b] [o|s|c]" << endl;
                cout << "       u = U-Boot: l = Linux:  b = Bare metal" << endl;
                cout << "       o = OVP memory: s = SystemC TLM memory:  c = OVP memory and icov" << endl;
                exit(0);
                break;
        }
    }

    REPORT_ENABLE_ALL_REPORTING ();

    cout << "Constructing." << endl;
    TLL5000 top("top", exe, mc, bc, coverage);

    cout << "default time resolution = " << sc_get_time_resolution() << endl;

    // start the simulation
    cout << "Starting sc_main." << endl;
    sc_start(-1);

    cout << "Finished sc_main." << endl;
    return 0;                             // return okay status
}
