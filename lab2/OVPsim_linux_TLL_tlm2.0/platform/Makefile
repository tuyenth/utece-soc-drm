# Makefile for TLL platform model

PROCESSOR= arm

COMPONENT_INCS= -IPeripheral_TLM2.0_Wrappers
COMPONENT_OBJS= Build/Linux/TLL6219.o	\
				Build/Linux/MyViterbi.o  \
                Build/Linux/TLL5000.o

PLATFORM= TLL_tlm2.0

CPP	= g++ -m32
LD	= g++ -m32


all:
	$(MAKE) -f Makefile_TLM2.Linux CPP="$(CPP)" LD="$(LD)"	\
		PROCESSOR=$(PROCESSOR) 			\
		COMPONENT_INCS="$(COMPONENT_INCS)"	\
		COMPONENT_OBJS="$(COMPONENT_OBJS)"	\
		PLATFORM=$(PLATFORM)			\
		all

clean:
	$(MAKE) -f Makefile_TLM2.Linux CPP="$(CPP)" LD="$(LD)"	\
		PROCESSOR=$(PROCESSOR) 			\
		COMPONENT_INCS="$(COMPONENT_INCS)"	\
		COMPONENT_OBJS="$(COMPONENT_OBJS)"	\
		PLATFORM=$(PLATFORM)			\
		clean
	$(RM) *~ *.log
	#$(RM) -r Build fs
