#pragma once

#include <array>
#include <cstdint>

#include <bbb/constants.h>

enum class BadgerboardConnectionState : std::uint8_t {
    Online,
    Connecting,
    Offline
};

enum class BadgerboardChannelState : std::uint8_t {
    PoweredOff = 1,
    PoweredOn = 2,
    Started = 3,
    Reprogrammed = 4,
    FailedPowerOff = 5,
    FailedPowerOn = 6,
    FailedStart = 7,
    FailedReprogram = 8,
};

enum class BadgerboardDataRunState : std::uint8_t {
    Stopped = 1,
    Beginning = 2,
    InProgress = 3,
    Completed = 4,
    Aborting = 5,
    Aborted = 6,
    Failed = 7,
};

enum class BadgerboardConfigState : std::uint8_t {
    NotConfigured = 1,
    Configured = 2,
};

struct BadgerboardState {
    BadgerboardConnectionState connection;

    BadgerboardConfigState hubConfig;
    BadgerboardConfigState runConfig;

    BadgerboardDataRunState run;

    std::array<BadgerboardChannelState, N_BADGERBOARD_CHANNELS> channels;
};
