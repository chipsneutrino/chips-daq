/**
 * Program name: DAQsitter, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "elastic_interface.h"
#include "monitoring_server.h"

int main(int argc, char* argv[])
{
    // Default settings
    std::string config_file = "../data/config.opt";
    bool save_elastic = false;
    bool save_file = false;
    bool use_gui = false;
    float clb_frac = 1.0;
    float bbb_frac = 1.0;

    bool print_logs = true;
    bool print_debug = false;
    int  index_threads = 100;

    // Argument handling
    boost::program_options::options_description desc("Options");
    desc.add_options()("help,h", "DAQsitter...")
    ("elastic", "Save monitoring data to elasticsearch")
    ("file", "Save monitoring data to ROOT file")
    ("gui", "Show the old monitoring GUI")
    ("config,c", boost::program_options::value<std::string>(&config_file), "Configuration file (../data/config.opt)")
    ("clb_frac", boost::program_options::value<float>(&clb_frac), "Fraction of CLB packets to use (1.0)")
    ("bbb_frac", boost::program_options::value<float>(&bbb_frac), "Fraction of BBB packets to use (1.0)")
    ("logs", boost::program_options::value<bool>(&print_logs), "Print logs to stdout (true)")
    ("debug", boost::program_options::value<bool>(&print_debug), "Print ElasticInterface debug messages (false)")
    ("threads", boost::program_options::value<int>(&index_threads), "Number of ElasticInterface indexing threads (100)");

    try {
        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("elastic")) {
            save_elastic = true;
        }
        if (vm.count("file")) {
            save_file = true;
        }
        if (vm.count("gui")) {
            use_gui = true;
        }
        boost::program_options::notify(vm);
    } catch (const boost::program_options::error& e) {
        throw std::runtime_error("DAQsitter - po Argument Error");
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("DAQsitter - runtime Argument Error");
    }

    g_elastic.init(print_logs, print_debug, index_threads); // Initialise the ElasticInterface

    // Log the setup to elasticsearch
    std::string setup{};
    if (save_elastic)
        setup += "elastic";
    if (save_file)
        setup += ",file";
    if (use_gui)
        setup += ",gui";
    g_elastic.log(INFO, "MonitoringServer Start ({}) ({}) ({},{})", config_file, setup, clb_frac, bbb_frac);

    // Start the MonitoringServer
    MonitoringServer server(config_file, save_elastic, save_file, use_gui, clb_frac, bbb_frac);

    g_elastic.log(INFO, "MonitoringServer Stopped");

    return 0;
}
