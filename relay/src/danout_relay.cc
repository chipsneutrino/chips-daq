/**
 * DanoutRelay - Relay control for the Danout board
 */

#include <relay/danout_relay.h>

DanoutRelay::DanoutRelay(int ip, int port)
    : Relay(ip, port)
{
    setUnitName("DanoutRelay[{}]", ip_);
    log(INFO, "Creating DanoutRelay({})", ip_); 
}

void DanoutRelay::on(int channel)
{
    //TODO
}

void DanoutRelay::pulse(int channel)
{
    //TODO
}

void DanoutRelay::off(int channel)
{
    //TODO    
}

void DanoutRelay::status()
{
    //TODO    
}



