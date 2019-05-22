/**
 * Merge-sorter - Algorithm to sort CLB event queues and merge them into a single queue
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#ifndef MERGE_SORTER_H
#define MERGE_SORTER_H

#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>

#include "clb_event.h"

class MergeSorter {
    enum class left_right {
        LEFT,
        RIGHT
    };

    using pair = std::pair<CLBEventQueue, CLBEventQueue>;
    using key_array = std::vector<CLBEventMultiQueue::key_type>;

    mutable std::vector<pair> buffer_;
    mutable CLBEventQueue mirror_;

    CLBEvent marker_;

    /**
     * Get internal buffer.
     *
     * \param  level         depth in internal buffer
     * \param  side          either of two of a kind (left/right)
     * \return               internal buffer
     */
    inline CLBEventQueue& get_buffer(const unsigned int level, const left_right side) const
    {
        if (level == 0) {
            return mirror_;
        } else if (level - 1 < buffer_.size()) {
            switch (side) {
            case left_right::LEFT:
                return buffer_[level - 1].first;
            case left_right::RIGHT:
                return buffer_[level - 1].second;
            }
        }

        throw std::runtime_error("merger::get_buffer() illegal argument value.");
    }

    /**
     * Fast merge of two data sets into one data set.
     *
     * It is assumed that the input data sets are sorted and terminated by an end marker.
     * In this, an end marker corresponds to a value that is larger than any physical value.
     * The output data set will be terminated by the same end marker.
     *
     * \param  first         first  data set
     * \param  second        second data set
     * \param  output        output data set
     */
    static void merge_to_buffer(const CLBEventQueue& first, const CLBEventQueue& second, CLBEventQueue& output);

    /**
     * Fast copy of data set.
     *
     * \param  input         input  data set
     * \param  output        output data set
     */
    static void copy_to_buffer(const CLBEventQueue& input, CLBEventQueue& output);

    /**
     * Recursive merge.
     *
     * \param  input         set of data sets
     * \param  begin         begin of key set
     * \param  end           end   of key set
     * \param  level         depth in internal buffer
     * \param  side          either of two of a kind (left/right)
     */
    void merge(CLBEventMultiQueue& input, key_array::const_iterator begin, key_array::const_iterator end, const unsigned int level = 0, const left_right side = left_right::LEFT) const;

public:
    explicit MergeSorter();
    virtual ~MergeSorter() = default;

    void merge(CLBEventMultiQueue& input, CLBEventQueue& output);
};

#endif /* MERGE_SORTER_H */