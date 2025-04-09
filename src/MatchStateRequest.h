#pragma once
#include <deque>
#include <unordered_map>
#include <mutex>
#include <string>

struct MatchStateRequest {
    int inningsNumber = 1;
    int target = 0;
    int runs = 0;
    int wickets = 0;
    double overs = 0.0;
    int ballsRemaining = 120;

    std::deque<int> recentRuns;
    std::unordered_map<std::string, double> bowlerImpact;
    double pitchModifier = 0.0;
}