#pragma once

#include <string>

#include <fmt/format.h>
#include <libconfig.h++>

class Config {
public:
    explicit Config();

    void init(const std::string& process_name);

    bool lookupBool(const char* path) const { return lookup<bool>(path); }
    double lookupDouble(const char* path) const { return lookup<double>(path); }

    std::int32_t lookupI32(const char* path) const { return lookup<std::int32_t>(path); }
    std::int64_t lookupI64(const char* path) const { return lookup<std::int64_t>(path); }

    std::uint32_t lookupU32(const char* path) const { return lookup<std::uint32_t>(path); }
    std::uint64_t lookupU64(const char* path) const { return lookup<std::uint64_t>(path); }

    std::string lookupString(const char* path) const { return lookup<const char*>(path); }

private:
    std::string cfg_file_path_;
    bool loaded_;
    libconfig::Config cfg_;

    static std::string determineConfigDirectory();

    template <typename ValueType>
    ValueType lookup(const char* path) const
    {
        if (!loaded_) {
            throw std::runtime_error { fmt::format("Attempted to read key '{}' before configuration was loaded.", path) };
        }

        try {
            return cfg_.lookup(path);
        } catch (const libconfig::SettingNotFoundException& ex) {
            throw std::runtime_error { fmt::format("Missing configuration key '{}' in {}", ex.getPath(), cfg_file_path_) };
        }
    }
};

extern Config g_config; ///< Global instance of this class