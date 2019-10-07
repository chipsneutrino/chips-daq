/**
 * ECRelay - Relay control for IA relay boards in the Nikhef electronics containers
 */

#pragma once

#include <relay/relay.h>
#include <util/elastic_interface.h>

#define PORT_IA_3K 23
#define DELAY_IA_3K 50000

class ECRelay: public Relay {
public:
    /// Create a ECRelay, calling Relay constructor
    ECRelay(int ip);

    /// Destroy a ECRelay
    ~ECRelay() {};

    /// Turn on a relay channel
    void on(int channel);

    // Pulse a relay channel
    void pulse(int channel);

    /// Turn off a relay channel
    void off(int channel);

    /// Get the channel status from the relay
    void status();
};
