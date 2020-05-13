#include <algorithm>

#include "trigger_predictor.h"

TriggerPredictor::TriggerPredictor(std::size_t n_last, TimeDiff init_interval)
    : observed_ {}
    , sorted_ {}
    , last_timestamp_ {}
    , next_ { 0 }
{
    observed_.resize(n_last);
    sorted_.resize(n_last);
    std::fill(observed_.begin(), observed_.end(), init_interval);
}

void TriggerPredictor::addTrigger(const tai_timestamp& timestamp)
{
    if (last_timestamp_.empty()) {
        last_timestamp_ = timestamp;
        return;
    }

    // TODO: possible precision loss here, implement timestamp arithmetic correctly
    observed_[next_] = timestamp.combined_secs() - last_timestamp_.combined_secs();

    next_ = (next_ + 1) % observed_.size();
    last_timestamp_ = timestamp;

    std::copy(observed_.begin(), observed_.end(), sorted_.begin());
    std::sort(sorted_.begin(), sorted_.end());
    learned_interval_ = sorted_[sorted_.size() / 2];
}

const tai_timestamp& TriggerPredictor::lastTimestamp() const
{
    return last_timestamp_;
}

TimeDiff TriggerPredictor::learnedInterval() const
{
    return learned_interval_;
}
