/**
 * RunFile - Wrapper class for file output
 * 
 * This class writes the output of data taking to output
 * files on per-run basis.
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <memory>

#include <TFile.h>
#include <TTree.h>

#include <util/pmt_hit.h>

class PMTHitQueue;

class RunFile {
public:
    explicit RunFile(std::string path);
    virtual ~RunFile() = default;

    // No copy- / move-semantics, sorry!
    RunFile(const RunFile& other) = delete;
    RunFile(RunFile&& other) = delete;
    RunFile&& operator=(const RunFile& other) = delete;
    RunFile&& operator=(RunFile&& other) = delete;

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

    // Optical hits tree
    TTree* opt_hits_;
    mutable PMTHit hit_;
    void createOptHits();
};
