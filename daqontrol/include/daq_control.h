/**
 * DAQControl - Controller class for the CLBs and BBBs
 * 
 * This is the main class that deals with controlling the detector componenets.
 * It holds CLBController and BBBController components and dispatches commands
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "util/command_receiver.h"
#include "util/daq_config.h"

class DAQControl : public CommandHandler {
public:
    /**
     * Create a DAQControl
     * This creates a DAQControl, setting up the controllers etc...
     * Initial work is then added to the IO_service before run() is called to
     * start to main loop.
     */
    explicit DAQControl(std::string config_file);

    /// Destroy a DAQControl
    virtual ~DAQControl() = default;

    // for safety, no copy- or move-semantics
    DAQControl(const DAQControl& other) = delete;
    DAQControl(DAQControl&& other) = delete;

    DAQControl& operator=(const DAQControl& other) = delete;
    DAQControl& operator=(DAQControl&& other) = delete;

    virtual void handleStartCommand(RunType which) override;
    virtual void handleStopCommand() override;
    virtual void handleExitCommand() override;

    void run();

private:
    /**
     * Bound to thread creation
     * Allows us to modify how the thread operates and what it does
     */
    void ioServiceThread();

    /// Create CLB and BBB controllers depending on configuration.
    void setupControllers();

    // Settings
    DAQConfig config_;  ///< DAQConfig read from config file
    std::list<std::pair<int, std::string>> controller_list_; ///< List of controller type and ip address
    int n_threads_; ///< The number of threads to use

    // Running mode
    bool mode_; ///< false = Not Running, True = Running
    RunType run_type_;

    // IO_service stuff
    std::shared_ptr<boost::asio::io_service> io_service_; ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;
    boost::thread_group thread_group_; ///< Group of threads to do the work
};
