// OddsModel.cpp
#include "OddsModel.h"
#include <algorithm>

OddsModel& OddsModel::getInstance() {
    static OddsModel instance;
    return instance;
}

double OddsModel::computeProbability(const MatchState& match, double lastBaseProb, const odds::BallUpdate& req) const {
    if (match.getRemainingBalls() <= 0 || match.wickets >= 10) {
        return 0.01;
    }

    double runRate = match.getCurrentRunRate();
    double requiredRate = match.getRequiredRunRate();
    double momentum = match.getMomentum();
    double wicketFactor = match.getWicketFactor();
    double pitchMod = match.pitchModifier;
    double runsRemanining = req.targetscore() - req.currentscore();
    double baseProb = 0.5;

    // -----------------------
    // ⚾ Batter & Bowler Impact
    // -----------------------
    double batterImpact = 5.0;
    double bowlerImpact = 5.0;

    auto itBatter = match.batterImpact.find(req.striker());
    if (itBatter != match.batterImpact.end()) {
        batterImpact = itBatter->second;
    }

    auto itBowler = match.bowlerImpact.find(req.bowler());
    if (itBowler != match.bowlerImpact.end()) {
        bowlerImpact = itBowler->second;
    }

    baseProb += (batterImpact - 5.0) * 0.03;
    baseProb -= (bowlerImpact - 5.0) * 0.025;

    if (req.innings() == 1) {
        baseProb += (momentum - 6.0) * 0.02;

        if (match.wickets <= 2 && match.overs < 6) {
            baseProb += 0.03; // good powerplay start
        }

        if (match.overs > 16 && momentum > 7.0) {
            baseProb += 0.02;
        }

        if (pitchMod < 0.9) {
            baseProb -= 0.03;
        }

    } else {
        // 🏏 Second Innings (Chasing)
        double pressure = (runRate - requiredRate) / std::max(requiredRate, 1.0);
        baseProb += pressure * 0.3;
        baseProb += (momentum - 6.0) * 0.02;

        if (match.wickets >= 6 && match.getRemainingBalls() > 12) {
            baseProb -= 0.05; // high pressure with tailenders
        }

        if (match.getRemainingBalls() < 12 && runsRemanining < 20) {
            baseProb += 0.05;
        }

        if (pitchMod < 0.9) {
            baseProb -= 0.03;
        }
    }

    baseProb *= wicketFactor;
    baseProb += pitchMod * 0.05;

    // Clamp
    baseProb = std::clamp(baseProb, 0.01, 0.99);

    double smoothingFactor = 0.7;
    double smoothedProb = lastBaseProb * smoothingFactor + baseProb * (1.0 - smoothingFactor);

    return std::clamp(smoothedProb, 0.01, 0.99);
}

double OddsModel::applyExposureAdjustment(double baseProb, const ExposureState& exposure) const {
    double teamA = exposure.teamAExposure.load();
    double teamB = exposure.teamBExposure.load();
    double total = teamA + teamB + 1e-9;

    double skew = (teamA - teamB) / total;
    double adjustment = 1.0 - 0.1 * skew;

    return std::clamp(baseProb * adjustment, 0.01, 0.99);
}

