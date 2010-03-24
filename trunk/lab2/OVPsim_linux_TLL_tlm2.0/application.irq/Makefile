# --- Makefile for simple Adder example.

# ARM cross-compiler
PROC	= arm
TYPE	= linux
VERSION	= arm-2008q1
LIBPATH	= /usr/local/$(PROC)/$(VERSION)/lib
INCPATH	= /usr/local/$(PROC)/$(VERSION)/include
CC	= $(PROC)-$(TYPE)-gcc
CFLAGS	= --static -I$(INCPATH) -L$(LIBPATH)
STRIP	= $(PROC)-$(TYPE)-strip

UNAME 	= $(shell uname -r)
KERNEL26= 2.6
KERNELVERSION= $(findstring $(KERNEL26),$(UNAME))
KDIR	= /home/projects/gerstl/soc/coglinux-2.6.16-integrator
KFLAGS  = -fno-common

obj-m	= fpga_drv.o


all: fpga_drv.ko example

fpga_drv.ko: fpga_drv.c
	$(MAKE) -C $(KDIR) M=$(PWD) EXTRA_CFLAGS="$(KFLAGS)" modules

example: example.c
	$(CC) example.c $(CFLAGS) -o $@ ;\
	$(STRIP) $@ 


clean:
	$(RM) .skeleton* .*.cmd *.o *.ko *.mod.c
	$(RM) -R .tmp*
	$(RM) example *~
