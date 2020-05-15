#include "periodic_spill_scheduler.h"

PeriodicSpillScheduler::PeriodicSpillScheduler(std::size_t n_batches_ahead, std::chrono::milliseconds batch_duration)
    : BasicSpillScheduler {}
    , n_batches_ahead_ { n_batches_ahead }
    , batch_duration_s_ { batch_duration.count() / 1000. }
{
    setUnitName("PeriodicSpillScheduler");
}

void PeriodicSpillScheduler::updateSchedule(SpillList& schedule, const tai_timestamp& last_approx_timestamp)
{
    if (last_approx_timestamp.empty()) {
        // If there is no data, wait for more.
        log(WARNING, "No hits received. Cannot schedule spills yet.");
        return;
    }

    // Initialize the very first spill.
    if (schedule.empty()) {
        SpillPtr first { new Spill };

        first->start_time = last_approx_timestamp;

        first->end_time = first->start_time;
        first->end_time.secs += batch_duration_s_;

        schedule.push_back(first);
    }

    // At this point, there's always a previous spill.
    while (schedule.size() < n_batches_ahead_) {
        SpillPtr next { new Spill };

        next->start_time = schedule.back()->end_time;

        next->end_time = next->start_time;
        next->end_time.secs += batch_duration_s_;

        schedule.push_back(next);
    }
}
