CC=g++

INCDIRS = 		../../fh_message/include ../include /home/jtingey/root/include /opt/boost/include /home/jtingey/elasticlient/external/cpr/include /home/jtingey/elasticlient/include /home/jtingey/elasticlient/external/jsoncpp/include
LIBDIRS = 		../../fh_message/src /opt/boost/lib /home/jtingey/elasticlient/build/lib
LIBS = 			fh_message boost_system boost_chrono boost_program_options boost_thread elasticlient cpr jsoncpp

CXXFLAGS =		$(INCDIRS:%=-I%) -O2 -g -Wall -fmessage-length=0 -std=c++0x
ROOTFLAGS = 	`root-config --cflags --glibs`

OBJS =          monitoring_config.o monitoring_server.o data_handler.o clb_handler.o bbb_handler.o daq_handler.o packet_generator.o elastic_interface.o

DAQ =        	daqonite
COMMAND = 		daqommand
SIM = 			daqulator
MON = 			daqsitter

$(DAQ): $(OBJS) *.cc ../include/*.h
	$(CC) -o $(DAQ) daqonite.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(COMMAND): daqommand.cc
	$(CC) -o $(COMMAND) daqommand.cc $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(SIM): $(OBJS) daqulator.cc
	$(CC) -o $(SIM) daqulator.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

$(MON): $(OBJS) daqsitter.cc
	$(CC) -o $(MON) daqsitter.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) $(CXXFLAGS)

all: 		$(DAQ) $(COMMAND) $(SIM) $(MON)
daq: 		$(DAQ)
command: 	$(COMMAND)
sim: 		$(SIM)
mon: 		$(MON)
.PHONY: all daq command sim mon

clean:
	rm -f $(OBJS) $(DAQ) $(COMMAND) $(SIM) $(MON)
.PHONY: clean

