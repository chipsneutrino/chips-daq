/**
 * DataHandler - Handler class for combining the data and saving to file
 */

#include <functional>

#include "data_handler.h"
#include "run_file.h"
#include <util/elastic_interface.h>

DataHandler::DataHandler()
    : output_thread_{}
    , scheduling_thread_{}
    , output_running_{ false }
    , scheduling_running_{ false }
    , run_type_{}
    , run_num_{}
    , file_name_{}
    , waiting_batches_{}
    , last_approx_timestamp_{ 0 }
    , batch_scheduler_{}
    , infinite_scheduler_{ new InfiniteScheduler }
    , regular_scheduler_{ new RegularScheduler(8, std::chrono::minutes(1)) }
    , spill_scheduler_{ new SpillScheduler(55812, 20, 1.5, 8, 0.5) }
    , current_schedule_{}
    , current_schedule_mtx_{}
    , n_slots_{ 0 }
    , n_batches_{ 0 }
{
}

void DataHandler::startRun(RunType which)
{
    // Set the fRun_type, fRun_num and fFile_name run variables
    run_type_ = which;
    getRunNumAndName();

    // Prepare queues and schedule.
    waiting_batches_.consume_all([](Batch& b) { ; });
    {
        boost::unique_lock<boost::upgrade_mutex> l{ current_schedule_mtx_ };
        current_schedule_.clear();
    }

    // TODO: determine this from run_type
    last_approx_timestamp_ = 0;
    n_batches_ = 0;
    batch_scheduler_ = static_cast<const std::shared_ptr<RegularScheduler>&>(regular_scheduler_);

    // Start output thread.
    g_elastic.log(WARNING, "Start mining into container {}", file_name_);
    output_running_ = scheduling_running_ = true;
    output_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&DataHandler::outputThread, this)) };
    scheduling_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&DataHandler::schedulingThread, this)) };
}

void DataHandler::stopRun()
{
    g_elastic.log(WARNING, "Signal stop mining into container {}, waiting for output threads to complete...", file_name_);

    // Wait for the output thread to end
    joinThreads();
    g_elastic.log(WARNING, "Stop mining into container {}", file_name_);

    // Reset the run variables
    run_type_ = {};
    run_num_ = {};
    file_name_ = "";
}

void DataHandler::getRunNumAndName()
{
    const int run_type_no = static_cast<int>(run_type_);
    int runNums[NUMRUNTYPES];
    std::ifstream runNumFile("../data/runNumbers.dat");
    if (runNumFile.fail()) {
        runNumFile.close();
        // The file does not yet exist so lets create it
        std::ofstream newFile("../data/runNumbers.dat");
        if (newFile.is_open()) {
            for (int i = 0; i < NUMRUNTYPES; i++) {
                if (run_type_no == i) {
                    newFile << 2 << "\n";
                } else {
                    newFile << 1 << "\n";
                }
            }
            newFile.close();
        } else {
            throw std::runtime_error("daqonite - Error: Unable to create ../data/runNumbers.dat!");
        }
    } else {
        // The file exists so read from it
        for (int i = 0; i < NUMRUNTYPES; i++) {
            runNumFile >> runNums[i];
            if (runNums[i] < 1) {
                runNums[i] = 1;
            }
            if (run_type_no == i) {
                run_num_ = runNums[i];
            }
        }
        runNumFile.close();

        // Now create the updated file
        std::ofstream updateFile("../data/runNumbers.dat");
        if (updateFile.is_open()) {
            for (int i = 0; i < NUMRUNTYPES; i++) {
                if (run_type_no == i) {
                    updateFile << runNums[i] + 1 << "\n";
                } else {
                    updateFile << 1 << "\n";
                }
            }
            updateFile.close();
        } else {
            throw std::runtime_error("daqonite - Error: Unable to update runNumbers.dat!");
        }
    }

    file_name_ = fmt::format("../data/type{}_run{}.root", run_type_no, run_num_);
}

