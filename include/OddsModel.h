#pragma once
#include "MatchState.h"

class OddsModel {
public:
    double computeProbability(const MatchState& match) const;
};
