#include "OddsEngineServiceImpl.h"
#include "../src/GameManager.h"
#include "../src/utils/EventProcessor.h"


grpc::Status OddsEngineServiceImpl::UpdateMatchState(
    grpc::ServerContext* context,
    const odds::MatchStateRequest* request,
    odds::OddsResponse* response) {

    // Create an Event for BallUpdate
    Event ev;
    ev.type = EventType::BallUpdate;
    ev.gameId = request->gameid();
    ev.matchUpdate = request->update();

    queue.push(ev);

    // Respond with an empty OddsResponse (could be updated after processing if needed)
    response->set_winprobability(0.0);  // Placeholder
    return grpc::Status::OK;
}

grpc::Status OddsEngineServiceImpl::PlaceBet(
    grpc::ServerContext* context,
    const odds::BetRequest* request,
    odds::OddsResponse* response) {

    Event ev;
    ev.type = EventType::Bet;
    ev.gameId = request->gameid();
    ev.bet = request->bet();

    queue.push(ev);

    // Placeholder response
    response->set_winprobability(0.0);
    return grpc::Status::OK;
}

grpc::Status OddsEngineServiceImpl::GetOdds(
    grpc::ServerContext* context,
    const odds::OddsQueryRequest* request,
    odds::OddsResponse* response) {

    auto& gm = GameManager::getInstance();
    auto game = gm.getOrCreateGame(request->gameid());
    auto ctx = game->getOrCreateMarket(request->market());

    std::lock_guard<std::mutex> lock(ctx->mtx);

    response->set_winprobability(ctx->lastComputedProbability);
    return grpc::Status::OK;
}

grpc::Status OddsEngineServiceImpl::SetInitialOdds(
    grpc::ServerContext* context,
    const odds::SetInitialOddsRequest* request,
    odds::Ack* response) {

    auto& gm = GameManager::getInstance();
    auto game = gm.getOrCreateGame(request->game_id());
    auto ctx = game->getOrCreateMarket(request->market());

    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->lastComputedProbability = request->initialprobability();

    response->set_message("Initial odds set successfully.");
    return grpc::Status::OK;
}

OddsEngineServiceImpl::OddsEngineServiceImpl(ConcurrentQueue<Event>& q) : queue(q) {}
