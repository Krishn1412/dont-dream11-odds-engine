#include "utils/EventProcessor.h"
#include "../proto/odds_engine.pb.h"
#include "./GameManager.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>

const std::vector<std::string> GAME_IDS = {"game_1", "game_2"};
const std::vector<std::string> MARKETS = {"match_winner", "runs_over", "first_innings_total"};

void initializeGamesAndMarkets() {
    auto& gm = GameManager::getInstance();

    for (const auto& gameId : GAME_IDS) {
        auto game = gm.getOrCreateGame(gameId);
        for (const auto& market : MARKETS) {
            auto ctx = game->getOrCreateMarket(market);
            std::lock_guard<std::mutex> lock(ctx->mtx);
            ctx->initialOdds =  std::make_pair(0.55, 0.45);
            if (ctx->initialOdds.has_value()) {
                std::cout << "[Init] Game: " << gameId << ", Market: " << market
                        << ", Initial Odds (A: " << ctx->initialOdds->first
                        << ", B: " << ctx->initialOdds->second << ")\n";
            }
        }
    }
}

int main() {
    ConcurrentQueue<Event> queue;
    startEventProcessor(queue, 4);

    initializeGamesAndMarkets();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> runDist(0, 6);
    std::uniform_real_distribution<> stakeDist(50.0, 150.0);
    std::uniform_real_distribution<> oddsDist(1.2, 3.0);
    std::bernoulli_distribution teamDist(0.5);
    std::uniform_int_distribution<> gameChooser(0, GAME_IDS.size() - 1);
    std::uniform_int_distribution<> marketChooser(0, MARKETS.size() - 1);

    // Simulated BallUpdate generator thread
    std::thread([&]() {
        while (true) {
            std::string gameId = GAME_IDS[gameChooser(gen)];
            std::string market = MARKETS[marketChooser(gen)];

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
            ev.gameId = gameId;
            ev.market = market;
            ev.matchUpdate = update;

            queue.push(ev);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }).detach();

    // Simulated Bet generator thread
    std::thread([&]() {
        for (int i = 0; i < 100000; ++i) {
            std::string gameId = GAME_IDS[gameChooser(gen)];
            std::string market = MARKETS[marketChooser(gen)];

            odds::Bet bet;
            bet.set_userid("user_" + std::to_string(i));
            bet.set_stake(stakeDist(gen));
            bet.set_market(market);
            bet.set_odds(oddsDist(gen));
            bet.set_teama(teamDist(gen));

            Event ev;
            ev.type = EventType::Bet;
            ev.gameId = gameId;
            ev.market = market;
            ev.bet = bet;

            queue.push(ev);

            if (i % 500 == 0)
                std::cout << "[BetGen] Pushed " << i << " bets\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }).detach();

    std::cout << "[Main] Test simulation running for multiple games and markets...\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}

//  clang++ -std=c++17 -pthread -I/opt/homebrew/opt/protobuf/include -I/opt/homebrew/include -I/opt/homebrew/opt/abseil/include src/main.cpp  src/utils/EventProcessor.cpp proto/odds_engine.pb.cc  proto/odds_engine.grpc.pb.cc  -lprotobuf -lgrpc++ -lgrpc -o odds_test