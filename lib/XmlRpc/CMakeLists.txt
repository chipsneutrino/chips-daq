add_library(XmlRpc SHARED 
    include/base64.h
    include/XmlRpc.h
    include/XmlRpcClient.h                src/XmlRpcClient.cpp
    include/XmlRpcDispatch.h              src/XmlRpcDispatch.cpp
    include/XmlRpcException.h
    include/XmlRpcServer.h                src/XmlRpcServer.cpp
    include/XmlRpcServerConnection.h      src/XmlRpcServerConnection.cpp
    include/XmlRpcServerMethod.h          src/XmlRpcServerMethod.cpp
    include/XmlRpcSocket.h                src/XmlRpcSocket.cpp
    include/XmlRpcSource.h                src/XmlRpcSource.cpp
    include/XmlRpcUtil.h                  src/XmlRpcUtil.cpp
    include/XmlRpcValue.h                 src/XmlRpcValue.cpp
    )
target_include_directories(XmlRpc PUBLIC include)
