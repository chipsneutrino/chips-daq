#include <relay/relay.h>
#include <relay/mc_relay.h>
#include <relay/ec_relay.h>

int main(int argc, char *argv[])
{
    // Initialise the elasticsearch interface.
    g_elastic.init(true, false, 1); // log to stdout and use 1 threads for indexing

    MCRelay mc_relay(3232238338); // Create a master container relay
    mc_relay.status();
    mc_relay.on(1);
    mc_relay.status();
    mc_relay.off(1);
    mc_relay.status();

    ECRelay ec_relay(3232238381); // Create a electronics container relay

    return 1;
}