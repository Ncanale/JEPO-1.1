#!/bin/bash

n_cores=8
n_events=100000
n_runs=7

#beam properties
particle=deuteron
#target=C
target=Empty
energy=270
file_name="${particle}${target}-${energy}MeV"

#detector properties
#configuration=PARALLEL    #in all caps
configuration=PERPENDICULAR    #in all caps
smearing=0.19

PERPENDICULAR="1111111111111111111111111111"
PARALLEL="0001000000110000010000001100"

rm -r output/${particle}*-*

#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i "s+/run/beamOn.*+/run/beamOn $n_events+" ../n_event.mac
#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i "s/G4int NoE = .*/G4int NoE = $n_events;/" ../src/BT2017PriGenAct.cc
sed -i "s/G4double Smear = .*/G4double Smear = $smearing;/" ../src/BT2017EveAct.cc

/usr/bin/ninja-build

sed -i "s/^NTHREADS.*/NTHREADS		    $n_cores/" config.cfg
sed -i "s/^PARTICLENAME.*/PARTICLENAME            $particle/" config.cfg
sed -i "s/^BEAMKINETICENERGY.*/BEAMKINETICENERGY	$energy/" config.cfg
sed -i "s/^TRACKERCONFIG.*/TRACKERCONFIG	$configuration/" config.cfg
sed -i "s/^TRACKERSETUP.*/TRACKERSETUP            ${!configuration}/" config.cfg

sed -i "s/USETARGET.*/USETARGET		    OFF/" config.cfg
if [[ "$target" != "Empty" ]]
then
    sed -i "s/USETARGET.*/USETARGET		    ON/" config.cfg
    sed -i "s/TARGETMATERIAL.*/TARGETMATERIAL		$target/" config.cfg
fi

sed -i "s/^const int n_runs.*/const int n_runs = $n_runs;/" output/Simulation_runner.cpp
sed -i "s/^const int nth.*/const int nth = $n_cores;/" output/Simulation_runner.cpp
sed -i "s/^int Energy.*/int Energy = $energy;/" output/Simulation_runner.cpp
sed -i "s/^bool configuration.*/bool configuration = $configuration;/" output/Simulation_runner.cpp
sed -i "s+^string Particle.*+string Particle = \"$particle\";+" output/Simulation_runner.cpp
sed -i "s+^string Target.*+string Target = \"$target\";+" output/Simulation_runner.cpp

sed -i "s/^n_runs      = .*/n_runs      = $n_runs/" output/Peak_fitter.py
sed -i "s/^Target= .*/Target= \"$target\"/" output/Peak_fitter.py
sed -i "s/^Smearing=.*/Smearing=$smearing/" output/Peak_fitter.py
sed -i "s/^Energy=.*/Energy=$energy/" output/Peak_fitter.py
sed -i "s/^configuration.*/configuration = \"$configuration\"/" output/Peak_fitter.py

# my_array=(0.1  5 10 15 20 25 29.9)
# diff_array=(-0.12653898391960397 -0.18102083710866382 -0.18259513068555297 -0.2821885293846259 -0.11893810067990816 0.4623504105310071 0.6781449768635459)
subtracted=(0.227 5.181 10.183 15.282 20.119 24.538 29.222)
# phi_array=(0 45 90 135 225 270 315)
phi_array=(0.0 22.5 45.0 67.5 90.0 112.5 135.0 157.5 202.5 225.0 247.5 270.0 292.5 315.0 337.5)

for (( i=0; i<$n_runs; i++ ))
do
	if [[ "$configuration" == "PARALLEL" ]]
	then
		#sed -i "s/TRANSLATE.*/TRANSLATE              	$((5 * $i))/" config.cfg
		sed -i "s/TRANSLATE.*/TRANSLATE              	${subtracted[$i]}/" config.cfg
	elif [[ "$configuration" == "PERPENDICULAR" ]]
	then
		sed -i "s/MINTHETA.*/MINTHETA                $((6 + (3 * $i))).0/" config.cfg
		sed -i "s/MAXTHETA.*/MAXTHETA                $((6 + (3 * $i))).0/" config.cfg
		#sed -i "s/MINTHETA.*/MINTHETA                ${phi_array[$i]}/" config.cfg
		#sed -i "s/MAXTHETA.*/MAXTHETA                ${phi_array[$i]}/" config.cfg
	else
		echo -e "\e[31mCHECK CONFIGURATION!!!\e[39m..."
	fi
	
	./jepo -m n_event.mac
  for (( j=0; j<$n_cores; j++ ))
  do
    mv output/${file_name}_t$j.root output/${file_name}_t$j-$i.root
  done

  mv output/$file_name.root output/$file_name-$i.root
  echo -e "\e[31mRun $(($i + 1)) done \e[39m..."
done

cd output
echo "Particle: $particle
Target: $target
Energy: $energy
Smearing: $smearing
Configuration: $configuration" > "details_$(date).txt"

root -q Simulation_runner.cpp
python3 Peak_fitter.py
if [[ "$configuration" == "PARALLEL" ]]
then
    python3 Offset_plotter.py 
fi

cd ..
