/**
 * Program name: DAQonite, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "daq_handler.h"

int main(int argc, char *argv[])
{
    // Initialise the elasticsearch interface
    g_elastic.init("daqonite", true, false); // We want log message to be printed to stdout

    // Default settings
    bool collect_clb_data = true;
    bool collect_bbb_data = false;
    unsigned int num_threads = 3;

    // Argument handling
    boost::program_options::options_description desc("Options");
    desc.add_options()("help,h", "DAQonite")("threads,t", boost::program_options::value<unsigned int>(&num_threads),
                                             "Number of threads to use, default = 3");

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
        throw std::runtime_error("daqonite - po Argument Error");
    }
    catch (const std::runtime_error &e)
    {
        throw std::runtime_error("daqonite - runtime Argument Error");
    }

    if (num_threads < 1 || num_threads > 8)
    {
        throw std::runtime_error("daqonite - Error: Invalid number of threads. Valid = [1,8]!");
    }

    // Log the setup to elasticsearch
    g_elastic.log(INFO, "Checking hard hats, high-vis, boots and gloves!");
    std::string setup = "DAQ Handler Start (";
    if (collect_clb_data)
    {
        setup += "clb";
    }
    if (collect_bbb_data)
    {
        setup += ",bbb";
    }
    setup += ") (" + std::to_string(num_threads) + ")";
    g_elastic.log(INFO, setup);

    // Need to start a TApplication if we are using the GUI
    DAQHandler daq_handler(collect_clb_data, collect_bbb_data, num_threads);

    g_elastic.log(INFO, "Done for the day!");

    return 0;
}
