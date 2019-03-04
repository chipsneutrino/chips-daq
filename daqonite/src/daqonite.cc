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
 * $ daqonite -h
 * for a detailed help.
 *
 */

#include <TApplication.h>

#include "daq_handler.h"

int main(int argc, char* argv[]) {
	std::cout << "\nDAQonite - by Josh Tingey MSci, JoshTingeyDAQDemon.Josh" << std::endl;
	std::cout << "DAQonite - Checking hard hats, high-vis, boots and gloves" << std::endl;

	// Default settings
	bool collect_clb_data			= true;
	bool collect_bbb_data 			= false;
	bool use_gui					= true;
	unsigned int num_threads		= 3;
	std::string config_file			= "../data/config.opt";

	// Argument handling
	boost::program_options::options_description desc("Options");
	desc.add_options()
		("help,h", "DAQonite - Default:\n - Shows GUI\n - Always shows monitoring\n - Saves files when in run")
		("noGui",  "Don't show the monitoring GUI")
		("threads,t", boost::program_options::value<unsigned int>(&num_threads),
          		   "Number of threads to use, default = 3")
		("config,c", boost::program_options::value<std::string>(&config_file),
          		   "Configuration file, default = ../data/config.opt");

	try {
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}
		if (vm.count("noGui")) {
			use_gui = false;
		}
		boost::program_options::notify(vm);

	} catch (const boost::program_options::error& e) {
		std::cerr << "DAQonite - Error: " << e.what() << '\n' << desc << std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "DAQonite - Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (num_threads < 1 || num_threads > 8) {
		throw std::runtime_error("DAQonite - Error: Invalid number of threads. Valid = [1,8]!");
	}

	// Need to start a TApplication if we are using the GUI
	DAQHandler * daq_handler;
	if (use_gui) {
		TApplication the_app("app", &argc, argv);

		daq_handler = new DAQHandler(collect_clb_data, collect_bbb_data,
									  use_gui, num_threads, config_file);
	} else {
		daq_handler = new DAQHandler(collect_clb_data, collect_bbb_data,
									  use_gui, num_threads, config_file);	
	}
	delete daq_handler;
}
