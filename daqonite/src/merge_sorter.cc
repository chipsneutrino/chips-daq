/**
 * Merge-sorter - Algorithm to sort CLB event queues and merge them into a single queue
 */

#include <functional>
#include <limits>

#include "merge_sorter.h"

MergeSorter::MergeSorter()
    :buffer_{},
     mirror_{},
     marker_{}
{
    marker_.SortKey = std::numeric_limits<decltype(CLBEvent::SortKey)>::max();
}

void MergeSorter::merge(CLBEventMultiQueue& input, CLBEventQueue& output)
{
    // configure depth of internal buffer: nearest power of two
    std::size_t N = 0;
    for (std::size_t i = input.size(); i != 0; i >>= 1) {
        ++N;
    }

    if (N != 0) {
        buffer_.resize(N - 1);

        key_array keys {};
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

void MergeSorter::copy_to_buffer(const CLBEventQueue& input, CLBEventQueue& output)
{
    std::size_t n = input.size();
    output.resize(n); // allocate memory

    auto in = input.cbegin();
    auto out = output.begin();

    for (; n != 0; --n, ++in, ++out) {
        *out = *in; // copy including end marker
    }
}

void MergeSorter::merge_to_buffer(const CLBEventQueue& first, const CLBEventQueue& second, CLBEventQueue& output)
{
    int n = (first.size() - 1 + // correct for end markers
        second.size() - 1);

    output.resize(n + 1); // allocate memory

    auto i = first.cbegin();
    auto j = second.cbegin();
    auto out = output.begin();

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

void MergeSorter::merge(CLBEventMultiQueue& input, key_array::const_iterator begin, key_array::const_iterator end, const unsigned int level, const left_right side) const
{
    const std::ptrdiff_t N = std::distance(begin, end);

    switch (N) {
    case 0:
        break;

    case 1:
        copy_to_buffer(input[begin[0]], get_buffer(level, side));
        break;

    case 2:
        merge_to_buffer(input[begin[0]], input[begin[1]], get_buffer(level, side));
        break;

    default:
        // recursion
        merge(input, begin,         begin + N / 2,  level + 1, left_right::LEFT);
        merge(input, begin + N / 2, end,            level + 1, left_right::RIGHT);

        // combination
        merge_to_buffer(
            get_buffer(level + 1, left_right::LEFT), 
            get_buffer(level + 1, left_right::RIGHT),
            get_buffer(level, side)
            );
        break;
    }
}
