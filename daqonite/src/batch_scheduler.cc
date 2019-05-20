#include <limits>

#include "batch_scheduler.h"

void InfiniteScheduler::updateSchedule(BatchSchedule& schedule)
{
    if (schedule.empty()) {
        Batch single_window{};
        single_window.start_time = 0;
        single_window.end_time = std::numeric_limits<decltype(Batch::end_time)>::max();

        schedule.push_back(std::move(single_window));
    }
}

void RegularScheduler::updateSchedule(BatchSchedule& schedule)
{
    // TODO: implement me!
}
