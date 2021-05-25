#!/bin/zsh

mkdir build
cd build
cmake ..
make -j$(nproc)
chmod 755 run.sh
