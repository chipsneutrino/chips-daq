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
#include "util/command_receiver.h"
#include "data_handler.h"

class DAQHandler : public CommandHandler {
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

    virtual void handleStartCommand(RunType which) override;
    virtual void handleStopCommand() override;
    virtual void handleExitCommand() override;

    void run();

    bool getMode() const { return mode_; }
    RunType getRunType() { return run_type_; }

private:
    /**
     * Bound to thread creation
     * Allows us to modify how the thread operates and what it does
     */
    void ioServiceThread();

    /// Create CLB and BBB handlers depending on configuration.
    void setupHandlers();

    // Settings
    bool collect_clb_data_; ///< Should we collect CLB data?
    bool collect_bbb_data_; ///< Should we collect BBB data?
    std::list<int> clb_ports_; ///< Port numbers where CLB handlers are listening.
    int n_threads_; ///< The number of threads to use

    // Running mode
    bool mode_; ///< false = Not Running, True = Running
    RunType run_type_;

    // IO_service stuff
    std::shared_ptr<boost::asio::io_service> io_service_; ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;
    boost::thread_group thread_group_; ///< Group of threads to do the work

    // Other components
    std::shared_ptr<DataHandler> data_handler_; ///< DataHandler object
    std::list<std::unique_ptr<CLBHandler>> clb_handlers_; ///< Pointers to CLBHandlers
    std::unique_ptr<BBBHandler> bbb_handler_; ///< Pointer to BBBHandler
};
