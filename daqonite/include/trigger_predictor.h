/**
 * TriggerPredictor - Median-filter predictor for trigger period.
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <vector>

using Timestamp = long double;
using TimeDiff = long double;

class TriggerPredictor {
    std::vector<TimeDiff> observed_;
    mutable std::vector<TimeDiff> sorted_;
    Timestamp last_timestamp_;
    TimeDiff learned_interval_;
    std::size_t next_;

public:
    explicit TriggerPredictor(std::size_t n_last, TimeDiff init_interval);

    void addTrigger(Timestamp timestamp);
    TimeDiff learnedInterval() const;
};
