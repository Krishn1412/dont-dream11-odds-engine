// GameManager.cpp
#include "GameManager.h"


std::shared_ptr<GameContext> GameManager::getOrCreateGame(const std::string& gameId) {
    std::lock_guard<std::mutex> lock(gamesMutex);
    auto it = games.find(gameId);
    if (it != games.end()) return it->second;

    auto context = std::make_shared<GameContext>();
    games[gameId] = context;
    return context;
}