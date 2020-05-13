/**
 * BBBHitReceiver - Handler class for the BBB data stream
 * 
 * This class deals with the specifics of the BBB data stream, using the
 * fh_library API, to communicate with the Madison beaglebones.
 *
 * Author: Josh Tingey, Petr Mánek
 * Contact: j.tingey.16@ucl.ac.uk, petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>

#include <bbb/packets.h>

#include "data_handler.h"
#include "hit_receiver.h"

class BBBHitReceiver : public HitReceiver {
public:
    BBBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, int opt_port);

    ~BBBHitReceiver() = default;

protected:
    /// Process BBB optical data packet.
    void processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine) override;
};
