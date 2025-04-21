// OddsModel.cpp
#include "OddsModel.h"
#include <algorithm>

OddsModel& OddsModel::getInstance() {
    static OddsModel instance;
    return instance;
}

double OddsModel::computeProbability(const MatchState& match, double lastBaseProb) const {
    if (match.getRemainingBalls() <= 0 || match.wickets >= 10) {
        return 0.01;
    }

    double runRate = match.getCurrentRunRate();
    double requiredRate = match.getRequiredRunRate();
    double momentum = match.getMomentum();
    double wicketFactor = match.getWicketFactor();
    double pitchMod = match.pitchModifier;

    double pressure = (runRate - requiredRate) / std::max(requiredRate, 1.0);

    double baseProb = 0.5;
    baseProb += pressure * 0.3;
    baseProb += (momentum - 6.0) * 0.02;
    baseProb += pitchMod * 0.05;
    baseProb *= wicketFactor;

    baseProb = std::clamp(baseProb, 0.01, 0.99);

    double smoothingFactor = 0.7;  // Higher = more inertia
    double smoothedProb = lastBaseProb * smoothingFactor + baseProb * (1.0 - smoothingFactor);

    return std::clamp(smoothedProb, 0.01, 0.99);
}


double OddsModel::applyExposureAdjustment(double baseProb, const ExposureState& exposure) const {
    double teamA = exposure.teamAExposure.load();
    double teamB = exposure.teamBExposure.load();
    double total = teamA + teamB + 1e-9;

    double skew = (teamA - teamB) / total;
    double adjustment = 1.0 - 0.1 * skew;

    return std::clamp(baseProb * adjustment, 0.01, 0.99);
}