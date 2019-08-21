/**
 * ECRelay - Relay control for IA relay boards in the Nikhef electronics containers
 */

#include <relay/ec_relay.h>

ECRelay::ECRelay(int ip)
    : Relay(ip, IA_3K_PORT)
{
    g_elastic.log(INFO, "Created ECRelay({})", ip_); 
}

void ECRelay::on(int channel)
{
    //TODO
}

void ECRelay::off(int channel)
{
    //TODO    
}

void ECRelay::status()
{
    //TODO    
}


