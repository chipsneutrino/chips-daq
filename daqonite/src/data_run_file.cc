#include <util/pmt_hit_queues.h>

#include "data_run.h"
#include "data_run_file.h"

DataRunFile::DataRunFile(std::string path)
    : file_ { TFile::Open(path.c_str(), "RECREATE") }
{
    createRunParams();
    createSpills();
    createOptHits();
    createOptAnnotations();
    createTDUSignals();
}

void DataRunFile::close()
{
    if (file_->IsOpen()) {
        run_params_->Write();
        spills_->Write();
        opt_hits_->Write();
        opt_annotations_->Write();
        tdu_signals_->Write();

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

void DataRunFile::createSpills()
{
    spills_ = new TTree("spills", "Sequence of spills scheduled during the run");
    spills_->SetDirectory(file_.get());
    // from this point on, the TTree is owned by TFile

    spills_->Branch("number", &spill_number_, "number/l");
    spills_->Branch("tai_time_started_s", &spill_time_started_.secs, "tai_time_started_s/l");
    spills_->Branch("tai_time_started_ns", &spill_time_started_.nanosecs, "tai_time_started_ns/i");
    spills_->Branch("tai_time_stopped_s", &spill_time_stopped_.secs, "tai_time_stopped_s/l");
    spills_->Branch("tai_time_stopped_ns", &spill_time_stopped_.nanosecs, "tai_time_stopped_ns/i");
    spills_->Branch("opt_hits_begin", &spill_opt_hits_begin_, "opt_hits_begin/l");
    spills_->Branch("opt_hits_end", &spill_opt_hits_end_, "opt_hits_end/l");
    spills_->Branch("opt_annotations_begin", &spill_opt_annotations_begin_, "opt_annotations_begin/l");
    spills_->Branch("opt_annotations_end", &spill_opt_annotations_end_, "opt_annotations_end/l");
}

void DataRunFile::createOptHits()
{
    opt_hits_ = new TTree("opt_hits", "Sequence of optical hits taken from all planes during multiple spills, guaranteed to be time-sorted within the scope of a single spill");
    opt_hits_->SetDirectory(file_.get());
    // from this point on, the TTree is owned by TFile

    opt_hits_->Branch("plane_number", &hit_.plane_number, "plane_number/i");
    opt_hits_->Branch("channel_number", &hit_.channel_number, "channel_number/b");
    opt_hits_->Branch("tai_time_s", &hit_.timestamp.secs, "tai_time_s/l");
    opt_hits_->Branch("tai_time_ns", &hit_.timestamp.nanosecs, "tai_time_ns/i");
    opt_hits_->Branch("tot", &hit_.tot, "tot/b");
    opt_hits_->Branch("adc0", &hit_.adc0, "adc0/b");
}

void DataRunFile::createOptAnnotations()
{
    opt_annotations_ = new TTree("opt_annotations", "Sequence of annotations of the optical hit data (containing information about gaps, dropped channels, etc.), guaranteed to be time-sorted within the scope of a single spill");
    opt_annotations_->SetDirectory(file_.get());
    // from this point on, the TTree is owned by TFile

    opt_annotations_->Branch("type", &annotation_.type, "type/b");
    opt_annotations_->Branch("plane_number", &annotation_.plane_number, "plane_number/i");
    opt_annotations_->Branch("channel_number", &annotation_.channel_number, "plane_number/n");
    opt_annotations_->Branch("tai_time_start_s", &annotation_.time_start.secs, "tai_time_start_s/l");
    opt_annotations_->Branch("tai_time_start_ns", &annotation_.time_start.nanosecs, "tai_time_start_ns/i");
    opt_annotations_->Branch("tai_time_end_s", &annotation_.time_end.secs, "tai_time_end_s/l");
    opt_annotations_->Branch("tai_time_end_ns", &annotation_.time_end.nanosecs, "tai_time_end_ns/i");
}

void DataRunFile::createTDUSignals()
{
    tdu_signals_ = new TTree("tdu_signals", "Sequence of Fermilab accelerator time signals received from the NOvA TDU during the run period");
    tdu_signals_->SetDirectory(file_.get());
    // from this point on, the TTree is owned by TFile

    tdu_signals_->Branch("type", &tdu_signal_.type, "type/I");
    tdu_signals_->Branch("nova_time", &tdu_signal_.nova_time, "type/l");
    tdu_signals_->Branch("tai_time_s", &tdu_signal_.time.secs, "tai_time_start_s/l");
    tdu_signals_->Branch("tai_time_ns", &tdu_signal_.time.nanosecs, "tai_time_start_ns/i");
}

void DataRunFile::writeSpill(const SpillPtr spill, const PMTHitQueue& merged_hits) const
{
    spill_number_ = spill->spill_number;
    spill_time_started_ = spill->start_time;
    spill_time_stopped_ = spill->end_time;
    spill_opt_hits_begin_ = opt_hits_->GetEntries();

    // fill hits one by one
    for (const PMTHit& src_hit : merged_hits) {
        hit_ = src_hit;
        opt_hits_->Fill();
    }

    spill_opt_hits_end_ = opt_hits_->GetEntries();

    spill_opt_annotations_begin_ = opt_annotations_->GetEntries();

    // TODO: fill annotations

    spill_opt_annotations_end_ = opt_annotations_->GetEntries();
    spills_->Fill();
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

    // TODO: write spill signals

    run_params_->Fill();
}
