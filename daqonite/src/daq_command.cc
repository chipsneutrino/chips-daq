#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class DAQ_command {
    public:
        DAQ_command(boost::asio::io_service& io_service, 
                    const std::string& host, 
                    const std::string& port) : 
                    io_service_(io_service), 
                    socket_(io_service, udp::endpoint(udp::v4(), 0)) {
                    udp::resolver resolver(io_service_);
                    udp::resolver::query query(udp::v4(), host, port);
                    udp::resolver::iterator iter = resolver.resolve(query);
                    endpoint_ = *iter; }

        ~DAQ_command() {
            socket_.close();
        }

        void send(const std::string& msg) {
            socket_.send_to(boost::asio::buffer(msg, msg.size()), endpoint_);
        }

    private:
	    boost::asio::io_service& io_service_;
	    udp::socket socket_;
	    udp::endpoint endpoint_;
};

int main(int argc, char** argv) {
    // Make the IO service and DAQ_command object
    boost::asio::io_service io_service;
	DAQ_command client(io_service, "localhost", "1096"); // Hardcode port 1096 for now

    // Check the command and additional arguments are valid
    if (argc < 2 || argc > 3) {
        std::cout << "Error: You need to enter a valid command!" << std::endl;
        std::cout << "'Options: start [runType], new [runType], stop, exit'" << std::endl;
        return -1;        
    } else {
    	if (strncmp(argv[1], "start", 5) == 0 && argc == 3) {
            if (((int)*argv[2]-48) >= 4 || ((int)*argv[2]-48) < 0) {
                std::cout << "Error: Invalid run type!" << std::endl;
                return -1;        
            }
            std::cout << "Sending start command with run type -> " << argv[2] << std::endl;
            char *combined = new char[strlen(argv[1]) + strlen(argv[2]) + 1];
            strcpy(combined, argv[1]);
            strcat(combined, argv[2]);
            client.send(combined);
            delete combined;

		} else if (strncmp(argv[1], "stop", 4) == 0) {
            std::cout << "Sending stop command... " << std::endl;
            client.send(argv[1]);

		} else if (strncmp(argv[1], "exit", 4) == 0) {
            std::cout << "Sending exit command... " << std::endl;
            client.send(argv[1]);

		} else {
			std::cout << "Error: You need to enter a valid command!" << std::endl;
            std::cout << "'Options: start [runType], new [runType], stop, exit'" << std::endl;
            return -1;  
		}
    }
    return 0;
}