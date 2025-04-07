#pragma once
#include <string>

template<typename AmountType = double>
struct Bet {
    std::string userId;
    AmountType stake;
    AmountType odds;
    bool teamA;
};
