#pragma once

#include "control_bus/fsm.h"

namespace ControlBus {
namespace states {
    class Offline : public FSM {
        void entry() override;
        void react(events::Connected const& e);
    };

    class Online : public FSM {
        void entry() override;
        void react(events::Disconnected const& e) override;
    };
}
}
