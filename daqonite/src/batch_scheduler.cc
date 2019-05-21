#include <limits>

#include "batch_scheduler.h"

void InfiniteScheduler::updateSchedule(BatchSchedule& schedule)
{
    if (schedule.empty()) {
        Batch single_window{};

        single_window.started = false;
        single_window.last_updated_time = std::chrono::steady_clock::now();

        single_window.clb_opt_data = new CLBEventMultiQueue();

        single_window.start_time = 0;
        single_window.end_time = std::numeric_limits<decltype(Batch::end_time)>::max();

        schedule.push_back(std::move(single_window));
    }
}

void RegularScheduler::updateSchedule(BatchSchedule& schedule)
{
    // TODO: implement me!
}
