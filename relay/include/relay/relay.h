/**
 * Relay - Base class for a Relay, either a Danout, 8020, IA
 */

#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

#include <boost/asio.hpp>

#include <util/logging.h>

class Relay: protected Logging {
public:
    /// Create a Relay
    Relay(int ip, int port)
        : ip_(boost::asio::ip::address_v4(ip).to_string())
        , port_(port) 
    {
        setUnitName("Relay[{}]", ip_);

        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port_);
        serv_addr.sin_addr.s_addr = inet_addr(ip_.c_str());

        char buffer[1024] = {0}; 
        if ((fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        { 
            log(ERROR, "Could not create relay socket!");
        } 
     
        if (connect(fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
            log(ERROR, "Could not connect to relay ({})", ip_); 
        }
    };

    /// Destroy a Relay
    virtual ~Relay() {}

    /// Turn on a relay channel
    virtual void on(int channel)    = 0;

    /// Pulse a relay channel
    virtual void pulse(int channel) = 0;

    /// Turn off a relay channel
    virtual void off(int channel)   = 0;

    /// Get the channel status from the relay
    virtual void status()           = 0;

protected:
    std::string ip_;    ///< IP address of the relay
    int port_;          ///< Relay port
    int fd_;            ///< File descriptor for socket
};
