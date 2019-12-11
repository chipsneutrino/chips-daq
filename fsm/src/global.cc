#include <iostream>

#include <boost/program_options.hpp>

#include "daqonite/observer.h"
#include "daqontrol/observer.h"
#include "daqsitter/observer.h"
#include "global.h"
#include "ops_uplink.h"
#include "reporter.h"

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
    desc.add_options()("help,h", "FSM");

    opts::variables_map vm {};
    opts::store(opts::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    opts::notify(vm);

    ops_bus_url_ = Config::getString("OPS_BUS");
    daqonite_bus_url_ = Config::getString("DAQONITE_BUS");
    daqontrol_bus_url_ = Config::getString("DAQONTROL_BUS");
    daqsitter_bus_url_ = Config::getString("DAQSITTER_BUS");
    control_bus_url_ = Config::getString("CONTROL_BUS");
}

void Global::setupComponents()
{
    control_bus_ = std::make_shared<ControlBus::BusMaster>(control_bus_url_);
    async_components_.add(control_bus_);
    async_components_.add(std::make_shared<Daqonite::Observer>(daqonite_bus_url_));
    async_components_.add(std::make_shared<Daqontrol::Observer>(daqontrol_bus_url_));
    async_components_.add(std::make_shared<Daqsitter::Observer>(daqsitter_bus_url_));
    async_components_.add(std::make_shared<OpsUplink>(ops_bus_url_));
    async_components_.add(std::make_shared<Reporter>());
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