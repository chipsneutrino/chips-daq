#pragma once

#include <stdexcept>
#include <string>

struct Config {
    Config() = delete;

    static std::string getString(const char* key);

private:
    static std::string normalizeKey(const char* key);
    static std::string getEnvironmentValue(const char* key);
};

class ConfigException : public std::runtime_error {
public:
    explicit ConfigException(const std::string& what_arg)
        : runtime_error { what_arg }
    {
    }
};