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

    void Exit::react(StateUpdate const&)
    {
    }

    void Ready::entry()
    {
        g_elastic.log(INFO, "Experiment : Ready");
        global.sendEvent(StateUpdate{});
    }

    void Ready::react(OpsCommands::StartRun const& e)
    {
        // We perform the ControlMessage "action" before calling entry
        // We can then pass the RunType from here
        auto action = [=] { 
            ControlMessage msg{};
            msg.Discriminator = ControlMessage::StartRun::Discriminator;
            msg.Payload.pStartRun = ControlMessage::StartRun{};
            msg.Payload.pStartRun.Which = e.type;
            global.sendControlMessage(std::move(msg)); 
        };
        transit<states::StartingRun>(action);
    }

    void Ready::react(StateUpdate const&)
    {
    }

    void StartingRun::entry()
    {
        g_elastic.log(INFO, "Experiment : StartingRun");
        global.sendEvent(StateUpdate{});
    }

    void StartingRun::react(StateUpdate const&)
    {
        if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            transit<states::Error>();
            return;
        }

        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }

        if (Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Run>();
            return;
        }
    }

    void Run::entry()
    {
        g_elastic.log(INFO, "Experiment : Run");
        global.sendEvent(StateUpdate{});
    }

    void Run::react(StateUpdate const&)
    {
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }
    }

    void Run::react(OpsCommands::StopRun const& e)
    {
        transit<states::StoppingRun>();
    }

    void StoppingRun::entry()
    {
        g_elastic.log(INFO, "Experiment : StoppingRun");
        global.sendEvent(StateUpdate{});

        {
            ControlMessage msg{};
            msg.Discriminator = ControlMessage::StopRun::Discriminator;
            global.sendControlMessage(std::move(msg));
        }
    }

    void StoppingRun::react(StateUpdate const&)
    {
        if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            transit<states::Error>();
            return;
        }

        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }

        if (Daqonite::FSM::is_in_state<Daqonite::states::Idle>()) {
            transit<states::Ready>();
            return;
        }
    }

    void Error::entry()
    {
        g_elastic.log(INFO, "Experiment : Error");
        global.sendEvent(StateUpdate{});
    }

    void Error::react(StateUpdate const&)
    {
    }
}
}
