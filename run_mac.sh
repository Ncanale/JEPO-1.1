#!/bin/zsh

n_cores=6
n_events=100000
n_runs=10

#beam properties
particle=deuteron
# target=C
target=Empty
energy=270
file_name="${particle}${target}-${energy}MeV"

#detector properties
# configuration=PARALLEL          #in all caps
configuration=PERPENDICULAR     #in all caps
# angle=PHI                       #in all caps
angle=THETA                     #in all caps

theta_start=5
theta_step=1

smearing=0.220

PERPENDICULAR="1111111111111111111111111111"
PARALLEL="0001000000110000010000001100"

rm -r output/*.bak
rm -r output/${particle}*-*

#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i .bak "s+/run/beamOn.*+/run/beamOn $n_events+" n_event.mac
#echo -e "\e[31mNumber of events: $n_events \e[39m..."
sed -i .bak "s/G4int NoE = .*/G4int NoE = $n_events;/" ../JEPO-1.1/src/BT2017PriGenAct.cc
sed -i .bak "s/G4double Smear = .*/G4double Smear = $smearing;/" ../JEPO-1.1/src/BT2017EveAct.cc

sed -i .bak "s+//comment this section...+//comment this section... \n/*+" output/Simulation_runner.cpp
sed -i .bak "s+//... if ROOT is installed from source+*/\n//... if ROOT is installed from source+" output/Simulation_runner.cpp

sed -i .bak "s/^NTHREADS.*/NTHREADS		    $n_cores/" config.cfg
sed -i .bak "s/^PARTICLENAME.*/PARTICLENAME            $particle/" config.cfg
sed -i .bak "s/^BEAMKINETICENERGY.*/BEAMKINETICENERGY	$energy/" config.cfg
sed -i .bak "s/^TRACKERCONFIG.*/TRACKERCONFIG	$configuration/" config.cfg
sed -i .bak "s/^TRACKERSETUP.*/TRACKERSETUP            ${!configuration}/" config.cfg

sed -i .bak "s/USETARGET.*/USETARGET		    OFF/" config.cfg

if [[ "$target" != "Empty" ]]
then
    sed -i .bak "s/USETARGET.*/USETARGET		    ON/" config.cfg
    sed -i .bak "s/TARGETMATERIAL.*/TARGETMATERIAL		$target/" config.cfg
fi

sed -i .bak "s/^const int n_runs.*/const int n_runs = $n_runs;/" output/Simulation_runner.cpp
sed -i .bak "s/^const int nth.*/const int nth = $n_cores;/" output/Simulation_runner.cpp
sed -i .bak "s/^int Energy.*/int Energy = $energy;/" output/Simulation_runner.cpp
sed -i .bak "s/^bool configuration.*/bool configuration = $configuration;/" output/Simulation_runner.cpp
sed -i .bak "s+^string Particle.*+string Particle = \"$particle\";+" output/Simulation_runner.cpp
sed -i .bak "s+^string Target.*+string Target = \"$target\";+" output/Simulation_runner.cpp

sed -i .bak "s/^n_runs      = .*/n_runs      = $n_runs/" output/Peak_fitter.py
sed -i .bak "s/^Target.*/Target = \"$target\"/" output/Peak_fitter.py
sed -i .bak "s/^Energy.*/Energy = $energy/" output/Peak_fitter.py
sed -i .bak "s/^Smearing.*/Smearing = $smearing/" output/Peak_fitter.py
sed -i .bak "s/^configuration.*/configuration = \"$configuration\"/" output/Peak_fitter.py

if [[ "$configuration" == "PARALLEL" ]]
then
  sed -i .bak "s/^setting=.*/setting='G4'/" output/Peak_fitter.py
  # root -q -l "beam_profile.cpp($n_events)" # Own code for generating beam profile, comment for new users
  # sed -i .bak "s/m_FlagBeamFile =.*/m_FlagBeamFile = 1;/" ../JEPO-1.1/src/BT2017PriGenAct.cc # Own code for generating beam profile, comment for new users (or set = 0)
