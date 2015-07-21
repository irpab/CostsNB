CC=g++
CFLAGS=-std=c++11
CC_INCLUDE=-I json/ -I ./

all: core

jsoncpp.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c jsoncpp.cpp -o jsoncpp.o

costsnb_clisrv_decoder.o: gen_xslt
	g++ -g -std=c++11 -c costsnb_clisrv_decoder.cpp -o costsnb_clisrv_decoder.o

costsnb_clisrv_encoder.o: gen_xslt
	g++ -g -std=c++11 -c costsnb_clisrv_encoder.cpp -o costsnb_clisrv_encoder.o

costsnb_tcp_transport.o: costsnb_clisrv_decoder.o costsnb_clisrv_encoder.o
	g++ -g -std=c++11 -c costsnb_tcp_transport.cpp -o costsnb_tcp_transport.o

core: jsoncpp.o costsnb_tcp_transport.o
	$(CC) $(CFLAGS) $(CC_INCLUDE) -c costs_nb_core.cpp -o costs_nb_core.o

ut: core
	$(CC) $(CFLAGS) $(CC_INCLUDE) costsnb_clisrv_decoder.o costsnb_clisrv_encoder.o costsnb_tcp_transport.o jsoncpp.o costs_nb_core.o costs_nb_core_ut.cpp -o ut

run_ut: ut
	./ut

gen_xslt:
	saxon-xslt costsnb_clisrv_messages.xml costsnb_clisrv_cpp_hdr_opcodes.xslt > costsnb_clisrv_opcodes.h
	saxon-xslt costsnb_clisrv_messages.xml costsnb_clisrv_cpp_hdr_structs.xslt > costsnb_clisrv_structs.h
	saxon-xslt costsnb_clisrv_messages.xml costsnb_clisrv_cpp_decoder.xslt > costsnb_clisrv_decoder.cpp
	saxon-xslt costsnb_clisrv_messages.xml costsnb_clisrv_cpp_encoder.xslt > costsnb_clisrv_encoder.cpp
	saxon-xslt costsnb_clisrv_messages.xml costsnb_clisrv_cpp_hdr_encoder.xslt > costsnb_clisrv_encoder.h

clean:
	rm -f *.o ut
