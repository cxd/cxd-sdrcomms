#!/bin/bash

# dependencies for raspberry pi 6.18.50+rpt-rpi-2712 #1 SMP PREEMPT Debian 1:6.18.50-1+rpt1 (2026-09-11) aarch64 GNU/Linux
sudo apt update
sudo apt install -y libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
sudo apt install libiio-dev libiio-utils libad9361-dev libad9361-0 iiod libiio0
sudo apt install libvulkan-dev vulkan-tools
sudo apt install glslang-dev glslang-tools glslc
sudo apt install libonnxruntime-dev -y
sudo apt install onnxruntime-tools libarmnnaclcommon33t64 libarmnn33t64 libarmnn-gpuacc-backend33 libarmnn-cpuacc-backend33 libarmnn-cpuref-backend33
sudo apt-get install -y libvulkan-dev vulkan-tools glslang-dev glslang-tools glslc mosquitto-dev libmosquitto-dev libonnxruntime-dev libliquid-dev libfftw3-dev