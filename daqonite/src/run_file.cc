#include <TTree.h>

#include <util/pmt_hit_queues.h>

#include "run_file.h"

RunFile::RunFile(std::string path)
    : file_ { path.c_str(), "RECREATE" }
{
    opt_tree_ = new TTree("opt_hits", "opt_hits");
    opt_tree_->SetDirectory(&file_);
    addHitBranches();

    mon_tree_ = new TTree("CLBMon_tree", "CLBMon_tree");
    mon_tree_->SetDirectory(&file_);
    addMonCLBBranches();
}

void RunFile::close()
{
    if (file_.IsOpen()) {
        opt_tree_->Write();
        mon_tree_->Write();

        file_.Close();
    }
}

void RunFile::flush()
{
    // TODO: make sure data up till now is written
}

bool RunFile::isOpen() const
{
    return file_.IsOpen();
}

void RunFile::addHitBranches()
{
    opt_tree_->Branch("plane_number", &hit_.plane_number, "plane_number/i");
    opt_tree_->Branch("channel_number", &hit_.channel_number, "channel_number/b");
    opt_tree_->Branch("tai_time_s", &hit_.timestamp.secs, "tai_time_s/l");
    opt_tree_->Branch("tai_time_ns", &hit_.timestamp.nanosecs, "tai_time_ns/i");
    opt_tree_->Branch("tot", &hit_.tot, "tot/b");
    opt_tree_->Branch("adc0", &hit_.adc0, "adc0/b");
}

void RunFile::addMonCLBBranches()
{
    mon_tree_->Branch("PomId", &fPomId_mon_clb, "fPomId_mon_clb/i");
    mon_tree_->Branch("TimeStamp_s", &fTimestamp_s_mon_clb, "fTimestamp_s_mon_clb/i");
    mon_tree_->Branch("Pad", &fPad_mon_clb, "fPad_mon_clb/i");
    mon_tree_->Branch("Valid", &fValid_mon_clb, "fValid_mon_clb/i");
    mon_tree_->Branch("Temperate", &fTemperate_mon_clb, "fTemperate_mon_clb/s");
    mon_tree_->Branch("Humidity", &fHumidity_mon_clb, "fHumidity_mon_clb/s");
    mon_tree_->Branch("Hits", &fHits_mon_clb, "fHits_mon_clb[30]/i");
}

void RunFile::writeHitQueue(const PMTHitQueue& queue) const
{
    // fill hits one by one
    for (const PMTHit& queue_hit : queue) {
        hit_ = queue_hit;
        opt_tree_->Fill();
    }
}