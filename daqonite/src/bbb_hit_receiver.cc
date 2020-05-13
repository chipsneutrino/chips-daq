/**
 * BBBHitReceiver - Hit receiver class for the BBB optical data stream
 */

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "bbb_hit_receiver.h"

BBBHitReceiver::BBBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, int opt_port)
    : HitReceiver { io_service, data_handler, opt_port, sizeof(opt_packet_header_t) }
    , next_sequence_number_ { 0 }
{
    setUnitName("BBBHitReceiver[{}]", opt_port);
}

void BBBHitReceiver::startData()
{
    // Reset sequence number before we start receiving hits
    next_sequence_number_ = 0;

    HitReceiver::startData();
}

void BBBHitReceiver::processDatagram(const char* datagram, std::size_t datagram_size, bool do_mine)
{
    // Check the size of the packet is consistent with opt_packet_header_t + some hits
    const std::size_t remaining_bytes = datagram_size - sizeof(opt_packet_header_t);
    const std::ldiv_t div = std::div((long)remaining_bytes, sizeof(opt_packet_hit_t));
    if (div.rem != 0) {
        log(WARNING, "Received datagram with invalid body (expected multiple of {}, got {} which has remainder {})",
            sizeof(opt_packet_hit_t), remaining_bytes, div.rem);
        reportBadDatagram();
        return;
    }

    // Cast the beggining of the packet to the opt_packet_header_t
    const opt_packet_header_t& header_optical = *reinterpret_cast<const opt_packet_header_t*>(datagram);

    const auto& header { *reinterpret_cast<const opt_packet_header_t*>(datagram) };
    log(DEBUG, "Have optical header with run = {},\t plane = {},\t seq = {},\t hits = {}.",
        header.common.run_number, header.common.plane_number, header.common.sequence_number, header.hit_count);

    if (header.hit_count != div.quot) {
        log(WARNING, "Reported hit count differs from data size (datagram reports {} hits, buffer contains {})",
            header.hit_count, div.quot);
        reportBadDatagram();
        return;
    }

    const std::uint32_t n_hits { header.hit_count };
    hit new_hit {};

    // Assign the variables we need from the header
    new_hit.plane_number = header.common.plane_number;

    static constexpr std::uint32_t TICKS_PER_S { 100000000 }; // TODO: move this to packet.h
    const tai_timestamp datagram_start_time {
        header.common.window_start.ticks_since_year / TICKS_PER_S, // FIXME: add year's seconds
        static_cast<std::uint32_t>(header.common.window_start.ticks_since_year % TICKS_PER_S) // FIXME: tick == ns?
    };

    if (header.common.sequence_number > next_sequence_number_) {
        // We missed some datagrams. The gap ends at the start of this datagram.
        reportDataStreamGap(datagram_start_time);
    } else if (header.common.sequence_number < next_sequence_number_) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    next_sequence_number_ = 1 + header.common.sequence_number;

    // FIXME: timestamps
    reportGoodDatagram(header.common.plane_number, tai_timestamp {}, tai_timestamp {}, n_hits);

    if (!do_mine) {
        return;
    }

    data_handler_->updateLastApproxTimestamp(datagram_start_time);
    HitMultiQueue* multi_queue = data_handler_->findCLBOpticalQueue(datagram_start_time, dataSlotIndex());

    if (!multi_queue) {
        // Timestamp not matched to any open batch, discard packet.
        // TODO: devise a reporting mechanism for this
        return;
    }

    std::lock_guard<std::mutex> l { multi_queue->write_mutex };
    if (multi_queue->closed_for_writing) {
        // Have a batch, which has been closed but not yet removed from the schedule. Discard packet.
        // TODO: devise a reporting mechanism for this
        return;
    }

    // Find/create queue for this POM
    HitQueue& event_queue = multi_queue->get_queue_for_writing(new_hit.plane_number);

    // Find the number of hits this packet contains and loop over them all
    event_queue.reserve(event_queue.size() + n_hits);
    auto hit { reinterpret_cast<const opt_packet_hit_t*>(datagram + sizeof(opt_packet_header_t)) };
    for (int i = 0; i < n_hits; ++i, ++hit) { // TODO: find a way to unroll and vectorize this loop
        // Assign the hit channel
        new_hit.channel_number = 1 + (0x0F & hit->channel_and_flags);

        // TODO: use flags

        new_hit.timestamp = datagram_start_time;
        new_hit.timestamp.nanosecs += hit->timestamp; // FIXME: timestamp in ns?
        new_hit.timestamp.normalise();

        new_hit.sort_key = new_hit.timestamp.combined_secs();

        // Assign the hit TOT and ADC
        new_hit.tot = hit->tot;
        new_hit.adc0 = hit->adc0;

        log(DEBUG, "Have hit with plane = {}, channel = {},\t timestamp = {},\t ToT = {},\t ADC0 = {}.",
            new_hit.plane_number, new_hit.channel_number, new_hit.timestamp, new_hit.tot, new_hit.adc0);

        event_queue.emplace_back(std::cref(new_hit));
    }
}
