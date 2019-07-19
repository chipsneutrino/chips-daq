/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"

DAQControl::DAQControl(std::string config_file)
    : config_(config_file.c_str())
    , controllers_{}
    , n_threads_(1)
    , state_(Control::Initialising)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{
    setupFromConfig(); // Setup the topology from the configuration
}

void DAQControl::handleConfigCommand()
{
    g_elastic.log(INFO, "DAQControl: Configure");
    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postConfigure(); 
}

void DAQControl::handleStartDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Starting Data");
    if (state_ == Control::Started) // If data is currently being produced log
    {
        g_elastic.log(INFO, "DAQControl is already started");
        return;
    }
    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postStartData();  
}

void DAQControl::handleStopDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Stopping Data");
    if (state_ == Control::Configured) // If data is not currently being produced log 
    {
        g_elastic.log(INFO, "DAQControl is already stopped");
        return;
    }
    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postStopData();  
}

void DAQControl::handleStartRunCommand(RunType which)
{
    g_elastic.log(INFO, "DAQControl: Starting Run of type {}", (int)which);
    if (config_.is_nano_enabled_ && which!=RunType::TestFlasher)
    {
        g_elastic.log(ERROR, "A flasher is enabled! Incorrect run type!");
    }
    else if (!config_.is_nano_enabled_ && which==RunType::TestFlasher)
    {
        g_elastic.log(ERROR, "No flashers are enabled! Needed for this run type!");
    }
}

void DAQControl::handleStopRunCommand()
{
    g_elastic.log(INFO, "DAQControl: Stopping Run");
}

void DAQControl::handleExitCommand()
{
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

    // Start the global state updating
    io_service_->post(boost::bind(&DAQControl::postUpdateState, this));

    thread_group_.join_all(); // Wait until all threads finish (BLOCKING)
}

void DAQControl::init()
{
    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postInit(); 
}

void DAQControl::setupFromConfig()
{
    config_.printConfig(); // Print the full configuration

    // Build the app topology of controllers from the configuration
    for (int controller=0; controller<config_.num_controllers_; controller++)
    {
        if (config_.configs_[controller].enabled_)
        {
            controllers_.push_back(new CLBController(config_.configs_[controller]));
        }
    }
}

void DAQControl::postUpdateState()
{
    std::vector<Control::Status> states;
    for(int i=0; i<controllers_.size(); i++) 
    {
        states.push_back(controllers_[i]->getState());
    }

    if (std::equal(states.begin() + 1, states.end(), states.begin()) && states[0]!=state_)
    {
        state_ = states[0];
        g_elastic.log(INFO, "DAQControl State Change to: {}", (int)state_);
    }

    sleep(1); // Sleep for a second before checking again

    // Post this method to the io_service again
    io_service_->post(boost::bind(&DAQControl::postUpdateState, this));
}


