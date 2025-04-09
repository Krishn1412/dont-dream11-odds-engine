#include <bits/stdc++.h>
#include "MatchState.h"

class OddsEngineServiceImpl final : public odds::OddsEngine::Service {
public:
    grpc::Status UpdateMatchState(grpc::ServerContext* context, const odds::MatchStateRequest* request,
                                  odds::OddsResponse* response) override {
        MatchState state = convertToInternalState(*request);
        double probability = oddsModel.computeProbability(state);
        response->set_winprobability(probability);
        return grpc::Status::OK;
    }

    grpc::Status PlaceBet(grpc::ServerContext* context, const odds::BetRequest* request,
                          odds::BetConfirmation* reply) override {
        // Place bet logic here
        double potentialPayout = request->stake() * request->odds();
        reply->set_accepted(true);
        reply->set_betid("BET123456");
        reply->set_potentialpayout(potentialPayout);
        return grpc::Status::OK;
    }

private:
    OddsModel oddsModel;

    MatchState convertToInternalState(const odds::MatchStateRequest& req) {
        MatchState s;
        s.innings = req.innings();
        s.targetScore = req.targetscore();
        s.currentScore = req.currentscore();
        s.wicketsLeft = req.wicketsleft();
        s.ballsRemaining = req.ballsremaining();
        s.recentRuns = std::deque<int>(req.recentruns().begin(), req.recentruns().end());
        s.bowlerImpact = std::unordered_map<std::string, double>(req.bowlerimpact().begin(), req.bowlerimpact().end());
        s.pitchModifier = req.pitchmodifier();
        return s;
    }
};


// protoc --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` odds_engine.proto


void RunServer() {
    std::string server_address("0.0.0.0:50051");
    OddsEngineServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Odds Engine server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}