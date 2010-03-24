This is a Preliminary release of the ARM TLM2.0 Demo

Please see Limitations section below

Please report any issue to support@imperas.com


###############################################################################
# 1. Purpose of Demo
###############################################################################
This demo will show how to execute linux running on an ARM926 processor on the 
TLL platform created in SystemC TLM2.0 and utilizing a high performance OVP 
processor model and OVP peripheral models wrapped for use with TLM2.0.

The Platform contains

	TLL6219 Daughter Board
	   i.MX21 Application Processor
	      ARM926 Core
	      Interrupt Controller
	      UART
	   128M Bytes RAM (bridged to separate area)
	   Mouse/Keyboard Interfaces
	   Flash Memory
	TLL5000 Base Board
	   CPLD
	   External FPGA hardware
	      Simple adder example   

The platform is used to execute a precompiled version of linux that can be 
interacted with using a console. 


###############################################################################
# 2. Limitations
###############################################################################
The simulation is not terminating on a <CTRL-C>. The simulation must be 
terminated by killing the running process. 


###############################################################################
# 3. Re-Building
###############################################################################
               
The simulation platform ArmIntegratorCP_tlm2.0.Linux.exe is compiled 
using GCC under Linux.

The compilation is performed by running 'make'.


###############################################################################
# 4. Running the TLM2.0 platform simulation with OVP
###############################################################################
Executing 'TLL_tlm2.0.Linux.exe' will start the platform. 

When the simulation starts it will require a connection to a UART device. 
This can be accomplished by starting a telnet session in a separate shell. 

Start the emulated terminal session using 'telnet localhost 9000'.

If somebody else is already using port 9000 on the same machine, you will
have to switch to a different port. Change the line
	p->addAttr("portnum", 9000);
in TLL6219.h to a free port number of your choice and recompile the platform
model with 'make clean ; make'.  


###############################################################################
# 5 Compiling User Programs, and adding to disk image
###############################################################################
Firstly, make sure that the ARM cross-compiler environment is setup properly
and build your user program using the cross-compiler too chain.

Now, we want to add this to our ramdisk.

Extract Ramdisk to file system
% ./unpackImage.sh fs.img fs

Copy benchmarks into filesystem
% cp -r ../application fs

Package filesystem back into a ramdisk
% ./packImage.sh fs fs.img

Re run the simulator, this time there will be a top level directory called
'application', which contains the applications build in the previous steps
these can now be executed in the simulated Linux.
