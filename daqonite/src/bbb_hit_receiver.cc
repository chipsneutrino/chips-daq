/**
 * BBBHitReceiver - Hit receiver class for the BBB optical data stream
 */

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "bbb_hit_receiver.h"

BBBHitReceiver::BBBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<SpillSchedule> spill_schedule, int opt_port)
    : BasicHitReceiver { io_service, spill_schedule, opt_port, sizeof(opt_packet_header_t), sizeof(opt_packet_hit_t) }
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

    const tai_timestamp datagram_start_time { header.common.window_start.secs, header.common.window_start.nanosecs };
    const std::uint32_t plane_number { header.common.plane_number };

    if (!checkAndIncrementSequenceNumber(plane_number, header.common.sequence_number, datagram_start_time)) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    // FIXME: timestamps
    reportGoodDatagram(header.common.plane_number, datagram_start_time, tai_timestamp {}, n_hits);

    if (do_mine) {
        const opt_packet_hit_t* hits_begin { reinterpret_cast<const opt_packet_hit_t*>(datagram + sizeof(opt_packet_header_t)) };
        mineHits(hits_begin, n_hits, datagram_start_time, plane_number);
    }
}

void BBBHitReceiver::mineHits(const opt_packet_hit_t* hits_begin, std::size_t n_hits, const tai_timestamp& base_time, std::uint32_t plane_number)
{
    // TODO: perhaps use the lowest hit time in datagram here instead?
    SpillDataSlot* slot { spill_schedule_->findDataSlot(base_time, dataSlotIndex()) };

    if (!slot) {
        // Timestamp not matched to any open batch, discard packet.
        // TODO: devise a reporting mechanism for this
        return;
    }

    // From this point on, the queue is being written into, and the batch cannot be closed until the end of scope.
    std::lock_guard<std::mutex> l { slot->mutex };

    if (slot->closed_for_writing) {
        // Have a batch, which has been closed but not yet removed from the schedule. Discard packet.
        // TODO: devise a reporting mechanism for this
        return;
    }

    // Find/create queue for this POM
    PMTHitQueue& event_queue { slot->opt_hit_queue.get_queue_for_writing(plane_number) };

    // Find the number of hits this packet contains and loop over them all
    event_queue.reserve(event_queue.size() + n_hits);
    const opt_packet_hit_t* hits_end { hits_begin + n_hits };
    for (auto hits_it = hits_begin; hits_it != hits_end; ++hits_it) { // NOTE: vectorize this loop?
        // To avoid unnecessary copying, we first add the hit and then set its values later.
        // We can do this because we are holding on to the mutex.
        event_queue.emplace_back();
        PMTHit& dest_hit { event_queue.back() };

        const opt_packet_hit_t& src_hit { *hits_it };

        // Assign basic hit fields
        dest_hit.plane_number = plane_number;
        dest_hit.channel_number = 1 + (0x0F & src_hit.channel_and_flags);
        dest_hit.tot = src_hit.tot;
        dest_hit.adc0 = src_hit.adc0;

        // TODO: use flags

        // Assign hit timestamp
        dest_hit.timestamp = base_time;
        dest_hit.timestamp.nanosecs += src_hit.timestamp; // FIXME: timestamp in ns?
        dest_hit.timestamp.normalise();

        dest_hit.sort_key = dest_hit.timestamp.combined_secs();
    }
}
