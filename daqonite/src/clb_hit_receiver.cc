/**
 * CLBHitReceiver - Hit receiver class for the CLB optical data stream
 */

#include <boost/bind.hpp>

#include <clb/data_structs.h>
#include <clb/header_structs.h>

#include "clb_hit_receiver.h"

CLBHitReceiver::CLBHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<SpillSchedule> spill_schedule, int opt_port)
    : BasicHitReceiver { io_service, spill_schedule, opt_port, sizeof(CLBCommonHeader), sizeof(hit_t), true }
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
    const tai_timestamp base_time { header.timeStamp().sec(), header.timeStamp().tics() * 16 };
    const std::uint32_t plane_number { header.pomIdentifier() };

    if (!checkAndIncrementSequenceNumber(plane_number, header.udpSequenceNumber(), base_time)) {
        // Late datagram, discard it.
        reportBadDatagram();
        return;
    }

    // Peek at the timestamps of the first and the last hit in the datagram.
    tai_timestamp datagram_first_timestamp { base_time };
    tai_timestamp datagram_last_timestamp { base_time };
    const hit_t* hits_begin { reinterpret_cast<const hit_t*>(datagram + sizeof(CLBCommonHeader)) };

    if (n_hits > 0) {
        datagram_first_timestamp = calculateHitTime(hits_begin[0], base_time);
        datagram_last_timestamp = calculateHitTime(hits_begin[n_hits - 1], base_time);
    }

    reportGoodDatagram(plane_number, datagram_first_timestamp, datagram_last_timestamp, n_hits);

    if (do_mine) {
        mineHits(hits_begin, n_hits, base_time, plane_number);
    }
}

void CLBHitReceiver::mineHits(const hit_t* hits_begin, std::size_t n_hits, const tai_timestamp& base_time, std::uint32_t plane_number)
{
    SpillDataSlot* found_slot {};
    if (!(found_slot = findAndLockDataSlot(base_time))) {
        // Have no slot to store the hits, discard datagram.
        return;
    }

    // The slot will be automatically unlocked at the end of this scope.
    SpillDataSlot& slot { *found_slot };
    std::lock_guard<std::mutex> l { slot.mutex, std::adopt_lock };

    // Find/create a queue for this plane.
    PMTHitQueue& event_queue { slot.opt_hit_queue.get_queue_for_writing(plane_number) };

    // Enqueue all the hits!
    event_queue.reserve(event_queue.size() + n_hits);
    const hit_t* hits_end { hits_begin + n_hits };
    for (auto hits_it = hits_begin; hits_it != hits_end; ++hits_it) { // NOTE: vectorize this loop?
        // To avoid unnecessary copying, we first add the hit and then set its values later.
        // We can do this because we are holding on to the mutex.
        event_queue.emplace_back();
        PMTHit& dest_hit { event_queue.back() };

        const hit_t& src_hit { *hits_it };

        // Assign hit fields
        dest_hit.plane_number = plane_number;
        dest_hit.channel_number = src_hit.channel;
        dest_hit.tot = src_hit.ToT;
        dest_hit.adc0 = PMTHit::NO_ADC0; // CLBs do not report ADC
        dest_hit.timestamp = calculateHitTime(src_hit, base_time);

        dest_hit.sort_key = dest_hit.timestamp.combined_secs();
    }
}

tai_timestamp CLBHitReceiver::calculateHitTime(const hit_t& hit, const tai_timestamp& base_time)
{
    // Hit time is expressed as [ns] offset w.r.t. a base timestamp.
    // Need to change the ordering of the bytes to get the correct hit time
    // FIXME: There surely must be a more elegant (and performant) way to do this
    tai_timestamp timestamp { base_time };
    timestamp.nanosecs += static_cast<std::uint32_t>(hit.timestamp1) << 24;
    timestamp.nanosecs += static_cast<std::uint32_t>(hit.timestamp2) << 16;
    timestamp.nanosecs += static_cast<std::uint32_t>(hit.timestamp3) << 8;
    timestamp.nanosecs += static_cast<std::uint32_t>(hit.timestamp4);
    timestamp.normalise();

    return timestamp;
}
