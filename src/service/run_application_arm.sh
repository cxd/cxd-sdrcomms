#!/bin/bash

export SOAPY_SDR_PLUGIN_PATH=/opt/sdrcomms/lib/
./application --server-uri=tcp://127.0.0.1:11111 \
              --driver-name=remote \
              --remote-driver=plutosdr \
              --device-uri=ip:pluto.local \
              --bind-address=127.0.0.1
