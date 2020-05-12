/**
 * CLBHitReceiver - Hit receiver class for the CLB optical data stream
 */

#include <boost/bind.hpp>

#include <clb/data_structs.h>
#include <clb/header_structs.h>
#include <util/elastic_interface.h>

#include "clb_hit_receiver.h"

CLBHitReceiver::CLBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port, int handler_id)
    : HitReceiver { io_service, data_handler, mode, opt_port, handler_id, sizeof(CLBCommonHeader) }
{
    setUnitName("CLBHitReceiver[{}]", handler_id);
    log(INFO, "Started on port {}", handler_id, opt_port);
}

bool CLBHitReceiver::processPacket(const char* datagram, std::size_t size)
{
    // Check the size of the packet is consistent with CLBCommonHeader + some hits
    const std::size_t remaining_bytes = size - sizeof(CLBCommonHeader);
    const std::ldiv_t div = std::div((long)remaining_bytes, sizeof(hit_t));
    if (div.rem != 0) {
        log(WARNING, "Received packet with invalid body (expected multiple of {}, got {} which has remainder {})",
            handlerID(), sizeof(hit_t), remaining_bytes, div.rem);
        return false;
    }

    // Cast the beggining of the packet to the CLBCommonHeader
    const CLBCommonHeader& header = *reinterpret_cast<const CLBCommonHeader*>(datagram);

    // Check the type of the packet is optical from the CLBCommonHeader
    const std::pair<int, std::string>& type = getType(header);
    if (type.first != OPTO) {
        log(WARNING, "Received other than optical packet (expected {}, got {} which is {})",
            handlerID(), OPTO, type.first, type.second);
        return false;
    }

    int n_hits = div.quot;
    CLBEvent new_event {};

    // Assign the variables we need from the header
    new_event.PomId = header.pomIdentifier();
    new_event.Timestamp_s = header.timeStamp().sec();
    const std::uint32_t time_stamp_ns = header.timeStamp().tics() * 16;

    data_handler_->updateLastApproxTimestamp(new_event.Timestamp_s);
    CLBEventMultiQueue* multi_queue = data_handler_->findCLBOpticalQueue(new_event.Timestamp_s + 1e-9 * time_stamp_ns, dataSlotIndex());

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
    const hit_t* hit = reinterpret_cast<const hit_t*>(datagram + sizeof(CLBCommonHeader));
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
