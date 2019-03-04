/**
 * DAQommand - Application to control DAQonite
 * 
 * Sends commands over a local UDP socket to a DAQonite application to
 * start and stop runs etc...
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class DAQommand {
    public:

        /// Create a DAQommand object
        DAQommand(boost::asio::io_service& io_service, 
                    const std::string& host, 
                    const std::string& port) : 
                    io_service_(io_service), 
                    socket_(io_service, udp::endpoint(udp::v4(), 0)) {
                    udp::resolver resolver(io_service_);
                    udp::resolver::query query(udp::v4(), host, port);
                    udp::resolver::iterator iter = resolver.resolve(query);
                    endpoint_ = *iter; }

        /// Destroy a DAQommand object
        ~DAQommand() {
            socket_.close();
        }

        // Send a message 
        void send(const std::string& msg) {
            socket_.send_to(boost::asio::buffer(msg, msg.size()), endpoint_);
        }

    private:
	    boost::asio::io_service& io_service_;   ///< BOOST io_service
	    udp::socket socket_;                    ///< Local UDP socket to communicate with DAQonite
	    udp::endpoint endpoint_;                ///< Local UDP endpoint
};

int main(int argc, char** argv) {
    // Make the IO service and DAQommand object
    boost::asio::io_service io_service;
	DAQommand client(io_service, "localhost", "1096"); // Hardcode port 1096 for now

    // Check the command and additional arguments are valid
    if (argc < 2 || argc > 3) {
        std::cout << "DAQommand - Error: You need to enter a valid command!" << std::endl;
        std::cout << "DAQommand - Options: start [runType], new [runType], stop, exit" << std::endl;
        return -1;        
    } else {
    	if (strncmp(argv[1], "start", 5) == 0 && argc == 3) {
            if (((int)*argv[2]-48) >= 4 || ((int)*argv[2]-48) < 0) {
                std::cout << "DAQommand - Error: - Error: Invalid run type!" << std::endl;
                return -1;        
            }
            std::cout << "DAQommand - Sending start command with run type -> " << argv[2] << std::endl;
            char *combined = new char[strlen(argv[1]) + strlen(argv[2]) + 1];
            strcpy(combined, argv[1]);
            strcat(combined, argv[2]);
            client.send(combined);
            delete combined;

		} else if (strncmp(argv[1], "stop", 4) == 0) {
            std::cout << "DAQommand - Sending stop command... " << std::endl;
            client.send(argv[1]);

		} else if (strncmp(argv[1], "exit", 4) == 0) {
            std::cout << "DAQommand - Sending exit command... " << std::endl;
            client.send(argv[1]);

		} else {
			std::cout << "DAQommand - Error: You need to enter a valid command!" << std::endl;
            std::cout << "DAQommand - Options: start [runType], new [runType], stop, exit" << std::endl;
            return -1;  
		}
    }
    return 0;
}