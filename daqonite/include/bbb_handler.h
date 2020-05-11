/**
 * BBBHandler - Handler class for the BBB data stream
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

class BBBHandler : public HitReceiver {
public:
    BBBHandler(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port, int handler_id);

    ~BBBHandler() = default;

protected:
    /// Process CLB optical data packet.
    bool processPacket(const char* datagram, std::size_t size) override;

    std::uint32_t next_sequence_number_;
};
