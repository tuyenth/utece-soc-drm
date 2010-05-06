File Description:
table.txt.1
input.txt.1
golden.txt.1
These three files are test vectors. table.txt.1 stores a puncture table.
input.txt.1 stores input data. golden.txt.1 stores the golden file.
To obtain the puncture table, we execute the drm program on ARM simulator
and dump the puncture table calculated by CViterbiDecoder::Init.
To obtain the input and golden output data, we execute the drm program on ARM
simulator and dump the input to and output from CViterbiDecoder::Decode.

tb.cpp
This file is the test bench. It reads the puncture table, input file, and 
output file. It then feeds the puncture table and input data to the DUT, 
reads the output from the DUT, and compares the output obtained from the DUT
and the golden data.

FXP.h
GlobalDefinitions.h
MyViterbi.cpp
MyViterbi.h
TableDRMGlobal.h
These files are the DUT.

To run the test bench, just type 'make', then run './tb'.

SCVerity.output
The log of simulation.
