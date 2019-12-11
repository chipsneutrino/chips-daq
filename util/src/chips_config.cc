#include <algorithm>
#include <cctype>
#include <cstdlib>

#include <fmt/core.h>

#include "chips_config.h"

std::string Config::getString(const char* key)
{
    return getEnvironmentValue(key);
}

std::string Config::normalizeKey(const char* key)
{
    std::string normalized { key };
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::toupper);
    return fmt::format("CHIPS_{}", normalized);
}

std::string Config::getEnvironmentValue(const char* key)
{
    const std::string normalized_key { normalizeKey(key) };
    const char* value = std::getenv(normalized_key.c_str());

    if (value == nullptr) {
        throw ConfigException {
            fmt::format("Cannot read configuration key '{}'. Is the CHIPS environment set up properly?",
                normalized_key)
        };
    }

    return { value };
}