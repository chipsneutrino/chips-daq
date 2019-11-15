/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"

DAQControl::DAQControl(std::string config_file, bool disable_hv)
    : config_(config_file.c_str())
    , controllers_{}
    , n_threads_(1)
    , current_state_(Control::Initialising)
    , target_state_(Control::Ready)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{
    setupFromConfig(disable_hv); // Setup the topology from the configuration
}

void DAQControl::handleConfigCommand(std::string config_file)
{
    g_elastic.log(INFO, "DAQControl: Configure");
    config_.loadConfig(config_file.c_str()); // Load the new configuration
    for(int i=0; i<controllers_.size(); i++) 
    {
        if (controllers_[i]->dropped()) continue;
        if (!config_.configs_[i].enabled_) continue;
        controllers_[i]->setConfig(config_.configs_[i]);
        controllers_[i]->postConfigure();
    }
    target_state_ = Control::Configured;
}

void DAQControl::handleStartDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Starting Data");
    if (current_state_ == Control::Started) // If data is currently being produced log
    {
        g_elastic.log(INFO, "DAQControl is already started");
        return;
    }
    for(int i=0; i<controllers_.size(); i++) 
    {
        if (controllers_[i]->dropped()) continue;
        if (!config_.configs_[i].enabled_) continue;
        controllers_[i]->postStartData();  
    }
    target_state_ = Control::Started;
}

void DAQControl::handleStopDataCommand()
{
    g_elastic.log(INFO, "DAQControl: Stopping Data");
    if (current_state_ == Control::Configured) // If data is not currently being produced log 
    {
        g_elastic.log(INFO, "DAQControl is already stopped");
        return;
    }
    for(int i=0; i<controllers_.size(); i++) 
    {
        if (controllers_[i]->dropped()) continue;
        if (!config_.configs_[i].enabled_) continue;
        controllers_[i]->postStopData();  
    }
    target_state_ = Control::Configured;
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
    g_elastic.log(INFO, "DAQControl: Exit");
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
    io_service_->post(boost::bind(&DAQControl::stateUpdate, this));

    thread_group_.join_all(); // Wait until all threads finish (BLOCKING)
}

void DAQControl::init()
{
    for(int i=0; i<controllers_.size(); i++) controllers_[i]->postInit(); 
}

void DAQControl::setupFromConfig(bool disable_hv)
{
    if (disable_hv) g_elastic.log(INFO, "Will disable the HV on all controllers!");

    g_elastic.log(INFO, "Setting up {} controllers...", config_.num_controllers_);

    // Build the app topology of controllers from the configuration
    for (int controller=0; controller<config_.num_controllers_; controller++)
    {
        if (config_.configs_[controller].enabled_)
        {
            bool hv_flag;
            if (disable_hv) hv_flag = true;
            else hv_flag = !config_.configs_[controller].hv_enabled_;

            if (config_.configs_[controller].type_ == CLB)
            {
                controllers_.push_back(new CLBController(config_.configs_[controller], hv_flag));
            }
            else if (config_.configs_[controller].type_ == BBB)
            {
                controllers_.push_back(new BBBController(config_.configs_[controller], hv_flag));
            }
        }
    }
}

void DAQControl::stateUpdate()
{
    std::vector<Control::Status> states;
    for(int i=0; i<controllers_.size(); i++) 
    {
        if (controllers_[i]->dropped()) continue;
        if (!config_.configs_[i].enabled_) continue;

        Control::Status state = controllers_[i]->getState(); // Get the current controller state

	    int max_sleep_time   = 10000000;  // 10e6 us 
	    int total_sleep_time = 0;
	    int sleep_time       = 500; // us

        while( state != target_state_ && total_sleep_time < max_sleep_time ){
            usleep(sleep_time);
            total_sleep_time += sleep_time;
            state = controllers_[i]->getState(); // Get the current controller state                                                                   
        }

        if (state != target_state_ && !controllers_[i]->isWorking()) 
        {
            g_elastic.log(WARNING, "Dropping controller ({})", controllers_[i]->getID());
            controllers_[i]->drop(); // For now we just drop the controller
            continue;
        }

        states.push_back(controllers_[i]->getState());
    }

    if (states.size() == 0)
    {
        g_elastic.log(ERROR, "All controllers have been dropped!");
        return;
    }

    // If all controllers have moved to the next state update the current_state_
    if (std::equal(states.begin() + 1, states.end(), states.begin()) && states[0]!=current_state_)
    {
        current_state_ = states[0];
        g_elastic.log(INFO, "DAQControl State Change to: {}", (int)current_state_);
    }

    usleep(200); // Sleep for 200 microseconds

    // Post this method to the io_service again
    io_service_->post(boost::bind(&DAQControl::stateUpdate, this));
}


