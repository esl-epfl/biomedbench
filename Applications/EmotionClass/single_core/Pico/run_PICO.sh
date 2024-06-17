#!/bin/bash
# simple script to compile project for Raspberry Pi PICO

if [ -d "build" ]; then
    echo " ** Build already exists - Recompiling project ... ** "
else
    mkdir build
fi

cd build
cmake -DPICO_NO_FLASH=1 -DPICO_COPY_TO_RAM=0 ..
make
