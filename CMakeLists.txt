cmake_minimum_required(VERSION 3.15)
project(odds_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/abseil")

find_package(absl REQUIRED)
# Add your proto generated files
set(PROTO_SRCS
    proto/odds_engine.pb.cc
    proto/odds_engine.grpc.pb.cc
)

set(SOURCE_FILES
    src/utils/ExposureTracker.cpp
    src/GameContext.cpp
    src/GameManager.cpp
    src/main.cpp
    src/utils/EventProcessor.cpp
    src/MatchState.cpp 
    src/OddsModel.cpp
    proto/OddsEngineServiceImpl.cpp
    ${PROTO_SRCS}
)
include_directories(
    /opt/homebrew/lib
    /opt/homebrew/opt/protobuf/include
    /opt/homebrew/include
    /opt/homebrew/opt/abseil/lib
    /opt/homebrew/opt/abseil/include
    /opt/homebrew/opt/abseil/include/absl
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
    absl_log_initialize
    absl_log_sink
    absl_log_severity
    absl_log_internal_message
    absl_log_internal_format
    absl_log_internal_conditions
    absl_log_internal_check_op
    absl_log_internal_nullguard
    absl_log_globals

    # Cord internals
    absl_cord
    absl_cord_internal
    absl_cordz_functions
    absl_cordz_handle
    absl_cordz_info

    absl_strings
    absl_base
    absl_cord
    absl_hash
    absl_synchronization
    pthread
)
