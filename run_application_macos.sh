#!/bin/bash

export LD_LIBRARY_PATH=/opt/homebrew/lib:$LD_LIBRARY_PATH
export VK_ICD_FILENAMES=/opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json
export VK_DRIVER_FILES=/opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json;VK_LOADER_DEBUG=all
export DYLD_FALLBACK_LIBRARY_PATH=/opt/homebrew/opt/molten-vk/lib:$DYLD_FALLBACK_LIBRARY_PATH
export SOAPY_SDR_PLUGIN_PATH=/Users/cd/Projects/cpp/sdrcomms/cmake-build-debug/_deps/soapypluto-build/

EXEC_PATH=cmake-build-debug/src/application/application

# using local pluto device driver
./$EXEC_PATH --driver-name=plutosdr \
             --remote-driver=plutosdr \
             --device-uri=ip:pluto.local \
             --bind-address= \
             --server-uri= \
             --udp-server-address=192.168.0.89 \
             --model-path=/Users/cd/Projects/cpp/sdrcomms/src/resources \
             --model-file=/Users/cd/Projects/cpp/sdrcomms/src/resources/mod_dualbranch_tcn.json \
             --class-map-file=/Users/cd/Projects/cpp/sdrcomms/src/resources/class_map_v1.json

