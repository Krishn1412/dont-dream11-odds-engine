#!/bin/bash

set -e  # Exit on error

# Make sure build directory exists
mkdir -p build
cd build

# Configure project with CMake
cmake ..

# Build the project
cmake --build .

# Run the compiled binary (replace with your actual output name if different)
./odds_test
