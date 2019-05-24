/**
 * CLBHandler - Handler class for the CLB optical data stream
 * 
 * This class deals with the specifics of the CLB data stream, unpacking
 * the UDP binary stream into the actual data and storing into .root
 * files.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "clb_data_structs.h"
#include "clb_header_structs.h"
#include "data_handler.h"
#include "elastic_interface.h"

/// Buffer size in bytes for optical data
const static size_t buffer_size_opt = 10000;

/// The default port for CLB UDP optical data
const static unsigned int default_opto_port = 56015;

using boost::asio::ip::udp;

class CLBHandler {
public:
    /// Create a CLBHandler
    CLBHandler(boost::asio::io_service* io_service,
        DataHandler* data_handler,
        bool* mode);

    /// Destroy a CLBHandler
    ~CLBHandler();

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

    // CLBHandler settings/input
    bool fCollect_optical; ///< Should we collect optical data?
    bool fCollect_monitoring; ///< Should we collect monitoring data?
    DataHandler* fData_handler; ///< Pointer to the DataHandler
    bool* fMode; ///< false = Monitoring, True = Running
    std::size_t const fBuffer_size; ///< Size of the buffers

    // BOOST data collection
    boost::asio::ip::udp::socket fSocket_optical; ///< Optical data UDP socket
    char fBuffer_optical[buffer_size_opt] __attribute__((aligned(8))); ///< Optical data buffer
};
