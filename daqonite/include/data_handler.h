/**
 * DataHandler - Handler class for combining the data and saving to file
 * 
 * This class deals with combing the data stream from both the CLB and BBB,
 * sorting and then saving to file
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef DATA_HANDLER_H_
#define DATA_HANDLER_H_

#include <iostream>
#include <stdexcept>
#include <fstream>

#include "TFile.h"
#include "TTree.h"

#include "elastic_interface.h"
#include "clb_event.h"
#include "merge_sorter.h"

#define NUMRUNTYPES 4

class DataHandler {
	public:
		/// Create a DataHandler
		DataHandler(bool collect_clb_data, bool collect_bbb_data);

		/// Destroy a DataHandler
		~DataHandler();

        /**
		 * Start a data taking run
		 * Sets the run variables, opens the output file, and adds TTrees and branches
		 */
        void startRun(int run_type);

        /**
		 * Stop a data taking run
		 * Writes the TTrees to file, close the output file, and cleanup/reset variables
		 */
        void stopRun();

        /// Fill the optical CLB TTree
        void fillOptCLBTree();

        /// Fill the monitoring CLB TTree
        void fillMonCLBTree();

        /// Fill the optical BBB TTree (TODO)
        void fillOptBBBTree();

        /// Fill the monitoring BBB TTree (TODO)
        void fillMonBBBTree();

        int getRunType() {
            return fRun_type;
        }

        int getRunNum() {
            return fRun_num;
        }

        TString getOutputName() {
            return fFile_name;
        }

        CLBEventMultiQueue fCLB_events;

        // fMon_tree_clb Variables
		uint32_t 	fPomId_mon_clb;		    ///< Mon CLB: Header POM ID (4 bytes)
		uint32_t 	fTimestamp_s_mon_clb;   ///< Mon CLB: Header timestamp (4 bytes)
		uint32_t 	fPad_mon_clb;   		///< Mon CLB: Header Pad (4 bytes)
		uint32_t 	fValid_mon_clb; 		///< Mon CLB: Header Valid (4 bytes)
		uint16_t 	fTemperate_mon_clb; 	///< Mon CLB: Temperature data (2 bytes)
		uint16_t 	fHumidity_mon_clb;	    ///< Mon CLB: Humidity data (2 bytes)
		uint32_t 	fHits_mon_clb[30];  	///< Mon CLB: Channel Hits (4 bytes)

        // fOpt_tree_bbb Variables (TODO)

        // fMon_tree_bbb Variables (TODO)

	private:
        // fOpt_tree_clb Variables
		uint32_t  	fPomId_opt_clb;			///< Opt CLB: Header POM ID (4 bytes)
		uint8_t 	fChannel_opt_clb;		///< Opt CLB: Hit Channel ID (1 bytes)
		uint32_t 	fTimestamp_s_opt_clb;	///< Opt CLB: Header timestamp (4 bytes)
		uint32_t 	fTimestamp_ns_opt_clb;	///< Opt CLB: Hit timestamp (4 bytes)
		int8_t 		fTot_opt_clb;			///< Opt CLB: Hit TOT value (1 bytes)

        /**
		 * Finds the run number of the given run type from file
		 * and the updates the file having incremented the run number
         * Then determines the output file name
		 */
        void getRunNumAndName();

        /// Add the branches to the optical CLB TTree
        void addOptCLBBranches();

        /// Add the branches to the monitoring CLB TTree
        void addMonCLBBranches();

        /// Add the branches to the optical BBB TTree (TODO)
        void addOptBBBBranches();

        /// Add the branches to the monitoring BBB TTree (TODO)
        void addMonBBBBranches();

        // Settings
        bool        fCollect_clb_data;      ///< Are we going to collect clb data?
        bool        fCollect_bbb_data;      ///< Are we going to collect bbb data?

        // Run Variables
		int 		fRun_type;				///< Type of run (data, test, etc...)
		int         fRun_num;               ///< Run number found from "../data/runNumbers.dat"
        TString 	fFile_name;				///< Output file name

        // ROOT File and Tree's
		TFile* 		fOutput_file;			///< ROOT output file
		TTree* 	    fOpt_tree_clb;			///< ROOT CLB optical output TTree
		TTree* 		fMon_tree_clb;			///< ROOT CLB monitoring output TTree
		TTree* 		fOpt_tree_bbb;			///< ROOT BBB optical output TTree
		TTree* 		fMon_tree_bbb;			///< ROOT BBB monitoring output TTree
};

#endif
