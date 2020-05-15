#include <functional>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "data_handler.h"
#include "run_file.h"

DataHandler::DataHandler()
    : Logging {}
    , AsyncComponent {}
    , last_approx_timestamp_ {}
    , scheduler_ {}
    , current_schedule_ {}
    , current_schedule_mtx_ {}
    , n_slots_ { 0 }
    , n_spills_ { 0 }
    , data_run_serialiser_ {}
{
    setUnitName("DataHandler");
}

void DataHandler::startRun(const std::shared_ptr<DataRun>& run, const std::shared_ptr<DataRunSerialiser>& run_serialiser)
{
    scheduler_ = run->getScheduler();
    data_run_serialiser_ = run_serialiser;

    // Prepare schedule.
    {
        boost::unique_lock<boost::upgrade_mutex> l { current_schedule_mtx_ };
        current_schedule_.clear();
    }

    last_approx_timestamp_ = tai_timestamp {};
    n_spills_ = 0;

    // Start output thread.
    runAsync();
}

void DataHandler::stopRun()
{
    log(DEBUG, "Joining scheduling thread.");

    // Kill scheduling thread and wait until it's done.
    notifyJoin();
    join();

    log(DEBUG, "Scheduling thread joined.");

    scheduler_.reset();
    data_run_serialiser_.reset();
}

PMTMultiPlaneHitQueue* DataHandler::findHitQueue(const tai_timestamp& timestamp, int data_slot_idx)
{
    // For reading schedule, we need reader's access.
    boost::shared_lock<boost::upgrade_mutex> lk { current_schedule_mtx_ };

    // TODO: if current_schedule_ is sorted, use binary search
    for (SpillPtr spill : current_schedule_) {
        if (timestamp >= spill->start_time && timestamp < spill->end_time) {
            spill->started = true;
            spill->last_updated_time = utc_timestamp::now();
            return spill->opt_hit_queues + data_slot_idx;
        }
    }

    return nullptr;
}

void DataHandler::closeSpill(SpillPtr spill)
{
    // Signal to CLB threads that no writes should be performed.
    for (int i = 0; i < n_slots_; ++i) {
        spill->opt_hit_queues[i].closed_for_writing = true;
    }

    // Wait for any ongoing writes to finish.
    for (int i = 0; i < n_slots_; ++i) {
        std::lock_guard<std::mutex> lk { spill->opt_hit_queues[i].mutex };
        // TODO: lock mutexes instead of using lock_guard
    }

    // At this point, no thread should be writing data to any of the queues.

    if (!spill->started) {
        log(INFO, "Spill {} discarded (not started at the time of closing).", spill->idx);
        delete spill;
        return;
    }

    log(INFO, "Closing spill {} for processing.", spill->idx);
    data_run_serialiser_->serialiseSpill(std::move(spill));
}

void DataHandler::closeOldSpills(SpillSchedule& schedule)
{
    // TODO: make maturation period configurable
    static constexpr std::uint64_t MATURATION_SECONDS { 4 };

    auto close_time { utc_timestamp::now() };
    close_time.secs -= MATURATION_SECONDS;

    for (auto it = schedule.begin(); it != schedule.end();) {
        SpillPtr spill { *it };
        if (spill->started && spill->last_updated_time < close_time) {
            closeSpill(std::move(*it));
            it = schedule.erase(it);
        } else {
            ++it;
        }
    }
}

void DataHandler::updateLastApproxTimestamp(const tai_timestamp& timestamp)
{
    // FIXME: this function accesses `last_approx_timestamp_` from a different thread
    // a race condition is possible
    if (timestamp > last_approx_timestamp_) {
        last_approx_timestamp_ = timestamp;
    }
}

void DataHandler::run()
{
    log(INFO, "Scheduling thread up and running");
    scheduler_->beginScheduling();

    while (running_) {
        {
            boost::upgrade_lock<boost::upgrade_mutex> lk { current_schedule_mtx_ };

            // Copy current schedule
            SpillSchedule new_schedule { std::cref(current_schedule_) };

            // Remove old spills from the schedule
            closeOldSpills(new_schedule);

            // Add some more
            scheduler_->updateSchedule(new_schedule, last_approx_timestamp_);
            prepareNewSpills(new_schedule);

            {
                boost::upgrade_to_unique_lock<boost::upgrade_mutex> ulk { lk };
                current_schedule_.swap(new_schedule);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Close remaining spills.
    SpillSchedule new_schedule { std::cref(current_schedule_) };
    for (auto it = new_schedule.begin(); it != new_schedule.end();) {
        closeSpill(std::move(*it));
        it = new_schedule.erase(it);
    }

    {
        boost::unique_lock<boost::upgrade_mutex> ulk { current_schedule_mtx_ };
        current_schedule_.swap(new_schedule);
    }

    scheduler_->endScheduling();
    log(INFO, "Scheduling thread signing off");
}

void DataHandler::prepareNewSpills(SpillSchedule& schedule)
{
    for (SpillPtr spill : schedule) {
        if (spill->created) {
            spill->created = false;

            spill->idx = n_spills_++;
            spill->started = false;
            spill->last_updated_time = utc_timestamp::now();
            spill->opt_hit_queues = new PMTMultiPlaneHitQueue[n_slots_];
            spill->n_data_slots = n_slots_;

            log(INFO, "Scheduling spill {} with time interval: [{}, {}]", spill->idx, spill->start_time, spill->end_time);
        }
    }
}

int DataHandler::assignNewSlot()
{
    return n_slots_++;
}
