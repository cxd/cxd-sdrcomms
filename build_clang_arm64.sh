#!/bin/bash

podman build --platform linux/arm64 -t arm64-native-builder -f Containerfile.arm64 .

rm -rf build_emulated_arm64

podman run --rm \
  -v "$(pwd)":/workspace \
  --platform linux/arm64 \
  arm64-native-builder \
  bash -c "cmake -B build_emulated_arm64 -DSDL_KMSDRM=ON -DSDL_OPENGLES=ON && cmake --build build_emulated_arm64"

echo "🗑️ Dropping the compiler blueprint image from disk..."
# Deletes the base build image to return your host system to a spotless state
#podman rmi arm64-native-builder