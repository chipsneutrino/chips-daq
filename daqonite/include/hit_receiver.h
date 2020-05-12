/**
 * HitReceiver - Receiver class for optical data streams
 * 
 * This class deals with the specifics of UDP network communication. It manages
 * a socket, receives datagrams containing optical hits, and responds to start/stop
 * run commands.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>

#include <boost/asio.hpp>

#include <util/logging.h>

#include "data_handler.h"
#include "data_run.h"

class HitReceiver : protected Logging {
public:
    explicit HitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, int opt_port, std::size_t header_size);

    virtual ~HitReceiver() = default;

    // for safety, no copy- or move-semantics
    HitReceiver(const HitReceiver& other) = delete;
    HitReceiver(HitReceiver&& other) = delete;

    HitReceiver& operator=(const HitReceiver& other) = delete;
    HitReceiver& operator=(HitReceiver&& other) = delete;

    virtual void startData();
    virtual void stopData();

    virtual void startRun(std::shared_ptr<DataRun>& run);
    virtual void stopRun();
    inline int dataSlotIndex() const { return data_slot_idx_; }

protected:
    virtual void processDatagram(const char* datagram, std::size_t datagram_size, bool do_mine) = 0;

    using timestamp_type = std::uint64_t; // TODO: we will probably want this to be a struct with fine/coarse

    void reportDataStreamGap(timestamp_type gap_end);
    void reportBadDatagram();
    void reportGoodDatagram(std::uint32_t plane_id, timestamp_type start_time, timestamp_type end_time, std::uint64_t n_hits);

    std::shared_ptr<DataHandler> data_handler_; ///< Pointer to the DataHandler

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

    int data_slot_idx_; ///< Unique data slot index assigned by DataHandler to prevent overwrites

    std::size_t header_size_;

    /**
     * IO_service optical data work function.
     * Calls the async_receive() on the IO_service for the optical data stream.
     */
    void requestDatagram();

    void receiveDatagram(boost::system::error_code const& error, std::size_t size);

    void checkAndProcessDatagram(const char* datagram, std::size_t datagram_size, bool do_mine);
};
