#include <functional>
#include <sstream>

#include "spill_scheduler.h"
#include <util/elastic_interface.h>

SpillScheduler::SpillScheduler(int port, std::size_t trigger_memory_size, double init_period_guess, std::size_t n_batches_ahead, double time_window_radius)
    : port_ { port }
    , trigger_memory_size_ { trigger_memory_size }
    , init_period_guess_ { init_period_guess }
    , n_batches_ahead_ { n_batches_ahead }
    , time_window_radius_ { time_window_radius }
    , spill_server_running_ {}
    , spill_server_thread_ {}
    , spill_server_ {}
{
    spill_server_running_ = true;
    spill_server_thread_ = std::unique_ptr<std::thread> { new std::thread(std::bind(&SpillScheduler::workSpillServer, this)) };
}

void SpillScheduler::join()
{
    if (spill_server_thread_ && spill_server_thread_->joinable()) {
        spill_server_running_ = false;
        spill_server_thread_->join();
    }

    spill_server_thread_.reset();
}

void SpillScheduler::updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp)
{
    // TODO: configure these
    static const std::size_t n_batches_ahead_ = 8;
    static const double window_half_width_s_ = 0.005;

    if (schedule.size() >= n_batches_ahead_) {
        return;
    }

    // FIXME: race condition? get both at the same time
    const Timestamp last_timestamp = predictor_->lastTimestamp();
    const TimeDiff learned_interval = predictor_->learnedInterval();

    // TODO: set up intervals based on the predicted triggers
    // This can be done once:
    //   (1) we know what time representation we'll use
    //   (2) we know how trigger predictions and signals will be matched
    //   (3) white rabbits start giving sensible timestamps

    Timestamp schedule_basis_timestamp { static_cast<Timestamp>(last_approx_timestamp) };
    if (!schedule.empty()) {
        // Ensure that the timestamp we use as a basis for scheduling is as recent as possible.
        // This prevents creating batch in the past if the schedule capacity is too small.
        const Timestamp last_scheduled_timestamp { schedule.back().end_time };

        if (last_scheduled_timestamp >= schedule_basis_timestamp) {
            schedule_basis_timestamp = last_scheduled_timestamp;
        } else {
            g_elastic.log(WARNING, "Data ({}) is more recent than the last scheduled batch ({}). Some batches likely were missed. Try increasing schedule capacity!",
                schedule_basis_timestamp, last_scheduled_timestamp);
        }
    }

    if (schedule_basis_timestamp < 1e-3) {
        // If there is no data, wait for more.
        g_elastic.log(WARNING, "No packets received. Cannot schedule batches yet.");
        return;
    }

    // Determine at which spill the scheduling starts.
    int coef;
    if (schedule_basis_timestamp < last_timestamp) {
        // The last spill (and possibly some more before it) have not been scheduled.
        coef = 0;
    } else {
        // Extrapolate the next spill that hasn't been observed/scheduled.
        coef = 1 + (schedule_basis_timestamp - last_timestamp) / learned_interval;
    }

    g_elastic.log(INFO, "Will schedule {} more batches. Starting after timestamp {} (extrapolation factor {}x).",
        n_batches_ahead_ - schedule.size(), schedule_basis_timestamp, coef);

    while (schedule.size() < n_batches_ahead_) {
        const Timestamp center_timestamp = last_timestamp + learned_interval * coef;

        Batch next {};
        next.created = true;
        next.start_time = center_timestamp - window_half_width_s_;
        next.end_time = center_timestamp + window_half_width_s_;

        schedule.push_back(std::move(next));
        ++coef;
    }
}

void SpillScheduler::workSpillServer()
{
    using namespace XmlRpc;

    g_elastic.log(INFO, "SpillServer up and running at port {}!", port_);
    spill_server_ = std::unique_ptr<XmlRpcServer>(new XmlRpcServer);
    predictor_ = std::make_shared<TriggerPredictor>(trigger_memory_size_, init_period_guess_);

    {
        Spill spill_method { spill_server_.get(), predictor_ };
        XmlRpc::setVerbosity(0);
        spill_server_->bindAndListen(port_);

        while (spill_server_running_) {
            spill_server_->work(1);
        }

        spill_server_->shutdown();
    }

    spill_server_.reset();
    g_elastic.log(INFO, "SpillServer signing off!");
}

