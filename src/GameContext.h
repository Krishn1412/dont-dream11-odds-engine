#pragma once
#include "MarketContext.h"
#include <unordered_map>

#include <mutex>

class GameContext {
public:
    // Retrieves an existing market or creates a new one if not present
    std::shared_ptr<MarketContext> getOrCreateMarket(const std::string& marketName);

private:
    std::unordered_map<std::string, std::shared_ptr<MarketContext>> markets;
    std::mutex mtx;
};
