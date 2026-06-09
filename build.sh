#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update
sudo apt-get install -y libboost-all-dev libsqlitecpp-dev libsqlite3-dev libwebsocketpp-dev nlohmann-json3-dev libssl-dev

cmake -S . -B build
cmake --build build -j"$(nproc)"
