#include "data_run_serialiser.h"
#include "merge_sorter.h"
#include "run_file.h"

DataRunSerialiser::DataRunSerialiser(const std::shared_ptr<DataRun>& data_run)
    : Logging {}
    , AsyncComponent {}
    , data_run_ { data_run }
    , waiting_spills_ {}
{
    setUnitName("DataRunSerialiser");
}

void DataRunSerialiser::serialiseSpill(SpillPtr spill)
{
    waiting_spills_.push(spill);
}

void DataRunSerialiser::run()
{
    log(INFO, "Output thread up and running");

    // Open output.
    const std::string out_file_path { data_run_->getOutputFilePath() };
    log(DEBUG, "Run {} will be saved in at: '{}'", data_run_->logDescription(), out_file_path);

    RunFile out_file { out_file_path };
    if (!out_file.isOpen()) {
        log(ERROR, "Error opening file for writing: '{}'", out_file_path);
        return;
    }

    MergeSorter sorter {};
    PMTHitQueue out_queue {};
    for (;;) {
        // Obtain a spill to process.
        bool have_spill = false;
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
            PMTMultiPlaneHitQueue& slot_multiqueue { current_spill->opt_hit_queues[data_slot_idx] };
            for (auto it = slot_multiqueue.begin(); it != slot_multiqueue.end(); ++it) {
                events.emplace(it->first, std::move(it->second));
            }
        }

        log(INFO, "Processing spill {} (from {} planes)", current_spill->idx, events.size());

        // Calculate complete timestamps & make sure sequence is sorted
        std::size_t n_hits { 0 };
        for (auto& key_value : events) {
            PMTHitQueue& queue { key_value.second };
            n_hits += queue.size();

            // TODO: report disorder measure to backend
            const std::size_t n_swaps = insertSort(queue);
            log(INFO, "POM #{} ({} hits) required {} swaps to achieve time ordering", key_value.first, queue.size(), n_swaps);
        }

        out_queue.clear();

        if (n_hits > 0) {
            // Merge-sort CLB events.
            log(INFO, "Merge-sorting {} hits", n_hits);
            sorter.merge(events, out_queue);
        }

        // Write sorted events out.
        out_file.writeHitQueue(out_queue);
        out_file.flush();
        out_queue.clear();

        log(INFO, "Spill {} done and written", current_spill->idx);
        delete current_spill;
    }

    // Close output file.
    out_file.close();

    log(INFO, "Output thread signing off");
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