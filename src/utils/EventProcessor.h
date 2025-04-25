#pragma once
#include <memory>
#include "ConcurrentQueue.h"
#include "../MatchState.h"
#include "../../proto/odds_engine.pb.h"
#include "../../proto/odds_engine.grpc.pb.h"
#include <string>
#include <thread>
#include <iostream>

enum class EventType {
    BallUpdate,
    Bet
};

struct Event {
    EventType type;
    std::string gameId;
    odds::BallUpdate matchUpdate;
    odds::Bet bet;
};


void startEventProcessor(ConcurrentQueue<Event>& queue, int numThreads = 4);
