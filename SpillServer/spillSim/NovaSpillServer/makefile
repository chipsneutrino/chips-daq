OBJS+=NssSpillInfo.o
OBJS+=NssUtil.o

CXXFLAGS += -std=c++11

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

libNovaSpillServer.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

all: libNovaSpillServer.so

clean:
	rm *.o *.so
