# dont-dream11-odds-engine (WIP)

core odds calculator part of don't-dream11, which has two more services. A python middleware with modelling logic and a nextjs system for website.

It has two odds calculator, a model based that will calculate it based on the match stats at the moment, the other being match-bets based calculator that does that based on
the supply and demand. The final calculation is some combination of these two.

This cpp codebase will be the high speed structure that does the dynamic odds calculation.


clang++ -std=c++17 *.cpp -o odds_engine && ./odds_engine  

clang++ -std=c++17 src/main.cpp src/OddsModel.cpp -o odds_engine

clang++ -std=c++17 -pthread -I/opt/homebrew/opt/protobuf/include -I/opt/homebrew/include -I/opt/homebrew/opt/abseil/include src/main.cpp  src/utils/EventProcessor.cpp proto/odds_engine.pb.cc  proto/odds_engine.grpc.pb.cc  -lprotobuf -lgrpc++ -lgrpc -o odds_test

 to run the module, run the shell script run.sh



protoc -I=proto \                                                    
  --cpp_out=proto \
  --grpc_out=proto \
  --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
  proto/odds_engine.proto

  run this for proto file generation

This is a pure odds calculator which will only talk to the Python Manager service to provide and update the odds. It uses gRPC api for that.
