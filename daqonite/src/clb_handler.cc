/**
 * CLBHandler - Handler class for the CLB optical data stream
 */

#include <boost/bind.hpp>

#include <clb/data_structs.h>
#include <clb/header_structs.h>
#include <util/elastic_interface.h>

#include "clb_handler.h"

CLBHandler::CLBHandler(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port, int handler_id)
    : data_handler_{ std::move(data_handler) }
    , mode_{ mode }
    , buffer_size_{ buffer_size_opt }
    , socket_optical_{ *io_service, udp::endpoint(udp::v4(), opt_port) }
    , data_slot_idx_{ data_handler_->assignNewSlot() }
    , handler_id_{ handler_id }
{
    // Setup the sockets
    socket_optical_.set_option(udp::socket::receive_buffer_size{ 33554432 });

    g_elastic.log(INFO, "CLB Handler {} started on port {}", handler_id_, opt_port);
}

void CLBHandler::workOpticalData()
{
    using namespace boost::asio::placeholders;
    socket_optical_.async_receive(boost::asio::buffer(&buffer_optical_[0], buffer_size_),
        boost::bind(&CLBHandler::handleOpticalData, this, error, bytes_transferred));
}

void CLBHandler::handleOpticalData(const boost::system::error_code& error, std::size_t size)
{
    if (error) {
        g_elastic.log(ERROR, "CLB Handler {} caught error {}: {}", handler_id_, error.value(), error.category().name());
        g_elastic.log(WARNING, "CLB Handler {} stopping work on optical socket due to error.", handler_id_);
        return;
    }

    if (*mode_ != true) {
        // We are not in a run so just call the work method again
        workOpticalData();
        return;      
    }

    // Check the packet has at least a CLB header in it
    if (size < sizeof(CLBCommonHeader)) {
        g_elastic.log(WARNING, "CLB Handler {} received packet without header (expected at least {}, got {})",
            handler_id_, sizeof(CLBCommonHeader), size);
        workOpticalData();
        return;
    }

    // Check the size of the packet is consistent with CLBCommonHeader + some hits
    const std::size_t remaining_bytes = size - sizeof(CLBCommonHeader);
    const std::ldiv_t div = std::div((long)remaining_bytes, sizeof(hit_t));
    if (div.rem != 0) {
        g_elastic.log(WARNING, "CLB Handler {} received packet with invalid body (expected multiple of {}, got {} which has remainder {})",
            handler_id_, sizeof(hit_t), remaining_bytes, div.rem);
        workOpticalData();
        return;
    }

    // Cast the beggining of the packet to the CLBCommonHeader
    const CLBCommonHeader& header_optical = *reinterpret_cast<const CLBCommonHeader*>(buffer_optical_);

    // Check the type of the packet is optical from the CLBCommonHeader
    const std::pair<int, std::string>& type = getType(header_optical);
    if (type.first != OPTO) {
        g_elastic.log(WARNING, "CLB Handler {} received other than optical packet (expected {}, got {} which is {})",
            handler_id_, OPTO, type.first, type.second);
        workOpticalData();
        return;
    }

    const hit_t* hit_start = reinterpret_cast<const hit_t*>(buffer_optical_ + sizeof(CLBCommonHeader));
    processPacket(header_optical, div.quot, hit_start);
    workOpticalData();
}

bool CLBHandler::processPacket(const CLBCommonHeader& header, int n_hits, const hit_t* hit) const
{
    CLBEvent new_event{};

    // Assign the variables we need from the header
    new_event.PomId = header.pomIdentifier();
    new_event.Timestamp_s = header.timeStamp().sec();
    const std::uint32_t time_stamp_ns = header.timeStamp().tics() * 16;

    data_handler_->updateLastApproxTimestamp(new_event.Timestamp_s);
    CLBEventMultiQueue* multi_queue = data_handler_->findCLBOpticalQueue(new_event.Timestamp_s + 1e-9 * time_stamp_ns, data_slot_idx_);

    if (!multi_queue) {
        // Timestamp not matched to any open batch, discard packet.
        return false;
    }

    std::lock_guard<std::mutex> l{ multi_queue->write_mutex };
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
        new_event.Channel = hit->channel;

        // Need to change the ordering of the bytes to get the correct hit time
        new_event.Timestamp_ns = time_stamp_ns;
        new_event.Timestamp_ns += ((std::uint32_t)hit->timestamp1) << 24;
        new_event.Timestamp_ns += ((std::uint32_t)hit->timestamp2) << 16;
        new_event.Timestamp_ns += ((std::uint32_t)hit->timestamp3) << 8;
        new_event.Timestamp_ns += (std::uint32_t)hit->timestamp4;

        new_event.SortKey = new_event.Timestamp_s + 1e-9 * new_event.Timestamp_ns;

        // Assign the hit TOT
        new_event.Tot = hit->ToT;

        event_queue.emplace_back(std::cref(new_event));
    }

    return true;
}
