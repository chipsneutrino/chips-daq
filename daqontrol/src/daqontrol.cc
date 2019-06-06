/**
 * Program name: DAQontrol, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "util/command_receiver.h"
#include "daq_control.h"
#include "util/signal_receiver.h"
#include <util/elastic_interface.h>

namespace exit_code {
static constexpr int success = 0;
}

int main(int argc, char* argv[])
{
    // Initialise the elasticsearch interface.
    g_elastic.init(true, false, 10); // log to stdout and use 10 threads for indexing

    g_elastic.log(INFO, "Starting DAQontrol");

    {
        // Main entry point.
        std::shared_ptr<DAQControl> daq_control{ new DAQControl() };

        std::unique_ptr<SignalReceiver> signal_receiver{ new SignalReceiver };
        signal_receiver->setHandler(daq_control);
        signal_receiver->runAsync();

        std::unique_ptr<CommandReceiver> cmd_receiver{ new CommandReceiver };
        cmd_receiver->setHandler(daq_control);
        cmd_receiver->runAsync();

        daq_control->run();

        cmd_receiver->join();
        signal_receiver->join();
    }

    g_elastic.log(INFO, "Stopping DAQontrol");
    return exit_code::success;
}
