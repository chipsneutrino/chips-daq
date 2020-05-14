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
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <list>
#include <memory>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <util/command_receiver.h>
#include <util/logging.h>

#include "basic_hit_receiver.h"
#include "data_handler.h"
#include "data_run.h"
#include "scheduling_pool.h"

class DAQHandler : public CommandHandler, protected Logging {
public:
    /**
     * Create a DAQHandler
     * This creates a DAQHandler, setting up the sockets etc...
     * Initial work is then added to the IO_service before run() is called to
     * start to main loop.
     */
    explicit DAQHandler(const std::string& data_path);

    /// Destroy a DAQHandler
    virtual ~DAQHandler() = default;

    // for safety, no copy- or move-semantics
    DAQHandler(const DAQHandler& other) = delete;
    DAQHandler(DAQHandler&& other) = delete;

    DAQHandler& operator=(const DAQHandler& other) = delete;
    DAQHandler& operator=(DAQHandler&& other) = delete;

    virtual void handleConfigCommand(std::string config_file) override;
    virtual void handleStartDataCommand() override;
    virtual void handleStopDataCommand() override;
    virtual void handleStartRunCommand(RunType which) override;
    virtual void handleStopRunCommand() override;
    virtual void handleExitCommand() override;

    inline const std::shared_ptr<DataRun>& getRun() const { return run_; }

    void run();

private:
    /// Create CLB and BBB hit receivers depending on configuration.
    void createHitReceivers();

    // Settings
    std::list<int> clb_ports_; ///< Port numbers where CLB hit receivers are listening.
    std::list<int> bbb_ports_; ///< Port numbers where BBB hit receivers are listening.
    std::string output_directory_path_;

    // Running mode
    std::shared_ptr<DataRun> run_;

    // IO_service stuff
    using io_service = boost::asio::io_service;
    std::shared_ptr<io_service> io_service_; ///< BOOST io_service. The heart of everything
    std::unique_ptr<io_service::work> run_work_;
    boost::thread_group thread_group_; ///< Group of threads to do the work

    // Other components
    std::shared_ptr<DataHandler> data_handler_; ///< DataHandler object
    std::list<std::unique_ptr<BasicHitReceiver>> hit_receivers_; ///< Pointers to hit receivers
    std::shared_ptr<SchedulingPool> scheduling_;
};
