#!/bin/zsh

sed -i .bak "s+//comment this section...+//comment this section... \n/*+" output/Simulation_runner.cpp
sed -i .bak "s+//... if ROOT is installed from source+*/\n//... if ROOT is installed from source+" output/Simulation_runner.cpp

cat run.sh |
sed "s+/bin/bash+/bin/zsh+" |
sed "s/sed -i \"/sed -i .bak \"/" |
sed "s+/usr/bin/ninja-build+make -j\$(nproc)+" |
sed "s+../src+../JEPO-1.1/src+" |
sh
