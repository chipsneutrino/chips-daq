/**
 * DataHandler - Handler class for combining the data and saving to file
 */

#include <functional>

#include "run_file.h"
#include "data_handler.h"

DataHandler::DataHandler()
	:output_thread_{},
	 scheduling_thread_{},
	 input_running_{false},
	 output_running_{false},
	 scheduling_running_{false},
	 run_type_{-1},
	 run_num_{-1},
	 file_name_{},
	 waiting_batches_{},
	 waiting_batches_mtx_{},
	 waiting_batches_cv_{},
	 batch_scheduler_{},
	 current_schedule_{}
{
	run_type_ = -1;
	run_num_ = -1;
    file_name_ = "";
}

void DataHandler::startRun(int run_type) {
    // Set the fRun_type, fRun_num and fFile_name run variables
    run_type_ = run_type;
    getRunNumAndName();

	// Prepare queues and schedule.
	{
		std::lock_guard<std::mutex> l{waiting_batches_mtx_};
		waiting_batches_.clear();
	}

	// TODO: determine this from run_type
	batch_scheduler_ = std::make_shared<InfiniteScheduler>();

	current_schedule_.clear();
	batch_scheduler_->updateSchedule(current_schedule_);

	// Start output thread.
	g_elastic.log(WARNING, "Start mining into container " + file_name_);
	input_running_ = output_running_ = scheduling_running_ = true;
	output_thread_ = std::make_shared<std::thread>(std::bind(&DataHandler::outputThread, this));
	scheduling_thread_ = std::make_shared<std::thread>(std::bind(&DataHandler::schedulingThread, this));
}

void DataHandler::stopRun() {
	input_running_ = false;
	g_elastic.log(WARNING, "Signal stop mining into container " + file_name_);

	// Wait for the output thread to end
	joinThreads();
	g_elastic.log(WARNING, "Stop mining into container " + file_name_);

    // Reset the run variables
    run_type_ = -1;
	run_num_ = -1;
    file_name_ = "";
}

