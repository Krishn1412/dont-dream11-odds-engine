#include "EventProcessor.h"
#include "../OddsModel.h"
#include <chrono>

// This simulates your odds engine’s internal state
static MatchState globalMatchState;  // Shared match state
std::mutex globalStateMutex;

void processMatchUpdate(odds::BallUpdate& req) {
    std::lock_guard<std::mutex> lock(globalStateMutex);

    // Update match state using the proto message
    globalMatchState.inningsNumber = req.innings();
    globalMatchState.target = req.targetscore();
    globalMatchState.runs = req.currentscore();
    globalMatchState.wickets = 10 - req.wicketsleft(); // assuming 10 wickets total
    globalMatchState.ballsRemaining = req.ballsremaining();
    globalMatchState.recentRuns = std::deque<int>(req.recentruns().begin(), req.recentruns().end());

    globalMatchState.bowlerImpact.clear();
    for (const auto& entry : req.bowlerimpact()) {
        globalMatchState.bowlerImpact[entry.first] = entry.second;
    }

    globalMatchState.pitchModifier = req.pitchmodifier();

    std::cout << "[MatchUpdate] State updated. New odds: "
              << OddsModel::getInstance().computeProbability(globalMatchState) << "\n";
}

void flushBets(std::vector<odds::Bet>& bets) {
    std::lock_guard<std::mutex> lock(globalStateMutex);

    double odds = OddsModel::getInstance().computeProbability(globalMatchState);

    std::cout << "[Batch] Processing " << bets.size() << " bets at odds: " << odds << "\n";

    for (odds::Bet& bet : bets) {
        double payout = bet.stake() * odds;

        std::cout << "  [Bet] User: " << bet.userid()
                  << ", Stake: " << bet.stake()
                  << ", Payout: " << payout << "\n";
    }
}

void eventLoop(ConcurrentQueue<Event>& queue, size_t batchSize = 1000, int flushIntervalMs = 50) {
    std::vector<odds::Bet> betBatch;
    betBatch.reserve(batchSize);

    auto lastFlush = std::chrono::steady_clock::now();

    while (true) {
        Event event;
        bool gotEvent = queue.try_pop(event);

        auto now = std::chrono::steady_clock::now();
        bool timeToFlush = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFlush).count() >= flushIntervalMs;

        if (gotEvent) {
            switch (event.type) {
                case EventType::BallUpdate:
                    processMatchUpdate(event.matchUpdate);
                    break;
                case EventType::Bet:
                    betBatch.push_back(event.bet);
                    break;
            }
        }

        if (!betBatch.empty() && (betBatch.size() >= batchSize || timeToFlush)) {
            flushBets(betBatch);
            betBatch.clear();
            lastFlush = std::chrono::steady_clock::now();
        }

        // Light sleep to reduce CPU spinning
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