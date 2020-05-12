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

class CLBCommonHeader;
struct hit_t;

class CLBHitReceiver : public HitReceiver {
public:
    explicit CLBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, int opt_port);

    virtual ~CLBHitReceiver() = default;

private:
    /// Process CLB optical data packet.
    bool processDatagram(const char* datagram, std::size_t datagram_size, bool do_mine) override;
};
