#pragma once

#include <memory>

#include "ConcurrentQueue.h"
#include "../MatchState.h"
#include "../../proto/odds_engine.grpc.pb.h"
#include <string>
#include <thread>
#include <iostream>

// Define types of events the engine will process
enum class EventType {
    MatchUpdate,
    BetPlacement
};

// Struct to wrap gRPC messages as a unified event
struct Event {
    EventType type;
    Odds::MatchStateRequest matchUpdate;
    Odds::BetRequest bet;
};

// Function to start the event processor in one or more threads
void startEventProcessor(ConcurrentQueue<Event>& queue, int numThreads = 4);
