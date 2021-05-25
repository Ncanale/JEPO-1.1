#!/bin/zsh

cd .. 
mkdir build
cd build
cmake ../JEPO-1.1
make -j$(nproc)
chmod 755 run_mac.sh
