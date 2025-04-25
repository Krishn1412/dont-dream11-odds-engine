#pragma once
#include "MarketContext.h"
#include <unordered_map>

#include <mutex>

class GameContext {
public:
    MatchState state;
    std::unordered_map<std::string, BatterStats> batterStats;
    std::unordered_map<std::string, BowlerStats> bowlerStats;
    std::shared_ptr<MarketContext> getOrCreateMarket(const std::string& marketName);
    std::vector<std::string> getActiveMarkets();
    std::mutex mtx;

private:
    std::unordered_map<std::string, std::shared_ptr<MarketContext>> markets;
};
