#pragma once
#include "Bet.h"
#include <vector>
#include <shared_mutex>

class RiskManager {
public:
    void placeBet(const Bet<>& bet);
    double getMarketProbability() const;

private:
    std::vector<Bet<>> bets;
    mutable std::shared_mutex mutex;
    double getExposure(bool teamA) const;
};
