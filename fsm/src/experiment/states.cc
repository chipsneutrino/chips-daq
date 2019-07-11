#include <util/control_msg.h>
#include <util/elastic_interface.h>

#include "control_bus/states.h"
#include "daqonite/states.h"
#include "daqontrol/states.h"
#include "experiment/states.h"
#include "global.h"
#include "global_events.h"

namespace Experiment {
namespace states {
    /// Init State
    void Init::entry()
    {
        g_elastic.log(INFO, "Experiment : Init");
        global.sendEvent(StateUpdate{});
    }

    void Init::react(StateUpdate const&)
    {
        if (ControlBus::FSM::is_in_state<ControlBus::states::Online>() 
            && Daqonite::FSM::is_in_state<Daqonite::states::Idle>()
            && Daqontrol::FSM::is_in_state<Daqontrol::states::Idle>()) 
        {
            transit<states::Ready>();
        }
    }

    /// Exit State
    void Exit::entry()
    {
        g_elastic.log(INFO, "Experiment : Exit");
        global.sendEvent(StateUpdate{});

        global.terminate();
    }

    void Exit::react(StateUpdate const&)
    {
    }

    /// Ready State
    void Ready::entry()
    {
        g_elastic.log(INFO, "Experiment : Ready");
        global.sendEvent(StateUpdate{});
    }

    void Ready::react(OpsCommands::Config const& e)
    {
        transit<states::Configuring>();
    }

    void Ready::react(StateUpdate const&)
    {
    }

    /// Configuring State
    void Configuring::entry()
    {
        g_elastic.log(INFO, "Experiment : Configuring");
        global.sendEvent(StateUpdate{});

        {
            ControlMessage msg{};
            msg.Discriminator = ControlMessage::Config::Discriminator;
            global.sendControlMessage(std::move(msg));
        }
    }

    void Configuring::react(StateUpdate const&)
    {
        if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQonite is in a valid state
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Idle>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()) {
            transit<states::Configured>();
            return;
        }
    }

    /// Configured State
    void Configured::entry()
    {
        g_elastic.log(INFO, "Experiment : Configured");
        global.sendEvent(StateUpdate{});
    }

    void Configured::react(StateUpdate const&)
    {
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()) {
            transit<states::Error>();
            return;
        }
    }

    void Configured::react(OpsCommands::StartData const& e)
    {
        transit<states::StartingData>();
    }

    /// StartingData State
    void StartingData::entry()
    {
        g_elastic.log(INFO, "Experiment : StartingData");
        global.sendEvent(StateUpdate{});

        {
            ControlMessage msg{};
            msg.Discriminator = ControlMessage::StartData::Discriminator;
            global.sendControlMessage(std::move(msg));
        }
    }

    void StartingData::react(StateUpdate const&)
    {
        if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQonite is in a valid state
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Idle>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) {
            transit<states::Started>();
            return;
        }
    }

    /// Started State
    void Started::entry()
    {
        g_elastic.log(INFO, "Experiment : Started");
        global.sendEvent(StateUpdate{});
    }

    void Started::react(StateUpdate const&)
    {
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) {
            transit<states::Error>();
            return;
        }
    }

    void Started::react(OpsCommands::StopData const& e)
    {
        transit<states::StoppingData>();
    }

    void Started::react(OpsCommands::StartRun const& e)
    {
        // We perform the ControlMessage "action" before calling entry
        // We can then pass the RunType from here
        auto action = [=] { 
            ControlMessage msg{};
            msg.Discriminator = ControlMessage::StartRun::Discriminator;
            msg.Payload.pStartRun = ControlMessage::StartRun{};
            msg.Payload.pStartRun.Which = e.type;
            msg.Payload.pStartRun.flasher_v = e.flasher_v;
            global.sendControlMessage(std::move(msg)); 
        };
        transit<states::StartingRun>(action);
    }

    /// StoppingData State
    void StoppingData::entry()
    {
        g_elastic.log(INFO, "Experiment : StoppingData");
        global.sendEvent(StateUpdate{});

        {
            ControlMessage msg{};
            msg.Discriminator = ControlMessage::StopData::Discriminator;
            global.sendControlMessage(std::move(msg));
        }
    }

    void StoppingData::react(StateUpdate const&)
    {
        if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQonite is in a valid state
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Idle>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()) {
            transit<states::Configured>();
            return;
        }
    }

    /// StartingRun State
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

        // Check DAQonite is in a valid state
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Idle>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Mining>() && Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) {
            transit<states::Running>();
            return;
        }
    }

    void Running::entry()
    {
        g_elastic.log(INFO, "Experiment : Running");
        global.sendEvent(StateUpdate{});
    }

    void Running::react(StateUpdate const&)
    {
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Mining>() && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) {
            transit<states::Error>();
            return;
        }
    }

    void Running::react(OpsCommands::StopRun const& e)
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

        // Check DAQonite is in a valid state
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::Mining>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Idle>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) {
            transit<states::Started>();
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
