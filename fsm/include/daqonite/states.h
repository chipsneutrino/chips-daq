#pragma once

#include "daqonite/events.h"
#include "daqonite/fsm.h"

namespace Daqonite {
namespace states {
    class Offline : public FSM {
        void entry() override;
        void react(events::Disconnected const&) override {}
    };

    class Unknown : public FSM {
        void entry() override;
        void react(events::Idle const&) override;
        void react(events::Mining const&) override;
        void react(events::Connected const&) override {}
    };

    class Idle : public FSM {
        void entry() override;
        void react(events::Mining const&) override;
    };

    class Mining : public FSM {
        void entry() override;
        void react(events::Idle const&) override;
    };
}
}