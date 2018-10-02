/*
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

#include "daq_handler.h"

#include <TQObject.h>
#include <RQ_OBJECT.h>

#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TGClient.h>

#include "TFile.h"
#include "TTree.h"
#include <TApplication.h>
#include <TSystem.h>

int main(int argc, char* argv[]) {

	std::cout << "################################################################################" << std::endl;
	std::cout << "#                            WELCOME TO DAQONITE!!!                            #" << std::endl;
	std::cout << "#                 by Josh Tingey MSci, JoshTingeyDAQDemon.Josh                 #" << std::endl;
	std::cout << "################################################################################" << std::endl << std::endl;
	std::cout << "DAQonite - Checking hard hats, high-vis vests, steel capped boots and gloves..." 	<< std::endl;

	// Default settings
	bool collect_optical 			= true;
	bool collect_monitoring 		= true;
	unsigned int port_optical 		= (unsigned int)default_opto_port;
	unsigned int port_monitoring 	= (unsigned int)default_moni_port;
	bool saveData					= false;
	std::string filename;
	bool usingGui					= false;

	// Argument handling
	po::options_description desc("Options");
	desc.add_options()("help,h", "Print this help and exit.")
			("gui,g", "Use the GUI.")
			("optPort,o",po::value<unsigned int>(&port_optical) ->implicit_value(default_opto_port) ->value_name("port_optical"),
					"Set optical port, default (56015).")
			("monPort,m", po::value<unsigned int>(&port_monitoring) ->implicit_value(default_moni_port) ->value_name("port_monitoring"),
					"Set monitoring port, default (56017).")
			("savefile,f", po::value<std::string>(&filename)->value_name("filename"),
					"Save the acquired data to a file.")
			("justOpt", "Just mine the optical data.")
			("justMon", "Just mine the monitoring data");

	try {
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}

		if (vm.count("gui")) {
			usingGui = true;
		}

		po::notify(vm);

		if (vm.count("justOpt")) {
			collect_monitoring = false;
		}

		if (vm.count("justMon")) {
			collect_optical = false;
		}

		if (vm.count("savefile")) {
			saveData = true;
			if (!collect_optical || !collect_monitoring) {
				throw std::runtime_error("DAQonite: Error: Need to mine something!");
			}
		}

	} catch (const po::error& e) {
		std::cerr << "DAQonite: Error: " << e.what() << '\n' << desc
				<< std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "DAQonite: Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (usingGui) {
		// Need to create a TApplication before building the GUI in the DAQ_handler
		TApplication theApp("app", &argc, argv);
		DAQ_handler * daq_handler = new DAQ_handler(collect_optical, collect_monitoring, false, false,
													port_optical, port_monitoring, (unsigned int)999,
													saveData, filename, usingGui);
		daq_handler->startRun(1,2);
	} else {
		// Just Load a DAQ_handler
		TApplication theApp("app", &argc, argv);
		DAQ_handler * daq_handler = new DAQ_handler(collect_optical, collect_monitoring, false, false,
													port_optical, port_monitoring, (unsigned int)999,
													saveData, filename, usingGui);
		daq_handler->startRun(1,2);
	}


	std::cout << "DAQonite - Done for the day!" << std::endl;
}
