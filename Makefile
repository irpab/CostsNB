CC=g++
CFLAGS=-std=c++11
CC_INCLUDE=-I json/ -I restclient-cpp/ -I ./

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

base64.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c base64.cpp -o base64.o

miniz.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c miniz.cpp -o miniz.o

miniz_wrp.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c miniz_wrp.cpp -o miniz_wrp.o

restclient-cpp.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c restclient.cpp -o restclient-cpp.o

ut: base64.o miniz.o miniz_wrp.o categories_to_json_converter.o utils.o categories_to_backend.o jsoncpp.o costs_nb_core.o restclient-cpp.o
	$(CC) $(CFLAGS) $(CC_INCLUDE) base64.o miniz.o miniz_wrp.o categories_to_json_converter.o utils.o categories_to_backend.o jsoncpp.o costs_nb_core.o restclient-cpp.o -lcurl costs_nb_core_ut.cpp -o ut

run_ut: ut
	./ut

clean:
	rm -f *.o ut
