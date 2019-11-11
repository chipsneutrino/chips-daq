/**
 * Program name: power, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 */

#include <boost/asio.hpp>

#include <relay/relay.h>
#include <relay/mc_relay.h>
#include <relay/ec_relay.h>

#define PULSE_ATTEMPTS 3

unsigned long junction_box_ips[2] = {3232238345, 3232238346};

int power_channels[6][4] = {{1,  2,  3,  4},
                            {5,  6,  7,  8},
                            {9,  10, 11, 12},
                            {13, 14, 15, 16},
                            {17, 18, 19, 20},
                            {1,  2, 19, 20}};

int power_relay[6] = {1, 1, 1, 1, 1, 0};

int pulse_channels[6][2] = {{17, 14},
                            {13, 12},
                            {11, 10},
                            {8,  7},
                            {6,  15},
                            {5,  16}};

int pulse_relay[6] = {0, 0, 0, 0, 0, 0};

void boxOn(int box);
void boxOff(int box);

int main(int argc, char *argv[])
{
    g_elastic.init(true, false, 1); // log to stdout and use 1 threads for indexing

    // Get the type of command
    const std::string type{argv[1]};

    if(type == "box") // We want to turn on/off an entire box
    {

        if (argc != 4) {
            std::cout << "Wrong number of arguments!" << std::endl;
            return -1;
        }

        int channel = atoi(argv[2]);
        int command = atoi(argv[3]);       

        if (command == 0) {
            boxOff(channel);
        } else if (command == 1) {
            boxOn(channel);
        } else {
            std::cout << "Unknown command!" << std::endl;
            return -1;
        }       
    }
    else // We are controlling a specific relay
    {
        if (argc != 5) {
            std::cout << "Wrong number of arguments!" << std::endl;
            return -1;
        }

        // Get the ip address of the relay
        boost::asio::ip::address_v4 address = boost::asio::ip::make_address_v4(argv[2]);
        unsigned long ip = address.to_ulong();

        std::unique_ptr<Relay> relay;
        if(type == "mc") {
            relay = std::unique_ptr<Relay>(new MCRelay(ip));
        } else if (type == "ec") {
            relay = std::unique_ptr<Relay>(new ECRelay(ip));
        } else {
            std::cout << "Unknown relay type!" << std::endl;
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
            std::cout << "Unknown command!" << std::endl;
            return -1;
        }

        relay->status(); // Print out the relay status
    }

    return 1;
}

void boxOn(int box)
{
    MCRelay jb_relay_1(junction_box_ips[0]);
    MCRelay jb_relay_2(junction_box_ips[1]);

    // Power on the 4 output channels
    for(int i=0; i<4; i++)
    {
        std::cout << power_channels[box-1][i] << "(" << power_relay[box-1] << ") on" << std::endl;
        if(power_relay[box-1] == 0) jb_relay_1.on(power_channels[box-1][i]);
        else jb_relay_2.on(power_channels[box-1][i]);
    }

    // Pulse the two control relays
    for(int attempts=0; attempts<PULSE_ATTEMPTS; attempts++)
    {
        for(int i=0; i<2; i++)
        {
            sleep(1);
            std::cout << pulse_channels[box-1][i] << "(" << pulse_relay[box-1] << ") pulse" << std::endl;
            if(pulse_relay[box-1] == 0) jb_relay_1.pulse(pulse_channels[box-1][i]);
            else jb_relay_2.pulse(pulse_channels[box-1][i]);
        }
    }

    jb_relay_1.status(); // Print out the relay status
    jb_relay_2.status(); // Print out the relay status
}

void boxOff(int box)
{
    MCRelay jb_relay_1(junction_box_ips[0]);
    MCRelay jb_relay_2(junction_box_ips[1]);

    // Power on the 4 output channels
    for(int i=0; i<4; i++)
    {
        std::cout << power_channels[box-1][i] << "(" << power_relay[box-1] << ") off" << std::endl;
        if(power_relay[box-1] == 0) jb_relay_1.off(power_channels[box-1][i]);
        else jb_relay_2.off(power_channels[box-1][i]);
    }

    jb_relay_1.status(); // Print out the relay status
    jb_relay_2.status(); // Print out the relay status
}
