/**
 * Program name: power, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 */

#include <boost/asio.hpp>

#include <relay/relay.h>
#include <relay/mc_relay.h>
#include <relay/ec_relay.h>

int main(int argc, char *argv[])
{
    g_elastic.init(true, false, 1); // log to stdout and use 1 threads for indexing

    if (argc != 5) {
        std::cout << "Wrong number of arguments!" << std::endl;
        return -1;
    }

    boost::asio::ip::address_v4 address = boost::asio::ip::make_address_v4(argv[2]);
    unsigned long ip = address.to_ulong();

    const std::string type{argv[1]};
    std::unique_ptr<Relay> relay;
    if(type == "mc") {
        relay = std::unique_ptr<Relay>(new MCRelay(ip));
    } else if (type == "ec") {
        relay = std::unique_ptr<Relay>(new ECRelay(ip));
    } else {
        std::cout << "Unknow relay type!" << std::endl;
        return -1;
    }

    int channel = atoi(argv[3]);
    int command = atoi(argv[4]);

    if (command == 0) {
        relay->off(channel);
    } else if (command == 1) {
        relay->on(channel);
    } else if (command == 2) {
        relay->pulse(channel);
    } else {
        std::cout << "Unknow command!" << std::endl;
        return -1;
    }

    relay->status();
    return 1;
}
