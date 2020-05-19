#pragma once

#include <mutex>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

/// Enum for describing the different logging severity levels
enum Severity {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

static std::string severityToString(Severity level);
static Severity severityFromString(const std::string& str);

class Logging;

class LoggingMultiplexer {
public:
    LoggingMultiplexer(const LoggingMultiplexer&) = delete;
    void operator=(const LoggingMultiplexer&) = delete;

    static void init();

    void log(Severity level, const std::string& unit_name, const std::string& message);
    void logWithoutES(Severity level, const std::string& unit_name, const std::string& message);

private:
    explicit LoggingMultiplexer();

    void configure();

    static LoggingMultiplexer& getInstance()
    {
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        static LoggingMultiplexer instance {};
        return instance;
    }

    bool configured_;

    bool stderr_enabled_;
    Severity stderr_min_severity_;
    std::mutex stderr_mutex_;
    void logToStderr(Severity level, const std::string& unit_name, const std::string& message);

    bool es_enabled_;
    Severity es_min_severity_;
    void logToES(Severity level, const std::string& unit_name, const std::string& message);

    friend class Logging;
};

class Logging {
public:
    Logging();
    virtual ~Logging() = default;

protected:
    template <typename S, typename... Args>
    inline void setUnitName(const S& format_str, const Args&... args)
    {
        setUnitName(fmt::format(format_str, args...));
    }

    /// Nice pretty-print formatting using fmt for logging
    template <typename S, typename... Args>
    inline void log(Severity level, const S& format_str, const Args&... args)
    {
        LoggingMultiplexer::getInstance().log(level, unit_name_, fmt::format(format_str, args...));
    }
    template <typename S, typename... Args>
    inline void logWithoutES(Severity level, const S& format_str, const Args&... args)
    {
        LoggingMultiplexer::getInstance().logWithoutES(level, unit_name_, fmt::format(format_str, args...));
    }

private:
    std::string unit_name_;

    void setUnitName(std::string&& unit_name);
};