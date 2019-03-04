CC=g++

INCDIRS = 		../../fh_message/include ../include /usr/cern/root/include
LIBDIRS = 		../../fh_message/src
LIBS = 			fh_message boost_system boost_chrono boost_program_options m pthread boost_thread

CXXFLAGS =		$(INCDIRS:%=-I%) -O2 -g -Wall -fmessage-length=0 -std=c++0x

ROOTFLAGS = 	`root-config --cflags --glibs`

OBJS =          monitoring_buffer.o monitoring_config.o monitoring_gui.o data_handler.o clb_handler.o bbb_handler.o daq_handler.o clb_simulator.o daqonite.o

rootcint:
	rootcint -f monitoring_gui_dict.cc -c ../include/monitoring_gui.h monitoring_gui_linkdef.h

TARGET =        daqonite

TEST = 			bbb_test

COMMAND = 		daqommand

$(TARGET): $(OBJS) *.cc ../include/*.h
	$(CC) -o $(TARGET) monitoring_gui_dict.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS)

$(TEST): $(OBJS) bbb_test.o
	$(CC) -o $(TEST) bbb_test.o $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) 

$(COMMAND): daqommand.cc
	$(CC) -o $(COMMAND) daqommand.cc $(LIBDIRS:%=-L%) $(LIBS:%=-l%)
	
all: rootcint $(TARGET) $(COMMAND)

command: $(COMMAND)

test: $(TEST)

clean:
	rm -f $(OBJS) $(TARGET) $(COMMAND) $(TEST) *.o monitoring_gui_dict.cc monitoring_gui_dict.h
