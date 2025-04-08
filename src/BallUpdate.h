// BallUpdate.h
#pragma once
#include <string>
#include <optional>

struct BallUpdate {
    int runsThisBall = 0;
    bool isWicket = false;
    std::string bowler;
    std::optional<double> newPitchModifier;
};
