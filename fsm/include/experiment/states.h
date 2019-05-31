#include "experiment/fsm.h"

namespace Experiment {
namespace states {
    class Init : public FSM {
        void entry() override;
        void react(StateUpdate const&) override;
    };
    class Exit : public FSM {
        void entry() override;
        void react(StateUpdate const&) override;
    };
    class Ready : public FSM {
        void entry() override;
        void react(OpsCommands::StartRun const&) override;
        void react(StateUpdate const&) override;
    };
    class StartingRun : public FSM {
        void entry() override;
        void react(StateUpdate const&) override;
    };
    class Run : public FSM {
        void entry() override;
        void react(OpsCommands::StopRun const&) override;
        void react(StateUpdate const&) override;
    };
    class StoppingRun : public FSM {
        void entry() override;
        void react(StateUpdate const&) override;
    };
    class Error : public FSM {
        void entry() override;
        void react(StateUpdate const&) override;
    };
}
}