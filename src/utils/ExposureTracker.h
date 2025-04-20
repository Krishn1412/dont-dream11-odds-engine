#pragma once

#include <mutex>
#include <atomic>
#include "../../proto/odds_engine.pb.h"

enum Outcome {
    TEAM_A_WIN = 0,
    TEAM_B_WIN = 1
};

struct ExposureState {
    std::atomic<double> teamAExposure{0.0};
    std::atomic<double> teamBExposure{0.0};

    void applyBet(odds::Bet& bet) {
        if (bet.teama()) {
            teamAExposure = teamAExposure + bet.stake();
        } else {
            teamBExposure = teamBExposure + bet.stake();
        }
    }

    void reset() {
        teamAExposure.store(0.0);
        teamBExposure.store(0.0);
    }
};

extern ExposureState globalExposure;
extern ExposureState batchExposure;