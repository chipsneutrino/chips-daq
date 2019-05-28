/**
 * CLBHandler - Handler class for the CLB optical data stream
 * 
 * This class deals with the specifics of the CLB data stream, unpacking
 * the UDP binary stream into the actual data and storing into .root
 * files.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 *
 * Co-author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <boost/asio.hpp>
#include <memory>

#include "data_handler.h"

/// Buffer size in bytes for optical data
const static size_t buffer_size_opt = 10000;

/// The default port for CLB UDP optical data
const static unsigned int default_opto_port = 56015;

using boost::asio::ip::udp;

class CLBCommonHeader;
struct hit_t;

class CLBHandler {
public:
    explicit CLBHandler(std::shared_ptr<boost::asio::io_service> io_service,
        std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port, int handler_id);

    virtual ~CLBHandler() = default;

    // for safety, no copy- or move-semantics
    CLBHandler(const CLBHandler& other) = delete;
    CLBHandler(CLBHandler&& other) = delete;

    CLBHandler& operator=(const CLBHandler& other) = delete;
    CLBHandler& operator=(CLBHandler&& other) = delete;

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

    bool processPacket(const CLBCommonHeader& header, int n_hits, const hit_t* hit) const;

    // CLBHandler settings/input
    std::shared_ptr<DataHandler> data_handler_; ///< Pointer to the DataHandler
    const bool* const mode_; ///< false = Monitoring, True = Running
    const std::size_t buffer_size_; ///< Size of the buffers

    // BOOST data collection
    udp::socket socket_optical_; ///< Optical data UDP socket
    char buffer_optical_[buffer_size_opt] __attribute__((aligned(8))); ///< Optical data buffer

    int data_slot_idx_;
    int handler_id_;
};
