#pragma once

#include <atomic>
#include <mutex>

#include <util/annotation_queues.h>
#include <util/pmt_hit_queues.h>

struct SpillDataSlot {
    std::mutex mutex; /// threads need to hold this mutex before accessing any field of the slot
    std::atomic_bool closed_for_writing; /// if true, writing is no longer enabled

    PMTMultiPlaneHitQueue opt_hit_queue; ///< Optical hits, grouped by plane numbers.
    AnnotationQueue opt_annotation_queue; ///< Annotations, all together

    explicit SpillDataSlot()
        : mutex {}
        , closed_for_writing { false }
        , opt_hit_queue {}
        , opt_annotation_queue {}
    {
    }

    // no copy semantics
    SpillDataSlot(const SpillDataSlot& other) = delete;
    SpillDataSlot& operator=(const SpillDataSlot& other) = delete;
};
