OBJS+=fakeSpillSender.o

CXXFLAGS += -std=c++11
LDFLAGS += -l NovaSpillServer -L ./NovaSpillServer
LDFLAGS += -l NovaTimingUtilities -L ./NovaTimingUtilities
LDFLAGS += -l XmlRpc -L ./XmlRpc
LDFLAGS += -l boost_date_time
LDFLAGS += -l boost_program_options

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

fakeSpillSender: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS)

all: fakeSpillSender

clean:
	rm *.o fakeSpillSender
