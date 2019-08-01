#include "global.h"
#include "daqonite/observer.h"
#include "daqontrol/observer.h"
#include "daqsitter/observer.h"
#include "ops_uplink.h"

Global global{};

void Global::waitUntilTerminated()
{
    std::unique_lock<std::mutex> lk{ mtx_terminate_ };
    cv_terminate_.wait(lk, [this] { return !running_; });
}

void Global::terminate()
{
    running_ = false;
    cv_terminate_.notify_all();
}

void Global::setupComponents()
{
    control_bus_ = std::make_shared<ControlBus::BusMaster>();
    async_components_.add(control_bus_);
    async_components_.add(std::make_shared<Daqonite::Observer>());
    async_components_.add(std::make_shared<Daqontrol::Observer>());
    async_components_.add(std::make_shared<Daqsitter::Observer>());
    async_components_.add(std::make_shared<OpsUplink>());
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