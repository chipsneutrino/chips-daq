#pragma once

#include <tinyfsm.hpp>

#include "global_events.h"
#include "ops_uplink.h"

namespace Experiment {
class FSM : public tinyfsm::Fsm<FSM> {
public:
    virtual void entry(void) = 0;
    void exit(void) {}

    void react(KillSignal const&);
    virtual void react(StateUpdate const& update) = 0;
    virtual void react(OpsCommands::Config const&) {}
    virtual void react(OpsCommands::StartData const&) {}
    virtual void react(OpsCommands::StopData const&) {}
    virtual void react(OpsCommands::StartRun const&) {}
    virtual void react(OpsCommands::StopRun const&) {}

    void react(tinyfsm::Event const&) {}
};
}
