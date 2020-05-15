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
    void writeHitQueue(const PMTHitQueue& queue) const;

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

    // Optical hits tree
    TTree* opt_hits_;
    mutable PMTHit hit_;
    void createOptHits();
};
