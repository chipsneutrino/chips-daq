#include <thread>

#include "reporter.h"
#include <util/elastic_interface.h>

#include "control_bus/states.h"
#include "daqonite/states.h"
#include "daqontrol/states.h"
#include "daqsitter/states.h"
#include "experiment/states.h"
#include "global.h"
#include "global_events.h"

Reporter::Reporter()
{
}

void Reporter::run()
{
    while (running_) {

        // Only report every 2 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // Report on the ControlBus state
        if(ControlBus::FSM::is_in_state<ControlBus::states::Online>()) {
            g_elastic.state("ControlBus", "Online");
        } else if (ControlBus::FSM::is_in_state<ControlBus::states::Offline>()) {
            g_elastic.state("ControlBus", "Offline");
        }

        // Report on the Experiment state
        if(Experiment::FSM::is_in_state<Experiment::states::Init>()) {
            g_elastic.state("Experiment", "Init");
        } else if (Experiment::FSM::is_in_state<Experiment::states::Exit>()) {
            g_elastic.state("Experiment", "Exit");
        } else if (Experiment::FSM::is_in_state<Experiment::states::Ready>()) {
            g_elastic.state("Experiment", "Ready");
        } else if (Experiment::FSM::is_in_state<Experiment::states::Configuring>()) {
            g_elastic.state("Experiment", "Configuring");
        } else if (Experiment::FSM::is_in_state<Experiment::states::Configured>()) {
            g_elastic.state("Experiment", "Configured");
        } else if (Experiment::FSM::is_in_state<Experiment::states::StartingData>()) {
            g_elastic.state("Experiment", "StartingData");
        } else if (Experiment::FSM::is_in_state<Experiment::states::Started>()) {
            g_elastic.state("Experiment", "Started");
        } else if (Experiment::FSM::is_in_state<Experiment::states::StoppingData>()) {
            g_elastic.state("Experiment", "StoppingData");
        } else if (Experiment::FSM::is_in_state<Experiment::states::StartingRun>()) {
            g_elastic.state("Experiment", "StartingRun");
        } else if (Experiment::FSM::is_in_state<Experiment::states::Running>()) {
            g_elastic.state("Experiment", "Running");
        } else if (Experiment::FSM::is_in_state<Experiment::states::StoppingRun>()) {
            g_elastic.state("Experiment", "StoppingRun");
        } else if (Experiment::FSM::is_in_state<Experiment::states::Error>()) {
            g_elastic.state("Experiment", "Error");
        }

        // Report on Daqonite state
        if(Daqonite::FSM::is_in_state<Daqonite::states::Offline>()) {
            g_elastic.state("Daqonite", "Offline");
        } else if (Daqonite::FSM::is_in_state<Daqonite::states::Unknown>()) {
            g_elastic.state("Daqonite", "Unknown");
        } else if (Daqonite::FSM::is_in_state<Daqonite::states::Ready>()) {
            g_elastic.state("Daqonite", "Ready");
        } else if (Daqonite::FSM::is_in_state<Daqonite::states::Running>()) {
            g_elastic.state("Daqonite", "Running");
        }     

        // Report on Daqontrol state
        if(Daqontrol::FSM::is_in_state<Daqontrol::states::Offline>()) {
            g_elastic.state("Daqontrol", "Offline");
        } else if (Daqontrol::FSM::is_in_state<Daqontrol::states::Unknown>()) {
            g_elastic.state("Daqontrol", "Unknown");
        } else if (Daqontrol::FSM::is_in_state<Daqontrol::states::Initialising>()) {
            g_elastic.state("Daqontrol", "Initialising");
        } else if (Daqontrol::FSM::is_in_state<Daqontrol::states::Ready>()) {
            g_elastic.state("Daqontrol", "Ready");
        } else if (Daqontrol::FSM::is_in_state<Daqontrol::states::Configured>()) {
            g_elastic.state("Daqontrol", "Configured");
        } else if (Daqontrol::FSM::is_in_state<Daqontrol::states::Started>()) {
            g_elastic.state("Daqontrol", "Started");
        } 

        // Report on Daqsitter state
        if(Daqsitter::FSM::is_in_state<Daqsitter::states::Offline>()) {
            g_elastic.state("Daqsitter", "Offline");
        } else if (Daqsitter::FSM::is_in_state<Daqsitter::states::Unknown>()) {
            g_elastic.state("Daqsitter", "Unknown");
        } else if (Daqsitter::FSM::is_in_state<Daqsitter::states::Ready>()) {
            g_elastic.state("Daqsitter", "Ready");
        } else if (Daqsitter::FSM::is_in_state<Daqsitter::states::Started>()) {
            g_elastic.state("Daqsitter", "Started");
        }         
    }
}
