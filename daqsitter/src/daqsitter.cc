/**
 * Program name: DAQsitter, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include <boost/program_options.hpp>

#include "daqsitter_publisher.h"
#include "monitoring_handler.h"
#include "util/command_receiver.h"
#include "util/signal_receiver.h"
#include "util/singleton_process.h"
#include <util/config.h>
#include <util/elastic_interface.h>

namespace exit_code {
static constexpr int success = 0;
}

class Main : protected Logging {
public:
    Main()
        : Logging {}
    {
        setUnitName("Main");
    }

    int main(int argc, char* argv[])
    {
        // Check that no other DAQsitter instances are running
        SingletonProcess singleton(11113);
        if (!singleton())
            throw std::runtime_error("DAQsitter already running!");

        // Default settings
        std::string config = "../data/config.opt";
        bool elastic = false;
        bool file = false;
        float sample_frac = 0.001;

        const std::string process_name { argv[0] };

        // Argument handling
        boost::program_options::options_description desc("Options");
        desc.add_options()("help,h", "DAQsitter...")("elastic", "Save monitoring data to elasticsearch")("file", "Save monitoring data to ROOT file")("config,c", boost::program_options::value<std::string>(&config), "Configuration file (../data/config.opt)")("sample", boost::program_options::value<float>(&sample_frac), "Fraction of packets to use (0.01)");

        try {
            boost::program_options::variables_map vm;
            boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);

            if (vm.count("help")) {
                std::cout << desc << std::endl;
                return EXIT_SUCCESS;
            }
            if (vm.count("elastic")) {
                elastic = true;
            }
            if (vm.count("file")) {
                file = true;
            }
            boost::program_options::notify(vm);
        } catch (const boost::program_options::error& e) {
            throw std::runtime_error("DAQsitter - po Argument Error");
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("DAQsitter - runtime Argument Error");
        }

        g_config.init(process_name);
        LoggingMultiplexer::init();
        g_elastic.init(process_name);
        log(INFO, "Starting DAQsitter");

        {
            // Main entry point.
            std::shared_ptr<MonitoringHandler> mon_handler { new MonitoringHandler(config, elastic, file, sample_frac) };
            std::shared_ptr<DaqsitterPublisher> bus_publisher { new DaqsitterPublisher(mon_handler) };

            std::unique_ptr<SignalReceiver> signal_receiver { new SignalReceiver };
            signal_receiver->setHandler(mon_handler);
            signal_receiver->runAsync();

            std::unique_ptr<CommandReceiver> cmd_receiver { new CommandReceiver() };
            cmd_receiver->setHandler(mon_handler);
            cmd_receiver->runAsync();

            bus_publisher->runAsync();
            mon_handler->run();

            bus_publisher->join();
            cmd_receiver->join();
            signal_receiver->join();
        }

        log(INFO, "Stopping DAQsitter");
        return exit_code::success;
    }
};

int main(int argc, char* argv[])
{
    Main app {};
    return app.main(argc, argv);
}