std::string getSpillNameFromType(int type)
{
    std::string ret = "Undefined";
#define NAME_CHECK(arg) \
    if (type == k##arg) \
        ret = #arg;
    NAME_CHECK(NuMI);
    NAME_CHECK(BNB);
    NAME_CHECK(NuMItclk);
    NAME_CHECK(BNBtclk);
    NAME_CHECK(AccelOneHztclk);
    NAME_CHECK(Fake);
    NAME_CHECK(TestConnection);
    NAME_CHECK(SuperCycle);
    NAME_CHECK(NuMISampleTrig);
    NAME_CHECK(NuMIReset);
    NAME_CHECK(TBSpill);
    NAME_CHECK(TBTrig);
#undef NAME_CHECK
    return ret;
}

SpillScheduler::Spill::Spill(XmlRpc::XmlRpcServer* server, std::shared_ptr<TriggerPredictor> predictor)
    : XmlRpc::XmlRpcServerMethod("Spill", server)
    , predictor_ { predictor }
{
}

void convertNovaTimeToUnixTime(const std::uint64_t& inputNovaTime, struct timeval& outputUnixTime)
{
    // *time_t* of start of Nova epoch, 01-Jan-2010 00:00:00, UTC
    // This is the value subtracted from the UNIX time_t, timeval or timespec
    // seconds field. Since "novatime" does not have leap seconds and
    // "unixtime" does, leap seconds which happen _after_ the nova epoch will
    // need to be factored in to get the correct novatime which corresponds to
    // the time after the leap second.
    static constexpr std::uint32_t NOVA_EPOCH = 1262304000;

    // conversion factor (related to clock frequency)
    static constexpr std::uint64_t NOVA_TIME_FACTOR = 64000000;

    double doubleTime = (double)inputNovaTime / (double)NOVA_TIME_FACTOR;
    time_t time_sec = (time_t)doubleTime;

    outputUnixTime.tv_sec = NOVA_EPOCH + time_sec;
    outputUnixTime.tv_usec = (suseconds_t)((doubleTime - (double)time_sec) * 1000000);

    // test in chrono order

    if (outputUnixTime.tv_sec > 1341100799) // Jun 30 23:59:59 2012 UTC
        --outputUnixTime.tv_sec;

    if (outputUnixTime.tv_sec > 1435708799) // Jun 30 23:59:59 2015 UTC
        --outputUnixTime.tv_sec;

    if (outputUnixTime.tv_sec > 1483228799) // Dec 31 23:59:59 2016 UTC
        --outputUnixTime.tv_sec;
}

void SpillScheduler::Spill::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)
{
    static const std::string ok { "Ok" };
    static const std::string bad { "bad" };

    const int n_args = params.size();
    if (n_args != 2) {
        g_elastic.log(WARNING, "SpillServer received bad request (expected 2 arguments, got {})", n_args);
        result = bad;
        return;
    }

    struct timeval unixTime {
    };
    {
        // Don't trust XmlRpc with large int's.
        std::uint64_t ttime {};
        std::istringstream ss { static_cast<std::string>(params[0]) };
        ss >> ttime;
        convertNovaTimeToUnixTime(ttime, unixTime);
    }

    const int ttype { params[1] };

    // TODO: do something with the spill type

    // FIXME: decide on standardized time representation
    const double botchedTime = unixTime.tv_sec + 1e-6 * unixTime.tv_usec;

    // TODO: log this but do not clutter
    // g_elastic.log(INFO, "Received spill '{}' at timestamp {}.", getSpillNameFromType(ttype), botchedTime);

    predictor_->addTrigger(botchedTime);
    result = ok;
}
