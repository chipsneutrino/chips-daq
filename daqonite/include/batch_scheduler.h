/**
 * BatchScheduler - Class responsible for scheduling data taking windows
 * 
 * These classes maintain batch schedule -- a series of time intervals, to
 * which taken data is matched. 
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#ifndef BATCH_SCHEDULER_H_
#define BATCH_SCHEDULER_H_

#include <list>
#include <memory>
#include <chrono>

#include "clb_event.h"

struct Batch {
    double start_time;
    double end_time;
    
    bool started;
    std::chrono::steady_clock::time_point last_updated_time;

    CLBEventMultiQueue* clb_opt_data;

};

using BatchSchedule = std::list<Batch>;

class BatchScheduler {
public:
    virtual ~BatchScheduler() = default;
    virtual void updateSchedule(BatchSchedule& schedule) = 0;

};

class InfiniteScheduler: public BatchScheduler {
public:
    void updateSchedule(BatchSchedule& schedule) override;

};

class RegularScheduler: public BatchScheduler {
public:
    void updateSchedule(BatchSchedule& schedule) override;

};

// TODO: SpillScheduler

#endif /* BATCH_SCHEDULER_H_ */