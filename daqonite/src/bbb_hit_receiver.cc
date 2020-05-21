/**
 * BBBHitReceiver - Hit receiver class for the BBB optical data stream
 */

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "bbb_hit_receiver.h"

BBBHitReceiver::BBBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<SpillSchedule> spill_schedule, int opt_port)
    : BasicHitReceiver { io_service, spill_schedule, opt_port, sizeof(opt_packet_header_t), sizeof(opt_packet_hit_t), false }
{
    setUnitName("BBBHitReceiver[{}]", opt_port);
}

void BBBHitReceiver::processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine)
{
    const auto& common_header { *reinterpret_cast<const packet_common_header_t*>(datagram) };

    // Check the type of the packet is optical
    if (common_header.packet_type != UDP_PACKET_TYPE_OPTICAL) {
        log(WARNING, "Received non-optical packet (expected type {}, got {} instead)",
            UDP_PACKET_TYPE_OPTICAL, common_header.packet_type);
        reportBadDatagram();
        return;
    }

    // Cast the beggining of the packet to the opt_packet_header_t
    const auto& header { *reinterpret_cast<const opt_packet_header_t*>(datagram) };

    if (header.hit_count != n_hits) {
        log(WARNING, "Observed inconsistent hit counts (datagram reports {} hits but contains {} instead)",
            header.hit_count, n_hits);
        reportBadDatagram();
        return;
    }

    const tai_timestamp base_time { header.common.window_start.secs, header.common.window_start.nanosecs };
    const std::uint32_t plane_number { header.common.plane_number };

    if (!checkAndIncrementSequenceNumber(plane_number, header.common.sequence_number, base_time)) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    // Peek at the timestamps of the first and the last hit in the datagram.
    tai_timestamp datagram_first_timestamp { base_time };
    tai_timestamp datagram_last_timestamp { base_time };
    const opt_packet_hit_t* hits_begin { reinterpret_cast<const opt_packet_hit_t*>(datagram + sizeof(opt_packet_header_t)) };

    if (n_hits > 0) {
        datagram_first_timestamp = calculateHitTime(hits_begin[0], base_time);
        datagram_last_timestamp = calculateHitTime(hits_begin[n_hits - 1], base_time);
    }

    reportGoodDatagram(header.common.plane_number, datagram_first_timestamp, datagram_last_timestamp, n_hits);

    if (do_mine) {
        mineHits(hits_begin, n_hits, base_time, plane_number);
    }
}

void BBBHitReceiver::mineHits(const opt_packet_hit_t* hits_begin, std::size_t n_hits, const tai_timestamp& base_time, std::uint32_t plane_number)
{
    SpillDataSlot* found_slot {};
    if (!(found_slot = findAndLockDataSlot(base_time))) {
        // Have no slot to store the hits, discard datagram.
        return;
    }

    // The slot will be automatically unlocked at the end of this scope.
    SpillDataSlot& slot { *found_slot };
    std::lock_guard<std::mutex> l { slot.mutex, std::adopt_lock };

    // Find/create queue for this plane
    PMTHitQueue& event_queue { slot.opt_hit_queue.get_queue_for_writing(plane_number) };

    // Find the number of hits this packet contains and loop over them all
    event_queue.reserve(event_queue.size() + n_hits);
    const opt_packet_hit_t* hits_end { hits_begin + n_hits };
    for (auto hits_it = hits_begin; hits_it != hits_end; ++hits_it) { // NOTE: vectorize this loop?
        // To avoid unnecessary copying, we first add the hit and then set its values later.
        // We can do this because we are holding on to the mutex.
        event_queue.emplace_back();
        PMTHit& dest_hit { event_queue.back() };

        const opt_packet_hit_t& src_hit { *hits_it };

        // Assign hit fields
        dest_hit.plane_number = plane_number;
        dest_hit.channel_number = 1 + (0x0F & src_hit.channel_and_flags);
        dest_hit.tot = src_hit.tot;
        dest_hit.adc0 = src_hit.adc0;
        dest_hit.timestamp = calculateHitTime(src_hit, base_time);
        dest_hit.cpu_trigger = 0 != (OPT_PACKET_HIT_CPU_TRIGGER_FLAG & src_hit.channel_and_flags);

        dest_hit.sort_key = dest_hit.timestamp.combined_secs();
    }
}

tai_timestamp BBBHitReceiver::calculateHitTime(const opt_packet_hit_t& hit, const tai_timestamp& base_time)
{
    // Hit time is expressed as [ns] offset w.r.t. a base timestamp.
    tai_timestamp timestamp { base_time };
    timestamp.nanosecs += hit.timestamp;
    timestamp.normalise();

    return timestamp;
}
