#pragma once
#include "MatchState.h"

class OddsModel {
public:
    static OddsModel& getInstance() {
        static OddsModel instance;
        return instance;
    }

    double computeProbability(const MatchState& match) const;

private:
    OddsModel() = default;
    OddsModel(const OddsModel&) = delete;
    OddsModel& operator=(const OddsModel&) = delete;
};
