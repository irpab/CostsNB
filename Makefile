CC=g++
CFLAGS=-std=c++11
CC_INCLUDE=-I json/ -I ./

all: run_ut

jsoncpp.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c jsoncpp.cpp -o jsoncpp.o

categories_to_backend.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c categories_to_backend.cpp -o categories_to_backend.o

categories_to_json_converter.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c categories_to_json_converter.cpp -o categories_to_json_converter.o

utils.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c utils.cpp -o utils.o

costs_nb_core.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c costs_nb_core.cpp -o costs_nb_core.o

ut: categories_to_json_converter.o utils.o categories_to_backend.o jsoncpp.o costs_nb_core.o
	$(CC) $(CFLAGS) $(CC_INCLUDE) categories_to_json_converter.o utils.o categories_to_backend.o jsoncpp.o costs_nb_core.o -lrestclient-cpp costs_nb_core_ut.cpp -o ut

run_ut: ut
	./ut

clean:
	rm -f *.o ut
