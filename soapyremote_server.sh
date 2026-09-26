#!/bin/bash

echo $PWD

SOAPY_SDR_PLUGIN_PATH=$PWD/cmake-build-debug/_deps/soapypluto-build/
SOAPY_BIN=$PWD/cmake-build-debug/_deps/soapyremote-build/server/SoapySDRServer

SOAPY_REMOTE_NO_LOG=1
LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

$SOAPY_BIN --bind="0.0.0.0:11111"