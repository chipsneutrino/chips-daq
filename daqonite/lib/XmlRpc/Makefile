OBJS+=XmlRpcClient.o
OBJS+=XmlRpcDispatch.o
OBJS+=XmlRpcServer.o
OBJS+=XmlRpcServerConnection.o
OBJS+=XmlRpcServerMethod.o
OBJS+=XmlRpcSocket.o
OBJS+=XmlRpcSource.o
OBJS+=XmlRpcUtil.o
OBJS+=XmlRpcValue.o
LOBJS=$(OBJS:%=lib/%)

CXXFLAGS += -std=c++11 -fPIC
CXXFLAGS += -I include

all: lib/libXmlRpc.so
.PHONY: all

lib/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

lib/libXmlRpc.so: $(LOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(LOBJS)

clean:
	rm -f lib/*.o lib/*.so
.PHONY: clean

