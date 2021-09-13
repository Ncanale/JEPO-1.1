#!/bin/zsh

n_events=10000

sed -i .bak "s/^n_events=.*/n_events=$n_events/" ../JEPO-1.1/run_mac.sh
# my_array2=(Empty C)
# my_array2=(C)
my_array2=(Empty)

# my_array=( 0.00 0.10 0.150 0.200 0.210 0.211 0.212 0.213 0.214 0.215 0.216 0.217 0.218 0.219 0.220 0.221 0.222 0.223 0.224 0.225 0.226 0.227 0.228 0.229 0.230 0.231 0.232 0.233 0.234 0.235 0.236 0.237 0.238 0.239 0.240 0.250 0.300)
my_array=( 0.220 )
# my_array=( 0.00 0.10 0.150 0.200 0.210 0.212 0.214 0.216 0.218 0.220 0.222 0.224 0.226 0.228 0.230 0.232 0.234 0.236 0.238 0.240 0.250 0.300)
# for (( i=0; i<$n_runs; i++ ))

rm -r *.bak
rm -r residual_*

# for i in "${my_array[@]}"; do echo "$i"; done
# root -q -l "beam_profile.cpp($n_events)" # Own code for generating beam profile, comment for new users
# sed -i .bak "s/m_FlagBeamFile =.*/m_FlagBeamFile = 1;/" ../JEPO-1.1/src/BT2017PriGenAct.cc # Own code for generating beam profile, comment for new users (or set = 0)

for j in "${my_array2[@]}"
do
    sed -i .bak "s/^Target.*/Target=$j/" ../JEPO-1.1/run_mac.sh
    for i in "${my_array[@]}"
    do
        echo "$i"
        sed -i .bak "s/^smearing.*/smearing=$i/" ../JEPO-1.1/run_mac.sh
        cmake ../JEPO-1.1
        # make clean
        make -j6
        sh run_mac.sh
    done   

cd output
# python3 dE_residual_plotter.py
cd ..

done