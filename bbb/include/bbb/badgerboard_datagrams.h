#pragma once

#include <cstdint>

enum class BadgerboardRequestType : std::uint8_t {
    ConfigureHub = 1,
    ConfigureRun = 2,
    SetPowerState = 3,
    Reprogram = 4,
    BeginDataRun = 5,
    AbortDataRun = 6,
    Terminate = 7,
    Shutdown = 8
};

enum class BadgerboardResponseType : std::uint8_t {
    Ack = 1,
    Nak = 2,
};

struct BadgerboardResponse {
    BadgerboardResponseType response_type;
    BadgerboardRequestType request_type;
};

struct BadgerboardCommonHeader {
    BadgerboardRequestType type;
};

struct BadgerboardConfigureHubDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint32_t config_size;
};

struct BadgerboardConfigureRunDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint32_t config_size;
};

struct BadgerboardSetPowerStateDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint16_t channel_bitfield;
};

struct BadgerboardReprogramDatagramHeader {
    BadgerboardCommonHeader common;
    std::uint16_t channel_bitfield;
    std::uint32_t firmware_size;
};

struct BadgerboardBeginDataRunDatagramHeader {
    BadgerboardCommonHeader common;
};

struct BadgerboardAbortDataRunDatagramHeader {
    BadgerboardCommonHeader common;
};

struct BadgerboardTerminateDatagramHeader {
    BadgerboardCommonHeader common;
};

struct BadgerboardShutdownDatagramHeader {
    BadgerboardCommonHeader common;
};

static constexpr std::size_t BADGERBOARD_HEADER_MAX_SIZE = 65536;
