/**
 * DataRunFile - Wrapper class for file output
 * 
 * This class writes the output of data taking to output
 * files on per-run basis.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <memory>

#include <TFile.h>
#include <TTree.h>

#include <spill_scheduling/spill.h>
#include <util/annotation.h>
#include <util/pmt_hit.h>

class PMTHitQueue;
class DataRun;

class DataRunFile {
public:
    explicit DataRunFile(std::string path);
    virtual ~DataRunFile() = default;

    // no copy semantics
    DataRunFile(const DataRunFile& other) = delete;
    DataRunFile&& operator=(const DataRunFile& other) = delete;

    // no move semantics
    DataRunFile(DataRunFile&& other) = delete;
    DataRunFile&& operator=(DataRunFile&& other) = delete;

    void writeRunParametersAtStart(const std::shared_ptr<DataRun>& run) const;
    void writeRunParametersAtEnd(const std::shared_ptr<DataRun>& run) const;

    /// Save sorted queue of hits to the file.
    void writeSpill(const SpillPtr spill, const PMTHitQueue& merged_hits) const;

    /// Is the file open?
    bool isOpen() const;

    /// Make sure data written so far is propagated to disk.
    void flush();

    /// Close the file.
    void close();

private:
    std::unique_ptr<TFile> file_; ///< ROOT output TFile

    // Run parameters tree
    TTree* run_params_;
    mutable ULong64_t run_number_;
    mutable UChar_t run_type_;
    mutable utc_timestamp run_time_started_;
    mutable utc_timestamp run_time_stopped_;
    void createRunParams();

    // Run parameters tree
    TTree* spills_;
    mutable ULong64_t spill_number_;
    mutable tai_timestamp spill_time_started_;
    mutable tai_timestamp spill_time_stopped_;
    mutable ULong64_t spill_opt_hits_begin_;
    mutable ULong64_t spill_opt_hits_end_;
    mutable ULong64_t spill_opt_annotations_begin_;
    mutable ULong64_t spill_opt_annotations_end_;
    void createSpills();

    // Optical hits tree
    TTree* opt_hits_;
    mutable PMTHit hit_;
    void createOptHits();

    // Optical annotations tree
    TTree* opt_annotations_;
    mutable Annotation annotation_;
    void createOptAnnotations();
};
