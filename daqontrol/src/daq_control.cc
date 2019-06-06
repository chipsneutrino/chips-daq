/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"
#include <util/elastic_interface.h>

DAQControl::DAQControl(std::string config_file)
    : config_(config_file.c_str())
    , controller_list_{}
    , n_threads_{}
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{

    // Print the configuration
    config_.printShortConfig();

    // TODO: Make this configurable
    controller_list_.emplace_back(0, "192.168.11.13");

    // Calculate thread count
    n_threads_ = controller_list_.size();

    setupControllers();
}

void DAQControl::setupControllers()
{
    // Setup the controllers
}

void DAQControl::run()
{
    // Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "DAQ Control starting I/O service on {} threads", n_threads_);
    for (int i = 0; i < n_threads_; ++i) {
        thread_group_.create_thread(boost::bind(&DAQControl::ioServiceThread, this));
    }

    // Wait for all the threads to finish
    thread_group_.join_all();

    g_elastic.log(INFO, "DAQ Control finished.");
}

void DAQControl::ioServiceThread()
{
    io_service_->run();
}

void DAQControl::handleStartCommand(RunType which)
{
    // If we are currently running first stop the current run
    if (mode_ == true) {
        g_elastic.log(INFO, "DAQ Control stopping current run");
        handleStopCommand();
    }

    // Set the mode to data taking
    run_type_ = which;
    mode_ = true;
}

void DAQControl::handleStopCommand()
{
    // Check we are actually running
    if (mode_ == true) {
        // Set the mode to monitoring
        mode_ = false;

    } else {
        g_elastic.log(INFO, "DAQ Control already not running");
    }
}

void DAQControl::handleExitCommand()
{
    handleStopCommand();
    run_work_.reset();
    io_service_->stop();
}