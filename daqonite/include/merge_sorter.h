/**
 * Merge-sorter - A recursive tree-based algorithm to combine multiple time-sorted
 * PMT hit queues (usually one for every plane) into a single time-sorted PMT hit queue.
 * 
 * Shamelessly inspired by a similar implementation by KM3NeT.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>

#include <util/pmt_hit_queues.h>

class MergeSorter {
    enum class LeftRight {
        LEFT,
        RIGHT
    };

    using PMTHitQueuePair = std::pair<PMTHitQueue, PMTHitQueue>;
    using KeyArray = std::vector<PMTMultiPlaneHitQueue::key_type>;

    mutable std::vector<PMTHitQueuePair> buffer_;
    mutable PMTHitQueue mirror_;

    PMTHit marker_;

    /**
     * Get internal buffer.
     *
     * \param  level         depth in internal buffer
     * \param  side          either of two of a kind (left/right)
     * \return               internal buffer
     */
    inline PMTHitQueue& getBuffer(const unsigned int level, const LeftRight side) const
    {
        if (level == 0) {
            return mirror_;
        } else if (level - 1 < buffer_.size()) {
            switch (side) {
            case LeftRight::LEFT:
                return buffer_[level - 1].first;
            case LeftRight::RIGHT:
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
    static void mergeToBuffer(const PMTHitQueue& first, const PMTHitQueue& second, PMTHitQueue& output);

    /**
     * Fast copy of data set.
     *
     * \param  input         input  data set
     * \param  output        output data set
     */
    static void copyToBuffer(const PMTHitQueue& input, PMTHitQueue& output);

    /**
     * Recursive merge.
     *
     * \param  input         set of data sets
     * \param  begin         begin of key set
     * \param  end           end   of key set
     * \param  level         depth in internal buffer
     * \param  side          either of two of a kind (left/right)
     */
    void merge(PMTMultiPlaneHitQueue& input, KeyArray::const_iterator begin, KeyArray::const_iterator end,
        const unsigned int level = 0, const LeftRight side = LeftRight::LEFT) const;

public:
    explicit MergeSorter();
    virtual ~MergeSorter() = default;

    void merge(PMTMultiPlaneHitQueue& input, PMTHitQueue& output);
};
