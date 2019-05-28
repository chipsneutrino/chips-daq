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
 *
 * Co-author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <list>
#include <memory>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "bbb_handler.h"
#include "clb_handler.h"
#include "data_handler.h"

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
    explicit DAQHandler(bool collect_clb_data, bool collect_bbb_data);

    /// Destroy a DAQHandler
    virtual ~DAQHandler() = default;

    // for safety, no copy- or move-semantics
    DAQHandler(const DAQHandler& other) = delete;
    DAQHandler(DAQHandler&& other) = delete;

    DAQHandler& operator=(const DAQHandler& other) = delete;
    DAQHandler& operator=(DAQHandler&& other) = delete;

    void run();

private:
    /**
     * Bound to thread creation
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

    void setupHandlers();

    void cmdStart(int run_type);
    void cmdStop();
    void cmdExit();

    // Settings
    bool collect_clb_data_; ///< Should we collect CLB data?
    bool collect_bbb_data_; ///< Should we collect BBB data?
    std::list<int> clb_ports_;
    int n_threads_; ///< The number of threads to use

    // Running mode
    bool mode_; ///< false = Not Running, True = Running

    // IO_service stuff
    std::shared_ptr<boost::asio::io_service> io_service_; ///< BOOST io_service. The heart of everything
    boost::thread_group thread_group_; ///< Group of threads to do the work
    boost::asio::signal_set signal_set_; ///< BOOST signal_set
    udp::socket local_socket_; ///< Local UDP control socket
    char buffer_local_[buffer_size_local] __attribute__((aligned(8))); ///< Local socket buffer

    // Other components
    std::shared_ptr<DataHandler> data_handler_; ///< DataHandler object
    std::list<std::unique_ptr<CLBHandler>> clb_handlers_; ///< Pointers to CLBHandlers
    std::unique_ptr<BBBHandler> bbb_handler_; ///< Pointer to BBBHandler
};
