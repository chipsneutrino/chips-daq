CC = g++

# These are just default values for Josh.
# Setting them in env. variables will override them.
ELASTICLIENT_PATH ?= /home/jtingey/elasticlient
BOOST_PATH        ?= /opt/boost
ROOT_PATH         ?= /home/jtingey/root
CPP_STANDARD      ?= c++0x

INCDIRS  += ../../fh_message/include
INCDIRS  += ../include
INCDIRS  += $(ROOT_PATH)/include
INCDIRS  += $(BOOST_PATH)/include
INCDIRS  += $(ELASTICLIENT_PATH)/external/cpr/include
INCDIRS  += $(ELASTICLIENT_PATH)/external/jsoncpp/include
INCDIRS  += $(ELASTICLIENT_PATH)/include

LIBDIRS  += ../../fh_message/src
LIBDIRS  += $(BOOST_PATH)/lib
LIBDIRS  += $(ELASTICLIENT_PATH)/build/lib

LIBS      = fh_message boost_system boost_chrono boost_program_options boost_thread elasticlient cpr jsoncpp

CXXFLAGS  = $(INCDIRS:%=-I%) -O2 -g -Wall -fmessage-length=0 -std=$(CPP_STANDARD)
ROOTFLAGS = `root-config --cflags --glibs`

OBJS      = monitoring_config.o monitoring_server.o data_handler.o clb_handler.o bbb_handler.o daq_handler.o packet_generator.o elastic_interface.o

DAQ       = daqonite
COMMAND   = daqommand
SIM       = daqulator
MON       = daqsitter

all:        $(DAQ) $(COMMAND) $(SIM) $(MON)
daq:        $(DAQ)
command:    $(COMMAND)
sim:        $(SIM)
mon:        $(MON)
.PHONY: all daq command sim mon

clean:
	rm -f $(OBJS) $(DAQ) $(COMMAND) $(SIM) $(MON)
.PHONY: clean

$(DAQ): $(OBJS) *.cc ../include/*.h
	$(CC) -o $(DAQ) daqonite.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(COMMAND): daqommand.cc
	$(CC) -o $(COMMAND) daqommand.cc $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(SIM): $(OBJS) daqulator.cc
	$(CC) -o $(SIM) daqulator.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(MON): $(OBJS) daqsitter.cc
	$(CC) -o $(MON) daqsitter.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

