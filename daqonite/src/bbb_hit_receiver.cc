/**
 * BBBHitReceiver - Hit receiver class for the BBB optical data stream
 */

#include <boost/bind.hpp>

#include <util/elastic_interface.h>

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
        log(WARNING, "Received packet with invalid body (expected multiple of {}, got {} which has remainder {})",
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
    CLBEvent new_event {};

    // Assign the variables we need from the header
    new_event.PomId = header.common.plane_number;
    new_event.Timestamp_s = header.common.window_start.ticks_since_year / 100000000; // FIXME: add year's ticks
    const std::uint32_t time_stamp_ns = header.common.window_start.ticks_since_year % 100000000;

    if (header.common.sequence_number > next_sequence_number_) {
        // We missed some datagrams.
        // FIXME: timestamp
        reportDataStreamGap(42);
    } else if (header.common.sequence_number < next_sequence_number_) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    next_sequence_number_ = 1 + header.common.sequence_number;

    // FIXME: timestamps
    reportGoodDatagram(header.common.plane_number, 42, 42, n_hits);

    if (!do_mine) {
        return;
    }

    data_handler_->updateLastApproxTimestamp(new_event.Timestamp_s);
    CLBEventMultiQueue* multi_queue = data_handler_->findCLBOpticalQueue(new_event.Timestamp_s + 1e-9 * time_stamp_ns, dataSlotIndex());

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
    CLBEventQueue& event_queue = multi_queue->get_queue_for_writing(new_event.PomId);

    // Find the number of hits this packet contains and loop over them all
    event_queue.reserve(event_queue.size() + n_hits);
    auto hit { reinterpret_cast<const opt_packet_hit_t*>(datagram + sizeof(opt_packet_header_t)) };
    for (int i = 0; i < n_hits; ++i, ++hit) { // TODO: find a way to unroll and vectorize this loop
        // Assign the hit channel
        new_event.Channel = 1 + (0x0F & hit->channel_and_flags);

        new_event.Timestamp_ns = time_stamp_ns;
        new_event.Timestamp_ns += 10 * hit->timestamp; // FIXME: verify unit

        new_event.SortKey = new_event.Timestamp_s + 1e-9 * new_event.Timestamp_ns;

        // Assign the hit TOT
        new_event.Tot = hit->tot;

        // FIXME: adc0

        log(DEBUG, "Have hit with channel = {},\t timestamp = [{}, {}, {}],\t ToT = {},\t ADC0 = {}.", new_event.Channel, header.common.window_start.year, new_event.Timestamp_s, new_event.Timestamp_ns, new_event.Tot, hit->adc0);

        event_queue.emplace_back(std::cref(new_event));
    }
}
