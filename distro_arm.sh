#!/bin/bash

echo $PWD

mkdir -p build/arm64
mkdir -p build/arm64/usr/local/bin
mkdir -p build/arm64/usr/local/lib/modules

SOAPY_SDR_PLUGIN_PATH=$PWD/build_clang_amd64/_deps/soapypluto-build/
SOAPY_LIB=$PWD/build_clang_amd64/_deps/soapysdr-build/lib/
SOAPY_BIN=$PWD/build_clang_amd64/_deps/soapyremote-build/server/SoapySDRServer
cp $SOAPY_BIN build/arm64/usr/local/bin/
cp $SOAPY_LIB/*.so build/arm64/usr/local/lib/
cp $SOAPY_LIB/*.so.* build/arm64/usr/local/lib/
cp $SOAPY_SDR_PLUGIN_PATH/*.so build/arm64/usr/local/lib/modules/

