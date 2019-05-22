#include "TTree.h"

#include "clb_event.h"
#include "run_file.h"

RunFile::RunFile(std::string path)
    : file_ { path.c_str(), "RECREATE" }
{
    opt_tree_ = new TTree("CLBOpt_tree", "CLBOpt_tree");
    opt_tree_->SetDirectory(&file_);
    addOptCLBBranches();

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

void RunFile::addOptCLBBranches()
{
    opt_tree_->Branch("PomId", &fPomId_opt_clb, "fPomId_opt_clb/i");
    opt_tree_->Branch("Channel", &fChannel_opt_clb, "fChannel_opt_clb/b");
    opt_tree_->Branch("TimeStamp_s", &fTimestamp_s_opt_clb, "fTimestamp_s_opt_clb/i");
    opt_tree_->Branch("TimeStamp_ns", &fTimestamp_ns_opt_clb, "fTimestamp_ns_opt_clb/i");
    opt_tree_->Branch("ToT", &fTot_opt_clb, "fTot_opt_clb/B");
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

void RunFile::writeEventQueue(const CLBEventQueue& queue) const
{
    for (const CLBEvent& event : queue) {
        fPomId_opt_clb = event.PomId;
        fChannel_opt_clb = event.Channel;
        fTimestamp_s_opt_clb = event.Timestamp_s;
        fTimestamp_ns_opt_clb = event.Timestamp_ns;
        fTot_opt_clb = event.Tot;

        opt_tree_->Fill();
    }
}