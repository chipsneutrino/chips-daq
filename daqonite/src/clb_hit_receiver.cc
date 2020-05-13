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

    hit new_hit {};

    // Assign the variables we need from the header
    new_hit.plane_number = header.pomIdentifier();

    // TODO: verify that the time from the header indeed is TAI
    const tai_timestamp datagram_start_time { header.timeStamp().sec(), header.timeStamp().tics() * 16 };

    if (!checkAndIncrementSequenceNumber(header.udpSequenceNumber(), datagram_start_time)) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    // FIXME: timestamps
    reportGoodDatagram(new_hit.plane_number, datagram_start_time, tai_timestamp {}, n_hits);

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
    const hit_t* hit = reinterpret_cast<const hit_t*>(datagram + sizeof(CLBCommonHeader));
    for (int i = 0; i < n_hits; ++i, ++hit) { // TODO: find a way to unroll and vectorize this loop
        // Assign the hit channel
        new_hit.channel_number = hit->channel;

        // Need to change the ordering of the bytes to get the correct hit time
        new_hit.timestamp = datagram_start_time;
        new_hit.timestamp.nanosecs += ((std::uint32_t)hit->timestamp1) << 24;
        new_hit.timestamp.nanosecs += ((std::uint32_t)hit->timestamp2) << 16;
        new_hit.timestamp.nanosecs += ((std::uint32_t)hit->timestamp3) << 8;
        new_hit.timestamp.nanosecs += ((std::uint32_t)hit->timestamp4);
        new_hit.timestamp.normalise();

        new_hit.sort_key = new_hit.timestamp.combined_secs();

        // Assign the hit TOT and ADC
        new_hit.tot = hit->ToT; // FIXME: this TOT is sometimes negative, figure out why (sign bit?)
        new_hit.adc0 = hit::NO_ADC0;

        event_queue.emplace_back(std::cref(new_hit));
    }
}
