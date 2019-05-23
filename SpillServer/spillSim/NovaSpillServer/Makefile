OBJS+=NssSpillInfo.o
OBJS+=NssUtil.o

CXXFLAGS += -std=c++11 -fPIC

all: libNovaSpillServer.so
.PHONY: all

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

libNovaSpillServer.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

clean:
	rm -f *.o *.so
.PHONY: clean

