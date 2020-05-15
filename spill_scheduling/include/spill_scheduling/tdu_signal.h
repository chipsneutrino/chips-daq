#pragma once

#include <spill_scheduling/tdu_signal_type.h>
#include <util/timestamp.h>

struct TDUSignal {
    TDUSignalType type {};
    tai_timestamp time {};
    std::uint64_t nova_time {};
};
