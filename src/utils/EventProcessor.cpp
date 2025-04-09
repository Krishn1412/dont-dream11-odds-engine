#include "EventProcessor.h"
#include "OddsEngineCore.h"  // Your core logic to calculate odds, e.g. updateMatchState, calculateOdds
#include <chrono>

// This simulates your odds engine’s internal state
static MatchState globalMatchState;  // Shared match state
std::mutex globalStateMutex;

void processMatchUpdate(const Odds::MatchStateRequest& req) {
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

void processBet(const Odds::BetRequest& bet) {
    std::lock_guard<std::mutex> lock(globalStateMutex);

    double odds = OddsModel::getInstance().computeProbability(globalMatchState);
    double payout = bet.stake() * odds;

    std::cout << "[Bet] User: " << bet.userid()
              << " placed " << bet.stake()
              << " at odds " << odds
              << ". Potential Payout: " << payout << "\n";
}

void eventLoop(ConcurrentQueue<Event>& queue) {
    while (true) {
        Event event;
        queue.wait_and_pop(event);

        switch (event.type) {
            case EventType::MatchUpdate:
                processMatchUpdate(event.matchUpdate);
                break;
            case EventType::BetPlacement:
                processBet(event.bet);
                break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // simulate small delay
    }
}

void startEventProcessor(ConcurrentQueue<Event>& queue, int numThreads) {
    for (int i = 0; i < numThreads; ++i) {
        std::thread(eventLoop, std::ref(queue)).detach();
    }
}
