#pragma once

#include "daqontrol/events.h"
#include "daqontrol/fsm.h"

namespace Daqontrol {
namespace states {
    class Offline : public FSM {
        void entry() override;
        void react(events::Disconnected const&) override {}
    };

    class Unknown : public FSM {
        void entry() override;
        void react(events::Initialising const&) override;
        void react(events::Ready const&) override;
        void react(events::Configured const&) override;
        void react(events::Started const&) override;
        void react(events::Connected const&) override {}
    };

    class Initialising : public FSM {
        void entry() override;
        void react(events::Ready const&) override;
    };

    class Ready : public FSM {
        void entry() override;
        void react(events::Configured const&) override;
    };

    class Configured : public FSM {
        void entry() override;
        void react(events::Started const&) override;
    };

    class Started : public FSM {
        void entry() override;
        void react(events::Configured const&) override;
    };
}
}