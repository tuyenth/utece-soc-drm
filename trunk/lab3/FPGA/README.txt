MyViterbi converted from a SC module to a regular C++ class.
memcpy replaced with MyMemcpy
initiatorTransport is moved out of the class and set as the top level design.
A static MyViterbi is instantiated in function initiatorTransport. 

CVector and CMatrix need to be replaced.
