/**
 * BBBHandler - Handler class for the BBB data stream
 * 
 * This class deals with the specifics of the BBB data stream, using the
 * fh_library API, to communicate with the Madison beaglebones.
 *
 * Author: Josh Tingey, Petr Mánek
 * Contact: j.tingey.16@ucl.ac.uk, petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>

#include <boost/asio.hpp>

#include <bbb/packets.h>

#include "data_handler.h"

/// Buffer size in bytes for optical data
const static size_t buffer_size_bbb_opt = 10000;

using boost::asio::ip::udp;

class BBBHandler {
public:
    BBBHandler(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port, int handler_id);

    ~BBBHandler() = default;

    // for safety, no copy- or move-semantics
    BBBHandler(const BBBHandler& other) = delete;
    BBBHandler(BBBHandler&& other) = delete;

    BBBHandler& operator=(const BBBHandler& other) = delete;
    BBBHandler& operator=(BBBHandler&& other) = delete;

    /**
     * IO_service optical data work function.
     * Calls the async_receive() on the IO_service for the optical data stream.
     */
    void workOpticalData();

private:
    /**
     * Callback completion function for optical data async_receive().
     * Handles the received optical data after the async_receive() has completed. 
     * It fills the ROOT TTrees with the decoded data.
     * 
     * @param error Error code from async_receive()
     * @param size Number of bytes received
     */
    void handleOpticalData(boost::system::error_code const& error, std::size_t size);

    /// Process CLB optical data packet.
    bool processPacket(const opt_packet_header_t& header, std::uint32_t n_hits, const opt_packet_hit_t* hit) const;

    // CLBHandler settings/input
    std::shared_ptr<DataHandler> data_handler_; ///< Pointer to the DataHandler
    const bool* const mode_; ///< false = Monitoring, True = Running
    const std::size_t buffer_size_; ///< Size of the buffers

    // BOOST data collection
    udp::socket socket_optical_; ///< Optical data UDP socket
    char buffer_optical_[buffer_size_bbb_opt] __attribute__((aligned(8))); ///< Optical data buffer

    int data_slot_idx_; ///< Unique data slot index assigned by DataHandler to prevent overwrites
    int handler_id_; ///< Logging ID

    std::uint32_t next_sequence_number_;
};
