/**
 * MCRelay - Relay control for 8020 relay boards in the master container
 */

#pragma once

#include <relay/relay.h>
#include <util/elastic_interface.h>

#define PORT_8020 17494

class MCRelay: public Relay {
public:
    /// Create a MCRelay, calling Relay constructor
    MCRelay(int ip);

    /// Destroy a MCRelay
    ~MCRelay() {};

    /// Turn on a relay channel
    void on(int channel);

    /// Turn off a relay channel
    void off(int channel);

    /// Get the channel status from the relay
    void status();
};