void DataHandler::getRunNumAndName() {
	// 4 fRun_type -> 1) Data_normal, 2) Calibration, 3) Test_normal, 4) test_daq

	int runNums[NUMRUNTYPES];
	std::ifstream runNumFile("../data/runNumbers.dat");	
	if (runNumFile.fail()) {
		runNumFile.close();	
		// The file does not yet exist so lets create it
		std::ofstream newFile("../data/runNumbers.dat");
  		if (newFile.is_open()) {
			for (int i=0; i<NUMRUNTYPES; i++) {	
				if (run_type_ == i) { 
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
			if (run_type_ == i) { run_num_ = runNums[i]; }
		}
		runNumFile.close();	

		// Now create the updated file
		std::ofstream updateFile("../data/runNumbers.dat");
  		if (updateFile.is_open()) {
			for (int i=0; i<NUMRUNTYPES; i++) {	
				if (run_type_ == i) { 
					updateFile << runNums[i] + 1 << "\n"; 
				} else { updateFile << 1 << "\n"; }
			}
			updateFile.close();
		} else { throw std::runtime_error("daqonite - Error: Unable to update runNumbers.dat!"); }
	}  

	file_name_ = "../data/type";
    file_name_ += std::to_string(run_type_);
    file_name_ += "_run";
	file_name_ += std::to_string(run_num_);
	file_name_ += ".root";   
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

std::shared_ptr<CLBEventMultiQueue> DataHandler::findCLBOpticalQueue(double timestamp)
{
	if (!input_running_) {
		return nullptr;
	}

	// TODO: if current_schedule_ is sorted, use binary search
	for (Batch& batch : current_schedule_) {
		if (timestamp >= batch.start_time && timestamp <= batch.end_time) {
			batch.started = true;
			batch.last_updated_time = Clock::now();
			return batch.clb_opt_data;
		}
	}

	return nullptr;
}

void DataHandler::closeBatch(Batch&& batch)
{
	std::lock_guard<std::mutex> l{waiting_batches_mtx_};
	g_elastic.log(INFO, "Scheduling batch for processing (" + std::to_string(waiting_batches_.size()) + " waiting)");
	waiting_batches_.push_back(std::move(batch));
	waiting_batches_cv_.notify_one();
}

void DataHandler::closeOldBatches(BatchSchedule& schedule)
{
	static constexpr std::chrono::seconds MATURATION_PERIOD{20};
	const auto close_time = Clock::now() - MATURATION_PERIOD;

	for (auto it = schedule.begin(); it != schedule.end(); ) {
		if (it->started && it->last_updated_time < close_time) {
			closeBatch(std::move(*it));
			it = schedule.erase(it);
		} else {
			++it;
		}
	}
}

void DataHandler::schedulingThread()
{
	g_elastic.log(INFO, "Scheduling thread up and running");

	while (scheduling_running_) {
		// Copy current schedule
		BatchSchedule new_schedule{std::cref(current_schedule_)};

		// Remove old batches from the schedule
		closeOldBatches(new_schedule);
		
		// Add some more
		batch_scheduler_->updateSchedule(new_schedule);
		current_schedule_.swap(new_schedule);

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	g_elastic.log(INFO, "Scheduling thread signing off");
}

void DataHandler::outputThread()
{
	g_elastic.log(INFO, "Output thread up and running");

	RunFile out_file{file_name_};
	if (!out_file.isOpen()) {
		g_elastic.log(ERROR, "Error opening file at path: '" + file_name_ + "'");
		return;
	}

	MergeSorter sorter{};
	CLBEventQueue out_queue{};

	do {
		// Obtain a batch to process.
		bool have_batch = false;
		Batch current_batch{};
		{
			std::unique_lock<std::mutex> l{waiting_batches_mtx_};
	
			if (!waiting_batches_.empty()) {
				// If there's something to process, dequeue.
				current_batch = std::move(waiting_batches_.front());
				waiting_batches_.pop_front();
				have_batch = true;
			} else if (!output_running_) {
				// If not, and we're done, stop.
				break;
			} else {
				// Otherwise, sleep until there is.
				waiting_batches_cv_.wait(l);
			}
		}

		if (!have_batch) {
			continue;
		}

		// At this point, we always have a valid batch.
		CLBEventMultiQueue& events{*current_batch.clb_opt_data};
		g_elastic.log(INFO, "Have a batch from " + std::to_string(events.size()) + " POMs");

		// Calculate complete timestamps & make sure sequence is sorted
		std::size_t n_hits{0};
		for (auto& key_value : events) {
			CLBEventQueue& queue = key_value.second;
			n_hits += queue.size();

			for (CLBEvent& event : queue) {
				// TODO: vectorize?
				event.SortKey = event.Timestamp_s + 1e-9 * event.Timestamp_ns;
			}

			// TODO: report disorder measure to backend
			const std::size_t n_swaps = insertSort(queue);
			g_elastic.log(INFO, "POM #" + std::to_string(key_value.first) + " (" + std::to_string(queue.size()) + " hits) required " + std::to_string(n_swaps) + " swaps to achieve time ordering");
		}

		// Merge-sort CLB events.
		out_queue.clear();
		g_elastic.log(INFO, "Merge-sorting " + std::to_string(n_hits) + " hits");
		sorter.merge(events, out_queue);

		// Write sorted events out.
		out_file.writeEventQueue(out_queue);
		out_file.flush();
		out_queue.clear();

		// TODO: report metrics to backend

		g_elastic.log(INFO, "Batch done and written.");
	} while (true);

	// Close output file.
	out_file.close();

	g_elastic.log(INFO, "Output thread signing off");
}

void DataHandler::joinThreads()
{
	// Wake output thread.
	output_running_ = false;
	scheduling_running_ = false;
	waiting_batches_cv_.notify_all();

	// Wait until it terminates.
	if (output_thread_ && output_thread_->joinable()) {
		output_thread_->join();
	}

	if (scheduling_thread_ && scheduling_thread_->joinable()) {
		scheduling_thread_->join();
	}

	// Kill it.
	output_thread_.reset();
	scheduling_thread_.reset();
}
