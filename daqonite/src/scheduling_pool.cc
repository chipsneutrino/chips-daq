#include "scheduling_pool.h"

// TODO: make these configurable
SchedulingPool::SchedulingPool()
    : infinite_scheduler_ { new InfiniteSpillScheduler }
    , periodic_scheduler_ { new PeriodicSpillScheduler(8, std::chrono::minutes(1)) }
    , tdu_scheduler_ { new TDUSpillScheduler(55812, 20, 1.5, 8, 0.5) }
{
}