#include <util/pmt_hit_queues.h>

#include "data_run.h"
#include "data_run_file.h"

DataRunFile::DataRunFile(std::string path)
    : file_ { TFile::Open(path.c_str(), "RECREATE") }
{
    createRunParams();
    createOptHits();
}

void DataRunFile::close()
{
    if (file_->IsOpen()) {
        run_params_->Write();
        opt_hits_->Write();

        file_->Close();
    }
}

void DataRunFile::flush()
{
    // TODO: make sure data up till now is written
}

bool DataRunFile::isOpen() const
{
    return file_->IsOpen();
}

void DataRunFile::createRunParams()
{
    run_params_ = new TTree("run_params", "Information about the run");
    run_params_->SetDirectory(file_.get());
    // from this point on, the TTree is owned by TFile

    run_params_->Branch("number", &run_number_, "number/l");
    run_params_->Branch("type", &run_type_, "type/b");
    run_params_->Branch("utc_time_started_s", &run_time_started_.secs, "utc_time_started_s/l");
    run_params_->Branch("utc_time_started_ns", &run_time_started_.nanosecs, "utc_time_started_ns/i");
    run_params_->Branch("utc_time_stopped_s", &run_time_stopped_.secs, "utc_time_stopped_s/l");
    run_params_->Branch("utc_time_stopped_ns", &run_time_stopped_.nanosecs, "utc_time_stopped_ns/i");
}

void DataRunFile::createOptHits()
{
    opt_hits_ = new TTree("opt_hits", "Time-sorted sequence of optical hits from all planes");
    opt_hits_->SetDirectory(file_.get());
    // from this point on, the TTree is owned by TFile

    opt_hits_->Branch("plane_number", &hit_.plane_number, "plane_number/i");
    opt_hits_->Branch("channel_number", &hit_.channel_number, "channel_number/b");
    opt_hits_->Branch("tai_time_s", &hit_.timestamp.secs, "tai_time_s/l");
    opt_hits_->Branch("tai_time_ns", &hit_.timestamp.nanosecs, "tai_time_ns/i");
    opt_hits_->Branch("tot", &hit_.tot, "tot/b");
    opt_hits_->Branch("adc0", &hit_.adc0, "adc0/b");
}

void DataRunFile::writeHitQueue(const PMTHitQueue& queue) const
{
    // fill hits one by one
    for (const PMTHit& queue_hit : queue) {
        hit_ = queue_hit;
        opt_hits_->Fill();
    }
}

void DataRunFile::writeRunParametersAtStart(const std::shared_ptr<DataRun>& run) const
{
    // TODO: write configuration
}

void DataRunFile::writeRunParametersAtEnd(const std::shared_ptr<DataRun>& run) const
{
    run_number_ = run->getNumber();
    run_type_ = static_cast<UChar_t>(run->getType());
    run_time_started_ = run->getTimeStarted();
    run_time_stopped_ = run->getTimeStopped();

    // TODO: write hit counts, etc.

    run_params_->Fill();
}
