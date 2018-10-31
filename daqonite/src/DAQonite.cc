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
	bool collect_clb_optical 		= true;
	bool collect_clb_monitoring 	= true;
	bool collect_bbb_optical 		= false;
	bool collect_bbb_monitoring 	= false;
	bool useMonitoringGui			= true;	
	unsigned int numThreads			= 3;

	// Argument handling
	boost::program_options::options_description desc("Options");
	desc.add_options()
		("help,h", "DAQonite - Default:\n - Shows GUI\n - Always shows monitoring\n - Saves files when in run")
		("noGui",  "Don't show the monitoring GUI")
		("noOpt",  "Don't mine the optical data")
		("noMon",  "Don't mine the monitoring data")
		("threads,t", boost::program_options::value<unsigned int>(&numThreads),
          		   "Number of threads to use, default = 3");

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
		if (vm.count("noOpt")) {
			collect_clb_optical = false;
			collect_bbb_optical = false;
		}
		if (vm.count("noMon")) {
			collect_clb_monitoring = false;
			collect_bbb_monitoring = false;
		}
		boost::program_options::notify(vm);

	} catch (const boost::program_options::error& e) {
		std::cerr << "DAQonite - Error: " << e.what() << '\n' << desc << std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "DAQonite - Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (numThreads < 1 || numThreads > 4) {
		throw std::runtime_error("DAQonite - Error: Invalid number of threads. Valid = [1,4]!");
	}

	// Need to start a TApplication if we are using the GUI
	DAQ_handler * daq_handler;
	if (useMonitoringGui) {
		TApplication theApp("app", &argc, argv);

		daq_handler = new DAQ_handler(collect_clb_optical, collect_clb_monitoring, 
									  collect_bbb_optical, collect_bbb_monitoring,
									  useMonitoringGui, numThreads);
	} else {
		daq_handler = new DAQ_handler(collect_clb_optical, collect_clb_monitoring, 
									  collect_bbb_optical, collect_bbb_monitoring,
									  useMonitoringGui, numThreads);	
	}
	delete daq_handler;
}
