#include <ctime>
#include <sstream>

#include "xml_rpc_spill_method.h"
#include "nova_spill_signal_type.h"

XMLRPCSpillMethod::XMLRPCSpillMethod(XmlRpc::XmlRpcServer* server, std::shared_ptr<TriggerPredictor> predictor)
    : XmlRpc::XmlRpcServerMethod { "Spill", server }
    , Logging {}
    , predictor_ { predictor }
{
    setUnitName("XMLRPCSpillMethod");
}

// FIXME: we need a correct and reliable way to convert NOvA time to TAI
static void convertNovaTimeToUnixTime(const std::uint64_t& inputNovaTime, struct timeval& outputUnixTime)
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

void XMLRPCSpillMethod::execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)
{
    static const std::string ok { "Ok" };
    static const std::string bad { "bad" };

    const int n_args = params.size();
    if (n_args != 2) {
        log(WARNING, "Received bad request (expected 2 arguments, got {})", n_args);
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
    // const double botchedTime = unixTime.tv_sec + 1e-6 * unixTime.tv_usec;

    // TODO: log this but do not clutter
    // log(INFO, "Received spill '{}' at timestamp {}.", getSpillNameFromType(ttype), botchedTime);

    // FIXME: predictor_->addTrigger(botchedTime);
    result = ok;
}
