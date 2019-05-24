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

#include <atomic>
#include <chrono>
#include <ctime>
#include <list>
#include <memory>
#include <thread>

#include "XmlRpc.h"

#include "clb_event.h"

struct Batch {
    double start_time;
    double end_time;

    bool started;
    std::chrono::steady_clock::time_point last_updated_time;

    CLBEventMultiQueue* clb_opt_data;
};

using BatchSchedule = std::list<Batch>;

/// Base class for all schedulers, needs to be inherited and implemented.
class BatchScheduler {
public:
    virtual ~BatchScheduler() = default;
    virtual void beginScheduling() {}
    virtual void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) = 0;
    virtual void endScheduling() {}
};

/// Scheduler which produces only one infinite batch.
class InfiniteScheduler : public BatchScheduler {
public:
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;
};

/// Scheduler which produces batches of uniform length.
class RegularScheduler : public BatchScheduler {
    std::size_t n_batches_ahead_;
    double batch_duration_s_;

public:
    explicit RegularScheduler(std::size_t n_batches_ahead, std::chrono::milliseconds batch_duration);
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;
};

class SpillScheduler : public BatchScheduler {
    int port_;
    std::size_t trigger_memory_size_;
    double init_period_guess_;
    std::size_t n_batches_ahead_;
    double time_window_radius_;

    std::shared_ptr<std::thread> spill_server_thread_;

    class TriggerPredictor {
        std::vector<double> observed_;
        mutable std::vector<double> sorted_;
        double last_timestamp_;
        double learned_interval_;
        std::size_t next_;

    public:
        explicit TriggerPredictor(std::size_t n_last, double init_interval);
        void addTrigger(double timestamp);
        double learnedInterval() const;
    };

    class Spill : public XmlRpc::XmlRpcServerMethod {
        std::shared_ptr<TriggerPredictor> predictor_;

        static void convertNovaTimeToUnixTime(const std::uint64_t& inputNovaTime, struct timeval& outputUnixTime);

    public:
        Spill(XmlRpc::XmlRpcServer* server, std::shared_ptr<TriggerPredictor> predictor);
        void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    };

    enum SpillType {
        kNuMI, //MIBS $74 proton extraction into NuMI
        kBNB, //$1B paratisitic beam inhibit
        kNuMItclk, //tevatron clock, either $A9 or $AD depending on xml parameter
        kBNBtclk, //booster extraction, $1F (possibly sequence with $1D depending on configuration
        kAccelOneHztclk, //$8F 1 Hz clock
        kFake, //assigned if there is a parity error
        kTestConnection,
        kSuperCycle, //$00, Super cycle and master clock reset
        kNuMISampleTrig, //$A4,NuMI cycle sample trigger, reference for $A5
        kNuMIReset, //$A5, NuMI reset for beam
        kTBSpill, //$39, start of testbeam slow extraction
        kTBTrig, //testbeam trigger card signal
        kNSpillType // needs to be at the end, is used for range checking
    };
    static std::string getSpillNameFromType(int type);

    std::shared_ptr<XmlRpc::XmlRpcServer> spill_server_;
    std::shared_ptr<TriggerPredictor> predictor_;
    void workSpillServer();

public:
    explicit SpillScheduler(int port, std::size_t trigger_memory_size, double init_period_guess, std::size_t n_batches_ahead, double time_window_radius);
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;

    void join();
};

#endif /* BATCH_SCHEDULER_H_ */