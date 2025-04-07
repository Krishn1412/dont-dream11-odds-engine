#include "OddsModel.h"

double OddsModel::computeProbability(const MatchState& match) const {
    double ballsRemaining = 120 - match.overs * 6;
    if (ballsRemaining <= 0) return 0.0;

    double runRate = match.runs / (match.overs + 0.01);
    double requiredRate = match.target / 20.0;

    double pressure = (runRate - requiredRate) / requiredRate;
    double wicketPenalty = match.wickets / 10.0;

    double prob = 0.5 + pressure * 0.3 - wicketPenalty * 0.2;
    return std::max(0.01, std::min(0.99, prob));
}
