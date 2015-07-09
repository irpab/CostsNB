CC=g++
CFLAGS=-std=c++11
CC_INCLUDE=-I json/ -I ./

all: core

jsoncpp:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c jsoncpp.cpp -o jsoncpp.o

core: jsoncpp
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c costs_nb_core.cpp -o costs_nb_core.o

ut: core
	$(CC) $(CFLAGS) $(CC_INCLUDE) jsoncpp.o costs_nb_core.o costs_nb_core_ut.cpp -o ut

run_ut: ut
	./ut

clean:
	rm -f *.o ut
