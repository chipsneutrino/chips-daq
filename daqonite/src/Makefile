CC=g++

INCDIRS = 		../../fh_message/include ../include
LIBDIRS = 		../../fh_message/src
LIBS = 			fh_message boost_system boost_chrono boost_program_options m pthread

CXXFLAGS =		$(INCDIRS:%=-I%) -O2 -g -Wall -fmessage-length=0

ROOTFLAGS = 	`root-config --cflags --glibs`

OBJS =          daqonite.o daq_handler.o clb_handler.o bbb_handler.o daqonite_gui.o

rootcint:
	rootcint -f daqonite_gui_dict.cc -c ../include/daqonite_gui.h daqonite_gui_linkdef.h

TARGET =        daqonite

TEST = 			bbb_testServer

COMMAND = 		daq_command

$(TARGET): $(OBJS) *.cc ../include/*.h
	$(CC) -o $(TARGET) daqonite_gui_dict.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS)

$(TEST): $(OBJS) bbb_testServer.o
	$(CC) -o $(TEST) bbb_testServer.o $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) 

$(COMMAND): daq_command.cc
	$(CC) -o $(COMMAND) daq_command.cc $(LIBDIRS:%=-L%) $(LIBS:%=-l%)
	
all: rootcint $(TARGET) $(COMMAND)

command: $(COMMAND)

test: $(TEST)

clean:
	rm -f $(OBJS) $(TARGET) $(COMMAND) $(TEST) *.o daqonite_gui_dict.cc daqonite_gui_dict.h
