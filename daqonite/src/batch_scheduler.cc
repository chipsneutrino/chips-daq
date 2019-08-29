#include <limits>

#include <util/elastic_interface.h>

#include "batch_scheduler.h"

void InfiniteScheduler::updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp)
{
    if (schedule.empty()) {
        Batch single_window {};
        single_window.created = true;
        single_window.start_time = 0;
        single_window.end_time = std::numeric_limits<decltype(Batch::end_time)>::max();

        schedule.push_back(std::move(single_window));
    }
}

RegularScheduler::RegularScheduler(std::size_t n_batches_ahead, std::chrono::milliseconds batch_duration)
    : n_batches_ahead_ { n_batches_ahead }
    , batch_duration_s_ { batch_duration.count() / 1000. }
{
}

void RegularScheduler::updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp)
{
    if (last_approx_timestamp == 0) {
        // If there is no data, wait for more.
        g_elastic.log(WARNING, "No packets received. Cannot schedule batches yet.");
        return;
    }

    // Initialize the very first batch.
    if (schedule.empty()) {
        Batch first {};
        first.created = true;
        first.start_time = last_approx_timestamp;
        first.end_time = first.start_time + batch_duration_s_;

        schedule.push_back(std::move(first));
    }

    // At this point, there's always a previous batch.
    while (schedule.size() < n_batches_ahead_) {
        Batch next {};
        next.created = true;
        next.start_time = schedule.back().end_time;
        next.end_time = next.start_time + batch_duration_s_;

        schedule.push_back(std::move(next));
    }
}
