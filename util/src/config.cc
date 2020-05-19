#include <util/config.h> // using library import here to avoid name clash

Config g_config {}; ///< Global instance of this class

Config::Config()
    : loaded_ { false }
    , cfg_file_path_ {}
    , cfg_ {}
{
}

void Config::init(const std::string& process_name)
{
    // TODO: exceptions may be thrown here, handle them?
    const std::string cfg_dir { Config::determineConfigDirectory() };
    cfg_file_path_ = fmt::format("{}/{}.cfg", cfg_dir, process_name); // TODO: use std::filesystem

    try {
        cfg_.setIncludeDir(cfg_dir.c_str());
        cfg_.readFile(cfg_file_path_.c_str());
        loaded_ = true;
    } catch (const libconfig::FileIOException& ex) {
        throw std::runtime_error { fmt::format("I/O error while reading file: {} - {}", cfg_file_path_, ex.what()) };
    } catch (const libconfig::ParseException& ex) {
        throw std::runtime_error { fmt::format("Configuration parse error at {}:{} - {}", ex.getFile(), ex.getLine(), ex.getError()) };
    }
}

std::string Config::determineConfigDirectory()
{
    static const std::string env_var_name { "CHIPS_DIST_CONFIG_PATH" };
    const char* value = std::getenv(env_var_name.c_str());

    if (value == nullptr || std::strlen(value) == 0) {
        throw std::runtime_error {
            fmt::format("Cannot read environment variable '{}'. Is the CHIPS environment set up properly?",
                env_var_name)
        };
    }

    return value;
}
