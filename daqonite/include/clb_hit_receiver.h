/**
 * CLBHitReceiver - Hit receiver class for the CLB optical data stream
 * 
 * This class deals with the specifics of the CLB data stream, unpacking
 * the UDP binary stream into the actual data and storing into .root
 * files.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>

#include "data_handler.h"
#include "hit_receiver.h"

/// The default port for CLB UDP optical data
const static unsigned int default_opto_port = 56015;

class CLBCommonHeader;
struct hit_t;

class CLBHitReceiver : public HitReceiver {
public:
    explicit CLBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port, int handler_id);

    virtual ~CLBHitReceiver() = default;

private:
    /// Process CLB optical data packet.
    bool processPacket(const char* datagram, std::size_t size) override;
};
