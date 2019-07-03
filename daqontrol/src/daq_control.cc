/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"

DAQControl::DAQControl(std::string config_file)
    : config_(config_file.c_str())
    , controllers_{}
    , n_threads_{}
    , mode_(false)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{
    // Setup from the configuration
    setupFromConfig();
}

void DAQControl::setupFromConfig()
{
    // Print the configuration
    config_.printConfig();

    // For now just add the single test CLB at 192.168.11.36
    ControllerConfig config;
    config.enabled_ = true;
    config.ip_ = 3232238372;
    controllers_.push_back(new Controller(config));

    /*
    for (int clb=0; clb<config_.fNum_clbs; clb++)
    {
        controllers_.push_back(new Controller(config_.configs_[clb]));
    }
    */

    // Calculate thread count
    n_threads_ = 1;
}

void DAQControl::run()
{
    // Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "DAQ Control starting I/O service on {} threads", n_threads_);

    for (int i = 0; i < n_threads_; ++i) {
        thread_group_.create_thread(boost::bind(&DAQControl::ioServiceThread, this));
    }

    thread_group_.join_all();
}

void DAQControl::ioServiceThread()
{
    io_service_->run();
}

void DAQControl::handleStartCommand(RunType which)
{
    g_elastic.log(INFO, "DAQControl: Start");
    // If we are currently running first stop the current run
    if (mode_ == true) {

        g_elastic.log(INFO, "DAQ Control stopping current run");

        handleStopCommand();
    }
    // Set the mode to data taking
    run_type_ = which;
    mode_ = true;

    // Send start command to CLBs
    startRun();
}

void DAQControl::handleStopCommand()
{
    g_elastic.log(INFO, "DAQControl: Stop");
    // Check we are actually running
    if (mode_ == true) {
        // Set the mode to monitoring
        mode_ = false;
    } else {
        g_elastic.log(INFO, "DAQ Control already not running");
    }

    // Send stop command to CLBs
    stopRun();
}

void DAQControl::handleExitCommand()
{
    handleStopCommand();
    run_work_.reset();
    io_service_->stop();
}

void DAQControl::test()
{
    for(int i=0; i<controllers_.size(); i++)
    {
        controllers_[i]->postTest();  
    }
}

void DAQControl::init()
{
    for(int i=0; i<controllers_.size(); i++)
    {
        controllers_[i]->postInit();  
    }
}

void DAQControl::configure()
{
    for(int i=0; i<controllers_.size(); i++)
    {
        controllers_[i]->postConfigure();  
    }
}

void DAQControl::startRun()
{
    for(int i=0; i<controllers_.size(); i++)
    {
        controllers_[i]->postStartRun();  
    }
}

void DAQControl::stopRun()
{
    for(int i=0; i<controllers_.size(); i++)
    {
        controllers_[i]->postStopRun();  
    }
} 

void DAQControl::join() 
{
    // Wait for all the threads to finish
    thread_group_.join_all();
    g_elastic.log(INFO, "DAQ Control finished.");
}