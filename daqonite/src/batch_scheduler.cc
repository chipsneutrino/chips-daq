#include <functional>
#include <limits>
#include <sstream>

#include "batch_scheduler.h"
#include "elastic_interface.h"

void InfiniteScheduler::updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp)
{
    if (schedule.empty()) {
        Batch single_window{};

        single_window.started = false;
        single_window.last_updated_time = std::chrono::steady_clock::now();

        single_window.clb_opt_data = new CLBEventMultiQueue();

        single_window.start_time = 0;
        single_window.end_time = std::numeric_limits<decltype(Batch::end_time)>::max();

        schedule.push_back(std::move(single_window));
    }
}

RegularScheduler::RegularScheduler(std::size_t n_batches_ahead, std::chrono::milliseconds batch_duration)
    : n_batches_ahead_{ n_batches_ahead }
    , batch_duration_s_{ batch_duration.count() / 1000. }
{
}

void RegularScheduler::updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp)
{
    if (last_approx_timestamp == 0) {
        // If there is no data, wait for more.
        return;
    }

    // Initialize the very first batch.
    if (schedule.empty()) {
        Batch first{};

        first.started = false;
        first.last_updated_time = std::chrono::steady_clock::now();

        first.clb_opt_data = new CLBEventMultiQueue();

        first.start_time = last_approx_timestamp;
        first.end_time = first.start_time + batch_duration_s_;

        schedule.push_back(std::move(first));
    }

    // At this point, there's always a previous batch.
    while (schedule.size() < n_batches_ahead_) {
        Batch next{};

        next.started = false;
        next.last_updated_time = std::chrono::steady_clock::now();

        next.clb_opt_data = new CLBEventMultiQueue();

        next.start_time = schedule.back().end_time;
        next.end_time = next.start_time + batch_duration_s_;

        schedule.push_back(std::move(next));
    }
}

SpillScheduler::SpillScheduler(int port, std::size_t trigger_memory_size, double init_period_guess, std::size_t n_batches_ahead, double time_window_radius)
    : port_{ port }
    , trigger_memory_size_{ trigger_memory_size }
    , init_period_guess_{ init_period_guess }
    , n_batches_ahead_{ n_batches_ahead }
    , time_window_radius_{ time_window_radius }
    , spill_server_thread_{}
    , spill_server_{}
{
    spill_server_thread_ = std::make_shared<std::thread>(std::bind(&SpillScheduler::workSpillServer, this));
}

void SpillScheduler::join()
{
    if (spill_server_thread_ && spill_server_thread_->joinable()) {
        spill_server_->exit();
        spill_server_thread_->join();
    }

    spill_server_thread_.reset();
}

void SpillScheduler::updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp)
{
    const double learned_interval = predictor_->learnedInterval();

    // TODO: set up intervals based on the predicted triggers
    // This can be done once:
    //   (1) we know what time representation we'll use
    //   (2) we know how trigger predictions and signals will be matched
    //   (3) white rabbits start giving sensible timestamps
}

void SpillScheduler::workSpillServer()
{
    using namespace XmlRpc;

    g_elastic.log(INFO, "SpillServer up and running at port {}!", port_);
    spill_server_ = std::make_shared<XmlRpcServer>();
    predictor_ = std::make_shared<TriggerPredictor>(trigger_memory_size_, init_period_guess_);

    {
        Spill spill_method{ spill_server_.get(), predictor_ };
        XmlRpc::setVerbosity(0);
        spill_server_->bindAndListen(port_);

        // This will block thread until exit() is called.
        spill_server_->work(-1.0);
        spill_server_->shutdown();
    }

    spill_server_.reset();
    g_elastic.log(INFO, "SpillServer signing off!");
}

std::string SpillScheduler::getSpillNameFromType(int type)
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
    , predictor_{ predictor }
{
}

void SpillScheduler::Spill::convertNovaTimeToUnixTime(const std::uint64_t& inputNovaTime, struct timeval& outputUnixTime)
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
    static const std::string ok{ "Ok" };
    static const std::string bad{ "bad" };

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
        std::uint64_t ttime{};
        std::istringstream ss{ static_cast<std::string>(params[0]) };
        ss >> ttime;
        convertNovaTimeToUnixTime(ttime, unixTime);
    }

    const int ttype{ params[1] };

    // TODO: do something with the spill type

    // FIXME: decide on standardized time representation
    const double botchedTime = unixTime.tv_sec + 1e-6 * unixTime.tv_usec;
    predictor_->addTrigger(botchedTime);
    result = ok;
}

SpillScheduler::TriggerPredictor::TriggerPredictor(std::size_t n_last, double init_interval)
    : observed_{}
    , sorted_{}
    , last_timestamp_{ -1 }
    , next_{ 0 }
{
    observed_.resize(n_last);
    sorted_.resize(n_last);
    std::fill(observed_.begin(), observed_.end(), init_interval);
}

void SpillScheduler::TriggerPredictor::addTrigger(double timestamp)
{
    if (last_timestamp_ < 0) {
        last_timestamp_ = timestamp;
        return;
    }

    observed_[next_] = timestamp - last_timestamp_;
    next_ = (next_ + 1) % observed_.size();
    last_timestamp_ = timestamp;

    std::copy(observed_.begin(), observed_.end(), sorted_.begin());
    std::sort(sorted_.begin(), sorted_.end());
    learned_interval_ = sorted_[sorted_.size() / 2];
}

double SpillScheduler::TriggerPredictor::learnedInterval() const
{
    return learned_interval_;
}
