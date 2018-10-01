CC=g++

INCDIRS = 		../../fh_message/include ../include
LIBDIRS = 		../../fh_message/src
LIBS = 			fh_message boost_system boost_chrono boost_program_options m

CXXFLAGS =		$(INCDIRS:%=-I%) -O2 -g -Wall -fmessage-length=0

ROOTFLAGS = 	`root-config --cflags --glibs`

OBJS =          daqonite.o daq_handler.o clb_handler.o bbb_handler.o monitoring_plots.o daqonite_gui.o

rootcint:
	rootcint -f daqonite_gui_dict.cc -c ../include/daqonite_gui.h daqonite_gui_linkdef.h

TARGET =        daqonite

TEST = 			bbb_testServer

$(TARGET): $(OBJS) *.cc ../include/*.h
	$(CC) -o $(TARGET) daqonite_gui_dict.cc $(OBJS) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS)

$(TEST): $(OBJS) bbb_testServer.o
	$(CC) -o $(TEST) bbb_testServer.o $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(ROOTFLAGS) 
	
all: rootcint $(TARGET) $(TEST)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST) *.o daqonite_gui_dict.cc daqonite_gui_dict.h
