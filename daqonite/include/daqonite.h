#pragma once

#include <string>

struct daqonite_settings {
    // Default settings
    bool collect_clb_data = true;
    bool collect_bbb_data = false;

    std::string state_bus_url {};
    std::string control_bus_url {};
    std::string data_path {};
};

const daqonite_settings& get_settings();
