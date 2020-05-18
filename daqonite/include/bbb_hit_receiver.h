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

#include "basic_hit_receiver.h"
#include "spill_schedule.h"

class BBBHitReceiver : public BasicHitReceiver {
public:
    BBBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<SpillSchedule> spill_schedule, int opt_port);

    ~BBBHitReceiver() = default;

protected:
    /// Process BBB optical data packet.
    void processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine) override;

    void mineHits(const opt_packet_hit_t* hits_begin, std::size_t n_hits, const tai_timestamp& base_time, std::uint32_t plane_number);

    static tai_timestamp calculateHitTime(const opt_packet_hit_t& hit, const tai_timestamp& base_time);
};
