#include "infinite_spill_scheduler.h"

InfiniteSpillScheduler::InfiniteSpillScheduler()
    : BasicSpillScheduler {}
{
    setUnitName("InfiniteSpillScheduler");
}

void InfiniteSpillScheduler::updateSchedule(SpillList& schedule, const tai_timestamp& last_approx_timestamp)
{
    if (schedule.empty()) {
        SpillPtr new_spill { new Spill };
        new_spill->start_time = tai_timestamp::min_time();
        new_spill->end_time = tai_timestamp::max_time();

        schedule.emplace_back(new_spill);
    }
}
