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
        void react(events::Ready const&) override;
        void react(events::Running const&) override;
        void react(events::Connected const&) override {}
    };

    class Ready : public FSM {
        void entry() override;
        void react(events::Running const&) override;
    };

    class Running : public FSM {
        void entry() override;
        void react(events::Ready const&) override;
    };
}
}