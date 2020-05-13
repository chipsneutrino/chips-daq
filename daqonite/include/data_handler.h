/**
 * DataHandler - Handler class for combining the data and saving to file
 * 
 * This class deals with combing the data stream from both the CLB and BBB,
 * sorting and then saving to file
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 *
 * Co-author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>

#include <util/control_msg.h>
#include <util/logging.h>

#include "clb_event.h"
#include "data_run.h"
#include "merge_sorter.h"

class DataHandler : protected Logging {
public:
    /// Create a DataHandler
    explicit DataHandler();

    DataHandler(const DataHandler& other) = delete;
    DataHandler(DataHandler&& other) = delete;

    DataHandler& operator=(const DataHandler& other) = delete;
    DataHandler& operator=(DataHandler&& other) = delete;

    /// Destroy a DataHandler
    virtual ~DataHandler() = default;

    /**
     * Start a data taking run
     * Sets the run variables, opens the output file, and adds TTrees and branches
     */
    void startRun(const std::shared_ptr<DataRun>& run);

    /**
     * Stop a data taking run
     * Writes the TTrees to file, close the output file, and cleanup/reset variables
     */
    void stopRun();

    /// Find a queue for CLB data coming at a specific time.
    HitMultiQueue* findCLBOpticalQueue(const tai_timestamp& timestamp, int data_slot_idx);

    /// Bump up last approximate timestamp.
    void updateLastApproxTimestamp(const tai_timestamp& timestamp);

    /// Wait for threads to terminate.
    void join();

    /// Create new slot for CLB optical data. Must *not* be called during run.
    int assignNewSlot();

private:
    std::unique_ptr<std::thread> output_thread_; ///< Thread for merge-sorting and saving
    std::unique_ptr<std::thread> scheduling_thread_; ///< Thread for scheduling and closing batches

    /// Synchronously terminate all threads.
    void joinThreads();

    std::shared_ptr<DataRun> run_;
    std::atomic_bool output_running_; ///< Is output thread supposed to be running?
    std::atomic_bool scheduling_running_; ///< Is scheduling thread supposed to be running?

    using BatchQueue = boost::lockfree::queue<Batch, boost::lockfree::capacity<16>>;
    BatchQueue waiting_batches_; ///< Thread-safe FIFO queue for closed batches pending merge-sort

    /// Main entry point of the output thread.
    void outputThread(std::shared_ptr<DataRun> run);

    tai_timestamp last_approx_timestamp_; ///< Latest timestamp sufficiently in the past (used by scheduler)
    std::shared_ptr<BatchScheduler> batch_scheduler_; ///< Scheduler of batch intervals.

    BatchSchedule current_schedule_; ///< Batches open for data writing.
    boost::upgrade_mutex current_schedule_mtx_; ///< Multiple-reader / single-writer mutex for current schedule.

    int n_slots_; ///< Number of open data slots. Must be constant during runs.
    int n_batches_; ///< Number of opened batches. Used for indexing.

    /// Close all batches which were not modified for a sufficiently long duration.
    void closeOldBatches(BatchSchedule& schedule);

    /// Close one specific batch.
    void closeBatch(Batch&& batch);

    /// Main entry point of the scheduling thread.
    void schedulingThread();

    /// Allocate data structures for newly created batches.
    void prepareNewBatches(BatchSchedule& schedule);

    /// Dispose of data structures associated with a bathc.
    static void disposeBatch(Batch& batch);

    /// Implementation of conventional insert-sort algorithm used to pre-sort CLB queues.
    static std::size_t insertSort(HitQueue& queue) noexcept;
};
