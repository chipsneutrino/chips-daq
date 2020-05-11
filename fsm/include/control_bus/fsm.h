#pragma once

#include <tinyfsm.hpp>

#include <util/logging.h>

#include "control_bus/events.h"

namespace ControlBus {
class FSM : public tinyfsm::Fsm<FSM>, protected Logging {
public:
    FSM();

    virtual void entry(void) = 0;
    void exit(void) { }

    void react(tinyfsm::Event const&) { }
    virtual void react(events::Disconnected const&) { }
    virtual void react(events::Connected const&) { }
};
}
