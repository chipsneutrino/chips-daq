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
        // get starting timestamp

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

SpillScheduler::SpillScheduler(int port)
    : port_{ port }
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
    // TODO
}

void SpillScheduler::workSpillServer()
{
    using namespace XmlRpc;

    g_elastic.log(INFO, "SpillServer up and running at port {}!", port_);
    spill_server_ = std::make_shared<XmlRpcServer>();

    {
        Spill spill_method{ spill_server_.get() };
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

SpillScheduler::Spill::Spill(XmlRpc::XmlRpcServer* server)
    : XmlRpc::XmlRpcServerMethod("Spill", server)
{
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

    std::uint64_t ttime{};
    {
        // Don't trust XmlRpc with large int's.
        std::istringstream ss{ static_cast<std::string>(params[0]) };
        ss >> ttime;
    }

    const int ttype{ params[1] };
    g_elastic.log(INFO, "SpillServer received spill of type '{}' ({}) at time: {}", getSpillNameFromType(ttype), ttype, ttime);

    // TODO: do something with the spill

    result = ok;
}
