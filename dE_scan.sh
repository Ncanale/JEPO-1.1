#!/bin/zsh

# my_array=(0 0.1 0.2 0.3)
my_array=(0.15 0.16 0.17 0.18 0.19 0.20 0.21 0.22 0.23 0.24 0.25)
# for (( i=0; i<$n_runs; i++ ))

# for i in "${my_array[@]}"; do echo "$i"; done
for i in "${my_array[@]}"
do
    echo "$i"
    sed -i .bak "s/^Smearing.*/Smearing=$i/" run_mac.sh
    sh run_mac.sh
done    