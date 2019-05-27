/**
 * SpillScheduler - Batch scheduler based on FNAL spill signals
 * 
 * Spill signals come in as XML/RPC calls with type NOvA timestamps.
 * The scheduler runs an asynchronous server to receive these calls, 
 * converts clock signals and predicts the next few triggers.
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <ctime>
#include <memory>
#include <thread>

#include "XmlRpc.h"

#include "batch_scheduler.h"
#include "trigger_predictor.h"

/// Various types of spill signals. Used for discrimination.
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

std::string getSpillNameFromType(int type);

/// Clock signal conversion from NoVA to UTC. Nicked from NSS time utils.
void convertNovaTimeToUnixTime(const std::uint64_t& inputNovaTime, struct timeval& outputUnixTime);

class SpillScheduler : public BatchScheduler {
    int port_;
    std::size_t trigger_memory_size_;
    double init_period_guess_;
    std::size_t n_batches_ahead_;
    double time_window_radius_;

    std::unique_ptr<std::thread> spill_server_thread_;

    class Spill : public XmlRpc::XmlRpcServerMethod {
        std::shared_ptr<TriggerPredictor> predictor_;

    public:
        Spill(XmlRpc::XmlRpcServer* server, std::shared_ptr<TriggerPredictor> predictor);
        void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    };

    std::unique_ptr<XmlRpc::XmlRpcServer> spill_server_;
    std::shared_ptr<TriggerPredictor> predictor_;
    void workSpillServer();

public:
    explicit SpillScheduler(int port, std::size_t trigger_memory_size, double init_period_guess, std::size_t n_batches_ahead, double time_window_radius);
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;

    void join();
};