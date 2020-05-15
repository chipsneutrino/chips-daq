#include <util/pmt_hit_queues.h>

#include "run_file.h"

RunFile::RunFile(std::string path)
    : file_ { TFile::Open(path.c_str(), "RECREATE") }
{
    createOptHits();
}

void RunFile::close()
{
    if (file_->IsOpen()) {
        opt_hits_->Write();

        file_->Close();
    }
}

void RunFile::flush()
{
    // TODO: make sure data up till now is written
}

bool RunFile::isOpen() const
{
    return file_->IsOpen();
}

void RunFile::createOptHits()
{
    opt_hits_ = new TTree("opt_hits", "opt_hits");
    opt_hits_->SetDirectory(file_.get());
    // from this point on, the TTree is owned by TFile

    opt_hits_->Branch("plane_number", &hit_.plane_number, "plane_number/i");
    opt_hits_->Branch("channel_number", &hit_.channel_number, "channel_number/b");
    opt_hits_->Branch("tai_time_s", &hit_.timestamp.secs, "tai_time_s/l");
    opt_hits_->Branch("tai_time_ns", &hit_.timestamp.nanosecs, "tai_time_ns/i");
    opt_hits_->Branch("tot", &hit_.tot, "tot/b");
    opt_hits_->Branch("adc0", &hit_.adc0, "adc0/b");
}

void RunFile::writeHitQueue(const PMTHitQueue& queue) const
{
    // fill hits one by one
    for (const PMTHit& queue_hit : queue) {
        hit_ = queue_hit;
        opt_hits_->Fill();
    }
}
