/**
 * SchedulingPool - Takes care of scheduling strategies, which are occasionally
 * used to determine batches during runs.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>

#include <spill_scheduling/infinite_spill_scheduler.h>
#include <spill_scheduling/periodic_spill_scheduler.h>
#include <spill_scheduling/tdu_spill_scheduler.h>

class SchedulingPool {
public:
    explicit SchedulingPool();

    inline const std::shared_ptr<InfiniteSpillScheduler>& infiniteScheduler() const { return infinite_scheduler_; }
    inline const std::shared_ptr<PeriodicSpillScheduler>& periodicScheduler() const { return periodic_scheduler_; }
    inline const std::shared_ptr<TDUSpillScheduler>& tduScheduler() const { return tdu_scheduler_; }

private:
    std::shared_ptr<InfiniteSpillScheduler> infinite_scheduler_;
    std::shared_ptr<PeriodicSpillScheduler> periodic_scheduler_;
    std::shared_ptr<TDUSpillScheduler> tdu_scheduler_;
};