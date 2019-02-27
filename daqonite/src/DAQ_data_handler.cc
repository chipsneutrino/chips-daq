/**
 * DAQ_data_handler - Handler class for combining the data and saving to file
 */

#include "DAQ_data_handler.h"

DAQ_data_handler::DAQ_data_handler(bool collect_clb_data, bool collect_bbb_data) :
                                   fCollect_clb_data(collect_clb_data),
                                   fCollect_bbb_data(collect_bbb_data) {

	fRun_type = -1;
	fRun_num = -1;
    fFile_name = "";

	fOutput_file = NULL;
	fOpt_tree_clb = NULL;
	fMon_tree_clb = NULL;
	fOpt_tree_bbb = NULL;
	fMon_tree_bbb = NULL;
}

DAQ_data_handler::~DAQ_data_handler() {
	if (fOutput_file!=NULL) {
		fOutput_file->Close();
		fOutput_file = NULL;
	}
}

void DAQ_data_handler::startRun(int run_type) {
    // Set the fRun_type, fRun_num and fFile_name run variables
    fRun_type = run_type;
    getRunNumAndName();

    std::cout << "\nDAQonite - Start mining into container: " << fFile_name << std::endl;

    // Open the ROOT output file and check it exists
    fOutput_file = new TFile(fFile_name, "RECREATE");
    if (!fOutput_file) { throw std::runtime_error("DAQonite - Error: Opening output file!"); }

    // Create the TTree's, check they exist and add the branches
    if (fCollect_clb_data) {
        fOpt_tree_clb = new TTree("CLBOpt_tree", "CLBOpt_tree");
        if (!fOpt_tree_clb) { throw std::runtime_error("DAQonite - Error: CLBOpt_tree!"); }
        addOptCLBBranches();
        fMon_tree_clb = new TTree("CLBMon_tree", "CLBMon_tree");
        if (!fMon_tree_clb) { throw std::runtime_error("DAQonite - Error: CLBMon_tree!"); }
        addMonCLBBranches();
    }

    if (fCollect_bbb_data) {
        fOpt_tree_bbb = new TTree("BBBOpt_tree", "BBBOpt_tree");
        if (!fOpt_tree_bbb) { throw std::runtime_error("DAQonite - Error: BBBOpt_tree!"); }
        fMon_tree_bbb = new TTree("BBBMon_tree", "BBBMon_tree");
        if (!fMon_tree_bbb) { throw std::runtime_error("DAQonite - Error: BBBMon_tree!"); }        
    }
}

void DAQ_data_handler::stopRun() {
    // Write the TTree's to the output file
    if (fCollect_clb_data && fOpt_tree_clb != NULL && fMon_tree_clb != NULL) {
        fOpt_tree_clb->Write();
        fMon_tree_clb->Write();
    }

    if (fCollect_bbb_data && fOpt_tree_bbb != NULL && fMon_tree_bbb != NULL) {
        fOpt_tree_bbb->Write();
        fMon_tree_bbb->Write();
    }    

    std::cout << "\nDAQonite - Stop mining into the container: " << fFile_name << std::endl;

    // Close the ROOT output file
    if (fOutput_file != NULL) { fOutput_file->Close(); }

    // Set all the pointer's to NULL and reset the run variables
	fOutput_file = NULL;
	fOpt_tree_clb = NULL;
	fMon_tree_clb = NULL;
    fOpt_tree_bbb = NULL;
	fMon_tree_bbb = NULL;    

    fRun_type = -1;
	fRun_num = -1;
    fFile_name = "";
}

void DAQ_data_handler::fillOptCLBTree() {
    // Need mutex lock/unlock
    fOpt_tree_clb->Fill();
}

void DAQ_data_handler::fillMonCLBTree() {
    // Need mutex lock/unlock
    fMon_tree_clb->Fill();
}

void DAQ_data_handler::fillOptBBBTree() {
    // Empty
}

void DAQ_data_handler::fillMonBBBTree() {
    // Empty
}

void DAQ_data_handler::getRunNumAndName() {
	// 4 fRun_type -> 1) Data_normal, 2) Calibration, 3) Test_normal, 4) test_daq

	int fRun_num = 1;
	int runNums[NUMRUNTYPES];
	std::ifstream runNumFile("../data/runNumbers.dat");	
	if(runNumFile.fail()) {
		runNumFile.close();	
		// The file does not yet exist so lets create it
		std::ofstream newFile("../data/runNumbers.dat");
  		if (newFile.is_open()) {
			for (int i=0; i<NUMRUNTYPES; i++) {	
				if (fRun_type == i) { 
					newFile << 2 << "\n"; 
				} else { newFile << 1 << "\n"; }
			}
			newFile.close();
		} else { throw std::runtime_error("DAQonite - Error: Unable to create ../data/runNumbers.dat!"); }
	} else {
		// The file exists so read from it
		for (int i=0; i<NUMRUNTYPES; i++) { 
			runNumFile >> runNums[i]; 
			if (runNums[i] < 1) { runNums[i] = 1; }
			if (fRun_type == i) { fRun_num = runNums[i]; }
		}
		runNumFile.close();	

		// Now create the updated file
		std::ofstream updateFile("../data/runNumbers.dat");
  		if (updateFile.is_open()) {
			for (int i=0; i<NUMRUNTYPES; i++) {	
				if (fRun_type == i) { 
					updateFile << runNums[i] + 1 << "\n"; 
				} else { updateFile << 1 << "\n"; }
			}
			updateFile.close();
		} else { throw std::runtime_error("DAQonite - Error: Unable to update runNumbers.dat!"); }
	}  

	fFile_name = "../data/type";
    fFile_name += fRun_type;
    fFile_name += "_run";
	fFile_name += fRun_num;
	fFile_name += ".root";   
}

void DAQ_data_handler::addOptCLBBranches() {
	fOpt_tree_clb->Branch("PomId", &fPomId_opt_clb, "fPomId_opt_clb/i");
	fOpt_tree_clb->Branch("Channel", &fChannel_opt_clb, "fChannel_opt_clb/b");
	fOpt_tree_clb->Branch("TimeStamp_s", &fTimestamp_s_opt_clb, "fTimestamp_s_opt_clb/i");
	fOpt_tree_clb->Branch("TimeStamp_ns", &fTimestamp_ns_opt_clb, "fTimestamp_ns_opt_clb/i");
	fOpt_tree_clb->Branch("ToT", &fTot_opt_clb, "fTot_opt_clb/B");
}

void DAQ_data_handler::addMonCLBBranches() {
	fMon_tree_clb->Branch("PomId", &fPomId_mon_clb, "fPomId_mon_clb/i");
	fMon_tree_clb->Branch("TimeStamp_s", &fTimestamp_s_mon_clb, "fTimestamp_s_mon_clb/i");
	fMon_tree_clb->Branch("Pad", &fPad_mon_clb, "fPad_mon_clb/i");
	fMon_tree_clb->Branch("Valid", &fValid_mon_clb, "fValid_mon_clb/i");
	fMon_tree_clb->Branch("Temperate", &fTemperate_mon_clb, "fTemperate_mon_clb/s");
	fMon_tree_clb->Branch("Humidity", &fHumidity_mon_clb, "fHumidity_mon_clb/s");
    fMon_tree_clb->Branch("Hits",&fHits_mon_clb,"fHits_mon_clb[30]/i");	
}

void DAQ_data_handler::addOptBBBBranches() {
    // Empty
}

void DAQ_data_handler::addMonBBBBranches() {
    // Empty
}
