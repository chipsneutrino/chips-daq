#pragma once

namespace control_msg {
using disc_type = unsigned char;

struct daq {
    static constexpr char const* url = "ipc:///tmp/daqonite.ipc";
    disc_type disc;

    /// Start a new data run
    struct start_run {
        static constexpr disc_type disc_value = 0;
        enum class run_type {
            data_normal = 1,
            calibration,
            test_normal,
            test_daq
        };

        run_type which;
    };

    /// Stop current run
    struct stop_run {
        static constexpr disc_type disc_value = 1;
    };

    /// Exit, possibly stopping current run
    struct exit {
        static constexpr disc_type disc_value = 2;
    };

    union {
        start_run p_start_run;
        stop_run p_stop_run;
        exit p_exit;
    } payload;
};
}
