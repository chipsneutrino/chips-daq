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

#include "data_handler.h"

/// Buffer size in bytes for optical data
const static size_t BUFFER_SIZE_HITS = 10000;

class HitReceiver {
public:
    explicit HitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port,
        int handler_id, std::size_t header_size);

    virtual ~HitReceiver() = default;

    // for safety, no copy- or move-semantics
    HitReceiver(const HitReceiver& other) = delete;
    HitReceiver(HitReceiver&& other) = delete;

    HitReceiver& operator=(const HitReceiver& other) = delete;
    HitReceiver& operator=(HitReceiver&& other) = delete;

    /**
     * IO_service optical data work function.
     * Calls the async_receive() on the IO_service for the optical data stream.
     */
    void workOpticalData();

    inline int dataSlotIndex() const { return data_slot_idx_; }
    inline int handlerID() const { return handler_id_; }

protected:
    virtual bool processPacket(const char* datagram, std::size_t size) = 0;

    std::shared_ptr<DataHandler> data_handler_; ///< Pointer to the DataHandler

private:
    // CLBHandler settings/input

    const bool* const mode_; ///< false = Monitoring, True = Running
    const std::size_t buffer_size_; ///< Size of the buffers

    // BOOST data collection
    boost::asio::ip::udp::socket socket_optical_; ///< Optical data UDP socket
    char buffer_optical_[BUFFER_SIZE_HITS] __attribute__((aligned(8))); ///< Optical data buffer

    int data_slot_idx_; ///< Unique data slot index assigned by DataHandler to prevent overwrites
    int handler_id_; ///< Logging ID

    std::size_t header_size_;

    void handleOpticalData(boost::system::error_code const& error, std::size_t size);
};
