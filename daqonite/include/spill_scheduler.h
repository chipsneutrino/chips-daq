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

#include <atomic>
#include <ctime>
#include <memory>
#include <thread>

#include "XmlRpc.h"

#include <util/logging.h>

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

class SpillScheduler : public BatchScheduler, protected Logging {
    int port_; ///< Port where spill messages are expected to come.
    std::size_t trigger_memory_size_; ///< How many past triggers to remember (more = robust, less = quickly adapting)
    double init_period_guess_; ///< Initial guess of trigger period in seconds. Will be eventually overwritten.
    std::size_t n_batches_ahead_; ///< How many batches to open in the future?
    double time_window_radius_; ///< Duration around spill time for batches.
    bool bypass_os_time_; ///< If true, spill scheduler will wait for optical data to get timestamps instead of calling OS functions. Use if OS time is not in sync with WR time.

    std::atomic_bool spill_server_running_; ///< Is the spill server supposed to be running?
    std::unique_ptr<std::thread> spill_server_thread_; ///< Spill server thread.

    class Spill : public XmlRpc::XmlRpcServerMethod, protected Logging {
        std::shared_ptr<TriggerPredictor> predictor_; ///< Spill interval predictor.

    public:
        Spill(XmlRpc::XmlRpcServer* server, std::shared_ptr<TriggerPredictor> predictor);

        /// Receive spill XML-RPC message.
        void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    };

    std::unique_ptr<XmlRpc::XmlRpcServer> spill_server_; ///< XML-RPC server handling spill requests.
    std::shared_ptr<TriggerPredictor> predictor_; ///< Spill interval predictor.

    /// Main loop of the spill server thread.
    void workSpillServer();

public:
    explicit SpillScheduler(int port, std::size_t trigger_memory_size, double init_period_guess, std::size_t n_batches_ahead, double time_window_radius);
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;

    /// Wait until spill server terminates.
    void join();
};