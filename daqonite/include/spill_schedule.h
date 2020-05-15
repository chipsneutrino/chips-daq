/**
 * SpillSchedule - Handler class for combining the data and saving to file
 * 
 * This class deals with combing the data stream from both the CLB and BBB,
 * sorting and then saving to file
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 *
 * Co-author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include <boost/thread.hpp>

#include <spill_scheduling/spill_data_slot.h>
#include <util/control_msg.h>
#include <util/logging.h>

#include "data_run.h"
#include "data_run_serialiser.h"

class SpillSchedule : protected Logging, public AsyncComponent {
public:
    explicit SpillSchedule();
    virtual ~SpillSchedule() = default;

    // no copy semantics
    SpillSchedule(const SpillSchedule& other) = delete;
    SpillSchedule& operator=(const SpillSchedule& other) = delete;

    // no move semantics
    SpillSchedule& operator=(SpillSchedule&& other) = delete;
    SpillSchedule(SpillSchedule&& other) = delete;

    /**
     * Start a data taking run
     * Sets the run variables, opens the output file, and adds TTrees and branches
     */
    void startRun(const std::shared_ptr<DataRun>& run, const std::shared_ptr<DataRunSerialiser>& run_serialiser);

    /**
     * Stop a data taking run
     * Writes the TTrees to file, close the output file, and cleanup/reset variables
     */
    void stopRun();

    SpillDataSlot* findDataSlot(const tai_timestamp& timestamp, std::size_t data_slot_idx);

    /// Bump up last approximate timestamp.
    void updateLastApproxTimestamp(const tai_timestamp& timestamp);

    /// Create new slot for spill data. Must *not* be called during run.
    std::size_t assignNewSlot();

protected:
    void run() override;

private:
    tai_timestamp last_approx_timestamp_; ///< Latest timestamp sufficiently in the past (used by scheduler)
    std::shared_ptr<BasicSpillScheduler> scheduler_; ///< Scheduler of spill intervals.

    SpillList current_schedule_; ///< Spills open for data writing.
    boost::upgrade_mutex current_schedule_mtx_; ///< Multiple-reader / single-writer mutex for current schedule.

    std::size_t n_slots_; ///< Number of open data slots. Must be constant during runs.
    std::size_t n_spills_; ///< Number of opened spills. Used for indexing.

    std::shared_ptr<DataRunSerialiser> data_run_serialiser_;

    /// Close all spills which were not modified for a sufficiently long duration.
    void closeOldSpills(SpillList& schedule);

    /// Close one specific spill.
    void closeSpill(SpillPtr spill);

    /// Allocate data structures for newly created spills.
    void prepareNewSpills(SpillList& schedule);
};
