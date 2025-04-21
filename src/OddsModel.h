#pragma once

#include "MatchState.h"
#include "utils/ExposureTracker.h"

class OddsModel {
public:
    static OddsModel& getInstance();

    double computeProbability(const MatchState& match, double lastBaseProb) const;
    double applyExposureAdjustment(double baseProbability, const ExposureState& exposure) const;

private:
    OddsModel() = default;
    OddsModel(const OddsModel&) = delete;
    OddsModel& operator=(const OddsModel&) = delete;
};