#!/bin/zsh

cat run.sh |
sed "s+/bin/bash+/bin/zsh+" |
sed "s/sed -i \"/sed -i .bak \"/" |
sed "s+/usr/bin/ninja-build+make -j\$(nproc)+" |
sed "s+../src+../JEPO-1.1/src+" |
sed "s+../n_event.mac+../JEPO-1.1/n_event.mac+" |
sh
