#pragma once

#include <cstdint>

#include <util/timestamp.h>

enum class AnnotationType : std::uint8_t {
    DROPPED_CHANNEL = 1
};

struct Annotation {
    AnnotationType type;
    std::uint32_t plane_number;
    std::uint8_t channel_number;
    tai_timestamp time_start;
    tai_timestamp time_end;
};
