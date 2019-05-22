OBJS+=TimingUtilities.o

CXXFLAGS += -std=c++11
LDFLAGS += -lboost_date_time

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

libNovaTimingUtilities.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

all: libNovaTimingUtilities.so

clean:
	rm *.o *.so