std::size_t DataHandler::insertSort(CLBEventQueue& queue) noexcept
{
    // Just your conventional O(n^2) insert-sort implementation.
    // Here utilized because insert-sort is actually O(n+k*n) for k-sorted sequences.
    // Since event queue should already be sorted, insert-sort will frequently only scan it in O(n).

    std::size_t n_swaps{ 0 };
    for (std::size_t i = 1; i < queue.size(); ++i) {
        for (std::size_t j = i; j > 0 && queue[j - 1] > queue[j]; --j) {
            std::swap(queue[j], queue[j - 1]);
            ++n_swaps;
        }
    }

    return n_swaps;
}

CLBEventMultiQueue* DataHandler::findCLBOpticalQueue(double timestamp, int data_slot_idx)
{
    // For reading schedule, we need reader's access.
    boost::shared_lock<boost::upgrade_mutex> lk{ current_schedule_mtx_ };

    // TODO: if current_schedule_ is sorted, use binary search
    for (Batch& batch : current_schedule_) {
        if (timestamp >= batch.start_time && timestamp < batch.end_time) {
            batch.started = true;
            batch.last_updated_time = Clock::now();
            return batch.clb_opt_data + data_slot_idx;
        }
    }

    return nullptr;
}

void DataHandler::closeBatch(Batch&& batch)
{
    // Signal to CLB threads that no writes should be performed.
    for (int i = 0; i < n_slots_; ++i) {
        batch.clb_opt_data[i].closed_for_writing = true;
    }

    // Wait for any ongoing writes to finish.
    for (int i = 0; i < n_slots_; ++i) {
        std::lock_guard<std::mutex> lk{ batch.clb_opt_data[i].write_mutex };
    }

    // At this point, no thread should be writing data to any of the queues.

    if (!batch.started) {
        g_elastic.log(INFO, "Batch {} was discarded because it was not started at the time of closing.", batch.idx);
        disposeBatch(batch);
        return;
    }

    g_elastic.log(INFO, "Closing batch {} for processing.", batch.idx);
    waiting_batches_.push(std::move(batch));
}

void DataHandler::closeOldBatches(BatchSchedule& schedule)
{
    static constexpr std::chrono::seconds MATURATION_PERIOD{ 20 };
    const auto close_time = Clock::now() - MATURATION_PERIOD;

    for (auto it = schedule.begin(); it != schedule.end();) {
        if (it->started && it->last_updated_time < close_time) {
            closeBatch(std::move(*it));
            it = schedule.erase(it);
        } else {
            ++it;
        }
    }
}

void DataHandler::updateLastApproxTimestamp(std::uint32_t timestamp)
{
    if (timestamp > last_approx_timestamp_) {
        last_approx_timestamp_ = timestamp;
    }
}

