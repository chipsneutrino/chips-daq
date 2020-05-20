#pragma once

#include <cstdint>

#include <bbb/constants.h>

enum class BadgerboardRequestType : std::uint8_t {
    ConfigureHub = 1,
    ConfigureRun = 2,
    SetPowerState = 3,
    Reprogram = 4,
    BeginDataRun = 5,
    AbortDataRun = 6,
    Terminate = 7,
    Shutdown = 8,
    ResetConfiguration = 9
};

enum class BadgerboardResponseType : std::uint8_t {
    Ack = 1,
    Nak = 2,
};

struct __attribute__((packed)) BadgerboardResponse {
    std::uint8_t response_type;
    std::uint8_t request_type;
    std::uint32_t seq_number;
};

struct __attribute__((packed)) BadgerboardCommonHeader {
    std::uint8_t type;
    std::uint32_t seq_number;
};

struct __attribute__((packed)) BadgerboardConfigureHubDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint32_t config_size;
};

struct __attribute__((packed)) BadgerboardConfigureRunDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint32_t config_size;
};

struct __attribute__((packed)) BadgerboardSetPowerStateDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint16_t channel_bitfield;
};

struct __attribute__((packed)) BadgerboardReprogramDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint16_t channel_bitfield;
    std::uint32_t firmware_size;
};

struct __attribute__((packed)) BadgerboardBeginDataRunDatagramHeader {
    BadgerboardCommonHeader common;
};

struct __attribute__((packed)) BadgerboardAbortDataRunDatagramHeader {
    BadgerboardCommonHeader common;
};

struct __attribute__((packed)) BadgerboardTerminateDatagramHeader {
    BadgerboardCommonHeader common;
};

struct __attribute__((packed)) BadgerboardShutdownDatagramHeader {
    BadgerboardCommonHeader common;
};

struct __attribute__((packed)) BadgerboardResetConfigurationDatagramHeader {
    BadgerboardCommonHeader common;
};

struct __attribute__((packed)) BadgerboardHeartbeatDatagram {
    std::uint8_t zero;
    std::uint32_t seq_number;
    std::uint8_t hub_config;
    std::uint8_t run_config;
    std::uint8_t run;
    std::uint8_t channel_states[N_BADGERBOARD_CHANNELS];
    double last_updated_time;
    double pressure;
    double relative_humidity;
    double temperature;
    double acceleration[3];
};
