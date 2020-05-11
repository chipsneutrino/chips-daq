#pragma once

#include <string>

#include <fmt/format.h>

/// Enum for describing the different logging severity levels
enum severity {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logging {
public:
    Logging();
    virtual ~Logging() = default;

protected:
    void setUnitName(std::string&& unit_name);

    /// Nice pretty-print formatting using fmt for logging
    template <typename S, typename... Args>
    inline void log(severity level, const S& format_str, const Args&... args)
    {
        log(level, fmt::format(format_str, args...));
    }

private:
    std::string unit_name_;

    void log(severity level, std::string&& message);
};