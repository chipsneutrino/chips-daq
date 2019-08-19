/**
 * SingletonProcess - Ensures there is only one instance of the DAQ process
 * 
 * It does this by binding a socket to a specific port.
 * If the process is already running this can not be done.
 * When the program exits for any reason the socket can then be used again.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#include <netinet/in.h>
#include <stdexcept>
#include <string>

class SingletonProcess {
public:
    SingletonProcess(uint16_t port)
        : socket_fd_(-1)
        , rc_(1)
        , port_(port) {}

    ~SingletonProcess()
    {
        if (socket_fd_ != -1) close(socket_fd_);
    }

    bool operator()()
    {
        if (socket_fd_ == -1 || rc_)
        {
            socket_fd_ = -1;
            rc_ = 1;

            if ((socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            {
                throw std::runtime_error("Could not create lock socket!");
            }
            else
            {
                struct sockaddr_in name;
                name.sin_family = AF_INET;
                name.sin_port = htons (port_);
                name.sin_addr.s_addr = htonl (INADDR_ANY);
                rc_ = bind (socket_fd_, (struct sockaddr *) &name, sizeof (name));
            }
        }
        return (socket_fd_ != -1 && rc_ == 0);
    }

private:
    int socket_fd_ = -1;
    int rc_;
    uint16_t port_;
};