/**
 * BasicHitReceiver - Common hit receiver implementation for optical data streams
 * 
 * This class deals with the specifics of UDP network communication. It manages
 * a socket, receives datagrams containing optical hits, and responds to start/stop
 * run commands. This is an abstract object, from which both specialised implementations
 * for CLBs and BBBs inherit.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>

#include <util/logging.h>

#include "data_run.h"
#include "spill_schedule.h"

class BasicHitReceiver : protected Logging {
public:
    explicit BasicHitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<SpillSchedule> spill_schedule, int opt_port,
        std::size_t expected_header_size, std::size_t expected_hit_size);

    virtual ~BasicHitReceiver() = default;

    // for safety, no copy- or move-semantics
    BasicHitReceiver(const BasicHitReceiver& other) = delete;
    BasicHitReceiver(BasicHitReceiver&& other) = delete;

    BasicHitReceiver& operator=(const BasicHitReceiver& other) = delete;
    BasicHitReceiver& operator=(BasicHitReceiver&& other) = delete;

    virtual void startData();
    virtual void stopData();

    virtual void startRun(std::shared_ptr<DataRun>& run);
    virtual void stopRun();

    inline std::size_t dataSlotIndex() const { return data_slot_idx_; }

protected:
    virtual void processDatagram(const char* datagram, std::size_t datagram_size, std::size_t n_hits, bool do_mine) = 0;

    bool checkAndIncrementSequenceNumber(std::uint32_t plane_number, std::uint32_t seq_number, const tai_timestamp& datagram_start_time);

    void reportBadDatagram();
    void reportGoodDatagram(std::uint32_t plane_id, const tai_timestamp& start_time, const tai_timestamp& end_time, std::uint64_t n_hits);

    std::shared_ptr<SpillSchedule> spill_schedule_; ///< Pointer to the SpillSchedule

private:
    enum class DataMode {
        Idle,
        Receiving,
        Mining
    };

    DataMode mode_;
    std::shared_ptr<DataRun> run_;

    // BOOST data collection
    boost::asio::ip::udp::socket socket_optical_; ///< Optical data UDP socket

    using DatagramBuffer = std::vector<char>;
    DatagramBuffer datagram_buffer_; ///< Optical data buffer

    std::size_t data_slot_idx_; ///< Unique data slot index assigned by SpillSchedule to prevent overwrites

    std::size_t expected_header_size_;
    std::size_t expected_hit_size_;

    using SequenceNumberMap = std::unordered_map<std::uint32_t, std::uint32_t>;
    SequenceNumberMap plane_to_next_sequence_number_;

    /**
     * IO_service optical data work function.
     * Calls the async_receive() on the IO_service for the optical data stream.
     */
    void requestDatagram();

    void receiveDatagram(boost::system::error_code const& error, std::size_t size);

    void checkAndProcessDatagram(const char* datagram, std::size_t datagram_size, bool do_mine);

    void reportDataStreamGap(const tai_timestamp& gap_end);
};
