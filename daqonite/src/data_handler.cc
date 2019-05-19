/**
 * DataHandler - Handler class for combining the data and saving to file
 */

#include "data_handler.h"

DataHandler::DataHandler(bool collect_clb_data, bool collect_bbb_data) :
								   fCLB_events(),
								   fMerge_sorter(),
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

DataHandler::~DataHandler() {
	if (fOutput_file!=NULL) {
		fOutput_file->Close();
		fOutput_file = NULL;
	}
}

void DataHandler::startRun(int run_type) {
    // Set the fRun_type, fRun_num and fFile_name run variables
    fRun_type = run_type;
    getRunNumAndName();

	std::string name = fFile_name.Data();
	g_elastic.log(WARNING, "Start mining into container " + name);

    // Open the ROOT output file and check it exists
    fOutput_file = new TFile(fFile_name, "RECREATE");
    if (!fOutput_file) { throw std::runtime_error("daqonite - Error: Opening output file!"); }

    // Create the TTree's, check they exist and add the branches
    if (fCollect_clb_data) {
        fOpt_tree_clb = new TTree("CLBOpt_tree", "CLBOpt_tree");
        if (!fOpt_tree_clb) { throw std::runtime_error("daqonite - Error: CLBOpt_tree!"); }
        addOptCLBBranches();
        fMon_tree_clb = new TTree("CLBMon_tree", "CLBMon_tree");
        if (!fMon_tree_clb) { throw std::runtime_error("daqonite - Error: CLBMon_tree!"); }
        addMonCLBBranches();
    }

    if (fCollect_bbb_data) {
        fOpt_tree_bbb = new TTree("BBBOpt_tree", "BBBOpt_tree");
        if (!fOpt_tree_bbb) { throw std::runtime_error("daqonite - Error: BBBOpt_tree!"); }
        fMon_tree_bbb = new TTree("BBBMon_tree", "BBBMon_tree");
        if (!fMon_tree_bbb) { throw std::runtime_error("daqonite - Error: BBBMon_tree!"); }        
    }
}

void DataHandler::stopRun() {
	{
		// Calculate complete timestamps & make sure sequence is sorted
		for (auto& key_value : fCLB_events) {
			CLBEventQueue& queue = key_value.second;

			// Calculate sort keys
			for (CLBEvent& event : queue) {
				// TODO: vectorize?
				event.SortKey = event.Timestamp_s + 1e-9 * event.Timestamp_ns;
			}

			// TODO: report disorder measure
			insertSort(queue);
		}

		// Merge-sort CLB events.
		CLBEventQueue merged_events{};
		fMerge_sorter.merge(fCLB_events, merged_events);
		fCLB_events.clear();

		// Write sorted events out to TTree.
		for (const CLBEvent& event : merged_events) {
			fPomId_opt_clb = event.PomId;
			fChannel_opt_clb = event.Channel;
			fTimestamp_s_opt_clb = event.Timestamp_s;
			fTimestamp_ns_opt_clb = event.Timestamp_ns;
			fTot_opt_clb = event.Tot;
			fOpt_tree_clb->Fill();
		}
	}

    // Write the TTree's to the output file
    if (fCollect_clb_data && fOpt_tree_clb != NULL && fMon_tree_clb != NULL) {
        fOpt_tree_clb->Write();
        fMon_tree_clb->Write();
    }

    if (fCollect_bbb_data && fOpt_tree_bbb != NULL && fMon_tree_bbb != NULL) {
        fOpt_tree_bbb->Write();
        fMon_tree_bbb->Write();
    }    

	std::string name = fFile_name.Data();
	g_elastic.log(WARNING, "Stop mining into container " + name);

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

void DataHandler::fillMonCLBTree() {
    // Need mutex lock/unlock
    fMon_tree_clb->Fill();
}

void DataHandler::fillOptBBBTree() {
    // Empty
}

void DataHandler::fillMonBBBTree() {
    // Empty
}

void DataHandler::getRunNumAndName() {
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
		} else { throw std::runtime_error("daqonite - Error: Unable to create ../data/runNumbers.dat!"); }
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
		} else { throw std::runtime_error("daqonite - Error: Unable to update runNumbers.dat!"); }
	}  

	fFile_name = "../data/type";
    fFile_name += fRun_type;
    fFile_name += "_run";
	fFile_name += fRun_num;
	fFile_name += ".root";   
}

void DataHandler::addOptCLBBranches() {
	fOpt_tree_clb->Branch("PomId", &fPomId_opt_clb, "fPomId_opt_clb/i");
	fOpt_tree_clb->Branch("Channel", &fChannel_opt_clb, "fChannel_opt_clb/b");
	fOpt_tree_clb->Branch("TimeStamp_s", &fTimestamp_s_opt_clb, "fTimestamp_s_opt_clb/i");
	fOpt_tree_clb->Branch("TimeStamp_ns", &fTimestamp_ns_opt_clb, "fTimestamp_ns_opt_clb/i");
	fOpt_tree_clb->Branch("ToT", &fTot_opt_clb, "fTot_opt_clb/B");
}

void DataHandler::addMonCLBBranches() {
	fMon_tree_clb->Branch("PomId", &fPomId_mon_clb, "fPomId_mon_clb/i");
	fMon_tree_clb->Branch("TimeStamp_s", &fTimestamp_s_mon_clb, "fTimestamp_s_mon_clb/i");
	fMon_tree_clb->Branch("Pad", &fPad_mon_clb, "fPad_mon_clb/i");
	fMon_tree_clb->Branch("Valid", &fValid_mon_clb, "fValid_mon_clb/i");
	fMon_tree_clb->Branch("Temperate", &fTemperate_mon_clb, "fTemperate_mon_clb/s");
	fMon_tree_clb->Branch("Humidity", &fHumidity_mon_clb, "fHumidity_mon_clb/s");
    fMon_tree_clb->Branch("Hits",&fHits_mon_clb,"fHits_mon_clb[30]/i");	
}

void DataHandler::addOptBBBBranches() {
    // Empty
}

void DataHandler::addMonBBBBranches() {
    // Empty
}

std::size_t DataHandler::insertSort(CLBEventQueue& queue) noexcept {
	// Just your conventional O(n^2) insert-sort implementation.
	// Here utilized because insert-sort is actually O(n+k*n) for k-sorted sequences.
	// Since event queue should already be sorted, insert-sort will frequently only scan it in O(n).

	std::size_t n_swaps{0};
    for (std::size_t i = 1; i < queue.size(); ++i) {
        for (std::size_t j = i; j > 0 && queue[j - 1] > queue[j]; --j) {
			std::swap(queue[j], queue[j - 1]);
			++n_swaps;
        }
    }

	return n_swaps;
}
