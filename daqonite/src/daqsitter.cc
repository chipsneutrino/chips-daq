/**
 * Program name: DAQsitter, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "elastic_interface.h"
#include "monitoring_server.h"

int main(int argc, char *argv[])
{
    // Initialise the elasticsearch interface
    g_elastic.init("daqsitter", true, false); // We want log message to be printed to stdout

    // Default settings
    std::string config_file = "../data/config.opt";
    bool save_elastic = false;
    bool save_file = false;
    bool use_gui = false;
    float clb_frac = 1.0;
    float bbb_frac = 1.0;

    // Argument handling
    boost::program_options::options_description desc("Options");
    desc.add_options()("help,h", "DAQsitter...")("elastic", "Save monitoring data to elasticsearch")("file", "Save monitoring data to ROOT file")("gui", "Show the old monitoring GUI")("config,c", boost::program_options::value<std::string>(&config_file), "Configuration file (../data/config.opt)")("clb_frac", boost::program_options::value<float>(&clb_frac), "Fraction of CLB packets to use (1.0)")("bbb_frac", boost::program_options::value<float>(&bbb_frac), "Fraction of BBB packets to use (1.0)");

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
            save_elastic = true;
        }
        if (vm.count("file"))
        {
            save_file = true;
        }
        if (vm.count("gui"))
        {
            use_gui = true;
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

    // Log the setup to elasticsearch
    std::string setup = "MonitoringServer Start (" + config_file + ") (";
    if (save_elastic)
    {
        setup += "elastic";
    }
    if (save_file)
    {
        setup += ",file";
    }
    if (use_gui)
    {
        setup += ",gui";
    }
    setup += ") (" + std::to_string(clb_frac) + "," + std::to_string(bbb_frac) + ")";
    g_elastic.log(INFO, setup);

    // Start the MonitoringServer
    MonitoringServer server(config_file, save_elastic, save_file, use_gui, clb_frac, bbb_frac);

    g_elastic.log(INFO, "MonitoringServer Stopped");

    return 0;
}
