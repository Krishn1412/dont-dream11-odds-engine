#include "odds_engine.grpc.pb.h"
#include "../src/utils/EventProcessor.h"
class OddsEngineServiceImpl final : public odds::OddsEngine::Service{
    public:
        explicit OddsEngineServiceImpl(ConcurrentQueue<Event>& queue);
        grpc::Status UpdateMatchState(grpc::ServerContext* context,
                                const odds::MatchStateRequest* request,
                                odds::OddsResponse* response) override;

        grpc::Status PlaceBet(grpc::ServerContext* context,
                                const odds::BetRequest* request,
                                odds::OddsResponse* response) override;

        grpc::Status GetOdds(grpc::ServerContext* context,
                            const odds::OddsQueryRequest* request,
                            odds::OddsResponse* response) override;

        grpc::Status SetInitialOdds(grpc::ServerContext* context,
                              const odds::SetInitialOddsRequest* request,
                              odds::Ack* response) override;
                    
    private:
        ConcurrentQueue<Event>& queue;
};