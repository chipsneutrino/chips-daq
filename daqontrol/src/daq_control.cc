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
    ControllerConfig config_clb;
    config_clb.enabled_ = true;
    config_clb.ip_ = 3232238372;
    controllers_.push_back(new CLBController(config_clb));

    ControllerConfig config_bbb;
    config_bbb.enabled_ = true;
    config_bbb.type_ = BBB;
    controllers_.push_back(new BBBController(config_bbb));

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

void DAQControl::handleConfigCommand()
{
    g_elastic.log(INFO, "DAQControl: Config");
    configure();
}

void DAQControl::handleStartDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Starting Data");
    // If data is currently being produced log
    if (mode_ == true) {
        g_elastic.log(INFO, "DAQControl is already started");
        return;
    }
    startData();
    mode_ = true;
}

void DAQControl::handleStopDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Stopping Data");
    // If data is not currently being produced log
    if (mode_ == false) {
        g_elastic.log(INFO, "DAQControl is already stopped");
        return;
    }
    stopData();
    mode_ = false;
}

void DAQControl::handleStartRunCommand(RunType which)
{
    g_elastic.log(INFO, "DAQControl: Starting Run");
}

void DAQControl::handleStopRunCommand()
{
    g_elastic.log(INFO, "DAQControl: Stopping Run");
}

void DAQControl::handleExitCommand()
{
    handleStopRunCommand();
    run_work_.reset();
    io_service_->stop();
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

void DAQControl::startData()
{
    for(int i=0; i<controllers_.size(); i++)
    {
        controllers_[i]->postStartData();  
    }
}

void DAQControl::stopData()
{
    for(int i=0; i<controllers_.size(); i++)
    {
        controllers_[i]->postStopData();  
    }
} 

void DAQControl::join() 
{
    // Wait for all the threads to finish
    thread_group_.join_all();
    g_elastic.log(INFO, "DAQ Control finished.");
}
