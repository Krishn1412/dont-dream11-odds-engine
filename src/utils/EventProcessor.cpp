#include "EventProcessor.h"
#include "../GameManager.h"
#include "../OddsModel.h"
#include <chrono>

void processMatchUpdate(const std::string& gameId, const odds::BallUpdate& req) {
    auto game = GameManager::getInstance().getOrCreateGame(gameId);

    std::lock_guard<std::mutex> gameLock(game->mtx);

    MatchState& state = game->state;

    state.inningsNumber = req.innings();
    state.target = req.targetscore();
    state.runs = req.currentscore();
    state.wickets = 10 - req.wicketsleft();
    state.ballsRemaining = req.ballsremaining();
    state.recentRuns = std::deque<int>(req.recentruns().begin(), req.recentruns().end());

    double batterScore = OddsModel::getInstance().computeBatterImpact(
        state,
        state.batterImpact,
        game->batterStats,
        req
    );

    double bowlerScore = OddsModel::getInstance().computeBowlerImpact(
        state,
        state.bowlerImpact,
        game->bowlerStats,
        req
    );

    state.pitchModifier = req.pitchmodifier();

    std::vector<std::string> markets = game->getActiveMarkets();
    for (const auto& marketName : markets) {
        auto marketCtx = game->getOrCreateMarket(marketName);
        std::lock_guard<std::mutex> marketLock(marketCtx->mtx);

        double prob = OddsModel::getInstance().computeProbability(state, marketCtx->lastComputedProbability, req);
        marketCtx->lastComputedProbability = prob;

        std::cout << "[MatchUpdate] Game: " << gameId << ", Market: " << marketName
                  << " updated. Base odds: " << prob << "\n";
    }
}

void flushExposure(const std::string& gameId, const std::string& market) {
    auto game = GameManager::getInstance().getOrCreateGame(gameId);
    auto marketCtx = game->getOrCreateMarket(market);
    std::lock_guard<std::mutex> lock(marketCtx->mtx);

    marketCtx->globalExposure.teamAExposure = marketCtx->globalExposure.teamAExposure + marketCtx->batchExposure.teamAExposure.load();
    marketCtx->globalExposure.teamBExposure = marketCtx->globalExposure.teamBExposure + marketCtx->batchExposure.teamBExposure.load();

    double adjustedOdds = OddsModel::getInstance().applyExposureAdjustment(
        marketCtx->lastComputedProbability, marketCtx->globalExposure);

    marketCtx->lastComputedProbability = adjustedOdds;
    std::cout << "[Flush] Game: " << gameId << ", Market: " << market
              << " new odds: " << adjustedOdds << "\n";
}

void eventLoop(ConcurrentQueue<Event>& queue, size_t batchSize = 1000, int flushIntervalMs = 50) {
    const double MAX_EXPOSURE_PER_BATCH = 1000.0;
    auto lastFlushTime = std::chrono::steady_clock::now();
    std::unordered_map<std::string, std::unordered_map<std::string, double>> exposureTracker;

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
                    grpc::string market = event.bet.market();
                    game->getOrCreateMarket(market)->batchExposure.applyBet(event.bet);
                    exposureTracker[event.gameId][market] += event.bet.stake();
                    break;
            }
        }

        for (auto& gameEntry : exposureTracker) {
            for (auto& marketEntry : gameEntry.second) {
                const std::string& gameId = gameEntry.first;
                const std::string& market = marketEntry.first;

                if (marketEntry.second >= MAX_EXPOSURE_PER_BATCH || timeToFlush) {
                    flushExposure(gameId, market);
                    GameManager::getInstance().getOrCreateGame(gameId)->getOrCreateMarket(market)->batchExposure.reset();
                    marketEntry.second = 0.0;
                }
            }
        }

        if (timeToFlush) {
            lastFlushTime = now;
        }

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