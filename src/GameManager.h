#pragma once
#include "GameContext.h"
#include <unordered_map>
#include <mutex>

class GameManager {
public:
    static GameManager& getInstance() {
        static GameManager instance;
        return instance;
    }

    std::shared_ptr<GameContext> getOrCreateGame(const std::string& gameId);

private:
    std::unordered_map<std::string, std::shared_ptr<GameContext>> games;
    std::mutex gamesMutex;
};
