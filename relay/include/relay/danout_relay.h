/**
 * DanoutRelay - Relay control for the Danout board
 */

#pragma once

#include <relay/relay.h>
#include <util/elastic_interface.h>

class DanoutRelay: public Relay {
public:
    /// Create a DanoutRelay, calling Relay constructor
    DanoutRelay(int ip, int port);

    /// Destroy a DanoutRelay
    ~DanoutRelay() {};

    /// Turn on a relay channel
    void on(int channel);

    /// Turn off a relay channel
    void off(int channel);

    /// Get the channel status from the relay
    void status();
};
