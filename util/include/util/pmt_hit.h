#pragma once

#include <util/timestamp.h>

/// A hit of a single PMT
struct PMTHit {
    std::uint32_t plane_number {}; /// uniquely identifies the plane
    std::uint8_t channel_number {}; /// which PMT in the plane was hit
    tai_timestamp timestamp {}; /// when the hit occurred
    std::uint8_t tot {}; /// time over threshold, corresponds with energy
    std::uint8_t adc0 {}; /// value of the ADC, only for Madison planes

    long double sort_key {}; /// precalculated compound timestamp, useful for sorting

    inline bool operator<(const PMTHit& other) const { return sort_key < other.sort_key; }
    inline bool operator>(const PMTHit& other) const { return sort_key > other.sort_key; }

    static constexpr std::uint8_t NO_ADC0 { 0 }; // TODO: pick an uncommon ADC value
};
