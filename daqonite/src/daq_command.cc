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

    if (argc != 2) {
        std::cout << "Error: You need to enter one command!" << std::endl;
        return -1;
    }

	boost::asio::io_service io_service;
	DAQ_command client(io_service, "localhost", "1096");

    std::cout << "Sending command -> " << argv[1] << std::endl;
    client.send(argv[1]);
    return 0;
}