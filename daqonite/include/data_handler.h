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
#include <memory>
#include <thread>
#include <chrono>

#include "TFile.h"
#include "TTree.h"
#include "boost/lockfree/queue.hpp"

#include "elastic_interface.h"
#include "clb_event.h"
#include "merge_sorter.h"
#include "batch_scheduler.h"

#define NUMRUNTYPES 4

class DataHandler {
	public:
		/// Create a DataHandler
		DataHandler();

		/// Destroy a DataHandler
		virtual ~DataHandler() = default;

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

        CLBEventMultiQueue* findCLBOpticalQueue(double timestamp);
        void updateLastApproxTimestamp(std::uint32_t timestamp);

	private:
        std::shared_ptr<std::thread> output_thread_;        ///< Thread for merge-sorting and saving
        std::shared_ptr<std::thread> scheduling_thread_;    ///< Thread for scheduling and closing batches
        void joinThreads();                                 ///< Synchronously terminate all threads
        
        std::atomic_bool output_running_;
        std::atomic_bool scheduling_running_;
		int 		run_type_;				                ///< Type of run (data, test, etc...)
		int         run_num_;                               ///< Run number found from "../data/runNumbers.dat"
        std::string	file_name_;				                ///< Output file name

        using Clock = std::chrono::steady_clock;
        using BatchQueue = boost::lockfree::queue<Batch, boost::lockfree::capacity<16>>;
        BatchQueue waiting_batches_;

        void outputThread();                              ///< Main entry point of the output thread

        std::atomic_uint32_t last_approx_timestamp_;
        std::shared_ptr<BatchScheduler> batch_scheduler_;
        BatchSchedule current_schedule_;

        void closeOldBatches(BatchSchedule& schedule);
        void closeBatch(Batch&& batch);
        void schedulingThread();
        
        static std::size_t insertSort(CLBEventQueue& queue) noexcept;
        
        /**
		 * Finds the run number of the given run type from file
		 * and the updates the file having incremented the run number
         * Then determines the output file name
		 */
        void getRunNumAndName();
};

#endif
