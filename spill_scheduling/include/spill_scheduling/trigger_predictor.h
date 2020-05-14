/**
 * TriggerPredictor - Median-filter predictor for trigger period.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <vector>

#include <util/timestamp.h>

// TODO: remove this in favour of timestamp arithmetic
using TimeDiff = long double;

class TriggerPredictor {
    std::vector<TimeDiff> observed_;
    mutable std::vector<TimeDiff> sorted_;
    tai_timestamp last_timestamp_;
    TimeDiff learned_interval_;
    std::size_t next_;

public:
    explicit TriggerPredictor(std::size_t n_last, TimeDiff init_interval);

    void addTrigger(const tai_timestamp& timestamp);
    const tai_timestamp& lastTimestamp() const;
    TimeDiff learnedInterval() const;
};
