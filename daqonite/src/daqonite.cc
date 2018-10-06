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

	std::cout << "#################################################################" << std::endl;
	std::cout << "#                    WELCOME TO DAQONITE!!!                     #" << std::endl;
	std::cout << "#         by Josh Tingey MSci, JoshTingeyDAQDemon.Josh          #" << std::endl;
	std::cout << "# Checking hard hats, high-vis, gloves and steel capped boot... #" << std::endl;
	std::cout << "#################################################################" << std::endl;

	// Default settings
	bool collect_clb_optical 		= true;
	bool collect_clb_monitoring 	= true;
	bool collect_bbb_optical 		= false;
	bool collect_bbb_monitoring 	= false;

	bool useMonitoringGui			= false;
	bool saveToFile					= false;

	int runType						= -1;

	// Argument handling
	po::options_description desc("Options");
	desc.add_options()("help,h", "Print this help and exit.")
			("gui,g", "Use the GUI.")
			("save,s", "Save data to file.")
			("runType,t", po::value<int>(&runType)->value_name("runType"),
				"Specify the run type")
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
			useMonitoringGui = true;
		}
		if (vm.count("save")) {
			saveToFile = true;
		}

		po::notify(vm);

		if (vm.count("justOpt")) {
			collect_clb_optical = false;
			collect_bbb_optical = false;
		}
		if (vm.count("justMon")) {
			collect_clb_monitoring = false;
			collect_bbb_monitoring = false;
		}

	} catch (const po::error& e) {
		std::cerr << "DAQonite: Error: " << e.what() << '\n' << desc << std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "DAQonite: Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (runType < 0 || runType >= NUMRUNTYPES) {
		throw std::runtime_error("DAQonite: Error: Need to specify a valid run type!");
	}

	// Need to start a TApplication if we are using the GUI
	DAQ_handler * daq_handler;
	if (useMonitoringGui) {
		TApplication theApp("app", &argc, argv);

		daq_handler = new DAQ_handler(collect_clb_optical, collect_clb_monitoring, 
									  collect_bbb_optical, collect_bbb_monitoring,
									  useMonitoringGui, saveToFile, runType);
	} else {
		daq_handler = new DAQ_handler(collect_clb_optical, collect_clb_monitoring, 
									  collect_bbb_optical, collect_bbb_monitoring,
									  useMonitoringGui, saveToFile, runType);	
	}
	delete daq_handler;
}
