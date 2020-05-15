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
    : BasicHitReceiver { io_service, data_handler, opt_port, sizeof(CLBCommonHeader), sizeof(hit_t) }
{
    setUnitName("CLBHitReceiver[{}]", opt_port);
}

void CLBHitReceiver::processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine)
{
    /*
    TODO:
      - verify that timestamps are indeed TAI, can do so by simply comparing them with the current TAI and UTC
      - refactor bitshifting in hit timestamps 1-4
      - support "long hits"
           Recording long hits: starting from firmware rev20160510, long hits, i.e. those hits whose time-over-threshold
           duration exceeding 255 ns, are coded by means of (I) a first hit recorded as: "TDC channel" = x, 
           "Time Stamp" = T0 and "Pulse Width" = 255, followed by (II) a second hit recorded as: same "TDC channel" = x, 
           "Time Stamp" = (T0 + 255) and "Pulse Width" = (original duration of the hit - 255). It should be noted that 
           very long hits could be recorded with more than 2 such "partial sub-hits". Warning: long hits are counted as 
           single hits in the hit rate monitor as well as the hit count which is used for HRV, independently of how many 
           "partial hits" are written in the data.
      - resolve hits across time slices:
           starting from CLB FW/SW [CLB_firmware_versions#Stable_releases rev20161014] hits that cross the boarder of 
           two time slices are recorded as follows: the hit is recorded in the timeslice it starts in with the correct 
           time but pulse width 0. The hit is not recorded in the following timeslice, even if it is a long hit.
      ... for more info see: https://wiki.km3net.de/index.php/DAQ/Readout_Technical_Design_Report_(TDR)#Data_acquisition_concept
    */

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

    // FIXME: this sequence number drops at the start of every window and will report fake gaps
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
    PMTMultiPlaneHitQueue* multi_queue { data_handler_->findHitQueue(base_time, dataSlotIndex()) };

    if (!multi_queue) {
        // Timestamp not matched to any open batch, discard datagram.
        // TODO: devise a reporting mechanism for this
        return;
    }

    // From this point on, the queue is being written into, and the batch cannot be closed until the end of scope.
    std::lock_guard<std::mutex> l { multi_queue->mutex };

    if (multi_queue->closed_for_writing) {
        // Have a batch, which has been closed but not yet removed from the schedule. Discard datagram.
        // TODO: devise a reporting mechanism for this
        return;
    }

    // Find/create a queue for this plane
    PMTHitQueue& event_queue { multi_queue->get_queue_for_writing(plane_number) };

    // Enqueue all the hits!
    event_queue.reserve(event_queue.size() + n_hits);
    const hit_t* hits_end { hits_begin + n_hits };
    for (auto hits_it = hits_begin; hits_it != hits_end; ++hits_it) { // NOTE: vectorize this loop?
        // To avoid unnecessary copying, we first add the hit and then set its values later.
        // We can do this because we are holding on to the mutex.
        event_queue.emplace_back();
        PMTHit& dest_hit { event_queue.back() };

        const hit_t& src_hit { *hits_it };

        // Assign basic hit fields
        dest_hit.plane_number = plane_number;
        dest_hit.channel_number = src_hit.channel;
        dest_hit.tot = src_hit.ToT;
        dest_hit.adc0 = PMTHit::NO_ADC0; // CLBs do not report ADC

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
