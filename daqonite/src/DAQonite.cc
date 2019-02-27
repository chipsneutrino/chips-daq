/**
 * Program name: DAQonite, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * This program is a modification of the CLBSwissKnife...
 * Original Author: Carmelo Pellegrino
 * E-mail: carmelo.pellegrino@bo.infn.it
 * Date:   27 October 2014

 * Modified Version Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 * Date:   18 September 2018
 *
 * Use
 * $ DAQonite -h
 * for a detailed help.
 *
 */

#include <TApplication.h>

#include "DAQ_handler.h"

int main(int argc, char* argv[]) {
	std::cout << "\nDAQonite - by Josh Tingey MSci, JoshTingeyDAQDemon.Josh" << std::endl;
	std::cout << "DAQonite - Checking hard hats, high-vis, boots and gloves" << std::endl;

	// Default settings
	bool collect_clb_data			= true;
	bool collect_bbb_data 			= false;
	bool useMonitoringGui			= true;
	unsigned int numThreads			= 3;
	std::string configFile			= "../data/config.opt";

	// Argument handling
	boost::program_options::options_description desc("Options");
	desc.add_options()
		("help,h", "DAQonite - Default:\n - Shows GUI\n - Always shows monitoring\n - Saves files when in run")
		("noGui",  "Don't show the monitoring GUI")
		("threads,t", boost::program_options::value<unsigned int>(&numThreads),
          		   "Number of threads to use, default = 3")
		("config,c", boost::program_options::value<std::string>(&configFile),
          		   "Configuration file, default = ../data/config.opt");

	try {
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}
		if (vm.count("noGui")) {
			useMonitoringGui = false;
		}
		boost::program_options::notify(vm);

	} catch (const boost::program_options::error& e) {
		std::cerr << "DAQonite - Error: " << e.what() << '\n' << desc << std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "DAQonite - Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (numThreads < 1 || numThreads > 8) {
		throw std::runtime_error("DAQonite - Error: Invalid number of threads. Valid = [1,8]!");
	}

	// Need to start a TApplication if we are using the GUI
	DAQ_handler * daq_handler;
	if (useMonitoringGui) {
		TApplication theApp("app", &argc, argv);

		daq_handler = new DAQ_handler(collect_clb_data, collect_bbb_data,
									  useMonitoringGui, numThreads, configFile);
	} else {
		daq_handler = new DAQ_handler(collect_clb_data, collect_bbb_data,
									  useMonitoringGui, numThreads, configFile);	
	}
	delete daq_handler;
}
