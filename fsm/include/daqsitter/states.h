#pragma once

#include "daqsitter/events.h"
#include "daqsitter/fsm.h"

namespace Daqsitter {
namespace states {
    class Offline : public FSM {
        void entry() override;
        void react(events::Disconnected const&) override {}
    };

    class Unknown : public FSM {
        void entry() override;
        void react(events::Ready const&) override;
        void react(events::Started const&) override;
        void react(events::Connected const&) override {}
    };

    class Ready : public FSM {
        void entry() override;
        void react(events::Started const&) override;
    };

    class Started : public FSM {
        void entry() override;
        void react(events::Ready const&) override;
    };
}
}