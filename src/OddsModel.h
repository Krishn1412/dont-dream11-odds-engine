#pragma once

#include "MatchState.h"
#include "utils/ExposureTracker.h"

class OddsModel {
public:
    static OddsModel& getInstance();

    double computeProbability(const MatchState& match, double lastBaseProb, const odds::BallUpdate& req) const;
    double applyExposureAdjustment(double baseProbability, const ExposureState& exposure) const;
    double computeBatterImpact(const MatchState& match, std::unordered_map<std::string, double> batterImpact, std::unordered_map<std::string, BatterStats>& batterStats, const odds::BallUpdate& req) const;
    double computeBowlerImpact(const MatchState& match, std::unordered_map<std::string, double> bowlerImpact, std::unordered_map<std::string, BowlerStats>& bowlerStats, const odds::BallUpdate& req) const;
private:
    OddsModel() = default;
    OddsModel(const OddsModel&) = delete;
    OddsModel& operator=(const OddsModel&) = delete;
};