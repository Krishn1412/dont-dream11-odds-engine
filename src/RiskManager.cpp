#include "RiskManager.h"

void RiskManager::placeBet(const Bet<>& bet) {
    std::unique_lock lock(mutex);
    bets.push_back(bet);
}

double RiskManager::getExposure(bool teamA) const {
    double exposure = 0.0;
    for (const auto& bet : bets) {
        if (bet.teamA == teamA)
            exposure += bet.stake * bet.odds;
    }
    return exposure;
}

double RiskManager::getMarketProbability() const {
    std::shared_lock lock(mutex);
    double expA = getExposure(true);
    double expB = getExposure(false);
    if (expA + expB == 0) return 0.5;
    return expA / (expA + expB);
}
