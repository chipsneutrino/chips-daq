#include <algorithm>

#include "trigger_predictor.h"

TriggerPredictor::TriggerPredictor(std::size_t n_last, TimeDiff init_interval)
    : observed_ {}
    , sorted_ {}
    , last_timestamp_ { -1 }
    , next_ { 0 }
{
    observed_.resize(n_last);
    sorted_.resize(n_last);
    std::fill(observed_.begin(), observed_.end(), init_interval);
}

void TriggerPredictor::addTrigger(Timestamp timestamp)
{
    if (last_timestamp_ < 0) {
        last_timestamp_ = timestamp;
        return;
    }

    observed_[next_] = timestamp - last_timestamp_;
    next_ = (next_ + 1) % observed_.size();
    last_timestamp_ = timestamp;

    std::copy(observed_.begin(), observed_.end(), sorted_.begin());
    std::sort(sorted_.begin(), sorted_.end());
    learned_interval_ = sorted_[sorted_.size() / 2];
}

Timestamp TriggerPredictor::lastTimestamp() const
{
    return last_timestamp_;
}

TimeDiff TriggerPredictor::learnedInterval() const
{
    return learned_interval_;
}
