CXX=g++
CXXFLAGS=-std=c++11
LDLIBS=-lcurl
INCLUDE=-I json/ -I restclient-cpp/ -I ./

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean run_ut all help

all: run_ut

OBJ = base64.o \
	categories_to_backend.o \
	categories_to_json_converter.o \
	costs_nb_core_ut.o \
	costs_nb_core.o \
	ini_config.o \
	jsoncpp.o \
	miniz_wrp.o \
	miniz.o \
	restclient.o \
	utils.o

ut: $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJ) $(LDLIBS) -o ut

run_ut: ut
	./ut

help:
	@echo "ut - build Unit Tests"
	@echo "run_ut - run all Unit Tests"

clean:
	rm -f *.o ut
