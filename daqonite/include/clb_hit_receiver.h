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

#include "basic_hit_receiver.h"
#include "spill_schedule.h"

class CLBCommonHeader;
struct hit_t;

class CLBHitReceiver : public BasicHitReceiver {
public:
    explicit CLBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<SpillSchedule> spill_schedule, int opt_port);

    virtual ~CLBHitReceiver() = default;

private:
    /// Process CLB optical data packet.
    void processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine) override;

    void mineHits(const hit_t* hits_begin, std::size_t n_hits, const tai_timestamp& base_time, std::uint32_t plane_number);
};
