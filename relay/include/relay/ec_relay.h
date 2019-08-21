/**
 * ECRelay - Relay control for IA relay boards in the Nikhef electronics containers
 */

#pragma once

#include <relay/relay.h>
#include <util/elastic_interface.h>

#define IA_3K_PORT 23

class ECRelay: public Relay {
public:
    /// Create a ECRelay, calling Relay constructor
    ECRelay(int ip);

    /// Destroy a ECRelay
    ~ECRelay() {};

    /// Turn on a relay channel
    void on(int channel);

    /// Turn off a relay channel
    void off(int channel);

    /// Get the channel status from the relay
    void status();
};
