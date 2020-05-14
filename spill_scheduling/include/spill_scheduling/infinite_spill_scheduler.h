/**
 * InfiniteSpillScheduler - Scheduler that produces only a single infinitely-long
 * spill
 * 
 * This scheduler is the most trivial, as it effectively implements the "always
 * live" detector behaviour. This is immensely useful for maintenance and debugging.
 * However, it comes at a cost of potentially unbounded memory consumption. Since the
 * spill never ends, hits that accumulate in it are not processed until the run is
 * manually stopped. Obviously, if left running for too long, this will eventually
 * exhaust RAM.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <spill_scheduling/basic_spill_scheduler.h>

class InfiniteSpillScheduler : public BasicSpillScheduler {
public:
    explicit InfiniteSpillScheduler();

    void updateSchedule(SpillSchedule& schedule, const tai_timestamp& last_approx_timestamp) override;
};
