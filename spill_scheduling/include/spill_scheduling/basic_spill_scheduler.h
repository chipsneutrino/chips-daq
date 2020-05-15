/**
 * BasicSpillScheduler - A base class for all spill scheduling strategies
 * 
 * These classes construct and maintain a spill schedule -- a series of time
 * intervals (that should ideally correspond to physical spills of the NuMI beam),
 * to which PMT hits are matched based on their timestamps. Spill schedulers live
 * in their own scheduling thread, where they periodically make adjustments to the
 * schedule.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <chrono>
#include <list>

#include <util/logging.h>
#include <util/pmt_hit.h>
#include <util/timestamp.h>

#include "spill.h"

using SpillSchedule = std::list<SpillPtr>;

/// Base class for all schedulers, needs to be inherited and implemented.
class BasicSpillScheduler : protected Logging {
public:
    explicit BasicSpillScheduler();
    virtual ~BasicSpillScheduler() = default;

    virtual void beginScheduling();
    virtual void updateSchedule(SpillSchedule& schedule, const tai_timestamp& last_approx_timestamp) = 0;
    virtual void endScheduling();
};
