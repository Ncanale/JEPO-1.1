#!/bin/bash

dirname=$(pwd)
cd ..
mv dirname $(pwd)/source
mkdir build
cd build
cmake ../source
make -j$(nproc)
chmod 755 run.sh
