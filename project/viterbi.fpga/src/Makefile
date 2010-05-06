CC=g++
LDFLAGS=-L/usr/local/packages/mentor_2008/catapult/Mgc_home/lib -lstdc++
#LDFLAGS=

all: tb

tb: MyViterbi.o tb.o
	gcc $(LDFLAGS) MyViterbi.o tb.o -o tb

MyViterbi.o: MyViterbi.cpp
	gcc -c MyViterbi.cpp -o MyViterbi.o

tb.o: tb.cpp
	gcc -c tb.cpp -o tb.o

clean:
	rm -f tb tb.o MyViterbi.o

