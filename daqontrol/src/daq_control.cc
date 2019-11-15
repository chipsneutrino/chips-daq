/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"

DAQControl::DAQControl()
    : controllers_{}
    , current_state_(Control::Ready)
    , target_state_(Control::Ready)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{
}

void DAQControl::handleConfigCommand(std::string config_file)
{
    g_elastic.log(INFO, "DAQControl: Configure");
    if(controllers_.size() != 0) // We are currently configured so we need to reset first
    {

        g_elastic.log(INFO, "DAQControl: Clearing old detector configuration");
        for (auto c : controllers_) 
        {
            if (!c->dropped() && c->config_.enabled_) c->postReset();
        }

        sleep(10);  // Wait for the reset to complete

        // Delete and clear the old controllers
        for (auto c : controllers_) delete c;
        controllers_.clear();
    }

    current_state_ = Control::Ready;

    // Build the app topology of controllers from the configuration
    config_ = DAQConfig(config_file.c_str());
    for (int c=0; c<config_.num_controllers_; c++)
    {
        if (config_.configs_[c].type_ == CLB) controllers_.push_back(new CLBController(config_.configs_[c]));
        else if (config_.configs_[c].type_ == BBB) controllers_.push_back(new BBBController(config_.configs_[c]));
    }

    // Configure the controllers
    for (auto c : controllers_) 
    {
        if (c->config_.enabled_) c->postConfigure();
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
    for (auto c : controllers_) 
    {
        if (!c->dropped() && c->config_.enabled_) c->postStartData();
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
    for (auto c : controllers_) 
    {
        if (!c->dropped() && c->config_.enabled_) c->postStopData();
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

void DAQControl::runAsync()
{
    // Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "DAQ Control starting I/O service on {} threads", 1);
    for (int i = 0; i < 1; ++i) {
        thread_group_.create_thread(boost::bind(&DAQControl::ioServiceThread, this));
    }

    io_service_->post(boost::bind(&DAQControl::stateUpdate, this)); // Start the global state updating

    thread_group_.join_all(); // Wait until all threads finish (BLOCKING)
}

void DAQControl::stateUpdate()
{
    std::vector<Control::Status> states;
    for (auto c : controllers_) 
    {
        if (!c->dropped() && c->config_.enabled_)
        {
            Control::Status state; // Get the current controller state

            int total_sleep_time = 0;
            while(state != target_state_ && total_sleep_time < 1000000){
                usleep(500);
                total_sleep_time += 500;
                state = c->getState(); // Get the current controller state                                                                   
            } 

            if (state != target_state_ && !c->isWorking()) 
            {
                g_elastic.log(WARNING, "Dropping controller ({})", c->getID());
                c->drop(); // For now we just drop the controller
                continue;
            }

            states.push_back(state);
        }  
    }

    if(states.size()!=0)
    {
        if (std::equal(states.begin() + 1, states.end(), states.begin()) && states[0]!=current_state_)
        {
            current_state_ = states[0];
            g_elastic.log(INFO, "DAQControl State Change to: {}", (int)current_state_);
        }
    }

    usleep(10000); // Sleep for 10000 microseconds

    // Post this method to the io_service again
    io_service_->post(boost::bind(&DAQControl::stateUpdate, this));
}


