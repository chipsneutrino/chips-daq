#include <util/control_msg.h>
#include <util/elastic_interface.h>

#include "control_bus/states.h"
#include "daqonite/states.h"
#include "daqontrol/states.h"
#include "daqsitter/states.h"
#include "experiment/states.h"
#include "global.h"
#include "global_events.h"

namespace Experiment {
namespace states {
    /// Init State
    void Init::entry()
    {
        g_elastic.log(INFO, "Experiment : Init");
        g_elastic.state("fsm", "Init");
        global.sendEvent(StateUpdate{});
    }

    void Init::react(StateUpdate const&)
    {
        if (ControlBus::FSM::is_in_state<ControlBus::states::Online>() 
            && Daqonite::FSM::is_in_state<Daqonite::states::Ready>()
            && Daqontrol::FSM::is_in_state<Daqontrol::states::Ready>()
            && Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>()) 
        {
            transit<states::Ready>();
        }
    }

    /// Exit State
    void Exit::entry()
    {
        g_elastic.log(INFO, "Experiment : Exit");
        g_elastic.state("fsm", "Exit");
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
        g_elastic.state("fsm", "Ready");
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
        g_elastic.state("fsm", "Configuring");
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
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Ready>() && !Daqonite::FSM::is_in_state<Daqonite::states::Running>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Ready>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check DAQsitter is in a valid state
        if (!Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>() && !Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Ready>() 
            && Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>()) {
            transit<states::Configured>();
            return;
        }
    }

    /// Configured State
    void Configured::entry()
    {
        g_elastic.log(INFO, "Experiment : Configured");
        g_elastic.state("fsm", "Configured");
        global.sendEvent(StateUpdate{});
    }

    void Configured::react(StateUpdate const&)
    {
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Ready>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>()) {
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
        g_elastic.state("fsm", "StartingData");
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
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Ready>() && !Daqonite::FSM::is_in_state<Daqonite::states::Running>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Ready>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check DAQsitter is in a valid state
        if (!Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>() && !Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Ready>() 
            && Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()
            && Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Started>();
            return;
        }
    }

    /// Started State
    void Started::entry()
    {
        g_elastic.log(INFO, "Experiment : Started");
        g_elastic.state("fsm", "Started");
        global.sendEvent(StateUpdate{});
    }

    void Started::react(StateUpdate const&)
    {
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Ready>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()
            && !Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
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
            global.sendControlMessage(std::move(msg)); 
        };
        transit<states::StartingRun>(action);
    }

    /// StoppingData State
    void StoppingData::entry()
    {
        g_elastic.log(INFO, "Experiment : StoppingData");
        g_elastic.state("fsm", "StoppingData");
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
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Ready>() && !Daqonite::FSM::is_in_state<Daqonite::states::Running>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Ready>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check DAQsitter is in a valid state
        if (!Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>() && !Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Ready>() 
            && Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>()) {
            transit<states::Configured>();
            return;
        }
    }

    /// StartingRun State
    void StartingRun::entry()
    {
        g_elastic.log(INFO, "Experiment : StartingRun");
        g_elastic.state("fsm", "StartingRun");
        global.sendEvent(StateUpdate{});
    }

    void StartingRun::react(StateUpdate const&)
    {
        if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQonite is in a valid state
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Ready>() && !Daqonite::FSM::is_in_state<Daqonite::states::Running>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Ready>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check DAQsitter is in a valid state
        if (!Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>() && !Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Running>() 
            && Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()
            && Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Running>();
            return;
        }
    }

    void Running::entry()
    {
        g_elastic.log(INFO, "Experiment : Running");
        g_elastic.state("fsm", "Running");
        global.sendEvent(StateUpdate{});
    }

    void Running::react(StateUpdate const&)
    {
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Running>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()
            && !Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
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
        g_elastic.state("fsm", "StoppingRun");
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
        if (!Daqonite::FSM::is_in_state<Daqonite::states::Ready>() && !Daqonite::FSM::is_in_state<Daqonite::states::Running>()) {
            transit<states::Error>();
            return;
        }

        // Check DAQontrol is in a valid state
        if (!Daqontrol::FSM::is_in_state<Daqontrol::states::Ready>() 
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()
            && !Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) 
        {
            transit<states::Error>();
            return;
        }

        // Check DAQsitter is in a valid state
        if (!Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>() && !Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Error>();
            return;
        }

        // Check applications are in the correct state to transmit state
        if (Daqonite::FSM::is_in_state<Daqonite::states::Ready>() 
            && Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()
            && Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            transit<states::Started>();
            return;
        }
    }

    void Error::entry()
    {
        g_elastic.log(INFO, "Experiment : Error");
        g_elastic.state("fsm", "Error");
        global.sendEvent(StateUpdate{});
    }

    void Error::react(StateUpdate const&)
    {
    }
}
}
