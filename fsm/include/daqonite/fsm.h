#pragma once

#include <tinyfsm.hpp>

#include <util/logging.h>

#include "daqonite/events.h"

namespace Daqonite {
class FSM : public tinyfsm::Fsm<FSM>, protected Logging {
public:
    FSM();

    virtual void entry(void) = 0;
    void exit(void) { }

    void react(tinyfsm::Event const&) { }
    virtual void react(events::Disconnected const&);
    virtual void react(events::Connected const&);
    virtual void react(events::Ready const&) { }
    virtual void react(events::Running const&) { }
};
}
