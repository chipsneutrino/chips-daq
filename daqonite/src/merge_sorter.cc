#include <functional>
#include <limits>

#include "merge_sorter.h"

MergeSorter::MergeSorter()
    : buffer_ {}
    , mirror_ {}
    , marker_ {}
{
    marker_.sort_key = std::numeric_limits<decltype(PMTHit::sort_key)>::max();
}

void MergeSorter::merge(PMTMultiPlaneHitQueue& input, PMTHitQueue& output)
{
    // configure depth of internal buffer: nearest power of two
    std::size_t N { 0 };
    for (std::size_t i = input.size(); i != 0; i >>= 1) {
        ++N;
    }

    if (N != 0) {
        buffer_.resize(N - 1);

        KeyArray keys {};
        keys.reserve(input.size());
        for (auto& key_value : input) {
            keys.push_back(key_value.first);

            // insert marker at the end of each queue
            key_value.second.emplace_back(std::cref(marker_));
        }

        // merge data
        mirror_.swap(output);
        merge(input, keys.cbegin(), keys.cend());
        mirror_.erase(mirror_.begin() + mirror_.size() - 1); // erase marker
        mirror_.swap(output);

        // TODO: erase added markers in input sequences?
    }
}

void MergeSorter::copyToBuffer(const PMTHitQueue& input, PMTHitQueue& output)
{
    std::size_t n = input.size();
    output.resize(n); // allocate memory

    auto in = input.cbegin();
    auto out = output.begin();

    for (; n != 0; --n, ++in, ++out) {
        *out = *in; // copy including end marker
    }
}

void MergeSorter::mergeToBuffer(const PMTHitQueue& first, const PMTHitQueue& second, PMTHitQueue& output)
{
    int n = (first.size() - 1 + // correct for end markers
        second.size() - 1);

    output.resize(n + 1); // allocate memory

    auto i { first.cbegin() };
    auto j { second.cbegin() };
    auto out { output.begin() };

    for (; n != 0; --n, ++out) {
        if (*i < *j) {
            *out = *i;
            ++i;
        } else {
            *out = *j;
            ++j;
        }
    }

    *out = *i; // copy end marker
}

void MergeSorter::merge(PMTMultiPlaneHitQueue& input, KeyArray::const_iterator begin, KeyArray::const_iterator end, const unsigned int level, const LeftRight side) const
{
    const std::ptrdiff_t N { std::distance(begin, end) };

    switch (N) {
    case 0:
        break;

    case 1:
        copyToBuffer(input[begin[0]], getBuffer(level, side));
        break;

    case 2:
        mergeToBuffer(input[begin[0]], input[begin[1]], getBuffer(level, side));
        break;

    default:
        // recursion
        merge(input, begin, begin + N / 2, level + 1, LeftRight::LEFT);
        merge(input, begin + N / 2, end, level + 1, LeftRight::RIGHT);

        // combination
        mergeToBuffer(
            getBuffer(level + 1, LeftRight::LEFT),
            getBuffer(level + 1, LeftRight::RIGHT),
            getBuffer(level, side));
        break;
    }
}
