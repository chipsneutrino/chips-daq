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
#include <string>

#include "daq_handler.h"
#include "daqonite_publisher.h"
#include "util/command_receiver.h"
#include "util/signal_receiver.h"
#include <util/chips_config.h>
#include <util/elastic_interface.h>
#include <util/logging.h>
#include <util/singleton_process.h>

namespace exit_code {
static constexpr int success = 0;
}

namespace settings {
std::string state_bus_url {};
std::string control_bus_url {};
std::string data_path {};
}

class Main : protected Logging {
public:
    Main()
        : Logging {}
    {
        setUnitName("Main");
    }

    void readSettings(int argc, char* argv[])
    {
        namespace opts = boost::program_options;

        // Argument handling
        opts::options_description desc { "Options" };
        desc.add_options()("help,h", "DAQonite");

        opts::variables_map vm {};
        opts::store(opts::command_line_parser(argc, argv).options(desc).run(), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            std::exit(exit_code::success);
        }

        opts::notify(vm);

        settings::state_bus_url = Config::getString("DAQONITE_BUS");
        settings::control_bus_url = Config::getString("CONTROL_BUS");
        settings::data_path = Config::getString("DATA_PATH");
    }

    int main(int argc, char* argv[])
    {
        // Check that no other DAQonite instances are running
        SingletonProcess singleton(11112);
        if (!singleton())
            throw std::runtime_error("DAQonite already running!");

        // Read configuration.
        readSettings(argc, argv);

        // Initialise the elasticsearch interface.
        g_elastic.init(true, false, 10); // log to stdout and use 10 threads for indexing
        log(INFO, "Checking hard hats, high-vis, boots and gloves!");

        {
            // Main entry point.
            std::shared_ptr<DAQHandler> daq_handler { new DAQHandler(settings::data_path) };
            std::shared_ptr<DaqonitePublisher> bus_publisher { new DaqonitePublisher(daq_handler, settings::state_bus_url) };

            std::unique_ptr<SignalReceiver> signal_receiver { new SignalReceiver };
            signal_receiver->setHandler(daq_handler);
            signal_receiver->runAsync();

            std::unique_ptr<CommandReceiver> cmd_receiver { new CommandReceiver(settings::control_bus_url) };
            cmd_receiver->setHandler(daq_handler);
            cmd_receiver->runAsync();

            bus_publisher->runAsync();
            daq_handler->run();

            bus_publisher->join();
            cmd_receiver->join();
            signal_receiver->join();
        }

        log(INFO, "Done for the day!");
        return exit_code::success;
    }
};

int main(int argc, char* argv[])
{
    Main app {};
    return app.main(argc, argv);
}
