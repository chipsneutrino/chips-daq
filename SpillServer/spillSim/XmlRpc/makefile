OBJS+=XmlRpcClient.o
OBJS+=XmlRpcDispatch.o
OBJS+=XmlRpcServer.o
OBJS+=XmlRpcServerConnection.o
OBJS+=XmlRpcServerMethod.o
OBJS+=XmlRpcSocket.o
OBJS+=XmlRpcSource.o
OBJS+=XmlRpcUtil.o
OBJS+=XmlRpcValue.o

CXXFLAGS += -std=c++11

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

libXmlRpc.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

all: libXmlRpc.so

clean:
	rm *.o *.so	
