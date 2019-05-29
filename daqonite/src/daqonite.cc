/**
 * Program name: DAQonite, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 *
 * Co-author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#include <boost/program_options.hpp>
#include <memory>

#include "command_receiver.h"
#include "daq_handler.h"
#include "elastic_interface.h"
#include "signal_receiver.h"

namespace exit_code {
static constexpr int success = 0;
}

namespace settings {
// Default settings
static bool collect_clb_data = true;
static bool collect_bbb_data = false;
}

void readSettings(int argc, char* argv[])
{
    namespace opts = boost::program_options;

    // Argument handling
    opts::options_description desc{ "Options" };
    desc.add_options()("help,h", "DAQonite");

    opts::variables_map vm{};
    opts::store(opts::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::exit(exit_code::success);
    }

    opts::notify(vm);
}

int main(int argc, char* argv[])
{
    // Read configuration.
    readSettings(argc, argv);

    // Initialise the elasticsearch interface.
    g_elastic.init(true, false, 10); // log to stdout and use 10 threads for indexing
    g_elastic.log(INFO, "Checking hard hats, high-vis, boots and gloves!");

    {
        // Main entry point.
        std::shared_ptr<DAQHandler> daq_handler{ new DAQHandler(settings::collect_clb_data, settings::collect_bbb_data) };

        std::unique_ptr<SignalReceiver> signal_receiver{ new SignalReceiver };
        signal_receiver->setHandler(daq_handler);
        signal_receiver->runAsync();

        std::unique_ptr<CommandReceiver> cmd_receiver{ new CommandReceiver };
        cmd_receiver->setHandler(daq_handler);
        cmd_receiver->runAsync();

        daq_handler->run();

        cmd_receiver->join();
        signal_receiver->join();
    }

    g_elastic.log(INFO, "Done for the day!");
    return exit_code::success;
}
