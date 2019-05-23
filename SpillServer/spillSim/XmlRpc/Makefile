OBJS+=XmlRpcClient.o
OBJS+=XmlRpcDispatch.o
OBJS+=XmlRpcServer.o
OBJS+=XmlRpcServerConnection.o
OBJS+=XmlRpcServerMethod.o
OBJS+=XmlRpcSocket.o
OBJS+=XmlRpcSource.o
OBJS+=XmlRpcUtil.o
OBJS+=XmlRpcValue.o

CXXFLAGS += -std=c++11 -fPIC

all: libXmlRpc.so
.PHONY: all

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

libXmlRpc.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

clean:
	rm -f *.o *.so
.PHONY: clean

