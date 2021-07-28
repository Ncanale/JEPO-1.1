#!/bin/zsh

# my_array2=(Empty C)
my_array2=(C)
# my_array2=(Empty)

# my_array=(0 0.1 0.2 0.3)
# my_array=(0.18 0.3)
# my_array=(0 0.1 0.15 0.16 0.17 0.18 0.19 0.20 0.21 0.22 0.23 0.24 0.25 0.3)
# my_array=(0 0.1 0.15 0.16 0.17 0.18 0.19 0.20 0.21 0.22 0.221 0.222 0.223 0.224 0.225 0.226 0.227 0.228 0.229 0.23 0.24 0.25 0.3)
# my_array=( )
my_array=(0.224)
# for (( i=0; i<$n_runs; i++ ))

# for i in "${my_array[@]}"; do echo "$i"; done
for j in "${my_array2[@]}"
do
    sed -i .bak "s/^Target.*/Target=$j/" run_mac.sh
    for i in "${my_array[@]}"
    do
        echo "$i"
        sed -i .bak "s/^smearing.*/smearing=$i/" run_mac.sh
        sh run_mac.sh
    done    
done