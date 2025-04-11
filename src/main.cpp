// #include "OddsModel.h"
// #include <iostream>
// using namespace std;
// int main() {
//     cout<<"heheh"<<endl;
//     MatchState match;
//     match.runs = 80;
//     match.wickets = 4;
//     match.overs = 10.0;
//     match.target = 160;
//     match.ballsRemaining = 60;
//     match.pitchModifier = 0.05;
//     std::deque<int> sampleRuns;
//     for (int run : {1, 0, 2, 4, 0, 1, 1, 1, 0, 2, 4, 6}) {
//         sampleRuns.push_back(run);
//     }
//     match.recentRuns = sampleRuns;
    
//     OddsModel model;
//     double winProb = model.computeProbability(match);

//     std::cout << "Computed win probability: " << winProb << std::endl;
//     return 0;
// }

#include "utils/EventProcessor.h"
#include "../proto/odds_engine.pb.h"
#include <thread>
#include <chrono>
#include <iostream>

int main() {
    ConcurrentQueue<Event> queue;
    startEventProcessor(queue, 4);  // 4 worker threads

    std::thread([&queue]() {
        for (int i = 0; i < 5; ++i) {
            odds::BallUpdate update;
            update.set_innings(2);
            update.set_targetscore(180);
            update.set_currentscore(100 + i * 5);
            update.set_wicketsleft(8 - i);
            update.set_ballsremaining(60 - i * 3);
            update.add_recentruns(4 + (i % 3));
            (*update.mutable_bowlerimpact())["BowlerA"] = 1.0;
            update.set_pitchmodifier(1.0);

            Event ev;
            ev.type = EventType::BallUpdate;
            ev.matchUpdate = update;
            queue.push(ev);

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }).detach();

    std::thread([&queue]() {
        for (int i = 0; i < 1000; ++i) {
            odds::Bet bet;
            bet.set_userid("user_" + std::to_string(i));
            bet.set_stake(100 + (rand() % 50));
            bet.set_market("India");
            bet.set_odds(1.5);

            Event ev;
            ev.type = EventType::Bet;
            ev.bet = bet;
            queue.push(ev);

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }).detach();

    std::cout << "[Main] Test event loop running...\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}



//  clang++ -std=c++17 -pthread -I/opt/homebrew/opt/protobuf/include -I/opt/homebrew/include -I/opt/homebrew/opt/abseil/include src/main.cpp  src/utils/EventProcessor.cpp proto/odds_engine.pb.cc  proto/odds_engine.grpc.pb.cc  -lprotobuf -lgrpc++ -lgrpc -o odds_test