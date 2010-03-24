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


all: example

example: example.c
	$(CC) example.c $(CFLAGS) -o $@ ;\
	$(STRIP) $@ 


clean:
	$(RM) *.o *~
	$(RM) example 
