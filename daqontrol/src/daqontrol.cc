/**
 * Program name: DAQontrol, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 */

#include "util/command_receiver.h"
#include "daq_control.h"
#include "util/signal_receiver.h"
#include <util/elastic_interface.h>

namespace exit_code {
static constexpr int success = 0;
}

namespace settings {
// Default settings    
static std::string config_name = "../data/config.opt";
}

int main(int argc, char* argv[])
{
    // Initialise the elasticsearch interface.
    g_elastic.init(true, false, 10); // log to stdout and use 10 threads for indexing

    g_elastic.log(INFO, "Starting DAQontrol");

    {
        // Main entry point.
        std::shared_ptr<DAQControl> daq_control{ new DAQControl(settings::config_name) };

        std::unique_ptr<SignalReceiver> signal_receiver{ new SignalReceiver };
        signal_receiver->setHandler(daq_control);
        signal_receiver->runAsync();

        std::unique_ptr<CommandReceiver> cmd_receiver{ new CommandReceiver };
        cmd_receiver->setHandler(daq_control);
        cmd_receiver->runAsync();

        daq_control->run();

	    //daq_control->setInitValues();
	    //daq_control->clbEvent(ClbEvents::INIT);
	    daq_control->init();     // it calls setInitValues and INIT
	    //daq_control->setPMTs();
	    //daq_control->disableHV();

	    cmd_receiver->join();     //// Why do this throw an error?
        signal_receiver->join();
    }

    g_elastic.log(INFO, "Stopping DAQontrol");
    return exit_code::success;
}