void DataHandler::schedulingThread()
{
    g_elastic.log(INFO, "Scheduling thread up and running");
    batch_scheduler_->beginScheduling();

    while (scheduling_running_) {
        {
            boost::upgrade_lock<boost::upgrade_mutex> lk{ current_schedule_mtx_ };

            // Copy current schedule
            BatchSchedule new_schedule{ std::cref(current_schedule_) };

            // Remove old batches from the schedule
            closeOldBatches(new_schedule);

            // Add some more
            batch_scheduler_->updateSchedule(new_schedule, last_approx_timestamp_);
            prepareNewBatches(new_schedule);

            {
                boost::upgrade_to_unique_lock<boost::upgrade_mutex> ulk{ lk };
                current_schedule_.swap(new_schedule);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Close remaining batches.
    BatchSchedule new_schedule{ std::cref(current_schedule_) };
    for (auto it = new_schedule.begin(); it != new_schedule.end();) {
        closeBatch(std::move(*it));
        it = new_schedule.erase(it);
    }

    {
        boost::unique_lock<boost::upgrade_mutex> ulk{ current_schedule_mtx_ };
        current_schedule_.swap(new_schedule);
    }

    batch_scheduler_->endScheduling();
    g_elastic.log(INFO, "Scheduling thread signing off");
}

void DataHandler::prepareNewBatches(BatchSchedule& schedule)
{
    for (Batch& batch : schedule) {
        if (batch.created) {
            batch.created = false;

            batch.idx = n_batches_++;
            batch.started = false;
            batch.last_updated_time = Clock::now();
            batch.clb_opt_data = new CLBEventMultiQueue[n_slots_];

            g_elastic.log(INFO, "Scheduling batch {} at with time interval: [{}, {}]", batch.idx, batch.start_time, batch.end_time);
        }
    }
}

void DataHandler::disposeBatch(Batch& batch)
{
    delete[] batch.clb_opt_data;
}

void DataHandler::outputThread()
{
    g_elastic.log(INFO, "Output thread up and running");

    // Open output.
    RunFile out_file{ file_name_ };
    if (!out_file.isOpen()) {
        g_elastic.log(ERROR, "Error opening file at path: '{}'", file_name_);
        return;
    }

    MergeSorter sorter{};
    CLBEventQueue out_queue{};
    for (;;) {
        // Obtain a batch to process.
        bool have_batch = false;
        Batch current_batch{};

        if (waiting_batches_.pop(current_batch)) {
            // If there's something to process, dequeue.
            have_batch = true;
        } else if (!output_running_) {
            // If not, and we're done, stop.
            break;
        } else {
            // Sleep.
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (!have_batch) {
            continue;
        }

        // At this point, we always have a valid batch.

        // Consolidate multi-queue by moving it into a single instance.
        CLBEventMultiQueue events{};
        for (int i = 0; i < n_slots_; ++i) {
            CLBEventMultiQueue& slot_multiqueue = current_batch.clb_opt_data[i];
            for (auto it = slot_multiqueue.begin(); it != slot_multiqueue.end(); ++it) {
                events.emplace(it->first, std::move(it->second));
            }
        }

        g_elastic.log(INFO, "Processing batch {} (from {} POMs)", current_batch.idx, events.size());

        // Calculate complete timestamps & make sure sequence is sorted
        std::size_t n_hits{ 0 };
        for (auto& key_value : events) {
            CLBEventQueue& queue = key_value.second;
            n_hits += queue.size();

            // TODO: report disorder measure to backend
            const std::size_t n_swaps = insertSort(queue);
            g_elastic.log(INFO, "POM #{} ({} hits) required {} swaps to achieve time ordering", key_value.first, queue.size(), n_swaps);
        }

        if (n_hits > 0) {
            // Merge-sort CLB events.
            out_queue.clear();
            g_elastic.log(INFO, "Merge-sorting {} hits", n_hits);
            sorter.merge(events, out_queue);

            // Write sorted events out.
            out_file.writeEventQueue(out_queue);
            out_file.flush();
            out_queue.clear();

            // TODO: report metrics to backend
        } else {
            g_elastic.log(INFO, "Batch {} is empty.", current_batch.idx);
        }

        g_elastic.log(INFO, "Batch {} done and written", current_batch.idx);
        disposeBatch(current_batch);
    }

    // Close output file.
    out_file.close();

    g_elastic.log(INFO, "Output thread signing off");
}

void DataHandler::joinThreads()
{
    // Kill scheduling thread and wait until it's done.
    scheduling_running_ = false;
    if (scheduling_thread_ && scheduling_thread_->joinable()) {
        scheduling_thread_->join();
    }

    // Now that all batches are closed, we can signal termination of the output thread.
    output_running_ = false;
    if (output_thread_ && output_thread_->joinable()) {
        output_thread_->join();
    }

    // Clean up.
    output_thread_.reset();
    scheduling_thread_.reset();
    batch_scheduler_.reset();
}

void DataHandler::join()
{
    spill_scheduler_->join();
}

int DataHandler::assignNewSlot()
{
    return n_slots_++;
}
