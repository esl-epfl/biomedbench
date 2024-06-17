#!/bin/bash
# simple script to compile project for Raspberry Pi PICO

rm -r build
mkdir build
cd build
cmake -DPICO_NO_FLASH=0 -DPICO_COPY_TO_RAM=0 ..
make
