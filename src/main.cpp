#include "utils/EventProcessor.h"
#include "../proto/odds_engine.pb.h"
#include "./GameManager.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>

const std::string GAME_ID = "game_1";
const std::string MARKET = "match_winner";

void initializeGameAndMarket() {
    auto& gm = GameManager::getInstance();
    auto game = gm.getOrCreateGame(GAME_ID);
    auto ctx = game->getOrCreateMarket(MARKET);
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->lastComputedProbability = 0.55;
    std::cout << "[Init] Game: " << GAME_ID << ", Market: " << MARKET
                << ", Initial Odds " << ctx->lastComputedProbability <<"\n";
}

int main() {
    ConcurrentQueue<Event> queue;
    startEventProcessor(queue, 2);

    initializeGameAndMarket();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> runDist(0, 6);
    std::uniform_real_distribution<> stakeDist(50.0, 150.0);
    std::uniform_real_distribution<> oddsDist(1.2, 3.0);
    std::bernoulli_distribution teamDist(0.5);

    // BallUpdate simulation
    std::thread([&]() {
        while (true) {
            int score = 100 + rand() % 30;
            int ballsRemaining = 60 - (rand() % 12);
            int wickets = 8 - (rand() % 3);

            odds::BallUpdate update;
            update.set_innings(2);
            update.set_targetscore(180);
            update.set_currentscore(score);
            update.set_wicketsleft(wickets);
            update.set_ballsremaining(ballsRemaining);

            for (int i = 0; i < 6; ++i)
                update.add_recentruns(runDist(gen));

            (*update.mutable_bowlerimpact())["BowlerA"] = 1.0 + (rand() % 3) * 0.1;
            update.set_pitchmodifier(1.0 + (rand() % 5) * 0.05);

            Event ev;
            ev.type = EventType::BallUpdate;
            ev.gameId = GAME_ID;
            ev.market = MARKET;
            ev.matchUpdate = update;

            queue.push(ev);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }).detach();

    // Bet simulation
    std::thread([&]() {
        for (int i = 0; i < 100000; ++i) {
            odds::Bet bet;
            bet.set_userid("user_" + std::to_string(i));
            bet.set_stake(stakeDist(gen));
            bet.set_market(MARKET);
            bet.set_odds(oddsDist(gen));
            bet.set_teama(teamDist(gen));

            Event ev;
            ev.type = EventType::Bet;
            ev.gameId = GAME_ID;
            ev.market = MARKET;
            ev.bet = bet;

            queue.push(ev);

            if (i % 500 == 0)
                std::cout << "[BetGen] Pushed " << i << " bets\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }).detach();

    std::cout << "[Main] Test simulation running for game_1, market match_winner...\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}
