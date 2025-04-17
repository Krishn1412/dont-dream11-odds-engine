cmake_minimum_required(VERSION 3.15)
project(odds_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your proto generated files
set(PROTO_SRCS
    proto/odds_engine.pb.cc
    proto/odds_engine.grpc.pb.cc
)

set(SOURCE_FILES
    src/main.cpp
    src/utils/EventProcessor.cpp
    src/MatchState.cpp 
    src/OddsModel.cpp
    ${PROTO_SRCS}
)

include_directories(
    /opt/homebrew/opt/protobuf/include
    /opt/homebrew/include
    /opt/homebrew/opt/abseil/include
)

link_directories(
    /opt/homebrew/opt/protobuf/lib
    /opt/homebrew/opt/grpc/lib
    /opt/homebrew/opt/abseil/lib
)

add_executable(odds_test ${SOURCE_FILES})

target_link_libraries(odds_test
    protobuf
    grpc++
    grpc
    gpr
    absl_base
    absl_cord
    absl_strings
    absl_cord_internal
    pthread
)
