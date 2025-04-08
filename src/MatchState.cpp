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

void MatchState::applyBallUpdate(const BallUpdate& update) {
    std::lock_guard<std::mutex> lock(oddsMutex);

    runs += update.runsThisBall;

    if (update.isWicket) {
        wickets = std::min(wickets + 1, 10);
    }

    int totalBalls = int(overs * 6) + 1;
    overs = totalBalls / 6 + (totalBalls % 6) / 10.0;

    if (ballsRemaining > 0) ballsRemaining--;

    if (recentRuns.size() >= 12) recentRuns.pop_front();
    recentRuns.push_back(update.runsThisBall);

    if (update.newPitchModifier.has_value()) {
        pitchModifier = update.newPitchModifier.value();
    }

    if (!update.bowler.empty()) {
        bowlerImpact[update.bowler] += 0.1;
    }
}
