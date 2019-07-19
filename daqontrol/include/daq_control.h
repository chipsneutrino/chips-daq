/**
 * DAQControl - Controller class for the CLBs and BBBs
 * 
 * This is the main class that deals with controlling the detector componenets.
 * It holds CLBController and BBBController components and dispatches commands
 *
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "clb_controller.h"
#include <clb/msg_types.h>
#include <clb/proc_var.h>
#include <clb/clb_subsys.h>

#include "bbb_controller.h"

#include <util/command_receiver.h>
#include <util/daq_config.h>
#include <util/elastic_interface.h>

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

    virtual void handleConfigCommand() override;
    virtual void handleStartDataCommand() override;
    virtual void handleStopDataCommand() override;
    virtual void handleStartRunCommand(RunType which) override;
    virtual void handleStopRunCommand() override;
    virtual void handleExitCommand() override;

    /// Starts the io_service on a thread group and then blocks at join_all()
    void run();

    /// Posts Initialisation work to all controller io_services
    void init();  

    /// Get the current mode of the overall DAQontrol application
    Control::Status getMode()
    {
        return current_state_;
    }

private:
    /// Calls run() on the io_service in a seperate threas
    void ioServiceThread() { io_service_->run(); }

    /// Create CLB and BBB controllers depending on configuration.
    void setupFromConfig();

    /// Post work to the io_service to check states and attempt retries
    void stateUpdate();

    DAQConfig config_;                                          ///< DAQConfig read from config file
    std::vector<Controller*> controllers_;                      ///< List of controllers
    int n_threads_;                                             ///< The number of threads to use
    Control::Status current_state_;                             ///< Current state of control operation
    Control::Status target_state_;                              ///< Target state of operation

    std::shared_ptr<boost::asio::io_service> io_service_;       ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;   ///< Work for the io_service
    boost::thread_group thread_group_;                          ///< Group of threads to do the work
};
