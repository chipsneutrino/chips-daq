/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"

DAQControl::DAQControl(std::string config_file)
    : config_(config_file.c_str())
    , controllers_{}
    , n_threads_{}
    , mode_(Idle)
    , run_type_(RunType::DataNormal)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{
    // Setup from the configuration
    setupFromConfig();
}

void DAQControl::handleConfigCommand()
{
    g_elastic.log(INFO, "DAQControl: Config");

    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postConfigure(); 

    mode_ = Configured;
}

void DAQControl::handleStartDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Starting Data");
    // If data is currently being produced log
    if (mode_ == Started) {
        g_elastic.log(INFO, "DAQControl is already started");
        return;
    }

    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postStartData();  

    mode_ = Started;
}

void DAQControl::handleStopDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Stopping Data");
    // If data is not currently being produced log
    if (mode_ == Configured) {
        g_elastic.log(INFO, "DAQControl is already stopped");
        return;
    }

    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postStopData();  

    mode_ = Configured;
}

void DAQControl::handleStartRunCommand(RunType which, float flasher_v)
{
    g_elastic.log(INFO, "DAQControl: Starting Run, V={}", flasher_v);
    run_type_ = which;
    if (run_type_ == RunType::TestFlasher) {
        for(int i=0; i<controllers_.size(); i++) controllers_[i]->postFlasherOn(flasher_v);
    }
}

void DAQControl::handleStopRunCommand()
{
    g_elastic.log(INFO, "DAQControl: Stopping Run");
    if (run_type_ == RunType::TestFlasher) {
        for(int i=0; i<controllers_.size(); i++) controllers_[i]->postFlasherOff();
    }
}

void DAQControl::handleExitCommand()
{
    handleStopRunCommand();
    run_work_.reset();
    io_service_->stop();
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

void DAQControl::init()
{
    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postInit(); 
}

void DAQControl::join() 
{
    // Wait for all the threads to finish
    thread_group_.join_all();
    g_elastic.log(INFO, "DAQ Control finished.");
}

void DAQControl::ioServiceThread()
{
    io_service_->run();
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


