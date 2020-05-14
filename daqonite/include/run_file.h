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

#include <cstdint>

#include <TFile.h>

#include <util/pmt_hit.h>

class PMTHitQueue;
class TTree;

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
    // ROOT File and Tree's
    TFile file_; ///< ROOT output TFile
    TTree* opt_tree_; ///< ROOT CLB optical output TTree
    TTree* mon_tree_; ///< ROOT CLB monitoring output TTree

    // opt_tree_ Variables
    mutable PMTHit hit_;

    /// Add the branches to the optical CLB TTree
    void addHitBranches();

    // mon_tree_ Variables
    mutable std::uint32_t fPomId_mon_clb; ///< Mon CLB: Header POM ID (4 bytes)
    mutable std::uint32_t fTimestamp_s_mon_clb; ///< Mon CLB: Header timestamp (4 bytes)
    mutable std::uint32_t fPad_mon_clb; ///< Mon CLB: Header Pad (4 bytes)
    mutable std::uint32_t fValid_mon_clb; ///< Mon CLB: Header Valid (4 bytes)
    mutable std::uint16_t fTemperate_mon_clb; ///< Mon CLB: Temperature data (2 bytes)
    mutable std::uint16_t fHumidity_mon_clb; ///< Mon CLB: Humidity data (2 bytes)
    mutable std::uint32_t fHits_mon_clb[30]; ///< Mon CLB: Channel Hits (4 bytes)

    /// Add the branches to the monitoring CLB TTree
    void addMonCLBBranches();
};
