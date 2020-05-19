#include <util/config.h>

#include "data_run_file.h"
#include "data_run_serialiser.h"
#include "merge_sorter.h"

DataRunSerialiser::DataRunSerialiser(const std::shared_ptr<DataRun>& data_run)
    : Logging {}
    , AsyncComponent {}
    , data_run_ { data_run }
    , waiting_spills_ { g_config.lookupU32("max_serialiser_queue_size") }
{
    setUnitName("DataRunSerialiser");
}

DataRunSerialiser::~DataRunSerialiser()
{
    waiting_spills_.consume_all([this](SpillPtr spill) {
        log(WARNING, "Spill {} found in queue after serialiser thread terminated. Discarding data.",
            spill->spill_number);
        delete spill;
    });
}

bool DataRunSerialiser::serialiseSpill(SpillPtr spill)
{
    return waiting_spills_.push(spill);
}

void DataRunSerialiser::run()
{
    log(DEBUG, "Output thread up and running");

    // Open output.
    const std::string out_file_path { data_run_->getOutputFilePath() };
    log(INFO, "Run {} will be saved at: '{}'", data_run_->logDescription(), out_file_path);

    DataRunFile out_file { out_file_path };
    if (!out_file.isOpen()) {
        log(ERROR, "Error opening file for writing: '{}'", out_file_path);
        return;
    }

    out_file.writeRunParametersAtStart(data_run_);
    out_file.flush();

    MergeSorter sorter {};
    PMTHitQueue out_queue {};
    for (;;) {
        // Obtain a spill to process.
        bool have_spill { false };
        SpillPtr current_spill {};

        if (waiting_spills_.pop(current_spill)) {
            // If there's something to process, dequeue.
            have_spill = true;
        } else if (!running_) {
            // If not, and we're done, stop.
            break;
        } else {
            // Sleep.
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (!have_spill) {
            continue;
        }

        // At this point, we always have a valid spill.

        // Consolidate multi-queue by moving it into a single instance.
        PMTMultiPlaneHitQueue events {};
        for (std::size_t data_slot_idx = 0; data_slot_idx < current_spill->n_data_slots; ++data_slot_idx) {
            SpillDataSlot& slot { current_spill->data_slots[data_slot_idx] };
            PMTMultiPlaneHitQueue& slot_multiqueue { slot.opt_hit_queue };
            for (auto it = slot_multiqueue.begin(); it != slot_multiqueue.end(); ++it) {
                events.emplace(it->first, std::move(it->second));
            }
        }

        // TODO: consolidate annotation queues in the same way

        log(INFO, "Processing spill {} (from {} planes)",
            current_spill->spill_number, events.size());

        // Calculate complete timestamps & make sure sequence is sorted
        std::size_t n_hits { 0 };
        for (auto& key_value : events) {
            PMTHitQueue& queue { key_value.second };
            n_hits += queue.size();

            // TODO: report disorder measure to backend
            const std::size_t n_swaps = insertSort(queue);
            log(INFO, "Plane {} ({} hits) required {} swaps to achieve time ordering", key_value.first, queue.size(), n_swaps);
        }

        out_queue.clear();

        if (n_hits > 0) {
            // Merge-sort hits.
            log(INFO, "Merge-sorting {} hits", n_hits);
            sorter.merge(events, out_queue);
        }

        // Write sorted events out.
        out_file.writeSpill(current_spill, out_queue);
        out_file.flush();
        out_queue.clear();

        log(INFO, "Spill {} done and written", current_spill->spill_number);
        delete current_spill;
    }

    out_file.writeRunParametersAtEnd(data_run_);
    out_file.flush();

    // Close output file.
    out_file.close();

    log(DEBUG, "Output thread signing off");
}

std::size_t DataRunSerialiser::insertSort(PMTHitQueue& queue) noexcept
{
    // Just your conventional O(n^2) insert-sort implementation.
    // Here utilized because insert-sort is actually O(n+k*n) for k-sorted sequences.
    // Since event queue should already be sorted, insert-sort will frequently only scan it in O(n).

    std::size_t n_swaps { 0 };
    for (std::size_t i = 1; i < queue.size(); ++i) {
        for (std::size_t j = i; j > 0 && queue[j - 1] > queue[j]; --j) {
            std::swap(queue[j], queue[j - 1]);
            ++n_swaps;
        }
    }

    return n_swaps;
}