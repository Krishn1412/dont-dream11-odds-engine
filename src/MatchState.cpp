#include "MatchState.h"

int MatchState::getRemainingBalls() const {
    return ballsRemaining;
}

double MatchState::getCurrentRunRate() const {
    return (overs > 0.01) ? runs / overs : 0.0;
}

double MatchState::getRequiredRunRate() const {
    return (ballsRemaining > 0) ? (target - runs) / (ballsRemaining / 6.0) : 100.0;
}

double MatchState::getMomentum() const {
    int sum = 0;
    for (int r : recentRuns) sum += r;
    return (recentRuns.empty() ? 0.0 : sum / double(recentRuns.size()));
}

double MatchState::getWicketFactor() const {
    return 1.0 - (wickets / 10.0);
}
