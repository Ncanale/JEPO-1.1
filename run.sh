#!/bin/bash

n_cores=$(nproc)
n_events=100000
n_runs=26

#beam properties
particle=deuteron
#target=C
target=Empty
energy=270
file_name="${particle}${target}-${energy}MeV"

#detector properties
#configuration=PARALLEL    #in all caps
configuration=PERPENDICULAR    #in all caps
#angle=PHI                       #in all caps
angle=THETA                     #in all caps

theta_start=5
theta_step=2

smearing=0.22

PERPENDICULAR="1111111111111111111111111111"
PARALLEL="0001000000110000010000001100"

rm -r output/*.bak
rm -r output/${particle}*-*

#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i "s+/run/beamOn.*+/run/beamOn $n_events+" ../n_event.mac
#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i "s/G4int NoE = .*/G4int NoE = $n_events;/" ../src/BT2017PriGenAct.cc
sed -i "s/G4double Smear = .*/G4double Smear = $smearing;/" ../src/BT2017EveAct.cc

if [[ "$configuration" == "PARALLEL" ]]
then
  root -q -l "beam_profile.cpp($n_events)" # Own code for generating beam profile, comment for new users
  sed -i  "s/m_FlagBeamFile =.*/m_FlagBeamFile = 1;/" ../src/BT2017PriGenAct.cc # Own code for generating beam profile, comment for new users (or set = 0)
elif [[ "$configuration" == "PERPENDICULAR" ]]
then
  sed -i "s/m_FlagBeamFile =.*/m_FlagBeamFile = 0;/" ../src/BT2017PriGenAct.cc # Own code for generating beam profile, comment for new users (or set = 0)
  sed -i "s/G4String angle=.*/G4String angle=\"$angle\";/" ../src/BT2017PriGenAct.cc
fi

/usr/bin/ninja-build

if [[ "$configuration" == "PARALLEL" ]]
then
  sed -i "s/^setting=.*/setting='G4'/" output/Peak_fitter.py
elif [[ "$configuration" == "PERPENDICULAR" ]]
then
  sed -i "s/^setting.*/setting = \"$angle\"/" output/Peak_fitter.py
  if [[ "$angle" == "THETA" ]]
  then 
   sed -i "s/theta_start = .*/theta_start = $theta_start/" output/Peak_fitter.py
   sed -i "s/theta_step = .*/theta_step = $theta_step/" output/Peak_fitter.py
  fi 
fi

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
sed -i "s/^Energy=.*/Energy=$energy/" output/Peak_fitter.py
sed -i "s/^Smearing=.*/Smearing=$smearing/" output/Peak_fitter.py
sed -i "s/^configuration.*/configuration = \"$configuration\"/" output/Peak_fitter.py

sleep 1
# subtracted=(0.1  5 10 15 20 25 29.9)
subtracted=(0.24739106414386924 5.065845196500294 10.152798578987705 15.218870720097698 20.038899210298244 24.438954737516877 28.93078785640231)

phi_array=(0.0 5.625 11.25 22.5 33.75 45.0 56.25 67.5 78.75 84.375 90.0 95.625 101.25 112.5 123.75 135.0 146.25 157.5 168.75 174.375 185.625 191.25 202.5 213.75 225.0 236.25 247.5 258.75 264.375 270.0 275.625 281.25 292.5 303.75 315.0 326.25 337.5 348.75 354.375) #39
theta_array=(5 5.5 6 6.5 7 7.5 8 8.5 9 9.5 10 10.5 11 11.5 12 12.5 13 13.5 14 14.5 15 15.5 16 16.5 17 17.5) #26
           
for (( i=0; i<$n_runs; i++ ))
do
	if [[ "$configuration" == "PARALLEL" ]]
	then
		echo -e "TRANSLATE ${subtracted[$i]}" 
		sed -i "s/TRANSLATE.*/TRANSLATE              	${subtracted[$i]}/" config.cfg
	elif [[ "$configuration" == "PERPENDICULAR" ]]
	then
		if [[ "$angle" == "PHI" ]]
		then
				sed -i "s/MINTHETA.*/MINTHETA                ${phi_array[$i]}/" config.cfg
				sed -i "s/MAXTHETA.*/MAXTHETA                ${phi_array[$i]}/" config.cfg
		elif [[ "$angle" == "THETA" ]]
		then
				sed -i "s/MINTHETA.*/MINTHETA                ${theta_array[$i]}/" config.cfg
				sed -i "s/MAXTHETA.*/MAXTHETA                ${theta_array[$i]}/" config.cfg
		else 
				echo -e "CHECK ANGLE!!!"
		fi
	else 
		echo -e "CHECK CONFIGURATION!!!"
 	fi
	sleep 1
	./jepo -m n_event.mac
  	for (( j=0; j<$n_cores; j++ ))
  	do
    	mv output/${file_name}_t$j.root output/${file_name}_t$j-$i.root
  	done

  	mv output/$file_name.root output/$file_name-$i.root
  	echo -e "\e[31mRun $(($i + 1)) done \e[39m..."
done

cd output
# echo "Particle: $particle
# Target: $target
# Energy: $energy
# Smearing: $smearing
# Configuration: $configuration" > "details_$(date).txt"

root -q -l Simulation_runner.cpp
python3 Peak_fitter.py
# if [[ "$configuration" == "PARALLEL" ]]
# then
#     python3 Offset_plotter.py 
# fi

cd ..
