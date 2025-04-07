#pragma once
#include "OddsModel.h"
#include "RiskManager.h"
#include "MatchState.h"

class OddsFusion {
public:
    OddsFusion(const OddsModel&, const RiskManager&, double alpha = 0.6);
    double computeFinalOdds(const MatchState& match) const;

private:
    const OddsModel& model;
    const RiskManager& risk;
    double alpha;

    double redistribute(double prob, double margin = 0.05) const;
};
