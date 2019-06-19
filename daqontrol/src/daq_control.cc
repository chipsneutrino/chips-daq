/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"

DAQControl::DAQControl(std::string config_file)
    : config_(config_file.c_str())
    , processors_{}
    , n_threads_{}
    , mode_(false)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{

    // Print the configuration
    config_.printShortConfig();

    setupProcessors();
}

void DAQControl::setupProcessors()
{
    // TODO: Make this configurable
    processors_.push_back(new MsgProcessor("192.168.11.11", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.12", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.14", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.15", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.16", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.17", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.18", io_service_));

    // Calculate thread count
    n_threads_ = processors_.size();
}

void DAQControl::run()
{
    // Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "DAQ Control starting I/O service on {} threads", n_threads_);
    for (int i = 0; i < n_threads_; ++i) {
        thread_group_.create_thread(boost::bind(&DAQControl::ioServiceThread, this));
    }
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

void DAQControl::testMessage()
{
    // Get the Date of the hardware and software revisions from the CLBs
    MsgWriter mw;
    processors_[0]->postCommand(MsgTypes::MSG_SYS_DATEREV, mw);  
}

void DAQControl::join() 
{
    // Wait for all the threads to finish
    thread_group_.join_all();

    g_elastic.log(INFO, "DAQ Control finished.");
}