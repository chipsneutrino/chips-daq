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
