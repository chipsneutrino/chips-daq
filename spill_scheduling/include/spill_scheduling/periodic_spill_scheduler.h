/**
 * PeriodicSpillScheduler - A scheduler that creates periodic spills of constant
 * duration
 * 
 * This scheduler simulates a more realistic spill signal by creating multiple
 * spills with a set frequency. The scheduler always strives to have N batches open
 * at any time, where N is an adjustable parameter.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <spill_scheduling/basic_spill_scheduler.h>

class PeriodicSpillScheduler : public BasicSpillScheduler {
    std::size_t n_batches_ahead_;
    double batch_duration_s_;

public:
    explicit PeriodicSpillScheduler(std::size_t n_batches_ahead, std::chrono::milliseconds batch_duration);

    void updateSchedule(SpillSchedule& schedule, const tai_timestamp& last_approx_timestamp) override;
};
