#include <util/control_msg.h>
#include <util/elastic_interface.h>

#include "control_bus/states.h"
#include "daqonite/states.h"
#include "experiment/states.h"
#include "global.h"
#include "global_events.h"

namespace Experiment {
namespace states {
    void Init::entry()
    {
        g_elastic.log(INFO, "Experiment : Init");
        global.sendEvent(StateUpdate{});
    }

    void Init::react(StateUpdate const&)
    {
        if (ControlBus::FSM::is_in_state<ControlBus::states::Online>() && Daqonite::FSM::is_in_state<Daqonite::states::Idle>()) {
            transit<states::Ready>();
        }
    }

    void Exit::entry()
    {
        g_elastic.log(INFO, "Experiment : Exit");
        global.sendEvent(StateUpdate{});

        global.terminate();
    }

    void Ready::entry()
    {
        g_elastic.log(INFO, "Experiment : Ready");
        global.sendEvent(StateUpdate{});
    }

    void Ready::react(OpsCommands::StartRun const& e)
    {
        transit<states::StartingRun>();
    }

    void StartingRun::entry()
    {
        g_elastic.log(INFO, "Experiment : StartingRun");
        global.sendEvent(StateUpdate{});

        {
            ControlMessage msg{};
            msg.Discriminator = ControlMessage::StartRun::Discriminator;
            msg.Payload.pStartRun = ControlMessage::StartRun{};
            msg.Payload.pStartRun.Which = RunType::TestDAQ; // FIXME: parametric
            global.sendControlMessage(std::move(msg));
        }
    }

    void StartingRun::react(StateUpdate const&)
    {
        if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            transit<states::Error>();
            return;
        }

        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::RunInProgress>()) {
            transit<states::Error>();
            return;
        }

        if (Daqonite::FSM::is_in_state<Daqonite::states::RunInProgress>()) {
            transit<states::Run>();
            return;
        }
    }

    void Run::entry()
    {
        g_elastic.log(INFO, "Experiment : Run");
        global.sendEvent(StateUpdate{});
    }

    void StoppingRun::entry()
    {
        g_elastic.log(INFO, "Experiment : StoppingRun");
        global.sendEvent(StateUpdate{});
    }

    void Error::entry()
    {
        g_elastic.log(INFO, "Experiment : Error");
        global.sendEvent(StateUpdate{});
    }
}
}