double OddsModel::computeBatterImpact(
    const MatchState& match,
    std::unordered_map<std::string, double> batterImpact,
    std::unordered_map<std::string, BatterStats>& batterStats,
    const odds::BallUpdate& req) const {

    const std::string& batter = req.striker();
    double pitchFactor = 1.0 + match.pitchModifier;
    double overs = match.overs;

    BatterStats& stats = batterStats[batter];
    stats.ballsFaced++;
    if (req.isdot()) stats.dotBalls++;
    if (req.isboundary()) stats.boundaries++;

    if (stats.ballsFaced <= 3) {
        return batterImpact[batter];
    }

    double delta = 0.0;
    std::ostringstream reason;

    if (overs <= 6.0) {
        if (stats.dotBalls >= 2) { delta -= 0.2 * pitchFactor; reason << "Early dots penalty. "; }
        if (stats.boundaries >= 1) { delta += 0.4 * pitchFactor; reason << "Early boundary reward. "; }
        if (req.runs() == 1 || req.runs() == 2) { delta += 0.05 * pitchFactor; reason << "Rotation reward. "; }

    } else if (overs <= 16.0) {
        if (req.isdot()) { delta -= 0.15 * pitchFactor; reason << "Mid-overs dot. "; }
        if (req.isboundary()) { delta += 0.3 * pitchFactor; reason << "Mid-overs boundary. "; }
        if (match.wickets >= 3 && req.isdot()) { delta -= 0.05 * pitchFactor; reason << "Pressure dot after 3 wickets. "; }

    } else {
        if (req.isdot()) { delta -= 0.3 * pitchFactor; reason << "Death over dot. "; }
        if (req.isboundary()) { delta += 0.6 * pitchFactor; reason << "Death over boundary. "; }
        if (req.runs() == 1 || req.runs() == 2) { delta += 0.1 * pitchFactor; reason << "Smart running. "; }
    }

    if (req.iswicket() && req.striker() == batter) {
        delta -= 0.6 * pitchFactor;
        reason << "Dismissed. ";
    }

    double oldImpact = batterImpact[batter];
    double newImpact = std::clamp(oldImpact + delta, 0.0, 10.0);
    // std::cout<<"$$$$$$$$$$$$$$$$############"<<std::endl;
    if (newImpact != oldImpact) {
        std::cout << "[ImpactChange] Batter: " << batter
                  << " | Old: " << oldImpact
                  << " -> New: " << newImpact
                  << " | Overs: " << overs
                  << " | Pitch Factor: " << pitchFactor
                  << " | Reason: " << reason.str() << "\n";
    }

    batterImpact[batter] = newImpact;
    return newImpact;
}

double OddsModel::computeBowlerImpact(
    const MatchState& match,
    std::unordered_map<std::string, double> bowlerImpact,
    std::unordered_map<std::string, BowlerStats>& bowlerStats,
    const odds::BallUpdate& req) const {

    const std::string& bowler = req.bowler();
    double pitchFactor = 1.0 + match.pitchModifier;
    double overs = match.overs;

    BowlerStats& stats = bowlerStats[bowler];
    stats.ballsBowled++;
    stats.totalRuns += req.runs();
    if (req.isdot()) stats.dotBalls++;
    if (req.isboundary()) stats.boundariesConceded++;
    if (req.iswicket()) stats.wickets++;

    double delta = 0.0;
    std::ostringstream reason;

    if (overs <= 6.0) {
        if (req.isdot()) { delta += 0.25 * pitchFactor; reason << "Dot ball in powerplay. "; }
        if (req.iswicket()) { delta += 0.6 * pitchFactor; reason << "Wicket in powerplay. "; }
        if (req.isboundary()) { delta -= 0.4 * pitchFactor; reason << "Boundary conceded in powerplay. "; }

    } else if (overs <= 16.0) {
        if (req.isdot()) { delta += 0.15 * pitchFactor; reason << "Dot in middle overs. "; }
        if (req.iswicket()) { delta += 0.5 * pitchFactor; reason << "Wicket in middle overs. "; }
        if (req.isboundary()) { delta -= 0.3 * pitchFactor; reason << "Boundary in middle overs. "; }

    } else {
        if (req.isdot()) { delta += 0.4 * pitchFactor; reason << "Dot in death. "; }
        if (req.iswicket()) { delta += 0.7 * pitchFactor; reason << "Wicket in death. "; }
        if (req.isboundary()) { delta -= 0.5 * pitchFactor; reason << "Boundary in death. "; }
    }

    if (stats.ballsBowled >= 6) {
        double economy = static_cast<double>(stats.totalRuns) * 6.0 / stats.ballsBowled;
        if (economy <= 6.0) { delta += 0.2 * pitchFactor; reason << "Low economy. "; }
        else if (economy >= 10.0) { delta -= 0.2 * pitchFactor; reason << "High economy. "; }
    }

    double oldImpact = bowlerImpact[bowler];
    double newImpact = std::clamp(oldImpact + delta, 0.0, 10.0);
    // std::cout<<"$$$$$$$$$$$$$$$$############"<<std::endl;
    if (newImpact != oldImpact) {
        std::cout << "[ImpactChange] Bowler: " << bowler
                  << " | Old: " << oldImpact
                  << " -> New: " << newImpact
                  << " | Overs: " << overs
                  << " | Pitch Factor: " << pitchFactor
                  << " | Reason: " << reason.str() << "\n";
    }

    bowlerImpact[bowler] = newImpact;
    return newImpact;
}
