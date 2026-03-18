#!/bin/bash

# install pico sdk
#git clone https://github.com/raspberrypi/pico-sdk.git ~/pico-sdk
#git -C ~/pico-sdk submodule update --init
export PICO_SDK_PATH=~/pico-sdk

rm -rf build
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -DAPPLE_MODEL=IIE -S . -B build
#-DAPPLE_MODEL=IIPLUS
cmake --build build -j $(nproc)
