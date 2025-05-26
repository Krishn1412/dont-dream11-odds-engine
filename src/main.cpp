#include <grpcpp/grpcpp.h>
#include "../proto/OddsEngineServiceImpl.h"
#include "utils/ConcurrentQueue.h"
#include "utils/EventProcessor.h"

void RunServer() {
  std::string server_address("0.0.0.0:50051");

  ConcurrentQueue<Event> queue;
  startEventProcessor(queue, 4);
  OddsEngineServiceImpl service(queue);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}

