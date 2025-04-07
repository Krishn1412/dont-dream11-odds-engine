// OddsModel.cpp
#include "OddsModel.h"
#include <algorithm>

double OddsModel::computeProbability(const MatchState& match) const {
    // std::scoped_lock lock(match.oddsMutex);  // Safe if used across threads

    if (match.getRemainingBalls() <= 0 || match.wickets >= 10) {
        return 0.01;
    }

    double runRate = match.getCurrentRunRate();
    double requiredRate = match.getRequiredRunRate();
    double momentum = match.getMomentum();
    double wicketFactor = match.getWicketFactor();
    double pitchMod = match.pitchModifier;

    // Pressure = (runRate - required) normalized
    double pressure = (runRate - requiredRate) / std::max(requiredRate, 1.0);

    // Combine factors with weights
    double baseProb = 0.5;
    baseProb += pressure * 0.3;
    baseProb += (momentum - 6.0) * 0.02; // avg momentum assumed ~6
    baseProb += pitchMod * 0.05;
    baseProb *= wicketFactor;

    // Clamp between 0.01 and 0.99
    return std::clamp(baseProb, 0.01, 0.99);
}
