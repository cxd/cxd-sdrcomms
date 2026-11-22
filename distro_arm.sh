#!/bin/bash

rm -rf build/arm64
echo $PWD

mkdir -p build/arm64/sdrcomms
mkdir -p build/arm64/sdrcomms/bin
mkdir -p build/arm64/sdrcomms/lib/modules
mkdir -p build/arm64/sdrcomms/services

SOAPY_SDR_PLUGIN_PATH=$PWD/build_emulated_arm64/_deps/soapypluto-build/
SOAPY_LIB=$PWD/build_emulated_arm64/_deps/soapysdr-build/lib/
SOAPY_BIN=$PWD/build_emulated_arm64/_deps/soapyremote-build/server/SoapySDRServer
SOAPY_CLIENT=$PWD/build_emulated_arm64/_deps/soapyremote-build/client

APP_BIN=$PWD/build_emulated_arm64/src/application/
CLIENT_BIN=$PWD/build_emulated_arm64/src/sdrcommsui/

cp $SOAPY_BIN build/arm64/sdrcomms/bin/
cp $SOAPY_LIB/*.so build/arm64/sdrcomms/lib/
cp $SOAPY_LIB/*.so.* build/arm64/sdrcomms/lib/
cp $SOAPY_SDR_PLUGIN_PATH/*.so build/arm64/sdrcomms/lib/modules/
cp $SOAPY_CLIENT/*.so build/arm64/sdrcomms/lib/modules/

# copy custom application
cp $APP_BIN/application build/arm64/sdrcomms/bin/
cp $CLIENT_BIN/sdrcommsui build/arm64/sdrcomms/bin/

cp -r src/service/bin build/arm64/sdrcomms/
cp -r src/resources build/arm64/
cp -r src/service/services build/arm64/
cp -r src/service/install_arm.sh build/arm64/
