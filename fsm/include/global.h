#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <util/async_component_group.h>

#include "control_bus/bus_master.h"
#include "fsm.h"

class Global {
    std::recursive_mutex mtx_dispatch_{};

    std::condition_variable cv_terminate_{};
    std::mutex mtx_terminate_{};
    std::atomic_bool running_{ true };

    AsyncComponentGroup async_components_;

    std::shared_ptr<ControlBus::BusMaster> control_bus_;

public:
    template <typename EventType>
    void
    sendEvent(EventType const& event)
    {
        std::lock_guard<std::recursive_mutex> lk{ mtx_dispatch_ };
        MainFSM::template dispatch<EventType>(event);
    }

    void sendControlMessage(ControlMessage&& message);

    void terminate();
    void waitUntilTerminated();

    void setupComponents();
    void runComponents();
    void notifyAndJoinComponents();
};

extern Global global;
