#include "utils/EventProcessor.h"
#include "../proto/odds_engine.pb.h"
#include "./GameManager.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

const std::string GAME_ID = "game_1";
const std::string MARKET = "match_winner";

void initializeGameAndMarket() {
    auto& gm = GameManager::getInstance();
    auto game = gm.getOrCreateGame(GAME_ID);
    auto ctx = game->getOrCreateMarket(MARKET);
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->lastComputedProbability = 0.55;

    std::vector<std::string> batters = {"Batter1", "Batter2", "Batter3", "Batter4", "Batter5"};
    std::vector<std::string> bowlers = {"BowlerA", "BowlerB", "BowlerC"};

    for (const auto& batter : batters) {
        ctx->state.batterImpact[batter] = 5.0; // neutral starting impact
        ctx->batterStats[batter] = BatterStats{};
    }

    for (const auto& bowler : bowlers) {
        ctx->state.bowlerImpact[bowler] = 5.0; // neutral starting impact
        ctx->bowlerStats[bowler] = BowlerStats{};
    }

    std::cout << "[Init] Game: " << GAME_ID << ", Market: " << MARKET
              << ", Initial Odds " << ctx->lastComputedProbability << "\n";
    std::cout << "[Init] Initialized batter/bowler impact scores and stats\n";
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
    std::uniform_int_distribution<> wicketChance(0, 100);
    std::uniform_int_distribution<> extraChance(0, 100);

    std::vector<std::string> batters = {"Batter1", "Batter2", "Batter3", "Batter4", "Batter5"};
    std::vector<std::string> bowlers = {"BowlerA", "BowlerB", "BowlerC"};
    int strikerIdx = 0;
    int nonStrikerIdx = 1;
    int nextBatterIdx = 2;
    int bowlerIdx = 0;

    int score = 100;
    int ballsRemaining = 60;
    int wickets = 2;

    // BallUpdate simulation
    std::thread([&]() {
        while (ballsRemaining > 0 && wickets < 10) {
            odds::BallUpdate update;
            update.set_innings(2);
            update.set_targetscore(180);
            update.set_currentscore(score);
            update.set_wicketsleft(10 - wickets);
            update.set_ballsremaining(ballsRemaining);

            std::string striker = batters[strikerIdx % batters.size()];
            std::string nonStriker = batters[nonStrikerIdx % batters.size()];
            std::string bowler = bowlers[bowlerIdx % bowlers.size()];

            update.set_striker(striker);
            update.set_nonstriker(nonStriker);
            update.set_bowler(bowler);

            update.set_pitchmodifier(1.0 + (rand() % 5) * 0.05);

            for (int i = 0; i < 6 && ballsRemaining > 0; ++i) {
                int run = runDist(gen);
                bool isWicket = (wicketChance(gen) < 8);  // ~8% chance
                bool isExtra = (extraChance(gen) < 3);     // ~3% chance
                bool isDot = (run == 0 && !isWicket);

                update.add_recentruns(run);
                update.set_iswicket(isWicket);
                update.set_isextra(isExtra);
                update.set_isdot(isDot);

                if (isWicket) {
                    ++wickets;
                    strikerIdx = nextBatterIdx++;
                } else {
                    score += run;
                    if (run % 2 == 1) std::swap(strikerIdx, nonStrikerIdx);
                }

                --ballsRemaining;
                update.set_currentscore(score);
                update.set_wicketsleft(10 - wickets);
                update.set_ballsremaining(ballsRemaining);

                Event ev;
                ev.type = EventType::BallUpdate;
                ev.gameId = GAME_ID;
                ev.market = MARKET;
                ev.matchUpdate = update;
                queue.push(ev);

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            // Change bowler every over
            ++bowlerIdx;
        }
    }).detach();

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
