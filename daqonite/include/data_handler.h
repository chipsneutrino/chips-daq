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

#include <boost/thread.hpp>

#include <util/control_msg.h>
#include <util/logging.h>

#include "data_run.h"
#include "data_run_serialiser.h"

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
    PMTMultiPlaneHitQueue* findHitQueue(const tai_timestamp& timestamp, int data_slot_idx);

    /// Bump up last approximate timestamp.
    void updateLastApproxTimestamp(const tai_timestamp& timestamp);

    /// Wait for threads to terminate.
    void join();

    /// Create new slot for CLB optical data. Must *not* be called during run.
    int assignNewSlot();

private:
    std::unique_ptr<DataRunSerialiser> serialiser_; ///< Thread for merge-sorting and saving
    std::unique_ptr<std::thread> scheduling_thread_; ///< Thread for scheduling and closing spills

    /// Synchronously terminate all threads.
    void joinThreads();

    std::atomic_bool scheduling_running_; ///< Is scheduling thread supposed to be running?

    tai_timestamp last_approx_timestamp_; ///< Latest timestamp sufficiently in the past (used by scheduler)
    std::shared_ptr<BasicSpillScheduler> scheduler_; ///< Scheduler of spill intervals.

    SpillSchedule current_schedule_; ///< Spills open for data writing.
    boost::upgrade_mutex current_schedule_mtx_; ///< Multiple-reader / single-writer mutex for current schedule.

    int n_slots_; ///< Number of open data slots. Must be constant during runs.
    int n_spills_; ///< Number of opened spills. Used for indexing.

    /// Close all spills which were not modified for a sufficiently long duration.
    void closeOldSpills(SpillSchedule& schedule);

    /// Close one specific spill.
    void closeSpill(SpillPtr spill);

    /// Main entry point of the scheduling thread.
    void schedulingThread();

    /// Allocate data structures for newly created spills.
    void prepareNewSpills(SpillSchedule& schedule);
};
