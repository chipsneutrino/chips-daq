/**
 * BBBHandler - Handler class for the BBB optical data stream
 */

#include <boost/bind.hpp>

#include <util/elastic_interface.h>

#include "bbb_handler.h"

BBBHandler::BBBHandler(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port, int handler_id)
    : data_handler_ { std::move(data_handler) }
    , mode_ { mode }
    , buffer_size_ { buffer_size_bbb_opt }
    , socket_optical_ { *io_service, udp::endpoint(udp::v4(), opt_port) }
    , data_slot_idx_ { data_handler_->assignNewSlot() }
    , handler_id_ { handler_id }
    , next_sequence_number_ { 0 }
{
    // Setup the sockets
    socket_optical_.set_option(udp::socket::receive_buffer_size { 33554432 });

    g_elastic.log(INFO, "BBB Handler {} started on port {}", handler_id_, opt_port);
}

void BBBHandler::workOpticalData()
{
    using namespace boost::asio::placeholders;
    socket_optical_.async_receive(boost::asio::buffer(&buffer_optical_[0], buffer_size_),
        boost::bind(&BBBHandler::handleOpticalData, this, error, bytes_transferred));
}

void BBBHandler::handleOpticalData(const boost::system::error_code& error, std::size_t size)
{
    if (error) {
        g_elastic.log(ERROR, "BBB Handler {} caught error {}: {}", handler_id_, error.value(), error.category().name());
        g_elastic.log(WARNING, "BBB Handler {} stopping work on optical socket due to error.", handler_id_);
        return;
    }

    if (*mode_ != true) {
        // We are not in a run so just call the work method again
        workOpticalData();
        return;
    }

    // Check the packet has at least a BBB header in it
    if (size < sizeof(opt_packet_header_t)) {
        g_elastic.log(WARNING, "BBB Handler {} received packet without header (expected at least {}, got {})",
            handler_id_, sizeof(opt_packet_header_t), size);
        workOpticalData();
        return;
    }

    // Check the size of the packet is consistent with opt_packet_header_t + some hits
    const std::size_t remaining_bytes = size - sizeof(opt_packet_header_t);
    const std::ldiv_t div = std::div((long)remaining_bytes, sizeof(opt_packet_hit_t));
    if (div.rem != 0) {
        g_elastic.log(WARNING, "BBB Handler {} received packet with invalid body (expected multiple of {}, got {} which has remainder {})",
            handler_id_, sizeof(opt_packet_hit_t), remaining_bytes, div.rem);
        workOpticalData();
        return;
    }

    // Cast the beggining of the packet to the opt_packet_header_t
    const opt_packet_header_t& header_optical = *reinterpret_cast<const opt_packet_header_t*>(buffer_optical_);

    const auto& header { *reinterpret_cast<const opt_packet_header_t*>(buffer_optical_) };
    g_elastic.log(DEBUG, "Have optical header with run = {},\t plane = {},\t seq = {},\t hits = {},\t data_hits = {}.",
        header.common.run_number, header.common.plane_number, header.common.sequence_number, header.hit_count, div.quot);

    if (header.common.sequence_number != next_sequence_number_) {
        g_elastic.log(WARNING, "Received a packet out of order (or after a gap), expected sequence number {}, got {} instead", next_sequence_number_, header.common.sequence_number);
    }
    next_sequence_number_ = 1 + header.common.sequence_number;

    const std::uint32_t n_hits = std::min((std::uint32_t)div.quot, header.hit_count);
    const opt_packet_hit_t* hit_start = reinterpret_cast<const opt_packet_hit_t*>(buffer_optical_ + sizeof(opt_packet_header_t));
    processPacket(header, n_hits, hit_start);
    workOpticalData();
}

bool BBBHandler::processPacket(const opt_packet_header_t& header, std::uint32_t n_hits, const opt_packet_hit_t* hit) const
{
    const opt_packet_hit_t* hit_ptr { reinterpret_cast<const opt_packet_hit_t*>(buffer_optical_ + sizeof(opt_packet_header_t)) };

    CLBEvent new_event {};

    // Assign the variables we need from the header
    new_event.PomId = header.common.plane_number;
    new_event.Timestamp_s = header.common.window_start.ticks_since_year / 100000000; // FIXME: add year's ticks
    const std::uint32_t time_stamp_ns = header.common.window_start.ticks_since_year % 100000000;

    data_handler_->updateLastApproxTimestamp(new_event.Timestamp_s);
    CLBEventMultiQueue* multi_queue = data_handler_->findCLBOpticalQueue(new_event.Timestamp_s + 1e-9 * time_stamp_ns, data_slot_idx_);

    if (!multi_queue) {
        // Timestamp not matched to any open batch, discard packet.
        return false;
    }

    std::lock_guard<std::mutex> l { multi_queue->write_mutex };
    if (multi_queue->closed_for_writing) {
        // Have a batch, which has been closed but not yet removed from the schedule. Discard packet.
        return false;
    }

    // Find/create queue for this POM
    CLBEventQueue& event_queue = multi_queue->get_queue_for_writing(new_event.PomId);

    // Find the number of hits this packet contains and loop over them all
    event_queue.reserve(event_queue.size() + n_hits);
    for (int i = 0; i < n_hits; ++i, ++hit) { // TODO: find a way to unroll and vectorize this loop
        // Assign the hit channel
        new_event.Channel = 1 + (0x0F & hit->channel_and_flags);

        new_event.Timestamp_ns = time_stamp_ns;
        new_event.Timestamp_ns += 10 * hit->timestamp; // FIXME: verify unit

        new_event.SortKey = new_event.Timestamp_s + 1e-9 * new_event.Timestamp_ns;

        // Assign the hit TOT
        new_event.Tot = hit->tot;

        // FIXME: adc0

        g_elastic.log(DEBUG, "Have hit with channel = {},\t timestamp = [{}, {}, {}],\t ToT = {},\t ADC0 = {}.", new_event.Channel, header.common.window_start.year, new_event.Timestamp_s, new_event.Timestamp_ns, new_event.Tot, hit->adc0);

        event_queue.emplace_back(std::cref(new_event));
    }

    return true;
}
