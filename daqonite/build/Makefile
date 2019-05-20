# Compilers
CC  = gcc
CXX = g++

# These are just default values for Josh.
# Setting them in env. variables will override them.
ELASTICLIENT_PATH ?= ../lib/elasticlient
BOOST_PATH        ?= /opt/boost
ROOT_PATH         ?= /home/jtingey/root
CPP_STANDARD      ?= c++0x
SRC_PATH          ?= ../src

# Include directories
INCDIRS  += ../../fh_message/include
INCDIRS  += ../include
INCDIRS  += $(ROOT_PATH)/include
INCDIRS  += $(BOOST_PATH)/include
INCDIRS  += $(ELASTICLIENT_PATH)/external/cpr/include
INCDIRS  += $(ELASTICLIENT_PATH)/external/jsoncpp/include
INCDIRS  += $(ELASTICLIENT_PATH)/include

# Library directories
LIBDIRS  += ../../fh_message/src
LIBDIRS  += $(BOOST_PATH)/lib
LIBDIRS  += $(ELASTICLIENT_PATH)/build/lib

# Libraries
LIBS      = fh_message boost_system boost_chrono boost_program_options boost_thread elasticlient cpr jsoncpp

# Compiler Flags
CXXFLAGS  = $(INCDIRS:%=-I%) -O2 -g -Wall -fmessage-length=0 -std=$(CPP_STANDARD)
ROOTFLAGS = `root-config --cflags --glibs`

# Objects
OBJS     += monitoring_config.o
OBJS     += monitoring_server.o
OBJS     += data_handler.o
OBJS     += clb_handler.o
OBJS     += bbb_handler.o
OBJS     += daq_handler.o
OBJS     += packet_generator.o
OBJS     += elastic_interface.o
OBJS     += merge_sorter.o
OBJS     += run_file.o

DAQ       = daqonite
COMMAND   = daqommand
SIM       = daqulator
MON       = daqsitter


###########################################################
# Build Targets & Rules
###########################################################

all:        $(DAQ) $(COMMAND) $(SIM) $(MON)
daq:        $(DAQ)
command:    $(COMMAND)
sim:        $(SIM)
mon:        $(MON)
.PHONY: all daq command sim mon

clean:
	rm -f $(OBJS) $(DAQ) $(COMMAND) $(SIM) $(MON)
.PHONY: clean

%.o: $(SRC_PATH)/%.cc
	@echo "CXX $@"
	@$(CXX) -o $@ -c $< $(CPPFLAGS) $(CXXFLAGS)

$(DAQ): $(OBJS) $(SRC_PATH)/*.cc ../include/*.h
	@echo "LD  $@"
	@$(CXX) -o $@ $(SRC_PATH)/$(DAQ).cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(COMMAND): $(SRC_PATH)/$(COMMAND).cc
	@echo "LD  $@"
	@$(CXX) -o $@ $< $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(SIM): $(OBJS) $(SRC_PATH)/$(SIM).cc
	@echo "LD  $@"
	@$(CXX) -o $@ $(SRC_PATH)/$(SIM).cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(MON): $(OBJS) $(SRC_PATH)/$(MON).cc
	@echo "LD  $@"
	@$(CXX) -o $@ $(SRC_PATH)/$(MON).cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)
