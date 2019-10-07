/**
 * MCRelay - Relay control for 8020 relay boards in the master container
 */

#include <relay/mc_relay.h>

MCRelay::MCRelay(int ip)
    : Relay(ip, PORT_8020)
{
    g_elastic.log(INFO, "Created MCRelay({})", ip_); 
}

void MCRelay::on(int channel)
{
    unsigned char msg[3] = {0x20, (unsigned char)channel, 0x00};
	int n = send(fd_, msg, sizeof(msg), 0);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not send on command!", ip_); 

    usleep(DELAY_8020);

    char buffer[256];
    n = read(fd_, buffer, 255);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not read on response!", ip_); 
}

void MCRelay::pulse(int channel)
{
    unsigned char msg[3] = {0x20, (unsigned char)channel, 0x01};
	int n = send(fd_, msg, sizeof(msg), 0);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not send pulse command!", ip_); 

    usleep(DELAY_8020);

    char buffer[256];
    n = read(fd_, buffer, 255);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not read pulse response!", ip_); 
}

void MCRelay::off(int channel)
{
    unsigned char msg[3] = {0x21, (unsigned char)channel, 0x00};
	int n = send(fd_, msg, sizeof(msg), 0);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not send off command!", ip_); 

    usleep(DELAY_8020);

    char buffer[256];
    n = read(fd_, buffer, 255);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not read off response!", ip_);
}

void MCRelay::status()
{
    unsigned char msg[1] = {0x24};
	int n = send(fd_, msg, sizeof(msg), 0);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not send status command!", ip_);  

    usleep(DELAY_8020);

    char buffer[256];
    n = read(fd_, buffer, 255);
    if (n < 0) g_elastic.log(ERROR, "MCRelay ({}) could not read status response!", ip_); 

    long tot = (0xFF & buffer[0]) + ((0xFF & buffer[1]) << 8) + ((0xFF & buffer[2]) << 16);    
    std::bitset<32> bits(tot);
    std::cout << "Status: " << bits.to_string() << std::endl; 
}


