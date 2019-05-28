OBJS+=TimingUtilities.o

CXXFLAGS += -std=c++11 -fPIC
LDFLAGS += -L $(BOOST_PATH)/lib -lboost_date_time

all: libNovaTimingUtilities.so
.PHONY: all

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

libNovaTimingUtilities.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

clean:
	rm -f *.o *.so
.PHONY: clean

