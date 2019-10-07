#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>

#include <util/async_component_group.h>

#include "control_bus/bus_master.h"
#include "fsm.h"

class Global {
    std::recursive_mutex mtx_dispatch_ {};

    std::condition_variable cv_terminate_ {};
    std::mutex mtx_terminate_ {};
    std::atomic_bool running_ { true };

    AsyncComponentGroup async_components_ {};

    std::shared_ptr<ControlBus::BusMaster> control_bus_ {};

    std::string ops_bus_url_ {};
    std::string control_bus_url_ {};
    std::string daqonite_bus_url_ {};
    std::string daqontrol_bus_url_ {};
    std::string daqsitter_bus_url_ {};

public:
    template <typename EventType>
    void
    sendEvent(EventType const& event)
    {
        std::lock_guard<std::recursive_mutex> lk { mtx_dispatch_ };
        MainFSM::template dispatch<EventType>(event);
    }

    void sendControlMessage(ControlMessage&& message);

    void readSettings(int argc, char* argv[]);

    void terminate();
    void waitUntilTerminated();

    void setupComponents();
    void runComponents();
    void notifyAndJoinComponents();
};

extern Global global;
