#!/bin/bash

export SOAPY_SDR_PLUGIN_PATH=/home/cd/workspace/cpp/cxd-sdrcomms/cmake-build-debug/_deps/soapyremote-build/client 
./application --server-uri=tcp://127.0.0.1:11111 \
              --driver-name=remote \
              --remote-driver=plutosdr \
              --device-uri=ip:pluto.local \
              --bind-address=127.0.0.1 \
              --udp-server-address=192.168.0.89 \
              --model-path=/Users/cd/Projects/cpp/sdrcomms/src/resources \
              --model-file=/Users/cd/Projects/cpp/sdrcomms/src/resources/mod_dualbranch_tcn.json \
              --class-map-file=/Users/cd/Projects/cpp/sdrcomms/src/resources/class_map_v1.json
