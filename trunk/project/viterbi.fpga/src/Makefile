CC=g++
LDFLAGS=-L/usr/local/packages/mentor_2008/catapult/Mgc_home/lib -lstdc++
CFLAGS=-I$(MGC_HOME)/shared/include 
#LDFLAGS=

all: tb

tb: MyViterbi.o tb.o
	gcc $(LDFLAGS) MyViterbi.o tb.o -o tb

MyViterbi.o: MyViterbi.cpp
	gcc -c $(CFLAGS) MyViterbi.cpp -o MyViterbi.o

tb.o: tb.cpp
	gcc -c $(CFLAGS) tb.cpp -o tb.o

clean:
	rm -f tb tb.o MyViterbi.o

