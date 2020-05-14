/**
 * CLBHitReceiver - Hit receiver class for the CLB optical data stream
 */

#include <boost/bind.hpp>

#include <clb/data_structs.h>
#include <clb/header_structs.h>
#include <util/elastic_interface.h>

#include "clb_hit_receiver.h"

CLBHitReceiver::CLBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, int opt_port)
    : HitReceiver { io_service, data_handler, opt_port, sizeof(CLBCommonHeader), sizeof(hit_t) }
{
    setUnitName("CLBHitReceiver[{}]", opt_port);
}

void CLBHitReceiver::processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine)
{
    // Cast the beggining of the packet to the CLBCommonHeader
    const auto& header { *reinterpret_cast<const CLBCommonHeader*>(datagram) };

    // Check the type of the packet is optical from the CLBCommonHeader
    const std::pair<int, std::string>& type = getType(header);
    if (type.first != OPTO) {
        log(WARNING, "Received non-optical packet (expected type {}, got {} which is {})",
            OPTO, type.first, type.second);
        reportBadDatagram();
        return;
    }

    // TODO: verify that the time from the header indeed is TAI
    const tai_timestamp datagram_start_time { header.timeStamp().sec(), header.timeStamp().tics() * 16 };

    if (!checkAndIncrementSequenceNumber(header.udpSequenceNumber(), datagram_start_time)) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    const std::uint32_t plane_number { header.pomIdentifier() };

    // FIXME: timestamps
    reportGoodDatagram(plane_number, datagram_start_time, tai_timestamp {}, n_hits);

    if (do_mine) {
        const hit_t* hits_begin { reinterpret_cast<const hit_t*>(datagram + sizeof(CLBCommonHeader)) };
        mineHits(hits_begin, n_hits, datagram_start_time, plane_number);
    }
}

void CLBHitReceiver::mineHits(const hit_t* hits_begin, std::size_t n_hits, const tai_timestamp& base_time, std::uint32_t plane_number)
{
    // TODO: perhaps use the lowest hit time in datagram here instead?
    HitMultiQueue* multi_queue { data_handler_->findCLBOpticalQueue(base_time, dataSlotIndex()) };

    if (!multi_queue) {
        // Timestamp not matched to any open batch, discard datagram.
        // TODO: devise a reporting mechanism for this
        return;
    }

    // From this point on, the queue is being written into, and the batch cannot be closed until the end of scope.
    std::lock_guard<std::mutex> l { multi_queue->write_mutex };

    if (multi_queue->closed_for_writing) {
        // Have a batch, which has been closed but not yet removed from the schedule. Discard datagram.
        // TODO: devise a reporting mechanism for this
        return;
    }

    // Find/create a queue for this plane
    HitQueue& event_queue { multi_queue->get_queue_for_writing(plane_number) };

    // Enqueue all the hits!
    event_queue.reserve(event_queue.size() + n_hits);
    const hit_t* hits_end { hits_begin + n_hits };
    for (auto hits_it = hits_begin; hits_it != hits_end; ++hits_it) { // NOTE: vectorize this loop?
        // To avoid unnecessary copying, we first add the hit and then set its values later.
        // We can do this because we are holding on to the mutex.
        event_queue.emplace_back();
        hit& dest_hit { event_queue.back() };

        const hit_t& src_hit { *hits_it };

        // Assign basic hit fields
        dest_hit.plane_number = plane_number;
        dest_hit.channel_number = src_hit.channel;
        dest_hit.tot = src_hit.ToT;
        dest_hit.adc0 = hit::NO_ADC0; // CLBs do not report ADC

        // Assign hit timestamp
        // Need to change the ordering of the bytes to get the correct hit time
        dest_hit.timestamp = base_time;
        dest_hit.timestamp.nanosecs += static_cast<std::uint32_t>(src_hit.timestamp1) << 24;
        dest_hit.timestamp.nanosecs += static_cast<std::uint32_t>(src_hit.timestamp2) << 16;
        dest_hit.timestamp.nanosecs += static_cast<std::uint32_t>(src_hit.timestamp3) << 8;
        dest_hit.timestamp.nanosecs += static_cast<std::uint32_t>(src_hit.timestamp4);
        dest_hit.timestamp.normalise();

        dest_hit.sort_key = dest_hit.timestamp.combined_secs();
    }
}
