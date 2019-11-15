/**
 * Program name: DAQontrol, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 */

#include <boost/program_options.hpp>

#include "util/command_receiver.h"
#include "util/signal_receiver.h"
#include <util/elastic_interface.h>
#include "util/singleton_process.h"

#include "daq_control.h"
#include "daqontrol_publisher.h"

namespace exit_code {
static constexpr int success = 0;
}

int main(int argc, char* argv[])
{
    // Check that no other DAQontrol instances are running
    SingletonProcess singleton(11111);
    if (!singleton()) throw std::runtime_error("DAQontrol already running!");

    std::string state_bus_url;
    std::string control_bus_url;

    boost::program_options::options_description desc("Options");
    desc.add_options()("help,h", "DAQontrol...")
        ("state-bus-url", boost::program_options::value<std::string>(&state_bus_url)->implicit_value("ipc:///tmp/chips_daqontrol.ipc"), "where DAQontrol publishes state messages")
        ("control-bus-url", boost::program_options::value<std::string>(&control_bus_url)->implicit_value("ipc:///tmp/chips_control.ipc"), "where DAQontrol listens for control messages");

    try
    {
        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }
        boost::program_options::notify(vm);
    }
    catch (const boost::program_options::error &e)
    {
        throw std::runtime_error("DAQontrol - po Argument Error");
    }
    catch (const std::runtime_error &e)
    {
        throw std::runtime_error("DAQontrol - runtime Argument Error");
    }

    // Initialise the elasticsearch interface.
    g_elastic.init(true, false, 10); // log to stdout and use 10 threads for indexing
    g_elastic.log(INFO, "Starting DAQontrol");

    {
        // Main entry point.
        std::shared_ptr<DAQControl> daq_control{ new DAQControl() };
        std::shared_ptr<DaqontrolPublisher> bus_publisher{ new DaqontrolPublisher(daq_control, state_bus_url) };

        std::unique_ptr<SignalReceiver> signal_receiver{ new SignalReceiver };
        signal_receiver->setHandler(daq_control);
        signal_receiver->runAsync();

        std::unique_ptr<CommandReceiver> cmd_receiver{ new CommandReceiver(control_bus_url) };
        cmd_receiver->setHandler(daq_control);
        cmd_receiver->runAsync();

        bus_publisher->runAsync();
        daq_control->runAsync();

        bus_publisher->join();
	    cmd_receiver->join();
        signal_receiver->join();
    }

    g_elastic.log(INFO, "Stopping DAQontrol");
    return exit_code::success;
}
