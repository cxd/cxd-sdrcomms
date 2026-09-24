#!/bin/bash

rm -rf build_clang_amd64
cmake -B build_clang_amd64 \
-DCMAKE_TOOLCHAIN_FILE=clang_linux_amd64.toolchain.cmake \
-DBoost_DIR=/usr/lib/x86_64-linux-gnu/cmake/Boost-1.83.0
cmake --build build_clang_amd64
