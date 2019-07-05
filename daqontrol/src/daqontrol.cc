/**
 * Program name: DAQontrol, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 */

#include <boost/program_options.hpp>

#include "util/command_receiver.h"
#include "util/signal_receiver.h"
#include <util/elastic_interface.h>

#include "daq_control.h"

namespace exit_code {
static constexpr int success = 0;
}

int main(int argc, char* argv[])
{
    // Default settings
    std::string config = "../data/config.opt";

    boost::program_options::options_description desc("Options");
    desc.add_options()("help,h", "DAQontrol...")
        ("config,c", boost::program_options::value<std::string>(&config), 
            "Configuration file (../data/config.opt)");

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
        std::shared_ptr<DAQControl> daq_control{ new DAQControl(config) };

        std::unique_ptr<SignalReceiver> signal_receiver{ new SignalReceiver };
        signal_receiver->setHandler(daq_control);
        signal_receiver->runAsync();

        std::unique_ptr<CommandReceiver> cmd_receiver{ new CommandReceiver };
        cmd_receiver->setHandler(daq_control);
        cmd_receiver->runAsync();

	    daq_control->init();
        //daq_control->configure();
        //daq_control->start();
        //daq_control->stop();

        daq_control->run();

	    cmd_receiver->join();
        signal_receiver->join();
    }

    g_elastic.log(INFO, "Stopping DAQontrol");
    return exit_code::success;
}
