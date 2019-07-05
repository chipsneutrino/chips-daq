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

#include "controller.h"
#include <clb/msg_types.h>
#include <clb/proc_var.h>
#include <clb/clb_subsys.h>

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

    virtual void handleStartCommand(RunType which) override;
    virtual void handleStopCommand() override;
    virtual void handleExitCommand() override;

    void run();

    void test();
    void initClb();  
    void configureClb(); 
    void startClb(); 
    void stopClb();            
    void quitClb();
    void resetClb();
    void pauseClb();
    void continueClb();

    
    void join();

private:
    /**
     * Bound to thread creation
     * Allows us to modify how the thread operates and what it does
     */
    void ioServiceThread();

    /// Create CLB and BBB processors depending on configuration.
    void setupFromConfig();

    // Settings
    DAQConfig config_;                      ///< DAQConfig read from config file
    std::vector<Controller*> controllers_;  ///< List of controllers
    int n_threads_;                         ///< The number of threads to use

    // Running mode
    bool mode_;                             ///< false = Not Running, True = Running
    RunType run_type_;                      ///< Current run type

    // IO_service stuff
    std::shared_ptr<boost::asio::io_service> io_service_;       ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;   ///< Work for the io_service
    boost::thread_group thread_group_;                          ///< Group of threads to do the work
};
