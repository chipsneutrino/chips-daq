/**
 * SchedulingPool - Takes care of scheduling strategies, which are occasionally
 * used to determine batches during runs.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>

#include "batch_scheduler.h"
#include "spill_scheduler.h"

class SchedulingPool {
public:
    SchedulingPool();

    inline const std::shared_ptr<InfiniteScheduler>& infiniteScheduler() const { return infinite_scheduler_; }
    inline const std::shared_ptr<RegularScheduler>& regularScheduler() const { return regular_scheduler_; }
    inline const std::shared_ptr<SpillScheduler>& spillScheduler() const { return spill_scheduler_; }

private:
    std::shared_ptr<InfiniteScheduler> infinite_scheduler_;
    std::shared_ptr<RegularScheduler> regular_scheduler_;
    std::shared_ptr<SpillScheduler> spill_scheduler_;
};