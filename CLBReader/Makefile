CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		CLBReader.o

LIBS = -lboost_system -lboost_chrono -lboost_program_options

TARGET =	CLBReader

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) -lpthread -lm

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
