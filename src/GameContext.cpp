#include "GameContext.h"

std::shared_ptr<MarketContext> GameContext::getOrCreateMarket(const std::string& marketName) {
    // std::lock_guard<std::mutex> lock(mtx);

    auto it = markets.find(marketName);
    if (it != markets.end()) {
        return it->second;
    }

    auto marketContext = std::make_shared<MarketContext>();
    markets[marketName] = marketContext;
    return marketContext;
}

std::vector<std::string> GameContext::getActiveMarkets() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::string> marketNames;
    for (const auto& pair : markets) {
        marketNames.push_back(pair.first);
    }
    return marketNames;
}