elif [[ "$configuration" == "PERPENDICULAR" ]]
then
  sed -i .bak "s/m_FlagBeamFile =.*/m_FlagBeamFile = 0;/" ../JEPO-1.1/src/BT2017PriGenAct.cc # Own code for generating beam profile, comment for new users (or set = 0)
  sed -i .bak "s/^setting.*/setting = \"$angle\"/" output/Peak_fitter.py
  sed -i .bak "s/G4String angle=.*/G4String angle=\"$angle\";/" ../JEPO-1.1/src/BT2017PriGenAct.cc
  sed -i .bak "s/^n_runs      = .*/n_runs      = $n_runs +1/" output/Peak_fitter.py #FOR PHI SCAN  
  if [[ "$angle" == "THETA" ]]
  then 
   sed -i .bak "s/^theta_start = .*/theta_start = $theta_start/" output/Peak_fitter.py
   sed -i .bak "s/^theta_step = .*/theta_step = $theta_step/" output/Peak_fitter.py
  fi 
fi

sleep 1
# make clean
make -j6

# subtracted=(0.1  5 10 15 20 25 29.9)
subtracted=(0.24739106414386924 5.065845196500294 10.152798578987705 15.218870720097698 20.038899210298244 24.438954737516877 28.93078785640231)

# phi_array=(0 45 90 135 225 270 315)
phi_array=(0.0 22.5 45.0 67.5 90.0 112.5 135.0 157.5 202.5 225.0 247.5 270.0 292.5 315.0 337.5) #15

for (( i=0; i<$n_runs; i++ ))
do
  if [[ "$configuration" == "PARALLEL" ]]
  then
    echo -e "TRANSLATE ${subtracted[$i]}" 
    # sed -i .bak "s/TRANSLATE.*/TRANSLATE              	$((5 * $i))/" config.cfg
    sed -i .bak "s/TRANSLATE.*/TRANSLATE              	${subtracted[$i]}/" config.cfg
  elif [[ "$configuration" == "PERPENDICULAR" ]]
  then
    if [[ "$angle" == "PHI" ]]
    then
      sed -i .bak "s/MINTHETA.*/MINTHETA                ${phi_array[$i]}/" config.cfg
      sed -i .bak "s/MAXTHETA.*/MAXTHETA                ${phi_array[$i]}/" config.cfg
    elif [[ "$angle" == "THETA" ]]
    then
      sed -i .bak "s/MINTHETA.*/MINTHETA                $(($theta_start + ($theta_step*$i))).0/" config.cfg
      sed -i .bak "s/MAXTHETA.*/MAXTHETA                $(($theta_start + ($theta_step*$i))).0/" config.cfg
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
  # echo -e "\e[31mRun $(($i + 1)) done \e[39m..."
  echo -e "** ** ** ** ** Run $(($i + 1)) done ... ** ** ** ** ** "
done

cd output

# echo "Particle: $particle
# Target: $target
# Energy: $energy
# Smearing: $smearing
# Configuration: $configuration" > "details_$(date).txt"

# sed -i .bak "s/TFile f(\"Offsets.*/TFile f(\"Offsets_$i.root\",\"RECREATE\");/" Simulation_runner.cpp
# sed -i .bak "s/f2 = rt.TFile.Open(\"Offsets_.*/f2 = rt.TFile.Open(\"Offsets_$i.root\")/" Offset_plotter.py

root -q -l Simulation_runner.cpp
if [[ "$configuration" == "PARALLEL" ]]
then
    python3 Peak_fitter.py
    # python3 Offset_plotter.py 
fi
if [[ "$configuration" == "PERPENDICULAR" ]]
then
    python3 Peak_fitter.py
    # python3 Offset_plotter.py 
fi

 
cd ..
