#pragma once

#include <tinyfsm.hpp>

#include "daqontrol/events.h"

namespace Daqontrol {
class FSM : public tinyfsm::Fsm<FSM> {
public:
    virtual void entry(void) = 0;
    void exit(void) {}

    void react(tinyfsm::Event const&) {}
    virtual void react(events::Disconnected const&);
    virtual void react(events::Connected const&);
    virtual void react(events::Initialising const&) {}
    virtual void react(events::Idle const&) {}
    virtual void react(events::Configured const&) {}
    virtual void react(events::Started const&) {}
};
}
