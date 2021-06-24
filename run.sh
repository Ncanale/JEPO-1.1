#!/bin/bash

n_cores=8
n_events=100000
n_runs=4

#beam properties
particle=deuteron
#target=C
target=Empty
energy=270
file_name="${particle}${target}-${energy}MeV"

#detector properties
#configuration=PARALLEL    #in all caps
configuration=PERPENDICULAR    #in all caps
Smearing=0.26

rm -r output/${particle}*-*

#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i "s+/run/beamOn.*+/run/beamOn $n_events+" ../n_event.mac
#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i "s/G4int NoE = .*/G4int NoE = $n_events;/" ../src/BT2017PriGenAct.cc
sed -i "s/G4double Smear = .*/G4double Smear = $Smearing;/" ../src/BT2017EveAct.cc

/usr/bin/ninja-build

sed -i "s/^NTHREADS.*/NTHREADS		    $n_cores/" config.cfg
sed -i "s/^PARTICLENAME.*/PARTICLENAME            $particle/" config.cfg
sed -i "s/^BEAMKINETICENERGY.*/BEAMKINETICENERGY	$energy/" config.cfg
sed -i "s/^TRACKERCONFIG.*/TRACKERCONFIG	$configuration/" config.cfg

sed -i "s/USETARGET.*/USETARGET		    OFF/" config.cfg
if [[ "$target" != "Empty" ]]
then
    sed -i "s/USETARGET.*/USETARGET		    ON/" config.cfg
    sed -i "s/TARGETMATERIAL.*/TARGETMATERIAL		$target/" config.cfg
fi

sed -i "s/^int n_runs.*/int n_runs = $n_runs;/" output/Simulation_runner.cpp
sed -i "s/^const int nth.*/const int nth = $n_cores;/" output/Simulation_runner.cpp
sed -i "s/^int Energy.*/int Energy = $energy;/" output/Simulation_runner.cpp
sed -i "s/^bool configuration.*/bool configuration = $configuration;/" output/Simulation_runner.cpp
#sed -i "s+^string path_name.*+string path_name = \"$(pwd)/output/\";+" output/Simulation_runner.cpp
sed -i "s+^string Particle.*+string Particle = \"$particle\";+" output/Simulation_runner.cpp
sed -i "s+^string Target.*+string Target = \"$target\";+" output/Simulation_runner.cpp

sed -i "s/^n_runs      = .*/n_runs      = $n_runs/" output/Peak_fitter.py
sed -i "s/^Target= .*/Target= \"$target\"/" output/Peak_fitter.py
sed -i "s/^Smearing=.*/Smearing=$Smearing/" output/Peak_fitter.py
sed -i "s/^configuration.*/configuration = \"$configuration\"/" output/Peak_fitter.py

for (( i=0; i<$n_runs; i++ ))
do
  #sed -i "s/TRANSLATE.*/TRANSLATE              	$((5 * $i))/" config.cfg
  sed -i "s/MINTHETA.*/MINTHETA                $((6 + (3 * $i))).0/" config.cfg
  sed -i "s/MAXTHETA.*/MAXTHETA                $((6 + (3 * $i))).0/" config.cfg
  ./jepo -m n_event.mac

  for (( j=0; j<$n_cores; j++ ))
  do
    mv output/${file_name}_t$j.root output/${file_name}_t$j-$i.root
  done

  mv output/$file_name.root output/$file_name-$i.root
  echo -e "\e[31mRun $(($i + 1)) done \e[39m..."
done

#sed -i "s/TRANSLATE.*/TRANSLATE              	0/" config.cfg
#sed -i "s/MAXTHETA.*/MAXTHETA                5.0/" config.cfg
#sed -i "s/MINTHETA.*/MINTHETA                5.0/" config.cfg
cd output
echo "Particle: $particle
Target: $target
Energy: $energy
Smearing: $Smearing
Configuration: $configuration" > "details_$(date).txt"
root -q Simulation_runner.cpp
python3 Peak_fitter.py
cd ..
