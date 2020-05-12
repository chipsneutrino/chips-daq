#include "scheduling_pool.h"

// TODO: make these configurable
SchedulingPool::SchedulingPool()
    : infinite_scheduler_ { new InfiniteScheduler }
    , regular_scheduler_ { new RegularScheduler(8, std::chrono::minutes(1)) }
    , spill_scheduler_ { new SpillScheduler(55812, 20, 1.5, 8, 0.5) }
{
}