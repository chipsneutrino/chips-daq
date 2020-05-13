/**
 * BBBHitReceiver - Hit receiver class for the BBB optical data stream
 */

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "bbb_hit_receiver.h"

BBBHitReceiver::BBBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, int opt_port)
    : HitReceiver { io_service, data_handler, opt_port, sizeof(opt_packet_header_t), sizeof(opt_packet_hit_t) }
{
    setUnitName("BBBHitReceiver[{}]", opt_port);
}

void BBBHitReceiver::processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine)
{
    // Cast the beggining of the packet to the opt_packet_header_t
    const auto& header { *reinterpret_cast<const opt_packet_header_t*>(datagram) };
    log(DEBUG, "Have optical header with run = {},\t plane = {},\t seq = {},\t hits = {}.",
        header.common.run_number, header.common.plane_number, header.common.sequence_number, header.hit_count);

    if (header.hit_count != n_hits) {
        log(WARNING, "Observed inconsistent hit counts (datagram reports {} hits but contains {} instead)",
            header.hit_count, n_hits);
        reportBadDatagram();
        return;
    }

    hit new_hit {};

    // Assign the variables we need from the header
    new_hit.plane_number = header.common.plane_number;

    static constexpr std::uint32_t TICKS_PER_S { 100000000 }; // TODO: move this to packet.h
    const tai_timestamp datagram_start_time {
        header.common.window_start.ticks_since_year / TICKS_PER_S, // FIXME: add year's seconds
        static_cast<std::uint32_t>(header.common.window_start.ticks_since_year % TICKS_PER_S) // FIXME: tick == ns?
    };

    if (!checkAndIncrementSequenceNumber(header.common.sequence_number, datagram_start_time)) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    // FIXME: timestamps
    reportGoodDatagram(header.common.plane_number, datagram_start_time, tai_timestamp {}, n_hits);

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
