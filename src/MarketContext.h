#pragma once
#include "MatchState.h"
#include "utils/ExposureTracker.h"
#include <optional>

struct MarketContext {
    MatchState state;
    ExposureState globalExposure;
    ExposureState batchExposure;
    std::optional<std::pair<double, double>> initialOdds;
    double lastComputedProbability = 0.5;
    std::mutex mtx;
};
