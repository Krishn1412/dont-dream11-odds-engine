#include "OddsFusion.h"
#include <algorithm>

OddsFusion::OddsFusion(const OddsModel& model_, const RiskManager& risk_, double alpha_)
    : model(model_), risk(risk_), alpha(alpha_) {}

double OddsFusion::computeFinalOdds(const MatchState& match) const {
    double modelProb = model.computeProbability(match);
    double marketProb = risk.getMarketProbability();
    double fused = alpha * modelProb + (1.0 - alpha) * marketProb;
    return redistribute(fused);
}

double OddsFusion::redistribute(double prob, double margin) const {
    prob = std::clamp(prob, 0.01, 0.99);
    return prob * (1.0 - margin) + margin * 0.5;
}
