#include "EventProcessor.h"
#include "../GameManager.h"
#include "../OddsModel.h"
#include <chrono>

void processMatchUpdate(const std::string& gameId, const odds::BallUpdate& req) {
    auto game = GameManager::getInstance().getOrCreateGame(gameId);
    std::lock_guard<std::mutex> lock(game->mtx);

    game->state.inningsNumber = req.innings();
    game->state.target = req.targetscore();
    game->state.runs = req.currentscore();
    game->state.wickets = 10 - req.wicketsleft();
    game->state.ballsRemaining = req.ballsremaining();
    game->state.recentRuns = std::deque<int>(req.recentruns().begin(), req.recentruns().end());
    game->state.bowlerImpact.clear();

    for (const auto& entry : req.bowlerimpact()) {
        game->state.bowlerImpact[entry.first] = entry.second;
    }
    game->state.pitchModifier = req.pitchmodifier();

    double prob = OddsModel::getInstance().computeProbability(game->state);
    std::cout << "[MatchUpdate] Game: " << gameId << " updated. Base odds: " << prob << "\n";
}

void flushExposure(const std::string& gameId) {
    auto game = GameManager::getInstance().getOrCreateGame(gameId);
    std::lock_guard<std::mutex> lock(game->mtx);

    game->globalExposure.teamAExposure = game->globalExposure.teamAExposure + game->batchExposure.teamAExposure.load();
    game->globalExposure.teamBExposure = game->globalExposure.teamBExposure + game->batchExposure.teamBExposure.load();

    double baseProb = OddsModel::getInstance().computeProbability(game->state);
    double newOdds = OddsModel::getInstance().applyExposureAdjustment(baseProb, game->globalExposure);

    std::cout << "[Flush] Game: " << gameId << " new odds: " << newOdds << "\n";
}

void eventLoop(ConcurrentQueue<Event>& queue, size_t batchSize = 1000, int flushIntervalMs = 50) {
    auto lastFlushTime = std::chrono::steady_clock::now();
    std::unordered_map<std::string, double> batchExposureCounter;

    while (true) {
        Event event;
        bool gotEvent = queue.try_pop(event);

        auto now = std::chrono::steady_clock::now();
        bool timeToFlush = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFlushTime).count() >= flushIntervalMs;

        if (gotEvent) {
            auto game = GameManager::getInstance().getOrCreateGame(event.gameId);
            switch (event.type) {
                case EventType::BallUpdate:
                    processMatchUpdate(event.gameId, event.matchUpdate);
                    break;
                case EventType::Bet:
                    game->batchExposure.applyBet(event.bet);
                    batchExposureCounter[event.gameId] += event.bet.stake();
                    break;
            }
        }

        for (auto& pair : batchExposureCounter) {
            if (pair.second >= 1000.0 || timeToFlush) {
                flushExposure(pair.first);
                GameManager::getInstance().getOrCreateGame(pair.first)->batchExposure.reset();
                pair.second = 0.0;
            }
        }

        if (timeToFlush) lastFlushTime = now;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void startEventProcessor(ConcurrentQueue<Event>& queue, int numThreads) {
    for (int i = 0; i < numThreads; ++i) {
        std::thread(eventLoop, std::ref(queue), 1000, 50).detach();
    }
}



// clang++ -std=c++17 -pthread  src/main.cpp  src/utils/EventProcessor.cpp proto/odds_engine.pb.cc  proto/odds_engine.grpc.pb.cc  -lprotobuf -lgrpc++ -lgrpc -o odds_test

// clang++ -std=c++17 -pthread -I/opt/homebrew/opt/protobuf/include -I/opt/homebrew/include -I/opt/homebrew/opt/abseil/include -L/opt/homebrew/opt/protobuf/lib -L/opt/homebrew/opt/grpc/lib src/main.cpp src/utils/EventProcessor.cpp proto/odds_engine.pb.cc  proto/odds_engine.grpc.pb.cc  -lprotobuf -lgrpc++ -lgrpc -lgpr -o odds_test