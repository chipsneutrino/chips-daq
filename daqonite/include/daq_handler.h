/**
 * DAQHandler - Handler class for combining data streams
 * 
 * This is the main class that deals with the DAQ across all stream
 * It holds a CLB_handler and BBB_handler object which deal with the 
 * individual streams data collection. It controls the IO_service 
 * which provides the backbone to the entire DAQonite program.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQ_HANDLER_H_
#define DAQ_HANDLER_H_

#include <boost/asio.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "bbb_handler.h"
#include "clb_handler.h"
#include "data_handler.h"
#include "elastic_interface.h"

/// Buffer size in bytes for local data
const static size_t buffer_size_local = 10000;

class DAQHandler {
public:
    /**
		 * Create a DAQHandler
		 * This creates a DAQHandler, setting up the sockets etc...
		 * Initial work is then added to the IO_service before run() is called to
		 * start to main loop.
		 */
    DAQHandler(bool collect_clb_data, bool collect_bbb_data,
        int numThreads);

    /// Destroy a DAQHandler
    ~DAQHandler();

private:
    /**
		 * Binded to thread creation
		 * Allows us to modify how the thread operates and what it does
		 */
    void ioServiceThread();

    /**
		 * Handles UNIX signals
		 * Handles the interupts from UNIX signals. Currently only ctrl-c is defined
		 * which calls exit() on the application.
		 * 
		 * @param error Signals error code
		 * @param signum Signal number
		 */
    void handleSignals(boost::system::error_code const& error, int signum);

    /// Calls the async_wait() on the signal_set
    void workSignals();

    /**
		 * Handles the local control socket
		 * Handles any commands sent from daq_command over the local control UDP socket.
		 * It reads the input buffer and determines the action to take.
		 * 
		 * @param error Error code from the async_receive()
		 * @param size Signal Number of bytes received
		 */
    void handleLocalSocket(boost::system::error_code const& error, std::size_t size);

    /// Calls the async_receive() on the local UDP control socket
    void workLocalSocket();

    // Settings
    bool fCollect_clb_data; ///< Should we collect CLB optical data?
    bool fCollect_bbb_data; ///< Should we collect CLB monitoring data?
    int fNum_threads; ///< The number of threads to use

    // Running mode
    bool fMode; ///< false = Not Running, True = Running

    // IO_service stuff
    boost::asio::io_service fIO_service; ///< BOOST io_service. The heart of everything
    boost::thread_group fThread_group; ///< Group of threads to do the work
    boost::asio::signal_set fSignal_set; ///< BOOST signal_set
    udp::socket fLocal_socket; ///< Local UDP control socket
    char fBuffer_local[buffer_size_local] __attribute__((aligned(8))); ///< Local socket buffer
    DataHandler fData_handler; ///< DataHandler object
    CLBHandler* fCLB_handler; ///< Pointer to CLBHandler
    BBBHandler* fBBB_handler; ///< Pointer to BBBHandler
};

#endif
