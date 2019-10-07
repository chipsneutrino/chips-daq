/**
 * ECRelay - Relay control for IA relay boards in the Nikhef electronics containers
 */

#include <relay/ec_relay.h>

ECRelay::ECRelay(int ip)
    : Relay(ip, PORT_IA_3K)
{
    g_elastic.log(INFO, "Created ECRelay({})", ip_);  

    fcntl(fd_, F_SETFL, O_NONBLOCK);
    char buffer[256];
    int n = read(fd_, buffer, 255);
}

void ECRelay::on(int channel)
{
    char relnum[2];
    sprintf(relnum, "%02X", (channel-1));
    char msg[7] = {0x21, 0x30, 0x30, 0x33, relnum[0], relnum[1], 0x0D};

	int n = send(fd_, msg, sizeof(msg), 0);
    if (n < 0) g_elastic.log(ERROR, "ECRelay ({}) could not send on command!", ip_); 

    usleep(DELAY_IA_3K);

    char buffer[256];
    n = read(fd_, buffer, 255);
    if (n != 5) g_elastic.log(ERROR, "ECRelay ({}) could not read on response!", ip_);
}

void ECRelay::pulse(int channel)
{
    //TODO
}

void ECRelay::off(int channel)
{
    char relnum[2];
    sprintf(relnum, "%02X", (channel-1));
    char msg[7] = {0x21, 0x30, 0x30, 0x34, relnum[0], relnum[1], 0x0D};

	int n = send(fd_, msg, sizeof(msg), 0);
    if (n < 0) g_elastic.log(ERROR, "ECRelay ({}) could not send off command!", ip_); 

    usleep(DELAY_IA_3K);

    char buffer[256];
    n = read(fd_, buffer, 255);
    if (n != 5) g_elastic.log(ERROR, "ECRelay ({}) could not read off response!", ip_);
}

void ECRelay::status()
{
    char msg[5] = {0x3F, 0x30, 0x30, 0x32, 0x0D};

    int n = send(fd_, msg, sizeof(msg), 0);
    if (n < 0) g_elastic.log(ERROR, "ECRelay ({}) could not send status command!", ip_); 

    usleep(DELAY_IA_3K);

    char buffer[256];
    n = read(fd_, buffer, 255);
    if (n != 10) g_elastic.log(ERROR, "ECRelay ({}) could not read status response!", ip_); 

    std::cout << n << std::endl;
    //std::bitset<32> bits(stol(std::string(buffer, 1, 8)));
    //std::cout << "Status: " << bits.to_string() << std::endl;   
}


