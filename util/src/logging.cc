#include <util/config.h>

#include "elastic_interface.h"
#include "logging.h"

std::string severityToString(Severity level)
{
    switch (level) {
    case Severity::TRACE:
        return "TRACE";
    case Severity::DEBUG:
        return "DEBUG";
    case Severity::INFO:
        return "INFO";
    case Severity::WARNING:
        return "WARNING";
    case Severity::ERROR:
        return "ERROR";
    case Severity::FATAL:
        return "FATAL";
    default:
        return "unknown";
    }
}

Severity severityFromString(const std::string& str)
{
    std::string normalised { str };
    std::transform(normalised.begin(), normalised.end(), normalised.begin(),
        [](unsigned char c) { return std::toupper(c); });

#define CMP_SEVERITY(sev)     \
    if (normalised == #sev) { \
        return Severity::sev; \
    }
    CMP_SEVERITY(TRACE);
    CMP_SEVERITY(DEBUG);
    CMP_SEVERITY(INFO);
    CMP_SEVERITY(WARNING);
    CMP_SEVERITY(ERROR);
    CMP_SEVERITY(FATAL);
#undef CMP_SEVERITY

    throw std::runtime_error { fmt::format("Unknown severity: '{}'", str) };
}

Logging::Logging()
    : unit_name_ { "unknown_unit" }
{
}

void Logging::setUnitName(std::string&& unit_name)
{
    unit_name_ = unit_name;
}

LoggingMultiplexer::LoggingMultiplexer()
    : configured_ { false }
    , stderr_enabled_ { false }
    , stderr_min_severity_ {}
    , stderr_mutex_ {}
    , es_enabled_ { false }
    , es_min_severity_ {}
{
}

void LoggingMultiplexer::init()
{
    LoggingMultiplexer::getInstance().configure();
}

void LoggingMultiplexer::configure()
{
    stderr_enabled_ = g_config.lookupBool("logging.stderr.enabled");
    stderr_min_severity_ = severityFromString(g_config.lookupString("logging.stderr.min_severity"));

    es_enabled_ = g_config.lookupBool("logging.elastic_search.enabled");
    es_min_severity_ = severityFromString(g_config.lookupString("logging.elastic_search.min_severity"));

    configured_ = true;
}

void LoggingMultiplexer::log(Severity level, const std::string& unit_name, const std::string& message)
{
    if (!configured_) {
        return;
    }

    if (stderr_enabled_ && level >= stderr_min_severity_) {
        logToStderr(level, unit_name, message);
    }

    if (es_enabled_ && level >= es_min_severity_) {
        logToES(level, unit_name, message);
    }
}

void LoggingMultiplexer::logWithoutES(Severity level, const std::string& unit_name, const std::string& message)
{
    if (!configured_) {
        return;
    }

    if (stderr_enabled_ && level >= stderr_min_severity_) {
        logToStderr(level, unit_name, message);
    }
}

void LoggingMultiplexer::logToStderr(Severity level, const std::string& unit_name, const std::string& message)
{
    std::lock_guard<std::mutex> l { stderr_mutex_ };
    fmt::print(stderr, "{} ({}):\t {}\n", severityToString(level), unit_name, message);
}

void LoggingMultiplexer::logToES(Severity level, const std::string& unit_name, const std::string& message)
{
    g_elastic.log(level, unit_name, message);
}