// MatchState.h
#pragma once
#include <deque>
#include <unordered_map>
#include <mutex>
#include <string>

struct MatchState {
    int inningsNumber = 1;
    int target = 0;
    int runs = 0;
    int wickets = 0;
    double overs = 0.0; // e.g. 10.3 overs = 10.5 here
    int ballsRemaining = 120;

    std::deque<int> recentRuns; // for momentum
    std::unordered_map<std::string, double> bowlerImpact;
    double pitchModifier = 0.0;

    std::mutex oddsMutex;

    int getRemainingBalls() const {
        return ballsRemaining;
    }

    double getCurrentRunRate() const {
        return (overs > 0.01) ? runs / overs : 0.0;
    }

    double getRequiredRunRate() const {
        return (ballsRemaining > 0) ? (target - runs) / (ballsRemaining / 6.0) : 100.0;
    }

    double getMomentum() const {
        int sum = 0;
        for (int r : recentRuns) sum += r;
        return (recentRuns.empty() ? 0.0 : sum / double(recentRuns.size()));
    }

    double getWicketFactor() const {
        return 1.0 - (wickets / 10.0); // more wickets left = higher factor
    }
};
