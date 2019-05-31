/**
 * Program name: DAQsitter, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include <util/elastic_interface.h>
#include "monitoring_server.h"

int main(int argc, char *argv[])
{
    // Default settings
    std::string config = "../data/config.opt";
    bool elastic = false;
    bool file = false;
    float sample_frac = 0.01;

    bool print_logs = true;
    bool print_debug = false;
    int index_threads = 100;

    // Argument handling
    boost::program_options::options_description desc("Options");
    desc.add_options()("help,h", "DAQsitter...")("elastic", "Save monitoring data to elasticsearch")("file", "Save monitoring data to ROOT file")("config,c", boost::program_options::value<std::string>(&config), "Configuration file (../data/config.opt)")("sample", boost::program_options::value<float>(&sample_frac), "Fraction of packets to use (0.01)")("logs", boost::program_options::value<bool>(&print_logs), "Print logs to stdout (true)")("debug", boost::program_options::value<bool>(&print_debug), "Print ElasticInterface debug messages (false)")("threads", boost::program_options::value<int>(&index_threads), "Number of ElasticInterface indexing threads (100)");

    try
    {
        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("elastic"))
        {
            elastic = true;
        }
        if (vm.count("file"))
        {
            file = true;
        }
        boost::program_options::notify(vm);
    }
    catch (const boost::program_options::error &e)
    {
        throw std::runtime_error("DAQsitter - po Argument Error");
    }
    catch (const std::runtime_error &e)
    {
        throw std::runtime_error("DAQsitter - runtime Argument Error");
    }

    g_elastic.init(print_logs, print_debug, index_threads); // Initialise the ElasticInterface

    g_elastic.log(INFO, "MonitoringServer: Start");

    // Start the MonitoringServer
    MonitoringServer server(config, elastic, file, sample_frac);

    g_elastic.log(INFO, "MonitoringServer: Stopped");

    return 0;
}
