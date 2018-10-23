CC=g++

INCDIRS = 		../../fh_message/include ../include
LIBDIRS = 		../../fh_message/src
LIBS = 			fh_message boost_system boost_chrono boost_program_options m pthread

CXXFLAGS =		$(INCDIRS:%=-I%) -O2 -g -Wall -fmessage-length=0

ROOTFLAGS = 	`root-config --cflags --glibs`

OBJS =          DAQonite.o DAQ_handler.o DAQ_clb_handler.o DAQ_bbb_handler.o Monitoring_gui.o

rootcint:
	rootcint -f Monitoring_gui_dict.cc -c ../include/Monitoring_gui.h Monitoring_gui_linkdef.h

TARGET =        DAQonite

TEST = 			BBBTest

COMMAND = 		DAQommand

$(TARGET): $(OBJS) *.cc ../include/*.h
	$(CC) -o $(TARGET) Monitoring_gui_dict.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS)

$(TEST): $(OBJS) BBBTest.o
	$(CC) -o $(TEST) BBBTest.o $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) 

$(COMMAND): DAQommand.cc
	$(CC) -o $(COMMAND) DAQommand.cc $(LIBDIRS:%=-L%) $(LIBS:%=-l%)
	
all: rootcint $(TARGET) $(COMMAND)

command: $(COMMAND)

test: $(TEST)

clean:
	rm -f $(OBJS) $(TARGET) $(COMMAND) $(TEST) *.o Monitoring_gui_dict.cc Monitoring_gui_dict.h
