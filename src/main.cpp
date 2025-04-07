#include "OddsModel.h"
#include <iostream>

int main() {
    MatchState match;
    match.runs = 80;
    match.wickets = 4;
    match.overs = 10.0;
    match.target = 160;
    match.ballsRemaining = 60;
    match.pitchModifier = 0.05;
    std::deque<int> sampleRuns;
    for (int run : {1, 0, 2, 4, 0, 1, 1, 1, 0, 2, 4, 6}) {
        sampleRuns.push_back(run);
    }
    match.recentRuns = sampleRuns;

    OddsModel model;
    double winProb = model.computeProbability(match);

    std::cout << "Computed win probability: " << winProb << std::endl;
    return 0;
}
