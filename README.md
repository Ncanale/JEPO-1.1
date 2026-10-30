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
    scripts/run_scan.sh build out/phi   phi   200000 [keys-file]   # phi scan, full detector
    scripts/run_scan.sh build out/tb    tb    100000 [keys-file]   # 2018 test-beam setup
    scripts/run_scan.sh build out/u     uniform 500000 [keys-file] # one run, theta uniform
    python analysis/sith_reco.py theta|phi|tb|uniform --runs 'out/theta/output/*MeV_t*-{run}.root' \
        --nruns 26 --geometry out/theta/config_last.cfg

The tb analysis reports the width of the offset (etaF - etaB, in mm) and the
single-layer resolution, width / sqrt(2). The phi analysis follows the thesis:
sigma_phi of the tracks reaching the LYSO face at r0 = 12 cm, extrapolated as
A cot(theta) with A = r0 sigma_phi / d; the uniform analysis instead fits
A cot(theta) over the whole theta range with uniform phi (A is larger, since
r0 = 12 cm is a favourable radius for the 16 scan directions).

keys-file holds "KEY value" lines overriding config.cfg, e.g. a different bar
geometry (TRACKERBASE, TRACKERHEIGHT, TRACKERNBARS, TRACKERLENGTH, TRACKERSETUP ALL)
or smearing model. scripts/analyse_original.sh runs the original ROOT tools
(output/Simulation_runner.cpp, output/Peak_fitter.py) on the same output.

scripts/scan_bars.sh runs the bar-size study (PHOTOSTAT and NOISE calibration on
the test beam, overlap checks, one uniform run per geometry and smearing model);
notebooks/ 01-04 show the validation of the port, the smearing calibration, the
scan results and the light collection (they read the outputs in ~/runs).

optical/ is a separate Geant4 application (sithopt) for the light collection of
one triangular bar: optical photons emitted in an EJ-200 bar wrapped in Teflon,
read out by 4 SiPMs (6x6 mm2) through a silicone pad, as on the 2018 boards:

    cd optical && mkdir build && cd build && cmake -G Ninja .. && ninja
    ./sithopt --base 60 --height 20 --length 420 --events 100000 --out b60_h20.root
    optical/run_optical.sh ~/runs/optical                     # all geometries + systematics
    python analysis/optical_summary.py ~/runs/optical --csv ~/runs/optical/summary.csv
    OPTICAL_CSV=~/runs/optical/summary.csv scripts/scan_bars.sh ~/runs/scan_bars

analysis/paper_figures.py writes the figures of the bar-size study for the paper.

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
  - NOISE smearing model (sigma_E independent of E, electronic noise).
  - optical/: light-collection simulation of one bar (sithopt), used to scale
    the photostatistics with the bar geometry (OPTICAL model of the scan).
  - scripts/run_scan.sh, analysis/sith_reco.py (Python port of
    Simulation_runner.cpp + Peak_fitter.py with configurable geometry,
    validated against them), analysis/compare_ntuples.py.
  - phi scan (run_scan.sh phi, sith_reco.py phi) as in the thesis; tb analysis
    also reports the single-layer resolution (offset width / sqrt(2)).
