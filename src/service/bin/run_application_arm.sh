#!/bin/bash

export SOAPY_SDR_PLUGIN_PATH=/opt/sdrcomms/lib/modules/
export SOAPY_LIB=/opt/sdrcomms/lib
export LD_LIBRARY_PATH=$SOAPY_LIB:$SOAPY_SDR_PLUGIN_PATH:$LD_LIBRARY_PATH
./application --server-uri=tcp://127.0.0.1:11111 \
              --driver-name=remote \
              --remote-driver=plutosdr \
              --device-uri=ip:pluto.local \
              --bind-address=127.0.0.1 \
              --udp-server-address=0.0.0.0 \
              --model-path=/opt/sdrcomms/resources \
              --model-file=/opt/sdrcomms/resources/mod_dualbranch_tcn.json \
              --class-map-file=/opt/sdrcomms/resources/class_map_v1.json
