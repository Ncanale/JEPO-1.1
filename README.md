# JEPO-1.1
Geant4 Simulation of Polarimeter setup of COSY Storage Ring

FORK rules!
Fine .... relax!!!
You got that????

##################################################
#   Instructions                                      #
##################################################

1. create folder "build" inside the source root folder
2. run `cmake ..` from build

(or)

1. run build.sh or build_mac.sh

custom experiments can be run by using/modifying run.sh

Reproducible scans without editing the sources (Geant4 11, see History):

    cmake -G Ninja -DWITH_GEANT4_UIVIS=OFF .. && ninja            # in build/
    scripts/run_scan.sh build out/theta theta 100000 [keys-file]   # theta scan, full detector
    scripts/run_scan.sh build out/tb    tb    100000 [keys-file]   # 2018 test-beam setup
    scripts/run_scan.sh build out/u     uniform 500000 [keys-file] # one run, theta uniform
    python analysis/sith_reco.py theta|tb|uniform --runs 'out/theta/output/*MeV_t*-{run}.root' \
        --nruns 26 --geometry out/theta/config_last.cfg

keys-file holds "KEY value" lines overriding config.cfg, e.g. a different bar
geometry (TRACKERBASE, TRACKERHEIGHT, TRACKERNBARS, TRACKERLENGTH, TRACKERSETUP ALL)
or smearing model. scripts/analyse_original.sh runs the original ROOT tools
(output/Simulation_runner.cpp, output/Peak_fitter.py) on the same output.

scripts/scan_bars.sh runs the bar-size study (PHOTOSTAT calibration on the test
beam, overlap checks, one uniform run per geometry); notebooks/ 01-03 show the
validation of the port, the smearing calibration and the scan results (they read
the outputs in ~/runs).

##################################################
#   History                                      #
##################################################

= 6. Dec. 2017. by Hoyong Jeong = 
  - Teflon, tedlar, kapton have been added.
    These surround the crystals.
    Thicknesses are 50 um, 50 um and 25 um respectively.
    Whereas front direction has double thickness for each of layers.

= 11. Apr. 2020. by Rahul Shankar = 
  - Triangular Bar Trackers added (Front stack is fixed and rear stack is movable in the Y direction).
  - Activated option to turn on/off dE Scintillators.
  
= 28. Apr. 2020. by Rahul Shankar =
  - Added customizability to tracker-bar setup via config file.
  - Added feature to sequentially name the tracker bars drawn.
  
= 28. Jan. 2021. by Rahul Shankar =
  - Added ability to switch between parallel and perpendicular configurations of tracker bars
    in config file.
  - Disabled switches of fixed and moving columns.

= 29. Oct. 2026. by Nicola Canale =
  - Ported to Geant4 11 (g4root.hh removed, CMake >= 3.16), -s <seed> option.
  - Tracker bar geometry from config.cfg (TRACKERBASE, TRACKERHEIGHT, TRACKERNBARS,
    TRACKERLENGTH, TRACKERFRONTOFFSET): any number of bars per row; bars crossing
    the central hole are found geometrically. Defaults reproduce the previous
    geometry exactly (identical events with the same seed).
  - Energy smearing, threshold, generator angle mode (GENANGLE) and beam file
    (BEAMFILE) from config.cfg instead of sed on the sources. New PHOTOSTAT
    smearing model (sigma_E/E proportional to 1/sqrt(E)).
  - beam.in read once per thread and indexed by event ID: the test-beam runs
    are now reproducible (the line used by an event depended on the threads).
  - EMFLUCTUATION option to choose the Geant4 energy-loss fluctuation model.
  - ntuple columns position_X/position_Y now hold the true event vertex (beam
    spot included) instead of the nominal beam axis, so that the true hit
    position is known; sith_reco.py uniform reports radial and azimuthal
    position resolutions (mm) besides the polar and azimuthal angles.
  - scripts/run_scan.sh, analysis/sith_reco.py (Python port of
    Simulation_runner.cpp + Peak_fitter.py with configurable geometry,
    validated against them), analysis/compare_ntuples.py.
