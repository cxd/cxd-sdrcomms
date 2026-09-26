#!/bin/bash

echo $PWD

SOAPY_SDR_PLUGIN_PATH=$PWD/cmake-build-debug/_deps/soapypluto-build/
SOAPY_LIB=$PWD/cmake-build-debug/_deps/soapysdr-build/lib/
SOAPY_BIN=$PWD/cmake-build-debug/_deps/soapyremote-build/server/SoapySDRServer

SOAPY_REMOTE_NO_LOG=1
LD_LIBRARY_PATH=/usr/local/lib:$SOAPY_LIB:$LD_LIBRARY_PATH

$SOAPY_BIN --bind="127.0.0.1:11111"