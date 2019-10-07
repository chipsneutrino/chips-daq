#include <iostream>

#include <boost/program_options.hpp>

#include "daqonite/observer.h"
#include "daqontrol/observer.h"
#include "daqsitter/observer.h"
#include "global.h"
#include "ops_uplink.h"

Global global {};

void Global::waitUntilTerminated()
{
    std::unique_lock<std::mutex> lk { mtx_terminate_ };
    cv_terminate_.wait(lk, [this] { return !running_; });
}

void Global::terminate()
{
    running_ = false;
    cv_terminate_.notify_all();
}

void Global::readSettings(int argc, char* argv[])
{
    namespace opts = boost::program_options;

    // Argument handling
    opts::options_description desc { "Options" };
    desc.add_options()("help,h", "FSM")
                      ("ops-bus-url", opts::value(&ops_bus_url_)->implicit_value("ipc:///tmp/chips_ops.ipc"), "where FSM listens for ops messages")
                      ("daqonite-bus-url", opts::value(&daqonite_bus_url_)->implicit_value("ipc:///tmp/chips_daqonite.ipc"), "where FSM listens for state messages from DAQonite")
                      ("daqontrol-bus-url", opts::value(&daqontrol_bus_url_)->implicit_value("ipc:///tmp/chips_daqontrol.ipc"), "where FSM listens for state messages from DAQontrol")
                      ("daqsitter-bus-url", opts::value(&daqsitter_bus_url_)->implicit_value("ipc:///tmp/chips_daqsitter.ipc"), "where FSM listens for state messages from DAQsitter")
                      ("control-bus-url", opts::value(&control_bus_url_)->implicit_value("ipc:///tmp/chips_control.ipc"), "where FSM sends control messages");

    opts::variables_map vm {};
    opts::store(opts::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    opts::notify(vm);
}

void Global::setupComponents()
{
    control_bus_ = std::make_shared<ControlBus::BusMaster>(control_bus_url_);
    async_components_.add(control_bus_);
    async_components_.add(std::make_shared<Daqonite::Observer>(daqonite_bus_url_));
    async_components_.add(std::make_shared<Daqontrol::Observer>(daqontrol_bus_url_));
    async_components_.add(std::make_shared<Daqsitter::Observer>(daqsitter_bus_url_));
    async_components_.add(std::make_shared<OpsUplink>(ops_bus_url_));
}

void Global::runComponents()
{
    async_components_.runAsync();
}

void Global::notifyAndJoinComponents()
{
    async_components_.notifyJoin();
    async_components_.join();
}

void Global::sendControlMessage(ControlMessage&& message)
{
    control_bus_->publish(std::move(message));
}