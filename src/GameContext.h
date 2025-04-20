#pragma once
#include "MatchState.h"
#include "utils/ExposureTracker.h"
#include <unordered_map>

#include <mutex>

struct GameContext {
    MatchState state;
    ExposureState globalExposure;
    ExposureState batchExposure;
    std::mutex mtx;
};
